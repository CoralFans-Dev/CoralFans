#pragma once

#include "mc/world/level/ChunkPos.h"

#include <deque>
#include <map>
#include <string>
#include <vector>

namespace coral_fans::functions {

// from trapdoor-ll

struct MSPTInfo {
    std::deque<long long>                         values;
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

    [[nodiscard]] inline long long sum() const { return signalUpdate + pendingAdd + pendingRemove + pendingUpdate; }
};

// normal profile

struct EntityInfo {
    long long time;
    int       count;
};

struct Profiler {
    enum class Type : int { normal, entity, chunk, pt };
    Profiler::Type                                        type         = Type::normal;
    bool                                                  profiling    = false;
    long long                                             totalRound   = 100;
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
    void        reset(Profiler::Type type);
    void        start(long long round, Profiler::Type type = Type::normal);
    void        stop();
};

} // namespace coral_fans::functions