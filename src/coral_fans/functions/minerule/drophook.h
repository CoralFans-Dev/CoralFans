#include "mc/util/Randomize.h"
#include "mc/world/level/block/ResourceDropsContext.h"


namespace coral_fans::functions {
class DropHookManager {
public:
    bool                        bedrockDrop;
    bool                        mbDrop;
    Randomize*                  ram;
    const ResourceDropsContext* dropsContext;

    static DropHookManager& getInstance() {
        static DropHookManager instance;
        return instance;
    }
};
void dropHook();
} // namespace coral_fans::functions