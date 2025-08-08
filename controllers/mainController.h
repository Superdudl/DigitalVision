#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include "../view/mainwindow.h"
#include "ui_mainwindow.h"
#include "cameracontroller.h"
#include "sharecontroller.h"
#include <memory>

class MainController : public QObject
{
    Q_OBJECT
public:
    explicit MainController(MainWindow* window, QObject *parent = nullptr);
    ~MainController();

    MainWindow* window;

    void find_screens();
    void connect_controllers();

private:
    std::shared_ptr<CameraController> camera_controller;
    ShareController* share_controller;
private slots:
    void close();
};

#endif // MAINCONTROLLER_H
