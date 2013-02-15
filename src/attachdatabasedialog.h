#ifndef ATTACHDATABASEDIALOG_H
#define ATTACHDATABASEDIALOG_H

#include <QDialog>

namespace Ui {
class AttachDatabaseDialog;
}

class AttachDatabaseDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AttachDatabaseDialog(QWidget *parent = 0);
    ~AttachDatabaseDialog();

    QString attachAs();
    QString databasePath();

    QString sql();
    
private slots:
    void on_pushButton_clicked();
    void updated();

private:
    Ui::AttachDatabaseDialog *ui;

};

#endif // ATTACHDATABASEDIALOG_H
