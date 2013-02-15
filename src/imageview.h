#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QWidget>
#include <QPaintEvent>

class ImageView : public QWidget
{
    Q_OBJECT
public:
    explicit ImageView(QWidget *parent = 0);
    
signals:
    
public slots:
    void setImage(const QImage &img);

protected:
    void paintEvent ( QPaintEvent * event );

private:
    QImage m_img;
    
};

#endif // IMAGEVIEW_H
