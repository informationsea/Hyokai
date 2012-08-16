#include "schemadialog.h"
#include "ui_schemadialog.h"

SchemaDialog::SchemaDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SchemaDialog)
{
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);
}

SchemaDialog::~SchemaDialog()
{
    delete ui;
}
