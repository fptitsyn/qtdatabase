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
    QStackedLayout *stLayout;
    QTableWidget *tableW;
    QComboBox *cbxCategory;

    QLabel *lblPhoto;
    QLineEdit *leCName,
              *lePhone;
    QPushButton *btnOK,
                *btnCancel;
    int contacts_id = -1;

private:
    bool createDB(QString fileName);

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

public slots:
    void slot_select();
    void slot_insert();
    void slot_update();
    void slot_delete();
    void slot_cancel();
    void slot_ok();
    void slot_photo();

    void slot_category_current_index_changed(int index);
};
#endif // WIDGET_H
