#include "addfilterdialog.h"
#include "ui_addfilterdialog.h"

#include <QSqlRecord>
#include <QSqlField>
#include <QDebug>

AddFilterDialog::AddFilterDialog(QSqlQueryModel *model, int columnIndex, QString defaultValue, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddFilterDialog), m_queryModel(model), m_intValidator(this), m_doubleValidator(this)
{
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);
    auto record = model->record();
    for (int i = 0; i < record.count(); i++) {
        ui->columnComboBox->addItem(record.fieldName(i));
    }
    ui->columnComboBox->setCurrentIndex(columnIndex);

    QList<QString> operatorList;
    operatorList << "=" << "<>" << "<" << ">" << "<=" << ">=";
    ui->operatorComboBox->addItems(operatorList);

    ui->lineEdit->setText(defaultValue);
}

AddFilterDialog::~AddFilterDialog()
{
    delete ui;
}

QString AddFilterDialog::whereSql() const
{
    QSqlField field = m_queryModel->record().field(ui->columnComboBox->currentIndex());
    switch (field.type()) {
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::Double:
        return ui->columnComboBox->currentText() + " " + ui->operatorComboBox->currentText() + " " + ui->lineEdit->text() + "";
    default:
        return ui->columnComboBox->currentText() + " " + ui->operatorComboBox->currentText() + " \"" + ui->lineEdit->text() + "\"";
    }
}

void AddFilterDialog::on_columnComboBox_currentIndexChanged(int index)
{
    QSqlField field = m_queryModel->record().field(index);
    qDebug() << "field" << field.name() << field.type();
    switch (field.type()) {
    case QVariant::Int:
    case QVariant::LongLong:
        ui->lineEdit->setValidator(&m_intValidator);
        break;
    case QVariant::Double:
        ui->lineEdit->setValidator(&m_doubleValidator);
        break;
    default:
        ui->lineEdit->setValidator(nullptr);
        break;
    }
}
