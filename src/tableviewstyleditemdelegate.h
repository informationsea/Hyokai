#ifndef TABLEVIEWSTYLEDITEMDELEGATE_H
#define TABLEVIEWSTYLEDITEMDELEGATE_H

#include <QStyledItemDelegate>

class TableViewStyledItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TableViewStyledItemDelegate(QObject *parent = 0);

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

}; // class TableViewStyledItemDelegate

#endif // TABLEVIEWSTYLEDITEMDELEGATE_H
