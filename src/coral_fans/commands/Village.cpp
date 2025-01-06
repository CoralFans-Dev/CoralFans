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
    ll::command::CommandRegistrar::getInstance().tryRegisterEnum(
        "villageShowType",
        {
            {"bounds", 0},
            {"raid", 1},
            {"spawn", 2},
            {"center", 3},
            {"poi", 4},
            {"bind", 5}
        },
        Bedrock::type_id<CommandRegistry, std::pair<std::string,uint64>>(),
        &CommandRegistry::parse<std::pair<std::string,uint64>>
    );
    villageCommand.runtimeOverload()
        .text("show")
        .required("type", ll::command::ParamKind::Enum, "villageShowType")
        .required("enable", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            switch (self["type"].get<ll::command::ParamKind::Enum>().index) {
            case 0:
                coral_fans::mod().getVillageManager().setShowBounds(self["enable"].get<ll::command::ParamKind::Bool>());
                break;
            case 1:
                coral_fans::mod().getVillageManager().setShowRaidBounds(
                    self["enable"].get<ll::command::ParamKind::Bool>()
                );
                break;
            case 2:
                coral_fans::mod().getVillageManager().setShowIronSpawn(self["enable"].get<ll::command::ParamKind::Bool>(
                ));
                break;
            case 3:
                coral_fans::mod().getVillageManager().setShowCenter(self["enable"].get<ll::command::ParamKind::Bool>());
                break;
            case 4:
                coral_fans::mod().getVillageManager().setShowPoiQuery(self["enable"].get<ll::command::ParamKind::Bool>()
                );
                break;
            case 5:
                coral_fans::mod().getVillageManager().setShowBind(self["enable"].get<ll::command::ParamKind::Bool>());
                break;
            }
            output.success("command.village.show.output"_tr(
                self["type"].get<ll::command::ParamKind::Enum>().name,
                self["enable"].get<ll::command::ParamKind::Bool>() ? "true" : "false"
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
}

} // namespace coral_fans::commands