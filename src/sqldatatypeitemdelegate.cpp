#include "sqldatatypeitemdelegate.h"

#include <QComboBox>
#include <QLineEdit>
#include <QStringList>

SqlDataTypeItemDelegate::SqlDataTypeItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

QWidget *SqlDataTypeItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStringList types;
    types << "INTEGER" << "REAL" << "TEXT" << "BOLB" << "vchar()";

    QComboBox *comboBox = new QComboBox(parent);
    comboBox->setEditable(true);
    comboBox->addItems(types);
    //connect(comboBox, SIGNAL(currentTextChanged(QString)), S)
    return comboBox;
}

void SqlDataTypeItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = (QComboBox *)editor;
    comboBox->lineEdit()->setText(index.data(Qt::EditRole).toString());
}

void SqlDataTypeItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = (QComboBox *)editor;
    model->setData(index, comboBox->lineEdit()->text());
}
