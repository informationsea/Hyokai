#ifndef SCHEMADIALOG_H
#define SCHEMADIALOG_H

#include <QDialog>
#include <QList>
#include "schematablemodel.h"

namespace Ui {
class SchemaDialog;
}

class SchemaDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SchemaDialog(QWidget *parent = 0);
    ~SchemaDialog();

    void setName(const QString &name);
    QString name();
    void setFields(const QList<SchemaField> &fields);
    const QList<SchemaField> &fields();

    QString createTableSql();

private slots:
    void on_addButton_clicked();
    void on_removeButton_clicked();
    void on_downButton_clicked();
    void on_upButton_clicked();

    void tableChanged();

private:
    Ui::SchemaDialog *ui;
    SchemaTableModel *model;
};

#endif // SCHEMADIALOG_H
