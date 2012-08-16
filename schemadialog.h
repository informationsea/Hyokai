#ifndef SCHEMADIALOG_H
#define SCHEMADIALOG_H

#include <QDialog>

namespace Ui {
class SchemaDialog;
}

class SchemaDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SchemaDialog(QWidget *parent = 0);
    ~SchemaDialog();
    
private:
    Ui::SchemaDialog *ui;
};

#endif // SCHEMADIALOG_H
