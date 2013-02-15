#include "attachdatabasedialog.h"
#include "ui_attachdatabasedialog.h"

#include "main.h"
#include "sheetmessagebox.h"
#include <QFileDialog>
#include <QFileInfo>

#define LAST_ATTACH_DIRECTORY "LAST_ATTACH_DIRECTORY"

AttachDatabaseDialog::AttachDatabaseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AttachDatabaseDialog)
{
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);
    connect(ui->attachAs, SIGNAL(textChanged(QString)), SLOT(updated()));
    connect(ui->databasePath, SIGNAL(textChanged(QString)), SLOT(updated()));
    updated();
}

AttachDatabaseDialog::~AttachDatabaseDialog()
{
    delete ui;
}

QString AttachDatabaseDialog::attachAs()
{
    return ui->attachAs->text();
}

QString AttachDatabaseDialog::databasePath()
{
    return ui->databasePath->text();
}

QString AttachDatabaseDialog::sql()
{
    return QString("ATTACH DATABASE \"%1\" AS \"%2\"").arg(databasePath().replace("\"", "\\\""), attachAs().replace("\"", "\\\""));
}

void AttachDatabaseDialog::on_pushButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(NULL, "Attach SQLite3 Database",
                                                tableview_settings->value(LAST_ATTACH_DIRECTORY, QDir::homePath()).toString(),
                                                "SQLite3 (*.sqlite3);; All (*)");
    if (path.isEmpty())
        return;
    QFileInfo fileInfo(path);
    tableview_settings->setValue(LAST_ATTACH_DIRECTORY, fileInfo.dir().absolutePath());
    ui->databasePath->setText(path);
}

void AttachDatabaseDialog::updated()
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!ui->attachAs->text().isEmpty() && !ui->databasePath->text().isEmpty());
}
