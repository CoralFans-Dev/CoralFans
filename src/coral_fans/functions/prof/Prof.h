#pragma once

#include "ll/api/base/StdInt.h"
#include "mc/world/level/ChunkPos.h"

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace coral_fans::functions {

// from trapdoor-ll

struct MSPTInfo {
    unsigned long long                            values[20] = {0};
    uint8_t                                       index      = 0;
    [[nodiscard]] long long                       mean() const;
    void                                          push(long long value);
    [[nodiscard]] long long                       min() const;
    [[nodiscard]] long long                       max() const;
    [[nodiscard]] std::pair<long long, long long> pairs() const;
};

struct ChunkProfileInfo {
    std::array<std::map<ChunkPos, std::vector<long long>>, 3> chunk_counter{};
    long long                                                 blockEntitiesTickTime = 0;
    long long                                                 randomTickTime        = 0;
    long long                                                 pendingTickTime       = 0;
    long long                                                 totalTickTime         = 0;

    [[nodiscard]] inline long long getChunkNumber() const {
        long long num = 0;
        for (auto& m : chunk_counter) num += m.size();
        return num;
    }

    inline void reset() {
        for (int i = 0; i < 3; i++) this->chunk_counter[i].clear();
        blockEntitiesTickTime = 0;
        randomTickTime        = 0;
        pendingTickTime       = 0;
        totalTickTime         = 0;
    }
};

/*
Level::tick
 - Redstone
    - Dimension::tickRedstone(shouldUpdate,cacheValue,evaluate)
    - pendingUpdate
    - pendingRemove
    - pendingAdd
 - Dimension::tick(chunk load/village)
 - entitySystem
 - Level::tick
    - LevelChunk::Tick
        - blockEntities
        - randomChunk
        - Actor::tick(non global)
*/

struct RedstoneProfileInfo {
    long long   signalUpdate  = 0; // Dimension::tickRedstone
    long long   pendingAdd    = 0; // PendingAdd
    long long   pendingUpdate = 0; // PendingUpdate
    long long   pendingRemove = 0; // pendingRemove
    inline void reset() {
        signalUpdate  = 0;
        pendingAdd    = 0;
        pendingUpdate = 0;
        pendingRemove = 0;
    }

    [[nodiscard]] inline long long sum() const { return signalUpdate + pendingAdd + pendingUpdate; }
};

// normal profile

struct EntityInfo {
    long long time;
    int       count;
};

struct Profiler {
    enum Type : uint64 { normal, entity, chunk, pt };
    static std::vector<std::pair<std::string, uint64>>    TypeVec;
    uint64                                                type         = Type::normal;
    bool                                                  profiling    = false;
    long long                                             totalRound   = 60;
    long long                                             currentRound = 0;
    ChunkProfileInfo                                      chunkInfo{};
    std::array<std::map<std::string, EntityInfo>, 3>      actorInfo{};
    RedstoneProfileInfo                                   redstoneInfo{};
    std::vector<long long>                                gameSessionTicksBuffer;
    long long                                             gameSessionTickTime      = 0; // mspt
    long long                                             coralfansSessionTickTime = 0; // coralfans mspt
    long long                                             dimensionTickTime        = 0; // chunk (un)load & village
    long long                                             entitySystemTickTime     = 0;
    std::array<std::map<ChunkPos, unsigned long long>, 3> ptCounter{};

    void        print() const;
    std::string printChunks() const;
    std::string printPendingTicks() const;
    std::string printBasics() const;
    std::string printActor() const;
    void        reset(uint64);
    void        start(long long, uint64 = Type::normal);
    void        stop();
};

void hookTick(bool);
} // namespace coral_fans::functions