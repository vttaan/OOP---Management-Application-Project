#include "global.h"
#ifndef SCHEDULE_VIEW_H
#define SCHEDULE_VIEW_H

namespace Ui {
class Schedule_View;
}

class Schedule_View : public QWidget
{
    Q_OBJECT

public:
    explicit Schedule_View(QWidget *parent = nullptr);
    ~Schedule_View();

    // for allow registrate, only fixed day to registrate
    void enableRegistration(bool isEnable);

    // show error when time overlapping
    void showError(const QString& mess);

    // loadData
    void setUpDataInputTable(const QList<QString>& listDays, int openTime, int closeTime);

    // data for pre-view table, when add success, controller call this function to update pre-view table. int in map is colums, QList is content of this colum
    void updateSumTable(const QMap<int, QList<QString>> weeklyData);

    // reset input table, controller call, when add shitf success
    void resetInputTable();

private:
    Ui::Schedule_View *ui;
    void setUpUI();
    QComboBox* createComboBox(const QList<QString>& item);

signals:

    // request in Schedule page
    void requestAddShift(const QString& day, const QString& startTime, const QString& endTime);
    void requestSaveShift();

    // switch tab
    void profileClicked();
    void dashboardClicked();
    void employeeClicked();
private slots:
    void buttonAddClicked();
    void buttonSaveClicked();
};

#endif // SCHEDULE_VIEW_H
