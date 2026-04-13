#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace coral_fans::functions {

struct mDimension {
    int32_t              mId;
    std::array<float, 7> surfaceCaps;
    std::array<float, 7> undergroundCaps;

    std::string toBytes() const {
        std::string bytes;
        bytes.reserve(sizeof(int32_t) + 14 * sizeof(float));

        auto push_val = [&](const auto& val) {
            const char* ptr = reinterpret_cast<const char*>(&val);
            bytes.append(ptr, sizeof(val));
        };

        push_val(mId);
        for (float f : surfaceCaps) push_val(f);
        for (float f : undergroundCaps) push_val(f);

        return bytes;
    }

    explicit mDimension(const std::string& bytes) {
        size_t expected = sizeof(int32_t) + 14 * sizeof(float);
        if (bytes.size() != expected) {
            throw std::invalid_argument("Invalid bytes size");
        }

        const char* ptr      = bytes.data();
        auto        read_val = [&](auto& val) {
            std::copy_n(ptr, sizeof(val), reinterpret_cast<char*>(&val));
            ptr += sizeof(val);
        };

        read_val(mId);
        for (float& f : surfaceCaps) read_val(f);
        for (float& f : undergroundCaps) read_val(f);
    }

    explicit mDimension(int id) : mId(id) {
        surfaceCaps.fill(0);
        undergroundCaps.fill(0);
    }

    mDimension() : mId(0) {
        surfaceCaps.fill(0);
        undergroundCaps.fill(0);
    }
};

class PopulationCapManager {
public:
    bool                                enabled   = false;
    int                                 globalMax = 200;
    std::unordered_map<int, mDimension> backupCaps;  // key: dimension id, value: mDimension struct
    std::unordered_map<int, mDimension> currentCaps; // key: dimension id, value: mDimension struct

public:
    static PopulationCapManager& getInstance() {
        static PopulationCapManager instance;
        return instance;
    }

    void init();

    void setEnabled(bool bl);
    void setGlobalMax(int count);
    bool setDimCap(int dimId, int category, bool isOnSurface, float count);
    bool resetDimCap(int dimId);

private:
    PopulationCapManager() = default;
};
} // namespace coral_fans::functions
