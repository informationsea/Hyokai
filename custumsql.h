#ifndef CUSTUMSQL_H
#define CUSTUMSQL_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>

#include "sqlquerymodelalternativebackground.h"

namespace Ui {
class CustumSql;
}

class CustumSql : public QDialog
{
    Q_OBJECT
    
public:
    explicit CustumSql(QSqlDatabase *database, QWidget *parent = 0);
    ~CustumSql();
    
private slots:
    void on_pushButton_clicked();

private:
    Ui::CustumSql *ui;
    QSqlDatabase *m_database;
    QSqlQuery m_query;
    SqlQueryModelAlternativeBackground m_querymodel;
};

#endif // CUSTUMSQL_H
