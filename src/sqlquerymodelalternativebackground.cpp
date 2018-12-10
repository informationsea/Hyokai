#include "sqlquerymodelalternativebackground.h"

#include <QColor>

SqlQueryModelAlternativeBackground::SqlQueryModelAlternativeBackground(QObject *parent) :
    QSqlQueryModel(parent)
{
}

QVariant SqlQueryModelAlternativeBackground::data(const QModelIndex & index, int role) const
{
    return QSqlQueryModel::data(index, role);
}
