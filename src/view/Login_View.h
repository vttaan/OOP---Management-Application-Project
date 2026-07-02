#ifndef LOGINVIEW_H
#define LOGINVIEW_H


//#include "ui_Login_View.h"
#include "control/Login_Control.h"
#include <QWidget>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QCoreApplication>

class Login_Control;

QT_BEGIN_NAMESPACE
namespace Ui { class Login_View; }
QT_END_NAMESPACE



class Login_View : public QWidget
{
	Q_OBJECT
private:
    Ui::Login_View *ui;
    QAction *hidePassword;
    QPixmap bgPixmap;
    void setupUI(); // setup icon/button/textbox/title
    void togglePassword();
    void initSignals(); // set up code connect include Signals and Slots
    void paintEvent(QPaintEvent *event) override;
    Login_Control* controller;
public:
    Login_View(Login_Control* controller = nullptr, QWidget *parent = nullptr);
    void clearInputs();
    void clearPassword();
    Login_Control* getController() const;
    ~Login_View();
signals:
    void loginSubmitted(const QString& username, const QString& password);
    void loginSuccessful();
private slots:
    void on_btnLogin_clicked();

    void on_txtLoginPassword_returnPressed();
};

#endif // LOGINVIEW_H
