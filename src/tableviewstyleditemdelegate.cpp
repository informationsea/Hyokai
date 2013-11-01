#include "tableviewstyleditemdelegate.h"

#include <QDebug>
#include <QPainter>
#include <QSqlTableModel>
#include <QTextOption>
#include <QVariant>

static inline bool isNumeric(const QVariant &value)
{
    return value.type() == QVariant::Int || value.type() == QVariant::Double;
}

TableViewStyledItemDelegate::TableViewStyledItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

void TableViewStyledItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // fills background
    const QSqlTableModel *model = static_cast<const QSqlTableModel *>(index.model());
    if (model->isDirty(index)) {
        painter->fillRect(option.rect, QColor(index.row() % 2 ? "#FFF4CF" : "#FFFAE9"));
    } else if (index.row() % 2) {
        painter->fillRect(option.rect, QColor("#E8EDF5"));
    }

    // when drawing numeric values
    if (isNumeric(index.data(Qt::DisplayRole))) {
        const QString text = QString::number(index.data(Qt::DisplayRole).toDouble(), 'f', 3);
        const QTextOption textOpts(Qt::AlignRight | Qt::AlignVCenter);
        painter->drawText(option.rect, text, textOpts);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}
