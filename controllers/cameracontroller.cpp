#include "cameracontroller.h"
#include <QMessageBox>
#include <QDebug>
#include <ui_mainwindow.h>
#include <QComboBox>
#include <QPushButton>
#include <QPixmap>

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
            QImage image(pFrameBuffer, FrameHead.iWidth, FrameHead.iHeight, FrameHead.iWidth * 3, QImage::Format::Format_RGB888);
            auto scaled_image = image.copy().scaled(ui->left_camera->size(), Qt::KeepAspectRatio);
            pixmap = QPixmap::fromImage(scaled_image).scaled(ui->left_camera->size(), Qt::KeepAspectRatio);;
            switch (*hCamera)
            {
            case 1:
                controller->setLeftImage(image);
                if (!pixmap.isNull())
                {
                    ui->left_camera->setPixmap(pixmap);
                }
                break;
            case 2:
                controller->setRightImage(image);
                if (!pixmap.isNull())
                {
                    ui->right_camera->setPixmap(pixmap);
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

QImage CameraController::getLeftImage()
{
    std::lock_guard lock(mtx);
    return left_image;
}

QImage CameraController::getRightImage()
{
    std::lock_guard lock(mtx);
    return right_image;
}

void CameraController::setLeftImage(QImage image)
{
    std::lock_guard lock(mtx);
    left_image = image;
}

void CameraController::setRightImage(QImage image)
{
    std::lock_guard lock(mtx);
    right_image = image;
}

void CameraController::connect_camera()
{
    if (hCamera.size() < 1) return;
    auto index = ui->DeviceList->currentIndex();

    auto path = QString("SN%2.config").arg(CameraList.at(index).acSn).toStdString();
    // auto status = CameraReadParameterFromFile(hCamera.at(index), path.data());

    // Максимум 2 потока
    if (!CameraIsActive.at(index))
    {
        CameraSetAeState(hCamera.at(index), TRUE);
        CameraGetCapability(hCamera.at(index), &CameraInfo.at(index));
        CameraSetIspOutFormat(hCamera.at(index), CAMERA_MEDIA_TYPE_BGR8);

        auto pCamera = &hCamera.at(index);
        threads.at(index) = std::make_unique<CameraThread>(pCamera, ui, this);
        CameraIsActive.at(index) = TRUE;
        threads.at(index)->start();
    }
}


