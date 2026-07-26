#include "global.h"
//#include "Control_Navigator.h"
#include "view/Login_View.h"
#include "Login_Control.h"

Login_Control::Login_Control(QObject *parent)
    : QObject(parent), view(nullptr), currentSession(nullptr)
{
}

Login_Control::~Login_Control()
{
    // view is owned by View_Navigator, do not delete here
    // currentSession is owned by Control_Navigator, do not delete here
}

void Login_Control::init()
{
    if (view)
        view->clearInputs();
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
        this->view->setController(this);
        QObject::connect(this->view, &Login_View::loginSubmitted, this, &Login_Control::handleLoginSubmission);
    }
}

void Login_Control::handleLoginSubmission(const QString &username, const QString &password)
{
    Login_Model model;
    User *newUser = model.verifyLogin(username, password);
    if (newUser != nullptr)
    {
        this->currentSession->saveCurrentInfo(newUser);
        emit loginSuccessful(this->currentSession->getCurrentUser());

        if (this->currentSession->getCurrentUser()->getRole() == "Manager")
        {
            qDebug() << "Manager logged in - Mo giao dien Quan ly";
        }
        else if (this->currentSession->getCurrentUser()->getRole() == "Staff")
        {
            qDebug() << "Staff logged in - Mo giao dien Nhan vien";
        }
    }
    else
    {
        QMessageBox::critical(view, "Login Failed", "Wrong username or Password!");
        if (view)
            view->clearPassword();
    }
}

//User *Login_Control::getUser() { return this->currentSession->getCurrentUser(); }
