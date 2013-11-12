#include "sqltablemodelalternativebackground.h"

#include <QColor>
#include <QFont>
#include "main.h"

SqlTableModelAlternativeBackground::SqlTableModelAlternativeBackground(QObject *parent, QSqlDatabase db) :
    QSqlTableModel(parent, db), m_editable(true), m_view(false)
{
}

QVariant SqlTableModelAlternativeBackground::data(const QModelIndex & index, int role) const
{
    switch (role) {
    case Qt::DisplayRole: {
        QVariant d = QSqlTableModel::data(index, role);
        if (d.type() == QVariant::String)
            return d.toString().split("\n").at(0);
        return d;
    }
    case Qt::FontRole: {
        //return QFont("monaco");
    }
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
