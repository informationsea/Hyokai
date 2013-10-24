#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <cmath>

#include "main.h"
#include "schemadialog.h"
#include "sheetmessagebox.h"
#include "sheettextinputdialog.h"
#include "customsqldialog.h"
#include "sqltablemodelalternativebackground.h"
#include "attachdatabasedialog.h"
#include "summarydialog.h"
#include "databaseconnectiondialog.h"
#include "sqlservice.h"
#include "sqlplotchart.h"
#include "sqlfileimporter.h"
#include "sqlfileexporter.h"

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QDesktopServices>
#include <QUrl>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QHeaderView>
#include <QCloseEvent>
#include <QFile>
#include <QRegExp>
#include <QDebug>
#include <QSqlRecord>
#include <QPixmap>
#include <QMenu>
#include <QList>
#include <QVariant>
#include <QClipboard>
#include <QDesktopServices>
#include <QUrl>
#include <QAction>
#include <QProgressDialog>
#include <QSqlField>
#include <QTimer>

#include <tablereader.hpp>
#include <csvreader.hpp>

#ifdef Q_OS_MACX
#if QT_VERSION >= 0x050000
#include <QMacNativeToolBar>
#endif
#endif

#include "sqlite3-extension/sqlite3.h"

#define RECENT_FILES "RECENT_FILES"
#define RECENT_FILES_MAX 10

static QSqlDatabase sqlite = QSqlDatabase::addDatabase("QSQLITE");
static int open_count = 0;

extern "C" {
int RegisterExtensionFunctions(sqlite3 *db);
}

MainWindow::MainWindow(QWidget *parent, QString path) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), m_isDirty(false)
{
    m_databasename = path;

    open_count++;
    m_database = QSqlDatabase::cloneDatabase(sqlite, QString::number(open_count));
    m_database.setDatabaseName(path);
    m_database.open();

    initialize();

    if (m_database.driverName() == "QSQLITE") {
        QList<QVariant> attachdb = tableview_settings->value(ATTACHED_DATABASES).toList();
        foreach(QVariant l, attachdb) {
            QStringList list = l.toStringList();
            QString as = list[0].replace("\"", "");
            QString db = list[1].replace("\"", "");
            QSqlQuery q = m_database.exec(QString("ATTACH DATABASE \"%1\" AS \"%2\"").arg(db, as));
            if (q.lastError().type() != QSqlError::NoError) {
                SheetMessageBox::critical(NULL, tr("Cannot attach"), m_database.lastError().text()+"\n\n"+q.lastQuery());
            }
        }
    }
}

MainWindow::MainWindow(const QSqlDatabase &database, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), m_isDirty(false)
{
    m_databasename = database.databaseName();
    m_database = database;
    initialize();
}

void MainWindow::initialize()
{
    ui->setupUi(this);
    m_rowcountlabel = new QLabel(ui->statusBar);
    //ui->statusBar->addWidget(sqlLineCount);
    ui->statusBar->addPermanentWidget(m_rowcountlabel);

    move(nextWindowPosition());

#if !defined(Q_OS_WIN32)
    QVariant v = m_database.driver()->handle();
    if (v.isValid() && qstrcmp(v.typeName(), "sqlite3*") == 0) {
        // v.data() returns a pointer to the handle
        sqlite3 *handle = *static_cast<sqlite3 **>(v.data());
        if (handle != 0) { // check that it is not NULL
            //sqlite3_enable_load_extension(handle, 1); // Enable extension
            RegisterExtensionFunctions(handle);
        }
    }
#endif

    ui->actionAbout_Qt->setMenuRole(QAction::AboutQtRole);
    ui->actionAbout_Table_View->setMenuRole(QAction::AboutRole);
    ui->actionQuit->setMenuRole(QAction::QuitRole);
    ui->actionPreference->setMenuRole(QAction::PreferencesRole);

    ui->mainToolBar->setIconSize(QSize(32, 32));

    setupTableModel();
    updateDatabase();

#if QT_VERSION >= 0x050000
    ui->tableView->horizontalHeader()->setSectionsMovable(true);
#else
    ui->tableView->horizontalHeader()->setMovable(true);
#endif
    connect(ui->sqlLine, SIGNAL(returnPressed()), SLOT(filterFinished()));
    connect(ui->sqlLine, SIGNAL(textChanged()), SLOT(filterChainging()));
    connect(ui->tableSelect, SIGNAL(currentIndexChanged(QString)), SLOT(tableChanged(QString)));
    connect(ui->tableView->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), SLOT(sortIndicatorChanged(int,Qt::SortOrder)));
    connect(ui->menuWindow, SIGNAL(aboutToShow()), SLOT(onWindowMenuShow()));
    connect(ui->menuRecent_Files, SIGNAL(aboutToShow()), SLOT(onRecentFileShow()));
    connect(ui->menuShowHiddenColumn, SIGNAL(aboutToShow()), SLOT(onShowHiddenColumnShow()));
    ui->tableView->horizontalHeader()->installEventFilter(this);
    ui->tableView->verticalHeader()->installEventFilter(this);
    ui->tableView->installEventFilter(this);
    ui->sqlLine->setDatabase(&m_database);

    if (m_databasename.compare(":memory:") == 0 || m_database.driverName() != "QSQLITE") {
        ui->actionView_in_File_Manager->setEnabled(false);
        setWindowTitle(QString("[*] ") + m_databasename);
    } else {
        setWindowTitle(QString("[*] ") + QFileInfo(m_databasename).completeBaseName());
    }

    filterFinished();

