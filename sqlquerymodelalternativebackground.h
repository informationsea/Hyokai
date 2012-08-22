#ifndef SQLQUERYMODELALTERNATIVEBACKGROUND_H
#define SQLQUERYMODELALTERNATIVEBACKGROUND_H

#include <QSqlQueryModel>

class SqlQueryModelAlternativeBackground : public QSqlQueryModel
{
    Q_OBJECT
public:
    explicit SqlQueryModelAlternativeBackground(QObject *parent = 0);

    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
signals:
    
public slots:
    
};

#endif // SQLQUERYMODELALTERNATIVEBACKGROUND_H
