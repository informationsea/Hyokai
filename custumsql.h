#ifndef CUSTUMSQL_H
#define CUSTUMSQL_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QMenu>
#include <QTableView>

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

    QTableView *tableView();
    
private slots:
    void on_pushButton_clicked();
    void on_assistButton_clicked();

    void setSqlTemplate();
    void insertSql();
    void joinSqlWizard();

private:
    Ui::CustumSql *ui;
    QSqlDatabase *m_database;
    QSqlQuery m_query;
    SqlQueryModelAlternativeBackground m_querymodel;
    QMenu *assistMenu;
};

#endif // CUSTUMSQL_H
