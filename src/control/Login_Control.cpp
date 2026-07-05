//#include "Control_Navigator.h"
#include "view/Login_View.h"
#include "Login_Control.h"



Login_Control::Login_Control(QObject *parent)
    : QObject(parent), view(new Login_View(this)), currentSession(nullptr)
{
    bool ok = connect(view, &Login_View::loginSubmitted, this, &Login_Control::handleLoginSubmission);
    Q_ASSERT(ok);
}

Login_Control::~Login_Control()
{
    if (view)
        delete view;
}

void Login_Control::init()
{
    this->getView()->clearInputs();
    if (view)
        view->show();
}

Login_View *Login_Control::getView()
{
    return this->view;
}

void Login_Control::setView(Login_View* view) {
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
    User* newUser = model.verifyLogin(username, password);
    //qDebug() << "pending\n";
    if (newUser != nullptr)
    {
        //qDebug() << "found\n";
        this->currentSession->saveCurrentInfo(newUser); // change the main control navigator's session into the correct user
        emit loginSuccessful(this->currentSession->getCurrentUser());

        // if (view)
        //     view->hide();

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
        view->clearPassword();
    }
}

//User *Login_Control::getUser() { return this->currentSession->getCurrentUser(); }
