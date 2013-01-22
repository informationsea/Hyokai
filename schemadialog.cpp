#include "schemadialog.h"
#include "ui_schemadialog.h"

#include <QItemSelectionModel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QDebug>
#include <QFileInfo>
#include <QProgressDialog>

#include "main.h"

SchemaDialog::SchemaDialog(QSqlDatabase *sql_database, QFile *importFile, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SchemaDialog), m_sql_database(sql_database), m_import_file(importFile)
{
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);
    model = new SchemaTableModel(this);
    model->setShowLogicalIndex(importFile);
    ui->tableView->setModel(model);
    ui->tableView->setDragEnabled(true);
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(tableChanged()));
    connect(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)), SLOT(tableChanged()));
    connect(model, SIGNAL(rowsRemoved (const QModelIndex &, int, int)), SLOT(tableChanged()));
    connect(ui->lineEdit, SIGNAL(textChanged(QString)), SLOT(tableChanged()));
    tableChanged();

    if (!importFile) {
        ui->importWidget->setVisible(false);
    }
}

SchemaDialog::~SchemaDialog()
{
    delete ui;
    delete model;
}

bool SchemaDialog::showImportOptions() const
{
    return m_import_file ? true : false;
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

QString SchemaDialog::name() const
{
    return ui->lineEdit->text();
}

void SchemaDialog::setFields(const QList<SchemaField> &fields)
{
    model->setFields(fields);
    tableChanged();
}

const QList<SchemaField> &SchemaDialog::fields() const
{
    return model->fields();
}

QString SchemaDialog::createTableSql() const
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

QStringList SchemaDialog::createIndexSqls() const
{
    QStringList results;
    foreach (const SchemaField field, fields()) {
        if (!field.indexedField()) continue;

        QString sql = QString("CREATE INDEX %1__%2__index on %1(%2)").arg(name(), field.name());
        results.append(sql);
    }

    return results;
}

void SchemaDialog::setDelimiter(char ch)
{
    ui->tabDelimiter->setChecked(false);
    ui->cammaDelimiter->setChecked(false);
    ui->customDelimiter->setChecked(false);
    ui->customDelimiterEdit->setEnabled(false);
    switch(ch) {
    case '\t':
        ui->tabDelimiter->setChecked(true);
        break;
    case ',':
        ui->cammaDelimiter->setChecked(true);
        break;
    default:
        ui->customDelimiter->setChecked(true);
        ui->customDelimiterEdit->setText(QChar(ch));
        break;
    }
}

char SchemaDialog::delimiter() const
{
    if (ui->tabDelimiter->isChecked()) {
        return '\t';
    } else if (ui->cammaDelimiter->isChecked()) {
        return ',';
    } else {
        if (ui->customDelimiterEdit->text().length())
            return ui->customDelimiterEdit->text().at(0).toAscii();
        else
            return '\t';
    }
}

int SchemaDialog::skipLines() const
{
    return ui->skipLinesSpin->value();
}

bool SchemaDialog::firstLineIsHeader() const
{
    return ui->headerLineCheckbox->isChecked();
}

QList<SchemaField> SchemaDialog::suggestSchema(QFile *file, char delimiter, int skipLines, bool firstLineIsHeader, int suggestLine, QWidget *progressParent)
{
    file->seek(0);
    QFileInfo fileInfo(file->fileName());

    QList<SchemaField> fields;
    QStringList fieldNames;
    //char separator = '\t';
    //if (import.endsWith(".csv"))
    //    separator = ',';

    QProgressDialog *suggestProgress = 0;
    if (suggestLine > 100 || suggestLine < 0) {
        suggestProgress = new QProgressDialog(tr("Suggesting fields"), tr("Skip"), 0, fileInfo.size(), progressParent);
        suggestProgress->setWindowModality(Qt::WindowModal);
    }

    for (int i = 0; i < skipLines; ++i) {
        file->readLine();
    }

    // suggests field name
    if (firstLineIsHeader) {
    QList<QByteArray> header = file->readLine().trimmed().split(delimiter);
    int counter = 0;
    foreach(QByteArray one, header) {
        QString newfieldname = normstr(one);
        if (fieldNames.contains(newfieldname)) {
            QString basename = newfieldname;
            int counter = 2;
            do {
                newfieldname = QString("%1_%2").arg(basename, QString::number(counter));
                counter += 1;
            } while(fieldNames.contains(newfieldname));
        }
        fields.append(SchemaField(newfieldname));
        fieldNames.append(newfieldname);
        fields.last().setFieldType(SchemaField::FIELD_INTEGER);
        fields.last().setLogicalIndex(counter);
        counter += 1;
    }
    }

    // suggests field type
    for (int i = 0; !file->atEnd() && (suggestLine < 0 || i < suggestLine); ++i) {
        QList<QByteArray> elements = file->readLine().trimmed().split(delimiter);
        while (elements.size() > fields.size()) {
            fields.append(SchemaField(QString("V%1").arg(QString::number(fields.size()))));
            fields.last().setFieldType(SchemaField::FIELD_INTEGER);
            fields.last().setLogicalIndex(fields.size()-1);
        }

        if (suggestProgress) {
            suggestProgress->setValue(file->pos());

            if (suggestProgress->wasCanceled()) {
                goto skipSuggest;
            }
        }

        for (int i = 0; i < fields.size() && i < elements.size(); i++) {
            fields[i].setMaximumLength(MAX(fields[i].maximumLength(), elements[i].length()));

            bool ok;
            QString str(elements[i]);
            switch (fields[i].fieldType()) {
            case SchemaField::FIELD_NONE:
            case SchemaField::FIELD_TEXT:
                break;
            case SchemaField::FIELD_INTEGER:
                str.toLongLong(&ok);
                if (ok)
                    break;
                fields[i].setFieldType(SchemaField::FIELD_REAL);
                // no break
            case SchemaField::FIELD_REAL:
                str.toDouble(&ok);
                if (ok)
                    break;
                fields[i].setFieldType(SchemaField::FIELD_TEXT);
                break;
            }
        }
    }

    skipSuggest:

    for(int i = 0; i < fields.size(); ++i) {
        if (fields[i].fieldType() == SchemaField::FIELD_INTEGER ||
                fields[i].fieldType() == SchemaField::FIELD_REAL ||
                fields[i].maximumLength() < 20)
            fields[i].setIndexedField(true);
    }

    if (suggestProgress) {
        suggestProgress->close();
        delete suggestProgress;
    }

    return fields;
}

void SchemaDialog::on_makeIndexButton_clicked()
{
    model->makeIndexForAll(true);
}

void SchemaDialog::on_notMakeIndexButton_clicked()
{
    model->makeIndexForAll(false);
}


void SchemaDialog::on_suggestColumnButton_clicked()
{
    model->setFields(suggestSchema(m_import_file, delimiter(), skipLines(), firstLineIsHeader(), ui->scanAllLine->isChecked() ? -1 : 20, this));
}
