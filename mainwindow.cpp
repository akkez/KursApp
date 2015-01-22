#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include "infowindow.h"
#include "config.h"
#include <QSqlQuery>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName(connectionString);

    if (!db.open()) {
        QMessageBox::critical(this, "Error", "Соединение с базой не открыто!");
        return;
    }

    QSqlQuery query;
    query.exec("USE kurs");

    query.prepare("EXEC logIn :tableName, :password;");
    query.bindValue(":tableName", ui->loginEdit->text().toInt());
    query.bindValue(":password", ui->passwordEdit->text());
    query.exec();

    query.next();
    QString value = query.value(0).toString();
    if (value != "1") {
        QMessageBox::warning(this, "warning", "Неправильный логин и пароль!");
        return;
    }

    this->showInfoWindow();
}

void MainWindow::showInfoWindow() {
    InfoWindow* w = new InfoWindow();
    w->tabNumber = ui->loginEdit->text().toInt();
    w->connectionString = connectionString;
    w->show();

    this->hide();
}
