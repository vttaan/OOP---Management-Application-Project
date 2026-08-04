#pragma once

#include "global.h"
#include "utils/SessionManage.h"
#include "model/Employee_Model.h"
#include "model/Dashboard_Model.h"

class Dashboard_View;

class Dashboard_Control : public QObject
{
    Q_OBJECT

private:
    Dashboard_View  *view;
    Employee_Model  *empModel;
    Dashboard_Model *dashModel;     // Owns all DB logic for the dashboard

    int m_selectedYear = 0;         // Year currently shown in the chart

    void loadEmployeeCards(const QList<User*>& list);
    void loadShiftPanel();
    void loadSalaryChart();

public:
    SessionManager *currentSession;
    Dashboard_Control(QObject *parent = nullptr);
    ~Dashboard_Control();
    Dashboard_View *getView();
    void setView(Dashboard_View *view);
    void init();

signals:
    void profilePageClicked();

public slots:
    void onYearChanged(int year);   // Triggered by View when user picks a different year tab
};
