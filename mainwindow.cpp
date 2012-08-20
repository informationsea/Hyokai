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
#include <QFile>
#include <QRegExp>
#include <QDebug>
#include <QSqlRecord>

#define LAST_IMPORT_DIRECTORY "LAST_IMPORT_DIRECTORY"
#define LAST_SQLITE_DIRECTORY "LAST_SQLITE_DIRECTORY"

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

    tableModel = new QSqlTableModel(this, theDb);
    tableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    ui->mainToolBar->setIconSize(QSize(22, 22));

    updateDatabase();

    ui->tableView->setModel(tableModel);
    connect(ui->sqlLine, SIGNAL(returnPressed()), SLOT(filterFinished()));
    connect(ui->tableSelect, SIGNAL(currentIndexChanged(QString)), SLOT(tableChanged(QString)));
    connect(ui->tableView->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), SLOT(sortIndicatorChanged(int,Qt::SortOrder)));
    connect(tableModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(updateTable()));

    filterFinished();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (filepath.compare(":memory:") == 0 && theDb.tables().size()) {
        QMessageBox::StandardButton selected =
                SheetMessageBox::warning(this, tr("All changes will be destoried."),
                                         tr("All changes in memory database will NOT be saved. You have to export table to save."),
                                         QMessageBox::Discard|QMessageBox::Cancel, QMessageBox::Cancel);
        switch(selected) {
        case QMessageBox::Discard:
            event->accept();
            return;
        default:
            event->ignore();
            return;
        }
    }

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
    if (tableModel->tableName().isEmpty())
        return;
    tableModel->setFilter(ui->sqlLine->text());
    tableModel->select();
    if (tableModel->lastError().type() != QSqlError::NoError) {
        SheetMessageBox::warning(this, tr("Cannot apply the filter."), tableModel->lastError().text());
    }

    QSqlQuery count;
    if (ui->sqlLine->text().isEmpty()) {
        count = theDb.exec(QString("SELECT count(*) FROM %1;").arg(tableModel->tableName()));
    } else {
        count = theDb.exec(QString("SELECT count(*) FROM %1 WHERE %2;").arg(tableModel->tableName(), ui->sqlLine->text()));
    }

    count.next();
    ui->tableView->setStatusTip(QString("%1 rows found").arg(QString::number(count.value(0).toLongLong())));
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

void MainWindow::updateDatabase()
{
    ui->tableSelect->clear();
    foreach(QString name, theDb.tables()) {
        ui->tableSelect->addItem(name);
    }

    if (tableModel->tableName().isEmpty() || !theDb.tables().contains(tableModel->tableName())) {
    if (theDb.tables().size())
        tableChanged(theDb.tables()[0]);
        ui->tableSelect->setCurrentIndex(ui->tableSelect->findText(tableModel->tableName()));
    }
}

void MainWindow::sortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
    if (isDuty) {
        SheetMessageBox::warning(this, tr("Data is not commited"), tr("You have to commit changes before sorting."));
        return;
    }
    ui->tableView->horizontalHeader()->setSortIndicatorShown(true);
    tableModel->sort(logicalIndex, order);
}

void MainWindow::on_actionGo_github_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/informationsea/TableView"));
}

void MainWindow::on_actionCommit_triggered()
{
    if (tableModel->submitAll()) {
        isDuty = false;
        setWindowModified(false);
        filterFinished();
    } else {
        SheetMessageBox::critical(this, tr("Cannot save table"), tableModel->lastError().text());
    }
}

void MainWindow::on_actionRevert_triggered()
{
    tableModel->revertAll();
    isDuty = false;
    setWindowModified(false);
    filterFinished();
}

