#include "ll/api/memory/Hook.h"
#include "mc/deps/shared_types/legacy/LevelEvent.h"
#include "mc/network/packet/LevelEventPacket.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/actor/player/AbilitiesIndex.h"
#include "mc/world/actor/player/LayeredAbilities.h"


#ifdef LL_PLAT_C
#include "ll/api/service/Bedrock.h"
#include "mc/server/ServerInstance.h"
#include <thread>
#endif


namespace coral_fans::functions {
LL_TYPE_INSTANCE_HOOK(PlayerLeftHook, ll::memory::HookPriority::Normal, ServerPlayer, &ServerPlayer::disconnect, void) {
#ifdef LL_PLAT_C
    if (auto serverInstance = ll::service::getServerInstance();
        !serverInstance
        || std::this_thread::get_id() != ll::service::getServerInstance()->mServerInstanceThread->get_id())
        return origin();
#endif
    getAbilities().setAbility(AbilitiesIndex::FlySpeed, 0.05f);
    return origin();
}

void TickHook(bool enable) { enable ? PlayerLeftHook::hook() : PlayerLeftHook::unhook(); }
} // namespace coral_fans::functions