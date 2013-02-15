#include "jointabledialog.h"
#include "ui_jointabledialog.h"

#include <QDebug>
#include <QComboBox>
#include <QLineEdit>
#include <QSqlDatabase>
#include <QSqlRecord>

JoinTableDialog::JoinTableDialog(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JoinTableDialog), m_database(db)
{
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);

    foreach(QString table, db->tables(QSql::AllTables)) {
        ui->leftTable->addItem(table);
        ui->rightTable->addItem(table);
    }

    connect(ui->leftTable, SIGNAL(editTextChanged(QString)), SLOT(leftTableChanged()));
    connect(ui->rightTable, SIGNAL(editTextChanged(QString)), SLOT(rightTableChanged()));
    leftTableChanged();
    rightTableChanged();
}

JoinTableDialog::~JoinTableDialog()
{
    delete ui;
}

QString JoinTableDialog::sql()
{
    QString type;
    if (ui->innerJoin->isChecked()) {
        type = "INNER JOIN";
    } else if (ui->leftOuterJoin->isChecked()) {
        type = "LEFT OUTER JOIN";
    }

    if (ui->naturalJoin->isChecked()) {
        return QString("SELECT * FROM %1 NATURAL %2 %3").arg(ui->leftTable->currentText(),
                                                             type,
                                                             ui->rightTable->currentText());
    } else {
        return QString("SELECT * FROM %1 %2 %3 ON %1.%4 = %3.%5").arg(ui->leftTable->currentText(),
                                                                      type,
                                                                      ui->rightTable->currentText(),
                                                                      ui->leftColumn->currentText(),
                                                                      ui->rightColumn->currentText());
    }
}

void JoinTableDialog::leftTableChanged()
{
    QSqlRecord record = m_database->record(ui->leftTable->lineEdit()->text());
    ui->leftColumn->clear();
    for (int i = 0; i < record.count(); i++) {
        ui->leftColumn->addItem(record.fieldName(i));
    }
}

void JoinTableDialog::rightTableChanged()
{
    QSqlRecord record = m_database->record(ui->rightTable->lineEdit()->text());
    ui->rightColumn->clear();
    for (int i = 0; i < record.count(); i++) {
        ui->rightColumn->addItem(record.fieldName(i));
    }
}

void JoinTableDialog::on_naturalJoin_clicked(bool checked)
{
    ui->leftColumn->setEnabled(!checked);
    ui->rightColumn->setEnabled(!checked);
}
