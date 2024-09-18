#include "coral_fans/functions/Data.h"
#include "coral_fans/base/Macros.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/ParamKind.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"

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
                blockPos = self["blockPos"].get<ll::command::ParamKind::BlockPos>().getBlockPos(player->getPosition());
            else blockPos = player->traceRay(5.25f, false, true).mBlockPos;
            output.success(functions::getBlockData(blockPos, player->getDimensionBlockSource().getBlock(blockPos)));
        });

    // block nbt <block|blockentity> [path]
    ll::command::CommandRegistrar::getInstance().tryRegisterEnum(
        "blockNbtType",
        {
            {"block",       0},
            {"blockentity", 1},
        },
        Bedrock::type_id<CommandRegistry, std::pair<std::string,uint64>>(),
        &CommandRegistry::parse<std::pair<std::string,uint64>>
    );
    dataCommand.runtimeOverload()
        .text("block")
        .text("nbt")
        .required("blockNbtType", ll::command::ParamKind::Enum, "blockNbtType")
        /* .optional("path", ll::command::ParamKind::String) */
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            BlockPos    blockPos = player->traceRay(5.25f, false, true).mBlockPos;
            std::string path;
            /* if (self["path"].has_value()) path = self["path"].get<ll::command::ParamKind::String>(); */
            auto rst = functions::getBlockNbt(
                self["blockNbtType"].get<ll::command::ParamKind::Enum>().second,
                player->getDimensionBlockSource(),
                blockPos,
                path
            );
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        });


    // block <blockPos: x y z> <block|blockentity> nbt [path]
    dataCommand.runtimeOverload()
        .text("block")
        .required("blockPos", ll::command::ParamKind::BlockPos)
        .text("nbt")
        .required("blockNbtType", ll::command::ParamKind::Enum, "blockNbtType")
        /* .optional("path", ll::command::ParamKind::String) */
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            BlockPos blockPos =
                self["blockPos"].get<ll::command::ParamKind::BlockPos>().getBlockPos(player->getPosition());
            std::string path;
            /* if (self["path"].has_value()) path = self["path"].get<ll::command::ParamKind::String>(); */
            auto rst = functions::getBlockNbt(
                self["blockNbtType"].get<ll::command::ParamKind::Enum>().second,
                player->getDimensionBlockSource(),
                blockPos,
                path
            );
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        });

    // entity
    dataCommand.runtimeOverload().text("entity").execute(
        [](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const&) {
            COMMAND_CHECK_PLAYER
            auto rst = functions::getEntityData(player->traceRay(5.25f, true, false).getEntity());
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        }
    );

    // entity nbt [path]
    dataCommand.runtimeOverload()
        .text("entity")
        .text("nbt")
        /* .optional("path", ll::command::ParamKind::String) */
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& /* self */) {
            COMMAND_CHECK_PLAYER
            std::string path;
            /* if (self["path"].has_value()) path = self["path"].get<ll::command::ParamKind::String>(); */
            auto rst = functions::getEntityNbt(player->traceRay(5.25f, true, false).getEntity(), path);
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        });

    // redstone <signal|info|chunk|conn> [blockPos: x y z]
    ll::command::CommandRegistrar::getInstance().tryRegisterEnum(
        "redstoneType",
        {
            {"chunk", 0},
            {"signal", 1},
            {"info", 2},
            {"conn", 3},
        },
        Bedrock::type_id<CommandRegistry, std::pair<std::string,uint64>>(),
        &CommandRegistry::parse<std::pair<std::string,uint64>>
    );
    dataCommand.runtimeOverload()
        .text("redstone")
        .required("redstoneType", ll::command::ParamKind::Enum, "redstoneType")
        .optional("blockPos", ll::command::ParamKind::BlockPos)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            BlockPos blockPos;
            if (self["blockPos"].has_value())
                blockPos = self["blockPos"].get<ll::command::ParamKind::BlockPos>().getBlockPos(player->getPosition());
            else blockPos = player->traceRay(5.25f, false, true).mBlockPos;
            auto rst = functions::showRedstoneComponentsInfo(
                player->getDimension(),
                blockPos,
                self["redstoneType"].get<ll::command::ParamKind::Enum>().second
            );
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        });
}

} // namespace coral_fans::commands