#pragma once
#include "model/Login_Model.h"
#include <QObject>
#include <QMessageBox>
#include <QDebug>

class Profile_View;

class Profile_Control : public QObject {
    Q_OBJECT

private:
    Profile_View* view;
    User* currentUser;

public:
    Profile_Control(QObject *parent = nullptr);
    ~Profile_Control();
    Profile_View* getView();
    void setView(Profile_View* view);
    void init();
signals:
    void backToPrevious();
    //void loginSuccessful(User* currentUser);
private slots:
    //void handleLoginSubmission(const QString& username, const QString& password);
};


