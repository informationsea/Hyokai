#include "sqlfileexporter.h"

#include <QSqlError>
#include <QSqlRecord>
#include <QFile>
#include <QFileInfo>
#include <QVariant>
//#include "qtxlsx/src/xlsx/xlsxdocument.h"

SqlFileExporter::SqlFileExporter(QSqlDatabase *database, QWidget *parent) :
    QObject(parent), m_database(database), m_parent(parent)
{
}

bool SqlFileExporter::exportTable(QSqlQuery query, const QString &outputpath, enum FileType fileType)
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

    if (!query.first()) {
        m_errorMessage = query.lastError().text();
        return false;
    }

    switch (fileType) {
    case FILETYPE_CSV:
    case FILETYPE_TVS:
        return exportTableAsCSV(query, outputpath, fileType == FILETYPE_CSV);
    //case FILETYPE_XLSX:
    //    return exportTableAsXLSX(query, outputpath);

    default:
        m_errorMessage = tr("Unsupported file type is selected (%1)").arg(fileType);
        return false;
    }
}

bool SqlFileExporter::exportTableAsCSV(QSqlQuery query, const QString &outputpath, bool csv) {
    QFile outputfile(outputpath);
    if (!outputfile.open(QIODevice::WriteOnly))
        return false;

    QString delimiter = "\t";
    if (csv)
        delimiter = ",";

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

/*
bool SqlFileExporter::exportTableAsXLSX(QSqlQuery query, const QString &outputpath)
{
    //
    QXlsx::Document doc;

    QSqlRecord record = query.record();
    for (int i = 0; i < record.count(); ++i)
        doc.write(1, i + 1, record.fieldName(i));

    //
    int row = 1;
    do {
        record = query.record();
        for (int col = 0; col < record.count(); ++col)
            doc.write(row + 1, col + 1, record.value(col));
        ++row;
    } while (query.next());

    //
    bool ret = doc.saveAs(outputpath);
    if (!ret) m_errorMessage = tr("Failed to write to the specified file.");

    return ret;
}
*/

QString SqlFileExporter::quoteCSVColumn(QString column)
{
    if (column.contains('"') || column.contains('\n') || column.contains('\r') || column.contains(',')) {
        column.replace("\"", "\"\"");
        return QString("\"%1\"").arg(column);
    }
    return column;
}
