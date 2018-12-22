#ifndef ADDFILTERDIALOG_H
#define ADDFILTERDIALOG_H

#include <QDialog>
#include <QSqlQueryModel>
#include <QIntValidator>
#include <QDoubleValidator>

namespace Ui {
class AddFilterDialog;
}

class AddFilterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddFilterDialog(QSqlQueryModel *model, int columnIndex = 0, QString defaultValue = "", QWidget *parent = nullptr);
    ~AddFilterDialog();

    QString whereSql() const;

private slots:
    void on_columnComboBox_currentIndexChanged(int index);

private:
    Ui::AddFilterDialog *ui;
    QSqlQueryModel *m_queryModel;
    QIntValidator m_intValidator;
    QDoubleValidator m_doubleValidator;
};

#endif // ADDFILTERDIALOG_H
