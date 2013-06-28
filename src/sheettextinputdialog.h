#ifndef SHEETTEXTINPUTDIALOG_H
#define SHEETTEXTINPUTDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>

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

    static QString textInput(const QString &title, const QString &text, QWidget *parent = 0, const QString defaultValue = "", bool multiline = false);
    
private:
    Ui::SheetTextInputDialog *ui;
    bool m_multiline;
};

#endif // SHEETTEXTINPUTDIALOG_H
