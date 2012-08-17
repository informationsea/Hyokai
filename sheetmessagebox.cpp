#include "sheetmessagebox.h"

QMessageBox* SheetMessageBox::makeMessageBox
( QWidget * parent, const QString & title, const QString & text,
  QMessageBox::StandardButtons buttons,
  QMessageBox::StandardButton defaultButton)
{

    QMessageBox *mesbox = new QMessageBox(parent);
    if(parent)
        mesbox->setWindowModality(Qt::WindowModal);
    else
        mesbox->setWindowModality(Qt::ApplicationModal);
#ifdef Q_WS_MACX
    mesbox->setText(title);
    mesbox->setInformativeText(text);
#else
    mesbox->setWindowTitle(title);
    mesbox->setText(text);
#endif
    mesbox->setStandardButtons(buttons);
    mesbox->setDefaultButton(defaultButton);

    return mesbox;
}

QMessageBox::StandardButton SheetMessageBox::critical
( QWidget * parent, const QString & title, const QString & text,
  QMessageBox::StandardButtons buttons,
  QMessageBox::StandardButton defaultButton)
{

    QMessageBox *mesbox = makeMessageBox(parent,title,text,buttons,defaultButton);
    mesbox->setIcon(QMessageBox::Critical);

    mesbox->exec();
    QMessageBox::StandardButton ret =
        mesbox->standardButton(mesbox->clickedButton());

    delete mesbox;

    return ret;
}

QMessageBox::StandardButton SheetMessageBox::information
( QWidget * parent, const QString & title, const QString & text,
  QMessageBox::StandardButtons buttons,
  QMessageBox::StandardButton defaultButton)
{

    QMessageBox *mesbox = makeMessageBox(parent,title,text,buttons,defaultButton);
    mesbox->setIcon(QMessageBox::Information);

    mesbox->exec();
    QMessageBox::StandardButton ret =
        mesbox->standardButton(mesbox->clickedButton());

    delete mesbox;

    return ret;
}

QMessageBox::StandardButton SheetMessageBox::question
( QWidget * parent, const QString & title, const QString & text,
  QMessageBox::StandardButtons buttons,
  QMessageBox::StandardButton defaultButton)
{

    QMessageBox *mesbox = makeMessageBox(parent,title,text,buttons,defaultButton);
    mesbox->setIcon(QMessageBox::Question);

    mesbox->exec();
    QMessageBox::StandardButton ret =
        mesbox->standardButton(mesbox->clickedButton());

    delete mesbox;

    return ret;
}

QMessageBox::StandardButton SheetMessageBox::warning
( QWidget * parent, const QString & title, const QString & text,
  QMessageBox::StandardButtons buttons,
  QMessageBox::StandardButton defaultButton)
{

    QMessageBox *mesbox = makeMessageBox(parent,title,text,buttons,defaultButton);
    mesbox->setIcon(QMessageBox::Warning);

    mesbox->exec();
    QMessageBox::StandardButton ret =
        mesbox->standardButton(mesbox->clickedButton());

    delete mesbox;

    return ret;
}
