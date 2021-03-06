#include "sqltextedit.h"

#include <QDebug>
#include <QBrush>
#include <QColor>
#include <QSqlRecord>
#include <QChar>
#include <QRegExp>
#include <QMenu>
#include "main.h"

SQLTextEdit::SQLTextEdit(QWidget *parent) :
    QPlainTextEdit(parent), m_popup(nullptr)
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

QStringList SQLTextEdit::loadFunctionList(const QString &driver)
{
    QFile listfile;
    if (driver == "QSQLITE")
        listfile.setFileName(":/txt/keywords/sqlite-functions.txt");
    else if (driver == "QMYSQL")
        listfile.setFileName(":/txt/keywords/mysql-functions.txt");
    else if (driver == "QPSQL")
        listfile.setFileName(":/txt/keywords/postgresql-functions.txt");
    else
        return QStringList();

    listfile.open(QIODevice::ReadOnly);

    QStringList list;
    QString line;

    while (!(line = listfile.readLine()).isEmpty()) {
        //int pos = line.indexOf('(');
        //if (pos != -1)
        //    line = line.mid(0, pos);
        list << line.trimmed();
    }
    return list;
}

QStringList SQLTextEdit::loadKeywords(const QString &driver)
{
    QFile listfile;
    if (driver == "QSQLITE")
        listfile.setFileName(":/txt/keywords/sqlite-keywords.txt");
    else if (driver == "QMYSQL")
        listfile.setFileName(":/txt/keywords/mysql-keywords.txt");
    else if (driver == "QPSQL")
        listfile.setFileName(":/txt/keywords/postgresql-keywords.txt");
    else
        return QStringList();

    listfile.open(QIODevice::ReadOnly);

    QStringList list;
    QString line;

    while (!(line = listfile.readLine()).isEmpty()) {
        list << line.trimmed();
    }
    return list;
}

void SQLTextEdit::addEntryHelper(QMenu *menu, QString str, int length) {
    QAction *action = new QAction(menu);
    QMap<QString, QVariant> data;
    data["text"] = str;
    data["length"] = length;
    action->setData(data);
    action->setText(str);
    connect(action, SIGNAL(triggered()), this, SLOT(autoComplete()));
    menu->addAction(action);
}

void SQLTextEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter ||
            event->key() == Qt::Key_Return) {
        emit returnPressed();;
        event->accept();
    } else if (event->key() == Qt::Key_Tab) {
        if (m_popup)
            delete m_popup;

        QTextCursor cursor = textCursor();
        QString textBlock = cursor.block().text();
        int seppos = textBlock.lastIndexOf(QRegExp("[\\W]"), cursor.positionInBlock()-1);
        seppos += 1;
        int length = cursor.positionInBlock()-seppos;

        m_popup = new QMenu(this);
        SQLCompleteCandidates suggest = m_syntaxHilighter->completeCandidates(textBlock.mid(seppos, length), textBlock);

        if (suggest.columns.length() < 10 || (suggest.tables.length() == 0 && suggest.keywords.length() == 0)) {
            foreach(QString one, suggest.columns) {
                addEntryHelper(m_popup, one, length);
            }
        } else {
            QMenu *columns = new QMenu(tr("Columns"), m_popup);

            foreach(QString one, suggest.columns) {
                addEntryHelper(columns, one, length);
            }
            m_popup->addMenu(columns);
        }

        if (m_popup->actions().length() > 0 && suggest.tables.length() > 0) {
            m_popup->addSeparator();
        }

        if (suggest.tables.length() < 10 || (suggest.columns.length() == 0 && suggest.keywords.length() == 0)) {
            foreach(QString one, suggest.tables) {
                addEntryHelper(m_popup, one, length);
            }
        } else {
            QMenu *tables = new QMenu(tr("Tables"), m_popup);

            foreach(QString one, suggest.tables) {
                addEntryHelper(tables, one, length);
            }
            m_popup->addMenu(tables);
        }

        if (m_popup->actions().length() > 0 && suggest.keywords.length() > 0) {
            m_popup->addSeparator();
        }

        if (suggest.keywords.length() < 10 || (suggest.tables.length() == 0 && suggest.columns.length() == 0)) {
            foreach(QString one, suggest.keywords) {
                addEntryHelper(m_popup, one, length);
            }
        } else {
            QMenu *keywords = new QMenu(tr("Keywords"), m_popup);

            foreach(QString one, suggest.keywords) {
                addEntryHelper(keywords, one, length);
            }
            m_popup->addMenu(keywords);
        }

        if (m_popup->actions().length() == 0) {
            QAction *action = new QAction(m_popup);
            action->setDisabled(true);
            action->setText(tr("No candidate"));
            m_popup->addAction(action);
        } else {
            m_popup->setActiveAction(m_popup->actions()[0]);
            m_popup->setFocus();
        }


        m_popup->popup(mapToGlobal(cursorRect().topLeft()));
    } else {
        QPlainTextEdit::keyPressEvent(event);
    }
}

