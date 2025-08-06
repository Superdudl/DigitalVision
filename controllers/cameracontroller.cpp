#include "cameracontroller.h"
#include <QMessageBox>
#include <QDebug>
#include <ui_mainwindow.h>
#include <QComboBox>
#include <QPushButton>

CameraThread::CameraThread(int *hCamera, QObject *parent)
{
    this->setParent(parent);
    this->hCamera = hCamera;

    qDebug() << "Поток создан";
}

CameraThread::~CameraThread()
{

}

void CameraThread::run()
{

}

CameraController::CameraController(Ui::MainWindow* m_ui, QObject *parent) : QObject(parent), ui{m_ui}
{

    //--------------------СЛОТЫ------------------------------------------------------------------------
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

    CameraList.resize(CameraNums);
    CameraInfo.resize(CameraNums);
    hCamera.resize(CameraNums);
    threads.reserve(2);


    for (int i = 0; i < CameraList.size(); ++i)
    {
        hCamera.at(i) = i;
        CameraGetEnumInfo(hCamera.at(i), &CameraList.at(i));
        status = CameraInitEx(i, -1, -1, &hCamera.at(i));
        CameraGetEnumInfo(hCamera.at(i), &CameraList.at(i));

        qDebug() << "Камера №" << i+1 << "-" << CameraList.at(i).acFriendlyName << "(" << CameraList.at(i).acSn << ")";

        QString str = QString("%1 (SN:%2)").arg(CameraList.at(i).acFriendlyName, CameraList.at(i).acSn);
        ui->DeviceList->addItem(str);

        CameraGetCapability(hCamera.at(i), &CameraInfo.at(i));

        BOOL bMonoSensor = CameraInfo.at(i).sIspCapacity.bMonoSensor;
        if (bMonoSensor)
        {
            CameraSetIspOutFormat(hCamera.at(i), CAMERA_MEDIA_TYPE_MONO8);
        }
        CameraSetTriggerMode(hCamera.at(i), 0);
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
            CameraUnInit(hCamera.at(i));
        }
    }

}

void CameraController::connect_camera()
{
    // auto index = ui->DeviceList->currentIndex();

    qDebug() << "Слот";

    // // Максимум 2 потока
    // for (int i = 0; i < 2; ++i)
    // {
    //     if (!threads.at(i)->isRunning())
    //     {
    //         auto pCamera = &hCamera.at(index);
    //         auto thread = std::make_unique<CameraThread>(pCamera, this);
    //         threads.push_back(std::move(thread));
    //     }
    // }
    return;
}


void CameraController::update_frame(const QImage &image)
{

}
