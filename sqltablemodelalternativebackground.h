#ifndef SQLTABLEMODELALTERNATIVEBACKGROUND_H
#define SQLTABLEMODELALTERNATIVEBACKGROUND_H

#include <QSqlTableModel>

class SqlTableModelAlternativeBackground : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit SqlTableModelAlternativeBackground(QObject *parent = 0, QSqlDatabase db = QSqlDatabase() );
    
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
signals:
    
public slots:
    
};

#endif // SQLTABLEMODELALTERNATIVEBACKGROUND_H
