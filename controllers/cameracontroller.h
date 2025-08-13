#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H
#define THREADS_NUM 2

#include <windows.h>
#include "CameraApi.h"
#include <QObject>
#include <QThread>
#include <QPixmap>
#include <vector>
#include <opencv2/opencv.hpp>
#include "ui_mainwindow.h"
#include <memory>
#include <QMutex>

class CameraController;

class CameraThread : public QThread
{
    Q_OBJECT
public:
    explicit CameraThread(int *hCamera, Ui::MainWindow* ui, CameraController* controller);
    ~CameraThread();

protected:
    void run() override;
    int* hCamera;
    tSdkCameraCapbility* CameraInfo;
    Ui::MainWindow* ui;
    CameraController* controller;
    QImage left_frame;
    QImage right_frame;

signals:
    void grabbed_left_image(QPixmap pixmap);
    void grabbed_right_image(QPixmap pixmap);
};

class CameraController : public QObject
{
    Q_OBJECT
public:
    explicit CameraController(Ui::MainWindow* m_ui, QObject *parent = nullptr);
    ~CameraController();

    struct CameraSettings
    {
        FLOAT Gain;
        FLOAT GainMax;
        FLOAT GainMin;
        FLOAT GainStep;

        FLOAT Exposure;
        FLOAT ExposureMax;
        FLOAT ExposureMin;
        FLOAT ExposureStep;

        BOOL AeState;
    };

    // Переменные камер
    int CameraNums = CameraEnumerateDeviceEx(); // количество подключенных камер
    std::vector<int> hCamera; // хэнлеры камер
    std::vector<tSdkCameraDevInfo> CameraList; // внешние параметры камер
    std::vector<tSdkCameraCapbility> CameraInfo; // внутренние параметры камер
    std::vector<BOOL> CameraIsActive;
    std::vector<CameraSettings> params;

    // параметры камер

    // Указатель на GUI
    Ui::MainWindow* ui = nullptr;

    // Доступные потоки под камеры (максимум 2)
    std::vector<std::shared_ptr<CameraThread>> threads;

    // Переменные для хранения изображений с камер
    cv::Mat left_image;
    cv::Mat right_image;

    void setRightImage(BYTE* pFrameBuffer, tSdkFrameHead *FrameHead);
    void setLeftImage(BYTE* pFrameBuffer, tSdkFrameHead *FrameHead);

    cv::Mat getRightImage();
    cv::Mat getLeftImage();

private:
    QMutex left_mutex;
    QMutex right_mutex;

private slots:
    void connect_camera();
    void show_left_image(QPixmap pixmap);
    void show_right_image(QPixmap pixmap);
};

#endif // CAMERACONTROLLER_H
