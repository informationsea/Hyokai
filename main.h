#ifndef MAIN_H
#define MAIN_H

#include <QList>
#include <QSettings>
#include "mainwindow.h"

extern QList<MainWindow *> windowList;
extern QSettings *tableview_settings;

void tableviewCleanupWindows();

// sqlite3-extension-functions.c
extern "C" {
    int RegisterExtensionFunctions(sqlite3 *db);
}

#endif // MAIN_H
