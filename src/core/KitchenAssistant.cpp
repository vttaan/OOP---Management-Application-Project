#include "core/KitchenAssistant.h"
#include "core/UserPrototypeRegistry.h"


namespace {
struct KitchenAssistantRegistrar {
    KitchenAssistantRegistrar() {
        UserPrototypeRegistry::instance().registerPrototype(
            "KitchenAssistant",
            new KitchenAssistant(0, "", "", "", "", "", "", "", 0, false, 0));
    }
} kitchenAssistantRegistrar;
}