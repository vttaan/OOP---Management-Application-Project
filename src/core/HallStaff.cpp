#include "core/HallStaff.h"
#include "core/UserPrototypeRegistry.h"


namespace {
struct HallStaffRegistrar {
    HallStaffRegistrar() {
        UserPrototypeRegistry::instance().registerPrototype(
            "HallStaff",
            new HallStaff(0, "", "", "", "", "", "", "", 0, false, 0));
    }
} hallStaffRegistrar;
}