#ifdef Q_OS_MACX
#if QT_VERSION >= 0x050000
    nativeToolbar = QtMacExtras::setNativeToolBar(ui->mainToolBar, true);
    nativeToolbar->setIconSize(QSize(32,32));
    nativeToolbar->setToolButtonStyle(static_cast<Qt::ToolButtonStyle>(0));
    ui->menuEdit->addAction(tr("Customize Toolbar"), nativeToolbar, SLOT(showCustomizationSheet()));
#endif
#endif

    if (m_databasename.compare(":memory:") != 0 && m_database.driverName() == "QSQLITE") {
        setWindowFilePath(m_databasename);

        // update recent files
        QStringList recent = tableview_settings->value(RECENT_FILES).toStringList();
        if (recent.contains(m_databasename))
            recent.removeOne(m_databasename);
        while (recent.size() >= RECENT_FILES_MAX)
            recent.removeLast();
        recent.insert(0, m_databasename);
        tableview_settings->setValue(RECENT_FILES, recent);
        tableview_settings->sync();
    } else {
        ui->actionR_code_to_import->setEnabled(false);
    }
}

void MainWindow::setupTableModel()
{
    m_tableModel = new SqlTableModelAlternativeBackground(this, m_database);
    m_tableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    ui->tableView->setModel(m_tableModel);
    connect(m_tableModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(tableUpdated()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_database.driverName() == "QSQLITE" && m_databasename.compare(":memory:") == 0 && m_database.tables().size()) {
        QMessageBox::StandardButton selected =
                SheetMessageBox::warning(this, tr("All changes will be destoried."),
                                         tr("All changes in memory database will NOT be saved. You have to export table to save."),
                                         QMessageBox::Discard|QMessageBox::Cancel, QMessageBox::Cancel);
        switch(selected) {
        case QMessageBox::Discard:
            event->accept();
            cleanupDatabase();
            return;
        default:
            event->ignore();
            return;
        }
    }

    if (!confirmDuty()) {
        event->ignore();
    } else {
        event->accept();
        cleanupDatabase();
    }
}


void MainWindow::cleanupDatabase()
{
    foreach(QDialog *dialog, m_dialogs) {
        if (dialog->isVisible()) {
            dialog->close();
        }
        delete dialog;
    }


    foreach(QWidget *widget, qApp->topLevelWidgets()) {
        MainWindow *mainWindow = dynamic_cast<MainWindow *>(widget);
        if (mainWindow && mainWindow != this && mainWindow->isVisible()) {
            if (mainWindow->database().driverName() == m_database.driverName() &&
                    mainWindow->database().connectionName() == m_database.connectionName()) {
                return; // shared connection is found.
            }
        }
    }

    // shared connection is not found.
    m_database.close(); //qDebug() << "close;";
}

bool MainWindow::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == ui->tableView->horizontalHeader() && ev->type() == QEvent::ContextMenu) {
        QContextMenuEvent *cev = static_cast<QContextMenuEvent *>(ev);
        int logical_index = ui->tableView->horizontalHeader()->logicalIndexAt(cev->pos());
        if (logical_index >= 0) {
            //qDebug() << obj << cev << cev->pos() << logical_index;
            cev->accept();
            QMenu popup(this);
            popup.move(cev->globalPos());
            QAction *name = popup.addAction(m_tableModel->headerData(logical_index, Qt::Horizontal).toString());
            name->setEnabled(false);
            popup.addSeparator();
            QAction *summary = popup.addAction("Summary");
            summary->setData(logical_index);
            connect(summary, SIGNAL(triggered()), SLOT(showColumnSummary()));
            QAction *hideColumn = popup.addAction("Hide");
            hideColumn->setData(logical_index);
            connect(hideColumn, SIGNAL(triggered()), SLOT(hideColumn()));
            if (!m_tableModel->isView()) {
                QAction *createIndex = popup.addAction(tr("Create index"));
                createIndex->setData(logical_index);
                connect(createIndex, SIGNAL(triggered()), SLOT(createIndexForColumn()));
            }
            popup.addSeparator();
            QAction *resizeColumns = popup.addAction(tr("Resize columns to fit contents"));
            connect(resizeColumns, SIGNAL(triggered()), ui->tableView, SLOT(resizeColumnsToContents()));
            popup.exec();
        }
        return true;
    } else if (obj == ui->tableView->verticalHeader() && ev->type() == QEvent::ContextMenu) {
        QContextMenuEvent *cev = static_cast<QContextMenuEvent *>(ev);
        QMenu popup(this);
        popup.move(cev->globalPos());
        QAction *resizeColumns = popup.addAction(tr("Resize rows to fit contents"));
        connect(resizeColumns, SIGNAL(triggered()), ui->tableView, SLOT(resizeRowsToContents()));
        popup.exec();
        return true;
    } else if (obj == ui->tableView && ev->type() == QEvent::ContextMenu) {
        QContextMenuEvent *cev = static_cast<QContextMenuEvent *>(ev);
        QPoint pos = cev->pos();
        pos.rx() -= ui->tableView->verticalHeader()->width();
        pos.ry() -= ui->tableView->horizontalHeader()->height();
        QModelIndex index = ui->tableView->indexAt(pos);
        if (index.isValid()) {
            //qDebug() << "Table View context menu" << index << pos;
            cev->accept();
            QMenu popup(this);
            popup.move(cev->globalPos());
            QAction *showContent = popup.addAction(tr("Copy"), this, SLOT(on_actionCopy_triggered()));
            showContent->setData(index);
            QAction *showCellContent = popup.addAction(tr("Show"), this, SLOT(showCell()));
            showCellContent->setData(index);
            popup.exec();
            return true;
        }
    }
    return false;
}

