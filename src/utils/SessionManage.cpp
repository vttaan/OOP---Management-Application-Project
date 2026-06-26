#include "utils/SessionManage.h"

SessionManager::SessionManager() : currentUser(nullptr) {}
SessionManager::~SessionManager() {
    clearInfo();
}

SessionManager* SessionManager::getInstance() {
    static SessionManager instance;
    return &instance;
}

void SessionManager::saveCurrentInfo(User* user) {
    if (user != nullptr) {
        this->currentUser = user;
    }
}

void SessionManager::clearInfo() {
    this->currentUser = nullptr;
}

const User* SessionManager::getCurrentUser() const {
    return this->currentUser;
}

bool SessionManager::checkPermission(const QString& requiredRole) const {
    if (currentUser == nullptr) {
        return false;
    }
    return (currentUser->getRole() == requiredRole);
}