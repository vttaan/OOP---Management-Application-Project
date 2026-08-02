#include "Cashier.h"
#include "userprototyperegistry.h"

namespace {
    struct CashierRegistrar {
        CashierRegistrar() {
            UserPrototypeRegistry::instance().registerPrototype(
                "Cashier",
                new Cashier(0, "", "", "", "", "", "", "", 0, false, 0));
        }
    } cashierRegistrar;
}