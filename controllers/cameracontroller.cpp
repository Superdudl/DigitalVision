#include "cameracontroller.h"
#include <QMessageBox>
#include <QDebug>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QPixmap>
#include <QString>
#include <algorithm>
#include <QSettings>

extern QSettings qapp_settings;

CameraThread::CameraThread(int *hCamera, Ui::MainWindow* ui, CameraController *parent)
{
    this->controller = parent;
    this->hCamera = hCamera;
    this->CameraInfo = &controller->CameraInfo.at(*hCamera - 1);
    this->ui = ui;
    qDebug() << "Поток создан";

    FrameBufferSize = CameraInfo->sResolutionRange.iWidthMax * CameraInfo->sResolutionRange.iHeightMax *  3;
    pFrameBuffer = (BYTE *)CameraAlignMalloc(FrameBufferSize, 16);
}

CameraThread::~CameraThread()
{
    qDebug() << "Вызван деструктор потока";
    CameraStop(*hCamera);
}

void CameraThread::run()
{
    CameraPlay(*hCamera);
    qDebug() << "Поток запущен";
    QPixmap pixmap;

    flag_reversed = qapp_settings.value("camera/reversed", false).toBool();

    if (flag_reversed)
    {
        if (*hCamera == 1) possition = CameraPossition::RIGHT;
        if (*hCamera == 2) possition = CameraPossition::LEFT;
    }
    else
    {
        if (*hCamera == 1) possition = CameraPossition::LEFT;
        if (*hCamera == 2) possition = CameraPossition::RIGHT;
    }

    while (!isInterruptionRequested())
    {
        auto status = CameraGetImageBuffer(*hCamera, &FrameHead, &pRawData, 2000);

        if (status == CAMERA_STATUS_SUCCESS)
        {
            CameraImageProcess(*hCamera, pRawData, pFrameBuffer, &FrameHead);
            CameraReleaseImageBuffer(*hCamera, pRawData);
            auto frame = cv::Mat(FrameHead.iHeight, FrameHead.iWidth, CV_8UC3, static_cast<uchar*>(pFrameBuffer), FrameHead.iWidth * 3);
            switch (possition)
            {
            case CameraPossition::LEFT:
                controller->setLeftImage(frame, &FrameHead);
                pixmap = controller->getLeftImage();
                if (!pixmap.isNull())
                {
                    emit grabbed_left_image(pixmap);
                }
                break;
            case CameraPossition::RIGHT:
                controller->setRightImage(frame, &FrameHead);
                pixmap = controller->getRightImage();
                if (!pixmap.isNull())
                {
                     emit grabbed_right_image(pixmap);
                }
                break;
            }

            if (isInterruptionRequested())
                qDebug() << "Вызвано прерывание";
        }

    }
    qDebug() << "Выход из цикла";
    CameraClearBuffer(*hCamera);
    CameraStop(*hCamera);
}

CameraController::CameraController(Ui::MainWindow* m_ui, QObject *parent) : QObject(parent), ui{m_ui}
{

    //----------------------------------------  СЛОТЫ  ------------------------------------------------
    connect(ui->connect_button, &QPushButton::clicked, this, &CameraController::connect_camera);
    connect(ui->disconnect_button, &QPushButton::clicked, this, &CameraController::disconnect_camera);
    connect(ui->DeviceList, &QComboBox::activated, this, &CameraController::update_ui);
    connect(ui->AeState, &QCheckBox::clicked, this, &CameraController::clicked_AeState);
    connect(ui->Exposure_edit, &QLineEdit::editingFinished, this, &CameraController::edit_Exposure);
    connect(ui->Gain_edit, &QLineEdit::editingFinished, this, &CameraController::edit_Gain);
    connect(ui->changePositions, &QPushButton::clicked, this, &CameraController::change_positions);
    //-------------------------------------------------------------------------------------------------

    if (this->CameraNums < 1)
    {
        QMessageBox::warning(
            nullptr,
            "Ошибка",
            "Камеры не найдены. Подключите камеры и перезапустите программу."
            );
    }

    if (this->CameraNums > THREADS_NUM)
    {
        CameraNums = THREADS_NUM;
    }

    CameraList.resize(CameraNums);
    CameraInfo.resize(CameraNums);
    hCamera.resize(CameraNums);
    threads.resize(THREADS_NUM);
    params.resize(CameraNums);
    CameraIsActive.resize(CameraNums);


    for (int i = 0; i < CameraList.size(); i++)
    {
        hCamera.at(i) = i;
        CameraIsActive.at(i) = FALSE;
        auto status = CameraInitEx(i, -1, -1, &hCamera.at(i));

        if (status != CAMERA_STATUS_SUCCESS)
        {
            printf("Failed to init the camera! Error code is %d", status);
            return;
        }

        CameraGetEnumInfo(hCamera.at(i), &CameraList.at(i));

        qDebug() << "Камера №" << i+1 << "-" << CameraList.at(i).acFriendlyName << "(" << CameraList.at(i).acSn << ")";

        QString str = QString("%1 (SN:%2)").arg(CameraList.at(i).acFriendlyName, CameraList.at(i).acSn);
        ui->DeviceList->addItem(str);
    }
    update_ui();
    qDebug() << "Найдено камер:" << CameraList.size();
}

