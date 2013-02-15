#include "custumsql.h"
#include "ui_custumsql.h"

#include "sheetmessagebox.h"
#include "jointabledialog.h"
#include "main.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include <QPoint>
#include <QSqlRecord>
#include <QLineEdit>
#include <QComboBox>
#include <QStringList>
#include <QFileDialog>

CustumSql::CustumSql(QSqlDatabase *database, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CustumSql), m_database(database),
    m_query(*database), m_querymodel(this)
{
    ui->setupUi(this);
    ui->tableView->setModel(&m_querymodel);
#if QT_VERSION >= 0x050000
    ui->tableView->horizontalHeader()->setSectionsMovable(true);
#else
    ui->tableView->horizontalHeader()->setMovable(true);
#endif
    setWindowTitle("Custum SQL "+QFileInfo(database->databaseName()).completeBaseName());

    ui->splitter->setStretchFactor(0, 0);
    ui->splitter->setStretchFactor(1, 1);

    ui->sql->setDatabase(m_database);

    connect(ui->sql, SIGNAL(returnPressed()), SLOT(on_pushButton_clicked()));

    createMenus();

    m_history = tableview_settings->value(CUSTUM_SQL_HISTORY).toStringList();
}

CustumSql::~CustumSql()
{
    delete ui;
    delete assistMenu;
    delete historyMenu;
    delete menu;
}

QTableView *CustumSql::tableView()
{
    return ui->tableView;
}

void CustumSql::selectTableAll()
{
    ui->tableView->selectAll();
}

void CustumSql::on_pushButton_clicked()
{
    if(ui->sql->toPlainText().isEmpty())
        return;
    m_query.exec(ui->sql->toPlainText());
    if (m_query.lastError().type() != QSqlError::NoError) {
        SheetMessageBox::critical(this, tr("SQL Error"), m_query.lastError().text()+"\n\n"+m_query.lastQuery());
        return;
    }

    ui->sql->setStyleSheet("SQLTextEdit{ background: white; }");

    if (m_history.isEmpty() || m_history.first() != m_query.lastQuery()) {
        m_history.insert(0, m_query.lastQuery());
        m_history.removeDuplicates();
        tableview_settings->setValue(CUSTUM_SQL_HISTORY, m_history);
    }

    if (m_query.isSelect()) {
        m_querymodel.setQuery(m_query);
    } else {
        SheetMessageBox::information(this, tr("SQL Report"), tr("The query SQL was finished successfully.") + "\n\n" + m_query.lastQuery());
    }
}


void CustumSql::on_assistButton_clicked()
{
    QPoint point = ui->assistButton->pos();
    point.setY(point.y() + ui->assistButton->size().height());
    assistMenu->popup(mapToGlobal(point));
}

void CustumSql::setSqlTemplate()
{
    QAction *sender = (QAction *)QObject::sender();
    ui->sql->setPlainText(sender->data().toString());
}


void CustumSql::insertSql()
{
    QAction *sender = (QAction *)QObject::sender();
    ui->sql->insertPlainText(sender->data().toString());
}

void CustumSql::joinSqlWizard()
{
    JoinTableDialog dialog(m_database, this);
    if (dialog.exec() != QDialog::Accepted)
        return;
    ui->sql->setPlainText(dialog.sql());
    on_pushButton_clicked();
}

#define CLEAR_TEXT "**CLEAR**"
#define MAX_HISTORY_SHOW_LENGTH 130
#define MAX_HISTORY_SHOW_ITEMS 10
#define MAX_HISTORY_SHOW_MORE_ITEMS 60

void CustumSql::on_historyButton_clicked()
{
    int count = 0;
    historyMenu->clear();
    QMenu *additionalHistoy;
    foreach(QString query, m_history) {
        if (count == MAX_HISTORY_SHOW_ITEMS+1) {
            historyMenu->addSeparator();
            additionalHistoy = historyMenu->addMenu(tr("More"));
        }
        QAction *action;

        QString showName = query;
        if (showName.length() > MAX_HISTORY_SHOW_LENGTH) {
            showName = query.left(MAX_HISTORY_SHOW_LENGTH/2-2) + QString(" ... ") + query.right(MAX_HISTORY_SHOW_LENGTH/2-2);
        }

        if (count <= MAX_HISTORY_SHOW_ITEMS)
            action = historyMenu->addAction(showName);
        else if (count < MAX_HISTORY_SHOW_MORE_ITEMS)
            action = additionalHistoy->addAction(showName);
        else
            break;
        action->setData(query);
        connect(action, SIGNAL(triggered()), SLOT(onHistorySelected()));
        count += 1;
    }
    historyMenu->addSeparator();
    QAction *clear = historyMenu->addAction(tr("Clear"));
    clear->setData(CLEAR_TEXT);
    connect(clear, SIGNAL(triggered()), SLOT(onHistorySelected()));

    QPoint point = ui->historyButton->pos();
    point.setY(point.y() + ui->historyButton->size().height());
    historyMenu->popup(mapToGlobal(point));
}

