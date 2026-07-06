#include "Profile_Control.h"
#include "view/Profile_View.h"

Profile_Control::Profile_Control(QObject *parent)
    :QObject(parent), view(nullptr), currentSession(nullptr){

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
        this->view->setController(this);
    }
}

User* Profile_Control::getUser() { return this->currentSession->getCurrentUser(); }


bool Profile_Control::handleProfileUpdate(const QString& name, const QString& dob, const QString& address, const QString& phoneNum, const QString& citizenId, const QString& avatarPath) {
    if (!currentSession->getCurrentUser()) return false;
    
    // Attempt database update via the model
    bool success = model.updateProfile(currentSession->getCurrentUser()->getIdEmployee(), name, dob, address, phoneNum, citizenId, avatarPath);
    if (success) {
        // Database update succeeded, now update the local User object so other views see it
        currentSession->getCurrentUser()->setName(name);
        currentSession->getCurrentUser()->setDOB(dob);
        currentSession->getCurrentUser()->setAddress(address);
        currentSession->getCurrentUser()->setPhoneNum(phoneNum);
        currentSession->getCurrentUser()->setIndentityID(citizenId);
        currentSession->getCurrentUser()->setAva(avatarPath);
        
        // Refresh the profile view with the new data
        view->loadUserData(currentSession);
        return true;
    }
    return false;
}

bool Profile_Control::handlePasswordUpdate(const QString& password) {
    if (!currentSession->getCurrentUser()) return false;

    // Attempt database update via the model
    bool success = model.updatePassword(currentSession->getCurrentUser()->getIdEmployee(), password);
    if (success) {
        currentSession->getCurrentUser()->setName(password);

        // Refresh the profile view with the new data
        view->loadUserData(currentSession);
        return true;
    }
    return false;
}

bool Profile_Control::checkIfMatchOldPassword(const QString& password) {
    return this->model.checkIfUserExist(this->currentSession->getCurrentUser()->getIdEmployee(), password);
}