void MainWindow::onWindowMenuShow()
{
    ui->menuWindow->clear();
    m_windowList.clear();
    m_windowList = QApplication::topLevelWidgets();
    for (int i = 0; i < m_windowList.size(); ++i) {
        if (m_windowList[i]->isVisible() && !m_windowList[i]->windowTitle().isEmpty()) {
            QAction *action = ui->menuWindow->addAction(m_windowList[i]->windowTitle().replace("[*] ", ""));
            action->setCheckable(true);
            if (m_windowList[i]->isActiveWindow())
                action->setChecked(true);
            action->setData(i);
            connect(action, SIGNAL(triggered()), SLOT(activate()));
        }
    }
}

void MainWindow::onRecentFileShow()
{
    QStringList recent = tableview_settings->value(RECENT_FILES).toStringList();

    ui->menuRecent_Files->clear();

    foreach(QString file, recent) {
        QFileInfo info(file);
        QAction *action = ui->menuRecent_Files->addAction(info.completeBaseName());
        action->setData(file);
        action->setToolTip(file);
        connect(action, SIGNAL(triggered()), SLOT(onRecentFileOpen()));
    }

    if (recent.size()) {
        ui->menuRecent_Files->addSeparator();
    }

    QAction *clear = ui->menuRecent_Files->addAction(tr("Clear"));
    connect(clear, SIGNAL(triggered()), SLOT(onClearRecentFiles()));
}

void MainWindow::onRecentFileOpen()
{
    QAction *action = static_cast<QAction *>(sender());
    QFileInfo fileinfo(action->data().toString());
    if (fileinfo.exists()) {
        open(action->data().toString());
    } else {
        SheetMessageBox::critical(0, tr("Cannot open recent file."), tr("%1 is not found.").arg(fileinfo.completeBaseName()));
    }
}

void MainWindow::onClearRecentFiles()
{
    tableview_settings->setValue(RECENT_FILES, QVariant());
    tableview_settings->clear();
}

void MainWindow::onShowHiddenColumnShow()
{
    ui->menuShowHiddenColumn->clear();

    int numLogicalColumns = m_tableModel->columnCount();
    bool isHiddenColumnExists = false;

    for (int logicalIndex = 0; logicalIndex < numLogicalColumns; logicalIndex++) {
        if (ui->tableView->isColumnHidden(logicalIndex)) {
            QString columnName = m_tableModel->headerData(logicalIndex, Qt::Horizontal).toString();

            QAction *showColumn = ui->menuShowHiddenColumn->addAction(columnName);
            showColumn->setData(logicalIndex);
            connect(showColumn, SIGNAL(triggered()), SLOT(showColumn()));

            isHiddenColumnExists = true;
        }
    }

    if (!isHiddenColumnExists) {
        QAction *empty = ui->menuShowHiddenColumn->addAction(tr("No hidden column"));
        empty->setDisabled(true);
    }
}

void MainWindow::showCell()
{
    QAction *action = (QAction *)sender();
    QModelIndex index = action->data().toModelIndex();
    QString header = m_tableModel->headerData(index.column(), Qt::Horizontal).toString();

    QMessageBox *message = SheetMessageBox::makeMessageBox(this, tr("%1, #%2").arg(header, QString::number(index.row()+1)),
                                                           m_tableModel->data(index).toString());
    if(m_tableModel->data(index) != m_tableModel->data(index, Qt::EditRole)) {
        message->setDetailedText(m_tableModel->data(index, Qt::EditRole).toString());
    }
    message->setIcon(QMessageBox::Information);
    message->exec();
    delete message;
}

void MainWindow::activate()
{
    QAction *action = (QAction *)sender();
    m_windowList[action->data().toInt()]->raise();
    m_windowList[action->data().toInt()]->activateWindow();
}

void MainWindow::showColumnSummary()
{
    QAction *sigsender = static_cast<QAction *>(sender());
    int logical_index = sigsender->data().toInt();
    QString column_name = m_tableModel->headerData(logical_index, Qt::Horizontal).toString();

    QSqlQuery query;
    if (ui->sqlLine->toPlainText().isEmpty()) {
        query = m_database.exec(QString("SELECT \"%1\" FROM %2").arg(column_name, m_tableModel->tableName()));
    } else {
        query = m_database.exec(QString("SELECT \"%1\" FROM %2 WHERE %3").arg(column_name, m_tableModel->tableName(), ui->sqlLine->toPlainText()));
    }

    if (query.lastError().type() != QSqlError::NoError) {
        SheetMessageBox::warning(this, tr("Cannot make summary"), m_database.lastError().text()+"\n\n"+query.lastQuery());
        return;
    }

    QList<double> doubleList;
    double sumValue = 0;
    bool ok = true;

    while(query.next()) {
        double value = query.record().value(0).toDouble(&ok);
        if (!ok) {
            SheetMessageBox::critical(this, tr("Column summary error"), tr("This column contains non-number data"));
            return;
        }
        doubleList.append(value);
        sumValue += value;
    }

    SummaryDialog *summary = new SummaryDialog(doubleList, column_name, this);
    summary->show();
    m_dialogs.append(summary);
}

