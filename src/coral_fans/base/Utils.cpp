#include "coral_fans/CoralFans.h"
#include "coral_fans/base/MySchedule.h"


#include "ll/api/i18n/I18n.h"
#include "mc/deps/nbt/CompoundTagVariant.h"
#include "mc/deps/nbt/ListTag.h"
#include "mc/world/Container.h"
#include "mc/world/actor/player/Inventory.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/actor/player/PlayerInventory.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/ChunkPos.h"
#include <memory>
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
        if (key.ends_with(')')) try {
                int index = (int)key.find('(');
                vec.emplace_back(
                    PathNode{
                        std::string{key.substr(0, index)},
                        std::stoi(std::string{key.substr(index + 1, key.length() - index - 2)}),
                        true
                    }
                );
            } catch (...) {
                return false;
            }
        else vec.emplace_back(PathNode{std::string{key}, 0, false});
    }
    return true;
}

} // namespace

std::pair<std::string, bool> getNbtFromTag(CompoundTag& tag, std::string const& path) {
    using ll::i18n_literals::operator""_tr;
    std::vector<PathNode> nodes;
    if (!parsePath(path, nodes)) return {"translate.data.error.cannotparse"_tr(), false};
    try {
        auto it = tag.mTags.find(nodes[0].id);
        if (it == tag.mTags.end()) return {"translate.data.error.geterror"_tr(), false};
        CompoundTagVariant& tagVariant = it->second;
        if (nodes[0].useIndex) {
            if (tagVariant.is_array()) {
                auto& list = tagVariant.get<ListTag>();
                if (nodes[0].index >= static_cast<int>(list.size()))
                    return {"translate.data.error.geterror"_tr(), false};
                tagVariant = std::move(list[nodes[0].index]);
            }
        }
        for (unsigned long long i = 1; i < nodes.size(); i++) {
            tagVariant = std::move(tagVariant[nodes[i].id]);
            if (nodes[i].useIndex) {
                if (tagVariant.is_array()) {
                    auto& list = tagVariant.get<ListTag>();
                    if (nodes[i].index >= static_cast<int>(list.size()))
                        return {"translate.data.error.geterror"_tr(), false};
                    tagVariant = std::move(list[nodes[i].index]);
                }
            }
        }
        return {tagVariant.toSnbt(SnbtFormat::PrettyChatPrint), true};
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
    auto& mod = CoralFans::getInstance();
    auto  s   = mod.getGeometryGroup()->box(
        dimid,
        {
            blockPos,
            {blockPos.x + 1, blockPos.y + 1, blockPos.z + 1}
    },
        color
    );
    my_schedule::MySchedule::getSchedule().add(
        [&, s](int&, int&) {
            mod.getGeometryGroup()->remove(s);
            return false;
        },
        time
    );
}

void swapItemInContainer(Player* player, int slot1, int slot2) {
    if (player && slot1 != slot2) {
        auto& inventory = player->mInventory->mInventory;
        if (slot1 == -1) {
            ItemStack tem = player->getOffhandSlot();
            player->setOffhandSlot(inventory->getItem(slot2));
            inventory->setItem(slot2, tem);
        } else if (slot2 == -1) {
            ItemStack tem = player->getOffhandSlot();
            player->setOffhandSlot(inventory->getItem(slot1));
            inventory->setItem(slot1, tem);
        } else {
            ItemStack tem = inventory->getItem(slot1);
            inventory->setItem(slot1, inventory->getItem(slot2));
            inventory->setItem(slot2, tem);
        }
    }
}
} // namespace coral_fans::utils