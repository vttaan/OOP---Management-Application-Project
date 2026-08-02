#include "userprototyperegistry.h"
UserPrototypeRegistry& UserPrototypeRegistry::instance()
{

    static UserPrototypeRegistry inst;
    return inst;
}

void UserPrototypeRegistry::registerPrototype(const QString& role, User* prototype)
{
    if (prototypes.contains(role))
        delete prototypes.value(role);
    prototypes.insert(role, prototype);
}

User* UserPrototypeRegistry::create(const QString& role) const
{
    auto it = prototypes.find(role);
    if (it == prototypes.end())
        return nullptr;
    return it.value()->clone();
}

UserPrototypeRegistry::~UserPrototypeRegistry()
{
    for (auto* p : prototypes)
        delete p;
}