#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/storage/DBChunkStorage.h"
#include <unordered_map>


namespace coral_fans::functions::locate {
class LocateManager {
private:
    std::mutex                   netherDecorationThreadIdsLock;
    std::vector<std::thread::id> netherDecorationThreadIds;

    std::mutex                                                  ancientDebrisPosMapLock;
    std::unordered_map<ChunkPos, std::pair<BlockPos, BlockPos>> ancientDebrisPosMap;

    struct DuplicatableHook1;
    struct DuplicatableHook2;

public:
    static LocateManager& getInstance() {
        static LocateManager instance;
        return instance;
    }
    static void hook(bool);
};
} // namespace coral_fans::functions::locate