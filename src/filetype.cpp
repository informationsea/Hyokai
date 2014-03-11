#include "filetype.h"

enum FileType FileTypeUtil::getFileTypeFromPath(const QString &path)
{
    if (path.endsWith(".csv") || path.endsWith(".csv.gz"))
        return FILETYPE_CSV;
    else if (path.endsWith(".txt") || path.endsWith(".tsv") || path.endsWith(".txt.gz") || path.endsWith(".tsv.gz"))
        return FILETYPE_TVS;
    else if (path.endsWith(".xlsx"))
        return FILETYPE_XLSX;
    else
        return FILETYPE_SUGGEST;
}
