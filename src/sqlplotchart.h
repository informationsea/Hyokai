#ifndef SQLPLOTCHART_H
#define SQLPLOTCHART_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QTemporaryFile>
#include <QList>

#include "spscatterplotter.h"
#include "sphistogramplotter.h"

namespace Ui {
class SqlPlotChart;
}

class SqlPlotChart : public QDialog
{
    Q_OBJECT
private:
    enum FieldType {
        FIELD_STRING,
        FIELD_NUMERIC
    };

public:
    explicit SqlPlotChart(QSqlDatabase *database, QWidget *parent = 0, const QString &defaultName = "");
    ~SqlPlotChart();

    void setFilter(const QString &filter);

protected:
    void changeEvent ( QEvent * event );
    
private:
    Ui::SqlPlotChart *ui;
    QSqlDatabase *m_database;
    FieldType m_fields_type[2];

    SPHistogramPlotter m_histogramPlotter;
    SPScatterPlotter m_scatterPlotter;


private slots:
    void refreshTables();
    void on_chartTypeComboBox_currentIndexChanged(int index);
    void on_plotButton_clicked();
    void on_tableComboBox_editTextChanged(const QString &arg1);
    void on_exportImageButton_clicked();
};

#endif // SQLPLOTCHART_H
