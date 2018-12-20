#include "tableviewstatus.h"

QDebug operator<<(QDebug debug, const TableViewState &status) {
    debug << "TableViewStatus{"
          << "filter:" << status.filter()
          << ", columnWidth1:" << status.columnWidth1()
          << ", columnWidth2:" << status.columnWidth2()
          << ", splitterWidth:" << status.splitterWidth()
          << ", hideColumn1:" << status.hideColumn1()
          << ", hideColumn2:" << status.hideColumn2()
          << ", splitWindows:" << status.splitWindow()
          << ", shownRos:" << status.shownRows()
          << ", verticalScroll:" << status.verticalScroll()
          << ", horizontalScroll1:" << status.horizontalScroll1()
          << ", horizontalScroll2:" << status.horizontalScroll2()
          << "}";
    return debug;
}
