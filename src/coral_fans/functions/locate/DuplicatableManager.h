#include "bsci/GeometryGroup.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/block/Block.h"
#include <unordered_map>


namespace coral_fans::functions::locate {
class DuplicatableManager {
private:
    struct BlockPosPairHash {
        size_t operator()(const std::pair<BlockPos, BlockPos>& p) const {
            // 使用 Boost 的 hash_combine 或自定义组合
            size_t h1 = std::hash<BlockPos>()(p.first);
            size_t h2 = std::hash<BlockPos>()(p.second);

            return h1 ^ (h2 << 1);
        }
    };

    struct NetherData {
        std::unordered_set<std::pair<BlockPos, BlockPos>, BlockPosPairHash> ancientDebrisPosSet;
    };

    struct NetherBsciChunkData {
        bsci::GeometryGroup::GeoId ancientDebrisGeoId = {0};
        bool                       freshed            = true;
    };

public:
    enum class ShowType : uint {
        AncientDebris = 1 << 0,
    };

private:
    uint showType                   = 0;
    int  tickCounter                = 1;
    int  runtimeRemoveTickCounter   = 1;
    int  cacheDataRemoveTickCounter = 1;

    std::unordered_map<ChunkPos, NetherBsciChunkData> netherBsciChunkData;

    std::mutex                   netherDecorationThreadIdsLock;
    std::vector<std::thread::id> netherDecorationThreadIds;

    std::mutex                               netherDataMapLock;
    std::unordered_map<ChunkPos, NetherData> netherDataMap;

    struct DuplicatableHook1;
    struct DuplicatableHook2;

private:
    void removeData();
    void draw();
    void removeBsciData(ShowType);
    void bsciDataRuntimeRemove();

public:
    void tick();

public:
    void setShowType(ShowType, bool);
    bool getShowType(ShowType);

public:
    static DuplicatableManager& getInstance() {
        static DuplicatableManager instance;
        return instance;
    }
    static void hook(bool);
};
} // namespace coral_fans::functions::locate