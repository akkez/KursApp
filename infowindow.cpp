#include "infowindow.h"
#include "ui_infowindow.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <config.h>
#include <QDebug>
#include <QMessageBox>
#include <QDateTime>
#include <QDate>
#include <QSqlError>

InfoWindow::InfoWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::InfoWindow)
{
    ui->setupUi(this);
    QTimer::singleShot(20, this, SLOT(timerTimeout()));
    this->setFixedSize(this->size());
}

void InfoWindow::timerTimeout() {
    this->getMainInfo();
    this->getObjects();
    this->getPerformedWorks();
}

InfoWindow::~InfoWindow()
{
    delete ui;
}

bool InfoWindow::initializeConnection() {
    this->db = QSqlDatabase::addDatabase("QODBC");
    this->db.setDatabaseName(this->connectionString);

    if (!this->db.open()) {
        ui->infoLabel->setText("Ошибка соединения");
        return false;
    }

    QSqlQuery query;
    query.exec("USE kurs");
    return true;
}

void InfoWindow::getMainInfo() {
    if (!this->initializeConnection()) {
        return;
    }

    QSqlQuery query;
    query.prepare("EXEC OutData :tabNumber;");
    query.bindValue(":tabNumber", this->tabNumber);
    query.exec();

    query.next();

    QString fullName = query.value(0).toString();
    QString position = query.value(1).toString();
    QString info;
    if (query.value(4).isNull()) {

        QString siteId = query.value(2).toString();
        QString district = query.value(3).toString();

        info = QString("Вы зашли как %1, %2 #%3, район \"%4\"")
               .arg(fullName).arg(position).arg(siteId).arg(district);
     } else {
        QString managementName = query.value(4).toString();
        info = QString("Вы зашли как %1, %2 района \"%3\"")
               .arg(fullName).arg(position).arg(managementName);
    }

    ui->infoLabel->setText(info);
}

void InfoWindow::getObjects() {
    if (!this->initializeConnection()) {
        return;
    }

    QSqlQuery query;
    query.prepare("EXEC GetObjects :tabNumber;");
    query.bindValue(":tabNumber", this->tabNumber);
    query.exec();

    int i = 0;
    while (query.next()) {
        int objectId = query.value(0).toInt();
        QString objectName = query.value(1).toString();
        ui->comboBox->insertItem(++i, "Объект \"" + objectName + "\"");
        ui->comboBox->setItemData(i, objectId);
    }
}

void InfoWindow::on_comboBox_activated(int index)
{
    this->getPerformedWorks();
}

void InfoWindow::getPerformedWorks()
{
    if (!this->initializeConnection()) {
        return;
    }

    int objectId = ui->comboBox->itemData(ui->comboBox->currentIndex()).toInt();

    QSqlQuery query;
    query.prepare("EXEC GetPerformedWorks :tabNumber, :objectId, :onlyIsAchieved;");
    query.bindValue(":tabNumber", this->tabNumber);
    query.bindValue(":objectId", objectId);
    query.bindValue(":onlyIsAchieved", ui->checkBox->isChecked());

    query.exec();

    ui->tableWidget->setRowCount(100);
    ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Название работы"));
    ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Ответственный"));
    ui->tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem("Сумма затрат"));
    ui->tableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem("Дата начала"));
    ui->tableWidget->setHorizontalHeaderItem(4, new QTableWidgetItem("Дата окончания"));
    ui->tableWidget->setHorizontalHeaderItem(5, new QTableWidgetItem("Фактическая дата окончания"));
    ui->tableWidget->horizontalHeader()->resizeSection(0, 135);
    ui->tableWidget->horizontalHeader()->resizeSection(5, 185);
    int row = 0;

    int overallSum = 0;
    while (query.next()) {
        for (int i = 0; i < 6; i++) {
            QString text;
            if (query.value(i).type() == QVariant::DateTime) { //datetime
                QDateTime dt = query.value(i).toDateTime();
                QDate d = dt.date();
                text = QString("").sprintf("%02d.%02d.%04d", d.day(), d.month(), d.year());
                if (d.year() == 0) {
                    text = "";
                }
            } else {
                text = query.value(i).toString();
            }

            QTableWidgetItem* item = new QTableWidgetItem(text);
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            item->setData(Qt::UserRole, query.value(6).toInt()); //pk
            ui->tableWidget->setItem(row, i, item);
        }
        overallSum += query.value(2).toInt();

        row++;
    }
    ui->tableWidget->setRowCount(row);
    ui->sumLabel->setText(QString("Сумма затрат: %1 рублей").arg(overallSum));
}

void InfoWindow::on_checkBox_clicked()
{
    this->getPerformedWorks();
}

void InfoWindow::on_addWorkButton_clicked()
{
    EditWindow* editWindow = new EditWindow(this);
    editWindow->tabNumber = this->tabNumber;
    editWindow->workId = 0;
    editWindow->show();
}

void InfoWindow::on_deleteWorkButton_clicked()
{
    if (ui->tableWidget->currentItem() == NULL) {
        QMessageBox::warning(this, "", "Выделите элемент в таблице!");
        return;
    }

    int workId = QVariant(ui->tableWidget->currentItem()->data(Qt::UserRole)).toInt();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "", "Вы действительно хотите удалить элемент из таблицы?", QMessageBox::Yes|QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return;
    }
    this->removePerformedWork(workId);
    this->getPerformedWorks();
}

void InfoWindow::removePerformedWork(int workId) {
    QSqlQuery query;
    query.prepare("EXEC RemovePerformedWork :workId;");
    query.bindValue(":workId", workId);
    query.exec();

    if (query.lastError().type() != QSqlError::NoError) {
        QMessageBox::warning(this, "Warning", "Ошибка: " + query.lastError().text());
    }
}

void InfoWindow::on_editWorkButton_clicked()
{
    if (ui->tableWidget->currentItem() == NULL) {
        QMessageBox::warning(this, "", "Выделите элемент в таблице!");
        return;
    }

    int workId = QVariant(ui->tableWidget->currentItem()->data(Qt::UserRole)).toInt();
    EditWindow* editWindow = new EditWindow(this);
    editWindow->tabNumber = this->tabNumber;
    editWindow->workId = workId;
    editWindow->show();
}
