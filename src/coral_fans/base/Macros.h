#define COMMAND_CHECK_PLAYER                                                                                           \
    auto entity = origin.getEntity();                                                                                  \
    if (entity == nullptr || !entity->isType(ActorType::Player)) {                                                     \
        output.error("command.error.checkPlayer"_tr());                                                                \
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
        blockEntity->save(*blockEntityTag, , *SaveContextFactory::createCloneSaveContext());                           \
        blockSource.removeBlockEntity(blockPos);                                                                       \
        blockSource.setBlock(blockPos, Block::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);               \
        blockSource.setBlock(blockPos, newBlock, 3, BlockActor::create(*blockEntityTag), nullptr, nullptr);            \
        return;                                                                                                        \
    } else {                                                                                                           \
        blockSource.setBlock(blockPos, Block::tryGetFromRegistry("minecraft:air"), 3, nullptr, nullptr);               \
        blockSource.setBlock(blockPos, newBlock, 3, nullptr, nullptr);                                                 \
        return;                                                                                                        \
    }

#ifdef LL_PLAT_C
#include "ll/api/service/Bedrock.h"
#include "mc/server/ServerInstance.h"
#include <thread>

#define RETURN_IF_NOT_MAIN_THREAD(...)                                                                                 \
    do {                                                                                                               \
        if (auto serverInstance = ll::service::getServerInstance();                                                    \
            !serverInstance || std::this_thread::get_id() != serverInstance->mServerInstanceThread->get_id()) {        \
            __VA_ARGS__;                                                                                               \
        }                                                                                                              \
    } while (false)

#define RETURN_VOID_IF_NOT_MAIN_THREAD                                                                                 \
    do {                                                                                                               \
        if (auto serverInstance = ll::service::getServerInstance();                                                    \
            !serverInstance || std::this_thread::get_id() != serverInstance->mServerInstanceThread->get_id()) {        \
            return;                                                                                                    \
        }                                                                                                              \
    } while (false)
#else
#define RETURN_IF_NOT_MAIN_THREAD(...)
#define RETURN_VOID_IF_NOT_MAIN_THREAD
#endif