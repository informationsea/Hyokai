#include "customsqldialog.h"
#include "ui_customsqldialog.h"

#include "sheetmessagebox.h"
#include "jointabledialog.h"
#include "sqlservice.h"
#include "sqlfileexporter.h"
#include "main.h"
#include "sqlhistoryhelper.h"
#include "sqlasynchronousexecutor.h"
#include "summarydialog2.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include <QPoint>
#include <QSqlRecord>
#include <QLineEdit>
#include <QComboBox>
#include <QStringList>
#include <QFileDialog>
#include <QClipboard>
#include <QDebug>

CustomSqlDialog::CustomSqlDialog(QSqlDatabase *database, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CustomSql), m_database(database),
    m_query(*database), m_querymodel(this), m_asynchronousExecutor(nullptr)
{
    ui->setupUi(this);
    ui->tableView->setModel(&m_querymodel);
#if QT_VERSION >= 0x050000
    ui->tableView->horizontalHeader()->setSectionsMovable(true);
#else
    ui->tableView->horizontalHeader()->setMovable(true);
#endif
    ui->tableView->installEventFilter(this);
    ui->tableView->horizontalHeader()->installEventFilter(this);
    setWindowTitle("SQL "+QFileInfo(database->databaseName()).completeBaseName());
    if (database->driverName() == "QSQLITE") {
        setWindowFilePath(database->databaseName());
    }

    ui->splitter->setStretchFactor(0, 0);
    ui->splitter->setStretchFactor(1, 1);

    ui->sql->setDatabase(m_database);

    connect(ui->sql, SIGNAL(returnPressed()), SLOT(on_pushButton_clicked()));

    createMenus();

    m_history = tableview_settings->value(CUSTOM_SQL_HISTORY).toStringList();

    m_historyHelper = new SqlHistoryHelper(database, this);
}

CustomSqlDialog::~CustomSqlDialog()
{
    delete ui;
    delete assistMenu;
    delete historyMenu;
    delete menu;
    delete m_historyHelper;
    delete m_asynchronousExecutor;
}

QTableView *CustomSqlDialog::tableView()
{
    return ui->tableView;
}

bool CustomSqlDialog::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == ui->tableView && ev->type() == QEvent::ContextMenu) {
        QContextMenuEvent *cev = static_cast<QContextMenuEvent *>(ev);
        QPoint pos = cev->pos();
        pos.rx() -= ui->tableView->verticalHeader()->width();
        pos.ry() -= ui->tableView->horizontalHeader()->height();
        QModelIndex index = ui->tableView->indexAt(pos);
        if (index.isValid()) {
            cev->accept();
            QMenu popup(this);
            popup.move(cev->globalPos());
            QAction *showContent = popup.addAction(tr("Copy"), parent(), SLOT(on_actionCopy_triggered()));
            showContent->setData(index);
            QAction *showCellContent = popup.addAction(tr("Show"), this, SLOT(showCell()));
            showCellContent->setData(index);
            popup.exec();
            return true;
        }
    } else if (obj == ui->tableView->horizontalHeader() && ev->type() == QEvent::ContextMenu) {
        if (!m_query.isSelect())
            return false;

        QContextMenuEvent *cev = static_cast<QContextMenuEvent *>(ev);
        cev->accept();
        int logical_index = ui->tableView->horizontalHeader()->logicalIndexAt(cev->pos());
        if (logical_index >= 0) {
            QMenu popup(this);
            popup.move(cev->globalPos());
            QAction *showSummary = popup.addAction(tr("Summary"), this, SLOT(onShowSummary()));
            showSummary->setData(logical_index);
            QAction *sortAscending = popup.addAction(tr("Sort in ascending"), this, SLOT(onSortAscending()));
            sortAscending->setData(logical_index);
            QAction *sortDescending = popup.addAction(tr("Sort in descending"), this, SLOT(onSortDescending()));
            sortDescending->setData(logical_index);
            popup.exec();
        }
        return true;
    } else if (obj == ui->tableView && ev->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(ev);
        if (keyEvent->key() == Qt::Key_C && keyEvent->modifiers() == Qt::ControlModifier) {
            //qDebug() << "event filter" << obj << keyEvent;
            SqlService::copyFromTableView(ui->tableView, false);
            return true;
        }
    }
    return QObject::eventFilter(obj, ev);
}
void CustomSqlDialog::showCell()
{
    QAction *action = static_cast<QAction*>(sender());
    QModelIndex index = action->data().toModelIndex();
    QString header = m_querymodel.headerData(index.column(), Qt::Horizontal).toString();
    SheetMessageBox::information(this, tr("%1, #%2").arg(header, QString::number(index.row()+1)), m_querymodel.data(index).toString());
}

