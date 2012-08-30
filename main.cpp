#include <QtGui/QApplication>
#include "mainwindow.h"

#include "main.h"
#include "sqlite3.h"

QList<MainWindow *> windowList;
QSettings *tableview_settings;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("TableView");
    a.setOrganizationDomain("informationsea.info");
    a.setOrganizationName("informationsea");
    a.setApplicationVersion("0.1");
    tableview_settings = new QSettings(&a);
    MainWindow *w = new MainWindow;
    w->show();
    windowList.append(w);
    
    int value = a.exec();
    tableview_settings->sync();
    delete tableview_settings;
    return value;
}

void tableviewCleanupWindows()
{
    foreach(MainWindow* window, windowList) {
        if (!window->isVisible()) {
            delete window;
            windowList.removeOne(window);
        }
    }
}