void MainWindow::on_actionCreateTable_triggered()
{
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
            isDuty = false;
            break;
        case QMessageBox::Discard:
        default:
            tableModel->revertAll();
            isDuty = false;
            break;
        }
    }


    SchemaDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    QString sql = dialog.createTableSql();

    theDb.exec(sql);
    if (theDb.lastError().type() != QSqlError::NoError) {
        SheetMessageBox::warning(this, tr("Cannot make table"), theDb.lastError().text()+"\n\n"+sql);
    }
    updateDatabase();
    filterFinished();
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
    QString path = QFileDialog::getOpenFileName(NULL, "Open SQLite3 Database",
                                                tableview_settings->value(LAST_SQLITE_DIRECTORY, QDir::homePath()).toString(),
                                                "SQLite3 (*.sqlite3);; All (*)");
    if (path.isEmpty())
        return;
    open(path);
    QFileInfo fileInfo(path);
    tableview_settings->setValue(LAST_SQLITE_DIRECTORY, fileInfo.dir().absolutePath());
}

void MainWindow::on_actionNew_triggered()
{
    QString path = QFileDialog::getSaveFileName(NULL, "New SQLite3 Database",
                                                tableview_settings->value(LAST_SQLITE_DIRECTORY, QDir::homePath()).toString(),
                                                "SQLite3 (*.sqlite3);; All (*)");
    if (path.isEmpty())
        return;
    open(path);
    QFileInfo fileInfo(path);
    tableview_settings->setValue(LAST_SQLITE_DIRECTORY, fileInfo.dir().absolutePath());
}

void MainWindow::on_actionInsert_triggered()
{
    if (tableModel->tableName().isEmpty())
        return;
    QItemSelectionModel *selection = ui->tableView->selectionModel();
    QList<int> rows;
    foreach (QModelIndex index, selection->selectedIndexes()) {
        if (!rows.contains(index.row()))
            rows << index.row();
    }
    qSort(rows);
    if (rows.isEmpty())
        tableModel->insertRow(0);
    else
        tableModel->insertRow(rows.last()+1);
    updateTable();
}

void MainWindow::on_actionDelete_triggered()
{
    if (tableModel->tableName().isEmpty())
        return;
    QItemSelectionModel *selection = ui->tableView->selectionModel();
    QList<int> rows;
    foreach (QModelIndex index, selection->selectedIndexes()) {
        if (!rows.contains(index.row()))
            rows << index.row();
    }
    qSort(rows);
    while(!rows.isEmpty()) {
        tableModel->removeRow(rows.takeLast());
    }
    updateTable();
}

void MainWindow::on_actionQuit_triggered()
{
    foreach(MainWindow *window, ::windowList) {
        window->close();
    }
}

static QString normstr(QString str, bool shoudStartWithAlpha = true)
{
    str = str.trimmed();
    if (str.at(0).isDigit() && shoudStartWithAlpha) {
        str.insert(0, 'V');
    }
    str = str.replace("\"", "");
    str = str.replace(QRegExp("[^a-zA-Z0-9_]"), "_");
    return str;
}

