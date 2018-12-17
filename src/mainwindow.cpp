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
#include "databaseconnectiondialog.h"
#include "sqlservice.h"
#include "sqlplotchart.h"
#include "sqlfileimporter.h"
#include "sqlfileexporter.h"
#include "summarydialog2.h"

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
#include <QValidator>
#include <QIntValidator>
#include <QFont>
#include <QMimeData>
#include <QScrollBar>

#include <tablereader.hpp>
#include <csvreader.hpp>


#define RECENT_FILES "RECENT_FILES"
#define RECENT_FILES_MAX 10

#define CHANGE_SIZE_COLUMN 1
#define CHANGE_SIZE_ROW    2

#define TABLE_LIST_VISIBLE "TABLE_LIST_VISIBLE"
#define COLUMN_LIST_VISIBLE "COLUMN_LIST_VISIBLE"

static int open_count = 0;

MainWindow::MainWindow(QWidget *parent, QString path) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), m_isDirty(false), m_isClosing(false), m_splitColumn(0)
{
    m_databasename = path;

    open_count++;
    m_database = QSqlDatabase::addDatabase("QSQLITE", QString::number(open_count));
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
                SheetMessageBox::critical(nullptr, tr("Cannot attach"), m_database.lastError().text()+"\n\n"+q.lastQuery());
            }
        }
    }
}

MainWindow::MainWindow(const QSqlDatabase &database, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), m_isDirty(false), m_isClosing(false)
{
    m_databasename = database.databaseName();
    m_database = database;
    initialize();
}

void MainWindow::initialize()
{
    ui->setupUi(this);
    m_tableViewItemDelegate1 = new TableViewStyledItemDelegate(ui->tableView);
    ui->tableView->setItemDelegate(m_tableViewItemDelegate1);
    m_tableViewItemDelegate2 = new TableViewStyledItemDelegate(ui->tableView_2);
    ui->tableView_2->setItemDelegate(m_tableViewItemDelegate2);
    //m_rowcountlabel = new QLabel(ui->statusBar);
    m_rowcountlabel = ui->statusLabel;
    //ui->statusBar->addWidget(sqlLineCount);
    //ui->statusBar->addPermanentWidget(m_rowcountlabel);

    move(nextWindowPosition());

    ui->tabView->setDocumentMode(true);
    ui->tabView->setUsesScrollButtons(true);
    ui->tabView->setShape(QTabBar::RoundedSouth);
    connect(ui->tabView, SIGNAL(currentChanged(int)), SLOT(tableTabChanged(int)));

    ui->actionAbout_Qt->setMenuRole(QAction::AboutQtRole);
    ui->actionAbout_Table_View->setMenuRole(QAction::AboutRole);
    ui->actionQuit->setMenuRole(QAction::QuitRole);
    ui->actionPreference->setMenuRole(QAction::PreferencesRole);

    ui->mainToolBar->setIconSize(QSize(32, 32));

    setupTableModel();
    updateDatabase();

    ui->tableView->horizontalHeader()->setSectionsMovable(true);
    ui->tableView_2->horizontalHeader()->setSectionsMovable(true);

#ifdef Q_OS_MACX
    ui->menuWindow->setAsDockMenu();
#endif

    connect(ui->sqlLine, SIGNAL(returnPressed()), SLOT(filterFinished()));
    connect(ui->sqlLine, SIGNAL(textChanged()), SLOT(filterChainging()));
    connect(ui->tableSelect, SIGNAL(currentIndexChanged(QString)), SLOT(tableChanged(QString)));
    connect(ui->tableListView, SIGNAL(currentTextChanged(QString)), SLOT(tableChanged(QString)));
    connect(ui->tableView->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), SLOT(sortIndicatorChanged(int,Qt::SortOrder)));
    connect(ui->tableView_2->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), SLOT(sortIndicatorChanged(int,Qt::SortOrder)));
    connect(ui->tableView->verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(onTableViewScrollMoved(int)));
    connect(ui->tableView_2->verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(onTableViewScrollMoved(int)));
    connect(ui->menuWindow, SIGNAL(aboutToShow()), SLOT(onWindowMenuShow()));
    connect(ui->menuRecent_Files, SIGNAL(aboutToShow()), SLOT(onRecentFileShow()));
    connect(ui->menuShowHiddenColumn, SIGNAL(aboutToShow()), SLOT(onShowHiddenColumnShow()));
    connect(ui->mainToolBar, SIGNAL(visibilityChanged(bool)), SLOT(onToolbarVisibiltyChanged()));

    ui->tableListWidget->setVisible(tableview_settings->value(TABLE_LIST_VISIBLE).toBool());
    ui->columnListWidget->setVisible(tableview_settings->value(COLUMN_LIST_VISIBLE).toBool());
    ui->columnListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->columnListSearchEdit->setVisible(false);
    ui->columnListSearchClear->setVisible(false);

    ui->tableView->horizontalHeader()->installEventFilter(this);
    ui->tableView->verticalHeader()->installEventFilter(this);
    ui->tableView->installEventFilter(this);
    ui->tableView_2->horizontalHeader()->installEventFilter(this);
    ui->tableView_2->verticalHeader()->installEventFilter(this);
    ui->tableView_2->installEventFilter(this);
    QList<int> splitterSizes;
    splitterSizes << 0 << 1;
    ui->splitter->setSizes(splitterSizes);
    ui->splitter->setStretchFactor(0, 0);
    ui->splitter->setStretchFactor(1, 1);
    ui->sqlLine->setDatabase(&m_database);

    if (m_databasename.compare(":memory:") == 0 || m_database.driverName() != "QSQLITE") {
        ui->actionView_in_File_Manager->setEnabled(false);
        setWindowTitle(QString("[*] ") + m_databasename);
    } else {
        setWindowTitle(QString("[*] ") + QFileInfo(m_databasename).completeBaseName());
    }

    filterFinished();

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

    m_historyHelper = new SqlHistoryHelper(&m_database, this);

    setAcceptDrops(true);
    move(nextWindowPosition());
}

