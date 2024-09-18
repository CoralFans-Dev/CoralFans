#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Mod.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/ParamKind.h"
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
    enum class VillageShowType : int { bounds, raid, spawn, center, poi, bind };
    struct VillageShowParam {
        VillageShowType type;
        bool            enable;
    };
    villageCommand.overload<VillageShowParam>().text("show").required("type").required("enable").execute(
        [](CommandOrigin const&, CommandOutput& output, VillageShowParam const& param) {
            switch (param.type) {
            case VillageShowType::bounds:
                coral_fans::mod().getVillageManager().setShowBounds(param.enable);
                break;
            case VillageShowType::raid:
                coral_fans::mod().getVillageManager().setShowRaidBounds(param.enable);
                break;
            case VillageShowType::spawn:
                coral_fans::mod().getVillageManager().setShowIronSpawn(param.enable);
                break;
            case VillageShowType::center:
                coral_fans::mod().getVillageManager().setShowCenter(param.enable);
                break;
            case VillageShowType::poi:
                coral_fans::mod().getVillageManager().setShowPoiQuery(param.enable);
                break;
            case VillageShowType::bind:
                coral_fans::mod().getVillageManager().setShowBind(param.enable);
                break;
            }
            output.success(
                "command.village.show.output"_tr(magic_enum::enum_name(param.type), param.enable ? "true" : "false")
            );
        }
    );

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
        auto  hitrst = player->traceRay(5.25f, true, false);
        auto* actor  = hitrst.getEntity();
        if (!actor) return output.error("command.village.dweller.noactor"_tr());
        else {
            auto rst = coral_fans::mod().getVillageManager().getVillagerInfo(actor->getOrCreateUniqueID());
            if (rst.second) return output.success(rst.first);
            else return output.error(rst.first);
        }
    });
}

} // namespace coral_fans::commands