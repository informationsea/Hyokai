#include "preferencewindow.h"
#include "ui_preferencewindow.h"

#include <QVariant>
#include <QFileDialog>
#include <QDebug>

#include "attachdatabasedialog.h"
#include "customsql.h"
#include "sheetmessagebox.h"
#include "main.h"

#ifdef Q_OS_MACX
#if QT_VERSION >= 0x050000
#include <QMacNativeToolBar>
#endif
#endif

PreferenceWindow::PreferenceWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PreferenceWindow)
{
    ui->setupUi(this);
    ui->actionGeneral->setChecked(true);
    ui->stackedWidget->setCurrentIndex(0);

    m_attachmodel = new AttachDBTableModel(this);
    m_sqlmodel = new SqlTemplateTableModel(this);

    ui->tableAttachDB->setModel(m_attachmodel);
    ui->tableSqlTemplate->setModel(m_sqlmodel);

    ui->lineRPath->setText(tableview_settings->value(PATH_R, suggestRPath()).toString());


#ifdef Q_OS_MACX
#if QT_VERSION >= 0x050000
    QMacNativeToolBar *nativeToolbar = QtMacExtras::setNativeToolBar(ui->toolBar, true);
    nativeToolbar->setIconSize(QSize(32,32));
#endif
#endif

    move(nextWindowPosition());
}

PreferenceWindow::~PreferenceWindow()
{
    delete ui;
    delete m_attachmodel;
    delete m_sqlmodel;
}

void PreferenceWindow::activate()
{
    raise();
    activateWindow();
    setWindowFilePath(QString::null);
}

void PreferenceWindow::on_actionGeneral_triggered()
{
    uncheckAll();
    ui->actionGeneral->setChecked(true);
    ui->stackedWidget->setCurrentIndex(0);
}

void PreferenceWindow::on_actionAttach_DB_triggered()
{
    uncheckAll();
    ui->actionAttach_DB->setChecked(true);
    ui->stackedWidget->setCurrentIndex(1);
}

void PreferenceWindow::on_actionSQL_Templates_triggered()
{
    uncheckAll();
    ui->actionSQL_Templates->setChecked(true);
    ui->stackedWidget->setCurrentIndex(2);
}

void PreferenceWindow::uncheckAll()
{
    ui->actionAttach_DB->setChecked(false);
    ui->actionGeneral->setChecked(false);
    ui->actionSQL_Templates->setChecked(false);
}

void PreferenceWindow::on_pushButtonClearCustumSqlHistory_clicked()
{
    QStringList list;
    tableview_settings->setValue(CUSTOM_SQL_HISTORY, list);
}

AttachDBTableModel::AttachDBTableModel(QObject *parent)  : QAbstractTableModel(parent)
{
    QList<QVariant> templist = tableview_settings->value(ATTACHED_DATABASES).toList();
    list.clear();
    foreach(QVariant l, templist) {
        list << l.toStringList();
    }
}


QVariant AttachDBTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();
    switch (section) {
    case 1:
        return "DB";
    case 0:
        return "Attach As";
    }
    return QVariant();
}

QVariant AttachDBTableModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    return list[index.row()][index.column()];
}

Qt::ItemFlags AttachDBTableModel::flags(const QModelIndex &/*index*/) const
{
    return Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled;
}

bool AttachDBTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;
    list[index.row()][index.column()] = value.toString();
    saveList();
    return true;
}

bool AttachDBTableModel::insertAttachDB(const QString &db, const QString &as)
{
    foreach(QStringList i, list) {
        if (i[1] == as)
            return false;
    }

    QStringList l;
    l << as << db;
    beginInsertRows(QModelIndex(), list.size(), list.size());
    list.append(l);
    endInsertRows();

    saveList();
    return true;
}

bool AttachDBTableModel::removeRows(int row, int count, const QModelIndex & parent)
{
    beginRemoveRows(parent, row, row + count -1);
    for (int i = row + count - 1; i >= row; --i)
        list.removeAt(i);
    endRemoveRows();
    saveList();
    return true;
}

void AttachDBTableModel::saveList()
{
    QList<QVariant> templist;

    foreach(QStringList l, list) {
        templist << QVariant(l);
    }

    tableview_settings->setValue(ATTACHED_DATABASES, templist);
}

SqlTemplateTableModel::SqlTemplateTableModel(QObject *parent) : QAbstractTableModel(parent)
{
    sqllist = tableview_settings->value(SQL_TEMPLATES).toStringList();
}

QVariant SqlTemplateTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();
    switch (section) {
    case 0:
        return "SQL";
    }
    return QVariant();
}

QVariant SqlTemplateTableModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    return sqllist[index.row()];
}

Qt::ItemFlags SqlTemplateTableModel::flags(const QModelIndex &/*index*/) const
{
    return Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled;
}

bool SqlTemplateTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;
    sqllist[index.row()] = value.toString();
    saveList();
    return true;
}

bool SqlTemplateTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count -1);
    for (int i = row + count - 1; i >= row; --i)
        sqllist.removeAt(i);
    endRemoveRows();
    saveList();
    return true;
}

bool SqlTemplateTableModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count -1);
    for (int i = 0; i < count; ++i) {
        sqllist.append("");
    }
    endInsertRows();
    saveList();
    return true;
}

void SqlTemplateTableModel::saveList()
{
    QStringList savelist = sqllist;
    savelist.removeDuplicates();
    savelist.removeOne("");
    tableview_settings->setValue(SQL_TEMPLATES, savelist);
}

void PreferenceWindow::on_addAttachDB_clicked()
{
    AttachDatabaseDialog add(this);
    if (add.exec() != QDialog::Accepted)
        return;
    if (!m_attachmodel->insertAttachDB(add.databasePath(), add.attachAs())) {
        SheetMessageBox::critical(this, tr("Cannot attach"), tr("Cannot attach database because identifier is duplicated."));
    }
}

void PreferenceWindow::on_removeAttachDB_clicked()
{
    QList<int> rows = selectedRowsFromSelection(ui->tableAttachDB->selectionModel());
    while(!rows.isEmpty()) {
        m_attachmodel->removeRow(rows.takeLast());
    }
}


void PreferenceWindow::on_addSqlTemplate_clicked()
{
    m_sqlmodel->insertRow(m_sqlmodel->rowCount()-1);
}

void PreferenceWindow::on_removeSqlTemplate_clicked()
{
    QList<int> rows = selectedRowsFromSelection(ui->tableSqlTemplate->selectionModel());
    while(!rows.isEmpty()) {
        m_sqlmodel->removeRow(rows.takeLast());
    }
}

void PreferenceWindow::on_lineRPath_textChanged(const QString &arg1)
{
    if (arg1.endsWith("Rscript") || arg1.endsWith("Rscript.exe")) {
        tableview_settings->setValue(PATH_R, arg1);
    }
}

void PreferenceWindow::on_buttonRPath_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Select Rscript"), ui->lineRPath->text(), "Rscript (Rscript Rscript.exe)");
    if (path.isEmpty())
        return;
    tableview_settings->setValue(PATH_R, path);
    ui->lineRPath->setText(path);
}

void PreferenceWindow::on_pushButtonClearFilterHistory_clicked()
{
    tableview_settings->setValue(SQL_FILTER_HISTORY, QVariant());
}
