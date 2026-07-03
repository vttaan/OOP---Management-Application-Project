#include "Login_View.h"
#include "ui_Login_View.h"
#include "control/Login_Control.h"

Login_View::Login_View(Login_Control* controller, QWidget *parent)
	: QWidget(parent)
    , ui(new Ui::Login_View())
    , controller(controller)
{
	ui->setupUi(this);
    ui->btnLogin->setCheckable(true);
    ui->txtLoginUsername->setFocus();
    setupUI();
    initSignals();
    if (controller) {
        connect(controller, &Login_Control::loginSuccessful, this, &Login_View::loginSuccessful);

    }
}

Login_Control* Login_View::getController() const {
    return this->controller;
}

Login_View::~Login_View()
{
	delete ui;
}


void Login_View::clearInputs() {
    ui->txtLoginUsername->clear();
    ui->txtLoginPassword->clear();
    ui->txtLoginUsername->setFocus();
}

void Login_View::clearPassword() {
    ui->txtLoginPassword->clear();
    ui->txtLoginPassword->setFocus();
}

void Login_View::setupUI(){
    ui->txtLoginPassword->setEchoMode(QLineEdit::Password);
    bgPixmap = QPixmap(":/images/login_bg.jpg");
    hidePassword = ui->txtLoginPassword->addAction(
        QIcon(":/images/eyeOpen.svg"),
        QLineEdit::TrailingPosition
        );
}

void Login_View::initSignals(){
    connect(hidePassword,&QAction::triggered,this,&Login_View::togglePassword);
}


void Login_View::togglePassword()
{
    if (ui->txtLoginPassword->echoMode() == QLineEdit::Password) {

        ui->txtLoginPassword->setEchoMode(QLineEdit::Normal);
        hidePassword->setIcon(QIcon(":/images/eyeClosed.svg"));

    } else {

        ui->txtLoginPassword->setEchoMode(QLineEdit::Password);
        hidePassword->setIcon(QIcon(":/images/eyeOpen.svg"));

    }
}
void Login_View::on_btnLogin_clicked() {
    QString user = ui->txtLoginUsername->text();
    QString pass = ui->txtLoginPassword->text();

    if (user.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "Warning - Invalid information", "Enter fully your password and username");
        return;
    }

    emit loginSubmitted(user, pass);
}
void Login_View::paintEvent(QPaintEvent *event)
{

    QWidget::paintEvent(event);

    if (bgPixmap.isNull()) {
        qDebug() << "paintEvent bgPixmap NULL check your file qrc";
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QRect rightRect = ui->frmLoginRight->geometry();


    QPixmap scaled = bgPixmap.scaled(
        rightRect.size(),
        Qt::KeepAspectRatioByExpanding,
        Qt::SmoothTransformation
        );
    int x = rightRect.x() + (rightRect.width()  - scaled.width())  / 2;
    int y = rightRect.y() + (rightRect.height() - scaled.height()) / 2;
    painter.setClipRect(rightRect);
    painter.drawPixmap(x, y, scaled);

    //painter.fillRect(rightRect, QColor(0, 20, 60, 80));
}

void Login_View::on_txtLoginPassword_returnPressed()
{
    QString user = ui->txtLoginUsername->text();
    QString pass = ui->txtLoginPassword->text();

    ui->btnLogin->setEnabled(true);
    ui->btnLogin->setCheckable(true);
    if (user.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "Warning - Invalid information", "Enter fully your password and username");
        return;
    }
    emit loginSubmitted(user, pass);
    qDebug() << "login\n";
}

