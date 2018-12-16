#include "sqlitemcounttablemodel.h"

SqlItemCountTableModel::SqlItemCountTableModel(QSqlDatabase *database, QString fromValue, QString columnName, QString where, QObject *parent)
	: QSqlQueryModel(parent), m_database(database), m_fromValue(fromValue), m_columnName(columnName), m_where(where)
{
	// Summary
	if (m_where.isEmpty()) {
		m_query = QSqlQuery(QString("SELECT \"%1\", count(*) as count FROM %2 GROUP BY \"%1\" ORDER BY 2 DESC").arg(columnName, fromValue), *m_database);
	}
	else {
		m_query = QSqlQuery(QString("SELECT \"%1\", count(*) as count FROM %2 WHERE %3 GROUP BY \"%1\" ORDER BY 2 DESC").arg(columnName, fromValue, m_where), *m_database);
	}
	this->setQuery(m_query);
}

void SqlItemCountTableModel::sort(int column, Qt::SortOrder order)
{
	QString orderStr;
	if (order == Qt::SortOrder::AscendingOrder) {
		orderStr = "ASC";
	}
	else {
		orderStr = "DESC";
	}

	// Summary
	if (m_where.isEmpty()) {
		m_query = QSqlQuery(QString("SELECT \"%1\", count(*) as count FROM %2 GROUP BY \"%1\" ORDER BY %3 %4").
			arg(m_columnName, m_fromValue, QString::number(column + 1), orderStr), *m_database);
	}
	else {
		m_query = QSqlQuery(QString("SELECT \"%1\", count(*) as count FROM %2 WHERE %3 GROUP BY \"%1\" ORDER BY %3 %4").
			arg(m_columnName, m_fromValue, m_where, QString::number(column + 1), orderStr), *m_database);
	}
	this->setQuery(m_query);
}