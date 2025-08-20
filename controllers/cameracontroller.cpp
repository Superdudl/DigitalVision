#include "cameracontroller.h"
#include <QMessageBox>
#include <QDebug>
#include <ui_mainwindow.h>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QPixmap>
#include <QMutexLocker>
#include <QString>
#include <algorithm>

CameraThread::CameraThread(int *hCamera, Ui::MainWindow* ui, CameraController *parent)
{
    this->controller = parent;
    this->hCamera = hCamera;
    this->CameraInfo = &controller->CameraInfo.at(*hCamera - 1);
    this->ui = ui;
    qDebug() << "Поток создан";
}

CameraThread::~CameraThread()
{
    qDebug() << "Вызван деструктор потока";
    CameraStop(*hCamera);
}

void CameraThread::run()
{
    qDebug() << "Поток запущен";
    CameraPlay(*hCamera);
    UINT FrameBufferSize = CameraInfo->sResolutionRange.iWidthMax * CameraInfo->sResolutionRange.iHeightMax *  3;
    BYTE* pFrameBuffer = (BYTE *)CameraAlignMalloc(FrameBufferSize, 16);

    tSdkFrameHead FrameHead;
    QPixmap pixmap;

    while (!isInterruptionRequested())
    {
        auto status = CameraGetImageBufferEx2(*hCamera, pFrameBuffer, 1, &FrameHead.iWidth, &FrameHead.iHeight, 2000);

        if (status == CAMERA_STATUS_SUCCESS)
        {
            FrameHead.uiMediaType = CAMERA_MEDIA_TYPE_BGR8;
            FrameHead.uBytes = FrameHead.iWidth * FrameHead.iHeight * 3;
            QImage scaled_image;
            switch (*hCamera)
            {
            case 1:
                left_frame = QImage(pFrameBuffer, FrameHead.iWidth, FrameHead.iHeight, FrameHead.iWidth * 3, QImage::Format::Format_RGB888);
                controller->setLeftImage(pFrameBuffer, &FrameHead);
                scaled_image = left_frame.scaled(ui->left_camera->size(), Qt::KeepAspectRatio);
                pixmap = QPixmap::fromImage(scaled_image);
                if (!pixmap.isNull())
                {
                    emit grabbed_left_image(pixmap);
                }
                break;
            case 2:
                right_frame = QImage(pFrameBuffer, FrameHead.iWidth, FrameHead.iHeight, FrameHead.iWidth * 3, QImage::Format::Format_RGB888);
                controller->setRightImage(pFrameBuffer, &FrameHead);
                scaled_image = right_frame.scaled(ui->right_camera->size(), Qt::KeepAspectRatio);
                pixmap = QPixmap::fromImage(scaled_image);
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

cv::Mat CameraController::getLeftImage()
{
    QMutexLocker locker(&left_mutex);
    return left_image;
}

cv::Mat CameraController::getRightImage()
{
    QMutexLocker locker(&right_mutex);
    return right_image;
}

void CameraController::setLeftImage(BYTE* pFrameBuffer, tSdkFrameHead *FrameHead)
{
    QMutexLocker locker(&left_mutex);
    left_image = cv::Mat(FrameHead->iHeight, FrameHead->iWidth, CV_8UC3, const_cast<uchar*>(pFrameBuffer),  FrameHead->iWidth * 3).clone();
}

void CameraController::setRightImage(BYTE* pFrameBuffer, tSdkFrameHead *FrameHead)
{
    QMutexLocker locker(&right_mutex);
    right_image = cv::Mat(FrameHead->iHeight, FrameHead->iWidth, CV_8UC3, const_cast<uchar*>(pFrameBuffer),  FrameHead->iWidth * 3).clone();
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

//--------------------------------------------------------------- СЛОТЫ -------------------------------------------------------------------
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
        threads.at(index) = std::make_shared<CameraThread>(pCamera, ui, this);
        CameraIsActive.at(index) = TRUE;
        //----------------------------------------  СЛОТЫ  --------------------------------------------------------------------------
        connect(threads.at(index).get(), &CameraThread::grabbed_left_image, this, &CameraController::show_left_image, Qt::QueuedConnection);
        connect(threads.at(index).get(), &CameraThread::grabbed_right_image, this, &CameraController::show_right_image, Qt::QueuedConnection);
        //---------------------------------------------------------------------------------------------------------------------------
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
        threads.at(index)->wait();
        CameraIsActive.at(index) = FALSE;
        auto path = QString("SN%2.config").arg(CameraList.at(index).acSn).toStdString();
        CameraSaveParameterToFile(hCamera.at(index), path.data());
    }
    update_ui();
}

void CameraController::show_left_image(QPixmap pixmap)
{
    this->ui->left_camera->setPixmap(pixmap);
}

void CameraController::show_right_image(QPixmap pixmap)
{
    this->ui->right_camera->setPixmap(pixmap);
}