void MainWindow::showColumn()
{
    QAction *sigsender = static_cast<QAction *>(sender());
    int logicalIndex = sigsender->data().toInt();
    ui->tableView->showColumn(logicalIndex);
}

void MainWindow::hideColumn()
{
    QAction *sigsender = static_cast<QAction *>(sender());
    int logicalIndex = sigsender->data().toInt();
    ui->tableView->hideColumn(logicalIndex);
}

void MainWindow::createIndexForColumn()
{
    QAction *sigsender = static_cast<QAction *>(sender());
    int logicalIndex = sigsender->data().toInt();

    QString tableName = m_tableModel->plainTableName();
    QString columnName = m_tableModel->headerData(logicalIndex, Qt::Horizontal).toString();

    QString indexName = QString("%1__%2__index").arg(tableName, columnName);

    QSqlQuery query = m_database.exec(QString("CREATE INDEX %1 ON %2(%3)").arg(indexName, tableName, columnName));
    if (query.lastError().type() != QSqlError::NoError) {
        SheetMessageBox::critical(this, tr("SQL Error"), query.lastError().text()+"\n\n"+query.lastQuery());
    }

    //SheetTextInputDialog dialog(this);
}

bool MainWindow::confirmDuty()
{
    if (m_isDirty) {
        QMessageBox::StandardButton selected =
                SheetMessageBox::question(this, tr("The table is changed."), tr("Do you want to save or discard changes?"),
                                          QMessageBox::Save|QMessageBox::Cancel|QMessageBox::Discard, QMessageBox::Save);
        switch(selected) {
        case QMessageBox::Cancel:
            ui->tableSelect->setCurrentIndex(ui->tableSelect->findText(m_tableModel->plainTableName()));
            return false;
        case QMessageBox::Save:
            if (!m_tableModel->submitAll()) {
                ui->tableSelect->setCurrentIndex(ui->tableSelect->findText(m_tableModel->plainTableName()));
                SheetMessageBox::critical(this, tr("Cannot save table"), m_tableModel->lastError().text());
                return false;
            }
            m_isDirty = false;
            break;
        case QMessageBox::Discard:
        default:
            m_isDirty = false;
            break;
        }
        return true;
    }
    return true;
}

void MainWindow::filterFinished()
{
    if (m_tableModel->plainTableName().isEmpty())
        return;
    m_tableModel->setFilter(ui->sqlLine->toPlainText());
    m_tableModel->select();
    if (m_tableModel->lastError().type() != QSqlError::NoError) {
        SheetMessageBox::warning(this, tr("Cannot apply the filter."), m_tableModel->lastError().text());
        return;
    }

    ui->sqlLine->setStyleSheet("");
    QStringList history = tableview_settings->value(SQL_FILTER_HISTORY).toStringList();
    if (history.contains(ui->sqlLine->toPlainText()))
        history.removeOne(ui->sqlLine->toPlainText());
    if (!ui->sqlLine->toPlainText().isEmpty())
        history.insert(0, ui->sqlLine->toPlainText());
    while (history.size() > SQL_FILTER_HISTORY_MAX) {
        history.removeLast();
    }
    tableview_settings->setValue(SQL_FILTER_HISTORY, history);

    m_rowcountlabel->setText(QString("%1 rows").arg(QString::number(m_tableModel->sqlRowCount())));
}

void MainWindow::filterChainging()
{
    if (m_tableModel->filter().trimmed() == ui->sqlLine->toPlainText().trimmed()) {
        ui->sqlLine->setStyleSheet("");
    } else {
        ui->sqlLine->setStyleSheet("SQLTextEdit{ background: #FAFFC5;}");
    }
}


void MainWindow::tableChanged(const QString &name)
{
    if (name == m_tableModel->plainTableName())
        return;
    if (name.isEmpty())
        return;
    if (!confirmDuty())
        return;

    m_tableModel->setTable(name);
    ui->sqlLine->setTable(name);
    ui->tableView->horizontalHeader()->setSortIndicatorShown(false);
    m_isDirty = false;
    setWindowModified(false);
    m_tableModel->select();
    if (m_tableModel->lastError().type() != QSqlError::NoError) {
        SheetMessageBox::warning(this, tr("Cannot select table."), m_tableModel->lastError().text());
        m_tableModel->clear();
        return;
    }
    ui->actionInsert->setEnabled(m_tableModel->editable());
    ui->actionDelete->setEnabled(m_tableModel->editable());
    ui->actionCommit->setEnabled(m_tableModel->editable());
    ui->actionRevert->setEnabled(m_tableModel->editable());
    setWindowTitle(QString("[*] %1 : %2").arg(m_tableModel->plainTableName(), QFileInfo(m_database.databaseName()).completeBaseName()));
    filterFinished();
}

void MainWindow::tableUpdated()
{
    m_isDirty = true;
    setWindowModified(true);
}

void MainWindow::updateDatabase()
{
    ui->tableSelect->clear();
    foreach(QString name, m_database.tables()) {
        ui->tableSelect->addItem(name);
    }

    foreach(QString name, m_database.tables(QSql::Views)) {
        ui->tableSelect->addItem(name);
    }

    if (m_tableModel->tableName().isEmpty() || !m_database.tables().contains(m_tableModel->tableName())) {
        if (m_database.tables().size())
            tableChanged(m_database.tables()[0]);
        ui->tableSelect->setCurrentIndex(ui->tableSelect->findText(m_tableModel->plainTableName()));
    }
}