void CustomSqlDialog::onShowSummary()
{
    QAction *action = static_cast<QAction*>(sender());
    int logicalIndex = action->data().toInt();

    QSqlRecord record = m_query.record();
    SummaryDialog2 *summary2 = new SummaryDialog2(m_database, "(" + m_query.lastQuery() + ")", record.fieldName(logicalIndex), "", parentWidget());
    summary2->show();
}

void CustomSqlDialog::onSortAscending()
{
    QAction *action = static_cast<QAction*>(sender());
    int logicalIndex = action->data().toInt();

    QSqlRecord record = m_query.record();
    QString newQueryText = QString("SELECT * FROM (%2) ORDER BY %1").arg(record.fieldName(logicalIndex), m_query.lastQuery());
    qDebug() << newQueryText;

    QSqlQuery newquery = m_database->exec(newQueryText);

    if  (newquery.lastError().isValid()) {
        SheetMessageBox::critical(this, tr("SQL Error"), newquery.lastError().text());
        return;
    }

    m_querymodel.setQuery(newquery);
}

void CustomSqlDialog::onSortDescending()
{
    QAction *action = static_cast<QAction*>(sender());
    int logicalIndex = action->data().toInt();

    QSqlRecord record = m_query.record();
    QString newQueryText = QString("SELECT * FROM (%2) ORDER BY %1 DESC").arg(record.fieldName(logicalIndex), m_query.lastQuery());
    qDebug() << newQueryText;

    QSqlQuery newquery = m_database->exec(newQueryText);

    if  (newquery.lastError().isValid()) {
        SheetMessageBox::critical(this, tr("SQL Error"), newquery.lastError().text());
        return;
    }

    m_querymodel.setQuery(newquery);
}

void CustomSqlDialog::selectTableAll()
{
    ui->tableView->selectAll();
}

void CustomSqlDialog::on_pushButton_clicked()
{
    if(ui->sql->toPlainText().isEmpty())
        return;
    m_query.prepare(ui->sql->toPlainText());

    delete m_asynchronousExecutor;
    m_asynchronousExecutor = new SqlAsynchronousExecutor(&m_query, this);
    connect(m_asynchronousExecutor, SIGNAL(finishQuery(QSqlQuery*,SqlAsynchronousExecutor*)), SLOT(finishQuery(QSqlQuery*,SqlAsynchronousExecutor*)));

    ui->pushButton->setEnabled(false);
    ui->historyButton->setEnabled(false);
    ui->menuButton->setEnabled(false);
    ui->assistButton->setEnabled(false);
    ui->sql->setEnabled(false);

    m_asynchronousExecutor->start();
}


void CustomSqlDialog::finishQuery(QSqlQuery *query, SqlAsynchronousExecutor *executor)
{
    ui->pushButton->setEnabled(true);
    ui->historyButton->setEnabled(true);
    ui->menuButton->setEnabled(true);
    ui->assistButton->setEnabled(true);
    ui->sql->setEnabled(true);

    if (query->lastError().type() != QSqlError::NoError) {
        SheetMessageBox::critical(this, tr("SQL Error"), m_query.lastError().driverText() + "\n" + m_query.lastError().databaseText()+"\n\n"+m_query.lastQuery());
        return;
    }

    ui->sql->setStyleSheet("SQLTextEdit{ background: white; color: black; }");

    if (m_history.isEmpty() || m_history.first() != m_query.lastQuery()) {
        m_history.insert(0, m_query.lastQuery());
        m_history.removeDuplicates();
        tableview_settings->setValue(CUSTOM_SQL_HISTORY, m_history);
    }

    m_historyHelper->insertCustomSqlHistory(m_query.lastQuery());

    if (m_query.isSelect()) {
        m_querymodel.setQuery(m_query);
    } else {
        SheetMessageBox::information(this, tr("SQL Report"), tr("The query SQL was finished successfully.") + "\n\n" + m_query.lastQuery());
    }
}

