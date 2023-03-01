#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QDir dir = QDir::current();
    QString s = QDir::toNativeSeparators(dir.absolutePath()) + QDir::separator() + "contacts.db";

    bool status = true;
    if (!dir.exists(s)) {
        status = createDB(s);
    }

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(s);

    if (db.open()) {
        s = "Connected";
    } else {
        s = "Disconnected";
    }

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout -> addStretch(1);
    vLayout -> addWidget(new QLabel(s + "-> " + db.databaseName()));
}

Widget::~Widget()
{
}

bool Widget::createDB(QString fileName) {
    bool status = false;
    QSqlDatabase newDB = QSqlDatabase::addDatabase("QSQLITE");
    newDB.setDatabaseName(fileName);
    if (newDB.open()) {
        QSqlQuery query(newDB);
        query.prepare("CREATE TABLE IF NOT EXISTS CONTACTS"
                      "(contacts_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "cname TEXT UNIQUE NOT NULL COLLATE NOCASE,"
                      "phone TEXT UNIQUE,"
                      "photo BLOB);");
        status = query.exec();
        if (status) {
            status = query.exec(".save" + fileName);
        }
    }
    return status;
}
