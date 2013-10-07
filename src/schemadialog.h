#ifndef SCHEMADIALOG_H
#define SCHEMADIALOG_H

#include <QDialog>
#include <QList>
#include <QFile>
#include <QWidget>
#include <QSqlDatabase>
#include "schematablemodel.h"
#include "sqlfileimporter.h"
#include "checkboxitemdelegate.h"

namespace Ui {
class SchemaDialog;
}

class SchemaDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SchemaDialog(QSqlDatabase *sql_database, const QString &file = QString(), QWidget *parent = 0);
    ~SchemaDialog();

    void setName(const QString &name);
    QString name() const;
    QString fileName() const {return m_import_file;}
    void setFields(const QList<SchemaField> &fields);
    const QList<SchemaField> &fields() const;
    bool showImportOptions() const;

    void setFileType(enum SqlFileImporter::FileType type);
    enum SqlFileImporter::FileType fileType();

    void setDuplicationMode(bool duplicate);
    int skipLines() const;
    bool firstLineIsHeader() const;
    bool useFts4() const;

    virtual bool eventFilter(QObject * watched, QEvent * event);

private slots:
    void on_addButton_clicked();
    void on_removeButton_clicked();
    void on_downButton_clicked();
    void on_upButton_clicked();
    void tableChanged();
    void on_makeIndexButton_clicked();
    void on_notMakeIndexButton_clicked();
    void on_suggestColumnButton_clicked();

    void tableClicked(const QModelIndex &index);

private:
    Ui::SchemaDialog *ui;
    SchemaTableModel *model;
    QSqlDatabase *m_sql_database;
    bool m_duplication_mode;
    QString m_import_file;

    CheckBoxItemDelegate *m_checkboxitem;
};

#endif // SCHEMADIALOG_H
