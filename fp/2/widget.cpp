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

    QFrame *frame1 = new QFrame();
    frame1 -> setFrameStyle(QFrame::Sunken | QFrame::Box);

    lblPhoto = new QLabel();
    lblPhoto -> setMinimumSize(64, 64);
    lblPhoto -> setMaximumSize(64, 64);
    lblPhoto -> setFrameStyle(QFrame::Sunken | QFrame::Box);

    leCName = new QLineEdit();
    lePhone = new QLineEdit();
    lePhone -> setInputMask("8-000-000-00-00;#");

    btnOK = new QPushButton("Ok");
    connect(btnOK, &QAbstractButton::clicked, this, &Widget::slot_ok);

    btnOK = new QPushButton("Ok");
    connect(btnOK, &QAbstractButton::clicked, this, &Widget::slot_ok);

    btnCancel = new QPushButton("Cancel");
    connect(btnCancel, &QAbstractButton::clicked, this, &Widget::slot_cancel);

    QHBoxLayout *frameButtonsLayout = new QHBoxLayout();
    frameButtonsLayout -> addStretch(1);
    frameButtonsLayout -> addWidget(btnOK);
    frameButtonsLayout -> addWidget(btnCancel);

    QFormLayout *formLayout = new QFormLayout(frame1);
    formLayout -> addRow("Photo", lblPhoto);
    formLayout -> addRow("Name", leCName);
    formLayout -> addRow("Phone", lePhone);
    formLayout -> addRow(frameButtonsLayout);

    tableW = new QTableWidget(0, 4);
    tableW -> setHorizontalHeaderLabels(QStringList() << "#" << "Name" << "Phone" << "Photo");

    stLayout = new QStackedLayout();
    stLayout -> addWidget(tableW);
    stLayout -> addWidget(frame1);

    QPushButton *btnInsert = new QPushButton("Insert");
    connect(btnInsert, &QAbstractButton::clicked, this, &Widget::slot_insert);

    QPushButton *btnUpdate = new QPushButton("Update");
    connect(btnUpdate, &QAbstractButton::clicked, this, &Widget::slot_update);

    QPushButton *btnDelete = new QPushButton("Delete");
    connect(btnDelete, &QAbstractButton::clicked, this, &Widget::slot_delete);

    QHBoxLayout *hButtonLayout = new QHBoxLayout();
    hButtonLayout -> addWidget(btnInsert);
    hButtonLayout -> addWidget(btnUpdate);
    hButtonLayout -> addWidget(btnDelete);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout -> addLayout(hButtonLayout);
    vLayout -> addLayout(stLayout, 1);
    vLayout -> addWidget(new QLabel(s + "-> " + db.databaseName()));

    slot_select();
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

void Widget::slot_select() {
    QSqlQuery qSelect;
    qSelect.prepare("SELECT count(*) AS count FROM CONTACTS;");
    qSelect.exec();
    qSelect.first();

    int count = qSelect.value("count").toInt();
    qSelect.finish();

    tableW -> clearContents();
    tableW -> setRowCount(count);

    qSelect.prepare("SELECT contacts_id, cname, phone, photo FROM CONTACTS ORDER BY cname;");

    int row = 0;
    while (qSelect.next()) {
        QTableWidgetItem *item0 = new QTableWidgetItem(QString("%1").arg(row + 1));
        item0 -> setData(Qt::UserRole, qSelect.value("contacts_id").toInt());
        QTableWidgetItem *item1 = new QTableWidgetItem(qSelect.value("cname").toString());
        QTableWidgetItem *item2 = new QTableWidgetItem(qSelect.value("phone").toString());
        QTableWidgetItem *item3 = new QTableWidgetItem("");

        tableW -> setItem(row, 0, item0);
        tableW -> setItem(row, 1, item1);
        tableW -> setItem(row, 2, item2);
        tableW -> setItem(row, 3, item3);

        row++;
    }

    qSelect.finish();
}

void Widget::slot_insert() {
    stLayout -> setCurrentIndex(1);
    lePhone -> clear();
    leCName -> clear();
    lblPhoto -> clear();
    contacts_id = -1;
}

void Widget::slot_update() {
    if (tableW -> currentItem()) {
        int row = tableW -> currentItem() -> row();
        contacts_id = tableW -> item(row, 0) -> data(Qt::UserRole).toInt();
        leCName -> setText(tableW -> item(row, 1) -> text());
        lePhone -> setText(tableW -> item(row, 2) -> text());
        // lblPhoto -> setPixmap();
        stLayout -> setCurrentIndex(1);
    }
}

void Widget::slot_delete() {

}

void Widget::slot_ok() {
    QSqlQuery query;
    if (contacts_id == -1) {
        query.prepare("INSERT INTO CONTACTS"
                      " (cname, phone, photo)"
                      " VALUES"
                      " (:cname, :phone, :photo);");
    } else {

    }

    query.bindValue(":cname", leCName -> text().trimmed());
    query.bindValue(":phone", lePhone -> text().trimmed());
    query.bindValue(":photo", NULL);

    query.exec();
    slot_select();
    stLayout -> setCurrentIndex(0);
}

void Widget::slot_cancel() {
    stLayout -> setCurrentIndex(0);
    slot_select();
}