void MainWindow::on_actionImportTable_triggered()
{
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
            isDuty = false;
            break;
        case QMessageBox::Discard:
        default:
            tableModel->revertAll();
            isDuty = false;
            break;
        }
    }

    QString import = QFileDialog::getOpenFileName(this, tr("Select import file"),
                                                  tableview_settings->value(LAST_IMPORT_DIRECTORY, QDir::homePath()).toString(),
                                                  tr("Text (*.txt *.csv);; All (*)"));
    if (import.isEmpty())
        return;
    QFile file(import);
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
        SheetMessageBox::critical(this, tr("Cannot open file"), tr("Cannot open import file."));
        return;
    }
    QFileInfo fileInfo(import);
    tableview_settings->setValue(LAST_IMPORT_DIRECTORY, fileInfo.dir().absolutePath());
    SchemaDialog dialog(this);
    dialog.setName(normstr(fileInfo.baseName(), false));

    QList<SchemaField> fields;
    QStringList fieldNames;
    char separator = '\t';
    if (import.endsWith(".csv"))
        separator = ',';

    // suggets field name
    QList<QByteArray> header = file.readLine().trimmed().split(separator);
    foreach(QByteArray one, header) {
        QString newfieldname = normstr(one);
        if (fieldNames.contains(newfieldname)) {
            QString basename = newfieldname;
            int counter = 2;
            do {
                newfieldname = QString("%1_%2").arg(basename, QString::number(counter));
                counter += 1;
            } while(fieldNames.contains(newfieldname));
        }
        fields.append(SchemaField(newfieldname));
        fieldNames.append(newfieldname);
        fields.last().setFieldType(SchemaField::FIELD_INTEGER);
    }

    // suggests field type
    while(!file.atEnd()) {
        QList<QByteArray> elements = file.readLine().trimmed().split(separator);
        while (elements.size() > fields.size()) {
            fields.append(SchemaField(QString("V%1").arg(QString::number(fields.size()))));
            fields.last().setFieldType(SchemaField::FIELD_INTEGER);
        }

        for (int i = 0; i < fields.size() && i < elements.size(); i++) {
            bool ok;
            QString str(elements[i]);
            switch (fields[i].fieldType()) {
            case SchemaField::FIELD_NONE:
            case SchemaField::FIELD_TEXT:
                break;
            case SchemaField::FIELD_INTEGER:
                str.toLongLong(&ok);
                if (ok)
                    break;
                fields[i].setFieldType(SchemaField::FIELD_REAL);
                // no break
            case SchemaField::FIELD_REAL:
                str.toDouble(&ok);
                if (ok)
                    break;
                fields[i].setFieldType(SchemaField::FIELD_NONE);
                break;
            }
        }
    }

    dialog.setFields(fields);
    if (dialog.exec() != QDialog::Accepted)
        return;


    // creat table
    fields = dialog.fields();
    QString sql = dialog.createTableSql();
    theDb.exec(sql);
    if (theDb.lastError().type() != QSqlError::NoError) {
        SheetMessageBox::warning(this, tr("Cannot make table"), theDb.lastError().text()+"\n\n"+sql);
        return;
    }

    // prepare insert SQL
    int insertNumber = dialog.fields().size();
    QString insertSqlText("?");
    for (int i = 0; i < insertNumber-1; i++) {
        insertSqlText.append(",?");
    }

    QSqlQuery insertQuery(QString("INSERT INTO %1 VALUES(%2)").arg(dialog.name(), insertSqlText), theDb);

    file.seek(0);
    file.readLine(); // skip header

    // insert data
    theDb.transaction();

    while(!file.atEnd()) {
        QList<QByteArray> elements = file.readLine().trimmed().split(separator);
        for (int i = 0; i < insertNumber; ++i) {
            if (i >= elements.size()) {
                insertQuery.bindValue(i, "");
                continue;
            }
            switch(fields[i].fieldType()) {
            case SchemaField::FIELD_INTEGER:
                insertQuery.bindValue(i, elements[i].toLongLong());
                break;
            case SchemaField::FIELD_REAL:
                insertQuery.bindValue(i, elements[i].toDouble());
                break;
            default:
                insertQuery.bindValue(i, QString(elements[i]));
                break;
            }
        }
        if (!insertQuery.exec()) {
            if (SheetMessageBox::warning(this, tr("Insert error"),
                                         insertQuery.lastError().text() + "\n\n" + insertQuery.lastQuery(),
                                         QMessageBox::Ok|QMessageBox::Abort) == QMessageBox::Abort) {
                break;
            }
        }
    }

    theDb.commit();

    tableModel->setTable(dialog.name());
    updateDatabase();
    filterFinished();
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::on_actionAbout_Table_View_triggered()
{
    QMessageBox::about(this, tr("Table View"), tr("Table View - SQLite Viewer\n\nhttps://github.com/informationsea/TableView"));
}
