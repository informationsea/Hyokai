#include "fileeventhandler.h"

#include <QFileOpenEvent>
#include <QApplication>
#include "main.h"
#include "mainwindow.h"
#include <QDebug>
#include "sqlfileimporter.h"

FileEventHandler::FileEventHandler(QObject *parent) :
    QObject(parent)
{
}

bool FileEventHandler::eventFilter(QObject *obj, QEvent *event)
{
    if (qApp == obj && event->type() == QEvent::FileOpen) {
        QFileOpenEvent *openevent = static_cast < QFileOpenEvent *>(event);
        qDebug() << openevent->file();

        if (openevent->file().endsWith(".sqlite3") ||
                openevent->file().endsWith(".sqlite")) {
            ::tableviewCleanupWindows();

            MainWindow *w = new MainWindow(NULL, openevent->file());
            w->show();

            // close in memory database
            if (::windowList.length() == 1) {
                MainWindow *onlyWindow = ::windowList.at(0);
                if (onlyWindow->isVisible() &&
                        onlyWindow->databaseName() == ":memory:" &&
                        !onlyWindow->isDirty() && onlyWindow->database().tables().size() == 0) {
                    onlyWindow->close();
                }
            }

            ::windowList.append(w);
            event->accept();
            return true;
        } else {

            MainWindow *w;
            foreach (MainWindow *window, ::windowList) {
                if (window->isVisible() && window->databaseName() == ":memory:") {
                    window->activateWindow();
                    w = window;
                    goto opened;
                }
            }

            ::tableviewCleanupWindows();
            w = new MainWindow(NULL, ":memory:");
            w->show();
            ::windowList.append(w);

opened:

            SqlAsynchronousFileImporter *importer = new SqlAsynchronousFileImporter(&w->database(), w);
            importer->executeImport(QStringList(openevent->file()));
            connect(importer, SIGNAL(finish(QStringList,bool,QString)), w, SLOT(importFinished(QStringList,bool,QString)));
            //w->refresh();

            return true;
        }
    }
    return false;
}
