#include "maincontroller.h"
#include "cameracontroller.h"
#include <QList>
#include <QScreen>
#include <QApplication>

#include <memory>

MainController::MainController(Ui::MainWindow* ui, QObject *parent)
    : QObject{parent}, ui{ui}
{
    find_screens();
    connect_controllers();
}

void MainController::find_screens()
{
    QApplication* app = qApp;
    auto screens = app->screens();

    for (int i = 0; i < screens.count(); ++i)
    {
        auto screen = screens.at(i);
        ui->DisplayCombo->addItem(screen->name());

    }
}

void MainController::connect_controllers()
{
    auto camera_controller = std::make_shared<CameraController>(ui, this);
}
