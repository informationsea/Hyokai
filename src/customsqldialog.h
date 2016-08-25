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
#include "checkboxitemdelegate.h"

#define CUSTOM_SQL_HISTORY "CUSTUM_SQL_HISTORY"

class SqlHistoryHelper;
class SqlAsynchronousExecutor;

namespace Ui {
class CustomSql;
}

class CustomSqlDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit CustomSqlDialog(QSqlDatabase *database, QWidget *parent = 0);
    ~CustomSqlDialog();

    QTableView *tableView();

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

public slots:
    void finishQuery(QSqlQuery *query, SqlAsynchronousExecutor *executor);
    void selectTableAll();
    
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
    void onExportToR();
    void onFixedFontToggled(bool);
    void showCell();

    void onShowSummary();
    void onSortAscending();
    void onSortDescending();

    void on_sql_textChanged();

private:
    void createMenus();

    Ui::CustomSql *ui;
    QSqlDatabase *m_database;
    QSqlQuery m_query;
    SqlQueryModelAlternativeBackground m_querymodel;
    QStringList m_history;
    SqlHistoryHelper *m_historyHelper;
    SqlAsynchronousExecutor *m_asynchronousExecutor;

    QMenu *assistMenu;
    QMenu *menu;
    QMenu *historyMenu;
    QList<QAction *> m_menu_for_select;
};

#endif // CUSTUMSQL_H
