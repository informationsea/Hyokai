#include "sqltablemodelalternativebackground.h"

#include <QColor>

SqlTableModelAlternativeBackground::SqlTableModelAlternativeBackground(QObject *parent, QSqlDatabase db) :
    QSqlTableModel(parent, db)
{
}

QVariant SqlTableModelAlternativeBackground::data(const QModelIndex & index, int role) const
{
    if (role == Qt::BackgroundRole) {
        if (index.row() % 2)
            return QVariant(QColor("#E8EDF5"));
    }
    return QSqlTableModel::data(index, role);
}
