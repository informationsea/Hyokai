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

#include "main.h"
#include "sheetmessagebox.h"

#define SAFE_DELETE(x) if(x){delete x;x = NULL;}

SqlPlotChart::SqlPlotChart(QSqlDatabase *database, QWidget *parent, const QString &defaultName) :
    QDialog(parent),
    ui(new Ui::SqlPlotChart), m_database(database), m_rcode(0), m_rpng(0), m_rdata(0)
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
    setMaximumSize(size());
    refreshTables();
}

SqlPlotChart::~SqlPlotChart()
{
    delete ui;
    SAFE_DELETE(m_rcode);
    SAFE_DELETE(m_rpng);
    SAFE_DELETE(m_rdata);
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

QString SqlPlotChart::generateRcode(const QString &device)
{
    if (m_rdata == 0)
        return "";

    QString rcode;

    rcode += "library(lattice)\n";
    rcode += QString("axis.name <- list(\"%1\", \"%2\")\n").arg(ui->axis1ComboBox->currentText().replace("\"","\\\""), ui->axis2ComboBox->currentText().replace("\"","\\\""));

    rcode += QString("table.data.raw <- scan(\"%1\", list(%2, %3))\n").arg(m_rdata->fileName(),
                                                                           m_fields_type[0] == FIELD_NUMERIC ? "double()" : "character()",
                                                                           m_fields_type[1] == FIELD_NUMERIC ? "double()" : "character()");
    rcode += "table.data <- list(NA, NA)\n";
    for (int i = 0; i < 2; i++) {
        if (m_fields_type[i] == FIELD_NUMERIC)
            rcode += QString("table.data[[%1]] <- table.data.raw[[%1]]\n").arg(QString::number(i+1));
        else
            rcode += QString("table.data[[%1]] <- as.factor(table.data.raw[[%1]])\n").arg(QString::number(i+1));
    }

    rcode += device + "\n";

    switch (ui->chartTypeComboBox->currentIndex()) {
    case 0: { // scatter plot
        switch(ui->pchComboBox->currentIndex()) {
        case 0:
        default:
            rcode += "pch <- 1\n";
             break;
        case 1:
            rcode += "pch <- 20\n";
            break;
        case 2:
            rcode += "pch <- 4\n";
            break;
        case 3:
            rcode += "pch <- 3\n";
            break;
        case 4:
            rcode += "pch <- 2\n";
            break;
        case 5:
            rcode += "pch <- 17\n";
            break;
        case 6:
            rcode += "pch <- 0\n";
            break;
        case 7:
            rcode += "pch <- 15\n";
            break;
        }

        rcode += "alpha <- "+QString::number(ui->alphaSpin->value())+"\n";

        if (m_fields_type[0] == FIELD_NUMERIC && m_fields_type[1] == FIELD_NUMERIC) {
            rcode += "cor.result <- cor.test(table.data[[1]], table.data[[2]])\n"
                    "xyplot(table.data[[2]] ~ table.data[[1]], alpha=alpha, pch=pch, grid=T, xlab=axis.name[[1]], ylab=axis.name[[2]], main=sprintf(\"Correlation: %f   p-value: %f\", cor.result[[4]], cor.result[[3]]))\n";
        } else {
            rcode += "xyplot(table.data[[2]] ~ table.data[[1]], alpha=alpha, pch=pch, grid=T, xlab=axis.name[[1]], ylab=axis.name[[2]])\n";
        }
        break;
    }
    case 1: { // heatmap
        rcode += "library(gregmisc)\n"
                "cor.result <- cor.test(table.data[[1]], table.data[[2]])\n"
                "h2c <- hist2d(table.data[[1]], table.data[[2]], show=F, same.scale=F, nbins=c(20, 20))\n"
                "print(image(h2c$x, h2c$y, log2(h2c$count + 1), xlab=axis.name[[1]], ylab=axis.name[[2]], main=sprintf(\"Correlation: %f   p-value: %f\", cor.result[[4]], cor.result[[3]])))\n";
        break;
    }
    case 2: { // histogram
        rcode += "nint <- "+QString::number(ui->binSpin->value())+"\n";
        rcode += "histogram(table.data[[1]], nint=nint, xlab=axis.name[[1]])\n";
        break;
    }
    case 3: { // densityplot
        rcode += "densityplot(table.data[[1]], xlab=axis.name[[1]])\n";
        break;
    }
    }

    if (!device.isEmpty())
        rcode += "dev.off()\n";
    return rcode;
}

void SqlPlotChart::writeTable()
{
    if (m_rdata == 0) {
        m_rdata = new QTemporaryFile();
        m_rdata->open();
    }
    m_rdata->seek(0);
    m_rdata->resize(0);

    m_fields_type[0] = FIELD_NUMERIC;
    m_fields_type[1] = FIELD_NUMERIC;

    if (ui->axis1ComboBox->currentText().isEmpty())
        return;

    QSqlQuery query = m_database->exec(QString("SELECT %1, %2 FROM %3 %4").arg(ui->axis1ComboBox->currentText(),
                                                                               ui->axis2ComboBox->currentText(),
                                                                               ui->tableComboBox->currentText(),
                                                                               ui->sqlFilter->toPlainText().isEmpty() ? "" : QString("WHERE %1").arg(ui->sqlFilter->toPlainText())));
    if (query.lastError().type() != QSqlError::NoError) {
        SheetMessageBox::critical(this, tr("Cannot select table"), query.lastError().text());
        return;
    }

    if (query.first()) {
        do {
            for (int i = 0; i < 2; i++) {
                QVariant value = query.value(i);
                switch(value.type()) {
                case QVariant::Int:
                case QVariant::Double:
                case QVariant::LongLong:
                    m_rdata->write(value.toByteArray());
                    break;
                default: {
                    QByteArray data = value.toByteArray();
                    data.replace("\"", "\"\"");
                    m_rdata->write("\"");
                    m_rdata->write(data);
                    m_rdata->write("\"");
                    m_fields_type[i] = FIELD_STRING;
                    break;
                }
                }
                if (i == 0)
                    m_rdata->write("\t");
            }
            m_rdata->write("\n");
        } while (query.next());
    }

    m_rdata->flush();
    qDebug() << m_rdata->fileName();
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
    case 1:
        ui->axis2ComboBox->setEnabled(true);
        break;
    case 2:
    case 3:
        ui->axis2ComboBox->setEnabled(false);
        break;
    }


    ui->alphaSpin->setEnabled(index == 0);
    ui->pchComboBox->setEnabled(index == 0);
    ui->binSpin->setEnabled(index == 2);
}

