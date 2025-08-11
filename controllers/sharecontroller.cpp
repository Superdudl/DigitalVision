#include "sharecontroller.h"
#include <cstdlib>
#include <QPushButton>
#include <QApplication>
#include <QScreen>
#include <opencv2/opencv.hpp>
#include <QDebug>

ShareThread::ShareThread(std::shared_ptr<CameraController> camera_controller, QObject *parent)

{
    this->setParent(parent);
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
        if (!camera_controller->getLeftImage().isNull() && !camera_controller->getRightImage().isNull())
        {
            auto left_qimage = camera_controller->getLeftImage();
            auto right_qimage = camera_controller->getLeftImage();

            auto cv_left_image = cv::Mat(left_qimage.height(), left_qimage.width(), CV_8UC3, const_cast<uchar*>(left_qimage.bits()), left_qimage.bytesPerLine());
            auto cv_right_image = cv::Mat(right_qimage.height(), right_qimage.width(), CV_8UC3, const_cast<uchar*>(right_qimage.bits()), right_qimage.bytesPerLine());


            cv::Mat final_image;
            cv::hconcat(cv_left_image, cv_right_image, final_image);
            QImage qimage(final_image.data, final_image.cols, final_image.rows, final_image.step, QImage::Format_RGB888);
            pixmap = QPixmap::fromImage(qimage.scaled(screen->size()));
        }

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
