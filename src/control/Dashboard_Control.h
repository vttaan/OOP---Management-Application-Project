#pragma once

#include "global.h"
#include "utils/SessionManage.h"

class Dashboard_View;
class Dashboard_Model;

class Dashboard_Control : public QObject
{
    Q_OBJECT

private:
    Dashboard_View *view;

public:
    SessionManager *currentSession;
    Dashboard_Control(QObject *parent = nullptr);
    ~Dashboard_Control();
    Dashboard_View *getView();
    void setView(Dashboard_View *view);
    void init();
signals:
    //void logoutSubmitted();
    void profilePageClicked();
    //void employeeClicked();
    // void DashboardSuccessful(User* currentUser);
private slots:
    // void handleDashboardSubmission(const QString& username, const QString& password);
};