CameraController::~CameraController()
{
    if (CameraNums > 0)
    {
        for (int i = 0; i < CameraList.size(); ++i)
        {
            if (CameraIsActive.at(i))
            {
                threads.at(i)->requestInterruption();
                threads.at(i)->wait();
                qDebug() << threads.at(i)->isRunning();
                auto path = QString("SN%2.config").arg(CameraList.at(i).acSn).toStdString();
                CameraSaveParameterToFile(hCamera.at(i), path.data());
                CameraUnInit(hCamera.at(i));
            }
        }
    }
}

QPixmap CameraController::getLeftImage()
{
    QReadLocker locker(&left_mutex);
    return left_image;
}

QPixmap CameraController::getRightImage()
{
    QReadLocker locker(&right_mutex);
    return right_image;
}

void CameraController::setLeftImage(cv::Mat frame, tSdkFrameHead *FrameHead)
{
    QWriteLocker locker(&left_mutex);
    QImage qimage(frame.data, frame.cols, frame.rows, frame.step, QImage::Format::Format_BGR888);
    left_image = QPixmap::fromImage(qimage);
}

void CameraController::setRightImage(cv::Mat frame, tSdkFrameHead *FrameHead)
{
    QWriteLocker locker(&right_mutex);
    QImage qimage(frame.data, frame.cols, frame.rows, frame.step, QImage::Format::Format_RGB888);
    right_image = QPixmap::fromImage(qimage);
}

void CameraController::getCameraParams(int *index)
{
    auto params = &this->params.at(*index);
    auto hCamera = &this->hCamera.at(*index);
    CameraGetAnalogGainX(*hCamera, &params->Gain);
    CameraGetAnalogGainXRange(*hCamera, &params->GainMin, &params->GainMax, &params->GainStep);

    CameraGetExposureTime(*hCamera, &params->Exposure);
    CameraGetExposureTimeRange(*hCamera, &params->ExposureMin, &params->ExposureMax, &params->ExposureStep);

    CameraGetAeState(*hCamera, &params->AeState);
}

void CameraController::edit_Gain()
{
    auto index = ui->DeviceList->currentIndex();
    auto str = ui->Gain_edit->text();
    str.replace(',', '.');
    auto value = str.toFloat();
    value = std::clamp(value, params.at(index).GainMin, params.at(index).GainMax);
    CameraSetAnalogGainX(hCamera.at(index), value);
    update_ui();
}

void CameraController::edit_Exposure()
{
    auto index = ui->DeviceList->currentIndex();
    auto str = ui->Exposure_edit->text();
    str.replace(',', '.');
    auto value = str.toDouble();
    value = std::clamp(value, params.at(index).ExposureMin, params.at(index).ExposureMax);
    CameraSetExposureTime(hCamera.at(index), value);
    update_ui();
}

void CameraController::clicked_AeState()
{
    auto index = ui->DeviceList->currentIndex();
    CameraSetAeState(hCamera.at(index), ui->AeState->isChecked());
    update_ui();
}

