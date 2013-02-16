#include "mainwindow.h"

#include "main.h"
#include <QFileOpenEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QIcon>
#include <QFileInfo>

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
    a.setWindowIcon(QIcon(":/rc/images/icon128.png"));
#endif

    QStringList filelist;
    if (argc > 1) {
        for (int i = 0; i < argc-1; ++i) {
            filelist << argv[i+1];
        }
    } else {
        filelist << ":memory:";
    }

    handler = new FileEventHandler(&a);
    a.installEventFilter(handler);

    tableview_settings = new QSettings(&a);
    MainWindow *w = new MainWindow(NULL, filelist[0]);
    w->show();
    windowList.append(w);

    for (int i = 1; i < filelist.length(); ++i) {
        MainWindow *w2 = new MainWindow(NULL, filelist[i]);
        w2->show();
        windowList.append(w2);
    }
    
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

QString addQuote(QString name) {
    if (name.startsWith("\"") && name.endsWith("\""))
        return name;
    return QString("\"%1\"").arg(name);
}

QString removeQuote(QString name) {
    if (name.startsWith("\"") && name.endsWith("\"")) {
        return name.mid(1, name.size()-2);
    }
    return name;
}

QString suggestRPath()
{
#define R_CHECK_AND_RETURN(path) \
    if (QFileInfo(path).isExecutable())\
        return path;

    R_CHECK_AND_RETURN("/usr/bin/Rscript");
    R_CHECK_AND_RETURN("/usr/local/bin/Rscript");
    R_CHECK_AND_RETURN("/opt/local/bin/Rscript");

    return "Rscript";
}

QString normstr(QString str, bool shoudStartWithAlpha)
{
    str = str.trimmed();
    if (str.size() == 0)
        str = "V";
    if (str.at(0).isDigit() && shoudStartWithAlpha) {
        str.insert(0, 'V');
    }
    str = str.replace("\"", "");
    str = str.replace(QRegExp("[^a-zA-Z0-9_]"), "_");
    return str;
}
