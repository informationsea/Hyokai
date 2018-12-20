#ifndef TABLEVIEWSTATUS_H
#define TABLEVIEWSTATUS_H

#include <QString>
#include <QList>
#include <QDebug>

class TableViewState
{
public:
    TableViewState() {
        // default constructor
    }

    TableViewState(QString filter, QList<int> columnWidth1, QList<int> columnWidth2, QList<int> splitterWidth, QList<bool> hideColumn1, QList<bool> hideColumn2,
                    int splitWindow, int shownRows, int veritcalScroll, int horizontalScroll1, int horizontalScroll2) :
        _filter(filter), _columnWidth1(columnWidth1), _columnWidth2(columnWidth2), _splitterWidth(splitterWidth), _hideColumn1(hideColumn1), _hideColumn2(hideColumn2),
        _splitWindow(splitWindow), _shownRows(shownRows), _verticalScroll(veritcalScroll), _horizontalScroll1(horizontalScroll1),
        _horizontalScroll2(horizontalScroll2)
    {

    }

    const QString& filter() const {
        return _filter;
    }

    const QList<int>& columnWidth1() const {
        return _columnWidth1;
    }

    const QList<int>& columnWidth2() const {
        return _columnWidth2;
    }

    const QList<int>& splitterWidth() const {
        return _splitterWidth;
    }

    const QList<bool>& hideColumn1() const {
        return _hideColumn1;
    }

    const QList<bool>& hideColumn2() const {
        return _hideColumn2;
    }

    int splitWindow() const {
        return _splitWindow;
    }

    int shownRows() const {
        return _shownRows;
    }

    int verticalScroll() const {
        return _verticalScroll;
    }

    int horizontalScroll1() const {
        return _horizontalScroll1;
    }

    int horizontalScroll2() const {
        return _horizontalScroll2;
    }

private:
    QString _filter;
    QList<int> _columnWidth1;
    QList<int> _columnWidth2;
    QList<int> _splitterWidth;
    QList<bool> _hideColumn1;
    QList<bool> _hideColumn2;
    int _splitWindow;
    int _shownRows;
    int _verticalScroll;
    int _horizontalScroll1;
    int _horizontalScroll2;
};

QDebug operator<<(QDebug debug, const TableViewState &status);

#endif // TABLEVIEWSTATUS_H
