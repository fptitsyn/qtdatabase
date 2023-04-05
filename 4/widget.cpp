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

    QPushButton *btnPhoto = new QPushButton("Load photo");
    connect(btnPhoto, &QPushButton::clicked, this, &Widget::slot_photo);

    QFormLayout *formLayout = new QFormLayout(frame1);
    formLayout -> addRow("Photo", lblPhoto);
    formLayout -> addRow("File", btnPhoto);
    formLayout -> addRow("Name", leCName);
    formLayout -> addRow("Phone", lePhone);
    formLayout -> addRow(frameButtonsLayout);

    tableW = new QTableWidget(0, 4);
    tableW -> setHorizontalHeaderLabels(QStringList() << "#" << "Name" << "Phone" << "Photo");

    cbxCategory = new QComboBox();
    cbxCategory -> addItem("All contacts", 0);

    QSqlQuery query1("SELECT * FROM CATEGORY ORDER BY category;");
    while (query1.next()) {
        cbxCategory -> addItem(query1.value("category").toString(),
                               query1.value("category_id").toInt());
    }
    query1.finish();

    connect(cbxCategory, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Widget::slot_category_current_index_changed);

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
    vLayout -> addWidget(cbxCategory);
    vLayout -> addLayout(stLayout, 1);
    vLayout -> addWidget(new QLabel(s + "-> " + db.databaseName()));

    slot_select();
}

Widget::~Widget()
{
}

bool Widget::createDB(QString fileName) {
    int step = 0;
    QSqlDatabase newDB = QSqlDatabase::addDatabase("QSQLITE");
    newDB.setDatabaseName(fileName);
    if (newDB.open()) {
        QSqlQuery query(newDB);
        query.prepare("PRAGMA foreign_keys=on;");
        if (query.exec()) {
            step++;
        }

        query.prepare("CREATE TABLE IF NOT EXISTS CATEGORY"
                      " (category_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      " category TEXT UNIQUE NOT NULL COLLATE NOCASE);");
        if (query.exec()) {
            step++;
        }

        query.prepare("CREATE TABLE IF NOT EXISTS CONTACTS"
                      "(contacts_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "category_id INTEGER,"
                      "cname TEXT UNIQUE NOT NULL COLLATE NOCASE,"
                      "phone TEXT UNIQUE,"
                      "photo BLOB,"
                      "FOREIGN KEY (category_id) REFERENCES CATEGORY(category_id));");
        if (query.exec()) {
            step++;
        }

        query.exec("INSERT INTO CATEGORY (category) VALUES ('Учёба')");
        query.exec("INSERT INTO CATEGORY (category) VALUES ('Друзья')");
        query.exec("INSERT INTO CATEGORY (category) VALUES ('Семья')");

        query.exec(" CREATE TRIGGER tr_CATEGORY_BD"
                   " BEFORE DELETE ON CATEGORY"
                   " FOR EACH ROW"
                   " BEGIN"
                   " SELECT CASE WHEN (OLD.category_id IN (1, 2, 3) )"
                   " THEN RAISE(ABORT, 'This category can not be deleted!') END;"
                   " END;");

        query.exec(" CREATE TRIGGER tr_CATEGORY_BU"
                   " BEFORE UPDATE ON CATEGORY"
                   " FOR EACH ROW"
                   " BEGIN"
                   " SELECT CASE WHEN (OLD.category_id IN (1, 2, 3) )"
                   " THEN RAISE(ABORT, 'This category can not be updated!') END;"
                   " END;");
    }

    return step == 3;
}

