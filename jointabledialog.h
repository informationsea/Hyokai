#ifndef JOINTABLEDIALOG_H
#define JOINTABLEDIALOG_H

#include <QDialog>
#include <QSqlDatabase>
#include <QList>
#include <QSqlRelation>

namespace Ui {
class JoinTableDialog;
}

class JoinTableDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit JoinTableDialog(QSqlDatabase *db, QWidget *parent = 0);
    virtual ~JoinTableDialog();
    QString sql();

private slots:
    void leftTableChanged();
    void rightTableChanged();
    void on_naturalJoin_clicked(bool checked);

private:
    Ui::JoinTableDialog *ui;
    QSqlDatabase *m_database;
};

#endif // JOINTABLEDIALOG_H
