#include "coral_fans/Config.h"
#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Mod.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/ParamKind.h"
#include "ll/api/command/runtime/RuntimeCommand.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <utility>

namespace {

inline void regSubCmd(
    ll::command::CommandHandle&                                                              cmd,
    std::string const&                                                                       tag,
    std::span<std::pair<std::string, ll::command::ParamKind::Kind>> const&                   rArgs,
    std::span<std::pair<std::string, ll::command::ParamKind::Kind>> const&                   oArgs,
    std::function<std::pair<std::string, bool>(Player*, ll::command::RuntimeCommand const&)> spFn,
    std::function<std::pair<std::string, bool>(Player*, ll::command::RuntimeCommand const&)> gFn
) {
    using ll::i18n_literals::operator""_tr;
    auto ro1 = std::make_unique<ll::command::RuntimeOverload>(std::move(
        cmd.runtimeOverload().text("p").required("name", ll::command::ParamKind::SoftEnum, "spname").text(tag)
    ));
    auto ro2 = std::make_unique<ll::command::RuntimeOverload>(
        std::move(cmd.runtimeOverload().text("g").required("name", ll::command::ParamKind::SoftEnum, "gname").text(tag))
    );
    for (const auto& i : rArgs) {
        ro1 = std::make_unique<ll::command::RuntimeOverload>(std::move(ro1->required(i.first, i.second)));
        ro2 = std::make_unique<ll::command::RuntimeOverload>(std::move(ro2->required(i.first, i.second)));
    }
    for (const auto& i : oArgs) {
        ro1 = std::make_unique<ll::command::RuntimeOverload>(std::move(ro1->optional(i.first, i.second)));
        ro2 = std::make_unique<ll::command::RuntimeOverload>(std::move(ro2->optional(i.first, i.second)));
    }
    ro1->execute([spFn](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
        COMMAND_CHECK_PLAYER
        COMMAND_SIMPLAYER_CHECKPERMLIST
        auto rst = spFn(player, self);
        if (rst.second) output.success(rst.first);
        else output.error(rst.first);
    });
    ro2->execute([gFn](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
        COMMAND_CHECK_PLAYER
        COMMAND_SIMPLAYER_CHECKPERMLIST
        auto rst = gFn(player, self);
        if (rst.second) output.success(rst.first);
        else output.error(rst.first);
    });
}

} // namespace

namespace coral_fans::commands {

void registerSpCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    // reg cmd
    auto& spCommand = ll::command::CommandRegistrar::getInstance()
                          .getOrCreateCommand("sp", "command.sp.description"_tr(), permission);

    // reg softenum
    ll::command::CommandRegistrar::getInstance().tryRegisterSoftEnum("spname", {});
    ll::command::CommandRegistrar::getInstance().tryRegisterSoftEnum("gname", {});

