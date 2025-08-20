#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include "../view/mainwindow.h"
#include "cameracontroller.h"
#include "displaycontroller.h"
#include <memory>
#include <QDoubleValidator>

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

    QDoubleValidator m_doubleValidator;
private slots:
    void close();
};

#endif // MAINCONTROLLER_H
