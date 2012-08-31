#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "main.h"
#include "schemadialog.h"
#include "sheetmessagebox.h"
#include "custumsql.h"
#include "sqltablemodelalternativebackground.h"

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


#define LAST_IMPORT_DIRECTORY "LAST_IMPORT_DIRECTORY"
#define LAST_EXPORT_DIRECTORY "LAST_EXPORT_DIRECTORY"
#define LAST_SQLITE_DIRECTORY "LAST_SQLITE_DIRECTORY"

static QSqlDatabase sqlite = QSqlDatabase::addDatabase("QSQLITE");
static int open_count = 0;

MainWindow::MainWindow(QWidget *parent, QString path) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), isDuty(false), custumSql(0)
{
    ui->setupUi(this);
    m_filepath = path;
    QFileInfo fileinfo(path);
    setWindowTitle(QString("[*] ") + fileinfo.baseName());

    open_count++;
    m_database = QSqlDatabase::cloneDatabase(sqlite, QString::number(open_count));
    m_database.setDatabaseName(path);
    m_database.open();

    // register extension functions
    QVariant v = m_database.driver()->handle();
    if (v.isValid() && qstrcmp(v.typeName(), "sqlite3*")==0) {
        sqlite3 *handle = *static_cast<sqlite3 **>(v.data());
        if (handle != 0) {
            RegisterExtensionFunctions(handle);
        }
    }

    tableModel = new SqlTableModelAlternativeBackground(this, m_database);
    tableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    ui->mainToolBar->setIconSize(QSize(22, 22));

    updateDatabase();

    ui->tableView->setModel(tableModel);
    ui->tableView->horizontalHeader()->setMovable(true);
    connect(ui->sqlLine, SIGNAL(returnPressed()), SLOT(filterFinished()));
    connect(ui->tableSelect, SIGNAL(currentIndexChanged(QString)), SLOT(tableChanged(QString)));
    connect(ui->tableView->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), SLOT(sortIndicatorChanged(int,Qt::SortOrder)));
    connect(tableModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(tableUpdated()));
    connect(ui->menuWindow, SIGNAL(aboutToShow()), SLOT(onWindowMenuShow()));

    filterFinished();
}

MainWindow::~MainWindow()
{
    delete ui;
    if (custumSql) {
        delete custumSql;
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_filepath.compare(":memory:") == 0 && m_database.tables().size()) {
        QMessageBox::StandardButton selected =
                SheetMessageBox::warning(this, tr("All changes will be destoried."),
                                         tr("All changes in memory database will NOT be saved. You have to export table to save."),
                                         QMessageBox::Discard|QMessageBox::Cancel, QMessageBox::Cancel);
        switch(selected) {
        case QMessageBox::Discard:
            event->accept();
            if (custumSql) {
                custumSql->close();
            }
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
        if (custumSql) {
            custumSql->close();
        }
    }
}

void MainWindow::onWindowMenuShow()
{
    ui->menuWindow->clear();
    foreach(MainWindow *window, windowList) {
        if (window->isVisible()) {
            QAction *action = ui->menuWindow->addAction(QFileInfo(window->filePath()).baseName());
            connect(action, SIGNAL(triggered()), window, SLOT(activate()));
        }
    }
}

void MainWindow::activate()
{
    activateWindow();
}

bool MainWindow::confirmDuty()
{
    if (isDuty) {
        QMessageBox::StandardButton selected =
                SheetMessageBox::question(this, tr("The table is changed."), tr("Do you want to save or discard changes?"),
                                          QMessageBox::Save|QMessageBox::Cancel|QMessageBox::Discard, QMessageBox::Save);
        switch(selected) {
        case QMessageBox::Cancel:
            ui->tableSelect->setCurrentIndex(ui->tableSelect->findText(tableModel->tableName()));
            return false;
        case QMessageBox::Save:
            if (!tableModel->submitAll()) {
                ui->tableSelect->setCurrentIndex(ui->tableSelect->findText(tableModel->tableName()));
                SheetMessageBox::critical(this, tr("Cannot save table"), tableModel->lastError().text());
                return false;
            }
            isDuty = false;
            break;
        case QMessageBox::Discard:
        default:
            isDuty = false;
            break;
        }
        return true;
    }
    return true;
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
        count = m_database.exec(QString("SELECT count(*) FROM %1;").arg(tableModel->tableName()));
    } else {
        count = m_database.exec(QString("SELECT count(*) FROM %1 WHERE %2;").arg(tableModel->tableName(), ui->sqlLine->text()));
    }

    count.next();
    ui->tableView->setStatusTip(QString("%1 rows found").arg(QString::number(count.value(0).toLongLong())));
}


