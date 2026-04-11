#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H
#define THREADS_NUM 2

#include <windows.h>
#include "CameraApi.h"
#include <ui_mainwindow.h>
#include <QObject>
#include <QThread>
#include <QPixmap>
#include <vector>
#include <opencv2/opencv.hpp>
#include <memory>
#include <QReadWriteLock>

class CameraController;

enum class CameraPossition : int
{
    LEFT = 1,
    RIGHT = 2
};

class CameraThread : public QThread
{
    Q_OBJECT
public:
    explicit CameraThread(int *hCamera, Ui::MainWindow* ui, CameraController* controller);
    ~CameraThread();
    CameraPossition possition;
    static inline bool flag_reversed = false;

protected:
    void run() override;
    int* hCamera;
    tSdkCameraCapbility* CameraInfo;
    Ui::MainWindow* ui;
    CameraController* controller;
    QImage left_frame;
    QImage right_frame;

    UINT FrameBufferSize;
    BYTE* pFrameBuffer;
    tSdkFrameHead FrameHead;
    BYTE *pRawData;

signals:
    void grabbed_left_image(QPixmap pixmap);
    void grabbed_right_image(QPixmap pixmap);

private slots:

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

        double Exposure;
        double ExposureMax;
        double ExposureMin;
        double ExposureStep;

        BOOL AeState;
    };

    // Переменные камер
    int CameraNums = CameraEnumerateDeviceEx(); // количество подключенных камер
    std::vector<int> hCamera; // хэндлеры камер
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
    QPixmap left_image;
    QPixmap right_image;

    // Копирование изображений с камеры в QPixmap left_image
    void setRightImage(cv::Mat frame, tSdkFrameHead *FrameHead);
    // Копирование изображений с камеры в QPixmap right_image
    void setLeftImage(cv::Mat frame, tSdkFrameHead *FrameHead);

    QPixmap getRightImage();
    QPixmap getLeftImage();

    inline QReadWriteLock* getLeftMutex(){ return &left_mutex; }
    inline QReadWriteLock* getRightMutex(){ return &right_mutex; }


private:
    QReadWriteLock left_mutex;
    QReadWriteLock right_mutex;

    // Получение параметров камеры
    void getCameraParams(int *index);

private slots:
    void disconnect_camera();
    void connect_camera();
    void update_ui();
    void clicked_AeState();
    void edit_Exposure();
    void edit_Gain();
    void show_left_image(QPixmap pixmap);
    void show_right_image(QPixmap pixmap);
    void change_positions();
};

#endif // CAMERACONTROLLER_H
