#include "Login_Control.h"
#include "main_control.h"
#include "view/Login_View.h"

Login_Control::Login_Control(QObject *parent)
    : QObject(parent), view(new Login_View()), currentUser(nullptr)
{

    connect(view, &Login_View::loginSubmitted, this, &Login_Control::handleLoginSubmission);
}

Login_Control::~Login_Control()
{
    if (view)
        delete view;
    if (currentUser)
        delete currentUser;
}

void Login_Control::init()
{
    view->show();
}

Login_View *Login_Control::getView()
{
    return this->view;
}

void Login_Control::handleLoginSubmission(const QString &username, const QString &password)
{
    Login_Model model;
    User *user = model.verifyLogin(username.trimmed(), password.trimmed());

    if (user != nullptr)
    {
        currentUser = user;

        view->hide();

        if (currentUser->getRole() == "Manager")
        {
            qDebug() << "Manager logged in - Mo giao dien Quan ly";

            Main_Control *mainCtrl = new Main_Control(this);
            mainCtrl->init();
        }
        else if (currentUser->getRole() == "Staff")
        {
            qDebug() << "Staff logged in - Mo giao dien Nhan vien";

            Main_Control *mainCtrl = new Main_Control(this);
            mainCtrl->init();
        }
    }
    else
    {
        QMessageBox::critical(view, "Login Failed", "Wrong username or Password!");
        view->clearPassword();
    }
}