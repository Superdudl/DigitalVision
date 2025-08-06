#include <QApplication>
#include "view/mainwindow.h"
#include <QScreen>
#include <QDebug>
#include "controllers/maincontroller.h"

#include <memory>

int main(int argc, char* argv[])
{
    auto app = std::make_unique<QApplication>(argc, argv);
    auto window = new MainWindow();
    auto ui = window->ui;

    auto main_controller = std::make_unique<MainController>(ui, window);

    QList<QScreen*> screens = app->screens();

    for (int i = 0; i < screens.count(); ++i){
        auto screen = screens.at(i);
        qDebug() << "Монитор №" << i+1;
        qDebug() << "Имя:" << screen->name();
    }

    auto primeScreenGeometry = screens.at(0)->geometry();
    window->resize(0.8 * primeScreenGeometry.width(), 0.8 * primeScreenGeometry.height());
    window->move((primeScreenGeometry.width() - window->width()) / 2,
        (primeScreenGeometry.height() - window->height()) / 2);

    qDebug() << "Кнопка 1:" << window->ui->connect_button;
    window->show();
    app->exec();
}
