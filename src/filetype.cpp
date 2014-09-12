#include "filetype.h"

#include <QStringList>

enum FileType FileTypeUtil::getFileTypeFromPath(const QString &path)
{
    QStringList tsvsuffixes;
    tsvsuffixes << ".txt" << ".tsv" << ".bed" << ".gff" << ".gtf";
    QStringList compressedTsvSuffix;
    foreach(QString one, tsvsuffixes) {
        compressedTsvSuffix << one+".gz";
    }
    tsvsuffixes << compressedTsvSuffix;

    if (path.endsWith(".csv") || path.endsWith(".csv.gz"))
        return FILETYPE_CSV;
    else if (path.endsWith(".xlsx"))
        return FILETYPE_XLSX;
    else {
        foreach (QString one, tsvsuffixes) {
            if (path.endsWith(one))
                return FILETYPE_TVS;
        }

        return FILETYPE_SUGGEST;
    }
}
