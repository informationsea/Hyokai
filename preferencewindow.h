#ifndef PREFERENCEWINDOW_H
#define PREFERENCEWINDOW_H

#include <QMainWindow>
#include <QAbstractTableModel>
#include <QVariant>
#include <QList>
#include <QStringList>

#define ATTACHED_DATABASES "ATTACHED_DATABASES"
#define SQL_TEMPLATES "SQL_TEMPLATES"
#define PATH_R "PATH_R"

// ---- Attach DB Table Model ------
class AttachDBTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit AttachDBTableModel(QObject *parent = 0);
    virtual ~AttachDBTableModel(){}

    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    virtual int columnCount ( const QModelIndex & /*parent*/ = QModelIndex() ) const { return 2; }
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual int rowCount ( const QModelIndex & /*parent*/ = QModelIndex() ) const { return list.size(); }
    virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;
    virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

    virtual bool insertAttachDB(const QString &db, const QString &as);
    virtual bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );

private:
    QList<QStringList> list;
    void saveList();
};
// ---- End of Attach DB Table Model ------

// ---- SQL Template Table Model ------
class SqlTemplateTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit SqlTemplateTableModel(QObject *parent = 0);
    virtual ~SqlTemplateTableModel(){}

    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    virtual int columnCount ( const QModelIndex & /*parent*/ = QModelIndex() ) const { return 1; }
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual int rowCount ( const QModelIndex & /*parent*/ = QModelIndex() ) const {return sqllist.size();}
    virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;
    virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
    virtual bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
    virtual bool insertRows ( int row, int count, const QModelIndex & parent = QModelIndex() );

private:
    QStringList sqllist;
    void saveList();
};
// ---- End of SQL Template Table Model ------

namespace Ui {
class PreferenceWindow;
}

class PreferenceWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit PreferenceWindow(QWidget *parent = 0);
    ~PreferenceWindow();

public slots:
    void activate();
    
private slots:
    void on_actionGeneral_triggered();
    void on_actionAttach_DB_triggered();
    void on_actionSQL_Templates_triggered();
    void on_pushButtonClearCustumSqlHistory_clicked();
    void on_addAttachDB_clicked();

    void on_removeAttachDB_clicked();

    void on_addSqlTemplate_clicked();

    void on_removeSqlTemplate_clicked();

    void on_lineRPath_textChanged(const QString &arg1);

    void on_buttonRPath_clicked();

    void on_pushButtonClearFilterHistory_clicked();

private:
    Ui::PreferenceWindow *ui;
    AttachDBTableModel *m_attachmodel;
    SqlTemplateTableModel *m_sqlmodel;
    void uncheckAll();
};

#endif // PREFERENCEWINDOW_H
