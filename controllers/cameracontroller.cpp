#include "cameracontroller.h"
#include <QMessageBox>
#include <QDebug>
#include <ui_mainwindow.h>
#include <QComboBox>
#include <QPushButton>
#include <QPixmap>

CameraThread::CameraThread(int *hCamera, tSdkCameraCapbility* CameraInfo, QObject *parent)
{
    this->setParent(parent);
    this->hCamera = hCamera;
    this->CameraInfo = CameraInfo;

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

    while (!isInterruptionRequested())
    {
        auto status = CameraGetImageBufferEx2(*hCamera, pFrameBuffer, 1, &FrameHead.iWidth, &FrameHead.iHeight, 2000);

        if (status == CAMERA_STATUS_SUCCESS)
        {
            FrameHead.uiMediaType = CAMERA_MEDIA_TYPE_BGR8;
            FrameHead.uBytes = FrameHead.iWidth * FrameHead.iHeight * 3;
            QImage image(pFrameBuffer, FrameHead.iWidth, FrameHead.iHeight, FrameHead.iWidth * 3, QImage::Format::Format_RGB888);
            emit frame_grabbed(image, hCamera);

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

    CameraSdkStatus status;

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


    for (int i = 0; i < CameraList.size(); i++)
    {
        hCamera.at(i) = i;
        CameraGetEnumInfo(hCamera.at(i), &CameraList.at(i));
        status = CameraInitEx(i, -1, -1, &hCamera.at(i));

        if (status != CAMERA_STATUS_SUCCESS)
        {
            printf("Failed to init the camera! Error code is %d", status);
            break;
        }

        CameraGetEnumInfo(hCamera.at(i), &CameraList.at(i));

        qDebug() << "Камера №" << i+1 << "-" << CameraList.at(i).acFriendlyName << "(" << CameraList.at(i).acSn << ")";

        QString str = QString("%1 (SN:%2)").arg(CameraList.at(i).acFriendlyName, CameraList.at(i).acSn);
        ui->DeviceList->addItem(str);

        CameraGetCapability(hCamera.at(i), &CameraInfo.at(i));

        CameraSetIspOutFormat(hCamera.at(i), CAMERA_MEDIA_TYPE_BGR8);

        // BOOL AeState;
        // int TriggerMode;

        // CameraGetTriggerMode(hCamera.at(i), &TriggerMode);
        CameraSetTriggerMode(hCamera.at(i), 0);
        // CameraGetAeState(hCamera.at(i), &AeState);
        // CameraSetAeState(hCamera.at(i), TRUE);

        CameraSetAeState(hCamera.at(i), TRUE);

    }

    qDebug() << "Найдено камер:" << CameraList.size();
}

CameraController::~CameraController()
{
    if (CameraNums > 0)
    {
        for (int i = 0; i < CameraList.size(); ++i)
        {
            threads.at(i)->requestInterruption();
            threads.at(i)->wait();
            qDebug() << threads.at(i)->isRunning();
            CameraUnInit(hCamera.at(i));
        }
    }
}

void CameraController::connect_camera()
{
    auto index = ui->DeviceList->currentIndex();

    // Максимум 2 потока

    if (threads.size() <= THREADS_NUM && hCamera.size() > 0)
    {
        auto pCamera = &hCamera.at(index);
        threads.at(index) = std::make_unique<CameraThread>(pCamera, &CameraInfo.at(index), this);
        ui->connect_button->setEnabled(FALSE);
        ui->disconnect_button->setEnabled(TRUE);
        threads.at(index)->start();

        //----------------------------------------  СЛОТЫ  ---------------------------------------------------------------------------------------
        connect(threads.at(index).get(), &CameraThread::frame_grabbed, this, &CameraController::update_frame, Qt::QueuedConnection);
        //----------------------------------------------------------------------------------------------------------------------------------------
    }
}


void CameraController::update_frame(QImage image, const int *hCamera)
{
    QPixmap pixmap;
    switch (*hCamera)
    {
    case 1:
        left_image = image;
        pixmap = QPixmap::fromImage(left_image, Qt::ColorOnly);
        break;
    case 2:
        right_image = image;
        pixmap.fromImage(left_image);
        break;
    }
    if (!pixmap.isNull())
    {
        pixmap = pixmap.scaled(ui->left_camera->size(), Qt::KeepAspectRatio);
        ui->left_camera->setPixmap(pixmap);
    }
}