void MainWindow::sortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
    if (m_isDirty) {
        SheetMessageBox::warning(this, tr("Data is not commited"), tr("You have to commit changes before sorting."));
        return;
    }
    ui->tableView->horizontalHeader()->setSortIndicatorShown(true);
    m_tableModel->sort(logicalIndex, order);
}

void MainWindow::on_actionGo_github_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/informationsea/Hyokai"));
}

void MainWindow::on_actionCommit_triggered()
{
    if (m_tableModel->submitAll()) {
        m_isDirty = false;
        setWindowModified(false);
        filterFinished();
    } else {
        SheetMessageBox::critical(this, tr("Cannot save table"), m_tableModel->lastError().text());
    }
}

void MainWindow::on_actionRevert_triggered()
{
    m_tableModel->revertAll();
    m_isDirty = false;
    setWindowModified(false);
    filterFinished();
}

void MainWindow::on_actionCreateTable_triggered()
{
    if (!confirmDuty())
        return;

    SchemaDialog dialog(&m_database, 0, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    QStringList sqls = SqlFileImporter::createSql(dialog.name(), dialog.fields(), dialog.useFts4());

    foreach (const QString sql, sqls) {
        m_database.exec(sql);

        if (m_database.lastError().type() != QSqlError::NoError) {
            SheetMessageBox::warning(this, tr("Cannot make table"), m_database.lastError().text()+"\n\n"+sql);
            break;
        }
    }

    updateDatabase();
    filterFinished();
}

void MainWindow::open(QString path)
{
    if (path.isEmpty())
        return;
    ::tableviewCleanupWindows();
    MainWindow *w = new MainWindow(NULL, path);
    w->show();
    ::windowList.append(w);
    if (m_databasename.compare(":memory:") == 0 && m_database.tables().size() == 0) {
        close();
    }
}

void MainWindow::refresh()
{
    updateDatabase();
    filterFinished();
}

void MainWindow::on_actionOpen_triggered()
{
    QString path = QFileDialog::getOpenFileName(NULL, "Open SQLite3 Database or text file",
                                                tableview_settings->value(LAST_SQLITE_DIRECTORY, QDir::homePath()).toString(),
                                                "All (*.sqlite3 *.sqlite *.txt *.csv *.tsv);; SQLite3 (*.sqlite3 *.sqlite);; Text (*.txt);; CSV (*.csv);; Tab delimited (*.tsv);; All (*)");
    if (path.isEmpty())
        return;
    if (path.endsWith(".sqlite3") || path.endsWith(".sqlite")) {
        open(path);
    } else {
        SqlAsynchronousFileImporter *importer = new SqlAsynchronousFileImporter(&m_database, this);
        connect(importer, SIGNAL(finish(QStringList,bool,QString)), SLOT(importFinished(QStringList,bool,QString)));
        importer->executeImport(QStringList(path));
    }
    QFileInfo fileInfo(path);
    tableview_settings->setValue(LAST_SQLITE_DIRECTORY, fileInfo.dir().absolutePath());
    tableview_settings->sync();
}

void MainWindow::on_actionNew_triggered()
{
    QString defaultpath = QFileInfo(tableview_settings->value(LAST_SQLITE_DIRECTORY, QDir::homePath()).toString(), "Untitled.sqlite3").absoluteFilePath();
    QString path = QFileDialog::getSaveFileName(NULL, "New SQLite3 Database",
                                                defaultpath,
                                                "SQLite3 (*.sqlite3);; All (*)");
    if (path.isEmpty())
        return;
    open(path);
    QFileInfo fileInfo(path);
    tableview_settings->setValue(LAST_SQLITE_DIRECTORY, fileInfo.dir().absolutePath());
}

void MainWindow::on_actionInsert_triggered()
{
    if (m_tableModel->plainTableName().isEmpty())
        return;
    if (!m_tableModel->editable())
        return;
    QItemSelectionModel *selection = ui->tableView->selectionModel();
    QList<int> rows;
    foreach (QModelIndex index, selection->selectedIndexes()) {
        if (!rows.contains(index.row()))
            rows << index.row();
    }
    qSort(rows);
    if (rows.isEmpty())
        m_tableModel->insertRow(0);
    else
        m_tableModel->insertRow(rows.last()+1);
    tableUpdated();
}

void MainWindow::on_actionDelete_triggered()
{
    if (m_tableModel->plainTableName().isEmpty())
        return;
    if (!m_tableModel->editable())
        return;
    QItemSelectionModel *selection = ui->tableView->selectionModel();
    QList<int> rows;
    foreach (QModelIndex index, selection->selectedIndexes()) {
        if (!rows.contains(index.row()))
            rows << index.row();
    }
    qSort(rows);
    while(!rows.isEmpty()) {
        m_tableModel->removeRow(rows.takeLast());
    }
    tableUpdated();
}

void MainWindow::on_actionQuit_triggered()
{
    qApp->closeAllWindows();
}

void MainWindow::on_actionImportTable_triggered()
{
    if (!confirmDuty())
        return;

    QStringList import = QFileDialog::getOpenFileNames(this, tr("Select import file"),
                                                  tableview_settings->value(LAST_IMPORT_DIRECTORY, QDir::homePath()).toString(),
                                                  tr("Text (*.txt *.csv *.tsv);; All (*)"));

    if (import.isEmpty())
        return;

    SqlAsynchronousFileImporter *importer = new SqlAsynchronousFileImporter(&m_database, this);
    connect(importer, SIGNAL(finish(QStringList,bool,QString)), SLOT(importFinished(QStringList,bool,QString)));
    importer->executeImport(import);
}

void MainWindow::importFinished(QStringList importedTables, bool withError, QString errorMessage)
{
    qDebug() << "importFinished" << withError << errorMessage;
    if (withError) {
        SheetMessageBox::critical(this, tr("Cannot import file"), errorMessage);
        qDebug() << "Error";
    } else {
        updateDatabase();
        filterFinished();

        if (!importedTables.isEmpty())
            ui->tableSelect->setCurrentIndex(ui->tableSelect->findText(importedTables.last()));
    }
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::on_actionAbout_Table_View_triggered()
{
    QMessageBox about(this);
    about.setWindowTitle(tr("Hyokai"));
    about.setIconPixmap(QPixmap(":rc/images/icon128.png"));
    about.setTextFormat(Qt::RichText);
    about.setText(tr("Hyokai 0.3<br /><br />"
                     "Simple SQLite Viewer<br /><br />"
                     "Copyright (C) 2013 Yasunobu OKAMURA<br /><br />"
                     "Developing on <a href=\"https://github.com/informationsea/Hyokai\">Github</a><hr />"
                     "Some toolbar icons by <a href=\"http://tango.freedesktop.org\">Tango Desktop Project</a><br /><br />"
                     "Build at " __DATE__));
    about.exec();
}

void MainWindow::on_actionRun_Custum_SQL_triggered()
{
    CustomSqlDialog *customSql = new CustomSqlDialog(&m_database, this);
    customSql->show();
    m_dialogs.append(customSql);
}

void MainWindow::on_actionRefresh_triggered()
{
    if (!confirmDuty())
        return;
    updateDatabase();
    filterFinished();
}

void MainWindow::on_actionExport_Table_triggered()
{
    if (m_tableModel->plainTableName().isEmpty()) {
        SheetMessageBox::information(this, tr("No table is selected"), tr("Please select a table to export"));
        return;
    }

    if (!confirmDuty())
        return;

    QSqlQuery query;
    if (ui->sqlLine->toPlainText().isEmpty()) {
        query = m_database.exec(QString("SELECT * FROM %1").arg(m_tableModel->tableName()));
    } else {
        query = m_database.exec(QString("SELECT * FROM %1 WHERE %2").arg(m_tableModel->tableName(), ui->sqlLine->toPlainText()));
    }

    if (query.lastError().type() != QSqlError::NoError) {
        SheetMessageBox::warning(this, tr("Cannot export"), m_database.lastError().text()+"\n\n"+query.lastQuery());
        return;
    }

    QString defaultpath = QFileInfo(tableview_settings->value(LAST_EXPORT_DIRECTORY, QDir::homePath()).toString(),
                                    m_tableModel->plainTableName()+".csv").absoluteFilePath();
    //qDebug() << "defaultpath: " << defaultpath;
    QString outputpath = QFileDialog::getSaveFileName(this, tr("Export as text"),
                                                      defaultpath,
                                                      "CSV (*.csv);; Tab separated (*.txt)");
    if (outputpath.isEmpty())
        return;
    QFileInfo outputfileinfo(outputpath);
    tableview_settings->setValue(LAST_EXPORT_DIRECTORY, outputfileinfo.dir().absolutePath());

    SqlFileExporter exporter(&m_database, this);
    if (!exporter.exportTable(query, outputpath, outputpath.endsWith(".csv"))) {
        SheetMessageBox::critical(this, tr("Cannot export table"), exporter.errorMessage());
    }
}

void MainWindow::on_actionOpen_In_Memory_Database_triggered()
{
    foreach (MainWindow *window, windowList) {
        if (window->isVisible() && window->databaseName() == ":memory:") {
            window->activateWindow();
            return;
        }
    }
    open(":memory:");
}

void MainWindow::on_buttonClear_clicked()
{
    ui->sqlLine->setPlainText("");
    filterFinished();
}

void MainWindow::on_actionCopy_triggered()
{
    onCopyTriggered(false);
}


void MainWindow::on_actionCopy_with_header_triggered()
{
    onCopyTriggered(true);
}

void MainWindow::onCopyTriggered(bool withHeader)
{
    QTableView *tableView;
    QWidget *widget = qApp->activeWindow();

    tableView = ui->tableView;
    foreach(QDialog *dialog, m_dialogs) {
        if (widget == dialog) {
            CustomSqlDialog *customDialog = dynamic_cast<CustomSqlDialog *>(dialog);
            if (customDialog)
                tableView = customDialog->tableView();
            break;
        }
    }

    SqlService::copyFromTableView(tableView, withHeader);
}

void MainWindow::on_actionAttach_Database_triggered()
{
    AttachDatabaseDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    QSqlQuery query = m_database.exec(dialog.sql());

    if (query.lastError().type() != QSqlError::NoError) {
        SheetMessageBox::critical(this, tr("Cannot attach"), m_database.lastError().text()+"\n\n"+query.lastQuery());
        return;
    }
    //SheetMessageBox::information(this, tr("Database is attached."), tr("Database is attached successfully."));
}

void MainWindow::on_actionView_in_File_Manager_triggered()
{
    if (m_databasename == ":memory:")
        return;
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(m_databasename).dir().absolutePath()));
}

