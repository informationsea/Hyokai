#include "sheettextinputdialog.h"
#include "ui_sheettextinputdialog.h"

SheetTextInputDialog::SheetTextInputDialog(const QString &title, const QString &text, QDialogButtonBox::StandardButtons buttons, QWidget *parent, bool multiline) :
    QDialog(parent),
    ui(new Ui::SheetTextInputDialog)
{
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);
#ifdef Q_WS_MAC
    ui->label_2->setText(title);
    ui->label->setText(text);
#else
    setWindowTitle(title);
    ui->label->setText(text);
    ui->label_2->setVisible(false);
#endif
    ui->buttonBox->setStandardButtons(buttons);
    ui->lineEdit->setVisible(!multiline);
    ui->plainTextEdit->setVisible(multiline);
    m_multiline = multiline;
    resize(QSize());
}

SheetTextInputDialog::~SheetTextInputDialog()
{
    delete ui;
}

void SheetTextInputDialog::setText(const QString &value)
{
    if (m_multiline) {
        ui->plainTextEdit->setPlainText(value);
    } else {
        ui->lineEdit->setText(value);
    }
}

QString SheetTextInputDialog::text()
{
    if (m_multiline)
        return ui->plainTextEdit->toPlainText();
    return ui->lineEdit->text();
}

QString SheetTextInputDialog::textInput(const QString &title, const QString &text, QWidget *parent, const QString defaultValue, bool multiline)
{
    SheetTextInputDialog dialog(title, text, QDialogButtonBox::Ok|QDialogButtonBox::Cancel, parent, multiline);
    dialog.setText(defaultValue);
    int status = dialog.exec();
    switch(status) {
    case QDialog::Accepted:
        return dialog.text();
    default:
        return QString::null;
    }
}
