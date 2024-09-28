#include "coral_fans/base/Utils.h"
#include "ll/api/i18n/I18n.h"
// #include "ll/api/utils/StringUtils.h"
#include "magic_enum.hpp"
#include "mc/deps/core/mce/Color.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/nbt/CompoundTagVariant.h"
#include "mc/nbt/Tag.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/actor/BlockActor.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/material/Material.h"
#include "mc/world/phys/AABB.h"
#include "mc/world/redstone/circuit/CircuitSystem.h"

#include <string>
// #include <vector>

/*
namespace {

struct PathNode {
    std::string id;
    int         index;
    bool        useIndex;
};

bool parsePath(std::string path, std::vector<PathNode>& vec) {
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
*/

namespace coral_fans::functions {

std::string getBlockData(BlockPos blockPos, Block const& block) {
    using ll::i18n_literals::operator""_tr;
    const auto& m = block.getMaterial();
    return "translate.data.info.block"_tr(
        blockPos.toString(),
        block.buildDescriptionName(),
        block.getTypeName(),
        block.getBlockItemId(),
        block.getRuntimeId(),
        block.getVariant(),
        block.canInstatick(),
        block.hasBlockEntity(),
        block.isSolid(),
        m.getBlocksMotion(),
        m.isTopSolid(false, false),
        m.isSolid(),
        m.isSolidBlocking(),
        m.getTranslucency()
    );
}

std::pair<std::string, bool> getBlockNbt(uint64 type, BlockSource& blockSource, BlockPos blockPos, std::string path) {
    using ll::i18n_literals::operator""_tr;
    std::unique_ptr<CompoundTag> tag   = std::make_unique<CompoundTag>();
    const auto&                  block = blockSource.getBlock(blockPos);
    if (type == 0) *tag = block.getSerializationId();
    if (type == 1) {
        auto blockEntity = blockSource.getBlockEntity(blockPos);
        if (blockEntity) blockEntity->save(*tag);
        else return {"translate.data.error.noblockentity"_tr(), false};
    }
    if (path.empty()) return {tag->toSnbt(SnbtFormat::PrettyChatPrint), true};
    else {
        /*
        // has bug
        // cannot get items from a ListTag
        // e.g. Item[0].Block -> wrong message!
        std::vector<::PathNode> nodes;
        if (!parsePath(path, nodes)) return {"translate.data.error.cannotparse"_tr(), false};
        try {
            CompoundTagVariant tagVariant = (*tag)[nodes[0].id];
            if (nodes[0].useIndex) {
                if (tagVariant.is_array()) tagVariant = tagVariant[nodes[0].index];
                else return {"translate.data.error.notanarray"_tr(), false};
            }
            for (unsigned long long i = 1; i < nodes.size(); ++i) {
                tagVariant = tagVariant[nodes[i].id];
                if (nodes[i].useIndex) {
                    if (tagVariant.is_array()) tagVariant = tagVariant[nodes[i].index];
                    else return {"translate.data.error.notanarray"_tr(), false};
                }
            }
            return {tagVariant.toSnbt(), true};
        } catch (...) {
            return {"translate.data.error.geterror"_tr(), false};
        }
        */
        return {"", true};
    }
}

std::pair<std::string, bool> getEntityData(Actor* actor) {
    using ll::i18n_literals::operator""_tr;
    if (!actor) return {"translate.data.error.nullentity"_tr(), false};
    return {
        "translate.data.info.entity"_tr(
            actor->getNameTag(),
            actor->getTypeName(),
            actor->getOrCreateUniqueID().id,
            actor->getPosition().toString(),
            actor->getPosDelta().toString(),
            actor->getAABB().toString(),
            actor->isSurfaceMob()
        ),
        true
    };
}

std::pair<std::string, bool> getEntityNbt(Actor* actor, std::string path) {
    using ll::i18n_literals::operator""_tr;
    if (!actor) return {"translate.data.error.nullentity"_tr(), false};
    std::unique_ptr<CompoundTag> tag = std::make_unique<CompoundTag>();
    actor->save(*tag);
    if (path.empty()) return {tag->toSnbt(SnbtFormat::PrettyChatPrint), true};
    else return {"", true};
}

std::pair<std::string, bool> showRedstoneComponentsInfo(Dimension& dimension, BlockPos& pos, uint64 type) {
    auto& circuitSys = dimension.getCircuitSystem();
    auto& graph      = circuitSys.mSceneGraph;
    if (type == 0) {
        // chunk
        auto     chunkPos = utils::blockPosToChunkPos(pos);
        BlockPos chunkBlockPos{chunkPos.x * 16, 0, chunkPos.z * 16};
        auto     iter = graph.mActiveComponentsPerChunk.find(chunkBlockPos);
        if (iter != graph.mActiveComponentsPerChunk.end())
            for (auto& c : iter->second.mComponents)
                utils::shortHighligntBlock(dimension.mId, c.mPos, mce::Color::CYAN, 80);
        return {"", true};
    }
    using ll::i18n_literals::operator""_tr;
    auto* component = graph.getBaseComponent(pos);
    if (!component) return {"translate.data.error.nocircuitcomponent"_tr(), false};
    if (type == 1) {
        // signal
        std::string retstr = "translate.data.info.redstone.signal.title"_tr(component->getStrength());
        for (auto& source : component->mSources.mComponents)
            retstr += "translate.data.info.redstone.signal.info"_tr(
                source.mPos.toString(),
                source.mDampening,
                source.mDirectlyPowered ? "true" : "false",
                source.mComponent->getStrength()
            );
        return {retstr, true};
    }
    if (type == 2) {
        // info
        return {
            "translate.data.info.redstone.info"_tr(
                component->getStrength(),
                component->isSecondaryPowered() ? "true" : "false",
                component->canConsumerPower() ? "true" : "false",
                component->canStopPower() ? "true" : "false",
                component->isHalfPulse() ? "true" : "false",
                magic_enum::enum_name(component->mDirection)
            ),
            true
        };
    }
    if (type == 3) {
        // conn
        utils::shortHighligntBlock(dimension.mId, pos, mce::Color::GREEN, 80); // highlight self
        auto it = graph.mPowerAssociationMap.find(pos);
        if (it != graph.mPowerAssociationMap.end())
            for (auto& c : it->second.mComponents)
                utils::shortHighligntBlock(dimension.mId, c.mPos, mce::Color::YELLOW, 80); // highlight children
        for (auto& source : component->mSources.mComponents)
            utils::shortHighligntBlock(dimension.mId, source.mPos, mce::Color::RED, 80); // highlight parents
        return {"", true};
    }
    return {"", false};
}

} // namespace coral_fans::functions