void CameraController::update_ui()
{
    if (hCamera.size() < 1) return;
    auto index = ui->DeviceList->currentIndex();
    getCameraParams(&index);
    auto params = &this->params.at(index);

    if (CameraIsActive.at(index))
    {
        ui->connect_button->setEnabled(FALSE);
        ui->disconnect_button->setEnabled(TRUE);
        ui->AeState->setEnabled(TRUE);
    }
    else
    {
        ui->connect_button->setEnabled(TRUE);
        ui->disconnect_button->setEnabled(FALSE);
        ui->Exposure_edit->clear();
        ui->Exposure_edit->setEnabled(FALSE);
        ui->Gain_edit->clear();
        ui->Gain_edit->setEnabled(FALSE);
        ui->AeState->setChecked(FALSE);
        ui->AeState->setEnabled(FALSE);
        return;
    }

    if (!params->AeState)
    {
        ui->Exposure_edit->setEnabled(TRUE);
        ui->Exposure_edit->clear();
        auto value = QString::number(params->Exposure, 'f', 1);
        value.replace('.', ',');
        ui->Exposure_edit->insert(value);

        ui->Gain_edit->setEnabled(TRUE);
        ui->Gain_edit->clear();
        value = QString::number(params->Gain, 'f', 1);
        value.replace('.', ',');
        ui->Gain_edit->insert(value);

        ui->AeState->setChecked(FALSE);
    }
    else
    {
        ui->Exposure_edit->setEnabled(FALSE);
        ui->Exposure_edit->clear();
        auto value = QString::number(params->Exposure, 'f', 1);
        value.replace('.', ',');
        ui->Exposure_edit->insert(value);

        ui->Gain_edit->setEnabled(FALSE);
        ui->Gain_edit->clear();
        value = QString::number(params->Gain, 'f', 1);
        value.replace('.', ',');
        ui->Gain_edit->insert(value);

        ui->AeState->setChecked(TRUE);
    }
}

void CameraController::connect_camera()
{
    if (hCamera.size() < 1) return;
    auto index = ui->DeviceList->currentIndex();

    auto path = QString("SN%2.config").arg(CameraList.at(index).acSn).toStdString();
    auto status = CameraReadParameterFromFile(hCamera.at(index), path.data());

    // Максимум 2 потока
    if (!CameraIsActive.at(index))
    {
        CameraGetCapability(hCamera.at(index), &CameraInfo.at(index));
        CameraSetIspOutFormat(hCamera.at(index), CAMERA_MEDIA_TYPE_BGR8);

        auto pCamera = &hCamera.at(index);
        if (threads.at(index) == nullptr)
            threads.at(index) = std::make_shared<CameraThread>(pCamera, ui, this);
        CameraIsActive.at(index) = TRUE;

        connect(threads.at(index).get(), &CameraThread::grabbed_left_image, this, &CameraController::show_left_image, Qt::QueuedConnection);
        connect(threads.at(index).get(), &CameraThread::grabbed_right_image, this, &CameraController::show_right_image, Qt::QueuedConnection);

        threads.at(index)->start();
        update_ui();
    }
}

void CameraController::disconnect_camera()
{
    auto index = ui->DeviceList->currentIndex();
    if (CameraIsActive.at(index))
    {
        threads.at(index)->requestInterruption();
        threads.at(index)->wait(500);
        CameraIsActive.at(index) = FALSE;
        auto path = QString("SN%2.config").arg(CameraList.at(index).acSn).toStdString();
        CameraSaveParameterToFile(hCamera.at(index), path.data());
        QPixmap new_pixmap (1,1);
        new_pixmap.fill(Qt::black);

        auto possition = threads.at(index)->possition;

        switch (possition)
        {
        case CameraPossition::LEFT:
            disconnect(threads.at(index).get(), &CameraThread::grabbed_left_image, this, &CameraController::show_left_image);
            qApp->removePostedEvents(this, QEvent::MetaCall);
            show_left_image(new_pixmap);
        case CameraPossition::RIGHT:
            disconnect(threads.at(index).get(), &CameraThread::grabbed_right_image, this, &CameraController::show_right_image);
            qApp->removePostedEvents(this, QEvent::MetaCall);
            show_right_image(new_pixmap);
        }
    }
    update_ui();
}

void CameraController::show_left_image(QPixmap pixmap)
{
    QReadLocker lockL(&left_mutex);
    this->ui->left_camera->setPixmap(pixmap.scaled(ui->left_camera->size(), Qt::KeepAspectRatio, Qt::FastTransformation));
}

void CameraController::show_right_image(QPixmap pixmap)
{
    QReadLocker lockR(&right_mutex);
    this->ui->right_camera->setPixmap(pixmap.scaled(ui->right_camera->size(), Qt::KeepAspectRatio, Qt::FastTransformation));
}

void CameraController::change_positions()
{
    qDebug() << qapp_settings.fileName();
    qapp_settings.setValue("camera/reversed", !CameraThread::flag_reversed);
    for (auto thread : threads)
    {
        if (thread != nullptr && thread->isRunning())
        {
            thread->requestInterruption();
            thread->wait(500);
            qApp->removePostedEvents(this, QEvent::MetaCall);
            QPixmap new_pixmap (1,1);
            new_pixmap.fill(Qt::black);
            show_left_image(new_pixmap);
            show_right_image(new_pixmap);
            thread->start();
        }
    }
    update_ui();
}
