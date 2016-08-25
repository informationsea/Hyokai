#ifndef SQLASYNCHRONOUSEXECUTOR_H
#define SQLASYNCHRONOUSEXECUTOR_H

#include <QThread>

class QWidget;
class QSqlQuery;
class QProgressDialog;
class QProgressBar;
class QTimer;

class SqlAsynchronousExecutor : public QThread
{
    Q_OBJECT
public:
    SqlAsynchronousExecutor(QSqlQuery *query, QWidget *parent);
    virtual ~SqlAsynchronousExecutor();

    void start();
    virtual void run();

signals:
    void finishQuery(QSqlQuery *query, SqlAsynchronousExecutor *executor);

private slots:
    void closeProgress();
    void cancel();
    void timeout();

private:
    QWidget *m_parent;
    QSqlQuery *m_query;
    QProgressDialog *m_progress;
    QProgressBar *m_bar;
    QTimer *m_timer;

    int m_count;
};

#endif // SQLASYNCHRONOUSEXECUTOR_H
