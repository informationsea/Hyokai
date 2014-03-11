#ifndef FILETYPE_H
#define FILETYPE_H

#include <QString>

enum FileType {
    FILETYPE_SUGGEST,
    FILETYPE_TVS,
    FILETYPE_CSV,
    FILETYPE_XLSX
};

class FileTypeUtil {
public:
    static enum FileType getFileTypeFromPath(const QString &path);
}; // class FileTypeUtil

#endif // FILETYPE_H