void MainWindow::tableChanged(const QString &name)
{
    if (name == tableModel->tableName())
        return;
    if (!confirmDuty())
        return;

    tableModel->setTable(name);
    tableModel->select();
    ui->tableView->horizontalHeader()->setSortIndicatorShown(false);
    isDuty = false;
    setWindowModified(false);
}

void MainWindow::tableUpdated()
{
    isDuty = true;
    setWindowModified(true);
}

void MainWindow::updateDatabase()
{
    ui->tableSelect->clear();
    foreach(QString name, m_database.tables()) {
        ui->tableSelect->addItem(name);
    }

    if (tableModel->tableName().isEmpty() || !m_database.tables().contains(tableModel->tableName())) {
        if (m_database.tables().size())
            tableChanged(m_database.tables()[0]);
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
    if (!confirmDuty())
        return;

    SchemaDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    QStringList sqls;
    sqls.append(dialog.createTableSql());
    sqls.append(dialog.createIndexSqls());

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
    if (m_filepath.compare(":memory:") == 0 && m_database.tables().size() == 0) {
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
    tableUpdated();
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
    tableUpdated();
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

QString MainWindow::importFile(QString import, bool autoimport)
{
    if (import.isEmpty())
        return QString();
    QFile file(import);
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
        SheetMessageBox::critical(this, tr("Cannot open file"), tr("Cannot open import file."));
        return QString();
    }
    QFileInfo fileInfo(import);
    tableview_settings->setValue(LAST_IMPORT_DIRECTORY, fileInfo.dir().absolutePath());
    SchemaDialog dialog(this);
    dialog.setShowLogicalIndex(true);

    {
        // suggests table name
        QString tableName = normstr(fileInfo.completeBaseName());
        if (m_database.tables(QSql::AllTables).contains(tableName)) {
            QString baseName = tableName;
            int i = 2;
            while(m_database.tables(QSql::AllTables).contains(QString("%1_%2").arg(baseName, QString::number(i))))
                i++;
            tableName = QString("%1_%2").arg(baseName, QString::number(i));
        }
        dialog.setName(tableName);
    }

    QList<SchemaField> fields;
    QStringList fieldNames;
    char separator = '\t';
    if (import.endsWith(".csv"))
        separator = ',';

    // suggests field name
    QList<QByteArray> header = file.readLine().trimmed().split(separator);
    int counter = 0;
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
        fields.last().setLogicalIndex(counter);
        counter += 1;
    }

    // suggests field type
    while(!file.atEnd()) {
        QList<QByteArray> elements = file.readLine().trimmed().split(separator);
        while (elements.size() > fields.size()) {
            fields.append(SchemaField(QString("V%1").arg(QString::number(fields.size()))));
            fields.last().setFieldType(SchemaField::FIELD_INTEGER);
            fields.last().setLogicalIndex(fields.size()-1);
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

    for(int i = 0; i < fields.size(); ++i) {
        if (fields[i].fieldType() == SchemaField::FIELD_INTEGER)
            fields[i].setIndexedField(true);
    }

    dialog.setFields(fields);
    if (!autoimport) {
        if (dialog.exec() != QDialog::Accepted)
            return QString();
    }


    // creat table
    fields = dialog.fields();

    QStringList sqls;
    sqls.append(dialog.createTableSql());
    sqls.append(dialog.createIndexSqls());

    foreach (const QString sql, sqls) {
        m_database.exec(sql);

        if (m_database.lastError().type() != QSqlError::NoError) {
            SheetMessageBox::warning(this, tr("Cannot make table"), m_database.lastError().text()+"\n\n"+sql);
            return QString();
        }
    }

    // prepare insert SQL
    int insertNumber = dialog.fields().size();
    QString insertSqlText("?");
    for (int i = 0; i < insertNumber-1; i++) {
        insertSqlText.append(",?");
    }

    QSqlQuery insertQuery(QString("INSERT INTO %1 VALUES(%2)").arg(dialog.name(), insertSqlText), m_database);

    file.seek(0);
    file.readLine(); // skip header

    // insert data
    m_database.transaction();

    while(!file.atEnd()) {
        QList<QByteArray> elements = file.readLine().trimmed().split(separator);
        for (int i = 0; i < insertNumber; ++i) {
            if (fields[i].logicalIndex() >= elements.size() || fields[i].logicalIndex() < 0) {
                insertQuery.bindValue(i, "");
                continue;
            }
            switch(fields[i].fieldType()) {
            case SchemaField::FIELD_INTEGER:
                insertQuery.bindValue(i, elements[fields[i].logicalIndex()].toLongLong());
                break;
            case SchemaField::FIELD_REAL:
                insertQuery.bindValue(i, elements[fields[i].logicalIndex()].toDouble());
                break;
            default:
                insertQuery.bindValue(i, QString(elements[fields[i].logicalIndex()]));
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

    m_database.commit();
    return dialog.name();
}

void MainWindow::on_actionImportTable_triggered()
{
    if (!confirmDuty())
        return;

    QStringList import = QFileDialog::getOpenFileNames(this, tr("Select import file"),
                                                  tableview_settings->value(LAST_IMPORT_DIRECTORY, QDir::homePath()).toString(),
                                                  tr("Text (*.txt *.csv);; All (*)"));

    if (import.isEmpty())
        return;

    QMessageBox::StandardButton button = SheetMessageBox::question(this, tr("Multiple files are selected"), tr("Do you want to import with default options?"),
                                                                   QMessageBox::Yes|QMessageBox::No, QMessageBox::No);

    QString importedTable;
    foreach(QString path, import) {
        importedTable = importFile(path, button == QMessageBox::Yes);
        if (importedTable.isEmpty())
            break;
    }

    updateDatabase();
    filterFinished();
    if (!importedTable.isEmpty())
        ui->tableSelect->setCurrentIndex(ui->tableSelect->findText(importedTable));
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::on_actionAbout_Table_View_triggered()
{
    QMessageBox about(this);
    about.setWindowTitle(tr("Table View"));
    about.setIconPixmap(QPixmap(":rc/images/icon128.png"));
    about.setTextFormat(Qt::RichText);
    about.setText(tr("Table View 0.1<br /><br />"
                     "Simple SQLite Viewer<br /><br />"
                     "Developing on <a href=\"https://github.com/informationsea/TableView\">Github</a><hr />"
                     "Toolbar icons by <a href=\"http://tango.freedesktop.org\">Tango Desktop Project</a>"));
    about.exec();
}

void MainWindow::on_actionRun_Custum_SQL_triggered()
{
    if (custumSql) {
        if (custumSql->isVisible()) {
            custumSql->activateWindow();
            return;
        } else {
            delete custumSql;
        }
    }

    custumSql = new CustumSql(&m_database, this);
    custumSql->show();
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
    if (tableModel->tableName().isEmpty()) {
        SheetMessageBox::information(this, tr("No table is selected"), tr("Please select a table to export"));
        return;
    }

    if (!confirmDuty())
        return;

    QSqlQuery query;
    if (ui->sqlLine->text().isEmpty()) {
        query = m_database.exec(QString("SELECT * FROM %1").arg(tableModel->tableName()));
    } else {
        query = m_database.exec(QString("SELECT * FROM %1 WHERE %2").arg(tableModel->tableName(), ui->sqlLine->text()));
    }

    if (query.lastError().type() != QSqlError::NoError) {
        SheetMessageBox::warning(this, tr("Cannot export"), m_database.lastError().text()+"\n\n"+query.lastQuery());
        return;
    }

    QString outputpath = QFileDialog::getSaveFileName(this, tr("Export as text"), tableview_settings->value(LAST_EXPORT_DIRECTORY, QDir::homePath()).toString(), "Tab separated (*.txt);; CSV (*.csv)");
    if (outputpath.isEmpty())
        return;
    QFile outputfile(outputpath);
    QFileInfo outputfileinfo(outputpath);
    tableview_settings->setValue(LAST_EXPORT_DIRECTORY, outputfileinfo.dir().absolutePath());
    outputfile.open(QIODevice::WriteOnly);

    QSqlRecord records = m_database.record(tableModel->tableName());
    for (int i = 0; i < records.count(); ++i) {
        if (i != 0)
            outputfile.write("\t");
        outputfile.write(records.fieldName(i).toUtf8());
    }
    outputfile.write("\n");

    while(query.next()) {
        records = query.record();
        for (int i = 0; i < records.count(); ++i) {
            if (i != 0)
                outputfile.write("\t");
            outputfile.write(records.value(i).toString().toUtf8());
        }
        outputfile.write("\n");
    }

    outputfile.close();
}

void MainWindow::on_actionOpen_In_Memory_Database_triggered()
{
    foreach (MainWindow *window, windowList) {
        if (window->isVisible() && window->filePath() == ":memory:") {
            window->activateWindow();
            return;
        }
    }
    open(":memory:");
}
