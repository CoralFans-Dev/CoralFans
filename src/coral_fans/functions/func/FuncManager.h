#pragma once
#include "mc/world/level/BlockSource.h"
#include "mc/world/phys/HitResult.h"
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace coral_fans::functions {

// void registerAutoItemListener();
void autoItemHook(bool);
void autoTotemHook(bool);
void hookAutoTool(bool);
void registerContainerReader();
void forceOpenHook(bool);
void forcePlaceHook(uint64);
void safeExplodeHook(bool);
void hookFunctionsHopperCounter(bool);
void fastDropHook(bool);
void noPickUpHook(bool);
void portalDisabledHook(bool);

class FuncDropNoCostManager {
public:
    int  _slot, _count;
    bool mutex = false, mutex2 = true;

    static FuncDropNoCostManager& getInstance() {
        static FuncDropNoCostManager instance;
        return instance;
    }
    static void droppernocostHook(bool);
};

class HopperCounterChannel {
private:
    int                                              channel;
    std::map<std::string, unsigned long long>        counterList;
    std::map<unsigned long long, unsigned long long> gtCounter;
    unsigned long long                               gameTick = 0;

public:
    explicit HopperCounterChannel(int ch) : channel(ch), gameTick(0) {}
    void        reset();
    std::string info();
    void        add(std::string, unsigned long long);
    inline void tick() { ++this->gameTick; }
};

class HopperCounterManager {
private:
    std::vector<HopperCounterChannel> channels;

public:
    const static std::unordered_map<std::string, int> HOPPER_COUNTER_MAP;
    BlockSource*                                      region;
    BlockPos                                          pos;
    bool                                              mutex = false;

    HopperCounterManager() {
        for (int i = 0; i < 16; ++i) this->channels.emplace_back(i);
    }
    inline HopperCounterChannel& getChannel(int ch) { return this->channels[ch]; }
    inline void                  clearAllData() {
        for (auto& ch : this->channels) {
            ch.reset();
        }
    }
    void       tick();
    static int getViewChannel(BlockSource&, HitResult);

    static HopperCounterManager& getInstance() {
        static HopperCounterManager instance;
        return instance;
    }
};

class MaxPtManager {
public:
    int                  maxpt;
    static MaxPtManager& getInstance() {
        static MaxPtManager instance;
        return instance;
    }
};
} // namespace coral_fans::functions