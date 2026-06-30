#ifndef LOGINVIEW_H
#define LOGINVIEW_H


//#include "ui_Login_View.h"

#include <QWidget>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QCoreApplication>

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

public:
	Login_View(QWidget *parent = nullptr);
	~Login_View();
    void clearInputs();
    void clearPassword();

signals:
    void loginSubmitted(const QString& username, const QString& password);
private slots:
    void on_btnLogin_clicked();

    void on_txtLoginPassword_returnPressed();
};

#endif // LOGINVIEW_H
