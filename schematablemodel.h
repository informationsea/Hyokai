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

public:
    SchemaField();
    SchemaField(QString name);
    virtual ~SchemaField();

    QString name() const { return m_name; }
    enum FieldType fieldType() const {return m_type; }
    bool isPrimaryKey() const {return m_primary_key; }
    bool indexedField() const { return m_indexed_field; }

    void setName(const QString &name) { m_name = name; }
    void setFieldType(enum FieldType type) { m_type = type; }
    void setPrimaryKey(bool key) { m_primary_key = key; }
    void setIndexedField(bool flag) { m_indexed_field = flag; }
};

class SchemaTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit SchemaTableModel(QObject *parent = 0);
    Qt::ItemFlags flags ( const QModelIndex & index ) const;
    
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;

    virtual bool insertRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
    virtual bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
    virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

    void setFields(const QList<SchemaField> &fields);
    const QList<SchemaField> & fields() { return m_fieldList; }
    bool isVaild();

private:
    QList<SchemaField> m_fieldList;

signals:
    
public slots:
    
};

#endif // SCHEMATABLEMODEL_H
