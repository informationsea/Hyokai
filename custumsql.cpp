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

#define CUSTUM_SQL_HISTORY "CUSTUM_SQL_HISTORY"

CustumSql::CustumSql(QSqlDatabase *database, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CustumSql), m_database(database),
    m_query(*database), m_querymodel(this)
{
    ui->setupUi(this);
    ui->tableView->setModel(&m_querymodel);
    ui->tableView->horizontalHeader()->setMovable(true);
    setWindowTitle(QFileInfo(database->databaseName()).baseName());

    assistMenu = new QMenu(this);

    QStringList templateList;
    templateList << "SELECT * FROM table";
    templateList << "SELECT * FROM table1 INNTER JOIN table2 ON table1.column1 = table2.column2";
    templateList << "SELECT * FROM table1 LEFT OUTER JOIN table2 ON table1.column1 = table2.column2";
    templateList << "SELECT * FROM table1 NATUAL INNTER JOIN table2";
    templateList << "SELECT * FROM table1 NATUAL LEFT OUTER JOIN table2";
    templateList << "VACUUM";

    QAction *joinwizard = assistMenu->addAction(tr("SQL Join Wizard"));
    connect(joinwizard, SIGNAL(triggered()), SLOT(joinSqlWizard()));

    QMenu *sqltemplates = assistMenu->addMenu(tr("Replace with SQL Templates"));
    foreach(QString i, templateList) {
        QAction *action = sqltemplates->addAction(i);
        action->setData(i);
        connect(action, SIGNAL(triggered()), SLOT(setSqlTemplate()));
    }

    QStringList templateList2;
    templateList2 << " ORDER BY column ASC";
    templateList2 << " ORDER BY column DESC";
    templateList2 << " WHERE column == 0";
    templateList2 << " WHERE column < 0";
    templateList2 << " WHERE column > 0";
    templateList2 << " WHERE column LIKE pattern";
    templateList2 << " WHERE column GLOB pattern";
    templateList2 << " LIMIT limit";
    templateList2 << " OFFSET offset";
    templateList2 << " GROUP BY column";

    QMenu *sqltemplateinsert = assistMenu->addMenu(tr("Insert SQL Templates"));
    foreach(QString i, templateList2) {
        QAction *action = sqltemplateinsert->addAction(i);
        action->setData(i);
        connect(action, SIGNAL(triggered()), SLOT(insertSql()));
    }

    QStringList functions;
    functions << "avg(";
    functions << "distinct(";
    functions << "max(";
    functions << "min(";
    functions << "count(";
    functions << "total(";
    functions << "--";
    functions << "stdev(";
    functions << "variance(";
    functions << "median(";
    functions << "lower_quartile(";
    functions << "upper_quartile(";

    QMenu *functioninsert = assistMenu->addMenu(tr("SQL Functions"));
    foreach(QString i, functions) {
        if (i == "--") {
            functioninsert->addSeparator();
            continue;
        }
        QAction *action = functioninsert->addAction(i);
        action->setData(i);
        connect(action, SIGNAL(triggered()), SLOT(insertSql()));
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

    QStringList list = tableview_settings->value(CUSTUM_SQL_HISTORY).toStringList();
    foreach(QString i, list) {
        ui->sql->addItem(i);
    }
}

CustumSql::~CustumSql()
{
    delete ui;
    delete assistMenu;
}

QTableView *CustumSql::tableView()
{
    return ui->tableView;
}

void CustumSql::on_pushButton_clicked()
{
    if(ui->sql->lineEdit()->text().isEmpty())
        return;
    m_query.exec(ui->sql->lineEdit()->text());
    if (m_query.lastError().type() != QSqlError::NoError) {
        SheetMessageBox::critical(this, tr("SQL Error"), m_query.lastError().text()+"\n\n"+m_query.lastQuery());
        return;
    }


    QStringList list = tableview_settings->value(CUSTUM_SQL_HISTORY).toStringList();
    if (list.isEmpty() || list.first() != m_query.lastQuery()) {
        list.insert(0, m_query.lastQuery());
        list.removeDuplicates();
        tableview_settings->setValue(CUSTUM_SQL_HISTORY, list);
    }
    ui->sql->clear();
    foreach(QString i, list) {
        ui->sql->addItem(i);
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
    ui->sql->lineEdit()->setText(sender->data().toString());
}


void CustumSql::insertSql()
{
    QAction *sender = (QAction *)QObject::sender();
    ui->sql->lineEdit()->insert(sender->data().toString());
}

void CustumSql::joinSqlWizard()
{
    JoinTableDialog dialog(m_database, this);
    if (dialog.exec() != QDialog::Accepted)
        return;
    ui->sql->lineEdit()->setText(dialog.sql());
    on_pushButton_clicked();
}
