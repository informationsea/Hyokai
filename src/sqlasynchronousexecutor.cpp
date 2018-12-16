#include "sqlasynchronousexecutor.h"

#include <QWidget>
#include <QSqlQuery>
#include <QProgressDialog>
#include <QProgressBar>
#include <QDebug>
#include <QTimer>
#include <QSqlError>

SqlAsynchronousExecutor::SqlAsynchronousExecutor(QSqlQuery *query, QWidget *parent) :
    QThread(parent), m_parent(parent), m_query(query), m_timer(new QTimer(this)), m_count(0)
{
    connect(this, SIGNAL(finished()), SLOT(closeProgress()));
}

SqlAsynchronousExecutor::~SqlAsynchronousExecutor()
{
    delete m_progress;
    delete m_timer;
}

void SqlAsynchronousExecutor::start()
{
    //qDebug() << m_parent;
    m_progress = new QProgressDialog(m_parent);
    m_progress->setLabelText("Now Querying...");
    m_progress->setMinimumDuration(100);
    m_progress->setCancelButton(nullptr);
    m_progress->setAutoClose(false);
    m_progress->setAutoReset(false);
    m_bar = new QProgressBar(m_progress);
    m_progress->setBar(m_bar);
    m_progress->open(this, SLOT(cancel()));
    QThread::start();

    m_timer->setInterval(50);
    m_timer->setSingleShot(false);
    connect(m_timer, SIGNAL(timeout()), SLOT(timeout()));
    m_timer->start();
}

void SqlAsynchronousExecutor::run()
{

    auto error = m_query->lastError();
    if (error.isValid()) {
        emit finishQuery(m_query, this);
        return;
    }

    m_query->exec();

    emit finishQuery(m_query, this);
}

void SqlAsynchronousExecutor::closeProgress()
{
    m_timer->stop();
    m_progress->close();
}

void SqlAsynchronousExecutor::cancel()
{

}

void SqlAsynchronousExecutor::timeout()
{
    m_count += 2;
    if (m_count >= 200) m_count = 0;

    if (m_count == 100)
        m_bar->setInvertedAppearance(true);
    if (m_count == 0)
        m_bar->setInvertedAppearance(false);

    if (m_count >= 100)
        m_bar->setValue(200 - m_count);
    else
        m_bar->setValue(m_count % 100);
}