void MainWindow::on_actionPreference_triggered()
{
    if (preferenceDialog && preferenceDialog->isVisible()) {
        preferenceDialog->activateWindow();
    } else {
        preferenceDialog = new PreferenceWindow();
        preferenceDialog->show();
    }
}

void MainWindow::on_actionR_code_to_import_triggered()
{
    if (m_databasename == ":memory:")
        return;
    QString str = SqlService::createRcodeToImportWithTable(m_database, m_tableModel->plainTableName(), ui->sqlLine->toPlainText());
    QClipboard *clip = QApplication::clipboard();
    clip->setText(str);
}

void MainWindow::on_buttonAssist_clicked()
{
    m_assistPopup.clear();

    QMenu *historyMenu = m_assistPopup.addMenu(tr("History"));
    QStringList history = tableview_settings->value(SQL_FILTER_HISTORY).toStringList();
    foreach (QString oneHistory, history) {
        QString showHistory = oneHistory;
        if (oneHistory.length() > 150) {
            showHistory = QString("%1 .. %2").arg(oneHistory.left(75), oneHistory.right(75));
        }
        QAction *action = historyMenu->addAction(showHistory);
        action->setData(oneHistory);
        connect(action, SIGNAL(triggered()), SLOT(insertSqlFilter()));
    }
    if (history.isEmpty()) {
        historyMenu->setEnabled(false);
    } else {
        historyMenu->addSeparator();
        QAction *action = historyMenu->addAction(tr("Clear"));
        action->setData("<CLEAR>");
        connect(action, SIGNAL(triggered()), SLOT(insertSqlFilter()));
    }

    m_assistPopup.addSeparator();
    QMenu *columnMenu = m_assistPopup.addMenu(tr("Columns"));
    QSqlRecord records = m_database.record(m_tableModel->tableName());
    for (int i = 0; i < records.count(); ++i) {
        QAction *action = columnMenu->addAction(records.fieldName(i));
        action->setData(records.fieldName(i));
        connect(action, SIGNAL(triggered()), SLOT(insertSqlFilter()));
    }
    if (records.count() == 0) {
        columnMenu->setEnabled(false);
    }

    QMenu *tableMenu = m_assistPopup.addMenu(tr("Tables"));
    QStringList tableList = m_database.tables(QSql::AllTables);
    foreach(QString one, tableList) {
        QAction *action = tableMenu->addAction(one);
        action->setData(one);
        connect(action, SIGNAL(triggered()), SLOT(insertSqlFilter()));
    }

    m_assistPopup.addSeparator();
    QStringList operators;
    operators << "==" << ">=" << "<=" << ">" << "<" << "AND" << "OR" << "LIKE \"%\"";
    foreach (QString s, operators){
        QAction* action = m_assistPopup.addAction(s);
        action->setData(s);
        connect(action, SIGNAL(triggered()), SLOT(insertSqlFilter()));
    }

    QPoint point = ui->buttonAssist->pos();
    point.setY(point.y() + ui->buttonAssist->height());
    m_assistPopup.popup(mapToGlobal(point));
}

