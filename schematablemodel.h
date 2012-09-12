#ifndef SCHEMATABLEMODEL_H
#define SCHEMATABLEMODEL_H

#include <QAbstractTableModel>

#include <QModelIndex>
#include <QVariant>
#include <QList>

class SchemaField
{
public:
    enum FieldType { FIELD_TEXT, FIELD_INTEGER, FIELD_REAL, FIELD_NONE };
private:
    QString m_name;
    enum FieldType m_type;
    bool m_primary_key;
    bool m_indexed_field;
    int m_logical_index; /* only for import file */

public:
    SchemaField();
    SchemaField(QString name);
    virtual ~SchemaField();

    QString name() const { return m_name; }
    enum FieldType fieldType() const {return m_type; }
    bool isPrimaryKey() const {return m_primary_key; }
    bool indexedField() const { return m_indexed_field; }
    int logicalIndex() const {return m_logical_index; }

    void setName(const QString &name) { m_name = name; }
    void setFieldType(enum FieldType type) { m_type = type; }
    void setPrimaryKey(bool key) { m_primary_key = key; }
    void setIndexedField(bool flag) { m_indexed_field = flag; }
    void setLogicalIndex(int index) { m_logical_index = index; }
};

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

private:
    QList<SchemaField> m_fieldList;
    bool m_show_logical_index;

signals:
    
public slots:
    
};

#endif // SCHEMATABLEMODEL_H
