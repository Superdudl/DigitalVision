#include "maincontroller.h"
#include <QList>
#include <QScreen>
#include <QApplication>
#include <QPushButton>

#include <memory>

MainController::MainController(MainWindow* window, QObject *parent)
    : QObject{parent}, window{window}
{
    //-------------------------------- СЛОТЫ -------------------------------------
    connect(window, &MainWindow::windowClosing, this, &MainController::close);
    //----------------------------------------------------------------------------
    find_screens();
    connect_controllers();
}

MainController::~MainController()
{

}

void MainController::close()
{
    share_controller->close();
}

void MainController::find_screens()
{
    QApplication* app = qApp;
    auto screens = app->screens();

    for (int i = 0; i < screens.count(); ++i)
    {
        auto screen = screens.at(i);
        window->ui->DisplayCombo->addItem(screen->name());
    }
}

void MainController::connect_controllers()
{
    camera_controller = std::make_shared<CameraController>(window->ui, this);
    share_controller = new ShareController(camera_controller, this);
}
