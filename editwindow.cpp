#include "editwindow.h"
#include "ui_editwindow.h"
#include <QDebug>
#include <QSqlError>
#include <QMessageBox>

EditWindow::EditWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EditWindow)
{
    ui->setupUi(this);
    QTimer::singleShot(20, this, SLOT(timerTimeout()));
    this->setFixedSize(this->size());
}

EditWindow::~EditWindow()
{
    delete ui;
}

void EditWindow::timerTimeout() {
    this->retrieveForemens();
    this->retrieveWorkTypes();
    this->retrieveObjects();

    if (this->workId != 0) {
        this->retrievePerformedWork();
        ui->groupBox->setTitle("Редактирование работы");
        ui->saveButton->setText("Сохранить");
    } else {
        ui->dateStartEdit->setDateTime(QDateTime::currentDateTime());
        ui->dateFinishEdit->setDateTime(QDateTime::currentDateTime());
        ui->dateRealFinishEdit->setDateTime(QDateTime::currentDateTime());
        ui->groupBox->setTitle("Создание работы");
        ui->saveButton->setText("Создать");
    }
}

bool EditWindow::initializeConnection() {
    this->db = QSqlDatabase::addDatabase("QODBC");
    this->db.setDatabaseName(connectionString);

    if (!this->db.open()) {
        return false;
    }

    QSqlQuery query;
    query.exec("USE kurs");
    return true;
}



void EditWindow::retrieveForemens() {
    if (!this->initializeConnection()) {
        return;
    }
    QSqlQuery query;
    query.prepare("EXEC GetTeamForemens;");
    query.exec();

    int i = 0;
    while (query.next()) {
        int objectId = query.value(0).toInt();
        QString objectName = query.value(1).toString();
        ui->foremenComboBox->insertItem(i, objectName);
        ui->foremenComboBox->setItemData(i, objectId);

        i++;
    }
}

void EditWindow::retrieveWorkTypes() {
    if (!this->initializeConnection()) {
        return;
    }
    QSqlQuery query;
    query.prepare("EXEC GetWorkTypes :tabNumber;");
    query.bindValue(":tabNumber", this->tabNumber);
    query.exec();

    int i = 0;
    while (query.next()) {
        int objectId = query.value(0).toInt();
        QString objectName = query.value(1).toString();
        ui->workTypeComboBox->insertItem(i, objectName);
        ui->workTypeComboBox->setItemData(i, objectId);

        i++;
    }
}

void EditWindow::retrieveObjects() {
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
        ui->objectComboBox->insertItem(i, objectName);
        ui->objectComboBox->setItemData(i, objectId);

        i++;
    }
}

void EditWindow::retrievePerformedWork() {
    if (!this->initializeConnection()) {
        return;
    }

    QSqlQuery query;
    query.prepare("EXEC GetPerformedWorkById :pk;");
    query.bindValue(":pk", this->workId);
    query.exec();

    query.next();
    this->setComboboxPositionByValue(ui->workTypeComboBox, query.value(1).toInt());
    this->setComboboxPositionByValue(ui->objectComboBox, query.value(2).toInt());
    this->setComboboxPositionByValue(ui->foremenComboBox, query.value(3).toInt());

    ui->summaryEdit->setText(query.value(4).toString());
    ui->dateStartEdit->setDateTime(query.value(5).toDateTime());
    ui->dateFinishEdit->setDateTime(query.value(6).toDateTime());

    if (!query.value(7).isNull()) {
        ui->dateRealFinishEdit->setDateTime(query.value(7).toDateTime());
        ui->dateRealFinishEdit->setEnabled(true);
        ui->isAchievedCheckBox->setChecked(true);
    } else {
        ui->isAchievedCheckBox->setChecked(false);
        ui->dateRealFinishEdit->setEnabled(false);
    }
}

void EditWindow::on_isAchievedCheckBox_clicked()
{
    ui->dateRealFinishEdit->setEnabled(ui->isAchievedCheckBox->isChecked());
}

void EditWindow::on_saveButton_clicked()
{
    this->savePerformedWork();
    this->on_cancelButton_clicked();
}

void EditWindow::savePerformedWork() {
    ui->summaryEdit->setText(QString::number(ui->summaryEdit->text().toInt()));

    int objectId = ui->objectComboBox->currentData().toInt();
    int foremenId = ui->foremenComboBox->currentData().toInt();
    int workTypeId = ui->workTypeComboBox->currentData().toInt();
    int summ = ui->summaryEdit->text().toInt();
    QDateTime startDate = ui->dateStartEdit->dateTime();
    QDateTime finishDate = ui->dateFinishEdit->dateTime();
    QDateTime actualFinishDate;
    if (ui->isAchievedCheckBox->isChecked()) {
        actualFinishDate = ui->dateRealFinishEdit->dateTime();
    }

    if (!this->initializeConnection()) {
        return;
    }

    QSqlQuery query;
    query.prepare("EXEC SavePerformedWork :workTypeId, :foremenId, :summ, :objectId, :startDate, :finishDate, :actualFinishDate, :pk;");
    query.bindValue(":workTypeId", workTypeId);
    query.bindValue(":foremenId", foremenId);
    query.bindValue(":summ", summ);
    query.bindValue(":objectId", objectId);
    query.bindValue(":startDate", startDate);
    query.bindValue(":finishDate", finishDate);
    query.bindValue(":actualFinishDate", actualFinishDate);
    query.bindValue(":pk", this->workId);
    query.exec();

    if (query.lastError().type() == QSqlError::NoError) {
        if (this->workId == 0) {
            QMessageBox::information(this, "Ok", "Работа была добавлена.");
        } else {
            QMessageBox::information(this, "Ok", "Работа была отредактирована.");
        }
    }  else {
        QMessageBox::warning(this, "Warning", "Некорректная дата фактического завершения работы.");//"Ошибка: " + query.lastError().text());
    }
}

void EditWindow::on_cancelButton_clicked()
{
    InfoWindow* window = (InfoWindow*)this->parent();
    this->close();
    window->getPerformedWorks();
}

void EditWindow::setComboboxPositionByValue(QComboBox* widget, int value) {
    for (int i = 0; i < widget->count(); i++) {
        if (widget->itemData(i).toInt() != value) {
            continue;
        }
        widget->setCurrentIndex(i);
        return;
    }
}
