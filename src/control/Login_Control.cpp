#include "Login_Control.h"
#include "view/Login_View.h"
#include "main_control.h"

Login_Control::Login_Control(QObject *parent)
    : QObject(parent), view(new Login_View(this)), currentUser(nullptr)
{
    bool ok = connect(view, &Login_View::loginSubmitted, this, &Login_Control::handleLoginSubmission);
    Q_ASSERT(ok);
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
    if (view)
        view->show();
}

Login_View *Login_Control::getView()
{
    return this->view;
}

void Login_Control::setView(Login_View *view)
{
    this->view = view;
    if (this->view)
    {
        QObject::connect(this->view, &Login_View::loginSubmitted, this, &Login_Control::handleLoginSubmission);
    }
}

void Login_Control::handleLoginSubmission(const QString &username, const QString &password)
{
    Login_Model model;
    User *user = model.verifyLogin(username.trimmed(), password.trimmed());

    if (user != nullptr)
    {
        currentUser = user;
        emit loginSuccessful(this->currentUser);

        if (view)
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
        if (view)
            view->clearPassword();
    }
}

User *Login_Control::getUser() { return currentUser; }