void MainWindow::setupTableModel()
{
    m_tableModel = new SqlTableModelAlternativeBackground(this, m_database);
    m_tableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    ui->tableView->setModel(m_tableModel);
    ui->tableView_2->setModel(m_tableModel);
    connect(m_tableModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(tableUpdated()));
}

MainWindow::~MainWindow()
{
    delete m_historyHelper;
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
            m_isClosing = true;
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
        m_isClosing = true;
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
    QList<QTableView *> tableViews;
    tableViews << ui->tableView << ui->tableView_2;

    foreach(QTableView* oneTableView, tableViews) {
        if (obj == oneTableView->horizontalHeader() && ev->type() == QEvent::ContextMenu) {
            QContextMenuEvent *cev = static_cast<QContextMenuEvent *>(ev);
            int logical_index = oneTableView->horizontalHeader()->logicalIndexAt(cev->pos());
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
                QAction *copyColumnName = popup.addAction("Copy column name");
                copyColumnName->setData(logical_index);
                connect(copyColumnName, SIGNAL(triggered()), SLOT(copyColumnName()));
                QAction *hideColumn = popup.addAction("Hide");
                hideColumn->setData(logical_index);
                connect(hideColumn, SIGNAL(triggered()), SLOT(hideColumn()));
                if (!m_tableModel->isView()) {
                    QAction *createIndex = popup.addAction(tr("Create index"));
                    createIndex->setData(logical_index);
                    connect(createIndex, SIGNAL(triggered()), SLOT(createIndexForColumn()));
                }
                popup.addSeparator();
                QAction *fitColumns = popup.addAction(tr("Resize columns to fit contents"));
                connect(fitColumns, SIGNAL(triggered()), oneTableView, SLOT(resizeColumnsToContents()));
                QAction *resizeColumns = popup.addAction(tr("Resize columns"));
                resizeColumns->setData(CHANGE_SIZE_COLUMN);
                connect(resizeColumns, SIGNAL(triggered()), SLOT(onChangeColumnOrRowSize()));
                popup.addSeparator();
                QAction *setNumDecimalPlaces = popup.addAction(tr("Set number of decimal places"));
                setNumDecimalPlaces->setData(logical_index);
                connect(setNumDecimalPlaces, SIGNAL(triggered()), SLOT(setNumDecimalPlaces()));
                QAction *resetNumDecimalPlaces = popup.addAction(tr("Reset number of decimal places"));
                resetNumDecimalPlaces->setEnabled(m_tableViewItemDelegate1->numDecimalPlaces(logical_index) != TableViewStyledItemDelegate::NUM_DECIMAL_PLACES_NOT_SET);
                resetNumDecimalPlaces->setData(logical_index);
                connect(resetNumDecimalPlaces, SIGNAL(triggered()), SLOT(resetNumDecimalPlaces()));
                popup.exec();
            }
            return true;
        } else if (obj == oneTableView->verticalHeader() && ev->type() == QEvent::ContextMenu) {
            QContextMenuEvent *cev = static_cast<QContextMenuEvent *>(ev);
            QMenu popup(this);
            popup.move(cev->globalPos());
            QAction *fitColumns = popup.addAction(tr("Resize rows to fit contents"));
            connect(fitColumns, SIGNAL(triggered()), oneTableView, SLOT(resizeRowsToContents()));
            QAction *resizeColumns = popup.addAction(tr("Resize rows"));
            resizeColumns->setData(CHANGE_SIZE_ROW);
            connect(resizeColumns, SIGNAL(triggered()), SLOT(onChangeColumnOrRowSize()));
            popup.exec();
            return true;
        } else if (obj == oneTableView && ev->type() == QEvent::ContextMenu) {
            QContextMenuEvent *cev = static_cast<QContextMenuEvent *>(ev);
            QPoint pos = cev->pos();
            pos.rx() -= oneTableView->verticalHeader()->width();
            pos.ry() -= oneTableView->horizontalHeader()->height();
            QModelIndex index = oneTableView->indexAt(pos);
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
    }
    return false;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list")) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    QStringList fileList;
    foreach (QUrl oneUrl, urls) {
        if (oneUrl.isLocalFile()) {
            fileList.append(oneUrl.toLocalFile());
        }
    }

    if (fileList.isEmpty())
        return;

    SqlAsynchronousFileImporter *importer = new SqlAsynchronousFileImporter(&m_database, this);
    connect(importer, SIGNAL(finish(QStringList,bool,QString)), SLOT(importFinished(QStringList,bool,QString)));
    importer->executeImport(fileList);
    event->accept();
}

