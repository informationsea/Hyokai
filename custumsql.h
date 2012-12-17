#ifndef CUSTUMSQL_H
#define CUSTUMSQL_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QMenu>
#include <QTableView>
#include <QStringList>
#include <QList>

#include "sqlquerymodelalternativebackground.h"

#define CUSTUM_SQL_HISTORY "CUSTUM_SQL_HISTORY"

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

    void on_historyButton_clicked();
    void onHistorySelected();

    void on_menuButton_clicked();
    void onExportTable();
    void onCreateView();

    void on_sql_textChanged();

private:
    void createMenus();

    Ui::CustumSql *ui;
    QSqlDatabase *m_database;
    QSqlQuery m_query;
    SqlQueryModelAlternativeBackground m_querymodel;
    QStringList m_history;

    QMenu *assistMenu;
    QMenu *menu;
    QMenu *historyMenu;
    QList<QAction *> m_menu_for_select;
};

#endif // CUSTUMSQL_H
