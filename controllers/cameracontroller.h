#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <stdio.h>
#include <windows.h>
#include "CameraApi.h"

#include <QObject>

#include <memory>
#include <vector>

class Camera
{
private:

};

#endif // CAMERACONTROLLER_H

class CameraController : public QObject
{
    Q_OBJECT
public:
    explicit CameraController(QObject *parent = nullptr);


private:
    int CameraNums = CameraEnumerateDeviceEx();
    std::vector<tSdkCameraDevInfo> CameraList;

signals:
};
