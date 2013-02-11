#ifndef DATABASECONNECTIONDIALOG_H
#define DATABASECONNECTIONDIALOG_H

#include <QDialog>
#include <QSqlDatabase>

namespace Ui {
class DatabaseConnectionDialog;
}

class DatabaseConnectionDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit DatabaseConnectionDialog(QWidget *parent = 0);
    ~DatabaseConnectionDialog();

public slots:
    void accept();

private slots:
    void on_radioButtonSQLite_toggled(bool checked);

    void on_radioButtonMySQL_clicked(bool checked);

    void on_radioButtonPostgreSQL_clicked(bool checked);

    void on_pushButtonSQLiteChoose_clicked();

    void on_radioButtonODBC_clicked(bool checked);

private:
    Ui::DatabaseConnectionDialog *ui;
};

#endif // DATABASECONNECTIONDIALOG_H