void MainWindow::onChangeColumnOrRowSize()
{
    QAction *action = static_cast<QAction *>(sender());
    int type = action->data().toInt();
    int defaultsize = 100;
    QString title = tr("Change size of columns");
    if (type == CHANGE_SIZE_ROW) {
        defaultsize = 30;
        title = tr("Change size of rows");
    }

    QIntValidator intValidator;
    intValidator.setRange(0, 1000);
    QString result = SheetTextInputDialog::textInput(title, "", this, QString::number(defaultsize), false, &intValidator);
    if (result.isEmpty())
        return;

    int resultSize = result.toInt();
    if (type == CHANGE_SIZE_ROW) {
        int rows = m_tableModel->rowCount();
        for (int i = 0; i < rows; i++) {
            ui->tableView->setRowHeight(i, resultSize);
            ui->tableView_2->setRowHeight(i, resultSize);
        }
    } else {
        int columns = m_tableModel->columnCount();
        for (int i = 0; i < columns; i++) {
            ui->tableView->setColumnWidth(i, resultSize);
            ui->tableView_2->setColumnWidth(i, resultSize);
        }
    }
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
        SheetMessageBox::critical(nullptr, tr("Cannot open recent file."), tr("%1 is not found.").arg(fileinfo.completeBaseName()));
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
    } else {
        ui->menuShowHiddenColumn->addSeparator();
        QAction *showAll = ui->menuShowHiddenColumn->addAction(tr("Show all"));
        showAll->setData(-1);
        connect(showAll, SIGNAL(triggered()), SLOT(showColumn()));
    }
}

void MainWindow::showCell()
{
    QAction *action = static_cast<QAction *>(sender());
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
    QAction *action = static_cast<QAction *>(sender());
    m_windowList[action->data().toInt()]->raise();
    m_windowList[action->data().toInt()]->activateWindow();
}

void MainWindow::showColumnSummary()
{
    QAction *sigsender = static_cast<QAction *>(sender());
    int logical_index = sigsender->data().toInt();
    QString column_name = m_tableModel->headerData(logical_index, Qt::Horizontal).toString();

	SummaryDialog2 *sumamry2 = new SummaryDialog2(&m_database, m_tableModel->tableName(), column_name, ui->sqlLine->toPlainText(), this);
	sumamry2->show();
}

void MainWindow::showColumn()
{
    QAction *sigsender = static_cast<QAction *>(sender());
    int logicalIndex = sigsender->data().toInt();
    if (logicalIndex >= 0) {
        ui->tableView->showColumn(logicalIndex);
        ui->tableView_2->showColumn(logicalIndex);
    } else {
        for (int i = 0; i < m_tableModel->columnCount(); i++) {
            ui->tableView->showColumn(i);
            ui->tableView_2->showColumn(i);
        }
    }
}