void SqlPlotChart::on_plotButton_clicked()
{
    SAFE_DELETE(m_rcode);
    SAFE_DELETE(m_rpng);

    writeTable();

    m_rcode = new QTemporaryFile(this);
    m_rpng = new QTemporaryFile(this);
    m_rcode->open();
    m_rpng->open();

    QString code = generateRcode(QString("png(\"%1\")").arg(m_rpng->fileName()));

    m_rcode->write(code.toUtf8());
    m_rcode->flush();

    QStringList args;
    args << m_rcode->fileName();
    int retcode = QProcess::execute(tableview_settings->value(PATH_R, suggestRPath()).toString(), args);

    if (retcode) {
        SheetMessageBox::warning(this, tr("Cannot plot"), tr("Something wrong with R. If you are ploting heatmap, please confirm \"gregmisc\" package is installed."));
    }

    QImage img(m_rpng->fileName(), "PNG");
    ui->imageView->setImage(img);
    ui->imageView->repaint();

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

void SqlPlotChart::on_exportButton_clicked()
{
    writeTable();
    QString code = generateRcode("");
    QApplication::clipboard()->setText(code.left(code.size()-1));
}

void SqlPlotChart::on_exportImageButton_clicked()
{
    QString file = QFileDialog::getSaveFileName(this, tr("Save plot image"), QDir::homePath(), tr("PNG (*.png);;PDF (*.pdf);;Post Script (*.ps);;SVG (*.svg)"));
    if (file.isEmpty())
        return;

    SAFE_DELETE(m_rcode);

    writeTable();

    m_rcode = new QTemporaryFile(this);
    m_rcode->open();

    QString device;
    if (file.endsWith(".png")) {
        device = QString("png(\"%1\")").arg(file);
    } else if (file.endsWith(".pdf")) {
        device = QString("cairo_pdf(\"%1\")").arg(file);
    } else if (file.endsWith(".svg")) {
        device = QString("svg(\"%1\")").arg(file);
    } else if (file.endsWith(".ps")) {
        device = QString("cairo_ps(\"%1\")").arg(file);
    } else {
        device = QString("png(\"%1.png\")").arg(file);;
    }

    QString code = generateRcode(device);

    m_rcode->write(code.toUtf8());
    m_rcode->flush();

    QStringList args;
    args << m_rcode->fileName();
    int retcode = QProcess::execute(tableview_settings->value(PATH_R, suggestRPath()).toString(), args);

    if (retcode) {
        SheetMessageBox::warning(this, tr("Cannot plot"), tr("Something wrong with R. If you are ploting heatmap, please confirm \"gregmisc\" package is installed."));
    }
}
