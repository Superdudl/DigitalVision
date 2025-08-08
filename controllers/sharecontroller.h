#ifndef SHARECONTROLLER_H
#define SHARECONTROLLER_H

#include <QObject>
#include <QLabel>
#include <ui_mainwindow.h>
#include "cameracontroller.h"
#include <QThread>
#include <QScreen>

class ShareThread : public QThread
{
    Q_OBJECT
public:
    explicit ShareThread(std::shared_ptr<CameraController>, QObject *parent = nullptr);
    ~ShareThread();

    void run() override;

private:
    std::shared_ptr<CameraController> camera_controller;
    QLabel* shared_screen;
    QScreen* screen;
};

class ShareController : public QObject
{
    Q_OBJECT
public:
    explicit ShareController(std::shared_ptr<CameraController> camera_controller, QObject *parent = nullptr);
    ~ShareController();
    void close();

private:
    std::shared_ptr<CameraController> camera_controller;
    std::shared_ptr<ShareThread> thread;
    BOOL running = FALSE;
signals:

private slots:
    void share_screen();
};

#endif // SHARECONTROLLER_H
