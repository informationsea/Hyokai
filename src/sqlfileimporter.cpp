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

//#include "qtxlsx/src/xlsx/xlsxdocument.h"

#define SUGGEST_LINE 20

static bool isdigitstr(const char *str, size_t length)
{
    bool isOk = false;
    QString::fromUtf8(str, length).toInt(&isOk);

    return isOk;
}

static bool isrealstr(const char *str, size_t length)
{
    bool isOk = false;
    QString::fromUtf8(str, length).toDouble(&isOk);

    return isOk;
}

SqlFileImporter::SqlFileImporter(QSqlDatabase *database, QObject *parent) :
    QObject(parent), m_database(database), m_canceled(false)
{
}

QList<SchemaField> SqlFileImporter::suggestSchema(QString path, FileType type, int skipLines, bool firstLineIsHeader, bool preferText)
{
    if (type == FILETYPE_SUGGEST)
        type = FileTypeUtil::getFileTypeFromPath(path);

    switch (type) {
    case FILETYPE_CSV:
    case FILETYPE_TVS:
    default:
        return suggestSchemaFromCSV(path, type == FILETYPE_CSV, skipLines, firstLineIsHeader, preferText);
    //case FILETYPE_XLSX:
    //    return suggestSchemaFromXLSX(path, skipLines, firstLineIsHeader, preferText);

    }

}

