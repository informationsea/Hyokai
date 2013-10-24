#ifndef SHEETTEXTINPUTDIALOG_H
#define SHEETTEXTINPUTDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QValidator>

namespace Ui {
class SheetTextInputDialog;
}

class SheetTextInputDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SheetTextInputDialog(const QString &title, const QString &text, QDialogButtonBox::StandardButtons buttons, QWidget *parent = 0, bool multiline = false);
    ~SheetTextInputDialog();

    void setText(const QString &value);
    QString text();
    void setValidator(const QValidator *validator);
    const QValidator *validator();

    static QString textInput(const QString &title, const QString &text, QWidget *parent = 0, const QString defaultValue = "", bool multiline = false, const QValidator *validator = 0);
    
private:
    Ui::SheetTextInputDialog *ui;
    bool m_multiline;
};

#endif // SHEETTEXTINPUTDIALOG_H
