#ifndef EDITWINDOW_H
#define EDITWINDOW_H

#include <QMainWindow>
#include "infowindow.h"
#include <QSqlDatabase>
#include "config.h"
#include <QSqlQuery>
#include <QTimer>
#include <QComboBox>

namespace Ui {
class EditWindow;
}

class EditWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EditWindow(QWidget *parent = 0);
    ~EditWindow();
    void retrieveForemens();

    QSqlDatabase db;
    int tabNumber;
    int workId;
    bool initializeConnection();
    void retrieveWorkTypes();
    void retrieveObjects();
    void savePerformedWork();

    void retrievePerformedWork();
    void setComboboxPositionByValue(QComboBox *widget, int value);
private slots:
    void timerTimeout();
    void on_isAchievedCheckBox_clicked();
    void on_saveButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::EditWindow *ui;
};

#endif // EDITWINDOW_H
