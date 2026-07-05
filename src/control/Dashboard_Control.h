#pragma once
#include <QObject>
#include <QMessageBox>
#include <QDebug>
#include "utils/SessionManage.h"

class Dashboard_View;

class Dashboard_Control : public QObject {
    Q_OBJECT

private:
    Dashboard_View* view;

public:
    Dashboard_Control(QObject *parent = nullptr);
    ~Dashboard_Control();
    Dashboard_View* getView();
    SessionManager* currentSession;
    void setView(Dashboard_View* view);
    void init();
signals:
    void logoutSubmitted();
    void profilePageClicked();
    //void DashboardSuccessful(User* currentUser);
private slots:

    //void handleDashboardSubmission(const QString& username, const QString& password);
};


