#include "sqlfileimporter.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMap>
#include <QVariant>
#include <QByteArray>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QFileInfo>

#include <memory>
#include <ctype.h>
#include "tablereader.hpp"
#include "csvreader.hpp"
#include "sheetmessagebox.h"
#include "schemadialog.h"

#define SUGGEST_LINE 20

static bool isdigitstr(const char *str, size_t length)
{
    for (size_t i = 0; i < length; ++i) {
        if (!isdigit(*str++))
            return false;
    }
    return true;
}

static bool isrealstr(const char *str, size_t length)
{
    for (size_t i = 0; i < length; ++i) {
        char ch = *str++;
        if (!isdigit(ch) && ch != '.')
            return false;
    }
    return true;
}

SqlFileImporter::SqlFileImporter(QSqlDatabase *database, QObject *parent) :
    QObject(parent), m_database(database), m_canceled(false)
{
}

QList<SchemaField> SqlFileImporter::suggestSchema(QString path, FileType type, int skipLines, bool firstLineIsHeader, bool preferText)
{
    if (type == FILETYPE_SUGGEST) {
        if (path.toLower().endsWith(".csv"))
            type = FILETYPE_CSV;
        else
            type = FILETYPE_TVS;
    }

    TableReader *reader_raw;
    switch (type) {
    case FILETYPE_CSV:
        reader_raw = new CSVReader();
        break;
    case FILETYPE_TVS:
    default:
        reader_raw = new TableReader();
        break;
    }
    std::auto_ptr<TableReader> reader(reader_raw);
    if (!reader->open_path(path.toUtf8().data()))
        return QList<SchemaField>();

    QList<SchemaField> fields;
    QList<SchemaField::FieldType> fieldTypes;
    bool islineend;
    size_t readlen;

    for (int i = 0; i < skipLines; i++) {
        do {
            if (reader->readnext(&readlen, &islineend) == NULL)
                goto finishSkipLines;
        } while(!islineend);
    }
  finishSkipLines:

    if (firstLineIsHeader) {
        int j = 0;
        do {
            const char *column = reader->readnext(&readlen, &islineend);
            if (column == NULL)
                goto finishLineHeader;
            fields.append(SchemaField(SqlService::suggestFieldName(QString(QByteArray(column, readlen)), fields)));
            fields[j].setLogicalIndex(j);
            fieldTypes.append(SchemaField::FIELD_INTEGER);
            j++;
        } while (!islineend);
    }
  finishLineHeader:

    for (int i = 0; i < SUGGEST_LINE; i++) {
        int j = 0;
        do {
            const char *column = reader->readnext(&readlen, &islineend);
            if (column == NULL)
                goto finishSuggestFile;

            if (fields.size() <= j) {
                fields.append(SchemaField(SqlService::suggestFieldName(QString("V%1").arg(QString::number(j)), fields)));
                fieldTypes.append(SchemaField::FIELD_INTEGER);
            }

            switch (fieldTypes[j]) {
            case SchemaField::FIELD_INTEGER:
                if (isdigitstr(column, readlen))
                    break;
                fieldTypes[j] = SchemaField::FIELD_REAL;
                // no break
            case SchemaField::FIELD_REAL:
                if (isrealstr(column, readlen))
                    break;
                fieldTypes[j] = SchemaField::FIELD_TEXT;
                // no break
            case SchemaField::FIELD_TEXT:
            default:
                break;
            }

            if ((size_t)fields[j].maximumLength() < readlen)
                fields[j].setMaximumLength(readlen);

            j++;
        } while (!islineend);
    }
  finishSuggestFile:

    for (int i = 0; i < fields.size(); i++) {
        switch(fieldTypes[i]) {
        case SchemaField::FIELD_INTEGER:
            fields[i].setFieldType("INTEGER");
            fields[i].setIndexedField(true);
            break;
        case SchemaField::FIELD_REAL:
            fields[i].setFieldType("REAL");
            fields[i].setIndexedField(true);
            break;
        case SchemaField::FIELD_TEXT:
        default: {
            if (preferText) {
                fields[i].setFieldType("TEXT");
            } else {
                int round = ((int)(fields[i].maximumLength()*1.5/10))*10+10;
                fields[i].setFieldType(QString("varchar(%1)").arg(round));
            }

            if (fields[i].maximumLength() < 20) // make index for short text
                fields[i].setIndexedField(true);
            break;
        }
        }
    }

    return fields;
}

