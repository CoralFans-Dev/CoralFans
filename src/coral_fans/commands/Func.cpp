#include "coral_fans/base/Mod.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/i18n/I18n.h"
#include "magic_enum.hpp"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include <string>

namespace coral_fans::commands {
void registerFuncCommand(std::string permission) {
    using ll::i18n_literals::operator""_tr;

    // reg cmd
    auto& funcCommand = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "func",
        "command.func.description"_tr(),
        magic_enum::enum_cast<CommandPermissionLevel>(permission).value_or(CommandPermissionLevel::GameDirectors)
    );

    struct FuncIsOpenParam {
        bool isopen;
    };

    // func forceopen <bool>
    funcCommand.overload<FuncIsOpenParam>()
        .text("forceopen")
        .required("isopen")
        .execute([](CommandOrigin const&, CommandOutput& output, FuncIsOpenParam const& param) {
            if (coral_fans::mod().getConfigDb()->set("functions.global.forceopen", param.isopen ? "true" : "false"))
                output.success("command.func.forceopen.success"_tr(param.isopen ? "true" : "false"));
            else output.error("command.func.forceopen.error"_tr());
        });

    // func forceplace normal|entity|all
    struct FuncForcePlaceTypeParam {
        enum ForcePlaceType : int { normal, entity, all } forceplacetype;
    };
    funcCommand.overload<FuncForcePlaceTypeParam>()
        .text("forceplace")
        .required("forceplacetype")
        .execute([](CommandOrigin const&, CommandOutput& output, FuncForcePlaceTypeParam const& param) {
            if (coral_fans::mod().getConfigDb()->set(
                    "functions.global.forceplace",
                    magic_enum::enum_name(param.forceplacetype)
                ))
                output.success("command.func.forceplace.success"_tr(magic_enum::enum_name(param.forceplacetype)));
            else output.error("command.func.forceplace.error"_tr());
        });

    // func noclip <bool>
    funcCommand.overload<FuncIsOpenParam>().text("noclip").required("isopen").execute(
        [](CommandOrigin const&, CommandOutput& output, FuncIsOpenParam const& param) {
            if (coral_fans::mod().getConfigDb()->set("functions.global.noclip", param.isopen ? "true" : "false"))
                output.success("command.func.noclip.success"_tr(param.isopen ? "true" : "false"));
            else output.error("command.func.noclip.error"_tr());
        }
    );

    // func droppernocost <bool>
    funcCommand.overload<FuncIsOpenParam>()
        .text("droppernocost")
        .required("isopen")
        .execute([](CommandOrigin const&, CommandOutput& output, FuncIsOpenParam const& param) {
            if (coral_fans::mod().getConfigDb()->set("functions.global.droppernocost", param.isopen ? "true" : "false"))
                output.success("command.func.droppernocost.success"_tr(param.isopen ? "true" : "false"));
            else output.error("command.func.droppernocost.error"_tr());
        });

    // safeexplode
    funcCommand.overload<FuncIsOpenParam>()
        .text("safeexplode")
        .required("isopen")
        .execute([](CommandOrigin const&, CommandOutput& output, FuncIsOpenParam const& param) {
            if (coral_fans::mod().getConfigDb()->set("functions.global.safeexplode", param.isopen ? "true" : "false"))
                output.success("command.func.safeexplode.success"_tr(param.isopen ? "true" : "false"));
            else output.error("command.func.safeexplode.error"_tr());
        });

    // autotool
    funcCommand.overload<FuncIsOpenParam>()
        .text("autotool")
        .required("isopen")
        .execute([](CommandOrigin const&, CommandOutput& output, FuncIsOpenParam const& param) {
            if (coral_fans::mod().getConfigDb()->set("functions.global.autotool", param.isopen ? "true" : "false"))
                output.success("command.func.autotool.success"_tr(param.isopen ? "true" : "false"));
            else output.error("command.func.autotool.error"_tr());
        });

    // hoppercounter
    funcCommand.overload<FuncIsOpenParam>()
        .text("hoppercounter")
        .required("isopen")
        .execute([](CommandOrigin const&, CommandOutput& output, FuncIsOpenParam const& param) {
            if (coral_fans::mod().getConfigDb()->set("functions.global.hoppercounter", param.isopen ? "true" : "false"))
                output.success("command.func.hoppercounter.success"_tr(param.isopen ? "true" : "false"));
            else output.error("command.func.hoppercounter.error"_tr());
        });

    // maxpt <int>
    struct FuncMaxPtParam {
        int maxpt;
    };
    funcCommand.overload<FuncMaxPtParam>().text("maxpt").required("maxpt").execute(
        [](CommandOrigin const&, CommandOutput& output, FuncMaxPtParam const& param) {
            if (param.maxpt <= 0) output.error("command.func.maxpt.error.nonpositive"_tr());
            if (coral_fans::mod().getConfigDb()->set("functions.global.maxpt", std::to_string(param.maxpt)))
                output.success("command.func.maxpt.success"_tr(param.maxpt));
            else output.error("command.func.maxpt.error.failed"_tr());
        }
    );
}
} // namespace coral_fans::commands