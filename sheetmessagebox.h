#ifndef SHEETMESSAGEBOX_H
#define SHEETMESSAGEBOX_H

#include <QMessageBox>

class SheetMessageBox
{
private:
    SheetMessageBox() {}
    static QMessageBox* makeMessageBox
    ( QWidget * parent, const QString & title, const QString & text,
      QMessageBox::StandardButtons buttons = QMessageBox::Ok,
      QMessageBox::StandardButton defaultButton = QMessageBox::NoButton );

public:
    static QMessageBox::StandardButton critical
    ( QWidget * parent, const QString & title, const QString & text,
      QMessageBox::StandardButtons buttons = QMessageBox::Ok,
      QMessageBox::StandardButton defaultButton = QMessageBox::NoButton );

    static QMessageBox::StandardButton information
    ( QWidget * parent, const QString & title, const QString & text,
      QMessageBox::StandardButtons buttons = QMessageBox::Ok,
      QMessageBox::StandardButton defaultButton = QMessageBox::NoButton );

    static QMessageBox::StandardButton question
    ( QWidget * parent, const QString & title, const QString & text,
      QMessageBox::StandardButtons buttons = QMessageBox::Ok,
      QMessageBox::StandardButton defaultButton = QMessageBox::NoButton );

    static QMessageBox::StandardButton warning
    ( QWidget * parent, const QString & title, const QString & text,
      QMessageBox::StandardButtons buttons = QMessageBox::Ok,
      QMessageBox::StandardButton defaultButton = QMessageBox::NoButton );
};

#endif // SHEETMESSAGEBOX_H
