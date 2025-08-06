#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include "../view/mainwindow.h"
#include "ui_mainwindow.h"

class MainController : public QObject
{
    Q_OBJECT
public:
    explicit MainController(Ui::MainWindow* ui, QObject *parent = nullptr);

    Ui::MainWindow* ui;

    void find_screens();
    void connect_controllers();

signals:
};

#endif // MAINCONTROLLER_H
