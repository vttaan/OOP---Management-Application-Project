#include "global.h"
#include "Profile_View.h"
#include "ui_Profile_View.h"

Profile_View::Profile_View(Profile_Control* controller, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Profile_View)
    , controller(controller)
{
    ui->setupUi(this);
    ui->backButton->setIcon(QIcon(":/images/homeIcon.png"));
    // Provide a default placeholder avatar if none is set yet
    setupAvatar(""); // default avt

    // Connect to editprofile_widget & editpassword_widget out here in profile_view bc those two don't have control file.
    // Create edit profile sliding widget
    editProfileWidget = new EditProfile_Widget(this);
    editProfileWidget->setGeometry(this->rect());
    
    connect(editProfileWidget, &EditProfile_Widget::saveRequested, this, [this](const QString& name, const QString& dob, const QString& address, const QString& phone, const QString& citizenId, const QString& avatarPath) {
        if (this->controller) {
            bool success = this->controller->handleProfileUpdate(name, dob, address, phone, citizenId, avatarPath);
            if (success) {
                editProfileWidget->slideOut();
            } else {
                QMessageBox::warning(this, "Lỗi", "Không thể cập nhật thông tin!");
            }
        }
    });

    // Create edit password sliding widget
    editPasswordWidget = new EditPassword_Widget(this);
    editPasswordWidget->setGeometry(this->rect());

    connect(editPasswordWidget, &EditPassword_Widget::saveRequested, this, [this](const QString& oldPassword, const QString& newPassword) {
        if (this->controller) {
            if (this->editPasswordWidget->txtNewPassword->text() != this->editPasswordWidget->txtConfirmPassword->text()) {
                QMessageBox::warning(this, "Lỗi", "Mật khẩu mới không trùng khớp!");
                return;
            }
            
            PasswordChangeResult result = this->controller->handlePasswordUpdate(oldPassword, newPassword);
            
            if (result == PasswordChangeResult::SUCCESS) {
                editPasswordWidget->slideOut();
                QMessageBox::information(this, "Thành công", "Đổi mật khẩu thành công!");
            } else if (result == PasswordChangeResult::WRONG_OLD_PASSWORD) {
                QMessageBox::warning(this, "Lỗi", "Mật khẩu cũ không đúng!");
            } else if (result == PasswordChangeResult::NEW_PASSWORD_TOO_WEAK) {
                QMessageBox::warning(this, "Lỗi", "Mật khẩu mới quá yếu (ít nhất 6 ký tự)!");
            } else {
                QMessageBox::warning(this, "Lỗi", "Lỗi cơ sở dữ liệu!");
            }
        }
    });
}

Profile_View::~Profile_View()
{
    delete ui;
}

void Profile_View::loadUserData(SessionManager* currentSession) {

    qDebug() << this->getController()->getUser()->getName();
    ui->lblProfileName->setText(this->getController()->getUser()->getName());
    if (this->getController()->getUser()->getRole() == "Manager") ui->lblProfileRole->setText("Quản lý");
    else if (this->getController()->getUser()->getRole() == "Staff") ui->lblProfileRole->setText("Nhân viên");
    ui->lblVal_Id->setText(QString::number(this->getController()->getUser()->getIdEmployee()));
    ui->lblVal_DoB->setText(this->getController()->getUser()->getDOB());
    ui->lblVal_Address->setText(this->getController()->getUser()->getAddress());
    ui->lblVal_CitizenID->setText(this->getController()->getUser()->getIdentityID());
    ui->lblVal_Phone->setText(this->getController()->getUser()->getPhoneNum());
    setupAvatar(this->getController()->getUser()->getAvatarPath());
    //ui->lblVal_Email->setText(this->getController()->getUser()->get...());
    //ui->lblVal_Gender->setText(this->getController()->getUser()->get...());
}

void Profile_View::setupAvatar(const QString& imagePath)
{
    QDir appDir(QCoreApplication::applicationDirPath()); // ......./Debug
    appDir.cdUp(); // ..../MAP/build
    appDir.cdUp(); // ..../MAP
    QString folderPath = appDir.filePath("resources"); // .../MAP/resources
    qDebug() << folderPath;

    QPixmap avatarPixmap(folderPath + "/avatars/" + imagePath);
    if(!imagePath.isEmpty() && QFileInfo::exists(folderPath + "/avatars/" + imagePath)) { // image exists
        //qDebug() << "found";
        avatarPixmap.load(folderPath + "/avatars/" + imagePath);
    }

    if(avatarPixmap.isNull()) {
        //qDebug() << "notfound";
        avatarPixmap.load(":/images/avatarSample.png");
    }

    // Target avatar dimensions (180x180 px matching UI file limits)
    int size = 180;
    QPixmap scaled = avatarPixmap.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    QPixmap rounded(size, size);
    rounded.fill(Qt::transparent);

    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath path;
    path.addRoundedRect(0, 0, size, size, 12, 12);
    painter.setClipPath(path);

    // Center and paint the avatar
    int xOffset = (size - scaled.width()) / 2;
    int yOffset = (size - scaled.height()) / 2;
    painter.drawPixmap(xOffset, yOffset, scaled);
    painter.end();

    ui->lblAvatar->setPixmap(rounded);
}

void Profile_View::loadUserData(const QString& name, const QString& studentId, const QString& dob,
                                const QString& phone, const QString& email, const QString& avatarPath)
{
    // Populate left info panel
    ui->lblProfileName->setText(name);
    setupAvatar(avatarPath);

    // Populate general details
    ui->lblVal_Id->setText(studentId);
    ui->lblVal_DoB->setText(dob);

    // Populate detailed details
    ui->lblVal_Phone->setText(phone);
    ui->lblVal_Email->setText(email);
}

Profile_Control* Profile_View::getController() {
    return controller;
}

void Profile_View::setController(Profile_Control* controller) {
    this->controller = controller;
}

void Profile_View::on_backButton_clicked()
{
    emit this->getController()->backToPrevious();
}

void Profile_View::on_btnEditInfo_clicked()
{
    // Populate the edit panel with the current user's data and show it
    if (this->getController()->getUser()) {
        qDebug() << this->getController()->getUser()->getAvatarPath();
        editProfileWidget->setInitialData(this->getController()->getUser()->getName(), this->getController()->getUser()->getDOB(),
                                          this->getController()->getUser()->getAddress(), this->getController()->getUser()->getPhoneNum(),
                                          this->getController()->getUser()->getIdentityID(), this->getController()->getUser()->getAvatarPath());
    }
    editProfileWidget->slideIn();
}

void Profile_View::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (editProfileWidget) {
        editProfileWidget->setGeometry(this->rect());
    }
}

void Profile_View::on_btnEditPassword_clicked()
{
    if (this->getController()->getUser()) {
        editPasswordWidget->setInitialData();
    }
    editPasswordWidget->slideIn();
}

