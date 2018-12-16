#ifndef SQLITEMCOUNTTABLEMODEL_H
#define SQLITEMCOUNTTABLEMODEL_H

#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QSqlQuery>

class SqlItemCountTableModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    SqlItemCountTableModel(QSqlDatabase *database, QString fromValue, QString columnValue, QString where, QObject *parent = nullptr);

	void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
private:
	QSqlDatabase *m_database;
	QString m_fromValue;
	QString m_columnName;
	QString m_where;

	QSqlQuery m_query;
};

#endif // SQLITEMCOUNTTABLEMODEL_H
