#include "utils/SessionManage.h"

SessionManager::SessionManager() : currentUser(nullptr) {}
SessionManager::~SessionManager() {
    clearInfo();
}

SessionManager* SessionManager::getInstance() {
    static SessionManager instance;
    return &instance;
}

// void SessionManager::setCurrentUser(User* user) {
//     //if (this->currentUser) delete currentUser;
//     currentUser = user;
// }

void SessionManager::saveCurrentInfo(User* user) {
    //if (this->currentUser == user) return;
    if (this->currentUser != nullptr) delete this->currentUser;
    this->currentUser = user;
}

void SessionManager::clearInfo() {
    if (this->currentUser) {
        delete this->currentUser;
    }
    this->currentUser = nullptr;
}

User* SessionManager::getCurrentUser() const {
    return this->currentUser;
}

bool SessionManager::checkPermission(const QString& requiredRole) const {
    if (currentUser == nullptr) {
        return false;
    }
    return (currentUser->getRole() == requiredRole);
}