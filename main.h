#ifndef MAIN_H
#define MAIN_H

#include <QList>
#include <QSettings>
#include <QObject>
#include "mainwindow.h"
#include "preferencewindow.h"

#include <QItemSelectionModel>
#include <QPoint>

extern QList<MainWindow *> windowList;
extern QSettings *tableview_settings;
extern PreferenceWindow *preferenceDialog;

void tableviewCleanupWindows();
QList<int> selectedRowsFromSelection(QItemSelectionModel *selection);
QPoint nextWindowPosition();
QString addQuote(QString name);
QString removeQuote(QString name);

#endif // MAIN_H
