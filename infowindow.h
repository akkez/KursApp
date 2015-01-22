#ifndef INFOWINDOW_H
#define INFOWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QTimer>
#include <QMap>
#include "editwindow.h"

namespace Ui {
class InfoWindow;
}

class InfoWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit InfoWindow(QWidget *parent = 0);
    ~InfoWindow();

    int tabNumber;
    QString connectionString;

    void getMainInfo();
    bool initializeConnection();

    QSqlDatabase db;

    void getObjects();
    void getPerformedWorks();

    void removePerformedWork(int workId);
private slots:
    void timerTimeout();

    void on_comboBox_activated(int index);

    void on_checkBox_clicked();

    void on_addWorkButton_clicked();

    void on_deleteWorkButton_clicked();

    void on_editWorkButton_clicked();

private:
    Ui::InfoWindow *ui;
};

#endif // INFOWINDOW_H
