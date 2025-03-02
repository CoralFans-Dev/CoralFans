#include "coral_fans/functions/data/Data.h"
#include "coral_fans/base/Macros.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/ParamKind.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/server/commands/CommandVersion.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/phys/HitResult.h"


#include <string>

namespace coral_fans::commands {

void registerDataCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    // reg cmd
    auto& dataCommand = ll::command::CommandRegistrar::getInstance()
                            .getOrCreateCommand("data", "command.data.description"_tr(), permission);

    // block [blockPos: x y z]
    dataCommand.runtimeOverload()
        .text("block")
        .optional("blockPos", ll::command::ParamKind::BlockPos)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            BlockPos blockPos;
            if (self["blockPos"].has_value())
                blockPos = self["blockPos"].get<ll::command::ParamKind::BlockPos>().getBlockPos(
                    CommandVersion::CurrentVersion(),
                    origin,
                    {0, 0, 0}
                );

            else {
                const auto& hitrst = player->traceRay(5.25f, false, true);
                if (!hitrst) return output.error("command.data.error"_tr());
                blockPos = hitrst.mBlock;
            }
            output.success(functions::getBlockData(player->getDimensionBlockSource(), blockPos));
        });

    // block nbt [path]
    dataCommand.runtimeOverload()
        .text("block")
        .text("nbt")
        .optional("path", ll::command::ParamKind::String)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            const auto& hitrst = player->traceRay(5.25f, false, true);
            if (!hitrst) return output.error("command.data.error"_tr());
            BlockPos    blockPos = hitrst.mBlock;
            std::string path;
            if (self["path"].has_value()) path = self["path"].get<ll::command::ParamKind::String>();
            auto rst = functions::getBlockNbt(0, player->getDimensionBlockSource(), blockPos, path);
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        });

    // blockentity nbt [path]
    dataCommand.runtimeOverload()
        .text("blockentity")
        .text("nbt")
        .optional("path", ll::command::ParamKind::String)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            const auto& hitrst = player->traceRay(5.25f, false, true);
            if (!hitrst) return output.error("command.data.error"_tr());
            BlockPos    blockPos = hitrst.mBlock;
            std::string path;
            if (self["path"].has_value()) path = self["path"].get<ll::command::ParamKind::String>();
            auto rst = functions::getBlockNbt(1, player->getDimensionBlockSource(), blockPos, path);
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        });

    // block <blockPos: x y z> nbt [path]
    dataCommand.runtimeOverload()
        .text("block")
        .required("blockPos", ll::command::ParamKind::BlockPos)
        .text("nbt")
        .optional("path", ll::command::ParamKind::String)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            BlockPos blockPos = self["blockPos"].get<ll::command::ParamKind::BlockPos>().getBlockPos(
                CommandVersion::CurrentVersion(),
                origin,
                {0, 0, 0}
            );
            std::string path;
            if (self["path"].has_value()) path = self["path"].get<ll::command::ParamKind::String>();
            auto rst = functions::getBlockNbt(0, player->getDimensionBlockSource(), blockPos, path);
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        });

    // blockentity <blockPos: x y z> nbt [path]
    dataCommand.runtimeOverload()
        .text("blockentity")
        .required("blockPos", ll::command::ParamKind::BlockPos)
        .text("nbt")
        .optional("path", ll::command::ParamKind::String)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            BlockPos blockPos = self["blockPos"].get<ll::command::ParamKind::BlockPos>().getBlockPos(
                CommandVersion::CurrentVersion(),
                origin,
                {0, 0, 0}
            );
            std::string path;
            if (self["path"].has_value()) path = self["path"].get<ll::command::ParamKind::String>();
            auto rst = functions::getBlockNbt(1, player->getDimensionBlockSource(), blockPos, path);
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        });

    // blockentity highlight [radius: int] [time: int]
    dataCommand.runtimeOverload()
        .text("blockentity")
        .text("highlight")
        .optional("radius", ll::command::ParamKind::Int)
        .optional("time", ll::command::ParamKind::Int)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            int radius = 16;
            if (self["radius"].has_value()) radius = self["radius"].get<ll::command::ParamKind::Int>();
            int time = 20;
            if (self["time"].has_value()) time = self["time"].get<ll::command::ParamKind::Int>();
            functions::highlightBlockEntity(player, radius, time);
        });

    // entity
    dataCommand.runtimeOverload().text("entity").execute(
        [](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const&) {
            COMMAND_CHECK_PLAYER
            const auto& hitrst = player->traceRay(5.25f, true, false);
            if (!hitrst) return output.error("command.data.error"_tr());
            auto rst = functions::getEntityData(hitrst.getEntity());
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        }
    );

    // entity nbt [path]
    dataCommand.runtimeOverload()
        .text("entity")
        .text("nbt")
        .optional("path", ll::command::ParamKind::String)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            std::string path;
            if (self["path"].has_value()) path = self["path"].get<ll::command::ParamKind::String>();
            const auto& hitrst = player->traceRay(5.25f, true, false);
            if (!hitrst) return output.error("command.data.error"_tr());
            auto rst = functions::getEntityNbt(hitrst.getEntity(), path);
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        });

    // redstone <signal|info|chunk|conn> [blockPos: x y z]
    ll::command::CommandRegistrar::getInstance().tryRegisterRuntimeEnum(
        "redstoneType",
        {
            {"chunk",  0},
            {"signal", 1},
            {"info",   2},
            {"conn",   3},
    }
    );
    dataCommand.runtimeOverload()
        .text("redstone")
        .required("redstoneType", ll::command::ParamKind::Enum, "redstoneType")
        .optional("blockPos", ll::command::ParamKind::BlockPos)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            BlockPos blockPos;
            if (self["blockPos"].has_value())
                blockPos = self["blockPos"].get<ll::command::ParamKind::BlockPos>().getBlockPos(
                    CommandVersion::CurrentVersion(),
                    origin,
                    {0, 0, 0}
                );
            else {
                const auto& hitrst = player->traceRay(5.25f, false, true);
                if (!hitrst) return output.error("command.data.error"_tr());
                blockPos = hitrst.mBlock;
            }
            auto rst = functions::showRedstoneComponentsInfo(
                player->getDimension(),
                blockPos,
                self["redstoneType"].get<ll::command::ParamKind::Enum>().index
            );
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        });

    // item nbt [path]
    dataCommand.runtimeOverload()
        .text("item")
        .text("nbt")
        .optional("path", ll::command::ParamKind::String)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            std::string path;
            if (self["path"].has_value()) path = self["path"].get<ll::command::ParamKind::String>();
            auto rst = functions::getItemNbt(player->getSelectedItem(), path);
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        });

    // player <player: Player> uuid
    dataCommand.runtimeOverload()
        .text("player")
        .required("player", ll::command::ParamKind::Actor)
        .text("uuid")
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            const auto actors = self["player"].get<ll::command::ParamKind::Actor>().results(origin);
            for (const auto& i : actors) {
                if (i && i->isType(ActorType::Player)) {
                    Player* pl = static_cast<Player*>(i);
                    output.success("{} = {}", pl->getRealName(), pl->getUuid().asString());
                }
            }
        });
}

} // namespace coral_fans::commands