void CustumSql::onHistorySelected()
{
    QAction *senderAction = (QAction *)sender();
    if (senderAction->data().toString().compare(CLEAR_TEXT) == 0) {
        m_history.clear();
        tableview_settings->setValue(CUSTUM_SQL_HISTORY, m_history);
    } else {
        ui->sql->setPlainText(senderAction->data().toString());
    }
}

void CustumSql::on_menuButton_clicked()
{
    foreach(QAction* action, m_menu_for_select) {
        action->setEnabled(m_query.isSelect());
    }

    QPoint point = ui->menuButton->pos();
    point.setY(point.y() + ui->menuButton->size().height());
    menu->popup(mapToGlobal(point));
}

void CustumSql::onExportTable()
{
    QString outputpath = QFileDialog::getSaveFileName(this, tr("Export as text"),
                                                      tableview_settings->value(LAST_EXPORT_DIRECTORY, QDir::homePath()).toString(),
                                                      "Tab separated (*.txt);; CSV (*.csv)");
    if (outputpath.isEmpty())
        return;
    QFile outputfile(outputpath);
    QFileInfo outputfileinfo(outputpath);
    tableview_settings->setValue(LAST_EXPORT_DIRECTORY, outputfileinfo.dir().absolutePath());
    outputfile.open(QIODevice::WriteOnly);

    QString separator = "\t";
    if (outputpath.endsWith(".csv"))
        separator = ",";

    if (!m_query.first()) {
        SheetMessageBox::critical(this, tr("Something wrong"), tr("Something wrong with export"));
        return;
    }

    QSqlRecord records = m_query.record();
    for (int i = 0; i < records.count(); ++i) {
        if (i != 0)
            outputfile.write(separator.toUtf8());
        outputfile.write(records.fieldName(i).toUtf8());
    }
    outputfile.write("\n");

    do {
        records = m_query.record();
        for (int i = 0; i < records.count(); ++i) {
            if (i != 0)
                outputfile.write(separator.toUtf8());
            outputfile.write(records.value(i).toString().toUtf8());
        }
        outputfile.write("\n");
    } while (m_query.next());

    outputfile.close();
}

void CustumSql::onCreateView()
{
    ui->sql->setPlainText(QString("CREATE VIEW newview AS %1").arg(m_query.lastQuery()));
    ui->sql->textCursor().setPosition(QString("CREATE VIEW ").length());
    ui->sql->textCursor().setPosition(QString("CREATE VIEW newview").length(), QTextCursor::KeepAnchor);
}