void CustomSqlDialog::on_assistButton_clicked()
{
    QPoint point = ui->assistButton->pos();
    point.setY(point.y() + ui->assistButton->size().height());
    assistMenu->popup(mapToGlobal(point));
}

void CustomSqlDialog::setSqlTemplate()
{
    QAction *sender = static_cast<QAction*>(QObject::sender());
    ui->sql->setPlainText(sender->data().toString());
}


void CustomSqlDialog::insertSql()
{
    QAction *sender = static_cast<QAction*>(QObject::sender());
    ui->sql->insertPlainText(sender->data().toString());
}

void CustomSqlDialog::joinSqlWizard()
{
    JoinTableDialog dialog(m_database, this);
    if (dialog.exec() != QDialog::Accepted)
        return;
    ui->sql->setPlainText(dialog.sql());
    on_pushButton_clicked();
}

#define CLEAR_TEXT "**CLEAR**"
#define MAX_HISTORY_SHOW_LENGTH 130
#define MAX_HISTORY_SHOW_ITEMS 10
#define MAX_HISTORY_SHOW_MORE_ITEMS 60

void CustomSqlDialog::on_historyButton_clicked()
{
    int count = 0;
    historyMenu->clear();

    QStringList histories = m_historyHelper->customHistory();
    foreach (QString query, histories) {
        QString showName = query;
        if (showName.length() > MAX_HISTORY_SHOW_LENGTH)
            showName = query.left(MAX_HISTORY_SHOW_LENGTH/2-2) + QString(" ... ") + query.right(MAX_HISTORY_SHOW_LENGTH/2-2);

        QAction *action = historyMenu->addAction(showName);
        action->setData(query);
        connect(action, SIGNAL(triggered()), SLOT(onHistorySelected()));
    }

    if (!histories.isEmpty())
        historyMenu->addSeparator();\

    QMenu *additionalHistoy = historyMenu->addMenu(tr("All History"));;
    foreach(QString query, m_history) {
        QAction *action;

        QString showName = query;
        if (showName.length() > MAX_HISTORY_SHOW_LENGTH) {
            showName = query.left(MAX_HISTORY_SHOW_LENGTH/2-2) + QString(" ... ") + query.right(MAX_HISTORY_SHOW_LENGTH/2-2);
        }

        if (count < MAX_HISTORY_SHOW_MORE_ITEMS)
            action = additionalHistoy->addAction(showName);
        else
            break;

        action->setData(query);
        connect(action, SIGNAL(triggered()), SLOT(onHistorySelected()));
        count += 1;
    }
    historyMenu->addSeparator();
    QAction *clear = historyMenu->addAction(tr("Clear"));
    clear->setData(CLEAR_TEXT);
    connect(clear, SIGNAL(triggered()), SLOT(onHistorySelected()));

    QPoint point = ui->historyButton->pos();
    point.setY(point.y() + ui->historyButton->size().height());
    historyMenu->popup(mapToGlobal(point));
}

void CustomSqlDialog::onHistorySelected()
{
    QAction *senderAction = static_cast<QAction*>(sender());
    if (senderAction->data().toString().compare(CLEAR_TEXT) == 0) {
        m_history.clear();
        tableview_settings->setValue(CUSTOM_SQL_HISTORY, m_history);
    } else {
        ui->sql->setPlainText(senderAction->data().toString());
    }
}

