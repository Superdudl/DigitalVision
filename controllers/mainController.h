#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include "../view/mainwindow.h"
#include "ui_mainwindow.h"


#include <memory>
class MainController : public QObject
{
    Q_OBJECT
public:
    explicit MainController(std::shared_ptr<MainWindow> window, QObject *parent = nullptr);

    std::shared_ptr<MainWindow> window = nullptr;
    std::shared_ptr<Ui::MainWindow> ui = nullptr;

    void find_screens();
    void connect_controllers();

signals:
};

#endif // MAINCONTROLLER_H
