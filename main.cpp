#include <QtGui/QApplication>
#include "mainwindow.h"

#include "main.h"

QList<MainWindow *> windowList;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow *w = new MainWindow;
    w->show();
    windowList.append(w);
    
    return a.exec();
}
