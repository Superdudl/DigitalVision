#include "cameracontroller.h"
#include <QMessageBox>
#include <QDebug>
#include <ui_mainwindow.h>

CameraController::CameraController(Ui::MainWindow* ui, QObject *parent)
    : QObject{parent}, ui{ui}
{
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
    std::vector<int> hCamera (CameraNums);
    for (int i = 0; i < CameraList.size(); ++i)
    {
        hCamera.at(i) = i;
        CameraGetEnumInfo(hCamera.at(i), &CameraList.at(i));
        status = CameraInitEx(i, -1, -1, &hCamera.at(i));
        CameraGetEnumInfo(hCamera.at(i), &CameraList.at(i));

        qDebug() << "Камера №" << i+1 << "-" << CameraList.at(i).acFriendlyName << "(" << CameraList.at(i).acSn << ")";

        QString str = QString("%1 (SN:%2)").arg(CameraList.at(i).acFriendlyName).arg( CameraList.at(i).acSn);
        ui->DeviceList->addItem(str);
    }

    qDebug() << "Найдено камер:" << CameraList.size();
}