void SQLTextEdit::autoComplete()
{
    QAction *action = (QAction *)sender();

    QMap<QString, QVariant> insertData = action->data().toMap();
    QString insertText = insertData["text"].toString();
    int length = insertData["length"].toInt();
    textCursor().insertText(insertText.right(insertText.length() - length));
}


SQLSyntaxHighligter::SQLSyntaxHighligter(QTextDocument *parent):
    QSyntaxHighlighter(parent), m_database(nullptr)
{
    m_command_list << "ALTER TABLE"
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

    m_sql_command_format.setForeground(QBrush(QColor("#1F36E0")));
    m_sql_command_format.setFontWeight(QFont::Bold);
    m_sql_keyword_format.setForeground(QBrush(QColor("#48129B")));
    m_sql_keyword_format.setFontWeight(QFont::Bold);
    m_sql_table_format.setForeground(QBrush(QColor("#C27B1F")));
    m_sql_column_format.setForeground(QBrush(QColor("#78810E")));
    m_sql_quoted_format.setForeground(QBrush(QColor("#0D6629")));
    m_sql_function_format.setForeground(QBrush(QColor("#10547A")));
}



void SQLSyntaxHighligter::highlightBlock(const QString &text)
{
    highlightBlockHelper(text, m_keyword_list, m_sql_keyword_format);
    highlightBlockHelper(text, m_function_list, m_sql_function_format);

    foreach(QString command, m_command_list) {
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
            QSqlRecord record = m_database->record(oneTable);
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

        {
            QRegExp quoted("'[^']*'");
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

    m_keyword_list = SQLTextEdit::loadKeywords(m_database->driverName());

    foreach (QString line, SQLTextEdit::loadFunctionList(m_database->driverName())) {
        if (line.startsWith(">"))
            continue;
        int pos = line.indexOf('(');
        if (pos != -1)
            line = line.mid(0, pos);
           m_function_list << line;
    }
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

SQLCompleteCandidates SQLSyntaxHighligter::completeCandidates(const QString &prefix, const QString &blockText)
{
    SQLCompleteCandidates candidates;
    //qDebug() << "completeCandidates" << prefix << blockText;

    if (m_database) {
        foreach(QString one, m_keyword_list) {
            if (one.startsWith(prefix)) {
                candidates.keywords << one;
            }
        }

        QStringList tables = m_database->tables(QSql::AllTables);
        foreach(QString one, tables) {
            if (one.startsWith(prefix)) {
                candidates.tables << one;
            }
        }

        QStringList foundTables;
        if (m_table.isEmpty()) {
            foreach(QString oneTable, tables) {
                int pos = blockText.indexOf(oneTable);
                if (pos < 0) continue;
                if ((pos == 0 || !isPartOfName(blockText.at(pos-1))) &&
                        (pos+oneTable.length() >= blockText.length() || !isPartOfName(blockText.at(pos+oneTable.length())))) {
                    foundTables << oneTable;
                }
            }

            if (foundTables.isEmpty()) {
                foundTables << tables;
            }
        } else {
            foundTables << m_table;
        }

        foreach(QString oneTable, foundTables) {
            QSqlRecord record = m_database->record(oneTable);
            for (int i = 0; i < record.count(); ++i) {
                QString fieldName = record.fieldName(i);
                if (fieldName.startsWith(prefix)) {
                    candidates.columns << fieldName;
                }
            }
        }
    }

    return candidates;
}
