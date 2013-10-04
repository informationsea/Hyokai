#ifndef SQLFILEIMPORTER_H
#define SQLFILEIMPORTER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QList>
#include <QStringList>

#include "sqlservice.h"



class SqlFileImporter : public QObject
{
    Q_OBJECT
public:
    enum FileType {
        FILETYPE_SUGGEST,
        FILETYPE_TVS,
        FILETYPE_CSV
    };

    explicit SqlFileImporter(QSqlDatabase *database, QObject *parent = 0);

    static QList<SchemaField> suggestSchema(QString path, enum FileType type, int skipLines, bool firstLineIsHeader, bool preferText);
    static QStringList createSql(const QString &name, const QList<SchemaField> &fields, bool useFts4);
    bool importFile(QString path, const QString &name, const QList<SchemaField> &fields, enum FileType type, int skipLines, bool firstLineIsHeader);
    
signals:
    void progress(long progress);
    void progressMax(long progressMax);
    
public slots:

private:
    QSqlDatabase *m_database;
    bool m_canceled;
};

#endif // SQLFILEIMPORTER_H
