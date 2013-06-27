#ifndef SCHEMADIALOG_H
#define SCHEMADIALOG_H

#include <QDialog>
#include <QList>
#include <QFile>
#include <QWidget>
#include <QSqlDatabase>
#include "schematablemodel.h"

namespace Ui {
class SchemaDialog;
}

class SchemaDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SchemaDialog(QSqlDatabase *sql_database, QFile *importFile = 0, QWidget *parent = 0);
    ~SchemaDialog();

    void setName(const QString &name);
    QString name() const;
    void setFields(const QList<SchemaField> &fields);
    const QList<SchemaField> &fields() const;
    bool showImportOptions() const;

    QString createTableSql() const;
    QStringList createIndexSqls() const;

    void setDelimiter(char ch);
    void setDuplicationMode(bool duplicate);
    char delimiter() const;
    int skipLines() const;
    bool firstLineIsHeader() const;
    QList<SchemaField> suggestSchema(QFile *file, char delimiter, int skipLines, bool firstLineIsHeader, int suggestLine, QWidget *progressParent);

private slots:
    void on_addButton_clicked();
    void on_removeButton_clicked();
    void on_downButton_clicked();
    void on_upButton_clicked();
    void tableChanged();
    void on_makeIndexButton_clicked();
    void on_notMakeIndexButton_clicked();
    void on_suggestColumnButton_clicked();

private:
    Ui::SchemaDialog *ui;
    SchemaTableModel *model;
    QSqlDatabase *m_sql_database;
    bool m_duplication_mode;
    QFile *m_import_file;
};

#endif // SCHEMADIALOG_H
