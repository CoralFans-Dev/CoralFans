#define COMMAND_CHECK_PLAYER                                                                                           \
    auto* entity = origin.getEntity();                                                                                 \
    if (entity == nullptr || !entity->isType(ActorType::Player)) {                                                     \
        output.error("Only players can run this command");                                                             \
        return;                                                                                                        \
    }                                                                                                                  \
    auto* player = static_cast<Player*>(entity);

#define HOOK_HOPPER_RETURN                                                                                             \
    origin(slot, item);                                                                                                \
    return;

// from trapdoor-ll
#define PROF_TIMER(label, Codes)                                                                                       \
    auto start_##label = std::chrono::high_resolution_clock::now();                                                    \
    { Codes }                                                                                                          \
    auto e_##label    = std::chrono::high_resolution_clock ::now() - start_##label;                                    \
    auto time_##label = std::chrono::duration_cast<std::chrono::microseconds>(e_##label).count();

#define ROTATE_BLOCK(tag, rule, type, tagType)                                                                         \
    auto direction = type(states[tag].get<tagType>());                                                                 \
    auto next      = rule.find(direction);                                                                             \
    if (next == rule.end()) return;                                                                                    \
    auto newTag           = CompoundTag(nbtTag);                                                                       \
    newTag["states"][tag] = tagType(next->second);                                                                     \
    auto newBlock         = Block::tryGetFromRegistry(newTag);                                                         \
    if (!newBlock) return;                                                                                             \
    BlockActor* blockEntity = blockSource.getBlockEntity(blockPos);                                                    \
    if (blockEntity) {                                                                                                 \
        std::unique_ptr<CompoundTag> blockEntityTag = std::make_unique<CompoundTag>();                                 \
        blockEntity->save(*blockEntityTag);                                                                            \
        blockSource.removeBlockEntity(blockPos);                                                                       \
        blockSource.setBlock(blockPos, Block::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);               \
        blockSource.setBlock(blockPos, newBlock, 3, BlockActor::create(*blockEntityTag), nullptr, nullptr);            \
        return;                                                                                                        \
    } else {                                                                                                           \
        blockSource.setBlock(blockPos, Block::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);               \
        blockSource.setBlock(blockPos, newBlock, 3, nullptr, nullptr);                                                 \
        return;                                                                                                        \
    }

#define COMMAND_SIMPLAYER_CHECKPERMLIST                                                                                \
    auto& mod  = coral_fans::mod();                                                                                    \
    auto  uuid = player->getUuid().asString();                                                                         \
    switch (mod.getConfig().simPlayer.listType) {                                                                      \
    case coral_fans::config::ListType::disabled:                                                                       \
        break;                                                                                                         \
    case coral_fans::config::ListType::blacklist:                                                                      \
        if (mod.getConfig().simPlayer.list.find(uuid) != mod.getConfig().simPlayer.list.end())                         \
            return output.error("command.sp.error.permissiondenied"_tr());                                             \
        break;                                                                                                         \
    case coral_fans::config::ListType::whitelist:                                                                      \
        if (mod.getConfig().simPlayer.list.find(uuid) == mod.getConfig().simPlayer.list.end())                         \
            return output.error("command.sp.error.permissiondenied"_tr());                                             \
        break;                                                                                                         \
    }

// SimPlayer reg def
#define SP_REG_DEF(NAME, ...)                                                                                          \
    std::pair<std::string, bool> simPlayer##NAME(Player*, std::string const&, bool, __VA_ARGS__);                      \
    std::pair<std::string, bool> group##NAME(Player*, std::string const&, __VA_ARGS__);

// SimPlayer def with arg
#define SP_DEF_WA(NAME, ACTION, ARG_TYPE)                                                                              \
    std::pair<std::string, bool>                                                                                       \
        SimPlayerManager::simPlayer##NAME(Player* player, std::string const& spname, bool noCheck, ARG_TYPE arg) {     \
        using ll::i18n_literals::operator""_tr;                                                                        \
        auto uuid = player->getUuid();                                                                                 \
        auto it   = this -> mNameSimPlayerMap.find(spname);                                                            \
        if (it == this->mNameSimPlayerMap.end()) return {"translate.simplayer.error.notfound"_tr(), false};            \
        if (player->getCommandPermissionLevel() >= coral_fans::mod().getConfig().simPlayer.adminPermission || noCheck  \
            || uuid == it->second.ownerUuid) {                                                                         \
            if (it->second.status != SimPlayerStatus::Alive)                                                           \
                return {"translate.simplayer.error.statuserror"_tr(), false};                                          \
            if (it->second.simPlayer) it->second.ACTION(arg);                                                          \
            return {"translate.simplayer.success"_tr(), true};                                                         \
        }                                                                                                              \
        return {"translate.simplayer.error.permissiondenied"_tr(), false};                                             \
    }                                                                                                                  \
    std::pair<std::string, bool> SimPlayerManager::group##NAME(                                                        \
        Player*            player,                                                                                     \
        std::string const& gname,                                                                                      \
        ARG_TYPE           arg                                                                                         \
    ) {                                                                                                                \
        using ll::i18n_literals::operator""_tr;                                                                        \
        auto adminIt = this -> mGroupAdminMap.find(gname);                                                             \
        auto it      = this -> mGroupNameMap.find(gname);                                                              \
        if (it == this->mGroupNameMap.end() || adminIt == this->mGroupAdminMap.end())                                  \
            return {"translate.simplayer.error.notfound"_tr(), false};                                                 \
        if (adminIt->second.find(player->getUuid().asString()) == adminIt->second.end())                               \
            return {"translate.simplayer.error.permissiondenied"_tr(), false};                                         \
        for (auto const& v : it->second) this->simPlayer##NAME(player, v, true, arg);                                  \
        return {"translate.simplayer.success"_tr(), true};                                                             \
    }

