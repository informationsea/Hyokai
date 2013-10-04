#include "sqlfileimporter.h"

#include <QSqlQuery>
#include <QMap>
#include <QVariant>
#include <QByteArray>

#include <memory>
#include <ctype.h>
#include "tablereader.hpp"
#include "csvreader.hpp"

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
            fields.append(SchemaField(QString(QByteArray(column, readlen))));
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
                fields.append(SchemaField(QString("V%1").arg(QString::number(j))));
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

bool SqlFileImporter::importFile(QString path, const QString &name, const QList<SchemaField> &fields, FileType type, int skipLines, bool firstLineIsHeader)
{
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
    if (!reader->open_path(path.toUtf8().data()))
        return false;

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
    } while(1);
  finishImport:
    return true;
}
