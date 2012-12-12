#ifndef SUMMARYDIALOG_H
#define SUMMARYDIALOG_H

#include <QDialog>
#include <QList>
#include <QTemporaryFile>
#include <QImage>
#include <QBrush>

namespace Ui {
class SummaryDialog;
}

class SummaryDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SummaryDialog(const QList<double> &valus, const QString &columnName, QWidget *parent = 0);
    ~SummaryDialog();

protected:
    void closeEvent ( QCloseEvent * event );
    
private slots:
    void on_buttonCopyFull_clicked();

    void on_buttonCopyImport_clicked();

private:
    Ui::SummaryDialog *ui;

    QList<double> m_values;
    QString m_columnName;

    QString m_fullRScript;
    QString m_importRScript;

    QTemporaryFile *m_rdata_file;
    QTemporaryFile *m_rdraw_file;
    QTemporaryFile *m_rdraw_png;

    QImage *m_histogram;
    QBrush *m_histogram_brush;
};

#endif // SUMMARYDIALOG_H
