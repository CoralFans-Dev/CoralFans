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
    blockSource.setBlock(blockPos, newBlock, 3, nullptr, nullptr);                                                     \
    return;