#include "Calculate.h"
#include "coral_fans/base/Utils.h"
#include "ll/api/i18n/I18n.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/BlockTickingQueue.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/dimension/Dimension.h"
#include <utility>


namespace coral_fans::functions {
void calculatePt(Player* player) {
    using ll::i18n_literals::operator""_tr;
    ChunkPos     chunkPos = ChunkPos(player->getFeetBlockPos());
    BlockSource& region   = player->getDimensionBlockSource();
    auto         chunk    = player->getDimension().mBlockSource->get()->getChunk(chunkPos);
    if (chunk && chunk->mLoadState.get() == ChunkState::Loaded) {
        BlockTickingQueue&                        pt            = *chunk->mTickQueue;
        std::vector<BlockTickingQueue::BlockTick> nextTickQueue = pt.mNextTickQueue->mC;
        BlockTickingQueue::TickDataSet            copiedQueue;
        copiedQueue.mC = std::move(nextTickQueue);
        if (!copiedQueue.empty()) {
            BlockTickingQueue::TickDataSet activeQueue;
            utils::segmentAndSendToPlayer(
                "command.calculate.success.pt.title"_tr(
                    region.getLevel().getCurrentTick().tickID - 1,
                    chunkPos.toString(),
                    copiedQueue.size()
                ),
                player
            );
            std::map<std::pair<unsigned long long, std::string>, int> cal;
            for (; !copiedQueue.empty();) {
                auto& blockTick = copiedQueue.top();
                if (blockTick.mIsRemoved) {
                    std::pair<unsigned long long, std::string> tem = {
                        blockTick.mData.tick.tickID,
                        blockTick.mData.mBlock->getTypeName()
                    };
                    auto it = cal.find(tem);
                    if (it != cal.end()) {
                        it->second++;
                    } else {
                        cal.emplace(tem, 1);
                    }
                } else nextTickQueue.emplace_back(blockTick);
                (void)copiedQueue.pop();
            }
            for (auto it = cal.begin(); it != cal.end(); ++it) {
                utils::segmentAndSendToPlayer(
                    "command.calculate.success.pt.remove"_tr(it->first.second, it->first.first, it->second),
                    player
                );
            }

            copiedQueue.mC = std::move(nextTickQueue);
            std::map<std::pair<unsigned long long, std::string>, int> cal2;
            for (; !copiedQueue.empty();) {
                auto&                                      blockTick = copiedQueue.top();
                std::pair<unsigned long long, std::string> tem       = {
                    blockTick.mData.tick.tickID,
                    blockTick.mData.mBlock->getTypeName()
                };
                auto it = cal2.find(tem);
                if (it != cal2.end()) {
                    it->second++;
                } else cal2.emplace(tem, 1);
                (void)copiedQueue.pop();
            }
            for (auto it = cal2.begin(); it != cal2.end(); ++it) {
                utils::segmentAndSendToPlayer(
                    "command.calculate.success.pt.info"_tr(it->first.second, it->first.first, it->second),
                    player
                );
            }
        } else utils::segmentAndSendToPlayer("command.calculate.error.nopt"_tr(), player);
    }
}

void calculatePt2(Player* player) {
    using ll::i18n_literals::operator""_tr;
    ChunkPos     chunkPos = ChunkPos(player->getFeetBlockPos());
    BlockSource& region   = player->getDimensionBlockSource();
    auto         chunk    = player->getDimension().mBlockSource->get()->getChunk(chunkPos);
    if (chunk && chunk->mLoadState.get() == ChunkState::Loaded) {
        BlockTickingQueue&                        pt            = *chunk->mTickQueue;
        std::vector<BlockTickingQueue::BlockTick> nextTickQueue = pt.mNextTickQueue->mC;
        BlockTickingQueue::TickDataSet            copiedQueue;
        copiedQueue.mC = std::move(nextTickQueue);
        if (!copiedQueue.empty()) {
            BlockTickingQueue::TickDataSet activeQueue;
            utils::segmentAndSendToPlayer(
                "command.calculate.success.pt2.title"_tr(
                    region.getLevel().getCurrentTick().tickID - 1,
                    chunkPos.toString(),
                    copiedQueue.size()
                ),
                player
            );
            std::map<std::pair<BlockPos, std::string>, int> cal;
            while (!copiedQueue.empty()) {
                auto& blockTick = copiedQueue.top();
                if (blockTick.mIsRemoved) {
                    std::pair<BlockPos, std::string> tem =
                        std::make_pair(blockTick.mData.pos, blockTick.mData.mBlock->getTypeName());
                    auto it = cal.find(tem);
                    if (it != cal.end()) {
                        it->second++;
                    } else {
                        cal.emplace(tem, 1);
                    }
                } else nextTickQueue.emplace_back(blockTick);
                (void)copiedQueue.pop();
            }
            for (auto it = cal.begin(); it != cal.end(); ++it) {
                utils::segmentAndSendToPlayer(
                    "command.calculate.success.pt2.remove"_tr(it->first.second, it->first.first.toString(), it->second),
                    player
                );
            }

            copiedQueue.mC = std::move(nextTickQueue);
            std::map<std::pair<BlockPos, std::string>, int> cal2;
            for (; !copiedQueue.empty();) {
                auto&                            blockTick = copiedQueue.top();
                std::pair<BlockPos, std::string> tem =
                    std::make_pair(blockTick.mData.pos, blockTick.mData.mBlock->getTypeName());
                auto it = cal2.find(tem);
                if (it != cal2.end()) {
                    it->second++;
                } else cal2.emplace(tem, 1);
                (void)copiedQueue.pop();
            }
            for (auto it = cal2.begin(); it != cal2.end(); ++it) {
                utils::segmentAndSendToPlayer(
                    "command.calculate.success.pt2.info"_tr(it->first.second, it->first.first, it->second),
                    player
                );
            }
        } else utils::segmentAndSendToPlayer("command.calculate.error.nopt"_tr(), player);
    }
}
} // namespace coral_fans::functions