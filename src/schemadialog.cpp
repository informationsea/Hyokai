#include "schemadialog.h"
#include "ui_schemadialog.h"

#include <QItemSelectionModel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QDebug>
#include <QFileInfo>
#include <QProgressDialog>
#include <QRegExp>
#include <QEvent>
#include <QMouseEvent>

#include <csvreader.hpp>
#include <tablereader.hpp>
#include <memory>

#include "main.h"
#include "sqlfileimporter.h"
#include "sqldatatypeitemdelegate.h"

SchemaDialog::SchemaDialog(QSqlDatabase *sql_database, const QString &file, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SchemaDialog), m_sql_database(sql_database), m_import_file(file)
{
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);
    model = new SchemaTableModel(this);
    model->setShowLogicalIndex(!file.isEmpty());
    ui->tableView->setModel(model);
    ui->tableView->setDragEnabled(true);
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(tableChanged()));
    connect(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)), SLOT(tableChanged()));
    connect(model, SIGNAL(rowsRemoved (const QModelIndex &, int, int)), SLOT(tableChanged()));
    connect(ui->lineEdit, SIGNAL(textChanged(QString)), SLOT(tableChanged()));
    ui->tableView->installEventFilter(this);
    tableChanged();

    if (file.isEmpty()) {
        ui->importWidget->setVisible(false);
    } else {
        setWindowTitle(tr("Import file"));
    }

    ui->enableFTS4->setVisible(sql_database->driverName() == "QSQLITE");

    m_checkboxitem = new CheckBoxItemDelegate(this);
    connect(ui->tableView, SIGNAL(clicked(QModelIndex)), SLOT(tableClicked(QModelIndex)));
    ui->tableView->setItemDelegateForColumn(1, new SqlDataTypeItemDelegate(this));
}

SchemaDialog::~SchemaDialog()
{
    delete ui;
    delete model;
    delete m_checkboxitem;
}

bool SchemaDialog::showImportOptions() const
{
    return !m_import_file.isEmpty() ? true : false;
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
    QRegExp tableNameVaild("^[a-zA-Z_][0-9a-zA-Z_]*$");
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(model->isVaild() && tableNameVaild.exactMatch(ui->lineEdit->text()));
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

void SchemaDialog::setFirstLineIsHeader(bool enable)
{
    ui->headerLineCheckbox->setChecked(enable);
}

const QList<SchemaField> &SchemaDialog::fields() const
{
    return model->fields();
}

void SchemaDialog::setFileType(enum FileType type)
{
    m_fileType = type;

    switch (type) {
    case FILETYPE_CSV:
        ui->cammaDelimiter->setChecked(true);
        ui->tabDelimiter->setChecked(false);
        break;
    case FILETYPE_TVS:
        ui->cammaDelimiter->setChecked(false);
        ui->tabDelimiter->setChecked(true);
        break;
    case FILETYPE_XLSX:
        ui->fileTypeSelectLabel->hide();
        ui->cammaDelimiter->hide();
        ui->tabDelimiter->hide();
        break;

    default:
        // TODO
        break;
    }
}

enum FileType SchemaDialog::fileType()
{
    return m_fileType;
}

void SchemaDialog::setDuplicationMode(bool duplicate)
{
    m_duplication_mode = duplicate;
    ui->messageLabel->setText(tr("Information of primary key and indexes are not copied from orignal table."));
}

int SchemaDialog::skipLines() const
{
    return ui->skipLinesSpin->value();
}

bool SchemaDialog::firstLineIsHeader() const
{
    return ui->headerLineCheckbox->isChecked();
}

bool SchemaDialog::useFts4() const
{
    return ui->enableFTS4->isChecked() && m_sql_database->driverName() == "QSQLITE";
}

bool SchemaDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->tableView && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = (QMouseEvent *)event;
        QPoint pos = mouseEvent->pos();
        pos -= QPoint(ui->tableView->verticalHeader()->width(), ui->tableView->horizontalHeader()->height());
        QModelIndex index = ui->tableView->indexAt(pos);
        qDebug() << index;
    }
    return false;
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
    //model->setFields(suggestSchema(m_import_file, delimiter(), skipLines(), firstLineIsHeader(), ui->scanAllLine->isChecked() ? -1 : 20, this));

    model->setFields(SqlFileImporter::suggestSchema(m_import_file, m_fileType, skipLines(), firstLineIsHeader(), m_sql_database->driverName() == "QSQLITE"));
}

void SchemaDialog::tableClicked(const QModelIndex &index)
{
    if (index.data(Qt::EditRole).type() == QVariant::Bool) {
        model->setData(index, !index.data(Qt::EditRole).toBool());
    }
}