void CustumSql::createMenus()
{
    // Assist Menu
    assistMenu = new QMenu(this);

    QStringList templateList;
    templateList << "SELECT * FROM table"
                 << "SELECT * FROM table1 INNTER JOIN table2 ON table1.column1 = table2.column2"
                 << "SELECT * FROM table1 LEFT OUTER JOIN table2 ON table1.column1 = table2.column2"
                 << "SELECT * FROM table1 NATUAL INNTER JOIN table2"
                 << "SELECT * FROM table1 NATUAL LEFT OUTER JOIN table2"
                 << "DROP TABLE table1"
                 << "DROP VIEW view1"
                 << "DROP INDEX index1"
                 << "CREATE VIEW view1 AS SELECT *"
                 << "CREATE INDEX index1 ON table1(column1)"
                 << "VACUUM";

    QAction *joinwizard = assistMenu->addAction(tr("SQL Join Wizard"));
    connect(joinwizard, SIGNAL(triggered()), SLOT(joinSqlWizard()));

    QMenu *sqltemplates = assistMenu->addMenu(tr("Replace with SQL Templates"));
    foreach(QString i, templateList) {
        QAction *action = sqltemplates->addAction(i);
        action->setData(i);
        connect(action, SIGNAL(triggered()), SLOT(setSqlTemplate()));
    }

    QStringList userTemplates = tableview_settings->value(SQL_TEMPLATES).toStringList();
    if (!userTemplates.isEmpty()) {
        QMenu *usertemplatemenu = assistMenu->addMenu(tr("User Templates"));
        foreach (QString i, userTemplates) {
            QAction *action = usertemplatemenu->addAction(i);
            action->setData(i);
            connect(action, SIGNAL(triggered()), SLOT(insertSql()));
        }
    }

    QStringList templateList2;
    templateList2 << " ORDER BY column ASC"
                  << " ORDER BY column DESC"
                  << " WHERE column == 0"
                  << " WHERE column < 0"
                  << " WHERE column > 0"
                  << " WHERE column LIKE pattern"
                  << " WHERE column GLOB pattern"
                  << " LIMIT limit"
                  << " OFFSET offset"
                  << " GROUP BY column";

    QMenu *sqltemplateinsert = assistMenu->addMenu(tr("Insert SQL Templates"));
    foreach(QString i, templateList2) {
        QAction *action = sqltemplateinsert->addAction(i);
        action->setData(i);
        connect(action, SIGNAL(triggered()), SLOT(insertSql()));
    }

    QStringList commonFunctions;
    commonFunctions << "avg(";
    commonFunctions << "distinct(";
    commonFunctions << "max(";
    commonFunctions << "min(";
    commonFunctions << "count(";
    commonFunctions << "total(";
    commonFunctions << "--";
    commonFunctions << "stdev(";
    commonFunctions << "variance(";
    commonFunctions << "median(";
    commonFunctions << "lower_quartile(";
    commonFunctions << "upper_quartile(";

    QMenu *commonFunctionMenu = assistMenu->addMenu(tr("Common SQL Functions"));
    foreach(QString i, commonFunctions) {
        if (i == "--") {
            commonFunctionMenu->addSeparator();
        } else {
            QAction *action = commonFunctionMenu->addAction(i);
            action->setData(i);
            connect(action, SIGNAL(triggered()), SLOT(insertSql()));
        }
    }


    //QFile functions(":/txt/functionlist.txt");
    //functions.open(QIODevice::ReadOnly);

    QMenu *functioninsert = assistMenu->addMenu(tr("All SQL Functions"));
    //QByteArray functionLine;
    //while (!(functionLine = functions.readLine()).isEmpty()) {
    foreach(QString func, SQLTextEdit::loadFunctionList(m_database->driverName())) {
        //QString func(functionLine);
        if (func.startsWith(">")) {
            functioninsert->addSeparator();
            QAction *action = functioninsert->addAction(func.right(func.size()-1));
            action->setEnabled(false);
        } else {
            QAction *action = functioninsert->addAction(func.trimmed());
            action->setData(func.trimmed());
            connect(action, SIGNAL(triggered()), SLOT(insertSql()));
        }
    }

    QMenu *tables = assistMenu->addMenu(tr("Insert table name"));
    foreach(QString i, m_database->tables(QSql::AllTables)) {
        QAction *action = tables->addAction(i);
        action->setData(i);
        connect(action, SIGNAL(triggered()), SLOT(insertSql()));
    }

    QMenu *column = assistMenu->addMenu(tr("Insert column name"));
    foreach(QString i, m_database->tables(QSql::AllTables)) {
        QSqlRecord record = m_database->record(i);
        QMenu *one = column->addMenu(i);
        for(int j = 0; j < record.count(); j++) {
            QAction *action = one->addAction(record.fieldName(j));
            action->setData(record.fieldName(j));
            connect(action, SIGNAL(triggered()), SLOT(insertSql()));
        }
    }

    // History Menu
    historyMenu = new QMenu(this);

    // Other Menu
    menu = new QMenu(this);

    QAction* exportTable = menu->addAction(tr("Export Table"));
    connect(exportTable, SIGNAL(triggered()), SLOT(onExportTable()));
    m_menu_for_select.append(exportTable);

    QAction* createView = menu->addAction(tr("Create View for this table"));
    connect(createView, SIGNAL(triggered()), SLOT(onCreateView()));
    m_menu_for_select.append(createView);
}

void CustumSql::on_sql_textChanged()
{
    if (ui->sql->toPlainText().trimmed() == m_query.lastQuery()) {
        ui->sql->setStyleSheet("SQLTextEdit{ background: white; }");
    } else {
        ui->sql->setStyleSheet("SQLTextEdit{ background: #FAFFC5;}");
    }
}
