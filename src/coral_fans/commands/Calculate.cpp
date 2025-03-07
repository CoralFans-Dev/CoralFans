#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Utils.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/RuntimeCommand.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/BlockTickingQueue.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/TickNextTickData.h"
#include "mc/world/level/chunk/ChunkSource.h"
#include "mc/world/level/chunk/ChunkState.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/dimension/Dimension.h"


namespace coral_fans::commands {

void registerCalculateCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    // reg cmd
    auto& calculateCommand = ll::command::CommandRegistrar::getInstance()
                                 .getOrCreateCommand("calculate", "command.calculate.description"_tr(), permission);
    calculateCommand.runtimeOverload().text("pt").execute([](CommandOrigin const& origin,
                                                             CommandOutput&       output,
                                                             ll::command::RuntimeCommand const&) {
        COMMAND_CHECK_PLAYER
        ChunkPos     chunkPos = utils::blockPosToChunkPos(player->getFeetBlockPos());
        BlockSource& region   = player->getDimensionBlockSource();
        auto         chunk    = player->getDimension().mBlockSource->get()->getChunk(chunkPos);
        if (chunk && chunk->mLoadState.get() == ChunkState::Loaded) {
            BlockTickingQueue&                        pt            = *chunk->mTickQueue;
            std::vector<BlockTickingQueue::BlockTick> nextTickQueue = pt.mNextTickQueue->mC;
            BlockTickingQueue::TickDataSet            copiedQueue;
            copiedQueue.mC = std::move(nextTickQueue);
            if (!copiedQueue.empty()) {
                BlockTickingQueue::TickDataSet activeQueue;
                output.success("command.calculate.success.pt.title"_tr(
                    region.getLevel().getCurrentTick().tickID - 1,
                    chunkPos.toString(),
                    copiedQueue.size()
                ));
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
                    output.success(
                        "command.calculate.success.pt.remove"_tr(it->first.second, it->first.first, it->second)
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
                    output.success("command.calculate.success.pt.info"_tr(it->first.second, it->first.first, it->second)
                    );
                }
            } else output.error("command.calculate.error.nopt"_tr());
        }
    });
}
} // namespace coral_fans::commands