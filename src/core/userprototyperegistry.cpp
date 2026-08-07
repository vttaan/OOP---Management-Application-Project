#include "userprototyperegistry.h"
#include "core/Cashier.h"
#include "core/HallStaff.h"
#include "core/KitchenAssistant.h"
#include "core/Manager.h"

UserPrototypeRegistry::UserPrototypeRegistry()
{
    prototypes.insert("Cashier", new Cashier(0, "", "", "", "", "", "", "", 0.0, false, 0.0));
    prototypes.insert("HallStaff", new HallStaff(0, "", "", "", "", "", "", "", 0, false, 0.0));
    prototypes.insert("KitchenAssistant", new KitchenAssistant(0, "", "", "", "", "", "", "", 0, false, 0.0));
    prototypes.insert("Manager", new Manager("Manager", 0, "", "", "", "", "", "", "", 0));
}

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

QList<QString> UserPrototypeRegistry::getAvailableRoles() const
{
    return prototypes.keys();
}

UserPrototypeRegistry::~UserPrototypeRegistry()
{
    for (auto* p : prototypes)
        delete p;
}