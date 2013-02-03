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
        if (isDirty(index)) {
            if (index.row() %2)
                return QVariant(QColor("#FFF4CF"));
            else
                return QVariant(QColor("#FFFAE9"));
        }
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
    beginResetModel();
    clear();
    if (database().driverName() == "QSQLITE") {
        QSqlTableModel::setTable(addQuote(tableName));
    } else {
        QSqlTableModel::setTable(tableName);
    }

    m_view = false;
    foreach(QString view, database().tables(QSql::Views)) {
        if (view == plainTableName()) {
            m_view = true;
            break;
        }
    }
    endResetModel();
}

QString SqlTableModelAlternativeBackground::plainTableName() const
{
    return removeQuote(QSqlTableModel::tableName());
}

long long SqlTableModelAlternativeBackground::sqlRowCount()
{
    QSqlQuery count;
    if (filter().isEmpty()) {
        count = database().exec(QString("SELECT count(*) FROM %1;").arg(tableName()));
    } else {
        count = database().exec(QString("SELECT count(*) FROM %1 WHERE %2;").arg(tableName(), filter()));
    }

    count.next();
    return count.value(0).toLongLong();
}
