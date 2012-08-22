#include "sqlquerymodelalternativebackground.h"

#include <QColor>

SqlQueryModelAlternativeBackground::SqlQueryModelAlternativeBackground(QObject *parent) :
    QSqlQueryModel(parent)
{
}

QVariant SqlQueryModelAlternativeBackground::data(const QModelIndex & index, int role) const
{
    if (role == Qt::BackgroundRole) {
        if (index.row() % 2)
            return QVariant(QColor("#E8EDF5"));
    }
    return QSqlQueryModel::data(index, role);
}