QList<SchemaField> SqlFileImporter::suggestSchemaFromCSV(QString path, bool isCSV, int skipLines, bool firstLineIsHeader, bool preferText)
{
    std::auto_ptr<TableReader> reader(isCSV ? new CSVReader() : new TableReader());
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
                fields[j].setLogicalIndex(j);
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

QString SqlFileImporter::generateCreateTableSql(const QString &name, const QList<SchemaField> &fields, bool usingFTS4)
{
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

    return createSql;
}

QStringList SqlFileImporter::generateCreateIndexSql(const QString &name, const QList<SchemaField> &fields)
{
    QStringList sqllist;

    foreach (const SchemaField oneField, fields) {
        if (!oneField.indexedField()) continue;
        sqllist << QString("CREATE INDEX %1__%2__index on %1(%2)").arg(name, oneField.name());
    }

    return sqllist;
}

bool SqlFileImporter::createTables(const QString &name, const QList<SchemaField> &fields, bool useFts4)
{
    m_errorMessage = "";

    QSqlQuery query = m_database->exec(generateCreateTableSql(name, fields, useFts4));
    if (query.lastError().type() != QSqlError::NoError) {
        m_errorMessage = query.lastError().text();
        return false;
    }

    return true;
}

bool SqlFileImporter::createIndexes(const QString &name, const QList<SchemaField> &fields)
{
    m_errorMessage = "";

    QStringList sqls(generateCreateIndexSql(name, fields));
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

    if (type == FILETYPE_SUGGEST)
        type = FileTypeUtil::getFileTypeFromPath(path);

    switch (type) {
    case FILETYPE_CSV:
    case FILETYPE_TVS:
        return importCSVFile(path, name, fields, type == FILETYPE_CSV, skipLines, canceledFlag);

    //case FILETYPE_XLSX:
    //    return importXLSXFile(path, name, fields, skipLines, canceledFlag);

    default:
        m_errorMessage = tr("Unsupported file type is specified. (%1)").arg(type);
        return false;
    }

}

bool SqlFileImporter::importCSVFile(QString path, const QString &name, const QList<SchemaField> &fields, bool isCSV, int skipLines, volatile bool *canceledFlag)
{
    std::auto_ptr<TableReader> reader(isCSV ? new CSVReader : new TableReader);
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
        // clear column data
        foreach (QString one, logical2sqlindex.values()) {
            query.bindValue(one, QVariant());
        }

        // bind column data
        for (int columnNumber = 0; !islineend; columnNumber++) {
            const char *column = reader->readnext(&readlen, &islineend);
            if (column == NULL) goto finishImport;
            if (!logical2sqlindex.contains(columnNumber)) continue;

            QVariant data(QString(QByteArray(column, readlen)));
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

/*
bool SqlFileImporter::importXLSXFile(QString path, const QString &name, const QList<SchemaField> &fields, int skipLines, volatile bool *canceledFlag)
{
    //
    QStringList placeHolders;
    for (int i = 0; i < fields.size(); i++)
        placeHolders << QString(":V%1").arg(i);

    QSqlQuery query(*m_database);
    query.prepare(QString("INSERT INTO %1 VALUES(%2)").arg(name, placeHolders.join(",")));

    QMap<int, QString> logical2sqlindex;
    for (int i = 0; i < fields.size(); ++i)
        logical2sqlindex[fields[i].logicalIndex()] = placeHolders[i];

    //
    QXlsx::Document doc(path);

    //
    for (int row = skipLines; ; row++) {
        //
        bool allInvalidFlag = true;
        for (int col = 0; col < fields.size(); col++) {
            const QVariant value = getValueFromXLSXDocument(doc, row + 1, col + 1);
            query.bindValue(logical2sqlindex[col], value);
            allInvalidFlag &= !value.isValid();
        }

        if (allInvalidFlag) break;

        //
        query.exec();
        if (query.lastError().type() != QSqlError::NoError) {
            m_errorMessage = query.lastError().text();
            return false;
        }

        //
        if (row % 1000 == 0) {
            if (canceledFlag && *canceledFlag) {
                m_errorMessage = tr("Canceled by user");
                return false;
            }

            // TODO: tell progress
        }
    }

    return true;
}
*/

QString SqlFileImporter::errorMessage()
{
    return m_errorMessage;
}


SqlAsynchronousFileImporter::SqlAsynchronousFileImporter(QSqlDatabase *database, QWidget *parent) :
    QThread(parent), m_database(database), m_parent(parent), m_progress(0)
{
    m_withError = false;
}

SqlAsynchronousFileImporter::~SqlAsynchronousFileImporter()
{
    foreach(SchemaDialog *dialog, m_schemaList) {
        delete dialog;
    }

    delete m_progress;
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
        m_schemaList.last()->setFileType(FileTypeUtil::getFileTypeFromPath(path));

        QList<SchemaField> schema;
        if (path.endsWith(".bed") || path.endsWith(".bed.gz")) {
            schema = SqlFileImporter::suggestSchema(path, FILETYPE_SUGGEST, 0, false, m_database->driverName() == "QSQLITE");
            m_schemaList.last()->setFirstLineIsHeader(false);

            for (int i = 0; i < 6 && i < schema.length(); i++) {
                switch (i) {
                case 0:
                    schema[i].setName("chr");
                    break;
                case 1:
                    schema[i].setName("start");
                    break;
                case 2:
                    schema[i].setName("end");
                    break;
                case 3:
                    schema[i].setName("name");
                    break;
                case 4:
                    schema[i].setName("score");
                    break;
                case 5:
                    schema[i].setName("strand");
                    break;
                }
            }
        } else {
            schema = SqlFileImporter::suggestSchema(path, FILETYPE_SUGGEST, 0, true, m_database->driverName() == "QSQLITE");
        }

        m_schemaList.last()->setFields(schema);
        if (button == QMessageBox::No && m_schemaList.last()->exec() != QDialog::Accepted)
            return;
    }

    //connect(this, SIGNAL(finish(QStringList,bool,QString)), SLOT(finishThread()), Qt::QueuedConnection);
    connect(this, SIGNAL(finished()), SLOT(finishThread()));

    m_progress = new QProgressDialog(m_parent);
    m_progress->open(this, SLOT(canceled()));
    m_progress->setMaximum(sumsize);
    m_progress->setValue(0);

    connect(this, SIGNAL(updateProgressLabelText(QString)), m_progress, SLOT(setLabelText(QString)));
    connect(this, SIGNAL(updateProgressValue(int)), m_progress, SLOT(setValue(int)));
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

        if (!importer.createTables(dialog->name(), dialog->fields(), dialog->useFts4())) {
            m_errorMessage = importer.errorMessage();
            goto onerror;
        }
        if (!importer.importFile(dialog->fileName(), dialog->name(), dialog->fields(), dialog->fileType(), dialog->skipLines(), dialog->firstLineIsHeader(), &m_canceled)) {
            m_errorMessage = importer.errorMessage();
            goto onerror;
        }
        emit updateProgressLabelText(QString("Creating indexes for %1...").arg(dialog->name()));
        if (!importer.createIndexes(dialog->name(), dialog->fields())) {
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
    if (m_progress->isVisible())
        m_progress->close();
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
