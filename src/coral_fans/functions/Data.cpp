#include "coral_fans/base/Utils.h"
#include "ll/api/i18n/I18n.h"
#include "magic_enum.hpp"
#include "mc/deps/core/mce/Color.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/nbt/CompoundTagVariant.h"
#include "mc/nbt/Tag.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/item/registry/ItemStack.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/actor/BlockActor.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/material/Material.h"
#include "mc/world/phys/AABB.h"
#include "mc/world/redstone/circuit/CircuitSystem.h"

#include <string>

namespace coral_fans::functions {

std::string getBlockData(BlockSource& blockSource, BlockPos blockPos) {
    using ll::i18n_literals::operator""_tr;
    const auto& block = blockSource.getBlock(blockPos);
    const auto& m     = block.getMaterial();
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
        block.hasComparatorSignal() ? std::to_string(block.getComparatorSignal(blockSource, blockPos, 0)) : "-",
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
    else return utils::getNbtFromTag(*tag, path);
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
    else return utils::getNbtFromTag(*tag, path);
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
        auto& blockSource = dimension.getBlockSourceFromMainChunkSource();
        auto& block       = blockSource.getBlock(pos);
        return {
            "translate.data.info.redstone.info"_tr(
                component->getStrength(),
                component->isSecondaryPowered() ? "true" : "false",
                component->canConsumerPower() ? "true" : "false",
                component->canStopPower() ? "true" : "false",
                component->isHalfPulse() ? "true" : "false",
                magic_enum::enum_name(component->mDirection),
                block.hasComparatorSignal() ? std::to_string(block.getComparatorSignal(blockSource, pos, 0)) : "-"
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

std::pair<std::string, bool> getItemNbt(ItemStack const& item, std::string path) {
    using ll::i18n_literals::operator""_tr;
    auto tag = item.save();
    if (path.empty()) {
        if (tag) return {tag->toSnbt(SnbtFormat::PrettyChatPrint), true};
        else return {"translate.data.error.geterror"_tr(), false};
    } else return utils::getNbtFromTag(*tag, path);
}

void highlightBlockEntity(Player* player, int radius, int time) {
    if (player) {
        int               dimid  = player->getDimensionId();
        auto              origin = player->getPosition();
        Vec3              offset{radius, radius, radius};
        static int        colorindex = 0;
        static std::array colors{
            mce::Color::BLACK,
            mce::Color::BLUE,
            mce::Color::CYAN,
            mce::Color::GREEN,
            mce::Color::GREY,
            mce::Color::MINECOIN_GOLD,
            mce::Color::ORANGE,
            mce::Color::PINK,
            mce::Color::PURPLE,
            mce::Color::REBECCA_PURPLE,
            mce::Color::RED,
            mce::Color::WHITE,
            mce::Color::YELLOW
        };
        for (auto blockActor : player->getDimensionBlockSource().fetchBlockEntities({origin - offset, origin + offset}))
            if (blockActor) utils::shortHighligntBlock(dimid, blockActor->getPosition(), colors[colorindex], time);
        colorindex = (colorindex + 1) % 13;
    }
}

} // namespace coral_fans::functions