#include "mainwindow.h"

#include "main.h"
#include <QFileOpenEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QIcon>
#include <QFileInfo>
#include "sqlfileimporter.h"

#ifdef Q_OS_MACX
#if QT_VERSION >= 0x050000 && QT_VERSION < 0x050200
#include <QMacFunctions>
#endif
#endif

#include "fileeventhandler.h"

QList<MainWindow *> windowList;
QSettings *tableview_settings;
PreferenceWindow *preferenceDialog;
static FileEventHandler *handler;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Hyokai");
    a.setOrganizationDomain("informationsea.info");
    a.setOrganizationName("informationsea");
    a.setApplicationVersion("0.1");
#ifndef Q_OS_MACX
    a.setWindowIcon(QIcon(":/rc/images/icon128.png"));
#endif

    QStringList filelist;
    QStringList importlist;
    if (argc > 1) {
        for (int i = 0; i < argc-1; ++i) {
            QString path(argv[i+1]);
            if (path.endsWith(".sqlite") || path.endsWith(".sqlite3"))
                filelist << path;
            else
                importlist << path;
        }

    } else {
        filelist << ":memory:";
    }


    handler = new FileEventHandler(&a);
    a.installEventFilter(handler);

    tableview_settings = new QSettings(&a);

    if (!tableview_settings->contains(CREATE_SQL_HISTORY_TABLE)) {
        tableview_settings->setValue(CREATE_SQL_HISTORY_TABLE, true);
    }

    for (int i = 0; i < filelist.length(); ++i) {
        MainWindow *w2 = new MainWindow(NULL, filelist[i]);
        w2->show();
        windowList.append(w2);
    }

    if (!importlist.isEmpty()) {
        MainWindow *w2 = new MainWindow(NULL, ":memory:");
        w2->show();
        windowList.append(w2);
        SqlAsynchronousFileImporter *importer = new SqlAsynchronousFileImporter(&w2->database(), w2);
        importer->executeImport(importlist);
        QObject::connect(importer, SIGNAL(finish(QStringList,bool,QString)), w2, SLOT(importFinished(QStringList,bool,QString)));
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
            windowList.removeOne(window);
            delete window;
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

bool isDarkMode() {
    auto basecolor = QApplication::palette().color(QPalette::Base).toHsl().lightnessF();
    return basecolor < 0.5;
}
