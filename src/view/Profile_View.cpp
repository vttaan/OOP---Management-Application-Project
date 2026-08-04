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
    setupAvatar(""); // default avt

    editProfileWidget = new EditProfile_Widget(this);
    editProfileWidget->setGeometry(this->rect());
    
    connect(editProfileWidget, &EditProfile_Widget::saveRequested, this, [this](const QString& name, const QString& dob, const QString& address, const QString& phone, const QString& citizenId, const QString& avatarPath, const QString& gender) {
        if (this->controller) {
            bool success = this->controller->handleProfileUpdate(name, dob, address, phone, citizenId, avatarPath, gender);
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
    if (!this->getController() || !this->getController()->getUser()) return;

    User* user = this->getController()->getUser();
    ui->lblProfileName->setText(user->getName());

    QString roleStr = user->getRole();
    QString rUpper = roleStr.toUpper();
    QString roleDisplay;
    if (rUpper.contains("CASHIER") || rUpper.contains("THU NGÂN")) {
        roleDisplay = "Thu ngân";
    } else if (rUpper.contains("HALL") || rUpper.contains("SẢNH")) {
        roleDisplay = "Nhân viên sảnh";
    } else if (rUpper.contains("KITCHEN") || rUpper.contains("PHỤ BẾP") || rUpper.contains("BEP")) {
        roleDisplay = "Phụ bếp";
    } else if (rUpper.contains("MANAGER") || rUpper.contains("MANAGE") || rUpper.contains("ADMIN") || rUpper.contains("QUẢN LÝ")) {
        roleDisplay = "Quản lý";
    } else {
        roleDisplay = "Nhân viên";
    }
    QString typeEmployee = (user->getIsFixedEmployee()) ? " toàn thời gian" : " bán thời gian";
    ui->lblProfileRole->setText(roleDisplay + typeEmployee);

    ui->lblVal_Id->setText(QString::number(user->getIdEmployee()));
    ui->lblVal_DoB->setText(user->getDOB());
    ui->lblVal_Address->setText(user->getAddress());
    ui->lblVal_CitizenID->setText(user->getIdentityID());
    ui->lblVal_Phone->setText(user->getPhoneNum());
    setupAvatar(user->getAvatarPath());
    ui->lblVal_Gender->setText(user->getGender());
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
    ui->lblVal_Email->setHidden(true);
    ui->lblTitle_Email->setHidden(true);
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
                                          this->getController()->getUser()->getIdentityID(), this->getController()->getUser()->getAvatarPath(),
                                          this->getController()->getUser()->getGender());
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