QStringList SqlFileImporter::createSql(const QString &name, const QList<SchemaField> &fields, bool usingFTS4)
{
    QStringList sqllist;
    QString createSql("CREATE TABLE ");
    bool isFirstIteration = true;
    bool hasPrimaryKey = false;

    if (usingFTS4) {
        createSql = "CREATE VIRTUAL TABLE ";
    }

    createSql += name;

    if (usingFTS4) {
        createSql += " USING fts4";
    }

    createSql += "(";
    foreach(const SchemaField oneField, fields) {
        if (isFirstIteration)
            isFirstIteration = false;
        else
            createSql += ",";
        createSql += oneField.name();
        createSql += " ";
        createSql += oneField.fieldType();

        if (oneField.isPrimaryKey())
            hasPrimaryKey = true;
    }
    if (hasPrimaryKey) {
        createSql += ", PRIMARY KEY(";
        isFirstIteration = true;

        foreach(const SchemaField oneField, fields) {
            if (oneField.isPrimaryKey()) {
                if (isFirstIteration)
                    isFirstIteration = false;
                else
                    createSql += ",";
                createSql += oneField.name();
            }
        }
        createSql += ")";
    }
    createSql += ");";

    sqllist << createSql;


    foreach (const SchemaField oneField, fields) {
        if (!oneField.indexedField()) continue;
        sqllist << QString("CREATE INDEX %1__%2__index on %1(%2)").arg(name, oneField.name());
    }

    return sqllist;
}

bool SqlFileImporter::createTablesAndIndexes(const QString &name, const QList<SchemaField> &fields, bool useFts4)
{
    m_errorMessage = "";

    QStringList sqls(createSql(name, fields, useFts4));
    foreach (QString onesql, sqls) {
        QSqlQuery query = m_database->exec(onesql);
        if (query.lastError().type() != QSqlError::NoError) {
            m_errorMessage = query.lastError().text();
            return false;
        }
    }
    return true;
}

bool SqlFileImporter::importFile(QString path, const QString &name, const QList<SchemaField> &fields, FileType type, int skipLines, bool firstLineIsHeader, volatile bool *canceledFlag)
{
    m_errorMessage = "";
    m_canceled = false;
    if (firstLineIsHeader)
        skipLines++;

    if (type == FILETYPE_SUGGEST) {
        if (path.toLower().endsWith(".csv"))
            type = FILETYPE_CSV;
        else
            type = FILETYPE_TVS;
    }

    TableReader *reader_raw;
    switch (type) {
    case FILETYPE_CSV:
        reader_raw = new CSVReader();
        break;
    case FILETYPE_TVS:
    default:
        reader_raw = new TableReader();
        break;
    }
    std::auto_ptr<TableReader> reader(reader_raw);
    if (!reader->open_path(path.toUtf8().data())) {
        m_errorMessage = "Cannot open file.";
        return false;
    }

    bool islineend;
    size_t readlen;

    for (int i = 0; i < skipLines; i++) {
        do {
            if (reader->readnext(&readlen, &islineend) == NULL)
                goto finishSkipLines;
        } while(!islineend);
    }
  finishSkipLines:

    QStringList placeholders;
    for (int i = 0; i < fields.size(); ++i) {
        placeholders << QString(":V%1").arg(QString::number(i));
    }
    QSqlQuery query(*m_database);
    query.prepare(QString("INSERT INTO %1 VALUES(%2)").arg(name, placeholders.join(",")));

    QMap<int, QString> logical2sqlindex;
    for (int i = 0; i < fields.size(); ++i) {
        logical2sqlindex[fields[i].logicalIndex()] = placeholders[i];
    }

    int currentLine = 0;
    do {
        islineend = false;
        for (int columnNumber = 0; !islineend; columnNumber++) {
            const char *column = reader->readnext(&readlen, &islineend);
            if (column == NULL) goto finishImport;
            if (!logical2sqlindex.contains(columnNumber)) continue;
            QVariant data(QByteArray(column, readlen));
            query.bindValue(logical2sqlindex[columnNumber], data);
        }
        query.exec();
        if (query.lastError().type() != QSqlError::NoError) {
            m_errorMessage = query.lastError().text();
            return false;
        }

        if ((currentLine % 1000) == 0) {
            if (canceledFlag && *canceledFlag) {
                m_errorMessage = tr("Canceled by user");
                return false;
            }
            emit progress(reader->tell());
        }

        currentLine++;
    } while(1);
  finishImport:
    return true;
}

QString SqlFileImporter::errorMessage()
{
    return m_errorMessage;
}


