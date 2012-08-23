#include "schemadialog.h"
#include "ui_schemadialog.h"

#include <QItemSelectionModel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QDebug>

SchemaDialog::SchemaDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SchemaDialog)
{
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);
    model = new SchemaTableModel(this);
    ui->tableView->setModel(model);
    ui->tableView->setDragEnabled(true);
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(tableChanged()));
    connect(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)), SLOT(tableChanged()));
    connect(model, SIGNAL(rowsRemoved (const QModelIndex &, int, int)), SLOT(tableChanged()));
    connect(ui->lineEdit, SIGNAL(textChanged(QString)), SLOT(tableChanged()));
    tableChanged();
}

SchemaDialog::~SchemaDialog()
{
    delete ui;
    delete model;
}

void SchemaDialog::setShowLogicalIndex(bool flag)
{
    model->setShowLogicalIndex(flag);
}

bool SchemaDialog::showLogicalIndex()
{
    return model->showLogicalIndex();
}

void SchemaDialog::on_addButton_clicked()
{
    QItemSelectionModel *selection = ui->tableView->selectionModel();
    QList<int> rows;
    foreach (QModelIndex index, selection->selectedIndexes()) {
        if (!rows.contains(index.row()))
            rows << index.row();
    }
    qSort(rows);
    if (rows.isEmpty())
        model->insertRow(0);
    else
        model->insertRow(rows.last()+1);
}

void SchemaDialog::on_removeButton_clicked()
{
    QItemSelectionModel *selection = ui->tableView->selectionModel();
    QList<int> rows;
    foreach (QModelIndex index, selection->selectedIndexes()) {
        if (!rows.contains(index.row()))
            rows << index.row();
    }
    qSort(rows);
    while(!rows.isEmpty()) {
        model->removeRow(rows.takeLast());
    }
}

void SchemaDialog::on_downButton_clicked()
{
    QItemSelectionModel *selection = ui->tableView->selectionModel();
    QList<int> rows;
    foreach (QModelIndex index, selection->selectedIndexes()) {
        if (!rows.contains(index.row()))
            rows << index.row();
    }
    qSort(rows);
    if (!rows.isEmpty()) {
        model->moveDown(rows[0]);
    }

}

void SchemaDialog::on_upButton_clicked()
{
    QItemSelectionModel *selection = ui->tableView->selectionModel();
    QList<int> rows;
    foreach (QModelIndex index, selection->selectedIndexes()) {
        if (!rows.contains(index.row()))
            rows << index.row();
    }
    qSort(rows);
    if (!rows.isEmpty()) {
        model->moveUp(rows[0]);
    }
}

void SchemaDialog::tableChanged()
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(model->isVaild() && !ui->lineEdit->text().isEmpty());
}

void SchemaDialog::setName(const QString &name)
{
    ui->lineEdit->setText(name);
    tableChanged();
}

QString SchemaDialog::name()
{
    return ui->lineEdit->text();
}

void SchemaDialog::setFields(const QList<SchemaField> &fields)
{
    model->setFields(fields);
    tableChanged();
}

const QList<SchemaField> &SchemaDialog::fields()
{
    return model->fields();
}

QString SchemaDialog::createTableSql()
{
    QString sql("CREATE TABLE ");
    bool isFirstIteration = true;
    bool hasPrimaryKey = false;
    sql += name();
    sql += "(";
    foreach(const SchemaField field, fields()) {
        if (isFirstIteration)
            isFirstIteration = false;
        else
            sql += ",";
        sql += field.name();

        switch(field.fieldType()) {
        case SchemaField::FIELD_TEXT:
            sql += " TEXT";
            break;
        case SchemaField::FIELD_INTEGER:
            sql += " INTEGER";
            break;
        case SchemaField::FIELD_REAL:
            sql += " REAL";
            break;
        default:
            break;
        }
        if (field.isPrimaryKey())
            hasPrimaryKey = true;
    }
    if (hasPrimaryKey) {
        sql += ", PRIMARY KEY(";
        isFirstIteration = true;

        foreach(const SchemaField field, fields()) {
            if (field.isPrimaryKey()) {
                if (isFirstIteration)
                    isFirstIteration = false;
                else
                    sql += ",";
                sql += field.name();
            }
        }
        sql += ")";
    }
    sql += ");";

    //qDebug() << sql;
    return sql;
}

QStringList SchemaDialog::createIndexSqls()
{
    QStringList results;
    foreach (const SchemaField field, fields()) {
        if (!field.indexedField()) continue;

        QString sql = QString("CREATE INDEX %1__%2__index on %1(%2)").arg(name(), field.name());
        results.append(sql);
    }

    return results;
}
