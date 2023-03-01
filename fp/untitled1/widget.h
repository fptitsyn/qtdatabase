#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtWidgets>
#include <QtSql>

class Widget : public QWidget
{
    Q_OBJECT

public:
    QSqlDatabase db;

private:
    bool createDB(QString fileName);

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
};
#endif // WIDGET_H
