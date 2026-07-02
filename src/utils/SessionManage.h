#pragma once
#include "core/User.h"
#include <qstring.h>
class SessionManager
{
private:
    SessionManager();
    ~SessionManager();

    // Prevent copying and assignment
    SessionManager(const SessionManager &) = delete;
    SessionManager &operator=(const SessionManager &) = delete;

    User *currentUser;

public:
    static SessionManager *getInstance();

    void saveCurrentInfo(User *user);
    void clearInfo();
    const User *getCurrentUser() const;
    bool checkPermission(const QString &requiredRole) const;
};