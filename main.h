#ifndef MAIN_H
#define MAIN_H

#include <QList>
#include <QSettings>
#include "mainwindow.h"

extern QList<MainWindow *> windowList;
extern QSettings *tableview_settings;

void tableviewCleanupWindows();

#endif // MAIN_H
