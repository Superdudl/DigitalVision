#include "displaycontroller.h"
#include <QReadWriteLock>
#include <QPushButton>
#include <QApplication>
#include <QScreen>
#include <opencv2/opencv.hpp>
#include <QDebug>
#include <chrono>

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
    left_shared_screen->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    left_shared_screen->showFullScreen();

    right_shared_screen = new QLabel;
    right_shared_screen->setStyleSheet("background-color: black");
    screen = screens.at(right_index);
    geometry = screens.at(right_index)->geometry();
    right_shared_screen->move(geometry.x(), geometry.y());
    right_shared_screen->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
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
    using clock = std::chrono::steady_clock;
    auto time = clock::now();
    auto frame_duration = std::chrono::milliseconds(1000 / 30);

    while (!isInterruptionRequested())
    {
        time += frame_duration;

        left_pixmap = camera_controller->getLeftImage().scaled(left_shared_screen->size(), Qt::KeepAspectRatio, Qt::FastTransformation);
        right_pixmap = camera_controller->getRightImage().scaled(right_shared_screen->size(), Qt::KeepAspectRatio, Qt::FastTransformation);

        emit frame_ready(left_pixmap, right_pixmap);

        if (clock::now() > time)
        {
            time = clock::now();
        }

        std::this_thread::sleep_until(time);
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

void ShareController::share_screen()
{
    if (!running)
    {
        running = TRUE;
        thread = std::make_unique<ShareThread>(camera_controller);
        thread->start();
        //----------------------------------------  СЛОТЫ  --------------------------------------------------------------------------
        connect(camera_controller->ui->stop_button, &QPushButton::clicked, this, &ShareController::stop_sharing);
        connect(thread.get(), &ShareThread::frame_ready, this, &ShareController::update_frames, Qt::QueuedConnection);
        //---------------------------------------------------------------------------------------------------------------------------
    }
    return;
}

void ShareController::update_frames(QPixmap left_frame, QPixmap right_frame)
{
    if (!left_frame.isNull())
        thread->left_shared_screen->setPixmap(left_frame);
    if (!right_frame.isNull())
        thread->right_shared_screen->setPixmap(right_frame);
}

void ShareController::stop_sharing()
{
    if (running)
    {
        thread->blockSignals(true);
        qApp->removePostedEvents(this, QEvent::MetaCall);
        thread->requestInterruption();
        thread->wait();
        thread.reset();
        running = FALSE;
    }
}






















