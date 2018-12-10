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
    }

    // when drawing numeric values
    if (isNumeric(index.data(Qt::DisplayRole))) {
        if (m_numDecimalPlacesMap.contains(index.column())) {
            const QString text = QString::number(index.data(Qt::DisplayRole).toDouble(), 'f', m_numDecimalPlacesMap[index.column()]);
            const QTextOption textOpts(Qt::AlignRight | Qt::AlignVCenter);
            painter->drawText(option.rect.adjusted(-4, 0, -4, 0), text, textOpts);  // TODO: remove hard-coded padding value
        } else {
            QStyleOptionViewItem opt = option;
            opt.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
            QStyledItemDelegate::paint(painter, opt, index);
        }
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

int TableViewStyledItemDelegate::numDecimalPlaces(int column) const
{
    return m_numDecimalPlacesMap.contains(column) ? m_numDecimalPlacesMap[column] : NUM_DECIMAL_PLACES_NOT_SET;
}

void TableViewStyledItemDelegate::setRoundingPrecision(int column, int precision)
{
    if (column >= 0) {
        if (precision >= 0) {
            m_numDecimalPlacesMap[column] = precision;
        } else if (m_numDecimalPlacesMap.contains(column)) {
            m_numDecimalPlacesMap.remove(column);
        }
    }
}
