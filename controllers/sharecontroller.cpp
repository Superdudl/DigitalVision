#include "sharecontroller.h"
#include <cstdlib>
#include <QPushButton>
#include <QApplication>
#include <QScreen>

ShareThread::ShareThread(std::shared_ptr<CameraController> camera_controller, QObject *parent)

{
    this->setParent(parent);
    this->camera_controller = camera_controller;
    auto index = camera_controller->ui->DisplayCombo->currentIndex();
    QApplication* app = qApp;
    auto screens = app->screens();


    shared_screen = new QLabel;
    screen = screens.at(index);
    auto geometry = screens.at(index)->geometry();
    shared_screen->move(geometry.x(), geometry.y());
    shared_screen->showFullScreen();
}

ShareThread::~ShareThread()
{
    delete shared_screen;
}

void ShareThread::run()
{
    QPixmap pixmap;
    while (!isInterruptionRequested())
    {
        if (camera_controller->getLeftImage().isNull()) continue;;
        auto image = camera_controller->getLeftImage().copy();
        pixmap = QPixmap::fromImage(image.scaled(screen->size()));
        if (pixmap.isNull()) continue;
        shared_screen->setPixmap(pixmap);
    }
}

ShareController::ShareController(std::shared_ptr<CameraController> camera_controller, QObject *parent) : QObject{parent}
{
    this->camera_controller = camera_controller;
    connect(camera_controller->ui->start_button, &QPushButton::clicked, this, &ShareController::share_screen);
}

ShareController::~ShareController()
{
    if (running)
        thread->requestInterruption();
        running = FALSE;
}

void ShareController::close()
{
    delete this;
}

void ShareController::share_screen()
{
    system("DisplaySwitch.exe /extend");

    if (!running)
    {
        running = TRUE;
        thread = std::make_shared<ShareThread>(camera_controller);
        thread->start();
    }
}