void CustomSqlDialog::on_menuButton_clicked()
{
    foreach(QAction* action, m_menu_for_select) {
        action->setEnabled(m_query.isSelect());
    }

    QPoint point = ui->menuButton->pos();
    point.setY(point.y() + ui->menuButton->size().height());
    menu->popup(mapToGlobal(point));
}

void CustomSqlDialog::onExportTable()
{
    QString outputpath = QFileDialog::getSaveFileName(this, tr("Export as text"),
                                                      tableview_settings->value(LAST_EXPORT_DIRECTORY, QDir::homePath()).toString(),
                                                      "CSV (*.csv);; Tab separated (*.txt)");
    if (outputpath.isEmpty())
        return;
    if (!m_query.isSelect())
        return;

    SqlFileExporter exporter(nullptr, this);
    if (!exporter.exportTable(m_query, outputpath, FileTypeUtil::getFileTypeFromPath(outputpath)))
        SheetMessageBox::critical(this, tr("Cannot export table"), exporter.errorMessage());
}

void CustomSqlDialog::onCreateView()
{
    ui->sql->setPlainText(QString("CREATE VIEW newview AS %1").arg(m_query.lastQuery()));
    ui->sql->textCursor().setPosition(QString("CREATE VIEW ").length());
    ui->sql->textCursor().setPosition(QString("CREATE VIEW newview").length(), QTextCursor::KeepAnchor);
}

void CustomSqlDialog::onExportToR()
{
    QClipboard *clip = qApp->clipboard();
    clip->setText(SqlService::createRcodeToImport(*m_database, ui->sql->toPlainText(), "custom.table"));
}

void CustomSqlDialog::onFixedFontToggled(bool checked)
{
    QFont font;
    if (checked) {
        font.setFamily("Monaco");
        font.setStyleHint(QFont::Monospace);
    } else {
        font = QApplication::font();
    }
    ui->tableView->setFont(font);
}


