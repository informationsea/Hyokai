#ifndef SQLFILEEXPORTER_H
#define SQLFILEEXPORTER_H

#include <QObject>
#include <QString>
#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "filetype.h"

class SqlFileExporter : public QObject
{
    Q_OBJECT
public:
    explicit SqlFileExporter(QSqlDatabase *database, QWidget *parent = 0);

    bool exportTable(QSqlQuery query, const QString &outputpath, enum FileType fileType);
    QString errorMessage() {return m_errorMessage;}
    
signals:
    void progress(int progress);
    void progressMax(int progressMax);
    
public slots:

private:
    bool exportTableAsCSV(QSqlQuery query, const QString &outputpath, bool csv);
    //bool exportTableAsXLSX(QSqlQuery query, const QString &outputpath);

    QString quoteCSVColumn(QString column);

    QSqlDatabase *m_database;
    QWidget *m_parent;
    QString m_errorMessage;
};

#endif // SQLFILEEXPORTER_H
