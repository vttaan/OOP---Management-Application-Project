#include "global.h"
#include "view/Login_View.h"
#include "Login_Control.h"
#include "model/Change_password.h"

Login_Control::Login_Control(QObject *parent)
    : QObject(parent), view(nullptr), currentSession(SessionManager::getInstance())
{
}

Login_Control::~Login_Control()
{
    clearPendingInitialPasswordChange();
    // view is owned by View_Navigator, do not delete here
    // currentSession is owned by Control_Navigator, do not delete here
}

void Login_Control::init()
{
    clearPendingInitialPasswordChange();
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
        QObject::connect(this->view, &Login_View::loginSubmitted,
                         this, &Login_Control::handleLoginSubmission);
        QObject::connect(this->view, &Login_View::initialPasswordChangeSubmitted,
                         this, &Login_Control::handleInitialPasswordChange);
    }
}

void Login_Control::handleLoginSubmission(const QString &username, const QString &password)
{
    clearPendingInitialPasswordChange();

    Login_Model model;
    bool mustChangeInitialPassword = false;
    User *newUser = model.verifyLogin(username, password, &mustChangeInitialPassword);
    if (!newUser)
    {
        if (view)
        {
            view->showError("Sai tài khoản hoặc mật khẩu!");
            view->clearPassword();
        }
        return;
    }

    if (mustChangeInitialPassword)
    {
        pendingInitialPasswordUser = newUser;
        pendingInitialPassword = password;
        if (view)
            view->beginInitialPasswordChange();
        return;
    }

    completeLogin(newUser);
}

void Login_Control::handleInitialPasswordChange(const QString &newPassword)
{
    if (!pendingInitialPasswordUser)
    {
        if (view)
        {
            view->resetToLoginMode();
            view->showInitialPasswordChangeError(
                "Phiên đổi mật khẩu đã hết hạn. Vui lòng đăng nhập lại.");
        }
        return;
    }

    if (newPassword == pendingInitialPassword)
    {
        if (view)
            view->showInitialPasswordChangeError(
                "Mật khẩu mới phải khác mật khẩu do quản lý cấp.");
        return;
    }

    Change_password passwordModel;
    const PasswordChangeResult result = passwordModel.updatePassword(
        pendingInitialPasswordUser->getIdEmployee(), pendingInitialPassword, newPassword);

    if (result == PasswordChangeResult::SUCCESS)
    {
        User *user = pendingInitialPasswordUser;
        pendingInitialPasswordUser = nullptr;
        pendingInitialPassword.clear();
        if (view)
            view->resetToLoginMode();
        completeLogin(user);
        return;
    }

    QString errorMessage;
    switch (result)
    {
    case PasswordChangeResult::NEW_PASSWORD_TOO_WEAK:
        errorMessage = Change_password::getPasswordStrengthError(newPassword);
        if (errorMessage.isEmpty()) {
            errorMessage = "Mật khẩu mới không đủ mạnh.";
        }
        break;
    case PasswordChangeResult::WRONG_OLD_PASSWORD:
        errorMessage = "Mật khẩu ban đầu không còn hợp lệ. Vui lòng đăng nhập lại.";
        clearPendingInitialPasswordChange();
        if (view)
            view->resetToLoginMode();
        break;
    default:
        errorMessage = "Không thể cập nhật mật khẩu. Vui lòng thử lại.";
        break;
    }

    if (view)
        view->showInitialPasswordChangeError(errorMessage);
}

void Login_Control::completeLogin(User *user)
{
    currentSession->saveCurrentInfo(user);
    emit loginSuccessful(currentSession->getCurrentUser());

    if (currentSession->getCurrentUser()->getRole() == "Manager")
        qDebug() << "Manager logged in - Mo giao dien Quan ly";
    else if (currentSession->getCurrentUser()->getRole() == "Staff")
        qDebug() << "Staff logged in - Mo giao dien Nhan vien";
}

void Login_Control::clearPendingInitialPasswordChange()
{
    delete pendingInitialPasswordUser;
    pendingInitialPasswordUser = nullptr;
    pendingInitialPassword.clear();
}
