#pragma once

#include "coral_fans/base/Mod.h"

#include "coral_fans/base/MySchedule.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/io/Logger.h"
#include "mc/nbt/ByteTag.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/nbt/CompoundTagVariant.h"
#include "mc/nbt/ListTag.h"
#include "mc/nbt/StringTag.h"
#include "mc/world/Container.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/SaveContextFactory.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/ChunkPos.h"


#include <memory>
#include <optional>
#include <string>

namespace coral_fans::utils {

namespace {

struct PathNode {
    std::string id;
    int         index;
    bool        useIndex;
};

bool parsePath(std::string const& path, std::vector<PathNode>& vec) {
    for (auto key : ll::string_utils::splitByPattern(path, ".")) {
        if (key.ends_with(']')) try {
                vec.emplace_back(PathNode{
                    std::string{key.substr(0, key.find('['))},
                    std::stoi(std::string{key.substr(key.find('[') + 1, key.length() - key.find('[') - 2)}),
                    true
                });
            } catch (...) {
                return false;
            }
        else vec.emplace_back(PathNode{std::string{key}, 0, false});
    }
    return true;
}

} // namespace

std::pair<std::string, bool> getNbtFromTag(CompoundTag const tag, std::string const& path) {
    using ll::i18n_literals::operator""_tr;
    std::vector<PathNode>           nodes;
    std::vector<CompoundTagVariant> tags;
    if (!parsePath(path, nodes)) return {"translate.data.error.cannotparse"_tr(), false};
    try {
        tags.emplace_back(tag[nodes[0].id]);
        if (nodes[0].useIndex) {
            if (tags.back().is_array() && tags.back().get<ListTag>().getCompound(nodes[0].index))
                tags.emplace_back(*(tags.back().get<ListTag>().getCompound(nodes[0].index)));
            else return {"translate.data.error.notanarray"_tr(), false};
        }
        for (unsigned long long i = 1; i < nodes.size(); ++i) {
            tags.emplace_back(tags.back()[nodes[i].id]);
            if (nodes[i].useIndex) {
                if (tags.back().is_array() && tags.back().get<ListTag>().getCompound(nodes[i].index))
                    tags.emplace_back(*(tags.back().get<ListTag>().getCompound(nodes[i].index)));
                else return {"translate.data.error.notanarray"_tr(), false};
            }
        }
        return {tags.back().toSnbt(SnbtFormat::PrettyChatPrint), true};
    } catch (...) {
        return {"translate.data.error.geterror"_tr(), false};
    }
}

ChunkPos blockPosToChunkPos(BlockPos const& blockPos) {
    return ChunkPos{
        (blockPos.x < 0 ? blockPos.x - 15 : blockPos.x) / 16,
        (blockPos.z < 0 ? blockPos.z - 15 : blockPos.z) / 16
    };
}

std::string removeMinecraftPrefix(std::string const& s) { return s.find("minecraft:") == 0 ? s.substr(10) : s; }

void shortHighligntBlock(int dimid, BlockPos const& blockPos, mce::Color const& color, int time) {
    auto& mod = coral_fans::mod();
    auto  s   = mod.getGeometryGroup()->box(dimid, {blockPos, blockPos + BlockPos::ONE()}, color);
    my_schedule::MySchedule::getSchedule().add(time, [&, s]() { mod.getGeometryGroup()->remove(s); });
}

void swapItemInContainer(Player* player, int slot1, int slot2) {
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

std::optional<std::pair<CompoundTag, CompoundTag>>
getItemFromShulkerBox(std::unique_ptr<CompoundTag> tag, ItemStack const& itemStack, bool replace, int minCount) {
    if (tag && (*tag)["Name"].is_string() && (*tag)["Name"].get<StringTag>().ends_with("_shulker_box")
        && (*tag).contains("tag")) {
        auto list = (*tag)["tag"]["Items"].get<ListTag>();
        for (int i = 0; i < list.size(); ++i) {
            auto* item = list.getCompound(i);
            if (!item) continue;
            if ((*item)["Name"].is_string()
                && removeMinecraftPrefix((*item)["Name"].get<StringTag>())
                       == removeMinecraftPrefix(itemStack.getTypeName())
                && (*item)["Count"].is_number() && (*item)["Count"].get<ByteTag>().data > minCount) {
                CompoundTag ret = *item;
                if (replace) {
                    list[i]                            = itemStack.save(*SaveContextFactory::createCloneSaveContext());
                    list[i].get<CompoundTag>()["Slot"] = ret["Slot"];
                } else list.erase(list.begin() + i);
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