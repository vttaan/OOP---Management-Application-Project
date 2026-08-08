#pragma once

#include "global.h"
#include "core/User.h"

class SessionManager
{
private:
    SessionManager();
    ~SessionManager();

    SessionManager(const SessionManager &) = delete;
    SessionManager &operator=(const SessionManager &) = delete;

    User *currentUser = nullptr;

public:
    static SessionManager *getInstance();

    void saveCurrentInfo(User *user);
    void clearInfo();
    User *getCurrentUser() const;
    // void setCurrentUser(User* user);
    bool checkPermission(const QString &requiredRole) const;
};