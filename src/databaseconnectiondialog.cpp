#include "databaseconnectiondialog.h"
#include "ui_databaseconnectiondialog.h"

#include "main.h"
#include "mainwindow.h"
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QSqlError>

#include "sheetmessagebox.h"

DatabaseConnectionDialog::DatabaseConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DatabaseConnectionDialog)
{
    ui->setupUi(this);
}

DatabaseConnectionDialog::~DatabaseConnectionDialog()
{
    delete ui;
}

void DatabaseConnectionDialog::accept()
{
    QString connectionName = QDateTime::currentDateTime().toString("yyMMddhhmmsszzz");
    QSqlDatabase newconnection;
    if (ui->radioButtonSQLite->isChecked()) {
        newconnection = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        newconnection.setDatabaseName(ui->lineEditSQLiteDatabase->text());
        if (ui->checkBoxSQLiteReadOnly->isChecked()) {
            newconnection.setConnectOptions("QSQLITE_OPEN_READONLY=1;");
        }
    } else if (ui->radioButtonMySQL->isChecked() || ui->radioButtonPostgreSQL->isChecked()) {
        if (ui->radioButtonMySQL->isChecked()) {
            newconnection = QSqlDatabase::addDatabase("QMYSQL", connectionName);
        } else if (ui->radioButtonPostgreSQL->isChecked()) {
            newconnection = QSqlDatabase::addDatabase("QPSQL", connectionName);
        } else {
            newconnection = QSqlDatabase::addDatabase("QODBC", connectionName);
        }
        newconnection.setConnectOptions(ui->lineEditConnectionOptions->text());
        newconnection.setHostName(ui->lineEditHostName->text());
        newconnection.setPort(ui->spinBoxPort->value());
        newconnection.setDatabaseName(ui->lineEditDatabaseName->text());
        newconnection.setUserName(ui->lineEditUserName->text());
        newconnection.setPassword(ui->lineEditPassword->text());
    }

    if (!newconnection.open()) {
        SheetMessageBox::critical(this, tr("Cannot open database"), newconnection.lastError().text());
    } else {
        MainWindow *w = new MainWindow(newconnection);
        ::tableviewCleanupWindows();
        w->show();
        ::windowList.append(w);
        QDialog::accept();
    }
}

void DatabaseConnectionDialog::on_radioButtonSQLite_toggled(bool checked)
{
    if (checked) {
        ui->stackedWidget->setCurrentIndex(0);
    } else {
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void DatabaseConnectionDialog::on_radioButtonMySQL_clicked(bool checked)
{
    if (checked)
        ui->spinBoxPort->setValue(3306);
}

void DatabaseConnectionDialog::on_radioButtonPostgreSQL_clicked(bool checked)
{
    if (checked)
        ui->spinBoxPort->setValue(5432);
}

void DatabaseConnectionDialog::on_pushButtonSQLiteChoose_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open SQLite Database"), tableview_settings->value(LAST_SQLITE_DIRECTORY, QDir::homePath()).toString());
    if (path.isEmpty())
        return;
    tableview_settings->setValue(LAST_SQLITE_DIRECTORY, QFileInfo(path).dir().absolutePath());
    ui->lineEditSQLiteDatabase->setText(path);
}

void DatabaseConnectionDialog::on_radioButtonODBC_clicked(bool checked)
{
    // Do nothing
}
