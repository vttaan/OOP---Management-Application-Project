#pragma once
#include "core/User.h"
#include <qstring.h>
class SessionManager
{
private:

    SessionManager(const SessionManager &) = delete;
    SessionManager &operator=(const SessionManager &) = delete;

    User *currentUser = nullptr;

public:

    SessionManager();
    ~SessionManager();
    static SessionManager *getInstance();

    void saveCurrentInfo(User *user);
    void clearInfo();
    User *getCurrentUser() const;
    //void setCurrentUser(User* user);
    bool checkPermission(const QString &requiredRole) const;
};