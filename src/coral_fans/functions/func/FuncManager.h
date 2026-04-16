#pragma once
#include "mc/world/actor/player/Player.h"
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
void forcePlaceHook(uint64);
void safeExplodeHook(bool);
void fastDropHook(bool);
void noPickUpHook(bool);
void portalDisabledHook(bool);

class FuncDropNoCostManager {
private:
    FuncDropNoCostManager() = default;

public:
    int  _slot, _count;
    bool mutex = false, mutex2 = true;

    [[nodiscard]] static FuncDropNoCostManager& getInstance() {
        static FuncDropNoCostManager instance;
        return instance;
    }
    static void droppernocostHook(bool);
};

class HopperCounterManager {
    struct HopperCounterChannel {
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

private:
    std::vector<HopperCounterChannel> channels;
    bool                              enabled = false;

public:
    const static std::unordered_map<std::string, int> HOPPER_COUNTER_MAP;
    BlockSource*                                      region;
    BlockPos                                          pos;
    bool                                              mutex = false;

private:
    HopperCounterManager() {
        for (int i = 0; i < 16; ++i) this->channels.emplace_back(i);
    }

private:
    void hook(bool);

public:
    void setEnabled(bool);

public:
    inline HopperCounterChannel& getChannel(int ch) { return this->channels[ch]; }
    inline void                  clearAllData() {
        for (auto& ch : this->channels) {
            ch.reset();
        }
    }
    void       tick();
    static int getViewChannel(BlockSource&, HitResult);

public:
    [[nodiscard]] static HopperCounterManager& getInstance() {
        static HopperCounterManager instance;
        return instance;
    }
};

class MaxPtManager {
private:
    MaxPtManager() = default;

public:
    int                                maxpt;
    [[nodiscard]] static MaxPtManager& getInstance() {
        static MaxPtManager instance;
        return instance;
    }
};

class ContainerOpenManager {
private:
    ContainerOpenManager() = default;

public:
    Player* player    = nullptr;
    bool    forceOpen = false;

public:
    [[nodiscard]] static ContainerOpenManager& getInstance() {
        static ContainerOpenManager instance;
        return instance;
    }

    static void hook();

public:
    void sendContentToPlayer(Container* container);
};
} // namespace coral_fans::functions