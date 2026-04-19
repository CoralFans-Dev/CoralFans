#include "DuplicatableManager.h"
#include "NetherDuplicatableController.h"
#include "TheEndDuplicatableController.h"
#include "coral_fans/CoralFans.h"


#include "ll/api/service/Bedrock.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/dimension/Dimension.h"


namespace coral_fans::functions::locate {

DuplicatableManager::DuplicatableManager()
: netherController(&NetherDuplicatableController::getInstance()),
  theEndController(&TheEndDuplicatableController::getInstance()) {}

DuplicatableManager::~DuplicatableManager() = default;

DuplicatableManager& DuplicatableManager::getInstance() {
    static DuplicatableManager instance;
    return instance;
}

void DuplicatableManager::removeData() {
    auto level = ll::service::getLevel();
    if (!level) [[unlikely]]
        return;

    if (auto netherDim = level->getDimension(1).lock()) {
        netherController->removeInvalidData(*netherDim->mChunkSource);
    }
    if (auto theEndDim = level->getDimension(2).lock()) {
        theEndController->removeInvalidData(*theEndDim->mChunkSource);
    }
}

void DuplicatableManager::draw() {
    if (!static_cast<uint>(this->showType)) return;

    auto level = ll::service::getLevel();
    if (!level) [[unlikely]]
        return;

    static int radius = std::max(0, CoralFans::getInstance().getConfig().functions.locate.duplicatable.drawRadius);

    level->forEachPlayer([this, radius = radius](Player& player) {
        int   dimId  = player.getDimensionId();
        auto& region = player.getDimensionBlockSource();

        if (dimId == 1) {
            ChunkPos originChunkPos = ChunkPos(player.getFeetBlockPos());
            for (int i = -radius; i <= radius; ++i) {
                int maxJ = radius - abs(i);
                for (int j = -maxJ; j <= maxJ; ++j) {
                    ChunkPos chunkPos = ChunkPos(originChunkPos.x + i, originChunkPos.z + j);
                    netherController->draw(region, chunkPos, this->showType);
                }
            }
        } else if (dimId == 2) {
            ChunkPos originChunkPos = ChunkPos(player.getFeetBlockPos());
            for (int i = -6; i <= 6; ++i) {
                int maxJ = 6 - abs(i);
                for (int j = -maxJ; j <= maxJ; ++j) {
                    ChunkPos chunkPos = ChunkPos(originChunkPos.x + i, originChunkPos.z + j);
                    theEndController->draw(region, chunkPos, this->showType);
                }
            }
        }
        return true;
    });
}

void DuplicatableManager::bsciDataRuntimeRemove() {
    if (!static_cast<int>(this->showType)) return;

    auto level = ll::service::getLevel();
    if (!level) [[unlikely]]
        return;

    if (auto netherDim = level->getDimension(1).lock()) {
        netherController->bsciDataRuntimeRemoveInternal(this->showType);
    }
    if (auto theEndDim = level->getDimension(2).lock()) {
        theEndController->bsciDataRuntimeRemoveInternal(this->showType);
    }
}

void DuplicatableManager::tick() {
    if (!this->tickCounter) {
        this->draw();
        this->bsciDataRuntimeRemove();
        if (!this->cacheDataRemoveTickCounter) {
            this->removeData();
        }
        static int cacheRemoveScale =
            std::max(1, CoralFans::getInstance().getConfig().functions.locate.duplicatable.cacheRemoveScale);
        this->cacheDataRemoveTickCounter = (this->cacheDataRemoveTickCounter + 1) % cacheRemoveScale;
    }
    static int interval = std::max(1, CoralFans::getInstance().getConfig().functions.locate.duplicatable.drawInterval);
    this->tickCounter   = (this->tickCounter + 1) % interval;
}

void DuplicatableManager::setShowType(ShowType _showType, bool show) {
    uint typeValue = static_cast<uint>(_showType);
    if (((this->showType & typeValue) != 0) == show) return;

    if (show) {
        this->showType    |= typeValue;
        this->tickCounter  = 0;
    } else {
        this->showType &= ~typeValue;
        if (netherController->isResponsibleFor(typeValue)) {
            netherController->removeBsciDataByType(typeValue);
        }
        if (theEndController->isResponsibleFor(typeValue)) {
            theEndController->removeBsciDataByType(typeValue);
        }
    }
}

bool DuplicatableManager::getShowType(ShowType _showType) { return this->showType & static_cast<uint>(_showType); }

void DuplicatableManager::clear() {
    this->showType = 0;
    netherController->clearInternal();
    theEndController->clearInternal();
}

void DuplicatableManager::hook(bool enable) {
    NetherDuplicatableController::hook(enable);
    TheEndDuplicatableController::hook(enable);
}

} // namespace coral_fans::functions::locate
