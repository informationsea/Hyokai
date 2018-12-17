#include "summarydialog2.h"
#include "ui_summarydialog2.h"

#include <QSqlRecord>
#include <QFileDialog>
#include <QDebug>
#include <cmath>

/* values should be sorted */
static QList<double> quantile(const QList<double> &values, const QList<double> &q)
{
    QList<double> result;
    foreach (double v, q) {
        if (v >= 1 || values.length() == 1) {
            result.append(values.last());
        } else {
            double pos = v*(values.length()-1);
            int integer = static_cast<int>(pos);
            double decimal = pos - integer;
            result.append(values[integer]*(1-decimal) + values[integer+1]*decimal);
        }
    }
    return result;
}

SummaryDialog2::SummaryDialog2(QSqlDatabase *database, QString fromValue, QString columnName, QString where, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SummaryDialog2),
	m_database(database),
	m_fromValue(fromValue),
	m_columnName(columnName),
	m_where(where),
	m_model(database, fromValue, columnName, where, this),
	m_sumValue(0),
	m_numberOfSkippedRow(0)
{
    ui->setupUi(this);

    setWindowModality(Qt::NonModal);
    setWindowTitle(QString("%1 / %2").arg(columnName, fromValue));

	// Summary
	ui->summaryTableView->setModel(&m_model);
	ui->summaryTableView->sortByColumn(1, Qt::SortOrder::DescendingOrder);

	// Distribution
	QSqlQuery distributionQuery;
	if (m_where.isEmpty()) {
        distributionQuery = QSqlQuery(QString("SELECT \"%1\" FROM %2").arg(columnName, fromValue), *m_database);
	}
	else {
        distributionQuery = QSqlQuery(QString("SELECT \"%1\" FROM %2 WHERE %3").arg(columnName, fromValue, m_where), *m_database);
	}

    bool ok = false;
	while (distributionQuery.next()) {
		auto raw_value = distributionQuery.record().value(0);
		if (raw_value.isNull() || raw_value.toString() == "") {
			m_numberOfSkippedRow += 1;
			continue;
		}

		double value = raw_value.toDouble(&ok);
		if (!ok) {
			break;
		}
		m_doubleList.append(value);
	}

	ui->tabWidget->setCurrentIndex(0);

    if (ok) {
        // Quantile
        qSort(m_doubleList);
        QList<double> q;
        const int quantileDivision = 200;
        for (int i = 0; i < quantileDivision; ++i) {
            q << static_cast<double>(i) / quantileDivision;
        }
        QList<double> quantileResult = quantile(m_doubleList, q);
        QList<QPointF> cummurativeResult;
        for (int i = 0; i < quantileDivision; i++) {
            cummurativeResult << QPointF(q[i], quantileResult[i]);
        }
        //qDebug() << cummurativeResult;
        // TODO: Add sum, SD, mean
        //ui->summaryCommentLabel->setText(QString("%1 rows are skipped").arg(m_numberOfSkippedRow));

		m_histogramPlotter.setData(m_doubleList);
		m_histogramPlotter.setXLabel(columnName);
		ui->distributionPlot->setPlotter(&m_histogramPlotter);

		m_linePlotter.setData(cummurativeResult);
        m_linePlotter.setXLabel("Ratio");
        m_linePlotter.setYLabel(columnName);
		ui->cummurativePlot->setPlotter(&m_linePlotter);

        // Sum
        double sumValue = 0;

        foreach(double v, m_doubleList) {
            sumValue += v;
        }

        double meanValue = sumValue/m_doubleList.size();
        double sumDouble = 0;

        foreach(double v, m_doubleList) {
            sumDouble += (v - meanValue)*(v - meanValue);
        }

        double sdValue;
        if (m_doubleList.length() > 1) {
            sdValue = std::sqrt(sumDouble/(m_doubleList.length()-1));
        } else {
            sdValue = -1;
        }

        ui->sumValue->setText(QString::number(sumValue));
        ui->sdValue->setText(QString::number(sdValue));
        ui->meanValue->setText(QString::number(meanValue));
        ui->ignoredValue->setText(QString::number(m_numberOfSkippedRow));
    }
    else {
        ui->tabWidget->tabBar()->hide();
		ui->tabWidget->removeTab(2);
		ui->tabWidget->removeTab(1);
		//ui->tab_2->setVisible(false);
		//ui->tab_3->setVisible(false);
	}
}

SummaryDialog2::~SummaryDialog2()
{
    delete ui;
}

void SummaryDialog2::on_binWidthSpinBox_valueChanged(double arg1)
{
	m_histogramPlotter.setInterval(arg1);
	ui->distributionPlot->repaint();
}

void SummaryDialog2::on_exportImageButton_clicked()
{
	QString file = QFileDialog::getSaveFileName(this, tr("Save plot image"), QDir::homePath(), tr("PNG (*.png)"));
	if (file.isEmpty())
		return;

	QImage img(500, 500, QImage::Format_ARGB32);
	QPainter painter(&img);
	painter.fillRect(QRect(QPoint(0, 0), img.size()), QBrush(Qt::white));
	m_histogramPlotter.plot(painter, QRect(QPoint(0, 0), img.size()));

	img.save(file);
}

void SummaryDialog2::on_exportImageCummurativeButton_clicked()
{
    QString file = QFileDialog::getSaveFileName(this, tr("Save plot image"), QDir::homePath(), tr("PNG (*.png)"));
    if (file.isEmpty())
        return;

    QImage img(500, 500, QImage::Format_ARGB32);
    QPainter painter(&img);
    painter.fillRect(QRect(QPoint(0, 0), img.size()), QBrush(Qt::white));
    m_linePlotter.plot(painter, QRect(QPoint(0, 0), img.size()));

    img.save(file);
}
