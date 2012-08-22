#include "schematablemodel.h"

#include <QColor>

SchemaField::SchemaField() :
    m_name(""), m_type(FIELD_NONE), m_primary_key(false), m_indexed_field(false), m_logical_index(-1)
{

}


SchemaField::SchemaField(QString name) :
    m_name(name), m_type(FIELD_NONE), m_primary_key(false), m_indexed_field(false), m_logical_index(-1)
{

}


SchemaField::~SchemaField(){}

SchemaTableModel::SchemaTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}


Qt::ItemFlags SchemaTableModel::flags ( const QModelIndex & index ) const
{
    switch(index.column()){
    case 0:
    case 1:
    default:
        return Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled;
    case 2:
        return Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled|Qt::ItemIsUserCheckable;
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
            switch(m_fieldList[index.row()].fieldType()) {
            case SchemaField::FIELD_INTEGER:
                return "INTEGER";
            case SchemaField::FIELD_REAL:
                return "REAL";
            case SchemaField::FIELD_TEXT:
                return "TEXT";
            case SchemaField::FIELD_NONE:
                return "NONE";
            }
        case 2:
            return m_fieldList[index.row()].isPrimaryKey();
        case 3:
            return m_fieldList[index.row()].indexedField();
            break;
        }
    case Qt::BackgroundRole:
        if (index.row() % 2)
            return QVariant(QColor("#E8EDF5"));
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
            QString text = value.toString().toUpper();
            if (text == "INTEGER" || text.startsWith("INT")) {
                m_fieldList[index.row()].setFieldType(SchemaField::FIELD_INTEGER);
                emit dataChanged(index, index);
                return true;
            }
            if (text == "TEXT") {
                m_fieldList[index.row()].setFieldType(SchemaField::FIELD_TEXT);
                emit dataChanged(index, index);
                return true;
            }
            if (text == "REAL" || text.startsWith("DOUB") || text.startsWith("FL")) {
                m_fieldList[index.row()].setFieldType(SchemaField::FIELD_REAL);
                emit dataChanged(index, index);
                return true;
            }
            if (text == "NONE") {
                m_fieldList[index.row()].setFieldType(SchemaField::FIELD_NONE);
                emit dataChanged(index, index);
                return true;
            }
            // TODO: implement here
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
