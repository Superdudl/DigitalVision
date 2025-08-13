#ifndef SHARECONTROLLER_H
#define SHARECONTROLLER_H

#include <windows.h>
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
    QLabel* shared_screen;

private:
    std::shared_ptr<CameraController> camera_controller;
    QScreen* screen;
signals:
    void frame_ready(QPixmap pixmap);
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
    std::unique_ptr<ShareThread> thread;
    BOOL running = FALSE;
signals:

private slots:
    void share_screen();
    void update_image(QPixmap pixmap);
};

#endif // SHARECONTROLLER_H
