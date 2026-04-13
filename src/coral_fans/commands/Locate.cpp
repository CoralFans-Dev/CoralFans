#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Mod.h"
#include "coral_fans/functions/locate/DuplicatableManager.h"


#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/runtime/ParamKind.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/i18n/I18n.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"


namespace coral_fans::commands {
void registerLocateCommand(CommandPermissionLevel permission) {
    using ll::i18n_literals::operator""_tr;

    auto& locateCommand = ll::command::CommandRegistrar::getInstance(false)
                              .getOrCreateCommand("cflocate", "command.locate.description"_tr(), permission);

    // locate duplicatable
    // <netherite|nether_spring|nether_fire|glow_stone|mushroom|nether_gold|nether_quartz|nether_magma|nether_gravel|blackstone|soul_sand|end_island|chorus_flower|end_gateway>
    // <bool>
    std::vector<std::pair<std::string, uint64>> enums;
    auto&                                       duplicatableConfig = mod().getConfig().functions.locate.duplicatable;
    if (duplicatableConfig.netherite.enable) {
        enums.emplace_back("netherite", 0);
    }
    if (duplicatableConfig.netherSpring.enable) {
        enums.emplace_back("nether_spring", 1);
    }
    if (duplicatableConfig.netherFire.enable) {
        enums.emplace_back("nether_fire", 2);
    }
    if (duplicatableConfig.glowStone.enable) {
        enums.emplace_back("glow_stone", 3);
    }
    if (duplicatableConfig.mushroom.enable) {
        enums.emplace_back("mushroom", 4);
    }
    if (duplicatableConfig.netherGold.enable) {
        enums.emplace_back("nether_gold", 5);
    }
    if (duplicatableConfig.netherQuartz.enable) {
        enums.emplace_back("nether_quartz", 6);
    }
    if (duplicatableConfig.netherMagma.enable) {
        enums.emplace_back("nether_magma", 7);
    }
    if (duplicatableConfig.netherGravel.enable) {
        enums.emplace_back("nether_gravel", 8);
    }
    if (duplicatableConfig.netherBlackstone.enable) {
        enums.emplace_back("blackstone", 9);
    }
    if (duplicatableConfig.netherSoulSand.enable) {
        enums.emplace_back("soul_sand", 10);
    }
    if (duplicatableConfig.endIsland.enable) {
        enums.emplace_back("end_island", 11);
    }
    if (duplicatableConfig.chorusFlower.enable) {
        enums.emplace_back("chorus_flower", 12);
    }
    if (duplicatableConfig.endGateway.enable) {
        enums.emplace_back("end_gateway", 13);
    }
    ll::command::CommandRegistrar::getInstance(false).tryRegisterRuntimeEnum("duplicatableShowType", std::move(enums));

    locateCommand.runtimeOverload()
        .text("duplicatable")
        .required("type", ll::command::ParamKind::Enum, "duplicatableShowType")
        .optional("enable", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const&, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            auto& duplicatableManager = functions::locate::DuplicatableManager::getInstance();
            auto  showType            = functions::locate::DuplicatableManager::ShowType(0);
            switch (self["type"].get<ll::command::ParamKind::Enum>().index) {
            case 0:
                showType = functions::locate::DuplicatableManager::ShowType::Netherite;
                break;
            case 1:
                showType = functions::locate::DuplicatableManager::ShowType::NetherSpring;
                break;
            case 2:
                showType = functions::locate::DuplicatableManager::ShowType::NetherFire;
                break;
            case 3:
                showType = functions::locate::DuplicatableManager::ShowType::GlowStone;
                break;
            case 4:
                showType = functions::locate::DuplicatableManager::ShowType::Mushroom;
                break;
            case 5:
                showType = functions::locate::DuplicatableManager::ShowType::NetherGold;
                break;
            case 6:
                showType = functions::locate::DuplicatableManager::ShowType::NetherQuartz;
                break;
            case 7:
                showType = functions::locate::DuplicatableManager::ShowType::NetherMagma;
                break;
            case 8:
                showType = functions::locate::DuplicatableManager::ShowType::NetherGravel;
                break;
            case 9:
                showType = functions::locate::DuplicatableManager::ShowType::Blackstone;
                break;
            case 10:
                showType = functions::locate::DuplicatableManager::ShowType::SoulSand;
                break;
            case 11:
                showType = functions::locate::DuplicatableManager::ShowType::EndIsland;
                break;
            case 12:
                showType = functions::locate::DuplicatableManager::ShowType::ChorusFlower;
                break;
            case 13:
                showType = functions::locate::DuplicatableManager::ShowType::EndGateway;
                break;
            }
            if (!static_cast<uint>(showType)) return output.success("command.locate.duplicatable.show.error"_tr());
            if (self["enable"].has_value())
                duplicatableManager.setShowType(showType, self["enable"].get<ll::command::ParamKind::Bool>());
            else duplicatableManager.setShowType(showType, !duplicatableManager.getShowType(showType));
            bool isopen = duplicatableManager.getShowType(showType);
            output.success("command.locate.duplicatable.show.output"_tr(
                self["type"].get<ll::command::ParamKind::Enum>().name,
                isopen ? "true" : "false"
            ));
        });

    // locateCommand.runtimeOverload().text("test").execute(
    //     [](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
    //         COMMAND_CHECK_PLAYER
    //         output.success(functions::locate::DuplicatableManager::getInstance().test(ChunkPos(player->getPosition())));
    //     }
    // );

    functions::locate::DuplicatableManager::hook(true);
}
} // namespace coral_fans::commands