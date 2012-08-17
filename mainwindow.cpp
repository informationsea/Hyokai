#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "main.h"
#include "schemadialog.h"
#include "sheetmessagebox.h"

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

static QSqlDatabase sqlite = QSqlDatabase::addDatabase("QSQLITE");

MainWindow::MainWindow(QWidget *parent, QString path) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), isDuty(false)
{
    ui->setupUi(this);
    filepath = path;
    QFileInfo fileinfo(path);
    setWindowTitle(QString("[*] ") + fileinfo.baseName());

    theDb = QSqlDatabase::cloneDatabase(sqlite, path);
    theDb.setDatabaseName(path);
    theDb.open();

    ui->mainToolBar->setIconSize(QSize(22, 22));

    foreach(QString name, theDb.tables()) {
        ui->tableSelect->addItem(name);
    }

    tableModel = new QSqlTableModel(this, theDb);
    tableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    if (theDb.tables().size())
        tableChanged(theDb.tables()[0]);
    ui->tableView->setModel(tableModel);


    connect(ui->sqlLine, SIGNAL(returnPressed()), SLOT(filterFinished()));
    connect(ui->tableSelect, SIGNAL(currentIndexChanged(QString)), SLOT(tableChanged(QString)));
    connect(ui->tableView->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), SLOT(sortIndicatorChanged(int,Qt::SortOrder)));
    connect(tableModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(updateTable()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (isDuty) {
        QMessageBox::StandardButton selected =
                SheetMessageBox::question(this, tr("The table is changed."), tr("Do you want to save or discard changes?"),
                                          QMessageBox::Save|QMessageBox::Cancel|QMessageBox::Discard, QMessageBox::Save);
        switch(selected) {
        case QMessageBox::Cancel:
            event->ignore();
            return;
        case QMessageBox::Save:
            if (!tableModel->submitAll()) {
                SheetMessageBox::critical(this, tr("Cannot save table"), tableModel->lastError().text());
                event->ignore();
                return;
            }
            break;
        case QMessageBox::Discard:
        default:
            break;
        }
    }
    event->accept();
}

void MainWindow::filterFinished()
{
    tableModel->setFilter(ui->sqlLine->text());
    tableModel->select();
    if (tableModel->lastError().type() != QSqlError::NoError) {
        SheetMessageBox::warning(this, tr("Cannot apply the filter."), tableModel->lastError().text());
    }
}


void MainWindow::tableChanged(const QString &name)
{
    if (name == tableModel->tableName())
        return;
    if (isDuty) {
        QMessageBox::StandardButton selected =
                SheetMessageBox::question(this, tr("The table is changed."), tr("Do you want to save or discard changes?"),
                                          QMessageBox::Save|QMessageBox::Cancel|QMessageBox::Discard, QMessageBox::Save);
        switch(selected) {
        case QMessageBox::Cancel:
            ui->tableSelect->setCurrentIndex(ui->tableSelect->findText(tableModel->tableName()));
            return;
        case QMessageBox::Save:
            if (!tableModel->submitAll()) {
                ui->tableSelect->setCurrentIndex(ui->tableSelect->findText(tableModel->tableName()));
                SheetMessageBox::critical(this, tr("Cannot save table"), tableModel->lastError().text());
                return;
            }
            break;
        case QMessageBox::Discard:
        default:
            break;
        }
    }
    tableModel->setTable(name);
    tableModel->select();
    ui->tableView->horizontalHeader()->setSortIndicatorShown(false);
    isDuty = false;
    setWindowModified(false);
}

void MainWindow::updateTable()
{
    isDuty = true;
    setWindowModified(true);
}

void MainWindow::sortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
    ui->tableView->horizontalHeader()->setSortIndicatorShown(true);
    tableModel->sort(logicalIndex, order);
}

void MainWindow::on_actionGo_github_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/informationsea"));
}

void MainWindow::on_actionCommit_triggered()
{
    if (tableModel->submitAll()) {
        isDuty = false;
        setWindowModified(false);
    } else {
        SheetMessageBox::critical(this, tr("Cannot save table"), tableModel->lastError().text());
    }
}

void MainWindow::on_actionRevert_triggered()
{
    tableModel->revertAll();
    isDuty = false;
    setWindowModified(false);
}

void MainWindow::on_actionCreateTable_triggered()
{
    SchemaDialog dialog(this);
    dialog.exec();
}

void MainWindow::open(QString path)
{
    if (path.isEmpty())
        return;
    MainWindow *w = new MainWindow(NULL, path);
    w->show();
    ::windowList.append(w);
    if (filepath.compare(":memory:") == 0 && theDb.tables().size() == 0) {
        close();
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QString path = QFileDialog::getOpenFileName(NULL, "Open SQLite3 Database", QDir::homePath(), "SQLite3 (*.sqlite3);; All (*)");
    open(path);
}

void MainWindow::on_actionNew_triggered()
{
    QString path = QFileDialog::getSaveFileName(NULL, "New SQLite3 Database", QDir::homePath(), "SQLite3 (*.sqlite3);; All (*)");
    open(path);
}

void MainWindow::on_actionInsert_triggered()
{
    tableModel->insertRows(0, 1);
    updateTable();
}

void MainWindow::on_actionDelete_triggered()
{
    QItemSelectionModel *selection = ui->tableView->selectionModel();
    QList<int> rows;
    foreach (QModelIndex index, selection->selectedIndexes()) {
        if (!rows.contains(index.row()))
            rows << index.row();
    }
    qSort(rows);
    foreach(int r, rows) {
        tableModel->removeRow(r);
    }
    updateTable();
}

void MainWindow::on_actionQuit_triggered()
{
    foreach(MainWindow *window, ::windowList) {
        window->close();
    }
}
