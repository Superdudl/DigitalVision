#include "displaycontroller.h"
#include <QPushButton>
#include <QApplication>
#include <QScreen>
#include <opencv2/opencv.hpp>
#include <QDebug>

ShareThread::ShareThread(std::shared_ptr<CameraController> camera_controller, QObject *parent)

{
    this->camera_controller = camera_controller;
    auto left_index = camera_controller->ui->DisplayLeft->currentIndex();
    auto right_index = camera_controller->ui->DisplayRight->currentIndex();
    QApplication* app = qApp;
    auto screens = app->screens();


    left_shared_screen = new QLabel;
    left_shared_screen->setStyleSheet("background-color: black");
    screen = screens.at(left_index);
    auto geometry = screens.at(left_index)->geometry();
    left_shared_screen->move(geometry.x(), geometry.y());
    left_shared_screen->showFullScreen();

    right_shared_screen = new QLabel;
    right_shared_screen->setStyleSheet("background-color: black");
    screen = screens.at(right_index);
    geometry = screens.at(right_index)->geometry();
    right_shared_screen->move(geometry.x(), geometry.y());
    right_shared_screen->showFullScreen();
}

ShareThread::~ShareThread()
{
    delete left_shared_screen;
    delete right_shared_screen;
}

void ShareThread::run()
{
    QPixmap left_pixmap, right_pixmap;
    while (!isInterruptionRequested())
    {
        if (!camera_controller->getLeftImage().empty() && !camera_controller->getRightImage().empty())
        {
            auto cv_left_image = camera_controller->getLeftImage();
            auto cv_right_image = camera_controller->getRightImage();

            QImage q_left_image(cv_left_image.data, cv_left_image.cols, cv_left_image.rows, cv_left_image.step, QImage::Format_BGR888);
            left_pixmap = QPixmap::fromImage(q_left_image.scaled(screen->size(), Qt::KeepAspectRatio, Qt::FastTransformation));

            QImage q_right_image(cv_right_image.data, cv_right_image.cols, cv_right_image.rows, cv_right_image.step, QImage::Format_BGR888);
            right_pixmap = QPixmap::fromImage(q_right_image.scaled(screen->size(), Qt::KeepAspectRatio, Qt::FastTransformation));
        }

        if (!left_pixmap.isNull() && !right_pixmap.isNull()){
            emit frame_ready(left_pixmap, right_pixmap);
        }
    }
    return;
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

void ShareController::update_image(QPixmap left_pixmap, QPixmap right_pixmap)
{
    thread->left_shared_screen->setPixmap(left_pixmap);
    thread->right_shared_screen->setPixmap(right_pixmap);
}

void ShareController::share_screen()
{
    if (!running)
    {
        running = TRUE;
        thread = std::make_unique<ShareThread>(camera_controller);
        thread->start();
        //----------------------------------------  СЛОТЫ  --------------------------------------------------------------------------
        connect(thread.get(), &ShareThread::frame_ready, this, &ShareController::update_image, Qt::QueuedConnection);
        connect(camera_controller->ui->stop_button, &QPushButton::clicked, this, &ShareController::stop_sharing);
        //---------------------------------------------------------------------------------------------------------------------------
    }
    return;
}

void ShareController::stop_sharing()
{
    if (running)
    {
        thread->requestInterruption();
        thread->wait();
        thread.reset();
        running = FALSE;
    }
    return;
}






















