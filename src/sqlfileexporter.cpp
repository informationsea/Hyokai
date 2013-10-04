#include "sqlfileexporter.h"

#include <QSqlError>
#include <QSqlRecord>
#include <QFile>
#include <QFileInfo>
#include <QVariant>

SqlFileExporter::SqlFileExporter(QSqlDatabase *database, QWidget *parent) :
    QObject(parent), m_database(database), m_parent(parent)
{
}

bool SqlFileExporter::exportTable(QSqlQuery query, const QString &outputpath, bool csv)
{
    m_errorMessage.clear();

    if (!query.isSelect()) {
        m_errorMessage = tr("Query is not select");
        return false;
    }

    if (query.lastError().type() != QSqlError::NoError) {
        m_errorMessage = query.lastError().text()+"\n\n"+query.lastQuery();
        return false;
    }

    QFile outputfile(outputpath);
    if (!outputfile.open(QIODevice::WriteOnly))
        return false;

    QString delimiter = "\t";
    if (csv)
        delimiter = ",";

    if (!query.first()) {
        m_errorMessage = query.lastError().text();
        return false;
    }

    QSqlRecord record = query.record();
    for (int i = 0; i < record.count(); ++i) {
        if (i != 0)
            outputfile.write(delimiter.toUtf8());
        if (csv)
            outputfile.write(quoteCSVColumn(record.fieldName(i)).toUtf8());
        else
            outputfile.write(record.fieldName(i).toUtf8());
    }
    outputfile.write("\n");

    do {
        record = query.record();
        for (int i = 0; i < record.count(); ++i) {
            if (i != 0)
                outputfile.write(delimiter.toUtf8());
            if (csv)
                outputfile.write(quoteCSVColumn(record.value(i).toString()).toUtf8());
            else
                outputfile.write(record.value(i).toString().toUtf8());
        }
        outputfile.write("\n");
    } while(query.next());
    outputfile.close();
    return true;
}

QString SqlFileExporter::quoteCSVColumn(QString column)
{
    if (column.contains('"') || column.contains('\n') || column.contains('\r')) {
        column.replace("\"", "\"\"");
        return QString("\"%1\"").arg(column);
    }
    return column;
}
