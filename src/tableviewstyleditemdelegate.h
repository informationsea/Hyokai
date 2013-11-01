#ifndef TABLEVIEWSTYLEDITEMDELEGATE_H
#define TABLEVIEWSTYLEDITEMDELEGATE_H

#include <QMap>
#include <QStyledItemDelegate>

class TableViewStyledItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    enum { NUM_DECIMAL_PLACES_NOT_SET = -1 };

    explicit TableViewStyledItemDelegate(QObject *parent = 0);

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    int numDecimalPlaces(int column) const;
    void setRoundingPrecision(int column, int precision);

private:
    QMap<int, int> m_numDecimalPlacesMap;
}; // class TableViewStyledItemDelegate

#endif // TABLEVIEWSTYLEDITEMDELEGATE_H
