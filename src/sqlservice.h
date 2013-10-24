#ifndef SQLSERVICE_H
#define SQLSERVICE_H

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTableView>
#include <QList>
#include <QTemporaryFile>

class SchemaField
{
public:
    enum FieldType { FIELD_TEXT, FIELD_INTEGER, FIELD_REAL, FIELD_NONE };
private:
    QString m_name;
    QString m_type;
    bool m_primary_key;
    bool m_indexed_field;
    int m_logical_index; /* only for import file */
    int m_maximum_length; /* only for import file */

public:
    SchemaField();
    SchemaField(QString name);
    virtual ~SchemaField();

    QString name() const { return m_name; }
    QString fieldType() const {return m_type; }
    bool isPrimaryKey() const {return m_primary_key; }
    bool indexedField() const { return m_indexed_field; }
    int logicalIndex() const {return m_logical_index; }
    int maximumLength() const {return m_maximum_length; }

    void setName(const QString &name) { m_name = name; }
    void setFieldType(const QString &type) { m_type = type; }
    void setPrimaryKey(bool key) { m_primary_key = key; }
    void setIndexedField(bool flag) { m_indexed_field = flag; }
    void setLogicalIndex(int index) { m_logical_index = index; }
    void setMaximumLength(int length) {m_maximum_length = length; }
};

class SqlService
{
private:
    SqlService();

public:
    enum WriteType { BIN_INT32, BIN_INT64, BIN_DOUBLE};

    static bool isVaildTableName(const QString &name);
    static bool isVaildTableName(const QString &name, const QSqlDatabase *database); // check names in database
    static QString suggestTableName(const QString &templateName, const QSqlDatabase *database);
    static QString suggestFieldName(const QString &fieldName, const QList<SchemaField> &currentSchema);
    static QString createRcodeToImport(const QSqlDatabase &database, const QString &query, const QString &variableName);
    static QString createRcodeToImportWithTable(const QSqlDatabase &database, const QString &tableName, const QString &whereStatement);
    static void copyFromTableView(const QTableView *tableView, bool copyHeader);
    static QTemporaryFile *writeTableToBinary(QSqlQuery query, WriteType type, QObject *parent = 0);
};

#endif // SQLSERVICE_H
