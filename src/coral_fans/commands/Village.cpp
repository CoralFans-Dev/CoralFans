#include "coral_fans/functions/village/Village.h"
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
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/player/Player.h"

namespace coral_fans::commands {

void registerVillageCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    // reg cmd
    auto& villageCommand = ll::command::CommandRegistrar::getInstance()
                               .getOrCreateCommand("village", "command.village.description"_tr(), permission);

    // village show <bounds|raid|spawn|center|poi|bind> <bool>
    ll::command::CommandRegistrar::getInstance().tryRegisterRuntimeEnum(
        "villageShowType",
        {
            {"bounds", 0},
            {"raid",   1},
            {"spawn",  2},
            {"center", 3},
            {"poi",    4},
            {"bind",   5}
    }
    );
    villageCommand.runtimeOverload()
        .text("show")
        .required("type", ll::command::ParamKind::Enum, "villageShowType")
        .optional("enable", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            auto& villageManager = coral_fans::mod().getVillageManager();
            bool  isopen         = false;
            switch (self["type"].get<ll::command::ParamKind::Enum>().index) {
            case 0:
                if (self["enable"].has_value())
                    villageManager.mShowBounds = self["enable"].get<ll::command::ParamKind::Bool>();
                else villageManager.mShowBounds = !villageManager.mShowBounds;
                isopen = villageManager.mShowBounds;
                break;
            case 1:
                if (self["enable"].has_value())
                    villageManager.mShowRaidBounds = self["enable"].get<ll::command::ParamKind::Bool>();
                else villageManager.mShowRaidBounds = !villageManager.mShowRaidBounds;
                isopen = villageManager.mShowRaidBounds;
                break;
            case 2:
                if (self["enable"].has_value())
                    villageManager.mShowIronSpawn = self["enable"].get<ll::command::ParamKind::Bool>();
                else villageManager.mShowIronSpawn = !villageManager.mShowIronSpawn;
                isopen = villageManager.mShowIronSpawn;
                break;
            case 3:
                if (self["enable"].has_value())
                    villageManager.mShowCenter = self["enable"].get<ll::command::ParamKind::Bool>();
                else villageManager.mShowCenter = !villageManager.mShowCenter;
                isopen = villageManager.mShowCenter;
                break;
            case 4:
                if (self["enable"].has_value())
                    villageManager.mShowPoiQuery = self["enable"].get<ll::command::ParamKind::Bool>();
                else villageManager.mShowPoiQuery = !villageManager.mShowPoiQuery;
                isopen = villageManager.mShowPoiQuery;
                break;
            case 5:
                if (self["enable"].has_value())
                    villageManager.mShowBind = self["enable"].get<ll::command::ParamKind::Bool>();
                else villageManager.mShowBind = !villageManager.mShowBind;
                isopen = villageManager.mShowBind;
                break;
            }
            output.success("command.village.show.output"_tr(
                self["type"].get<ll::command::ParamKind::Enum>().name,
                isopen ? "true" : "false"
            ));
        });

    // village list
    villageCommand.overload().text("list").execute([](CommandOrigin const&, CommandOutput& output) {
        output.success(coral_fans::mod().getVillageManager().listTickingVillages());
    });

    // village info <id: softenum>
    ll::command::CommandRegistrar::getInstance().tryRegisterSoftEnum("villageid", {});
    villageCommand.runtimeOverload()
        .text("info")
        .required("id", ll::command::ParamKind::SoftEnum, "villageid")
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            auto rst =
                coral_fans::mod().getVillageManager().getVillageInfo(self["id"].get<ll::command::ParamKind::SoftEnum>()
                );
            if (rst.second) return output.success(rst.first);
            else return output.error(rst.first);
        });

    // village dweller
    villageCommand.overload().text("dweller").execute([](CommandOrigin const& origin, CommandOutput& output) {
        COMMAND_CHECK_PLAYER
        auto hitrst = player->traceRay(5.25f, true, false);
        if (!hitrst) return output.error("command.village.dweller.noactor"_tr());
        auto* actor = hitrst.getEntity();
        if (!actor) return output.error("command.village.dweller.noactor"_tr());
        else {
            auto rst = coral_fans::mod().getVillageManager().getVillagerInfo(actor->getOrCreateUniqueID());
            if (rst.second) return output.success(rst.first);
            else return output.error(rst.first);
        }
    });

    coral_fans::functions::CFVillageManager::hookVillage(true);
}

} // namespace coral_fans::commands