void CustomSqlDialog::createMenus()
{
    // Assist Menu
    assistMenu = new QMenu(this);

    QStringList templateList;
    templateList << "SELECT * FROM table"
                 << "SELECT * FROM table1 INNTER JOIN table2 ON table1.column1 = table2.column2"
                 << "SELECT * FROM table1 LEFT OUTER JOIN table2 ON table1.column1 = table2.column2"
                 << "SELECT * FROM table1 NATUAL INNTER JOIN table2"
                 << "SELECT * FROM table1 NATUAL LEFT OUTER JOIN table2"
                 << "DROP TABLE table1"
                 << "DROP VIEW view1"
                 << "DROP INDEX index1"
                 << "CREATE VIEW view1 AS SELECT *"
                 << "CREATE INDEX index1 ON table1(column1)"
                 << "VACUUM";

    QAction *joinwizard = assistMenu->addAction(tr("SQL Join Wizard"));
    connect(joinwizard, SIGNAL(triggered()), SLOT(joinSqlWizard()));

    QMenu *sqltemplates = assistMenu->addMenu(tr("Replace with SQL Templates"));
    foreach(QString i, templateList) {
        QAction *action = sqltemplates->addAction(i);
        action->setData(i);
        connect(action, SIGNAL(triggered()), SLOT(setSqlTemplate()));
    }

    QStringList userTemplates = tableview_settings->value(SQL_TEMPLATES).toStringList();
    if (!userTemplates.isEmpty()) {
        QMenu *usertemplatemenu = assistMenu->addMenu(tr("User Templates"));
        foreach (QString i, userTemplates) {
            QAction *action = usertemplatemenu->addAction(i);
            action->setData(i);
            connect(action, SIGNAL(triggered()), SLOT(insertSql()));
        }
    }

    QStringList templateList2;
    templateList2 << " ORDER BY column ASC"
                  << " ORDER BY column DESC"
                  << " WHERE column == 0"
                  << " WHERE column < 0"
                  << " WHERE column > 0"
                  << " WHERE column LIKE pattern"
                  << " WHERE column GLOB pattern"
                  << " LIMIT limit"
                  << " OFFSET offset"
                  << " GROUP BY column";

    QMenu *sqltemplateinsert = assistMenu->addMenu(tr("Insert SQL Templates"));
    foreach(QString i, templateList2) {
        QAction *action = sqltemplateinsert->addAction(i);
        action->setData(i);
        connect(action, SIGNAL(triggered()), SLOT(insertSql()));
    }

    QStringList commonFunctions;
    commonFunctions << "avg(";
    commonFunctions << "distinct(";
    commonFunctions << "max(";
    commonFunctions << "min(";
    commonFunctions << "count(";
    commonFunctions << "total(";

    QMenu *commonFunctionMenu = assistMenu->addMenu(tr("Common SQL Functions"));
    foreach(QString i, commonFunctions) {
        if (i == "--") {
            commonFunctionMenu->addSeparator();
        } else {
            QAction *action = commonFunctionMenu->addAction(i);
            action->setData(i);
            connect(action, SIGNAL(triggered()), SLOT(insertSql()));
        }
    }


    //QFile functions(":/txt/functionlist.txt");
    //functions.open(QIODevice::ReadOnly);

    QMenu *functioninsert = assistMenu->addMenu(tr("All SQL Functions"));
    //QByteArray functionLine;
    //while (!(functionLine = functions.readLine()).isEmpty()) {
    foreach(QString func, SQLTextEdit::loadFunctionList(m_database->driverName())) {
        //QString func(functionLine);
        if (func.startsWith(">")) {
            functioninsert->addSeparator();
            QAction *action = functioninsert->addAction(func.right(func.size()-1));
            action->setEnabled(false);
        } else {
            QAction *action = functioninsert->addAction(func.trimmed());
            action->setData(func.trimmed());
            connect(action, SIGNAL(triggered()), SLOT(insertSql()));
        }
    }

    QMenu *tables = assistMenu->addMenu(tr("Insert table name"));
    foreach(QString i, m_database->tables(QSql::AllTables)) {
        QAction *action = tables->addAction(i);
        action->setData(i);
        connect(action, SIGNAL(triggered()), SLOT(insertSql()));
    }

    QMenu *column = assistMenu->addMenu(tr("Insert column name"));
    foreach(QString i, m_database->tables(QSql::AllTables)) {
        QSqlRecord record = m_database->record(i);
        QMenu *one = column->addMenu(i);
        for(int j = 0; j < record.count(); j++) {
            QAction *action = one->addAction(record.fieldName(j));
            action->setData(record.fieldName(j));
            connect(action, SIGNAL(triggered()), SLOT(insertSql()));
        }
    }

    // History Menu
    historyMenu = new QMenu(this);

    // Other Menu
    menu = new QMenu(this);

    QAction *fixedfont = menu->addAction(tr("Use fixed width font"));
    connect(fixedfont, SIGNAL(triggered(bool)), SLOT(onFixedFontToggled(bool)));
    fixedfont->setCheckable(true);

    QAction* exportTable = menu->addAction(tr("Export Table"));
    connect(exportTable, SIGNAL(triggered()), SLOT(onExportTable()));
    m_menu_for_select.append(exportTable);

    QAction* createView = menu->addAction(tr("Create View for this table"));
    connect(createView, SIGNAL(triggered()), SLOT(onCreateView()));
    m_menu_for_select.append(createView);

    if (m_database->driverName() == "QSQLITE" && m_database->databaseName() != ":memory:") {
        QAction* exportToR = menu->addAction(tr("Export to R"));
        connect(exportToR, SIGNAL(triggered()), SLOT(onExportToR()));
        m_menu_for_select.append(exportToR);
    }
}

void CustomSqlDialog::on_sql_textChanged()
{
    if (ui->sql->toPlainText().trimmed() == m_query.lastQuery()) {
        ui->sql->setStyleSheet("SQLTextEdit{ background: white; color: black; }");
    } else {
        ui->sql->setStyleSheet("SQLTextEdit{ background: #FAFFC5; color: black;}");
    }
}
