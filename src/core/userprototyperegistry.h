#ifndef USERPROTOTYPEREGISTRY_H
#define USERPROTOTYPEREGISTRY_H
#include "global.h"
#include "User.h"

class UserPrototypeRegistry
{
public:
    static UserPrototypeRegistry& instance();

    void registerPrototype(const QString& role, User* prototype);


    User* create(const QString& role) const;
    QList<QString> getAvailableRoles() const;

private:
    UserPrototypeRegistry();
    ~UserPrototypeRegistry();
    UserPrototypeRegistry(const UserPrototypeRegistry&) = delete;
    UserPrototypeRegistry& operator=(const UserPrototypeRegistry&) = delete;

    QMap<QString, User*> prototypes;
};

#endif // USERPROTOTYPEREGISTRY_H
