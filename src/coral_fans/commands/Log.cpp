#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Utils.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/RuntimeCommand.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/service/Bedrock.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/BlockTickingQueue.h"
#include "mc/world/level/ChunkPos.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/LevelSeed64.h"
#include "mc/world/level/chunk/ChunkSource.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include <string>

namespace coral_fans::commands {

void registerLogCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    // reg cmd
    auto& logCommand = ll::command::CommandRegistrar::getInstance()
                           .getOrCreateCommand("log", "command.log.description"_tr(), permission);

    logCommand.runtimeOverload()
        .text("levelseed")
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const&) {
            auto level = ll::service::getLevel();
            if (!level.has_value()) return output.error("command.log.error.nulllevel"_tr());
            output.success("{}", level->getLevelSeed64().mValue);
        });

    logCommand.runtimeOverload().text("pt").execute(
        [](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const&) {
            COMMAND_CHECK_PLAYER
            ChunkPos     chunkPos = utils::blockPosToChunkPos(player->getFeetBlockPos());
            BlockSource& region   = player->getDimensionBlockSource();
            // from
            // https://github.com/glibcxx/figure_hack/blob/a46576ff8a5ed403eb366c89f4001fe86b5ec850/src/figure_hack/Commands/QueryPendingTick.h#L47
            // Authorized Use
            // Original License: LGPL-3.0
            auto chunk = player->getDimension().getChunkSource().getExistingChunk(chunkPos);
            if (chunk && chunk->isFullyLoaded()) {
                BlockTickingQueue& pt = chunk->getTickQueue();
                auto nextTickQueue    = ll::memory::dAccess<std::vector<BlockTickingQueue::BlockTick>>(&pt, 16);
                BlockTickingQueue::TickDataSet copiedQueue;
                copiedQueue.mC = std::move(nextTickQueue);
                if (!copiedQueue.empty()) {
                    BlockTickingQueue::TickDataSet activeQueue;
                    output.success("command.log.success.pt.title"_tr(
                        region.getLevel().getCurrentTick().t,
                        chunkPos.toString(),
                        copiedQueue.size()
                    ));
                    for (; !copiedQueue.empty();) {
                        auto& blockTick = copiedQueue.top();
                        if (blockTick.mIsRemoved)
                            output.success("command.log.success.pt.remove"_tr(blockTick.mData.mPos.toString()));
                        else nextTickQueue.emplace_back(blockTick);
                        (void)copiedQueue.pop();
                    }
                    copiedQueue.mC = std::move(nextTickQueue);
                    for (; !copiedQueue.empty();) {
                        auto& blockTick = copiedQueue.top();
                        output.success("command.log.success.pt.info"_tr(
                            blockTick.mData.mPos.toString(),
                            blockTick.mData.mTick.t,
                            blockTick.mData.mPriorityOffset,
                            blockTick.mData.mBlock->buildDescriptionName()
                        ));
                        (void)copiedQueue.pop();
                    }
                } else output.error("command.log.error.nopt"_tr());
            }
        }
    );
}

} // namespace coral_fans::commands