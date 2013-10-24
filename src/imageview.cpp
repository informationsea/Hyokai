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
    painter.drawText(20, 20, "R is required to plot data.\n");
    painter.drawText(20, 40, "If you already installed R, but cannot plot, please set \"Rscript\" path at Edit -> Preference");
    painter.drawImage(0, 0, m_img);
}
