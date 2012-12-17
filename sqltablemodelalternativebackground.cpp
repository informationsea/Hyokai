#include "sqltablemodelalternativebackground.h"

#include <QColor>
#include "main.h"

SqlTableModelAlternativeBackground::SqlTableModelAlternativeBackground(QObject *parent, QSqlDatabase db) :
    QSqlTableModel(parent, db), m_editable(true), m_view(false)
{
}

QVariant SqlTableModelAlternativeBackground::data(const QModelIndex & index, int role) const
{
    switch (role) {
    case Qt::BackgroundRole:
        if (index.row() % 2)
            return QVariant(QColor("#E8EDF5"));
        break;
    }

    return QSqlTableModel::data(index, role);
}

Qt::ItemFlags SqlTableModelAlternativeBackground::flags(const QModelIndex &index) const
{
    Qt::ItemFlags original = QSqlTableModel::flags(index);
    if (!m_editable || m_view) {
        return original & ~(Qt::ItemIsEditable);
    }
    return original;
}

void SqlTableModelAlternativeBackground::setTable(const QString &tableName)
{
    QSqlTableModel::setTable(addQuote(tableName));

    m_view = false;
    foreach(QString view, database().tables(QSql::Views)) {
        if (view == plainTableName()) {
            m_view = true;
            break;
        }
    }
}

QString SqlTableModelAlternativeBackground::plainTableName() const
{
    return removeQuote(QSqlTableModel::tableName());
}
