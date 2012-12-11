#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QLabel>

#include "custumsql.h"
#include "sqlite3.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0, QString path = ":memory:");
    ~MainWindow();

    QString filePath() { return m_filepath; }
    QString importFile(QString path, bool autoimport);
    void refresh();
    
private:
    Ui::MainWindow *ui;
    QSqlDatabase m_database;
    QSqlTableModel *m_tableModel;
    QLabel *m_rowcountlabel;
    QString m_filepath;
    bool m_isDuty;
    CustumSql *m_custumSql;

    void open(QString path);
    bool confirmDuty(); // return false if canceled

    QWidgetList m_windowList;

public slots:
    virtual void closeEvent(QCloseEvent *event);

private slots:
    void filterFinished();
    void tableChanged(const QString &name);
    void sortIndicatorChanged(int logicalIndex, Qt::SortOrder order);
    void tableUpdated();
    void updateDatabase();
    void onWindowMenuShow();
    void activate();

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
};

#endif // MAINWINDOW_H
