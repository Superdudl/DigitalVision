#include "maincontroller.h"
#include <QList>
#include <QScreen>
#include <QApplication>
#include <QPushButton>

#include <memory>

MainController::MainController(Ui::MainWindow* m_ui, QObject *parent)
    : QObject{parent}, ui{m_ui}
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
    camera_controller = std::make_unique<CameraController>(ui, this);
}
