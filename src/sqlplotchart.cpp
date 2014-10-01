#include "sqlplotchart.h"
#include "ui_sqlplotchart.h"

#include <QDebug>
#include <QEvent>
#include <QLineEdit>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlResult>
#include <QBuffer>
#include <QProcess>
#include <QClipboard>
#include <QFileDialog>
#include <QPainter>

#include "main.h"
#include "sheetmessagebox.h"

#define SAFE_DELETE(x) if(x){delete x;x = NULL;}

SqlPlotChart::SqlPlotChart(QSqlDatabase *database, QWidget *parent, const QString &defaultName) :
    QDialog(parent),
    ui(new Ui::SqlPlotChart), m_database(database)
{
    ui->setupUi(this);
    ui->tableComboBox->lineEdit()->setText(defaultName);
    if (database->driverName() == "QSQLITE") {
        QFileInfo info(database->databaseName());
        setWindowTitle(QString("Plot : ") + info.completeBaseName());
    } else {
        setWindowTitle(QString("Plot : ") + database->databaseName());
    }
    ui->sqlFilter->setDatabase(m_database);
    connect(ui->sqlFilter, SIGNAL(returnPressed()), ui->plotButton, SLOT(click()));
    //setMaximumSize(size());
    //ui->groupBox->setMaximumWidth(ui->groupBox->previousInFocusChain());
    refreshTables();
}

SqlPlotChart::~SqlPlotChart()
{
    delete ui;
}

void SqlPlotChart::setFilter(const QString &filter)
{
    ui->sqlFilter->setPlainText(filter);
}

void SqlPlotChart::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);
    if (event->type() != QEvent::ActivationChange)
        return;
    //qDebug() << isActiveWindow();
    //if (isActiveWindow())
    //    refreshTables();
}

void SqlPlotChart::refreshTables()
{
    QString current = ui->tableComboBox->lineEdit()->text();
    ui->tableComboBox->clear();
    ui->tableComboBox->addItems(m_database->tables(QSql::AllTables));
    ui->tableComboBox->lineEdit()->setText(current);
}

void SqlPlotChart::on_chartTypeComboBox_currentIndexChanged(int index)
{
    switch (index) {
    case 0:
        ui->axis2ComboBox->setEnabled(true);
        break;
    case 1:
        ui->axis2ComboBox->setEnabled(false);
        break;
    }


    ui->alphaSpin->setEnabled(index == 0);
    ui->binSpin->setEnabled(index == 1);
}

void SqlPlotChart::on_plotButton_clicked()
{
    QSqlQuery query = m_database->exec(QString("SELECT %1, %2 FROM %3 %4").arg(ui->axis1ComboBox->currentText(),
                                                                               ui->axis2ComboBox->currentText(),
                                                                               ui->tableComboBox->currentText(),
                                                                               ui->sqlFilter->toPlainText().isEmpty() ? "" : QString("WHERE %1").arg(ui->sqlFilter->toPlainText())));
    if (query.lastError().type() != QSqlError::NoError) {
        SheetMessageBox::critical(this, tr("Cannot select table"), query.lastError().text());
        return;
    }

    switch (ui->chartTypeComboBox->currentIndex()) {
    case 0: { // Scatter Plot
        QList<QPointF> data;
        while (query.next()) {
            qreal x, y;
            bool ok;
            x = query.value(0).toDouble(&ok);
            if(!ok) goto onerror;
            y = query.value(1).toDouble(&ok);
            if(!ok) goto onerror;

            data << QPointF(x, y);
        }

        m_scatterPlotter.setData(data);
        m_scatterPlotter.setAlpha(ui->alphaSpin->value());
        ui->plotWidget->setPlotter(&m_scatterPlotter);
        break;
    }
    case 1: { // Histogram
        QList<double> data;

        while (query.next()) {
            qreal x;
            bool ok;
            x = query.value(0).toDouble(&ok);
            if(!ok) goto onerror;
            data << x;
        }

       m_histogramPlotter.setData(data);
       ui->plotWidget->setPlotter(&m_histogramPlotter);
       break;
    }
    }
    ui->plotWidget->repaint();

    return;
    onerror:
    SheetMessageBox::critical(this, tr("Cannot plot"), tr("Non-number value was found."));
    return;
}

void SqlPlotChart::on_tableComboBox_editTextChanged(const QString &arg1)
{
    qDebug() << "currentIndexChanged " << arg1;
    QSqlRecord record = m_database->record(arg1);
    QStringList columns;
    for (int i = 0; i < record.count(); ++i) {
        columns << record.fieldName(i);
    }
    ui->axis1ComboBox->clear();
    ui->axis2ComboBox->clear();
    ui->axis1ComboBox->addItems(columns);
    ui->axis2ComboBox->addItems(columns);
    ui->sqlFilter->setTable(arg1);
}


void SqlPlotChart::on_exportImageButton_clicked()
{
    QString file = QFileDialog::getSaveFileName(this, tr("Save plot image"), QDir::homePath(), tr("PNG (*.png)"));
    if (file.isEmpty())
        return;

    QImage img(500, 500, QImage::Format_ARGB32);
    QPainter painter(&img);
    painter.fillRect(QRect(QPoint(0,0), img.size()), QBrush(Qt::white));
    switch (ui->chartTypeComboBox->currentIndex()) {
    case 0:
        m_scatterPlotter.plot(painter, QRect(QPoint(0,0), img.size()));
        break;
    case 1:
        m_histogramPlotter.plot(painter, QRect(QPoint(0,0), img.size()));
        break;
    }

    img.save(file);
}