// SimPlayer def
#define SP_DEF(NAME, ACTION)                                                                                           \
    std::pair<std::string, bool> SimPlayerManager::simPlayer##NAME(                                                    \
        Player*            player,                                                                                     \
        std::string const& spname,                                                                                     \
        bool               noCheck                                                                                     \
    ) {                                                                                                                \
        using ll::i18n_literals::operator""_tr;                                                                        \
        auto uuid = player->getUuid();                                                                                 \
        auto it   = this -> mNameSimPlayerMap.find(spname);                                                            \
        if (it == this->mNameSimPlayerMap.end()) return {"translate.simplayer.error.notfound"_tr(), false};            \
        if (player->getCommandPermissionLevel() >= coral_fans::mod().getConfig().simPlayer.adminPermission || noCheck  \
            || uuid == it->second.ownerUuid) {                                                                         \
            if (it->second.status != SimPlayerStatus::Alive)                                                           \
                return {"translate.simplayer.error.statuserror"_tr(), false};                                          \
            if (it->second.simPlayer) it->second.ACTION();                                                             \
            return {"translate.simplayer.success"_tr(), true};                                                         \
        }                                                                                                              \
        return {"translate.simplayer.error.permissiondenied"_tr(), false};                                             \
    }                                                                                                                  \
    std::pair<std::string, bool> SimPlayerManager::group##NAME(Player* player, std::string const& gname) {             \
        using ll::i18n_literals::operator""_tr;                                                                        \
        auto adminIt = this -> mGroupAdminMap.find(gname);                                                             \
        auto it      = this -> mGroupNameMap.find(gname);                                                              \
        if (it == this->mGroupNameMap.end() || adminIt == this->mGroupAdminMap.end())                                  \
            return {"translate.simplayer.error.notfound"_tr(), false};                                                 \
        if (adminIt->second.find(player->getUuid().asString()) == adminIt->second.end())                               \
            return {"translate.simplayer.error.permissiondenied"_tr(), false};                                         \
        for (auto const& v : it->second) this->simPlayer##NAME(player, v, true);                                       \
        return {"translate.simplayer.success"_tr(), true};                                                             \
    }

