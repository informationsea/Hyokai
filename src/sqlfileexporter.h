#ifndef SQLFILEEXPORTER_H
#define SQLFILEEXPORTER_H

#include <QObject>
#include <QString>
#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>

class SqlFileExporter : public QObject
{
    Q_OBJECT
public:
    explicit SqlFileExporter(QSqlDatabase *database, QWidget *parent = 0);

    bool exportTable(QSqlQuery query, const QString &outputpath, bool csv);
    QString errorMessage() {return m_errorMessage;}
    
signals:
    void progress(int progress);
    void progressMax(int progressMax);
    
public slots:

private:
    QString quoteCSVColumn(QString column);

    QSqlDatabase *m_database;
    QWidget *m_parent;
    QString m_errorMessage;
};

#endif // SQLFILEEXPORTER_H
