#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <windows.h>
#include "CameraApi.h"
#include <QObject>
#include <QThread>
#include <vector>
#include "ui_mainwindow.h"
#include <memory>

class CameraThread : public QThread
{
    Q_OBJECT
public:
    explicit CameraThread(int *hCamera, QObject* parent = nullptr);
    ~CameraThread();

protected:
    void run() override;
    int* hCamera;

signals:
    void frame_grabbed(const QImage &image);

};

class CameraController : public QObject
{
    Q_OBJECT
public:
    explicit CameraController(Ui::MainWindow* m_ui, QObject *parent = nullptr);
    ~CameraController();

private:
    // Переменные камер
    int CameraNums = CameraEnumerateDeviceEx(); // количество подключенных камер
    std::vector<int> hCamera; // хэнлеры камер
    std::vector<tSdkCameraDevInfo> CameraList; // внешние параметры камер
    std::vector<tSdkCameraCapbility> CameraInfo; // внутренние параметры камер

    // Указатель на GUI
    Ui::MainWindow* ui = nullptr;

    // Доступные потоки под камеры (максимум 2)
    std::vector<std::unique_ptr<CameraThread>> threads;

    // Переменные для хранения изображений с камер
    QImage left_image;
    QImage right_image;


private slots:
    void update_frame(const QImage &image);
    void connect_camera();
};

#endif // CAMERACONTROLLER_H
