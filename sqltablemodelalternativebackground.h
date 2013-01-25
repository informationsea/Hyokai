#ifndef SQLTABLEMODELALTERNATIVEBACKGROUND_H
#define SQLTABLEMODELALTERNATIVEBACKGROUND_H

#include <QSqlTableModel>

class SqlTableModelAlternativeBackground : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit SqlTableModelAlternativeBackground(QObject *parent = 0, QSqlDatabase db = QSqlDatabase() );
    
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;

    virtual void setTable(const QString &tableName);
    QString plainTableName() const;

    bool editable() { return m_editable && !m_view; }
    bool isView() { return m_view; }

private:
    bool m_editable;
    bool m_view;

signals:
    
public slots:
    void setEditable(bool editable) {m_editable = editable;}
};

#endif // SQLTABLEMODELALTERNATIVEBACKGROUND_H
