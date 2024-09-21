#pragma once

#include "coral_fans/base/Mod.h"

#include "ll/api/chrono/GameChrono.h"
#include "ll/api/schedule/Scheduler.h"

#include "ll/api/schedule/Task.h"
#include "mc/deps/core/mce/Color.h"
#include "mc/nbt/ByteTag.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/nbt/CompoundTagVariant.h"
#include "mc/nbt/ListTag.h"
#include "mc/nbt/StringTag.h"
#include "mc/world/Container.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/ChunkPos.h"

#include <memory>
#include <optional>
#include <string>

namespace coral_fans::utils {


inline BlockPos facingToBlockPos(int facing) {
    switch (facing) {
    case 0:
        return BlockPos{0, -1, 0};
        break;
    case 1:
        return BlockPos{0, +1, 0};
        break;
    case 2:
        return BlockPos{0, 0, -1};
        break;
    case 3:
        return BlockPos{0, 0, +1};
        break;
    case 4:
        return BlockPos{-1, 0, 0};
        break;
    case 5:
        return BlockPos{+1, 0, 0};
        break;
    default:
        return BlockPos{0, 0, 0};
        break;
    }
};

inline ChunkPos blockPosToChunkPos(BlockPos const& blockPos) {
    return ChunkPos{
        (blockPos.x < 0 ? blockPos.x - 15 : blockPos.x) / 16,
        (blockPos.z < 0 ? blockPos.z - 15 : blockPos.z) / 16
    };
}

inline std::string removeMinecraftPrefix(std::string const& s) { return s.find("minecraft:") == 0 ? s.substr(10) : s; }

inline void shortHighligntBlock(int dimid, BlockPos const& blockPos, mce::Color const& color, int time) {
    auto& mod = coral_fans::mod();
    auto  s   = mod.getGeometryGroup()->box(dimid, {blockPos, blockPos + BlockPos::ONE}, color);
    mod.getTickScheduler().add<ll::schedule::DelayTask>(ll::chrono::ticks(time), [&, s] {
        mod.getGeometryGroup()->remove(s);
    });
}

inline void swapItemInContainer(Player* player, int slot1, int slot2) {
    if (player) {
        auto&     container = player->getInventory();
        ItemStack i1, i2;
        i1 = (slot1 == -1) ? player->getOffhandSlot().clone() : container.getItem(slot1).clone();
        i2 = (slot2 == -1) ? player->getOffhandSlot().clone() : container.getItem(slot2).clone();
        if (slot1 == -1) player->setOffhandSlot(i2);
        else container.setItem(slot1, i2);
        if (slot2 == -1) player->setOffhandSlot(i1);
        else container.setItem(slot2, i1);
    }
}

inline std::optional<std::pair<CompoundTag, CompoundTag>>
getItemFromShulkerBox(std::unique_ptr<CompoundTag> tag, ItemStack& itemStack, bool replace = false, int minCount = 0) {
    if (tag && (*tag)["Name"].is_string() && (*tag)["Name"].get<StringTag>().ends_with("_shulker_box")
        && (*tag).contains("tag")) {
        auto list = (*tag)["tag"]["Items"].get<ListTag>();
        for (unsigned long long i = 0; i < list.size(); ++i) {
            auto* item = list.getCompound(i);
            if (!item) continue;
            if ((*item)["Name"].is_string()
                && removeMinecraftPrefix((*item)["Name"].get<StringTag>())
                       == removeMinecraftPrefix(itemStack.getTypeName())
                && (*item)["Count"].is_number() && (*item)["Count"].get<ByteTag>().data > minCount) {
                CompoundTag ret = *item;
                if (replace) {
                    list[i]                            = itemStack.save();
                    list[i].get<CompoundTag>()["Slot"] = ret["Slot"];
                } else list.erase(i);
                (*tag)["tag"]["Items"] = list;
                return {
                    {*tag, ret}
                };
            }
        }
    }
    return std::nullopt;
}

} // namespace coral_fans::utils