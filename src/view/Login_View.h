#include "global.h"
#ifndef LOGINVIEW_H
#define LOGINVIEW_H

class Login_Control;

QT_BEGIN_NAMESPACE
namespace Ui
{
    class Login_View;
}
QT_END_NAMESPACE

class Login_View : public QWidget
{
    Q_OBJECT
private:
    enum class PageMode
    {
        Login,
        InitialPasswordChange
    };

    Ui::Login_View *ui;

    QAction *hidePassword;
    QAction *hideNewPassword;
    QPixmap bgPixmap;
    PageMode pageMode = PageMode::Login;
    void setupUI(); // setup icon/button/textbox/title
    void togglePassword();
    void toggleNewPassword();
    void initSignals(); // set up code connect include Signals and Slots
    void paintEvent(QPaintEvent *event) override;

public:
    Login_View(QWidget *parent = nullptr);
    void clearInputs();
    void clearPassword();
    void beginInitialPasswordChange();
    void resetToLoginMode();
    void showInitialPasswordChangeError(const QString &message);
    void showError(const QString &message);
    void clearError();
    ~Login_View();
signals:
    void loginSubmitted(const QString &username, const QString &password);
    void initialPasswordChangeSubmitted(const QString &newPassword);
    void loginSuccessful();
private slots:
    void on_btnLogin_clicked();

    void on_txtLoginPassword_returnPressed();
};

#endif // LOGINVIEW_H
