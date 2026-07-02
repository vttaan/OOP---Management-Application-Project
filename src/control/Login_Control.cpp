#include "Login_Control.h"
#include "view/Login_View.h"


Login_Control::Login_Control(QObject *parent)
    :QObject(parent), view(new Login_View(this)), currentUser(nullptr){

    bool kt = connect(view, &Login_View::loginSubmitted, this, &Login_Control::handleLoginSubmission);
    Q_ASSERT(kt);
}
Login_Control::~Login_Control() {
    if (view) delete view;
    if (currentUser) delete currentUser;
}
void Login_Control::init(){
    view->show();
}

Login_View* Login_Control::getView()  {
    return this->view;
}

void Login_Control::handleLoginSubmission(const QString& username,const QString& password){
    Login_Model model;
    //qDebug() << "pending\n";
    User *user = model.verifyLogin(username, password);
    if (user != nullptr) {
        //qDebug() << "success\n";
        currentUser = user;
        emit loginSuccessful();
        //view->hide();


        if (currentUser->getRole() == "Manager") {
            qDebug() << "Manager";
            // MainController

        } else if (currentUser->getRole() == "Staff") {
            qDebug() << "Staff";
            // MainController
        }

    } else {
        QMessageBox::critical(view, "Login Failed", "Wrong username or Password!");
        view->clearPassword();
    }
}