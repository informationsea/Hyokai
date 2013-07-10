#ifndef SQLSERVICE_H
#define SQLSERVICE_H

#include <QSqlDatabase>
#include <QSqlDriver>

class SqlService
{
private:
    SqlService();

public:
    static bool isVaildTableName(const QString &name);
    static bool isVaildTableName(const QString &name, const QSqlDatabase *database); // check names in database
    static QString suggestTableName(const QString &templateName, const QSqlDatabase *database);
    static QString createRcodeToImport(const QSqlDatabase &database, const QString &query, const QString &variableName);
    static QString createRcodeToImportWithTable(const QSqlDatabase &database, const QString &tableName, const QString &whereStatement);
};

#endif // SQLSERVICE_H
