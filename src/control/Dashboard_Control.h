#pragma once
#include "model/Login_Model.h"
#include <QObject>
#include <QMessageBox>
#include <QDebug>
#include "view/Dashboard_View.h"

class Dashboard_View;

class Dashboard_Control : public QObject {
    Q_OBJECT

private:
    Dashboard_View* view;
    User* currentUser;

public:
    Dashboard_Control(QObject *parent = nullptr);
    ~Dashboard_Control();
    Dashboard_View* getView();
    void setView(Dashboard_View* view);
    void init();
signals:
    void logoutSubmitted() const;
    void profilePageClicked() const;
    //void DashboardSuccessful(User* currentUser);
private slots:
    //void handleDashboardSubmission(const QString& username, const QString& password);
};