    // sp c autorespawn <isopen: bool>
    spCommand.runtimeOverload()
        .text("c")
        .text("autorespawn")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            auto& mod = coral_fans::mod();
            if (origin.getPermissionsLevel() < mod.getConfig().simPlayer.adminPermission)
                return output.error("command.sp.error.permissiondenied"_tr());
            mod.getSimPlayerManager().setAutoRespawn(self["isopen"].get<ll::command::ParamKind::Bool>());
            output.success("command.sp.success"_tr());
        });

    // sp c autojoin <isopen: bool>
    spCommand.runtimeOverload()
        .text("c")
        .text("autojoin")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            auto& mod = coral_fans::mod();
            if (origin.getPermissionsLevel() < mod.getConfig().simPlayer.adminPermission)
                return output.error("command.sp.error.permissiondenied"_tr());
            mod.getSimPlayerManager().setAutoJoin(self["isopen"].get<ll::command::ParamKind::Bool>());
            output.success("command.sp.success"_tr());
        });

    // sp list g
    spCommand.runtimeOverload().text("list").text("g").execute(
        [](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const&) {
            output.success(coral_fans::mod().getSimPlayerManager().listGroup());
        }
    );

    // sp list p
    spCommand.runtimeOverload().text("list").text("p").execute(
        [](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const&) {
            output.success(coral_fans::mod().getSimPlayerManager().listSimPlayer());
        }
    );

    // sp g <name: string> create
    spCommand.runtimeOverload()
        .text("g")
        .required("name", ll::command::ParamKind::SoftEnum, "gname")
        .text("create")
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            COMMAND_SIMPLAYER_CHECKPERMLIST
            auto rst =
                mod.getSimPlayerManager().createGroup(player, self["name"].get<ll::command::ParamKind::SoftEnum>());
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        });

    // sp g <name: string> delete
    spCommand.runtimeOverload()
        .text("g")
        .required("name", ll::command::ParamKind::SoftEnum, "gname")
        .text("delete")
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            COMMAND_SIMPLAYER_CHECKPERMLIST
            auto rst = coral_fans::mod().getSimPlayerManager().deleteGroup(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>()
            );
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        });

    // sp g <name: string> add <simplayer: string>
    spCommand.runtimeOverload()
        .text("g")
        .required("name", ll::command::ParamKind::SoftEnum, "gname")
        .text("add")
        .required("simplayer", ll::command::ParamKind::SoftEnum, "spname")
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            COMMAND_SIMPLAYER_CHECKPERMLIST
            auto rst = coral_fans::mod().getSimPlayerManager().addSpToGroup(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>(),
                self["simplayer"].get<ll::command::ParamKind::SoftEnum>()
            );
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        });

    // sp g <name: string> rm <simplayer: string>
    spCommand.runtimeOverload()
        .text("g")
        .required("name", ll::command::ParamKind::SoftEnum, "gname")
        .text("rm")
        .required("simplayer", ll::command::ParamKind::SoftEnum, "spname")
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            COMMAND_SIMPLAYER_CHECKPERMLIST
            auto rst = coral_fans::mod().getSimPlayerManager().rmSpFromGroup(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>(),
                self["simplayer"].get<ll::command::ParamKind::SoftEnum>()
            );
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        });

    // sp g <name: string> addadmin <player: Player>
    spCommand.runtimeOverload()
        .text("g")
        .required("name", ll::command::ParamKind::SoftEnum, "gname")
        .text("addadmin")
        .required("player", ll::command::ParamKind::Player)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            COMMAND_SIMPLAYER_CHECKPERMLIST
            if (self["player"].get<ll::command::ParamKind::Player>().results(origin).size() != 1)
                return output.error("command.sp.error.toomanyobj"_tr());
            auto rst = coral_fans::mod().getSimPlayerManager().addAdminToGroup(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>(),
                self["player"].get<ll::command::ParamKind::Player>().results(origin).data->front()
            );
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        });

    // sp g <name: string> rmadmin <player: Player>
    spCommand.runtimeOverload()
        .text("g")
        .required("name", ll::command::ParamKind::SoftEnum, "gname")
        .text("rmadmin")
        .required("player", ll::command::ParamKind::Player)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            COMMAND_SIMPLAYER_CHECKPERMLIST
            if (self["player"].get<ll::command::ParamKind::Player>().results(origin).size() != 1)
                return output.error("command.sp.error.toomanyobj"_tr());
            auto rst = coral_fans::mod().getSimPlayerManager().rmAdminFromGroup(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>(),
                self["player"].get<ll::command::ParamKind::Player>().results(origin).data->front()
            );
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        });

    // sp p <name: string> spawn
    spCommand.runtimeOverload()
        .text("p")
        .required("name", ll::command::ParamKind::SoftEnum, "spname")
        .text("spawn")
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            COMMAND_SIMPLAYER_CHECKPERMLIST
            auto rst = coral_fans::mod().getSimPlayerManager().spawnSimPlayer(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>(),
                player->getFeetPos(),
                player->getRotation()
            );
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        });

    // sp p <name: string> spawn pos <pos: x y z>
    spCommand.runtimeOverload()
        .text("p")
        .required("name", ll::command::ParamKind::SoftEnum, "spname")
        .text("spawn")
        .text("pos")
        .required("pos", ll::command::ParamKind::Vec3)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            COMMAND_SIMPLAYER_CHECKPERMLIST
            auto rst = coral_fans::mod().getSimPlayerManager().spawnSimPlayer(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>(),
                self["pos"].get<ll::command::ParamKind::Vec3>().getPosition(player->getFeetPos()),
                player->getRotation()
            );
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        });

    // sp p <name: string> spawn pos <pos: x y z> rot <rotx: float> <roty: float>
    spCommand.runtimeOverload()
        .text("p")
        .required("name", ll::command::ParamKind::SoftEnum, "spname")
        .text("spawn")
        .text("pos")
        .required("pos", ll::command::ParamKind::Vec3)
        .text("rot")
        .required("rotx", ll::command::ParamKind::Float)
        .required("roty", ll::command::ParamKind::Float)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            COMMAND_SIMPLAYER_CHECKPERMLIST
            auto rst = coral_fans::mod().getSimPlayerManager().spawnSimPlayer(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>(),
                self["pos"].get<ll::command::ParamKind::Vec3>().getPosition(player->getFeetPos()),
                {self["rotx"].get<ll::command::ParamKind::Float>(), self["roty"].get<ll::command::ParamKind::Float>()}
            );
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        });

    // sp g <name: string> spawn
    spCommand.runtimeOverload()
        .text("g")
        .required("name", ll::command::ParamKind::SoftEnum, "gname")
        .text("spawn")
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            COMMAND_CHECK_PLAYER
            COMMAND_SIMPLAYER_CHECKPERMLIST
            auto rst = coral_fans::mod().getSimPlayerManager().spawnGroup(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>()
            );
            if (rst.second) output.success(rst.first);
            else output.error(rst.first);
        });


    ::regSubCmd(
        spCommand,
        "despawn",
        {},
        {},
        [](Player* player, ll::command::RuntimeCommand const& self) {
            return coral_fans::mod().getSimPlayerManager().despawnSimPlayer(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>(),
                false
            );
        },
        [](Player* player, ll::command::RuntimeCommand const& self) {
            return coral_fans::mod().getSimPlayerManager().despawnGroup(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>()
            );
        }
    );

    ::regSubCmd(
        spCommand,
        "rm",
        {},
        {},
        [](Player* player, ll::command::RuntimeCommand const& self) {
            return coral_fans::mod().getSimPlayerManager().rmSimPlayer(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>(),
                false
            );
        },
        [](Player* player, ll::command::RuntimeCommand const& self) {
            return coral_fans::mod().getSimPlayerManager().rmGroup(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>()
            );
        }
    );

    ::regSubCmd(
        spCommand,
        "respawn",
        {},
        {},
        [](Player* player, ll::command::RuntimeCommand const& self) {
            return coral_fans::mod().getSimPlayerManager().respawnSimPlayer(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>(),
                false
            );
        },
        [](Player* player, ll::command::RuntimeCommand const& self) {
            return coral_fans::mod().getSimPlayerManager().respawnGroup(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>()
            );
        }
    );

    ::regSubCmd(
        spCommand,
        "stop",
        {},
        {},
        [](Player* player, ll::command::RuntimeCommand const& self) {
            return coral_fans::mod().getSimPlayerManager().simPlayerStop(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>(),
                false
            );
        },
        [](Player* player, ll::command::RuntimeCommand const& self) {
            return coral_fans::mod().getSimPlayerManager().groupStop(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>()
            );
        }
    );

    std::array<std::pair<std::string, ll::command::ParamKind::Kind>, 1> enableArg{
        std::make_pair("enable", ll::command::ParamKind::Bool)
    };

    ::regSubCmd(
        spCommand,
        "sneaking",
        enableArg,
        {},
        [](Player* player, ll::command::RuntimeCommand const& self) {
            return coral_fans::mod().getSimPlayerManager().simPlayerSneaking(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>(),
                false,
                self["enable"].get<ll::command::ParamKind::Bool>()
            );
        },
        [](Player* player, ll::command::RuntimeCommand const& self) {
            return coral_fans::mod().getSimPlayerManager().groupSneaking(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>(),
                self["enable"].get<ll::command::ParamKind::Bool>()
            );
        }
    );

    ::regSubCmd(
        spCommand,
        "swimming",
        enableArg,
        {},
        [](Player* player, ll::command::RuntimeCommand const& self) {
            return coral_fans::mod().getSimPlayerManager().simPlayerSwimming(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>(),
                false,
                self["enable"].get<ll::command::ParamKind::Bool>()
            );
        },
        [](Player* player, ll::command::RuntimeCommand const& self) {
            return coral_fans::mod().getSimPlayerManager().groupSwimming(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>(),
                self["enable"].get<ll::command::ParamKind::Bool>()
            );
        }
    );

    std::array<std::pair<std::string, ll::command::ParamKind::Kind>, 2> taskArg{
        std::make_pair("interval", ll::command::ParamKind::Int),
        std::make_pair("times", ll::command::ParamKind::Int)
    };

    ::regSubCmd(
        spCommand,
        "attack",
        {},
        taskArg,
        [](Player* player, ll::command::RuntimeCommand const& self) {
            return coral_fans::mod().getSimPlayerManager().simPlayerAttack(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>(),
                false,
                self["interval"].has_value() ? self["interval"].get<ll::command::ParamKind::Int>() : 20,
                self["times"].has_value() ? self["times"].get<ll::command::ParamKind::Int>() : 1
            );
        },
        [](Player* player, ll::command::RuntimeCommand const& self) {
            return coral_fans::mod().getSimPlayerManager().groupAttack(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>(),
                self["interval"].has_value() ? self["interval"].get<ll::command::ParamKind::Int>() : 20,
                self["times"].has_value() ? self["times"].get<ll::command::ParamKind::Int>() : 1
            );
        }
    );

    std::array<std::pair<std::string, ll::command::ParamKind::Kind>, 1> chatArg{
        std::make_pair("message", ll::command::ParamKind::RawText)
    };

    ::regSubCmd(
        spCommand,
        "chat",
        chatArg,
        taskArg,
        [](Player* player, ll::command::RuntimeCommand const& self) {
            return coral_fans::mod().getSimPlayerManager().simPlayerChat(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>(),
                false,
                self["message"].get<ll::command::ParamKind::RawText>().getText(),
                self["interval"].has_value() ? self["interval"].get<ll::command::ParamKind::Int>() : 20,
                self["times"].has_value() ? self["times"].get<ll::command::ParamKind::Int>() : 1
            );
        },
        [](Player* player, ll::command::RuntimeCommand const& self) {
            return coral_fans::mod().getSimPlayerManager().groupChat(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>(),
                self["message"].get<ll::command::ParamKind::RawText>().getText(),
                self["interval"].has_value() ? self["interval"].get<ll::command::ParamKind::Int>() : 20,
                self["times"].has_value() ? self["times"].get<ll::command::ParamKind::Int>() : 1
            );
        }
    );

    ::regSubCmd(
        spCommand,
        "destroy",
        {},
        taskArg,
        [](Player* player, ll::command::RuntimeCommand const& self) {
            return coral_fans::mod().getSimPlayerManager().simPlayerDestroy(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>(),
                false,
                self["interval"].has_value() ? self["interval"].get<ll::command::ParamKind::Int>() : 20,
                self["times"].has_value() ? self["times"].get<ll::command::ParamKind::Int>() : 1
            );
        },
        [](Player* player, ll::command::RuntimeCommand const& self) {
            return coral_fans::mod().getSimPlayerManager().groupDestroy(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>(),
                self["interval"].has_value() ? self["interval"].get<ll::command::ParamKind::Int>() : 20,
                self["times"].has_value() ? self["times"].get<ll::command::ParamKind::Int>() : 1
            );
        }
    );

    ::regSubCmd(
        spCommand,
        "drop",
        {},
        {},
        [](Player* player, ll::command::RuntimeCommand const& self) {
            return coral_fans::mod().getSimPlayerManager().simPlayerDropSelectedItem(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>(),
                false
            );
        },
        [](Player* player, ll::command::RuntimeCommand const& self) {
            return coral_fans::mod().getSimPlayerManager().groupDropSelectedItem(
                player,
                self["name"].get<ll::command::ParamKind::SoftEnum>()
            );
        }
    );
}

} // namespace coral_fans::commands