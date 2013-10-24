#ifndef SQLPLOTCHART_H
#define SQLPLOTCHART_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QTemporaryFile>
#include <QList>

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
    QTemporaryFile *m_rcode;
    QTemporaryFile *m_rpng;
    QTemporaryFile *m_rdata;
    FieldType m_fields_type[2];

    QString generateRcode(const QString &device);
    void writeTable();

private slots:
    void refreshTables();
    void on_chartTypeComboBox_currentIndexChanged(int index);
    void on_plotButton_clicked();
    void on_tableComboBox_editTextChanged(const QString &arg1);
    void on_exportButton_clicked();
};

#endif // SQLPLOTCHART_H