SqlAsynchronousFileImporter::SqlAsynchronousFileImporter(QSqlDatabase *database, QWidget *parent) :
    QThread(parent), m_database(database), m_parent(parent), m_progress(parent)
{
    m_withError = false;
    connect(this, SIGNAL(updateProgressLabelText(QString)), &m_progress, SLOT(setLabelText(QString)));
    connect(this, SIGNAL(updateProgressValue(int)), &m_progress, SLOT(setValue(int)));
}

SqlAsynchronousFileImporter::~SqlAsynchronousFileImporter()
{
    foreach(SchemaDialog *dialog, m_schemaList) {
        delete dialog;
    }
}

void SqlAsynchronousFileImporter::executeImport(QStringList files)
{
    QMessageBox::StandardButton button;
    if (files.size() > 1) {
        button = SheetMessageBox::question(m_parent, tr("Multiple files are selected"), tr("Do you want to import with default options?"),
                                           QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
    } else {
        button = QMessageBox::No;
    }

    qint64 sumsize = 0;
    foreach(QString path, files) {
        QFileInfo onefileinfo(path);
        m_schemaList << new SchemaDialog(m_database, path, m_parent);
        m_filesizes << onefileinfo.size();
        sumsize += m_filesizes.last();
        m_schemaList.last()->setName(SqlService::suggestTableName(onefileinfo.completeBaseName(), m_database));
        m_schemaList.last()->setFileType(path.endsWith(".csv") ? SqlFileImporter::FILETYPE_CSV : SqlFileImporter::FILETYPE_TVS);
        m_schemaList.last()->setFields(SqlFileImporter::suggestSchema(path, SqlFileImporter::FILETYPE_SUGGEST, 0, true, m_database->driverName() == "QSQLITE"));
        if (button == QMessageBox::No && m_schemaList.last()->exec() != QDialog::Accepted)
            return;
    }

    //connect(this, SIGNAL(finish(QStringList,bool,QString)), SLOT(finishThread()), Qt::QueuedConnection);
    connect(this, SIGNAL(finished()), SLOT(finishThread()));

    m_progress.open(this, SLOT(canceled()));
    m_progress.setMaximum(sumsize);
    m_progress.setValue(0);
    //connect(&m_progress, SIGNAL(canceled()), SLOT(canceled()));

    start();
}

void SqlAsynchronousFileImporter::run()
{
    qDebug() << "Async file importer started";
    m_prossingIndex = 0;
    m_importedTables.clear();
    m_errorMessage.clear();
    m_canceled = false;
    m_withError = false;
    m_database->transaction();
    foreach (SchemaDialog *dialog, m_schemaList) {
        emit updateProgressLabelText(QString("Importing %1...").arg(dialog->name()));
        SqlFileImporter importer(m_database);
        connect(&importer, SIGNAL(progress(long)), this, SLOT(importProgressUpdate(long)));

        if (!importer.createTablesAndIndexes(dialog->name(), dialog->fields(), dialog->useFts4())) {
            m_errorMessage = importer.errorMessage();
            goto onerror;
        }
        if (!importer.importFile(dialog->fileName(), dialog->name(), dialog->fields(), dialog->fileType(), dialog->skipLines(), dialog->firstLineIsHeader(), &m_canceled)) {
            m_errorMessage = importer.errorMessage();
            goto onerror;
        }
        m_importedTables << dialog->name();
        m_prossingIndex += 1;
        if (m_canceled) {
            m_errorMessage = tr("Canceled by user");
            goto onerror;
        }

        off_t processedSize = 0;
        for (int i = 0; i < m_prossingIndex; ++i)
            processedSize += m_filesizes[i];
        if (!m_canceled)
            emit updateProgressValue(processedSize);
    }
    qDebug() << "imported" << m_importedTables;
    //emit finish(m_importedTables, false, "");
    m_database->commit();
    m_withError = false;
    exit(0);
    return;
  onerror:
    qDebug() << "Error" << m_errorMessage;
    //emit finish(QStringList(), true, m_errorMessage);
    m_database->rollback();
    m_withError = true;
    exit(1);
    return;
}

void SqlAsynchronousFileImporter::finishThread()
{
    qDebug() << "finish thread";
    //m_progress.setValue(m_progress.maximum());
    if (m_progress.isVisible())
        m_progress.close();
    emit finish(m_importedTables, m_withError, m_errorMessage);
}

void SqlAsynchronousFileImporter::canceled()
{
    qDebug() << "Canceled";
    m_canceled = true;
}

void SqlAsynchronousFileImporter::importProgressUpdate(long value)
{
    if (m_canceled)
        return;
    off_t processedSize = 0;
    for (int i = 0; i < m_prossingIndex; ++i)
        processedSize += m_filesizes[i];
    processedSize += value;
    emit updateProgressValue(processedSize);
}
