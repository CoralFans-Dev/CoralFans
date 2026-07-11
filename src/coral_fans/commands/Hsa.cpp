#include "coral_fans/functions/hsa/Hsa.h"
#include "coral_fans/Config.h"
#include "coral_fans/base/Macros.h"


#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/ParamKind.h"
#include "ll/api/command/runtime/RuntimeCommand.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "mc/network/packet/TextPacket.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/world/level/ChunkPos.h"


namespace coral_fans::commands {
void registerHsaCommand(config::CommandConfigStruct& config) {
    if (!config.enabled) return;
    using ll::i18n_literals::operator""_tr;

    // reg cmd
    auto& hsaCommand = ll::command::CommandRegistrar::getInstance(false)
                           .getOrCreateCommand(config.command, "command.hsa.description"_tr(), config.permission);

    // hsa show [bool]
    hsaCommand.runtimeOverload()
        .text("show")
        .optional("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            using ll::i18n_literals::operator""_tr;
            auto& hsaManager = functions::HsaManager::getInstance();
            if (self["isopen"].has_value()) hsaManager.setHsaShow(self["isopen"].get<ll::command::ParamKind::Bool>());
            else hsaManager.setHsaShow(!hsaManager.getHsaShow());
            output.success("command.hsa.show.output"_tr(hsaManager.getHsaShow() ? "true" : "false"));
        });

    // hsa structure show [bool]
    hsaCommand.runtimeOverload()
        .text("structure")
        .text("show")
        .optional("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            using ll::i18n_literals::operator""_tr;
            auto& hsaManager = functions::HsaManager::getInstance();
            if (self["isopen"].has_value())
                hsaManager.setStructureShow(self["isopen"].get<ll::command::ParamKind::Bool>());
            else hsaManager.setStructureShow(!hsaManager.getStructureShow());
            output.success("command.hsa.structure.show.output"_tr(hsaManager.getStructureShow() ? "true" : "false"));
        });

    // hsa list
    hsaCommand.runtimeOverload().text("list").execute(
        [](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const&) {
            using ll::i18n_literals::operator""_tr;
            COMMAND_CHECK_PLAYER
            auto hsa = functions::HsaManager::getInstance().listChunkHsa(
                player->getDimensionBlockSource(),
                ChunkPos(player->getPosition())
            );
            for (auto& pos : hsa) {
                TextPacket::createRawMessage(std::format("[{}, {}, {}]", pos.x, pos.y, pos.z)).sendTo(*player);
            }
        }
    );

    // hsa structure list
    hsaCommand.runtimeOverload()
        .text("structure")
        .text("list")
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const&) {
            using ll::i18n_literals::operator""_tr;
            COMMAND_CHECK_PLAYER
            auto hsa = functions::HsaManager::getInstance().listChunkStructure(
                player->getDimensionBlockSource(),
                ChunkPos(player->getPosition())
            );
            for (auto& aabb : hsa) {
                TextPacket::createRawMessage(std::format(
                                                 "[{}, {}, {}] - [{}, {}, {}]",
                                                 aabb.min.x,
                                                 aabb.min.y,
                                                 aabb.min.z,
                                                 aabb.max.x,
                                                 aabb.max.y,
                                                 aabb.max.z
                                             ))
                    .sendTo(*player);
            }
        });
}
} // namespace coral_fans::commands