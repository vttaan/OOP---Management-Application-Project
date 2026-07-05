#pragma once
#include "utils/SessionManage.h"
#include "model/Profile_Model.h"
#include <QObject>
#include <QMessageBox>
#include <QDebug>

class Profile_View;

class Profile_Control : public QObject {
    Q_OBJECT

private:
    Profile_View* view;
    Profile_Model model;

public:
    Profile_Control(QObject *parent = nullptr);
    ~Profile_Control();
    SessionManager* currentSession;
    Profile_View* getView();
    //void editUserInfo();
    void setView(Profile_View* view);
    void init();
    bool checkIfMatchOldPassword(const QString& password);
    User* getUser();
    bool handleProfileUpdate(const QString& name, const QString& dob, const QString& address, const QString& phoneNum, const QString& citizenId, const QString& avatarPath);
    bool handlePasswordUpdate(const QString& password);
signals:
    void backToPrevious();
    //void loginSuccessful(User* currentUser);
private slots:
    //void handleLoginSubmission(const QString& username, const QString& password);
};
