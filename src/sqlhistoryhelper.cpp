#include "sqlhistoryhelper.h"

#include <QSqlQuery>
#include <QString>
#include <QSqlError>
#include <QDebug>

#include "sheetmessagebox.h"
#include "main.h"

#define HISTORY_TABLE_SQL "CREATE TABLE IF NOT EXISTS Hyokai_SQL_History(sql TEXT, table_name TEXT, type TEXT, created_at TEXT DEFAULT (datetime('now')))"

SqlHistoryHelper::SqlHistoryHelper(QSqlDatabase *database, QWidget *parent) : QObject(parent), m_database(database), m_parent(parent)
{

}

void SqlHistoryHelper::insertFilterHistory(QString tableName, QString sql)
{
    insertHistory("FILTER", tableName, sql);
}

void SqlHistoryHelper::insertCustomSqlHistory(QString sql)
{
    insertHistory("CUSTOM", QString::null, sql);
}

QStringList SqlHistoryHelper::filterHistory(QString tableName)
{
    QSqlQuery filterQuery(*m_database);
    filterQuery.prepare(QString("SELECT sql FROM Hyokai_SQL_History WHERE type = 'FILTER' AND table_name = \"%1\" GROUP BY sql ORDER BY max(created_at) DESC LIMIT 40").arg(tableName));
    filterQuery.exec();

    if (filterQuery.lastError().type() != QSqlError::NoError)
        qDebug() << "History Query" << filterQuery.lastError();

    QStringList filters;
    while (filterQuery.next()) {
        filters.append(filterQuery.value(0).toString());
    }

    return filters;
}

QStringList SqlHistoryHelper::customHistory()
{
    QSqlQuery filterQuery(*m_database);
    filterQuery.prepare("SELECT sql FROM Hyokai_SQL_History WHERE type = 'CUSTOM' GROUP BY sql ORDER BY max(created_at) DESC LIMIT 40");
    filterQuery.exec();

    if (filterQuery.lastError().type() != QSqlError::NoError)
        qDebug() << "History Query" << filterQuery.lastError();

    QStringList filters;
    while (filterQuery.next()) {
        filters.append(filterQuery.value(0).toString());
    }

    return filters;
}

void SqlHistoryHelper::insertHistory(QString type, QString tableName, QString sql) {
    if (!tableview_settings->value(CREATE_SQL_HISTORY_TABLE).toBool()) return;

    QSqlQuery createTable = m_database->exec(HISTORY_TABLE_SQL);
    if (createTable.lastError().type() != QSqlError::NoError) {
        SheetMessageBox::warning(m_parent, tr("Cannot create history table"), createTable.lastError().text());
        return;
    }

    QSqlQuery query(*m_database);
    query.prepare("INSERT INTO Hyokai_SQL_History(sql, table_name, type) VALUES (:sql, :table_name, :type)");
    query.bindValue(":sql", sql);
    query.bindValue(":table_name", tableName);
    query.bindValue(":type", type);
    query.exec();
    if (query.lastError().type() != QSqlError::NoError) {
        SheetMessageBox::warning(m_parent, tr("Cannot insert history"), query.lastError().text());
        return;
    }
}
