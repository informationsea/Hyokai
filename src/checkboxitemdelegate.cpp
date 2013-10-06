#include "checkboxitemdelegate.h"

#include <QCheckBox>
#include <QDebug>

CheckBoxItemDelegate::CheckBoxItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

QWidget *CheckBoxItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    qDebug() << "CheckBoxItemDelegate" << index;
    QCheckBox *checkbox = new QCheckBox(parent);
    return checkbox;
}

void CheckBoxItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    qDebug() << "setEditorData";
    QCheckBox *checkbox = (QCheckBox *)editor;
    checkbox->setChecked(index.data(Qt::EditRole).toBool());
}

void CheckBoxItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    qDebug() << "setModelData";
    QCheckBox *checkbox = (QCheckBox *)editor;
    checkbox->setChecked(model->data(index, Qt::EditRole).toBool());
}
