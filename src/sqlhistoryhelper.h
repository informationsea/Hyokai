#ifndef SQLHISTORYHELPER_H
#define SQLHISTORYHELPER_H

#include <QObject>
#include <QSqlDatabase>
#include <QStringList>

class SqlHistoryHelper : public QObject
{
    Q_OBJECT
public:
    explicit SqlHistoryHelper(QSqlDatabase *database, QWidget *parent = 0);

    void insertFilterHistory(QString tableName, QString sql);
    void insertCustomSqlHistory(QString sql);

    QStringList filterHistory(QString tableName);
    QStringList customHistory();

signals:

public slots:

private:
    QSqlDatabase *m_database;
    QWidget *m_parent;

    void insertHistory(QString type, QString tableName, QString sql) ;
};

#endif // SQLHISTORYHELPER_H
