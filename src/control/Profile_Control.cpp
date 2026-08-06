#include "global.h"
#include "Profile_Control.h"
#include "view/Profile_View.h"

Profile_Control::Profile_Control(QObject *parent)
    :QObject(parent), view(nullptr), currentSession(SessionManager::getInstance()){
}
Profile_Control::~Profile_Control() {
    if (view) delete view;
}
void Profile_Control::init(){
    view->show();
}

Profile_View* Profile_Control::getView()  {
    return this->view;
}

void Profile_Control::setView(Profile_View* view) {
    this->view = view;
    if (this->view) {
        connect(this->view, &Profile_View::requestProfileUpdate, this, &Profile_Control::handleProfileUpdate);
        connect(this->view, &Profile_View::requestPasswordUpdate, this, &Profile_Control::handlePasswordUpdate);
        connect(this->view, &Profile_View::backToPrevious, this, &Profile_Control::backToPrevious);
    }
}

User* Profile_Control::getUser() { return this->currentSession->getCurrentUser(); }

void Profile_Control::handleProfileUpdate(const QString& name, const QString& dob, const QString& address, const QString& phoneNum, const QString& citizenId, const QString& avatarPath, const QString& gender) {
    if (!currentSession->getCurrentUser()) {
        if(view) view->showProfileUpdateResult(false, "Phiên đăng nhập không hợp lệ.");
        return;
    }
    // Save avatar to avatars folder
    QString localAvatarName = saveAvatarLocally(this->currentSession->getCurrentUser()->getIdEmployee(), avatarPath);

    // Attempt database update via the model
    bool success = model.updateProfile(currentSession->getCurrentUser()->getIdEmployee(), name, dob, address, phoneNum, citizenId, localAvatarName, gender);
    if (success) {
        // Database update succeeded, now update the local User object so other views see it
        currentSession->getCurrentUser()->setName(name);
        currentSession->getCurrentUser()->setDOB(dob);
        currentSession->getCurrentUser()->setAddress(address);
        currentSession->getCurrentUser()->setPhoneNum(phoneNum);
        currentSession->getCurrentUser()->setIndentityID(citizenId);
        currentSession->getCurrentUser()->setAva(localAvatarName);
        currentSession->getCurrentUser()->setGender(gender);
        
        // Refresh the profile view with the new data
        view->loadUserData();
        emit profileUpdated();
        if(view) view->showProfileUpdateResult(true);
    } else {
        if(view) view->showProfileUpdateResult(false, "Không thể cập nhật CSDL.");
    }
}

void Profile_Control::handlePasswordUpdate(const QString& oldPassword, const QString& newPassword) {
    if (!currentSession->getCurrentUser()) {
        if(view) view->showPasswordUpdateResult(false, "Phiên đăng nhập không hợp lệ.");
        return;
    }

    // Attempt database update via the model
    PasswordChangeResult result = model.updatePassword(currentSession->getCurrentUser()->getIdEmployee(), oldPassword, newPassword);
    if (view) {
        if (result == PasswordChangeResult::SUCCESS) {
            view->showPasswordUpdateResult(true);
        } else if (result == PasswordChangeResult::WRONG_OLD_PASSWORD) {
            view->showPasswordUpdateResult(false, "Mật khẩu cũ không đúng!");
        } else if (result == PasswordChangeResult::NEW_PASSWORD_TOO_WEAK) {
            view->showPasswordUpdateResult(false, "Mật khẩu mới quá yếu (ít nhất 6 ký tự)!");
        } else {
            view->showPasswordUpdateResult(false, "Lỗi cơ sở dữ liệu!");
        }
    }
}

bool Profile_Control::checkIfMatchOldPassword(const QString& password) {
    return this->model.checkIfUserExist(this->currentSession->getCurrentUser()->getIdEmployee(), password);
}

void Profile_Control::loadUserData() { this->getView()->loadUserData(); }

QString Profile_Control::saveAvatarLocally(int empId,
                                          const QString &sourcePath) {
    if (sourcePath.isEmpty())
        return "";

    // If it's already a resource path, do not copy
    if (sourcePath.startsWith(":/"))
        return sourcePath;

    // If it's already just the name of a local avatar file (e.g. avatar_1001.png),
    // which does not contain any directory separators, return it as-is.
    if (!sourcePath.contains('/') && !sourcePath.contains('\\')) {
        return sourcePath;
    }

    QFileInfo sourceInfo(sourcePath);
    if (!sourceInfo.exists()) {
        if (sourceInfo.fileName() == sourcePath)
            return sourcePath;
        return "";
    }

    QDir appDir = QCoreApplication::applicationDirPath(); // debug folder
    appDir.cdUp(); // build folder
    appDir.cdUp(); // MAP folder
    // Create "avatars" directory in application directory if not exists
    QString targetDir = appDir.filePath("resources") + "/avatars"; // avatars folder in resouces folder
    QDir dir(targetDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // Generate path: appDir/avatars/avatar_X.ext
    QString ext = sourceInfo.suffix().toLower();
    if (ext.isEmpty())
        ext = "png";
    QString targetPath =
        QString("%1/avatar_%2.%3").arg(targetDir).arg(empId).arg(ext);

    // If the source file is already the target file, we don't need to copy
    if (QFileInfo(targetPath).absoluteFilePath() == sourceInfo.absoluteFilePath()) {
        return QString("avatar_%1.%2").arg(empId).arg(ext);
    }

    // If file already exists, remove it first to overwrite
    if (QFile::exists(targetPath)) {
        QFile::remove(targetPath);
    }

    if (QFile::copy(sourcePath, targetPath)) {
        return QString("avatar_%1.%2").arg(empId).arg(ext);
    }

    return QString("avatar_%1.%2").arg(empId).arg(ext);
}