#include "sqlservice.h"
#include "main.h"

#include <QRegExp>
#include <QApplication>
#include <QClipboard>

static QRegExp tableNameVaild("^[a-zA-Z_][0-9a-zA-Z_]*$");

SchemaField::SchemaField() :
    m_name(""), m_type("INTEGER"), m_primary_key(false), m_indexed_field(false), m_logical_index(-1), m_maximum_length(0)
{

}


SchemaField::SchemaField(QString name) :
    m_name(name), m_type("INTEGER"), m_primary_key(false), m_indexed_field(false), m_logical_index(-1), m_maximum_length(0)
{

}

SchemaField::~SchemaField(){}

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

QString SqlService::suggestFieldName(const QString &fieldName, const QList<SchemaField> &currentSchema)
{
    QString basename = normstr(fieldName);
    QString newname = basename;

    bool nameChanged = false;
    int i = 1;
    do {
        foreach (SchemaField onefield, currentSchema) {
            if (onefield.name() == newname) {
                newname = QString("%1_%2").arg(basename, QString::number(i++));
                nameChanged = true;
            }
        }
    } while (nameChanged);
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

#ifdef Q_OS_WIN32
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

void SqlService::copyFromTableView(const QTableView *tableView, bool copyHeader)
{
    QModelIndexList selectedIndex = tableView->selectionModel()->selectedIndexes();

    struct select_points {
        int x;
        int y;
    } left_top, right_bottom;

    if (selectedIndex.size() <= 0) return;

    // first data
    left_top.x = selectedIndex[0].column();
    left_top.y = selectedIndex[0].row();
    right_bottom.x = selectedIndex[0].column();
    right_bottom.y = selectedIndex[0].row();

    foreach(QModelIndex index, selectedIndex) {
        left_top.x = MIN(index.column(), left_top.x);
        left_top.y = MIN(index.row(), left_top.y);
        right_bottom.x = MAX(index.column(), right_bottom.x);
        right_bottom.y = MAX(index.row(), right_bottom.y);
    }

    int width = right_bottom.x - left_top.x + 1;
    int height = right_bottom.y - left_top.y + 1;

    QList<QList<QVariant> >  matrix;
    QStringList header;
    for (int i = 0; i < height; ++i) {
        QList<QVariant> line;
        for (int j = 0; j < width; ++j) {
            line.append(QVariant());
        }
        matrix.append(line);
    }

    for (int i = 0; i < width; ++i) {
        header.append(tableView->model()->headerData(i + left_top.x, Qt::Horizontal).toString());
    }

    foreach(QModelIndex index, selectedIndex) {
        matrix[index.row() - left_top.y][index.column() - left_top.x] = tableView->model()->data(index, Qt::EditRole);
    }

    QString clipboard;

    if (copyHeader) {
        for (int i = 0; i < width; ++i) {
            if (i != 0)
                clipboard += "\t";
            clipboard += header[i];
        }
        clipboard += "\n";
    }

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (j != 0)
                clipboard += "\t";
            clipboard += matrix[i][j].toString();
        }
        if (i != height-1)
            clipboard += "\n";
    }
    QClipboard *clip = QApplication::clipboard();
    clip->setText(clipboard);
}
