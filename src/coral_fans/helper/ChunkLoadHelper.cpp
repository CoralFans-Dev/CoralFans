#include "coral_fans/base/Mod.h"
#include <ll/api/memory/Hook.h>
#include <mc/network/LoopbackPacketSender.h>
#include <mc/network/packet/LevelChunkPacket.h>


namespace coral_fans::help {
LL_TYPE_INSTANCE_HOOK(
    ChunkLoadHelper,
    ll::memory::HookPriority::Normal,
    LoopbackPacketSender,
    &LoopbackPacketSender::$sendToClient,
    void,
    ::NetworkIdentifier const& id,
    ::Packet const&            packet,
    ::SubClientId              recipientSubId
) {
    if (packet.getId() == MinecraftPacketIds::FullChunkData) [[unlikely]] {

        const auto&          levelChunkPacket = static_cast<LevelChunkPacket const&>(packet);
        const auto&          chunkPos         = levelChunkPacket.mPos;
        const DimensionType& dimId            = *levelChunkPacket.mDimensionId;
        mod().onChunkLoaded(dimId, chunkPos);
    }
    origin(id, packet, recipientSubId);
};
} // namespace coral_fans::help