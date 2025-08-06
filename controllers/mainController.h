#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include "../view/mainwindow.h"
#include "ui_mainwindow.h"
#include "cameracontroller.h"
#include <memory>

class MainController : public QObject
{
    Q_OBJECT
public:
    explicit MainController(Ui::MainWindow* m_ui, QObject *parent = nullptr);

    Ui::MainWindow* ui;

    void find_screens();
    void connect_controllers();

private:
    std::unique_ptr<CameraController> camera_controller;

};

#endif // MAINCONTROLLER_H
