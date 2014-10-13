#ifndef SUMMARYDIALOG_H
#define SUMMARYDIALOG_H

#include <QDialog>
#include <QList>
#include <QTemporaryFile>
#include <QImage>
#include <QBrush>

#include "sphistogramplotter.h"

namespace Ui {
class SummaryDialog;
}

class SummaryDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SummaryDialog(const QList<double> &valus, const QString &tableAndcolumnName, QWidget *parent = 0);
    ~SummaryDialog();

protected:
    void closeEvent ( QCloseEvent * event );
    
private slots:
    void on_buttonCopyImport_clicked();

    void on_doubleSpinBox_valueChanged(double arg1);

    void on_exportImageButton_clicked();

private:
    Ui::SummaryDialog *ui;

    QList<double> m_values;
    QString m_tableAndColumnName;
    QString m_rName;

    QString m_drawRScript;

    QTemporaryFile *m_rdata_file;
    QTemporaryFile *m_rdraw_file;
    QTemporaryFile *m_rdraw_png;

    QImage *m_histogram;
    QBrush *m_histogram_brush;

    SPHistogramPlotter m_histogramPlotter;
};

#endif // SUMMARYDIALOG_H
