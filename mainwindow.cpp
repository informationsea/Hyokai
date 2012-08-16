#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "main.h"
#include "schemadialog.h"

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

static QSqlDatabase sqlite = QSqlDatabase::addDatabase("QSQLITE");

MainWindow::MainWindow(QWidget *parent, QString path) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    filepath = path;
    QFileInfo fileinfo(path);
    setWindowTitle(fileinfo.baseName());

    theDb = QSqlDatabase::cloneDatabase(sqlite, path);
    theDb.setDatabaseName(path);
    theDb.open();

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
    connect(ui->actionQuit, SIGNAL(triggered()), SLOT(quit()));
    connect(ui->tableView->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), SLOT(sortIndicatorChanged(int,Qt::SortOrder)));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::quit()
{
    foreach(MainWindow *window, ::windowList) {
        window->close();
    }
}

void MainWindow::filterFinished()
{
    tableModel->setFilter(ui->sqlLine->text());
    qDebug() << tableModel->lastError();
    tableModel->select();
}


void MainWindow::tableChanged(const QString &name)
{
    tableModel->setTable(name);
    tableModel->select();
    ui->tableView->horizontalHeader()->setSortIndicatorShown(false);
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
    tableModel->submitAll();
}

void MainWindow::on_actionRevert_triggered()
{
    tableModel->revertAll();
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
}
