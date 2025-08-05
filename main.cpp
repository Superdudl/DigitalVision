#include <QApplication>
#include "view/mainwindow.h"
#include <QScreen>
#include <QDebug>
#include "controllers/maincontroller.h"

#include <memory>

int main(int argc, char* argv[])
{
    auto app = std::make_shared<QApplication>(argc, argv);
    auto window = std::make_shared<MainWindow>();

    auto main_controller = std::make_shared<MainController>(window, window.get());

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

    window->show();
    app->exec();
}
