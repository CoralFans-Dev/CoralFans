#include "coral_fans/base/Mod.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/ParamKind.h"
#include "ll/api/command/runtime/RuntimeCommand.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include <string>

namespace coral_fans::commands {
void registerFuncCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    // reg cmd
    auto& funcCommand = ll::command::CommandRegistrar::getInstance()
                            .getOrCreateCommand("func", "command.func.description"_tr(), permission);

    // func forceopen <bool>
    funcCommand.runtimeOverload()
        .text("forceopen")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            if (coral_fans::mod().getConfigDb()->set("functions.global.forceopen", isopen ? "true" : "false"))
                output.success("command.func.forceopen.success"_tr(isopen ? "true" : "false"));
            else output.error("command.func.forceopen.error"_tr());
        });

    // func forceplace normal|entity|all
    ll::command::CommandRegistrar::getInstance().tryRegisterRuntimeEnum(
        "forceplaceLevel",
        {
            {"normal", 0},
            {"entity", 1},
            {"all",    2}
    }
    );
    funcCommand.runtimeOverload()
        .text("forceplace")
        .required("level", ll::command::ParamKind::Enum, "forceplaceLevel")
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            const auto val = self["level"].get<ll::command::ParamKind::Enum>();
            if (coral_fans::mod().getConfigDb()->set("functions.global.forceplace", val.name))
                output.success("command.func.forceplace.success"_tr(val.name));
            else output.error("command.func.forceplace.error"_tr());
        });

    // func noclip <bool>
    funcCommand.runtimeOverload()
        .text("noclip")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            if (coral_fans::mod().getConfigDb()->set("functions.global.noclip", isopen ? "true" : "false"))
                output.success("command.func.noclip.success"_tr(isopen ? "true" : "false"));
            else output.error("command.func.noclip.error"_tr());
        });

    // func droppernocost <bool>
    funcCommand.runtimeOverload()
        .text("droppernocost")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            if (coral_fans::mod().getConfigDb()->set("functions.global.droppernocost", isopen ? "true" : "false"))
                output.success("command.func.droppernocost.success"_tr(isopen ? "true" : "false"));
            else output.error("command.func.droppernocost.error"_tr());
        });

    // safeexplode
    funcCommand.runtimeOverload()
        .text("safeexplode")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            if (coral_fans::mod().getConfigDb()->set("functions.global.safeexplode", isopen ? "true" : "false"))
                output.success("command.func.safeexplode.success"_tr(isopen ? "true" : "false"));
            else output.error("command.func.safeexplode.error"_tr());
        });

    // autotool
    funcCommand.runtimeOverload()
        .text("autotool")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            if (coral_fans::mod().getConfigDb()->set("functions.global.autotool", isopen ? "true" : "false"))
                output.success("command.func.autotool.success"_tr(isopen ? "true" : "false"));
            else output.error("command.func.autotool.error"_tr());
        });

    // hoppercounter
    funcCommand.runtimeOverload()
        .text("hoppercounter")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            if (coral_fans::mod().getConfigDb()->set("functions.global.hoppercounter", isopen ? "true" : "false"))
                output.success("command.func.hoppercounter.success"_tr(isopen ? "true" : "false"));
            else output.error("command.func.hoppercounter.error"_tr());
        });

    // maxpt <int>
    funcCommand.runtimeOverload()
        .text("maxpt")
        .required("maxpt", ll::command::ParamKind::Int)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            int maxpt = self["maxpt"].get<ll::command::ParamKind::Int>();
            if (maxpt <= 0) output.error("command.func.maxpt.error.nonpositive"_tr());
            if (coral_fans::mod().getConfigDb()->set("functions.global.maxpt", std::to_string(maxpt)))
                output.success("command.func.maxpt.success"_tr(maxpt));
            else output.error("command.func.maxpt.error.failed"_tr());
        });

    // containerreader
    funcCommand.runtimeOverload()
        .text("containerreader")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            if (coral_fans::mod().getConfigDb()->set("functions.global.containerreader", isopen ? "true" : "false"))
                output.success("command.func.containerreader.success"_tr(isopen ? "true" : "false"));
            else output.error("command.func.containerreader.error"_tr());
        });

    // autototem
    funcCommand.runtimeOverload()
        .text("autototem")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            if (coral_fans::mod().getConfigDb()->set("functions.global.autototem", isopen ? "true" : "false"))
                output.success("command.func.autototem.success"_tr(isopen ? "true" : "false"));
            else output.error("command.func.autototem.error"_tr());
        });

    // autoitem
    funcCommand.runtimeOverload()
        .text("autoitem")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            if (coral_fans::mod().getConfigDb()->set("functions.global.autoitem", isopen ? "true" : "false"))
                output.success("command.func.autoitem.success"_tr(isopen ? "true" : "false"));
            else output.error("command.func.autoitem.error"_tr());
        });

    // fastdrop
    funcCommand.runtimeOverload()
        .text("fastdrop")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            if (coral_fans::mod().getConfigDb()->set("functions.global.fastdrop", isopen ? "true" : "false"))
                output.success("command.func.fastdrop.success"_tr(isopen ? "true" : "false"));
            else output.error("command.func.fastdrop.error"_tr());
        });

    // nopickup
    funcCommand.runtimeOverload()
        .text("nopickup")
        .required("isopen", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            bool isopen = self["isopen"].get<ll::command::ParamKind::Bool>();
            if (coral_fans::mod().getConfigDb()->set("functions.global.nopickup", isopen ? "true" : "false"))
                output.success("command.func.nopickup.success"_tr(isopen ? "true" : "false"));
            else output.error("command.func.nopickup.error"_tr());
        });
}
} // namespace coral_fans::commands