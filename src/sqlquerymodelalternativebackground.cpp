#include "sqlquerymodelalternativebackground.h"

#include <QColor>

SqlQueryModelAlternativeBackground::SqlQueryModelAlternativeBackground(QObject *parent) :
    QSqlQueryModel(parent)
{
}

QVariant SqlQueryModelAlternativeBackground::data(const QModelIndex & index, int role) const
{
    switch (role) {
    case Qt::DisplayRole: {
        QVariant d = QSqlQueryModel::data(index, role);
        if (d.type() == QVariant::String)
            return d.toString().split("\n").at(0);
        return d;
    }
    case Qt::ToolTipRole: {
        QVariant d = QSqlQueryModel::data(index, Qt::DisplayRole);
        return d;
    }
    case Qt::FontRole: {
        //return QFont("monaco");
    }
    }

    return QSqlQueryModel::data(index, role);
}
