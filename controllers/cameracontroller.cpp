#include "cameracontroller.h"
#include <QMessageBox>
#include <QDebug>

CameraController::CameraController(QObject *parent)
    : QObject{parent}
{
    if (this->CameraNums < 1)
    {
        QMessageBox::warning(
            nullptr,
            "Ошибка",
            "Камеры не найдены. Подключите камеры и перезапустите программу."
            );
    }

    qDebug() << "Найдено камер:" << CameraList.size();
}
