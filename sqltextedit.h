#ifndef SQLTEXTEDIT_H
#define SQLTEXTEDIT_H

#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QStringList>

class SQLSyntaxHighligter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit SQLSyntaxHighligter(QTextDocument *parent = 0);
    virtual void highlightBlock ( const QString & text );

private:
    QTextCharFormat m_base_format;
    QTextCharFormat m_sql_keyword_format;
    QTextCharFormat m_sql_command_format;
    QTextCharFormat m_sql_quoted_format;

    QStringList m_commant_list;
    QStringList m_keyword_list;
};

class SQLTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit SQLTextEdit(QWidget *parent = 0);
    virtual ~SQLTextEdit();

protected:
    void virtual keyPressEvent ( QKeyEvent * event );

private:
    SQLSyntaxHighligter *m_syntaxHilighter;
    
signals:
    
public slots:
    
};

#endif // SQLTEXTEDIT_H
