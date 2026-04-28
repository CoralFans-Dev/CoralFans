#pragma once

#include "mc/deps/core/math/IRandom.h"
#include "mc/world/level/block/ResourceDropsContext.h"
#include "mc/world/level/block/actor/PistonBlockActor.h"


namespace coral_fans::functions {
void bedrockDropHook(bool);
void mbDropHook(bool);
void portalSandFarmHook(bool);
void portalSpawnHook(bool);
void restoreAncillaryBrokenHook(bool);
void populationCapHook(bool);
void pistonCollisionHook(bool);


class MaxPtManager {
private:
    MaxPtManager() = default;

public:
    int                                maxpt = 100;
    [[nodiscard]] static MaxPtManager& getInstance() {
        static MaxPtManager instance;
        return instance;
    }
    static void hook(bool);
};


class RestoreAncillaryBrokenHelper {
public:
    bool                        mutex  = false;
    bool                        mutex2 = false;
    const ResourceDropsContext* dropsContext;
    IRandom*                    _randomize;
    PistonBlockActor*           pistonBlockActor;

private:
    RestoreAncillaryBrokenHelper() = default;

public:
    [[nodiscard]] static RestoreAncillaryBrokenHelper& getInstance() {
        static RestoreAncillaryBrokenHelper instance;
        return instance;
    }
};


class PopulationCapManager {
public:
    struct DimensionData {
        std::array<float, 7> surfaceCaps;
        std::array<float, 7> undergroundCaps;

        std::string toBytes() const {
            std::string bytes;
            bytes.reserve(14 * sizeof(float));

            auto push_val = [&](const auto& val) {
                const char* ptr = reinterpret_cast<const char*>(&val);
                bytes.append(ptr, sizeof(val));
            };

            for (float f : surfaceCaps) push_val(f);
            for (float f : undergroundCaps) push_val(f);

            return bytes;
        }

        explicit DimensionData(const std::string& bytes) {
            size_t expected = 14 * sizeof(float);
            if (bytes.size() != expected) {
                throw std::invalid_argument(
                    "Invalid bytes size: expected " + std::to_string(expected) + ", got " + std::to_string(bytes.size())
                );
            }

            const char* ptr      = bytes.data();
            auto        read_val = [&](auto& val) {
                memcpy(&val, ptr, sizeof(val));
                ptr += sizeof(val);
            };

            for (float& f : surfaceCaps) read_val(f);
            for (float& f : undergroundCaps) read_val(f);
        }

        DimensionData() {
            surfaceCaps.fill(0);
            undergroundCaps.fill(0);
        }
    };

public:
    bool                                          enabled   = false;
    int                                           globalMax = 200;
    std::array<std::unique_ptr<DimensionData>, 3> backupCaps;  // key: dimension id, value: mDimension struct
    std::array<std::unique_ptr<DimensionData>, 3> currentCaps; // key: dimension id, value: mDimension struct

public:
    [[nodiscard]] static PopulationCapManager& getInstance() {
        static PopulationCapManager instance;
        return instance;
    }

    void init();

    void setEnabled(bool bl);
    void setGlobalMax(int count);
    bool setDimCap(int dimId, int category, bool isOnSurface, float count);
    bool resetDimCap(int dimId);

    void clear();

private:
    PopulationCapManager() = default;
};
} // namespace coral_fans::functions