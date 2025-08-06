#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <windows.h>
#include "CameraApi.h"
#include <QObject>
#include <vector>
#include "ui_mainwindow.h"

class CameraController : public QObject
{
    Q_OBJECT
public:
    explicit CameraController(Ui::MainWindow* ui, QObject *parent = nullptr);


private:
    int CameraNums = CameraEnumerateDeviceEx();
    std::vector<tSdkCameraDevInfo> CameraList;
    Ui::MainWindow* ui = nullptr;

signals:
};

#endif // CAMERACONTROLLER_H
