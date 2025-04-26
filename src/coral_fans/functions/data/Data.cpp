#include "coral_fans/base/Utils.h"
#include "ll/api/i18n/I18n.h"
#include "mc/common/ActorUniqueID.h"
#include "mc/deps/core/math/Color.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/nbt/CompoundTagVariant.h"
#include "mc/nbt/Tag.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/item/SaveContextFactory.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/actor/BlockActor.h"
#include "mc/world/level/block/actor/BlockActorType.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/phys/AABB.h"
#include "mc/world/redstone/circuit/ChunkCircuitComponentList.h"
#include "mc/world/redstone/circuit/CircuitSceneGraph.h"
#include "mc/world/redstone/circuit/CircuitSystem.h"
#include "mc/world/redstone/circuit/components/BaseCircuitComponent.h"
#include "mc/world/redstone/circuit/components/CircuitComponentList.h"


#include <string>

namespace coral_fans::functions {

std::string getBlockData(BlockSource& blockSource, BlockPos blockPos) {
    using ll::i18n_literals::operator""_tr;
    const auto& block = blockSource.getBlock(blockPos);
    return "translate.data.info.block"_tr(
        blockPos.toString(),
        block.buildDescriptionName(),
        block.getTypeName(),
        block.getBlockItemId(),
        block.mLegacyBlock.get()->getVariant(block),
        block.mLegacyBlock.get()->canInstatick(),
        block.mLegacyBlock.get()->mBlockEntityType != BlockActorType::Undefined,
        block.mLegacyBlock.get()->hasComparatorSignal()
            ? std::to_string(block.mLegacyBlock.get()->getComparatorSignal(blockSource, blockPos, block, 0))
            : "-"
    );
}

std::pair<std::string, bool> getBlockNbt(uint64 type, BlockSource& blockSource, BlockPos blockPos, std::string path) {
    using ll::i18n_literals::operator""_tr;
    std::unique_ptr<CompoundTag> tag   = std::make_unique<CompoundTag>();
    const auto&                  block = blockSource.getBlock(blockPos);
    if (type == 0) *tag = block.mSerializationId;
    if (type == 1) {
        auto blockEntity = blockSource.getBlockEntity(blockPos);
        if (blockEntity) blockEntity->save(*tag, *SaveContextFactory::createCloneSaveContext());
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
            actor->getOrCreateUniqueID().rawID,
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
    auto&              circuitSys = dimension.mCircuitSystem;
    CircuitSceneGraph& graph      = circuitSys->mSceneGraph;
    if (type == 0) {
        // chunk
        auto     chunkPos = utils::blockPosToChunkPos(pos);
        BlockPos chunkBlockPos{chunkPos.x * 16, 0, chunkPos.z * 16};
        auto     iter = graph.mActiveComponentsPerChunk.find(chunkBlockPos);
        if (iter != graph.mActiveComponentsPerChunk.end())
            for (auto& c : std::vector<::ChunkCircuitComponentList::Item>(iter->second.mComponents))
                utils::shortHighligntBlock((DimensionType)dimension.mId, c.mPos, mce::Color::CYAN(), 80);
        return {"", true};
    }
    using ll::i18n_literals::operator""_tr;
    auto* component = graph.getComponent(pos, CircuitComponentType::CapacitorComponent);
    if (!component) component = graph.getComponent(pos, CircuitComponentType::ConsumerComponent);
    if (!component) component = graph.getComponent(pos, CircuitComponentType::TransporterComponent);
    if (!component) component = graph.getComponent(pos, CircuitComponentType::PoweredBlockComponent);
    if (!component) component = graph.getComponent(pos, CircuitComponentType::PoweredBlockComponent);
    if (!component) component = graph.getComponent(pos, CircuitComponentType::ProducerComponent);
    if (!component) component = graph.getComponent(pos, CircuitComponentType::BaseRailTransporter);
    /*
in 1.21.50
enum class CircuitComponentType : uint64 {
Undefined              = 1,
GroupMask              = 4294901760,    基础原件，铁轨，消费者，充能方块，生产者，红石粉，电容器的掩码，但是失效了
BaseCircuitComponent   = 2147483648,    貌似现在失效了
BaseRailTransporter    = 65536,         铁轨
ConsumerComponent      = 131072,        消费者
PoweredBlockComponent  = 262144,        充能方块
ProducerComponent      = 524288,        生产者
TransporterComponent   = 1048576,       红石粉
CapacitorComponent     = 2097152,       电容器，包括火把，比较器，中继器，观察者
PistonConsumer         = 131073,        活塞，已包含在生产者中
ComparatorCapacitor    = 2097153,       比较器，已包含在电容器中
PulseCapacitor         = 2097154,       脉冲电容器，仅发现观察者
RedstoneTorchCapacitor = 2097155,       红石火把
RepeaterCapacitor      = 2097156,       中继器，已包含在电容器中
};
*/
    if (!component) return {"translate.data.error.nocircuitcomponent"_tr(), false};
    if (type == 1) {
        // signal
        std::string retstr = "translate.data.info.redstone.signal.title"_tr(component->getStrength());
        for (auto& source : component->mSources->mComponents)
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
                component->mDirection,
                // "magic_enum::enum_name(component->mDirection)",
                block.mLegacyBlock.get()->hasComparatorSignal()
                    ? std::to_string(block.mLegacyBlock.get()->getComparatorSignal(blockSource, pos, block, 0))
                    : "-"
            ),
            true
        };
    }
    if (type == 3) {
        // conn
        utils::shortHighligntBlock((DimensionType)dimension.mId, pos, mce::Color::GREEN(), 80); // highlight self
        auto it = graph.mPowerAssociationMap.find(pos);
        if (it != graph.mPowerAssociationMap.end())
            for (auto& c : it->second.mComponents)
                utils::shortHighligntBlock(
                    (DimensionType)dimension.mId,
                    c.mPos,
                    mce::Color::YELLOW(),
                    80
                ); // highlight children
        for (auto& source : component->mSources->mComponents)
            utils::shortHighligntBlock(
                (DimensionType)dimension.mId,
                source.mPos,
                mce::Color::RED(),
                80
            ); // highlight parents
        return {"", true};
    }
    return {"", false};
}

std::pair<std::string, bool> getItemNbt(ItemStack const& item, std::string path) {
    using ll::i18n_literals::operator""_tr;
    auto tag = item.save(*SaveContextFactory::createCloneSaveContext());
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
            mce::Color::BLACK(),
            mce::Color::BLUE(),
            mce::Color::CYAN(),
            mce::Color::GREEN(),
            mce::Color::GREY(),
            mce::Color::MINECOIN_GOLD(),
            mce::Color::ORANGE(),
            mce::Color::PINK(),
            mce::Color::PURPLE(),
            mce::Color::REBECCA_PURPLE(),
            mce::Color::RED(),
            mce::Color::WHITE(),
            mce::Color::YELLOW()
        };
        for (auto blockActor : player->getDimensionBlockSource().fetchBlockEntities({origin - offset, origin + offset}))
            if (blockActor) utils::shortHighligntBlock(dimid, blockActor->mPosition, colors[colorindex], time);
        colorindex = (colorindex + 1) % 13;
    }
}

} // namespace coral_fans::functions