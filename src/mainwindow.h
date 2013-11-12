#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QLabel>

#include "sqltablemodelalternativebackground.h"
#include "customsqldialog.h"
#include "sqlplotchart.h"
#include "hyokaiconfig.h"
#include "tableviewstyleditemdelegate.h"

#ifdef ENABLE_MAC_NATIVE_TOOLBAR
class QMacNativeToolBar;
#endif

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0, QString path = ":memory:");
    explicit MainWindow(const QSqlDatabase &database, QWidget *parent = 0);
    ~MainWindow();

    QString databaseName() { return m_databasename; }
    QSqlDatabase &database() {return m_database;}
    void refresh();
//    void importOneFile(const QString &path);
    bool isDirty() { return m_isDirty; }

protected:
    bool eventFilter(QObject *obj, QEvent *ev);
    
private:
    Ui::MainWindow *ui;
#ifdef ENABLE_MAC_NATIVE_TOOLBAR
#ifdef Q_OS_MACX
#if QT_VERSION >= 0x050000
    QMacNativeToolBar *nativeToolbar;
#endif
#endif
#endif

    QSqlDatabase m_database;
    SqlTableModelAlternativeBackground *m_tableModel;
    QLabel *m_rowcountlabel;
    QString m_databasename;
    bool m_isDirty;
    QList<QDialog *> m_dialogs;
    QMenu m_assistPopup;
    TableViewStyledItemDelegate *m_tableViewItemDelegate;

    void initialize();

    void open(QString path);
    bool confirmDuty(); // return false if canceled
//    QString importFile(QString path, bool autoimport);
    void setupTableModel();

    QWidgetList m_windowList;

public slots:
    virtual void closeEvent(QCloseEvent *event);

private slots:
    void onChangeColumnOrRowSize();
    void filterFinished();
    void filterChainging();
    void tableChanged(const QString &name);
    void sortIndicatorChanged(int logicalIndex, Qt::SortOrder order);
    void tableUpdated();
    void updateDatabase();
    void onWindowMenuShow();
    void onRecentFileShow();
    void onRecentFileOpen();
    void onClearRecentFiles();
    void onShowHiddenColumnShow();
    void activate();
    void showColumnSummary();
    void showColumn();
    void hideColumn();
    void createIndexForColumn();
    void insertSqlFilter();
    void onCopyTriggered(bool withHeader);
    void showCell();
    void cleanupDatabase();
    void importFinished(QStringList importedTables, bool withError, QString errorMessage);
    void setNumDecimalPlaces();
    void resetNumDecimalPlaces();

    void on_actionGo_github_triggered();
    void on_actionCommit_triggered();
    void on_actionRevert_triggered();
    void on_actionCreateTable_triggered();
    void on_actionOpen_triggered();
    void on_actionNew_triggered();
    void on_actionInsert_triggered();
    void on_actionDelete_triggered();
    void on_actionQuit_triggered();
    void on_actionImportTable_triggered();
    void on_actionAbout_Qt_triggered();
    void on_actionAbout_Table_View_triggered();
    void on_actionRun_Custum_SQL_triggered();
    void on_actionRefresh_triggered();
    void on_actionExport_Table_triggered();
    void on_actionOpen_In_Memory_Database_triggered();
    void on_buttonClear_clicked();
    void on_actionAttach_Database_triggered();
    void on_actionCopy_triggered();
    void on_actionView_in_File_Manager_triggered();
    void on_actionPreference_triggered();
    void on_actionR_code_to_import_triggered();
    void on_buttonAssist_clicked();
    void on_actionGo_to_SQLite3_webpage_triggered();
    void on_actionDrop_Table_triggered();
    void on_actionCopy_with_header_triggered();
    void on_actionSelect_All_triggered();
    void on_actionConnect_to_database_triggered();
    void on_actionDuplicate_connection_triggered();
    void on_actionDatabase_Information_triggered();
    void on_actionDuplicate_Table_triggered();
    void on_actionPlot_triggered();
    void on_actionClose_triggered();
    void on_actionGo_to_Hyokai_info_triggered();
};

#endif // MAINWINDOW_H
