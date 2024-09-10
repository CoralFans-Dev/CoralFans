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