void Widget::slot_select() {
    int category_id = cbxCategory -> currentData(Qt::UserRole).toInt();

    QSqlQuery qSelect;
    QString s;
    if (category_id < 1) {
        s = "SELECT count(*) AS count FROM CONTACTS;";
    } else {
        s = QString("SELECT count(*) AS count FROM CONTACTS WHERE category_id = %1;").arg(category_id);
    }

    qSelect.prepare(s);
    qSelect.exec();
    qSelect.first();

    int count = qSelect.value("count").toInt();
    qSelect.finish();

    tableW -> clearContents();
    tableW -> setRowCount(count);

    if (category_id < 1) {
        s = "SELECT contacts_id, cname, phone, photo FROM CONTACTS ORDER BY cname;";
    } else {
        s = QString("SELECT contacts_id, cname, phone, photo FROM CONTACTS WHERE category_id = %1 ORDER BY cname;").arg(category_id);
    }

    qSelect.prepare(s);
    qSelect.exec();

    int row = 0;
    while (qSelect.next()) {
        QTableWidgetItem *item0 = new QTableWidgetItem(QString("%1").arg(row + 1));
        item0 -> setData(Qt::UserRole, qSelect.value("contacts_id").toInt());
        QTableWidgetItem *item1 = new QTableWidgetItem(qSelect.value("cname").toString());
        QTableWidgetItem *item2 = new QTableWidgetItem(qSelect.value("phone").toString());

        QByteArray ba = qSelect.value("photo").toByteArray();
        QPixmap pixmap;
        pixmap.loadFromData(ba);

        QTableWidgetItem *item3 = new QTableWidgetItem();
        item3 -> setIcon(QIcon(pixmap));

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

        QIcon icon = tableW -> item(row, 3) -> icon();
        QPixmap pixmap = icon.pixmap(QSize(64, 64), QIcon::Normal, QIcon::On);
        lblPhoto -> setPixmap(QPixmap(pixmap));

        stLayout -> setCurrentIndex(1);
    }
}

void Widget::slot_delete() {
    if (tableW -> currentItem()) {
        QMessageBox MessageBox;
        MessageBox.setText("Delete record?");
        MessageBox.setIcon(QMessageBox::Question);
        MessageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        MessageBox.setDefaultButton(QMessageBox::No);
        switch (MessageBox.exec()) {
            case QMessageBox::Yes: {
                int row = tableW -> currentItem() -> row();
                contacts_id = tableW -> item(row, 0) -> data(Qt::UserRole).toInt();
                QSqlQuery query;
                query.prepare("DELETE FROM CONTACTS"
                              " WHERE"
                              " contacts_id = :contacts_id");
                query.bindValue(":contacts_id", contacts_id);
                query.exec();
                slot_select();

                break;
            }
            default:
                break;
        }
    }
}

void Widget::slot_ok() {
    QSqlQuery query;
    if (contacts_id == -1) {
        query.prepare("INSERT INTO CONTACTS"
                      " (cname, phone, photo, category_id)"
                      " VALUES"
                      " (:cname, :phone, :photo, :category_id);");
    } else {
        query.prepare("UPDATE CONTACTS"
                      " SET"
                      " cname = :cname, phone = :phone, photo = :photo, category_id = :category_id"
                      " WHERE contacts_id = :contacts_id");

        query.bindValue(":contacts_id", contacts_id);
    }

    query.bindValue(":cname", leCName -> text().trimmed());
    query.bindValue(":phone", lePhone -> text().trimmed());

    int category_id = cbxCategory -> currentData(Qt::UserRole).toInt();
    query.bindValue(":category_id", category_id);

    if (lblPhoto -> pixmap()->isNull()) {
        query.bindValue(":photo", NULL);
    }
    else {
        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        lblPhoto -> pixmap() -> save(&buffer, "PNG");
        query.bindValue(":photo", ba);
    }

    query.exec();
    slot_select();
    stLayout -> setCurrentIndex(0);
}

void Widget::slot_cancel() {
    stLayout -> setCurrentIndex(0);
    slot_select();
}

void Widget::slot_photo() {
    QString filter = "All files (*.*);;Pictures (*.jpg *.jpeg *.png)";
    QString selectedFilter = "Pictures (*.jpg *.jepg *.png)";
    QString fileName = QFileDialog::getOpenFileName(this, "Open file", "", filter, &selectedFilter);
    if (QFileInfo::exists(fileName)) {
        QPixmap pixmap(fileName);
        lblPhoto -> setPixmap(pixmap.scaled(64, 64, Qt::KeepAspectRatio));
    }
}

void Widget::slot_category_current_index_changed(int index) {
    slot_select();
}
