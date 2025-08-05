#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <windows.h>
#include "CameraApi.h"
#include <QObject>
#include <vector>

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

#endif // CAMERACONTROLLER_H
