#include "summarydialog.h"
#include "ui_summarydialog.h"

#include <cmath>
#include <QVariant>
#include <QtAlgorithms>
#include <QClipboard>
#include <QProcess>
#include <QDebug>
#include <QStringList>
#include <QRegExp>

#include "main.h"
#include "preferencewindow.h"

template<class T> QList<QVariant> convertToVariant(QList<T> s) {
    QList<QVariant> l;
    foreach (T v, s) {
        l.append(QVariant(v));
    }
    return l;
}

/* values should be sorted */
static QList<double> quantile(const QList<double> &values, const QList<double> &q)
{
    QList<double> result;
    foreach (double v, q) {
        if (v >= 1 || values.length() == 1) {
            result.append(values.last());
        } else {
            double pos = v*(values.length()-1);
            int integer = (int)pos;
            double decimal = pos - integer;
            result.append(values[integer]*(1-decimal) + values[integer+1]*decimal);
        }
    }
    return result;
}

SummaryDialog::SummaryDialog(const QList<double> &values, const QString &tableAndcolumnName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SummaryDialog),
    m_values(values),
    m_tableAndColumnName(tableAndcolumnName),
    m_rName(tableAndcolumnName),
    m_rdata_file(new QTemporaryFile()),
    m_rdraw_file(new QTemporaryFile()),
    m_rdraw_png(new QTemporaryFile()), m_histogram(0), m_histogram_brush(0)
{
    ui->setupUi(this);

    setWindowModality(Qt::NonModal);
    setWindowTitle(QString(tr("Summary of %1")).arg(tableAndcolumnName));

    m_rName.replace(QRegExp("[^a-zA-Z0-9\\._]"), "_");

    double sumValue = 0;

    foreach(double v, m_values) {
        sumValue += v;
    }

    double meanValue = sumValue/m_values.size();
    double sumDouble = 0;

    foreach(double v, m_values) {
        sumDouble += (v - meanValue)*(v - meanValue);
    }

    double sdValue;
    if (m_values.length() > 1) {
        sdValue = std::sqrt(sumDouble/(m_values.length()-1));
    } else {
        sdValue = -1;
    }

    qSort(m_values);

    QString quantile_text;
    if (m_values.size()) {
        QList<double> q;
        q << 0 << 0.1 << 0.2 << 0.3 << 0.4 << 0.5 << 0.6 << 0.7 << 0.8 << 0.9 << 1;

        QList<double> quantile_result = quantile(m_values, q);

        for (int i = 0; i < q.size(); ++i) {
            QString t;
            t.sprintf("%3d%%: %f\n", (int)(q[i]*100), quantile_result[i]);
            quantile_text.append(t);
        }
    }

    QString summaryText = tr("Summary of %1\n\nSum:  %5\nMean: %2\nSD:   %3\n\n%4").arg(tableAndcolumnName,
                                                                                        QString::number(meanValue),
                                                                                        QString::number(sdValue),
                                                                                        quantile_text,
                                                                                        QString::number(sumValue));

    ui->summaryText->setPlainText(summaryText);

    m_rdata_file->open();
    m_rdata_file->seek(0);
    foreach (double oneValue, values) {
        m_rdata_file->write((const char *)&oneValue, sizeof(oneValue));
    }
    m_rdata_file->flush();
    qDebug() << m_rdata_file->fileName();

    m_rdraw_file->open();
    m_rdraw_png->open();
    qDebug() << m_rName;
    m_drawRScript = QString("library(lattice)\n"
                            "file.%3 <- file(\"%1\", \"rb\")\n"
                            "data.%3 <- readBin(file.%3, \"double\", %4, 8)\n"
                            "png(\"%2\", width=400, height=400)\n"
                            "densityplot(data.%3, panel=function(...){panel.grid(h=-1, v=-1);panel.densityplot(...)})\n"
                            "close.connection(file.%3)\n"
                            "dev.off()").
                    arg(m_rdata_file->fileName(), m_rdraw_png->fileName(), m_rName, QString::number(values.length()));
    qDebug() << m_drawRScript;

    m_rdraw_file->write(m_drawRScript.toUtf8());
    m_rdraw_file->flush();

    QStringList args;
    args << m_rdraw_file->fileName();
    int rt = QProcess::execute(tableview_settings->value(PATH_R, suggestRPath()).toString(), args);

    if (rt == 0) {
        QImage histogram(m_rdraw_png->fileName());
        ui->histogram->setImage(histogram);
        ui->histogram->repaint();
    }
}

SummaryDialog::~SummaryDialog()
{
    delete ui;
}

void SummaryDialog::closeEvent(QCloseEvent * /*event*/)
{
    delete m_rdata_file;
    delete m_rdraw_file;
    delete m_rdraw_png;
    if (m_histogram)
        delete m_histogram;
    if (m_histogram_brush)
        delete m_histogram_brush;
}

void SummaryDialog::on_buttonCopyImport_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString("library(lattice)\n"
                               "file.%2 <- file(\"%1\", \"rb\")\n"
                               "data.%2 <- readBin(file.%2, \"double\", %3, 8)\n"
                               "densityplot(data.%2, panel=function(...){panel.grid(h=-1, v=-1);panel.densityplot(...)})\n"
                               "close.connection(file.%2)\n").
                       arg(m_rdata_file->fileName(), m_rName, QString::number(m_values.length())));
}
