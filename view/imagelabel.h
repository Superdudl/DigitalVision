#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include <QLabel>
#include <QObject>
#include <QImage>
#include <QPainter>

namespace Ui {
class ImageLabel;
}

class ImageLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ImageLabel(QWidget *parent = nullptr);
    ~ImageLabel();
    using QLabel::setPixmap;

    void setImage(QImage image);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Ui::ImageLabel *ui;
    QImage image;
};

#endif // IMAGELABEL_H
