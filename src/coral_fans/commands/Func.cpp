#include "coral_fans/base/Mod.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "magic_enum.hpp"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"


namespace coral_fans::commands {
void registerFuncCommand() {
    // reg cmd
    auto& funcCommand = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "func",
        "Globally controls functions of CoralFans Mod.",
        CommandPermissionLevel::GameDirectors
    );

    struct FuncIsOpenParam {
        bool isopen;
    };

    // func forceopen <bool>
    funcCommand.overload<FuncIsOpenParam>()
        .text("forceopen")
        .required("isopen")
        .execute([](CommandOrigin const&, CommandOutput& output, FuncIsOpenParam const& param) {
            const bool rst =
                coral_fans::mod().getConfigDb()->set("functions.global.forceopen", param.isopen ? "true" : "false");
            if (rst) output.success("globally set forceopen \"{}\"", param.isopen ? "true" : "false");
            else output.error("failed to globally set forceopen");
        });

    // func forceplace normal|entity|all
    struct FuncForcePlaceTypeParam {
        enum ForcePlaceType : int { normal, entity, all } forceplacetype;
    };
    funcCommand.overload<FuncForcePlaceTypeParam>()
        .text("forceplace")
        .required("forceplacetype")
        .execute([](CommandOrigin const&, CommandOutput& output, FuncForcePlaceTypeParam const& param) {
            const bool rst = coral_fans::mod().getConfigDb()->set(
                "functions.global.forceplace",
                magic_enum::enum_name(param.forceplacetype)
            );
            if (rst) output.success("globally set forceplace \"{}\"", magic_enum::enum_name(param.forceplacetype));
            else output.error("failed to globally set forceplace");
        });

    // func noclip <bool>
    funcCommand.overload<FuncIsOpenParam>().text("noclip").required("isopen").execute(
        [](CommandOrigin const&, CommandOutput& output, FuncIsOpenParam const& param) {
            const bool rst =
                coral_fans::mod().getConfigDb()->set("functions.global.noclip", param.isopen ? "true" : "false");
            if (rst) output.success("globally set noclip \"{}\"", param.isopen ? "true" : "false");
            else output.error("failed to globally set noclip");
        }
    );

    // func droppernocost <bool>
    funcCommand.overload<FuncIsOpenParam>()
        .text("droppernocost")
        .required("isopen")
        .execute([](CommandOrigin const&, CommandOutput& output, FuncIsOpenParam const& param) {
            const bool rst =
                coral_fans::mod().getConfigDb()->set("functions.global.droppernocost", param.isopen ? "true" : "false");
            if (rst) output.success("globally set droppernocost \"{}\"", param.isopen ? "true" : "false");
            else output.error("failed to globally set droppernocost");
        });

    // safeexplode
    funcCommand.overload<FuncIsOpenParam>()
        .text("safeexplode")
        .required("isopen")
        .execute([](CommandOrigin const&, CommandOutput& output, FuncIsOpenParam const& param) {
            const bool rst =
                coral_fans::mod().getConfigDb()->set("functions.global.safeexplode", param.isopen ? "true" : "false");
            if (rst) output.success("globally set safeexplode \"{}\"", param.isopen ? "true" : "false");
            else output.error("failed to globally set safeexplode");
        });
}
} // namespace coral_fans::commands