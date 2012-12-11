#include "imageview.h"

#include <QPaintEngine>
#include <QDebug>

ImageView::ImageView(QWidget *parent) :
    QWidget(parent)
{
}

void ImageView::setImage(const QImage &img)
{
    m_img = img;
}

void ImageView::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.drawText(20, 20, "No Image");
    painter.drawImage(0, 0, m_img);
}