void MainWindow::hideColumn()
{
    QAction *sigsender = static_cast<QAction *>(sender());
    int logicalIndex = sigsender->data().toInt();
    ui->tableView->hideColumn(logicalIndex);
    ui->tableView_2->hideColumn(logicalIndex);
}

void MainWindow::copyColumnName()
{
    QAction *sigsender = static_cast<QAction *>(sender());
    int logicalIndex = sigsender->data().toInt();
    QClipboard *clip = QGuiApplication::clipboard();
    clip->setText(m_tableModel->headerData(logicalIndex, Qt::Horizontal).toString());
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

    ui->sqlLine->setStyleSheet("SQLTextEdit{ background: white; color: black;}");
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

    if (!ui->sqlLine->toPlainText().isEmpty()) {
        m_historyHelper->insertFilterHistory(m_tableModel->plainTableName(), m_tableModel->filter());
    }
}

void MainWindow::filterChainging()
{
    if (m_tableModel->filter().trimmed() == ui->sqlLine->toPlainText().trimmed()) {
        ui->sqlLine->setStyleSheet("SQLTextEdit{ background: white; color: black;}");
    } else {
        ui->sqlLine->setStyleSheet("SQLTextEdit{ background: #FAFFC5; color: black;}");
    }
}

void MainWindow::tableTabChanged(int index)
{
    tableChanged(ui->tabView->tabText(index));
}


void MainWindow::tableChanged(const QString &name)
{
    if (name == m_tableModel->plainTableName())
        return;
    if (name.isEmpty())
        return;
    if (!confirmDuty())
        return;


    int index = ui->tableSelect->findText(name);
    ui->tableSelect->setCurrentIndex(index);
    ui->tabView->setCurrentIndex(index);
    ui->tableListView->setCurrentRow(index);

    ui->sqlLine->clear();

    m_tableModel->setTable(name);
    ui->sqlLine->setTable(name);
    ui->tableView->horizontalHeader()->setSortIndicatorShown(false);
    ui->tableView_2->horizontalHeader()->setSortIndicatorShown(false);
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

    // reset column size
    int columns = m_tableModel->columnCount();
    for (int i = 0; i < columns; i++) {
        ui->tableView->setColumnWidth(i, 100);
        ui->tableView_2->setColumnWidth(i, 100);
    }

    int rows = m_tableModel->rowCount();
    for (int i = 0; i < rows; i++) {
        ui->tableView->setRowHeight(i, 30);
        ui->tableView->setRowHeight(i, 30);
    }

    // reset column list
    ui->columnListView->clear();
    for (int i = 0; i < columns; ++i) {
        ui->columnListView->addItem(m_tableModel->headerData(i, Qt::Horizontal).toString());
    }

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
    ui->tableListView->clear();
    while (ui->tabView->count())
        ui->tabView->removeTab(0);

    QStringList list = m_database.tables();
    list.append(m_database.tables(QSql::Views));
    list.removeOne("Hyokai_SQL_History");
    list.removeOne("sqlite_sequence");
    qSort(list);

    int index = 0;
    foreach(QString name, list) {
        ui->tableSelect->addItem(name);
        ui->tabView->addTab(name);
        ui->tabView->setTabToolTip(index, name);
        ui->tableListView->addItem(name);
        index += 1;
    }

    if (m_tableModel->tableName().isEmpty() || !list.contains(m_tableModel->tableName())) {
        if (list.size())
            tableChanged(list[0]);

        int index = ui->tableSelect->findText(m_tableModel->plainTableName());
        ui->tableSelect->setCurrentIndex(index);
        ui->tabView->setCurrentIndex(index);
        ui->tableListView->setCurrentRow(index);
    }
}