// SimPlayer def task with arg
#define SP_DEF_TASK_WA(NAME, ACTION, ARG_TYPE)                                                                         \
    std::pair<std::string, bool> SimPlayerManager::simPlayer##NAME(                                                    \
        Player*            player,                                                                                     \
        std::string const& spname,                                                                                     \
        bool               noCheck,                                                                                    \
        ARG_TYPE           arg,                                                                                        \
        int                interval,                                                                                   \
        int                times                                                                                       \
    ) {                                                                                                                \
        using ll::i18n_literals::operator""_tr;                                                                        \
        auto uuid = player->getUuid();                                                                                 \
        auto it   = this -> mNameSimPlayerMap.find(spname);                                                            \
        if (it == this->mNameSimPlayerMap.end()) return {"translate.simplayer.error.notfound"_tr(), false};            \
        if (player->getCommandPermissionLevel() >= coral_fans::mod().getConfig().simPlayer.adminPermission || noCheck  \
            || uuid == it->second.ownerUuid) {                                                                         \
            if (it->second.status != SimPlayerStatus::Alive)                                                           \
                return {"translate.simplayer.error.statuserror"_tr(), false};                                          \
            if (!it->second.isFree()) return {"translate.simplayer.error.nonfree"_tr(), false};                        \
            if (it->second.simPlayer) it->second.ACTION(arg);                                                          \
            if (interval >= 1) {                                                                                       \
                it->second.taskid =                                                                                    \
                    this->mScheduler->add(interval, [times, sp = it->second, arg](unsigned long long t) mutable {      \
                        if (times > 0 && t > (unsigned long long)times - 1) return false;                              \
                        if (sp.simPlayer) [[likely]] {                                                                 \
                            sp.ACTION(arg);                                                                            \
                            return true;                                                                               \
                        }                                                                                              \
                        return false;                                                                                  \
                    });                                                                                                \
            } else return {"translate.simplayer.error.nonpositive"_tr(), false};                                       \
            return {"translate.simplayer.success"_tr(), true};                                                         \
        }                                                                                                              \
        return {"translate.simplayer.error.permissiondenied"_tr(), false};                                             \
    }                                                                                                                  \
    std::pair<std::string, bool> SimPlayerManager::group##NAME(                                                        \
        Player*            player,                                                                                     \
        std::string const& gname,                                                                                      \
        ARG_TYPE           arg,                                                                                        \
        int                interval,                                                                                   \
        int                times                                                                                       \
    ) {                                                                                                                \
        using ll::i18n_literals::operator""_tr;                                                                        \
        auto adminIt = this -> mGroupAdminMap.find(gname);                                                             \
        auto it      = this -> mGroupNameMap.find(gname);                                                              \
        if (it == this->mGroupNameMap.end() || adminIt == this->mGroupAdminMap.end())                                  \
            return {"translate.simplayer.error.notfound"_tr(), false};                                                 \
        if (adminIt->second.find(player->getUuid().asString()) == adminIt->second.end())                               \
            return {"translate.simplayer.error.permissiondenied"_tr(), false};                                         \
        for (auto const& v : it->second) this->simPlayer##NAME(player, v, true, arg, interval, times);                 \
        return {"translate.simplayer.success"_tr(), true};                                                             \
    }

// SimPlayer def task
#define SP_DEF_TASK(NAME, ACTION)                                                                                      \
    std::pair<std::string, bool> SimPlayerManager::simPlayer##NAME(                                                    \
        Player*            player,                                                                                     \
        std::string const& spname,                                                                                     \
        bool               noCheck,                                                                                    \
        int                interval,                                                                                   \
        int                times                                                                                       \
    ) {                                                                                                                \
        using ll::i18n_literals::operator""_tr;                                                                        \
        auto uuid = player->getUuid();                                                                                 \
        auto it   = this -> mNameSimPlayerMap.find(spname);                                                            \
        if (it == this->mNameSimPlayerMap.end()) return {"translate.simplayer.error.notfound"_tr(), false};            \
        if (player->getCommandPermissionLevel() >= coral_fans::mod().getConfig().simPlayer.adminPermission || noCheck  \
            || uuid == it->second.ownerUuid) {                                                                         \
            if (it->second.status != SimPlayerStatus::Alive)                                                           \
                return {"translate.simplayer.error.statuserror"_tr(), false};                                          \
            if (!it->second.isFree()) return {"translate.simplayer.error.nonfree"_tr(), false};                        \
            if (it->second.simPlayer) it->second.ACTION();                                                             \
            if (interval >= 1) {                                                                                       \
                it->second.taskid =                                                                                    \
                    this->mScheduler->add(interval, [times, sp = it->second](unsigned long long t) mutable {           \
                        if (times > 0 && t > (unsigned long long)times - 1) return false;                              \
                        if (sp.simPlayer) [[likely]] {                                                                 \
                            sp.ACTION();                                                                               \
                            return true;                                                                               \
                        }                                                                                              \
                        return false;                                                                                  \
                    });                                                                                                \
            } else return {"translate.simplayer.error.nonpositive"_tr(), false};                                       \
            return {"translate.simplayer.success"_tr(), true};                                                         \
        }                                                                                                              \
        return {"translate.simplayer.error.permissiondenied"_tr(), false};                                             \
    }                                                                                                                  \
    std::pair<std::string, bool>                                                                                       \
        SimPlayerManager::group##NAME(Player* player, std::string const& gname, int interval, int times) {             \
        using ll::i18n_literals::operator""_tr;                                                                        \
        auto adminIt = this -> mGroupAdminMap.find(gname);                                                             \
        auto it      = this -> mGroupNameMap.find(gname);                                                              \
        if (it == this->mGroupNameMap.end() || adminIt == this->mGroupAdminMap.end())                                  \
            return {"translate.simplayer.error.notfound"_tr(), false};                                                 \
        if (adminIt->second.find(player->getUuid().asString()) == adminIt->second.end())                               \
            return {"translate.simplayer.error.permissiondenied"_tr(), false};                                         \
        for (auto const& v : it->second) this->simPlayer##NAME(player, v, true, interval, times);                      \
        return {"translate.simplayer.success"_tr(), true};                                                             \
    }
