#include "schematablemodel.h"

#include <QColor>

SchemaTableModel::SchemaTableModel(QObject *parent) :
    QAbstractTableModel(parent), m_show_logical_index(false)
{
}


Qt::ItemFlags SchemaTableModel::flags ( const QModelIndex & index ) const
{
    switch (index.column()) {
    case 0:
    case 1:
    case 4:
    default:
        return Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled;
    case 2:
    case 3:
        return Qt::ItemIsSelectable|Qt::ItemIsEnabled;
    }
}

QVariant SchemaTableModel::headerData ( int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
        return QVariant(section);
    if (orientation != Qt::Horizontal)
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        switch (section) {
        case 0:
            return tr("Field Name");
        case 1:
            return tr("Data Type");
        case 2:
            return tr("Primary Key");
        case 3:
            return tr("Index");
        case 4:
            return tr("Column index in import text");
        default:
            break;
        }
    default:
        break;
    }
    return QVariant();
}

int SchemaTableModel::columnCount ( const QModelIndex & /* parent */) const
{
    if (m_show_logical_index)
        return 5;
    return 4;
}

QVariant SchemaTableModel::data ( const QModelIndex & index, int role ) const
{
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch(index.column()) {
        case 0:
            return m_fieldList[index.row()].name();
        case 1:
            return m_fieldList[index.row()].fieldType();
        case 2:
            return m_fieldList[index.row()].isPrimaryKey();
        case 3:
            return m_fieldList[index.row()].indexedField();
        case 4:
            return m_fieldList[index.row()].logicalIndex();
        }
        break;
    default:
        break;
    }
    return QVariant();
}

int SchemaTableModel::rowCount ( const QModelIndex & /* parent */ ) const
{
    return m_fieldList.size();
}

bool SchemaTableModel::insertRows ( int row, int count, const QModelIndex & parent )
{
    beginInsertRows(parent, row, row);
    for (int i = 0; i < count; i++) {
        m_fieldList.insert(row, SchemaField());
    }
    endInsertRows();
    return true;
}

bool SchemaTableModel::removeRows ( int row, int count, const QModelIndex & parent )
{
    beginRemoveRows(parent, row, row+count-1);
    for (int i = count + row - 1; row <= i; i--) {
        m_fieldList.removeAt(i);
    }
    endRemoveRows();
    return true;
}

bool SchemaTableModel::setData ( const QModelIndex & index, const QVariant & value, int role )
{
    switch(role) {
    case Qt::EditRole:
        switch(index.column()) {
        case 0:
            m_fieldList[index.row()].setName(value.toString());
            emit dataChanged(index, index);
            return true;
        case 1:{
            m_fieldList[index.row()].setFieldType(value.toString());
            emit dataChanged(index, index);
            return false;
        }
        case 2:
            m_fieldList[index.row()].setPrimaryKey(value.toBool());
            emit dataChanged(index, index);
            return true;
        case 3:
            m_fieldList[index.row()].setIndexedField(value.toBool());
            emit dataChanged(index, index);
            return true;
        case 4:
            m_fieldList[index.row()].setLogicalIndex(value.toInt());
            emit dataChanged(index, index);
            return true;
        }
        break;        

    default:
        break;
    }

    return false;
}

bool SchemaTableModel::isVaild()
{
    if (m_fieldList.isEmpty())
        return false;
    foreach(SchemaField field, m_fieldList) {
        if (field.name().isEmpty())
            return false;
        if (field.name().at(0).isDigit())
            return false;
    }
    return true;
}


void SchemaTableModel::setFields(const QList<SchemaField> &fields)
{
    beginResetModel();
    m_fieldList = fields;
    endResetModel();
}

bool SchemaTableModel::moveUp(int row)
{
    if (row <= 0)
        return false;
    beginMoveRows(QModelIndex(), row, row, QModelIndex(), row-1);
    SchemaField temp = m_fieldList[row];
    m_fieldList.removeAt(row);
    m_fieldList.insert(row-1, temp);
    endMoveRows();
    return true;
}

bool SchemaTableModel::moveDown(int row)
{
    if (row >= rowCount()-1)
        return false;
    beginMoveRows(QModelIndex(), row, row, QModelIndex(), row+2);
    SchemaField temp = m_fieldList[row];
    m_fieldList.removeAt(row);
    m_fieldList.insert(row+1, temp);
    endMoveRows();
    return true;
}

void SchemaTableModel::setShowLogicalIndex(bool flag)
{
    beginResetModel();
    m_show_logical_index = flag;
    endResetModel();
}

bool SchemaTableModel::showLogicalIndex()
{
    return m_show_logical_index;
}

void SchemaTableModel::makeIndexForAll(bool make)
{
    for (int i = 0; i < m_fieldList.size(); ++i) {
        m_fieldList[i].setIndexedField(make);
    }
    emit dataChanged(index(0, 3), index(rowCount()-1, 3));
}