void MainWindow::sortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
    if (m_isDirty) {
        SheetMessageBox::warning(this, tr("Data is not commited"), tr("You have to commit changes before sorting."));
        return;
    }
    ui->tableView->horizontalHeader()->setSortIndicatorShown(true);
    ui->tableView->horizontalHeader()->setSortIndicator(logicalIndex, order);
    ui->tableView_2->horizontalHeader()->setSortIndicatorShown(true);
    ui->tableView_2->horizontalHeader()->setSortIndicator(logicalIndex, order);
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

    SchemaDialog dialog(&m_database, nullptr, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    QStringList sqls;
    sqls << SqlFileImporter::generateCreateTableSql(dialog.name(), dialog.fields(), dialog.useFts4());
    sqls << SqlFileImporter::generateCreateIndexSql(dialog.name(), dialog.fields());

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
    MainWindow *w = new MainWindow(nullptr, path);
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
    QString path = QFileDialog::getOpenFileName(nullptr, "Open SQLite3 Database or text file",
                                                tableview_settings->value(LAST_SQLITE_DIRECTORY, QDir::homePath()).toString(),

                                                "All (*.sqlite3 *.sqlite *.db *.txt *.csv *.tsv) ;; SQLite3 (*.sqlite3 *.sqlite *.db);; Text (*.txt);; CSV (*.csv);; Tab delimited (*.tsv);; Gene Annotations (*.bed *.gff *.gtf);; All (*)");

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
    QString path = QFileDialog::getSaveFileName(nullptr, "New SQLite3 Database",
                                                defaultpath,
                                                "SQLite3 (*.sqlite3 *.sqlite *.db);; All (*)");
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
    QItemSelectionModel *selection = ui->tableView_2->hasFocus() ? ui->tableView_2->selectionModel() : ui->tableView->selectionModel();
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
    QItemSelectionModel *selection = ui->tableView_2->hasFocus() ? ui->tableView_2->selectionModel() : ui->tableView->selectionModel();
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
                                                  tr("All (*.txt *.csv *.tsv *.txt.gz *.csv.gz *.tsv.gz *.bed *.gff *.gtf);; Gene Annotations (*.bed *.gff *.gtf *.bed.gz *.gff.gz *.gtf.gz);; All (*)"));

    if (import.isEmpty())
        return;

    tableview_settings->setValue(LAST_IMPORT_DIRECTORY, QFileInfo(import[0]).absoluteDir().absolutePath());

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
    QString messageDrivers;
    foreach(QString one, QSqlDatabase::drivers()) {
        if (!messageDrivers.isEmpty()) messageDrivers += ", ";
        messageDrivers += one;
    }

    QMessageBox about(this);
    about.setWindowTitle(tr("Hyokai"));
    about.setIconPixmap(QPixmap(":rc/images/icon128.png"));
    about.setTextFormat(Qt::RichText);
    about.setText(tr("Hyokai " HYOKAI_VERSION
                 #if defined(__amd64__) || defined(_M_AMD64)
                     " (64 bit)<br /><br />"
                 #else
                     " (32 bit)<br /><br />"
                 #endif
                     "Simple SQLite Viewer<br /><br />"
                     "Copyright (C) 2014-2018 Yasunobu OKAMURA<br /><br />"
                     "Developing on <a href=\"https://github.com/informationsea/Hyokai\">Github</a><hr />"
                     "Some toolbar icons by <a href=\"http://tango.freedesktop.org\">Tango Desktop Project</a><br /><br />"
                     "Build at " __DATE__ " " __TIME__ "<br />"
                     "Supported Databases: ") + messageDrivers);
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
    if (!exporter.exportTable(query, outputpath, FileTypeUtil::getFileTypeFromPath(outputpath))) {
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
    if (ui->tableView_2->hasFocus()) {
        tableView = ui->tableView_2;
    }
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


    QMenu *historyMenuForTable = m_assistPopup.addMenu(tr("History"));
    QStringList historyForTable = m_historyHelper->filterHistory(m_tableModel->plainTableName());
    foreach (QString oneHistory, historyForTable) {
        QString showHistory = oneHistory;
        if (oneHistory.length() > 150) {
            showHistory = QString("%1 .. %2").arg(oneHistory.left(75), oneHistory.right(75));
        }
        QAction *action = historyMenuForTable->addAction(showHistory);
        action->setData(oneHistory);
        connect(action, SIGNAL(triggered()), SLOT(replaceSqlFilter()));
    }
    if (historyMenuForTable->isEmpty())
        historyMenuForTable->setEnabled(false);

    QMenu *historyMenu = m_assistPopup.addMenu(tr("All History"));
    QStringList history = tableview_settings->value(SQL_FILTER_HISTORY).toStringList();
    foreach (QString oneHistory, history) {
        QString showHistory = oneHistory;
        if (oneHistory.length() > 150) {
            showHistory = QString("%1 .. %2").arg(oneHistory.left(75), oneHistory.right(75));
        }
        QAction *action = historyMenu->addAction(showHistory);
        action->setData(oneHistory);
        connect(action, SIGNAL(triggered()), SLOT(replaceSqlFilter()));
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

    QPoint p = ui->buttonAssist->mapToGlobal(QPoint(0, 0));
    p += QPoint(0, ui->buttonAssist->height());
    m_assistPopup.popup(p);
}

void MainWindow::insertSqlFilter()
{
    QAction *senderAction = static_cast<QAction *>(sender());
    QString add = senderAction->data().toString();
    if (add.compare("<CLEAR>") == 0) {
        tableview_settings->setValue(SQL_FILTER_HISTORY, QVariant());
        return;
    }
    ui->sqlLine->insertPlainText(add);
}

void MainWindow::replaceSqlFilter()
{
    QAction *senderAction = static_cast<QAction *>(sender());
    QString add = senderAction->data().toString();
    if (add.compare("<CLEAR>") == 0) {
        tableview_settings->setValue(SQL_FILTER_HISTORY, QVariant());
        return;
    }
    ui->sqlLine->document()->setPlainText(add);
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
    ui->tableView->setModel(nullptr);
    ui->tableView_2->setModel(nullptr);
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
    while (m_tableModel->canFetchMore()) {
        if (count == 100) {
            if (SheetMessageBox::warning(this, tr("Select All"), tr("This operation tooks long time. Do you want to continue?"), QMessageBox::Yes|QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {
                return;
            }
            count = 0;
        }
        m_tableModel->fetchMore();
        count += 1;
    }
    ui->tableView->selectAll();
    ui->tableView_2->selectAll();
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
                qDebug() << "Unknown field type " << field;
                newfield.setFieldType("TEXT");
                break;
            case QVariant::String:
                newfield.setFieldType("TEXT");
                break;
            }

            fields.append(newfield);
            orignalFieldNames << field.name();
            //qDebug() << field;
        }

        m_database.transaction();

        SchemaDialog dialog(&m_database, nullptr, this);
        dialog.setFields(fields);
        dialog.setName(SqlService::suggestTableName(tableName, &m_database));

        dialog.exec();

        foreach(QString sql, SqlFileImporter::generateCreateTableSql(dialog.name(), dialog.fields(), dialog.useFts4())) {
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

void MainWindow::setNumDecimalPlaces()
{
    const QAction *action = static_cast<QAction *>(sender());
    if (action == nullptr) return;

    int logicalIndex = action->data().toInt();
    if (logicalIndex < 0) return;

    int currentNumDecimalPlaces = m_tableViewItemDelegate1->numDecimalPlaces(logicalIndex);
    if (currentNumDecimalPlaces == TableViewStyledItemDelegate::NUM_DECIMAL_PLACES_NOT_SET) {
        currentNumDecimalPlaces = 4;
    }

    QIntValidator intValidator(0, 8);
    QString result = SheetTextInputDialog::textInput(
                tr("Set number of decimal places"), "", this, QString::number(currentNumDecimalPlaces), false, &intValidator);
    if (result.isEmpty()) return;

    m_tableViewItemDelegate1->setRoundingPrecision(logicalIndex, result.toInt());
    update();
}

void MainWindow::resetNumDecimalPlaces()
{
    const QAction *action = static_cast<QAction *>(sender());
    if (action == nullptr) return;

    int logicalIndex = action->data().toInt();
    if (logicalIndex < 0) return;

    m_tableViewItemDelegate1->setRoundingPrecision(logicalIndex, TableViewStyledItemDelegate::NUM_DECIMAL_PLACES_NOT_SET);
    update();
}

void MainWindow::onToolbarVisibiltyChanged()
{
    ui->actionShow_Toolbar->setChecked(ui->mainToolBar->isVisible());
}

void MainWindow::on_actionGo_to_Hyokai_info_triggered()
{
    QDesktopServices::openUrl(QUrl("http://hyokai.info"));
}

void MainWindow::on_actionUse_fixed_width_font_triggered(bool checked)
{
    QFont font;
    if (checked) {
        font.setFamily("Monaco");
        font.setStyleHint(QFont::Monospace);
    } else {
        font = QApplication::font();
    }
    ui->tableView->setFont(font);
    ui->tableView_2->setFont(font);
}

void MainWindow::on_actionShow_Toolbar_triggered(bool checked)
{
    ui->mainToolBar->setVisible(checked);
}

void MainWindow::on_actionShow_Table_List_View_triggered()
{
    ui->tableListWidget->setVisible(ui->actionShow_Table_List_View->isChecked());
    tableview_settings->setValue(TABLE_LIST_VISIBLE, QVariant(ui->actionShow_Table_List_View->isChecked()));
    tableview_settings->sync();
}

void MainWindow::on_columnListSearchClear_clicked()
{
    ui->columnListSearchEdit->clear();
}

void MainWindow::on_actionShow_Column_List_View_triggered()
{
    ui->columnListWidget->setVisible(ui->actionShow_Column_List_View->isChecked());
    tableview_settings->setValue(COLUMN_LIST_VISIBLE, QVariant(ui->actionShow_Column_List_View->isChecked()));
    tableview_settings->sync();
}

void MainWindow::on_columnListWidget_visibilityChanged(bool visible)
{
    if (m_isClosing) return;
    ui->actionShow_Column_List_View->setChecked(visible);
    tableview_settings->setValue(COLUMN_LIST_VISIBLE, QVariant(visible));
    tableview_settings->sync();
}

void MainWindow::on_tableListWidget_visibilityChanged(bool visible)
{
    if (m_isClosing) return;
    ui->actionShow_Table_List_View->setChecked(visible);
    tableview_settings->setValue(TABLE_LIST_VISIBLE, QVariant(visible));
    tableview_settings->sync();
}

void MainWindow::on_columnListView_currentRowChanged(int currentRow)
{
    if (currentRow < 0) return;
    auto currentVisibleIndex = ui->tableView->indexAt(QPoint(0, 0));
    auto index = m_tableModel->index(currentVisibleIndex.row(), currentRow);
    ui->tableView->scrollTo(index);
    ui->tableView->selectColumn(currentRow);
}

void MainWindow::onTableViewScrollMoved(int value) {
    if (value != ui->tableView->verticalScrollBar()->value()) {
        ui->tableView->verticalScrollBar()->setValue(value);
    }
    if (value != ui->tableView_2->verticalScrollBar()->value()) {
        ui->tableView_2->verticalScrollBar()->setValue(value);
    }
}

void MainWindow::on_splitter_splitterMoved(int pos, int index)
{
    if (index != 1) {
        return;
    }
    ui->tableView->verticalHeader()->setVisible(pos == 0);
    ui->actionSplit_Window->setChecked(pos != 0);

    if (pos == 0) {
        for (int i = 0; i < m_splitColumn; i++) {
            ui->tableView->showColumn(i);
        }
        for (int i = m_splitColumn; i < m_tableModel->columnCount(); i++) {
            ui->tableView_2->showColumn(i);
        }
    }
}

void MainWindow::on_actionSplit_Window_triggered(bool checked)
{
    if (checked) {
        auto selected = ui->tableView->selectionModel()->selectedIndexes();
        int selectedColumn = 0;

        if (selected.size() > 0) {
            selectedColumn = selected.first().column();
            foreach(QModelIndex index, selected) {
                selectedColumn = MIN(selectedColumn, index.column());
            }
        }

        if (selectedColumn == 0) {
            selectedColumn = 1;
        }


        m_splitColumn = selectedColumn;

        int columnWidth = ui->tableView->verticalHeader()->width();
        for (int i = 0; i < selectedColumn; i++) {
            columnWidth += ui->tableView->horizontalHeader()->sectionSize(i);
            ui->tableView->hideColumn(i);
        }

        for (int i = selectedColumn; i < m_tableModel->columnCount(); i++) {
            ui->tableView_2->hideColumn(i);
        }

        int viewWidth = ui->tableView->width() - ui->splitter->handleWidth();

        QList<int> sizes;
        if (viewWidth < columnWidth + 50) {
            sizes << viewWidth-50 << 50;
        } else {
            sizes << columnWidth << (viewWidth - columnWidth);
        }

        ui->splitter->setSizes(sizes);
        ui->tableView->verticalHeader()->setVisible(false);
    } else {
        for (int i = 0; i < m_splitColumn; i++) {
            ui->tableView->showColumn(i);
        }
        for (int i = m_splitColumn; i < m_tableModel->columnCount(); i++) {
            ui->tableView_2->showColumn(i);
        }
        QList<int> sizes;
        sizes << 0 << 1;
        ui->splitter->setSizes(sizes);
        ui->tableView->verticalHeader()->setVisible(true);
    }
}
