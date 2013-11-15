#ifndef SQLFILEIMPORTER_H
#define SQLFILEIMPORTER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QList>
#include <QStringList>
#include <QThread>
#include <QProgressDialog>

#include "sqlservice.h"

class SchemaDialog;

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
    static QString generateCreateTableSql(const QString &name, const QList<SchemaField> &fields, bool useFts4);
    static QStringList generateCreateIndexSql(const QString &name, const QList<SchemaField> &fields);
    bool createTables(const QString &name, const QList<SchemaField> &fields, bool useFts4);
    bool createIndexes(const QString &name, const QList<SchemaField> &fields);
    bool importFile(QString path, const QString &name, const QList<SchemaField> &fields, enum FileType type, int skipLines, bool firstLineIsHeader, volatile bool *canceledFlag = 0);
    QString errorMessage();
    
signals:
    void progress(long progress);
    //void progressMax(long progressMax);
    
public slots:

private:
    QSqlDatabase *m_database;
    bool m_canceled;
    QString m_errorMessage;
};

class SqlAsynchronousFileImporter : public QThread
{
    Q_OBJECT
public:
    SqlAsynchronousFileImporter(QSqlDatabase *database, QWidget *parent = 0);
    virtual ~SqlAsynchronousFileImporter();

    void executeImport(QStringList files);
    virtual void run();

signals:
    void finish(QStringList importedTables, bool withError, QString errorMessage);
    void updateProgressLabelText(QString file);
    void updateProgressValue(int value);

private slots:
    void finishThread();
    void canceled();
    void importProgressUpdate(long value);

private:
    QSqlDatabase *m_database;
    QWidget *m_parent;
    QList<SchemaDialog *> m_schemaList;
    QProgressDialog m_progress;
    QList<off_t> m_filesizes;

    QStringList m_importedTables;
    QString m_errorMessage;
    int m_prossingIndex;
    volatile bool m_canceled;
    bool m_withError;
};

#endif // SQLFILEIMPORTER_H
