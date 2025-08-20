#include "displaycontroller.h"
#include <cstdlib>
#include <QPushButton>
#include <QApplication>
#include <QScreen>
#include <opencv2/opencv.hpp>
#include <QDebug>

ShareThread::ShareThread(std::shared_ptr<CameraController> camera_controller, QObject *parent)

{
    this->camera_controller = camera_controller;
    auto index = camera_controller->ui->DisplayCombo->currentIndex();
    QApplication* app = qApp;
    auto screens = app->screens();


    shared_screen = new QLabel;
    shared_screen->setStyleSheet("background-color: black");
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
        if (!camera_controller->getLeftImage().empty() && !camera_controller->getRightImage().empty())
        {
            auto cv_left_image = camera_controller->getLeftImage();
            auto cv_right_image = camera_controller->getRightImage();

            cv::Mat final_image;
            cv::hconcat(cv_left_image, cv_right_image, final_image);
            QImage qimage(final_image.data, final_image.cols, final_image.rows, final_image.step, QImage::Format_RGB888);
            pixmap = QPixmap::fromImage(qimage.scaled(screen->size(), Qt::KeepAspectRatio));
        }

        if (!pixmap.isNull()){
            // shared_screen->setPixmap(pixmap);
            emit frame_ready(pixmap);
        }
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
    {
        thread->requestInterruption();
        thread->wait();
        running = FALSE;
    }
}

void ShareController::close()
{
    delete this;
}

void ShareController::update_image(QPixmap pixmap)
{
    thread->shared_screen->setPixmap(pixmap);
}

void ShareController::share_screen()
{
    system("DisplaySwitch.exe /extend");

    if (!running)
    {
        running = TRUE;
        thread = std::make_unique<ShareThread>(camera_controller);
        thread->start();
        //----------------------------------------  СЛОТЫ  --------------------------------------------------------------------------
        connect(thread.get(), &ShareThread::frame_ready, this, &ShareController::update_image, Qt::QueuedConnection);
        //---------------------------------------------------------------------------------------------------------------------------
    }
}
