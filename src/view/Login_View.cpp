#include "global.h"
#include "Login_View.h"
#include "ui_Login_View.h"

Login_View::Login_View(QWidget *parent)
	: QWidget(parent)
    , ui(new Ui::Login_View())
{
	ui->setupUi(this);
    ui->btnLogin->setCheckable(true);
    ui->lblInitialPasswordNotice->hide();
    ui->txtLoginUsername->setFocus();
    setupUI();
    initSignals();


}




Login_View::~Login_View()
{
	delete ui;
}

Login_Control *Login_View::getController() const
{
    return controller;
}

void Login_View::setController(Login_Control *newController)
{
    controller = newController;
}

void Login_View::clearInputs() {
    resetToLoginMode();
    ui->txtLoginUsername->clear();
    ui->txtLoginPassword->clear();
    ui->txtLoginUsername->setFocus();
}

void Login_View::clearPassword() {
    ui->txtLoginPassword->clear();
    ui->txtLoginPassword->setFocus();
}

void Login_View::beginInitialPasswordChange() {
    pageMode = PageMode::InitialPasswordChange;
    ui->lblTitle->setText("Đổi mật khẩu");
    ui->lblInitialPasswordNotice->show();
    ui->lblUsername->setText("Mật khẩu mới");
    ui->lblPassword->setText("Nhập lại mật khẩu");
    ui->txtLoginUsername->clear();
    ui->txtLoginUsername->setEchoMode(QLineEdit::Password);
    ui->txtLoginUsername->setPlaceholderText("Ít nhất 6 ký tự");
    hideNewPassword->setIcon(QIcon(":/images/eyeOpen.svg"));
    ui->txtLoginUsername->addAction(hideNewPassword, QLineEdit::TrailingPosition);
    ui->txtLoginPassword->clear();
    ui->txtLoginPassword->setEchoMode(QLineEdit::Password);
    hidePassword->setIcon(QIcon(":/images/eyeOpen.svg"));
    ui->txtLoginPassword->setPlaceholderText("Nhập lại mật khẩu mới");
    ui->btnLogin->setText("Đổi mật khẩu");
    ui->txtLoginUsername->setFocus();
}

void Login_View::resetToLoginMode() {
    if (pageMode == PageMode::Login)
        return;

    pageMode = PageMode::Login;
    ui->lblTitle->setText("Đăng Nhập");
    ui->lblInitialPasswordNotice->hide();
    ui->lblUsername->setText("Tên đăng nhập");
    ui->lblPassword->setText("Mật Khẩu");
    ui->txtLoginUsername->removeAction(hideNewPassword);
    ui->txtLoginUsername->setEchoMode(QLineEdit::Normal);
    ui->txtLoginUsername->setPlaceholderText("Mời nhập tên đăng nhập");
    ui->txtLoginPassword->setPlaceholderText("Mời nhập mật khẩu");
    ui->btnLogin->setText("Đăng nhập");
}

void Login_View::showInitialPasswordChangeError(const QString& message) {
    QMessageBox::warning(this, "Không thể đổi mật khẩu", message);
    ui->txtLoginUsername->clear();
    ui->txtLoginPassword->clear();
    ui->txtLoginUsername->setFocus();
}

void Login_View::setupUI(){
    ui->txtLoginPassword->setEchoMode(QLineEdit::Password);
    bgPixmap = QPixmap(":/images/login_bg.png");
    hidePassword = ui->txtLoginPassword->addAction(
        QIcon(":/images/eyeOpen.svg"),
        QLineEdit::TrailingPosition
        );
    hideNewPassword = new QAction(QIcon(":/images/eyeOpen.svg"), QString(), this);
}

void Login_View::initSignals(){
    connect(hidePassword,&QAction::triggered,this,&Login_View::togglePassword);
    connect(hideNewPassword, &QAction::triggered, this, &Login_View::toggleNewPassword);
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

void Login_View::toggleNewPassword()
{
    if (ui->txtLoginUsername->echoMode() == QLineEdit::Password) {
        ui->txtLoginUsername->setEchoMode(QLineEdit::Normal);
        hideNewPassword->setIcon(QIcon(":/images/eyeClosed.svg"));
    } else {
        ui->txtLoginUsername->setEchoMode(QLineEdit::Password);
        hideNewPassword->setIcon(QIcon(":/images/eyeOpen.svg"));
    }
}

void Login_View::on_btnLogin_clicked() {
    const QString firstValue = ui->txtLoginUsername->text();
    const QString secondValue = ui->txtLoginPassword->text();

    if (pageMode == PageMode::InitialPasswordChange) {
        if (firstValue.isEmpty() || secondValue.isEmpty()) {
            QMessageBox::warning(this, "Mật khẩu chưa hợp lệ", "Vui lòng nhập và xác nhận mật khẩu mới.");
            return;
        }
        if (firstValue != secondValue) {
            QMessageBox::warning(this, "Mật khẩu không khớp", "Hai mật khẩu mới phải giống nhau.");
            ui->txtLoginPassword->setFocus();
            return;
        }
        emit initialPasswordChangeSubmitted(firstValue);
        return;
    }

    if (firstValue.isEmpty() || secondValue.isEmpty()) {
        QMessageBox::warning(this, "Cảnh báo - Sai thông tin", "Nhập lại đầy đủ tên đăng nhập và mật khẩu");
        return;
    }

    emit loginSubmitted(firstValue, secondValue);
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
}

void Login_View::on_txtLoginPassword_returnPressed()
{
    ui->btnLogin->setEnabled(true);
    ui->btnLogin->setCheckable(true);
    on_btnLogin_clicked();
}