void MainWindow::insertSqlFilter()
{
    QAction *senderAction = (QAction *)sender();
    QString add = senderAction->data().toString();
    if (add.compare("<CLEAR>") == 0) {
        tableview_settings->setValue(SQL_FILTER_HISTORY, QVariant());
        return;
    }
    ui->sqlLine->insertPlainText(add);
}

void MainWindow::on_actionGo_to_SQLite3_webpage_triggered()
{
    QDesktopServices::openUrl(QUrl("http://www.sqlite.org"));
}

void MainWindow::on_actionDrop_Table_triggered()
{
    if (m_tableModel->tableName().isEmpty())
        return;
    int ret = SheetMessageBox::question(this, tr("Drop table or view"),
                                        tr("Are you sure to drop selected table or view?\nThis operation cannot be undone."),
                                        QMessageBox::Yes|QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret != QMessageBox::Yes)
        return;

    QString tableName = m_tableModel->tableName();
    bool isView = m_tableModel->isView();
    m_tableModel->setTable("");
    ui->tableView->setModel(0);
    delete m_tableModel;

    if (isView)
        m_database.exec(QString("DROP VIEW %1").arg(tableName));
    else
        m_database.exec(QString("DROP TABLE %1").arg(tableName));

    setupTableModel();

    if (m_database.lastError().type() != QSqlError::NoError) {
        SheetMessageBox::warning(this, tr("Cannot drop table or view"), m_database.lastError().text());
        return;
    }

    refresh();
}

