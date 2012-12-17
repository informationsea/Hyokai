#ifndef SQLTEXTEDIT_H
#define SQLTEXTEDIT_H

#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QStringList>
#include <QSqlDatabase>

class SQLSyntaxHighligter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit SQLSyntaxHighligter(QTextDocument *parent = 0);
    virtual void highlightBlock ( const QString & text );
    void setDatabase(QSqlDatabase *database);

private:
    QTextCharFormat m_base_format;
    QTextCharFormat m_sql_keyword_format;
    QTextCharFormat m_sql_command_format;
    QTextCharFormat m_sql_quoted_format;
    QTextCharFormat m_sql_table_format;
    QTextCharFormat m_sql_column_format;

    QStringList m_commant_list;
    QStringList m_keyword_list;

    QSqlDatabase *m_database;

    virtual QStringList highlightBlockHelper (const QString & text, const QStringList & keys, const QTextCharFormat & format );
};

class SQLTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit SQLTextEdit(QWidget *parent = 0);
    virtual ~SQLTextEdit();

    void setDatabase(QSqlDatabase *database);

protected:
    void virtual keyPressEvent ( QKeyEvent * event );

private:
    SQLSyntaxHighligter *m_syntaxHilighter;
    
signals:
    
public slots:
    
};

#endif // SQLTEXTEDIT_H
