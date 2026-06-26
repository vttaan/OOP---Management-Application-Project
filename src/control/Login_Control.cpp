
#include "Login_Control.h"


Login_Control::Login_Control(QObject *parent)
    :QObject(parent), view(new Login_View()), currentUser(nullptr){

    connect(view, &Login_View::loginSubmitted, this, &Login_Control::handleLoginSubmission);
}
Login_Control::~Login_Control() {
    if (view) delete view;
    if (currentUser) delete currentUser;
}
void Login_Control::init(){
    view->show();
}

void Login_Control::handleLoginSubmission(const QString& username,const QString& password){
    Login_Model model;
    User *user = model.verifyLogin(username.trimmed(), password.trimmed());

    if (user != nullptr) {
        currentUser = user;


        view->hide();


        if (currentUser->getRole() == "Manager") {
            qDebug() << "Manager";
            // MainController

        } else if (currentUser->getRole() == "Staff") {
            qDebug() << "Staff";
            // MainController
        }

    } else {
        QMessageBox::critical(view, "Login Failed", "Wrong username or Password!");
        view->clearInputs();
    }
}