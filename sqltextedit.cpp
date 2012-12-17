#include "sqltextedit.h"

#include <QDebug>
#include <QBrush>
#include <QColor>

SQLTextEdit::SQLTextEdit(QWidget *parent) :
    QPlainTextEdit(parent)
{
    m_syntaxHilighter = new SQLSyntaxHighligter(document());

}

SQLTextEdit::~SQLTextEdit()
{
}

void SQLTextEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter ||
            event->key() == Qt::Key_Return ||
            event->key() == Qt::Key_Tab) {
        event->ignore();
    } else {
        QPlainTextEdit::keyPressEvent(event);
    }
}


SQLSyntaxHighligter::SQLSyntaxHighligter(QTextDocument *parent):
    QSyntaxHighlighter(parent)
{
    m_commant_list << "ALTER TABLE"
                   << "ANALYZE"
                   << "ATTACH DATABASE"
                   << "CREATE INDEX"
                   << "CREATE TABLE"
                   << "CREATE TRIGGER"
                   << "CREATE VIEW"
                   << "CREATE VIRTUAL TABLE"
                   << "CREATE"
                   << "DELETE"
                   << "DETACH DATABASE"
                   << "DROP INDEX"
                   << "DROP TABLE"
                   << "DROP TRIGGER"
                   << "DROP VIEW"
                   << "EXPLAIN"
                   << "INSERT"
                   << "REPLACE"
                   << "PRAGMA"
                   << "REINDEX"
                   << "SELECT"
                   << "UPDATE"
                   << "VACUUM";

    QByteArray line;
    QFile keyword(":/txt/sqlkeywords.txt");
    keyword.open(QIODevice::ReadOnly);
    while(!(line = keyword.readLine()).isEmpty()) {
        m_keyword_list.append(line.trimmed());
    }

    m_sql_command_format.setForeground(QBrush(QColor("#1F36E0")));
    m_sql_command_format.setFontWeight(QFont::Bold);
    m_sql_keyword_format.setForeground(QBrush(QColor("#48129B")));
    m_sql_keyword_format.setFontWeight(QFont::Bold);
}

void SQLSyntaxHighligter::highlightBlock(const QString &text)
{
    foreach(QString keyword, m_keyword_list) {
        int pos = -1;
        while ((pos = text.indexOf(keyword, pos+1, Qt::CaseInsensitive)) >= 0) {
            if ((pos == 0 || text.at(pos-1).isSpace()) && (pos+keyword.length() >= text.length()-1 || text.at(pos+keyword.length()).isSpace()))
                setFormat(pos, keyword.length(), m_sql_keyword_format);
        }
    }

    foreach(QString command, m_commant_list) {
        if(text.startsWith(command, Qt::CaseInsensitive))
            setFormat(0, command.length(), m_sql_command_format);
    }
}
