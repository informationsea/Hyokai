#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <cmath>

#include "main.h"
#include "schemadialog.h"
#include "sheetmessagebox.h"
#include "custumsql.h"
#include "sqltablemodelalternativebackground.h"
#include "attachdatabasedialog.h"
#include "summarydialog.h"

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

#define LAST_IMPORT_DIRECTORY "LAST_IMPORT_DIRECTORY"
#define LAST_EXPORT_DIRECTORY "LAST_EXPORT_DIRECTORY"
#define LAST_SQLITE_DIRECTORY "LAST_SQLITE_DIRECTORY"

static QSqlDatabase sqlite = QSqlDatabase::addDatabase("QSQLITE");
static int open_count = 0;

MainWindow::MainWindow(QWidget *parent, QString path) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), m_isDuty(false), m_custumSql(0)
{
    ui->setupUi(this);
    m_rowcountlabel = new QLabel(ui->statusBar);
    //ui->statusBar->addWidget(sqlLineCount);
    ui->statusBar->addPermanentWidget(m_rowcountlabel);

    move(nextWindowPosition());

    m_filepath = path;

    open_count++;
    m_database = QSqlDatabase::cloneDatabase(sqlite, QString::number(open_count));
    m_database.setDatabaseName(path);
    m_database.open();

    m_tableModel = new SqlTableModelAlternativeBackground(this, m_database);
    m_tableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    ui->mainToolBar->setIconSize(QSize(22, 22));

    updateDatabase();

    ui->tableView->setModel(m_tableModel);
    ui->tableView->horizontalHeader()->setMovable(true);
    connect(ui->sqlLine, SIGNAL(returnPressed()), SLOT(filterFinished()));
    connect(ui->tableSelect, SIGNAL(currentIndexChanged(QString)), SLOT(tableChanged(QString)));
    connect(ui->tableView->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), SLOT(sortIndicatorChanged(int,Qt::SortOrder)));
    connect(m_tableModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(tableUpdated()));
    connect(ui->menuWindow, SIGNAL(aboutToShow()), SLOT(onWindowMenuShow()));
    ui->tableView->horizontalHeader()->installEventFilter(this);

    if (m_filepath.compare(":memory:") == 0) {
        ui->actionView_in_File_Manager->setEnabled(false);
    }

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

    filterFinished();

    if (m_filepath.compare(":memory:") != 0) {
        setWindowFilePath(m_filepath);
    } else {
        ui->actionR_code_to_import->setEnabled(false);
    }
    QFileInfo fileinfo(path);
    setWindowTitle(QString("[*] ") + fileinfo.baseName());
}

