#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QLabel>
#include <QTableView>

#include "sqltablemodelalternativebackground.h"
#include "customsqldialog.h"
#include "sqlplotchart.h"
#include "hyokaiconfig.h"
#include "tableviewstyleditemdelegate.h"
#include "sqlhistoryhelper.h"
#include "tableviewstatus.h"

#ifdef ENABLE_MAC_NATIVE_TOOLBAR
class QMacNativeToolBar;
#endif

class SqlHistoryHelper;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = nullptr, QString path = ":memory:");
    explicit MainWindow(const QSqlDatabase &database, QWidget *parent = nullptr);
    ~MainWindow();

    QString databaseName() { return m_databasename; }
    QSqlDatabase &database() {return m_database;}
    void refresh();
    bool isDirty() { return m_isDirty; }
    TableViewState saveTableState();
    void restoreTableState(const TableViewState &state);
    void resetTableState();

protected:
    bool eventFilter(QObject *obj, QEvent *ev);
    virtual void dragEnterEvent(QDragEnterEvent * event);
    virtual void dropEvent(QDropEvent * event);
    
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
    bool m_isClosing;
    QList<QDialog *> m_dialogs;
    QMenu m_assistPopup;
    TableViewStyledItemDelegate *m_tableViewItemDelegate1;
    TableViewStyledItemDelegate *m_tableViewItemDelegate2;
    SqlHistoryHelper *m_historyHelper;
    int m_splitColumn;
    bool m_tabChanging;
    QMap<QString, TableViewState> m_tableState;
    QFont m_defaultTableFont;
    int m_fontSize;
    int m_defaultVerticalSectionSize;

    void initialize();

    void open(QString path);
    bool confirmDuty(); // return false if canceled
    void setupTableModel();

    void popupHeaderContextMenu(QPoint globalPos, int logicalIndex, QTableView *tableView);
    QFont updateTableFont();

    QWidgetList m_windowList;

public slots:
    virtual void closeEvent(QCloseEvent *event);

private slots:
    void onChangeColumnOrRowSize();
    void filterFinished();
    void filterChainging();
    void tableTabChanged(int index);
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
    void colmunResized(int column, int oldWidth, int newWidth);
    void copyColumnName();
    void createIndexForColumn();
    void insertSqlFilter();
    void replaceSqlFilter();
    void onCopyTriggered(bool withHeader);
    void showCell();
    void cleanupDatabase();
    void importFinished(QStringList importedTables, bool withError, QString errorMessage);
    void setNumDecimalPlaces();
    void resetNumDecimalPlaces();
    void onToolbarVisibiltyChanged();
    void onTableViewScrollMoved(int value);
    void showFilterDialog();
    void showFilterDialogWithData();
    void addFilterAccepted();

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
    void on_actionUse_fixed_width_font_triggered(bool checked);
    void on_actionShow_Toolbar_triggered(bool checked);
    void on_actionShow_Table_List_View_triggered();
    void on_columnListSearchClear_clicked();
    void on_actionShow_Column_List_View_triggered();
    void on_columnListWidget_visibilityChanged(bool visible);
    void on_tableListWidget_visibilityChanged(bool visible);
    void on_columnListView_currentRowChanged(int currentRow);
    void on_splitter_splitterMoved(int pos, int index);
    void on_actionSplit_Window_triggered(bool checked);
    void on_columnListView_customContextMenuRequested(const QPoint &pos);
    void on_actionZoom_triggered();
    void on_actionUnzoom_triggered();
    void on_actionReset_font_size_triggered();
};

#endif // MAINWINDOW_H
