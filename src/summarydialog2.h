#ifndef SUMMARYDIALOG2_H
#define SUMMARYDIALOG2_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QTableView>

#include "sphistogramplotter.h"
#include "sqlitemcounttablemodel.h"
#include "splineplotter.h"

namespace Ui {
class SummaryDialog2;
}

class SummaryDialog2 : public QDialog
{
    Q_OBJECT


public:
    explicit SummaryDialog2(QSqlDatabase *database, QString fromValue, QString columnValue, QString where, QWidget *parent = nullptr);
    ~SummaryDialog2() override;

    QTableView* tableView();

private slots:
    void on_binWidthSpinBox_valueChanged(double arg1);

    void on_exportImageButton_clicked();

    void on_exportImageCummurativeButton_clicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::SummaryDialog2 *ui;
	QSqlDatabase *m_database;
	QString m_fromValue;
	QString m_columnName;
	QString m_where;

	SqlItemCountTableModel m_model;

	QList<double> m_doubleList;
	double m_sumValue;
	int m_numberOfSkippedRow;

	QImage *m_histogram;
	QBrush *m_histogram_brush;

	SPHistogramPlotter m_histogramPlotter;
	SPLinePlotter m_linePlotter;
};

#endif // SUMMARYDIALOG2_H