MainWindow::~MainWindow()
{
    delete ui;
    if (m_custumSql) {
        delete m_custumSql;
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
            if (m_custumSql) {
                m_custumSql->close();
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
        if (m_custumSql) {
            m_custumSql->close();
        }
    }
}

void MainWindow::importOneFile(const QString &path)
{
    QString importedTable = importFile(path, false);

    updateDatabase();
    filterFinished();
    if (!importedTable.isEmpty())
        ui->tableSelect->setCurrentIndex(ui->tableSelect->findText(importedTable));
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
            popup.exec();
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
        if (m_windowList[i]->isVisible()) {
            QAction *action = ui->menuWindow->addAction(m_windowList[i]->windowTitle().replace("[*] ", ""));
            action->setCheckable(true);
            if (m_windowList[i]->isActiveWindow())
                action->setChecked(true);
            action->setData(i);
            connect(action, SIGNAL(triggered()), SLOT(activate()));
        }
    }
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
    if (ui->sqlLine->text().isEmpty()) {
        query = m_database.exec(QString("SELECT %1 FROM %2").arg(column_name, m_tableModel->tableName()));
    } else {
        query = m_database.exec(QString("SELECT %1 FROM %2 WHERE %3").arg(column_name, m_tableModel->tableName(), ui->sqlLine->text()));
    }

    if (query.lastError().type() != QSqlError::NoError) {
        SheetMessageBox::warning(this, tr("Cannot export"), m_database.lastError().text()+"\n\n"+query.lastQuery());
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
}

bool MainWindow::confirmDuty()
{
    if (m_isDuty) {
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
            m_isDuty = false;
            break;
        case QMessageBox::Discard:
        default:
            m_isDuty = false;
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
    m_tableModel->setFilter(ui->sqlLine->text());
    m_tableModel->select();
    if (m_tableModel->lastError().type() != QSqlError::NoError) {
        SheetMessageBox::warning(this, tr("Cannot apply the filter."), m_tableModel->lastError().text());
    }

    QSqlQuery count;
    if (ui->sqlLine->text().isEmpty()) {
        count = m_database.exec(QString("SELECT count(*) FROM %1;").arg(m_tableModel->tableName()));
    } else {
        count = m_database.exec(QString("SELECT count(*) FROM %1 WHERE %2;").arg(m_tableModel->tableName(), ui->sqlLine->text()));
    }

    count.next();
    m_rowcountlabel->setText(QString("%1 rows").arg(QString::number(count.value(0).toLongLong())));
}


void MainWindow::tableChanged(const QString &name)
{
    if (name == m_tableModel->plainTableName())
        return;
    if (!confirmDuty())
        return;

    m_tableModel->setTable(name);
    m_tableModel->select();
    ui->tableView->horizontalHeader()->setSortIndicatorShown(false);
    m_isDuty = false;
    setWindowModified(false);
    filterFinished();
}

void MainWindow::tableUpdated()
{
    m_isDuty = true;
    setWindowModified(true);
}

void MainWindow::updateDatabase()
{
    ui->tableSelect->clear();
    foreach(QString name, m_database.tables()) {
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
    if (m_isDuty) {
        SheetMessageBox::warning(this, tr("Data is not commited"), tr("You have to commit changes before sorting."));
        return;
    }
    ui->tableView->horizontalHeader()->setSortIndicatorShown(true);
    m_tableModel->sort(logicalIndex, order);
}

void MainWindow::on_actionGo_github_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/informationsea/TableView"));
}

void MainWindow::on_actionCommit_triggered()
{
    if (m_tableModel->submitAll()) {
        m_isDuty = false;
        setWindowModified(false);
        filterFinished();
    } else {
        SheetMessageBox::critical(this, tr("Cannot save table"), m_tableModel->lastError().text());
    }
}

void MainWindow::on_actionRevert_triggered()
{
    m_tableModel->revertAll();
    m_isDuty = false;
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

void MainWindow::refresh()
{
    updateDatabase();
    filterFinished();
}

void MainWindow::on_actionOpen_triggered()
{
    QString path = QFileDialog::getOpenFileName(NULL, "Open SQLite3 Database or text file",
                                                tableview_settings->value(LAST_SQLITE_DIRECTORY, QDir::homePath()).toString(),
                                                "All (*.sqlite3; *.txt; *.csv);;SQLite3 (*.sqlite3);; Text (*.txt);; CSV (*.csv);; All (*)");
    if (path.isEmpty())
        return;
    if (path.endsWith(".sqlite3")) {
        open(path);
    } else {
        importOneFile(path);
    }
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
    if (m_tableModel->plainTableName().isEmpty())
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
    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        widget->close();
    }
}

static QString normstr(QString str, bool shoudStartWithAlpha = true)
{
    str = str.trimmed();
    if (str.size() == 0)
        str = "V";
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

    QProgressDialog progress1(tr("Suggesting fields"), tr("Skip"), 0, fileInfo.size(), this);
    progress1.setWindowModality(Qt::WindowModal);

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

        progress1.setValue(file.pos());
        if (progress1.wasCanceled()) {
            goto skipSuggest;
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

    skipSuggest:

    for(int i = 0; i < fields.size(); ++i) {
        if (fields[i].fieldType() == SchemaField::FIELD_INTEGER)
            fields[i].setIndexedField(true);
        if (fields[i].fieldType() == SchemaField::FIELD_REAL)
            fields[i].setIndexedField(true);
    }

    progress1.close();

    dialog.setFields(fields);
    if (!autoimport) {
        if (dialog.exec() != QDialog::Accepted)
            return QString();
    }

    QProgressDialog progress2(tr("Importing file"), tr("Cancel"), 0, fileInfo.size(), this);
    progress2.setWindowModality(Qt::WindowModal);

    // creat table
    fields = dialog.fields();

    m_database.transaction();

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

        progress2.setValue(file.pos());
        if (progress2.wasCanceled()) {
            m_database.rollback();
            return QString();
        }
    }

    progress2.close();

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

    QMessageBox::StandardButton button;
    if (import.size() > 1) {
        button = SheetMessageBox::question(this, tr("Multiple files are selected"), tr("Do you want to import with default options?"),
                                           QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
    } else {
        button = QMessageBox::No;
    }

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
    if (m_custumSql) {
        if (m_custumSql->isVisible()) {
            m_custumSql->activateWindow();
            return;
        } else {
            delete m_custumSql;
        }
    }

    m_custumSql = new CustumSql(&m_database, this);
    m_custumSql->show();
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
    if (ui->sqlLine->text().isEmpty()) {
        query = m_database.exec(QString("SELECT * FROM %1").arg(m_tableModel->tableName()));
    } else {
        query = m_database.exec(QString("SELECT * FROM %1 WHERE %2").arg(m_tableModel->tableName(), ui->sqlLine->text()));
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

    QString separator = "\t";
    if (outputpath.endsWith(".csv"))
        separator = ",";

    QSqlRecord records = m_database.record(m_tableModel->tableName());
    for (int i = 0; i < records.count(); ++i) {
        if (i != 0)
            outputfile.write(separator.toUtf8());
        outputfile.write(records.fieldName(i).toUtf8());
    }
    outputfile.write("\n");

    while(query.next()) {
        records = query.record();
        for (int i = 0; i < records.count(); ++i) {
            if (i != 0)
                outputfile.write(separator.toUtf8());
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

void MainWindow::on_buttonClear_clicked()
{
    ui->sqlLine->setText("");
    filterFinished();
}

#define MIN(x, y) ((x) > (y) ? (y) : (x))
#define MAX(x, y) ((x) < (y) ? (y) : (x))

void MainWindow::on_actionCopy_triggered()
{
    QModelIndexList selectedIndex;
    if (m_custumSql && m_custumSql->isActiveWindow()) {
        selectedIndex = m_custumSql->tableView()->selectionModel()->selectedIndexes();
    } else {
        selectedIndex = ui->tableView->selectionModel()->selectedIndexes();
    }
    struct select_points {
        int x;
        int y;
    } left_top, right_bottom;

    if (selectedIndex.size() <= 0) return;

    // first data
    left_top.x = selectedIndex[0].column();
    left_top.y = selectedIndex[0].row();
    right_bottom.x = selectedIndex[0].column();
    right_bottom.y = selectedIndex[0].row();

    foreach(QModelIndex index, selectedIndex) {
        left_top.x = MIN(index.column(), left_top.x);
        left_top.y = MIN(index.row(), left_top.y);
        right_bottom.x = MAX(index.column(), right_bottom.x);
        right_bottom.y = MAX(index.row(), right_bottom.y);
    }

    int width = right_bottom.x - left_top.x + 1;
    int height = right_bottom.y - left_top.y + 1;

    QList<QList<QVariant> >  matrix;
    for (int i = 0; i < height; ++i) {
        QList<QVariant> line;
        for (int j = 0; j < width; ++j) {
            line.append(QVariant());
        }
        matrix.append(line);
    }

    QTableView *tableView;
    if (m_custumSql && m_custumSql->isActiveWindow()) {
        tableView = m_custumSql->tableView();
    } else {
        tableView = ui->tableView;
    }

    foreach(QModelIndex index, selectedIndex) {
        matrix[index.row() - left_top.y][index.column() - left_top.x] = tableView->model()->data(index);
    }

    QString clipboard;

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (j != 0)
                clipboard += "\t";
            clipboard += matrix[i][j].toString();
        }
        if (i != height-1)
            clipboard += "\n";
    }
    QClipboard *clip = QApplication::clipboard();
    clip->setText(clipboard);
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
    if (m_filepath == ":memory:")
        return;
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(m_filepath).dir().absolutePath()));
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
    if (m_filepath == ":memory:")
        return;
    QString tableName = m_tableModel->tableName();
    if (m_tableModel->plainTableName().isEmpty())
        return;
    QFileInfo fileInfo(m_filepath);
    QString basename = fileInfo.baseName();
    QString str = QString("# install.packages(c(\"DBI\", \"RSQLite\")) to install SQLite library\n"
                          "library(RSQLite)\n"
                          "connection.%1 <- dbConnect(dbDriver(\"SQLite\"), dbname=\"%2\")\n"
                          "table.%3 <- dbGetQuery(connection.%1, \"select * from %3;\")\n"
                          "# dbDisconnect(connection.%1)\n").arg(basename, m_filepath, tableName);
    QClipboard *clip = QApplication::clipboard();
    clip->setText(str);
}
