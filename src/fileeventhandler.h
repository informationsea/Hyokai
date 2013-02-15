#ifndef FILEEVENTHANDLER_H
#define FILEEVENTHANDLER_H

#include <QObject>
#include <QEvent>

class FileEventHandler : public QObject
{
    Q_OBJECT
public:
    explicit FileEventHandler(QObject *parent = 0);
    
    bool eventFilter(QObject *obj, QEvent *event);
signals:
    
public slots:
    
};

#endif // FILEEVENTHANDLER_H
