#include <QtGui/QApplication>
#include "mainwindow.h"

#include "main.h"
#include <QFileOpenEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QIcon>

#include "fileeventhandler.h"

QList<MainWindow *> windowList;
QSettings *tableview_settings;
PreferenceWindow *preferenceDialog;
static FileEventHandler *handler;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("TableView");
    a.setOrganizationDomain("informationsea.info");
    a.setOrganizationName("informationsea");
    a.setApplicationVersion("0.1");
#ifndef Q_WS_MAC
    a.setWindowIcon(QIcon(":/rc/images/icon16.png"));
#endif

    handler = new FileEventHandler(&a);
    a.installEventFilter(handler);

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

QList<int> selectedRowsFromSelection(QItemSelectionModel *selection)
{
    QList<int> rows;
    foreach (QModelIndex index, selection->selectedIndexes()) {
        if (!rows.contains(index.row()))
            rows << index.row();
    }
    qSort(rows);
    return rows;
}

#define WINDOW_STEP 22
#define MINIMUM_WINDOW_DISPLAY_SIZE 300
static int p = 100;

QPoint nextWindowPosition()
{
    QPoint point(p+50, p);
    p += WINDOW_STEP;
    if (p+50 > qApp->desktop()->screenGeometry().width()-MINIMUM_WINDOW_DISPLAY_SIZE || p > qApp->desktop()->screenGeometry().height()-MINIMUM_WINDOW_DISPLAY_SIZE)
        p = 100;
    return point;
}
