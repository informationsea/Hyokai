#include "sqlservice.h"
#include "main.h"

#include <QRegExp>

static QRegExp tableNameVaild("^[a-zA-Z_][0-9a-zA-Z_]*$");

SqlService::SqlService()
{
}

bool SqlService::isVaildTableName(const QString &name)
{
    return tableNameVaild.exactMatch(name);
}

bool SqlService::isVaildTableName(const QString &name, const QSqlDatabase *database)
{
    if (!tableNameVaild.exactMatch(name))
        return false;
    QStringList tables = database->tables(QSql::AllTables);
    if (tables.contains(name))
        return false;
    return true;
}

QString SqlService::suggestTableName(const QString &templateName, const QSqlDatabase *database)
{
    QString newname = normstr(templateName);
    QStringList tables = database->tables(QSql::AllTables);

    if (tables.contains(newname)) {
        QString baseName = newname;
        int i = 2;
        while(tables.contains(QString("%1_%2").arg(baseName, QString::number(i))))
            i++;
        newname = QString("%1_%2").arg(baseName, QString::number(i));
    }

    return newname;
}

QString SqlService::createRcodeToImport(const QSqlDatabase &database, const QString &query, const QString &variableName)
{
    QString driver = database.driverName();
    QString rDriver, databaseType;
    if (driver == "QSQLITE") {
        rDriver = "RSQLite";
        databaseType = "SQLite";
    } else if (driver == "QMYSQL") {
        rDriver = "RMySQL";
        databaseType = "MySQL";
    }

    if (driver.isEmpty()) {
        return QString::null;
    }

    QString databaseName = database.databaseName();

#ifdef Q_WS_WIN32
    if (driver == "QSQLITE")
        databaseName = databaseName.replace('\\', '/');
#endif

    QString rcode = QString(
                "tryCatch(library(\"%1\"), error=function(e){install.packages(\"%1\");library(\"%1\")})\n"
                "tableview.drv <- dbDriver(\"%5\")\n"
                "tableview.con <- dbConnect(tableview.drv, dbname=\"%2\")\n"
                "%3 <- dbGetQuery(tableview.con, \"%4\")\n"
                "dbDisconnect(tableview.con)\n"
                ).arg(rDriver, databaseName, variableName, query, databaseType);
    return rcode;
}

QString SqlService::createRcodeToImportWithTable(const QSqlDatabase &database, const QString &tableName, const QString &whereStatement)
{
    if (whereStatement.isEmpty())
        return createRcodeToImport(database, QString("SELECT * FROM %1").arg(tableName), tableName);
    return createRcodeToImport(database, QString("SELECT * FROM %1 WHERE %2").arg(tableName, whereStatement), tableName);
}