void MainWindow::on_actionSelect_All_triggered()
{
    QWidget* widget = qApp->activeWindow();
    foreach(QDialog *dialog, m_dialogs) {
        if (widget == dialog) {
            CustomSqlDialog *customDialog = dynamic_cast<CustomSqlDialog *>(dialog);
            if (customDialog)
                customDialog->selectTableAll();
            return;
        }
    }

    int count = 0;
    ui->tableView->scrollToBottom();
    while (m_tableModel->sqlRowCount() != m_tableModel->rowCount()) {
        if (count == 10) {
            if (SheetMessageBox::warning(this, tr("Select All"), tr("This operation tooks long time. Do you want to continue?"), QMessageBox::Yes|QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {
                return;
            }
        }
        ui->tableView->scrollToBottom();
        count += 1;
    }
    ui->tableView->selectAll();
}

void MainWindow::on_actionConnect_to_database_triggered()
{
    DatabaseConnectionDialog *dialog = new DatabaseConnectionDialog();
    dialog->show();
}

void MainWindow::on_actionDuplicate_connection_triggered()
{
    ::tableviewCleanupWindows();
    MainWindow *w = new MainWindow(m_database);
    w->show();
    windowList.append(w);
}

void MainWindow::on_actionDatabase_Information_triggered()
{
    SheetMessageBox::information(this, tr("Database Information"),
                                 QString("Database type: %1\n"
                                         "Host name: %2\n"
                                         "Port: %3\n"
                                         "User name: %4\n"
                                         "Database name: %5\n"
                                         "Connection name: %6").arg(m_database.driverName(),
                                                                    m_database.hostName(),
                                                                    QString::number(m_database.port()),
                                                                    m_database.userName(),
                                                                    m_database.databaseName(),
                                                                    m_database.connectionName()));
}

void MainWindow::on_actionDuplicate_Table_triggered()
{
    QString tableName = m_tableModel->plainTableName();
    if (tableName.isEmpty())
        return;
    QString where = ui->sqlLine->toPlainText();
    QMessageBox::StandardButton ret =
            SheetMessageBox::question(this, tr("Table duplication"), tr("Do you want to tweek table scheme?"),
                                      QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
    QString newTableName;

    if (ret == QMessageBox::Yes) {
        QList<SchemaField> fields;
        QStringList orignalFieldNames;
        QSqlRecord record = m_database.record(tableName);
        for (int i = 0; i < record.count(); ++i) {
            QSqlField field = record.field(i);
            SchemaField newfield(field.name());
            switch(field.type()) {
            case QVariant::Int:
                newfield.setFieldType("INTEGER");
                break;
            case QVariant::Double:
                newfield.setFieldType("REAL");
                break;
            case QVariant::ByteArray:
                newfield.setFieldType("BOLB");
                break;
            default:
                qDebug() << field;
            case QVariant::String:
                newfield.setFieldType("TEXT");
                break;
            }

            fields.append(newfield);
            orignalFieldNames << field.name();
            //qDebug() << field;
        }

        m_database.transaction();

        SchemaDialog dialog(&m_database, NULL, this);
        dialog.setFields(fields);
        dialog.setName(SqlService::suggestTableName(tableName, &m_database));

        dialog.exec();

        foreach(QString sql, SqlFileImporter::createSql(dialog.name(), dialog.fields(), dialog.useFts4())) {
            QSqlQuery query = m_database.exec(sql);
            if (query.lastError().type() != QSqlError::NoError) {
                SheetMessageBox::critical(this, tr("Cannot create table or index"), query.lastError().text());
                m_database.rollback();
                return;
            }
        }

        QString commonFieldNames;
        foreach(SchemaField field, dialog.fields()) {
            if (orignalFieldNames.contains(field.name())) {
                if (!commonFieldNames.isEmpty())
                    commonFieldNames += ", ";
                commonFieldNames += field.name();
            }
        }

        QString whereStatement;
        if (!where.isEmpty()) {
            whereStatement = " WHERE " + where;
        }

        QSqlQuery query = m_database.exec(QString("INSERT INTO %1 SELECT %2 FROM %3 %4").arg(dialog.name(), commonFieldNames, m_tableModel->plainTableName(), whereStatement));
        if (query.lastError().type() != QSqlError::NoError) {
            SheetMessageBox::critical(this, tr("Cannot copy data"), query.lastError().text());
            m_database.rollback();
            return;
        }
        newTableName = dialog.name();
        m_database.commit();
    } else if (ret == QMessageBox::No) {
        newTableName = SheetTextInputDialog::textInput(tr("New table name"), tr("Please input new table name"), this, SqlService::suggestTableName(tableName, &m_database));
        if (newTableName.isEmpty())
            return;

        if (!SqlService::isVaildTableName(newTableName)) {
            SheetMessageBox::critical(this, tr("Invaild table name"), tr("The table name is not vaild."));
            return;
        }

        QString whereStatement;
        if (!where.isEmpty()) {
            whereStatement = " WHERE " + where;
        }

        QSqlQuery query = m_database.exec(QString("CREATE TABLE %1 AS SELECT * FROM %2 %3").arg(newTableName, m_tableModel->plainTableName(), whereStatement));
        if (query.lastError().type() != QSqlError::NoError) {
            SheetMessageBox::critical(this, tr("Cannot copy data"), query.lastError().text());
            return;
        }
    }

    refresh();
    ui->tableSelect->setCurrentIndex(ui->tableSelect->findText(newTableName));
    tableChanged(newTableName);
}

void MainWindow::on_actionPlot_triggered()
{
    SqlPlotChart *plotChart = new SqlPlotChart(&m_database, this, m_tableModel->plainTableName());
    plotChart->setFilter(ui->sqlLine->toPlainText());
    plotChart->show();
    m_dialogs.append(plotChart);
}



void MainWindow::on_actionClose_triggered()
{
    foreach (QDialog *widget, m_dialogs) {
        if (widget->isActiveWindow()) {
            widget->close();
            return;
        }
    }
    // no active dialog is found.
    close();
}
