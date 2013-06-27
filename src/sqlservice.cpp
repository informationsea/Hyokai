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
