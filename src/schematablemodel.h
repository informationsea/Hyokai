#ifndef SCHEMATABLEMODEL_H
#define SCHEMATABLEMODEL_H

#include <QAbstractTableModel>

#include <QModelIndex>
#include <QVariant>
#include <QList>

#include "sqlservice.h"

class SchemaTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit SchemaTableModel(QObject *parent = 0);
    Qt::ItemFlags flags ( const QModelIndex & index ) const;
    
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;

    virtual bool insertRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
    virtual bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
    virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

    bool moveUp(int row);
    bool moveDown(int row);

    void setFields(const QList<SchemaField> &fields);
    const QList<SchemaField> & fields() { return m_fieldList; }
    bool isVaild();

    void setShowLogicalIndex(bool flag);
    bool showLogicalIndex();

    void makeIndexForAll(bool make);

private:
    QList<SchemaField> m_fieldList;
    bool m_show_logical_index;

signals:
    
public slots:
    
};

#endif // SCHEMATABLEMODEL_H
