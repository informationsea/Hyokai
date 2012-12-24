#include "sqltextedit.h"

#include <QDebug>
#include <QBrush>
#include <QColor>
#include <QSqlRecord>
#include <QChar>
#include <QRegExp>
#include "main.h"

SQLTextEdit::SQLTextEdit(QWidget *parent) :
    QPlainTextEdit(parent)
{
    m_syntaxHilighter = new SQLSyntaxHighligter(document());

}

SQLTextEdit::~SQLTextEdit()
{
    delete m_syntaxHilighter;
}

void SQLTextEdit::setDatabase(QSqlDatabase *database)
{
    m_syntaxHilighter->setDatabase(database);
}

void SQLTextEdit::setTable(const QString &table)
{
    m_syntaxHilighter->setTable(table);
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
    QSyntaxHighlighter(parent), m_database(0)
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
    m_sql_table_format.setForeground(QBrush(QColor("#C27B1F")));
    m_sql_column_format.setForeground(QBrush(QColor("#78810E")));
    m_sql_quoted_format.setForeground(QBrush(QColor("#0D6629")));
}



void SQLSyntaxHighligter::highlightBlock(const QString &text)
{
    highlightBlockHelper(text, m_keyword_list, m_sql_keyword_format);

    foreach(QString command, m_commant_list) {
        if(text.startsWith(command, Qt::CaseInsensitive))
            setFormat(0, command.length(), m_sql_command_format);
    }

    if (m_database) {
        QStringList tables = m_database->tables(QSql::AllTables);
        QStringList foundTables = highlightBlockHelper(text, tables, m_sql_table_format);

        if (!m_table.isEmpty())
            foundTables << m_table;

        QStringList columns;
        foreach(QString oneTable, foundTables) {
            QSqlRecord record = m_database->record(addQuote(oneTable));
            for (int i = 0; i < record.count(); ++i) {
                columns << record.fieldName(i);
            }
        }

        highlightBlockHelper(text, columns, m_sql_column_format);

        { // highlight quote
            QRegExp quoted("\"[^\"]*\"");
            int pos = -1;
            while ((pos = text.indexOf(quoted, pos+1)) >= 0) {
                setFormat(pos, quoted.matchedLength(), m_sql_quoted_format);
                pos += quoted.matchedLength();
            }
        }
    }
}

void SQLSyntaxHighligter::setDatabase(QSqlDatabase *database)
{
    m_database = database;
}

void SQLSyntaxHighligter::setTable(const QString &table)
{
    m_table = table;
}

static bool isPartOfName(QChar ch)
{
    if (ch.isLetterOrNumber())
        return true;
    if (ch == QChar('_'))
        return true;
    return false;
}

QStringList SQLSyntaxHighligter::highlightBlockHelper(const QString & text, const QStringList &keys, const QTextCharFormat &format)
{
    QStringList found;
    foreach(QString one, keys) {
        int pos = -1;
        while ((pos = text.indexOf(one, pos+1, Qt::CaseInsensitive)) >= 0) {
            if ((pos == 0 || !isPartOfName(text.at(pos-1))) &&
                    (pos+one.length() >= text.length() || !isPartOfName(text.at(pos+one.length())))) {
                setFormat(pos, one.length(), format);

                if (!found.contains(one))
                    found << one;
            }
        }
    }
    return found;
}
