#ifndef MAIN_H
#define MAIN_H

#include <QList>
#include <QSettings>
#include <QObject>
#include "mainwindow.h"
#include "preferencewindow.h"

#include <QItemSelectionModel>
#include <QPoint>

#define LAST_IMPORT_DIRECTORY "LAST_IMPORT_DIRECTORY"
#define LAST_EXPORT_DIRECTORY "LAST_EXPORT_DIRECTORY"
#define LAST_SQLITE_DIRECTORY "LAST_SQLITE_DIRECTORY"

#define SQL_FILTER_HISTORY "SQL_FILTER_HISTORY"
#define SQL_FILTER_HISTORY_MAX 20

extern QList<MainWindow *> windowList;
extern QSettings *tableview_settings;
extern PreferenceWindow *preferenceDialog;

void tableviewCleanupWindows();
QList<int> selectedRowsFromSelection(QItemSelectionModel *selection);
QPoint nextWindowPosition();
QString addQuote(QString name);
QString removeQuote(QString name);
QString suggestRPath();

#endif // MAIN_H
