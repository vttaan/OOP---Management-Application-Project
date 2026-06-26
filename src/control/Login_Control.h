#pragma once
#include "view/Login_View.h"
#include "model/Login_Model.h"
#include <QObject>
#include <QMessageBox>
#include <QDebug>

class Login_Control : public QObject {
    Q_OBJECT

private:
    Login_View* view;
    User* currentUser;

public:
    Login_Control(QObject *parent = nullptr);
    ~Login_Control();

    void init();
private slots:
    void handleLoginSubmission(const QString& username, const QString& password);
};


