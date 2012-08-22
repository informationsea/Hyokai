#include "custumsql.h"
#include "ui_custumsql.h"

#include "sheetmessagebox.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>

CustumSql::CustumSql(QSqlDatabase *database, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CustumSql), m_database(database),
    m_query(*database), m_querymodel(this)
{
    ui->setupUi(this);
    ui->tableView->setModel(&m_querymodel);
    ui->tableView->horizontalHeader()->setMovable(true);
    setWindowTitle(QFileInfo(database->databaseName()).baseName());
}

CustumSql::~CustumSql()
{
    delete ui;
}

void CustumSql::on_pushButton_clicked()
{
    if(ui->lineEdit->text().isEmpty())
        return;
    m_query.exec(ui->lineEdit->text());
    if (m_query.lastError().type() != QSqlError::NoError) {
        SheetMessageBox::critical(this, tr("SQL Error"), m_query.lastError().text()+"\n\n"+ui->lineEdit->text());
        return;
    }
    m_querymodel.setQuery(m_query);
}
