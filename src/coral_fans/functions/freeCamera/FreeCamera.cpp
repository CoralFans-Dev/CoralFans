#include "FreeCamera.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/common/ActorUniqueID.h"
#include "mc/deps/core/math/Vec3.h"
#include "mc/network/ServerNetworkHandler.h"
#include "mc/network/packet/AddPlayerPacket.h"
#include "mc/network/packet/PlayerInputTick.h"
#include "mc/network/packet/PlayerSkinPacket.h"
#include "mc/network/packet/RemoveActorPacket.h"
#include "mc/network/packet/UpdateAbilitiesPacket.h"
#include "mc/network/packet/UpdatePlayerGameTypePacket.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/actor/Actor.h "
#include "mc/world/actor/player/SerializedSkin.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/Tick.h"


namespace coral_fans::functions {

void EnableFreeCameraPacket(Player* pl) {
    auto pkt            = UpdatePlayerGameTypePacket();
    pkt.mPlayerGameType = GameType::Spectator;
    pkt.mTargetPlayer   = pl->getOrCreateUniqueID();
    pkt.mTick->mValue   = ll::service::getLevel()->getCurrentTick().tickID;
    pkt.sendTo(*pl);
    // ((gmlib::world::actor::GMPlayer*)pl)->setClientGamemode(GameType::Spectator);
}

void SendFakePlayerPacket(Player* pl) {
    // Client Player
    auto pkt1             = AddPlayerPacket(*pl);
    pkt1.mEntityId->rawID = pkt1.mEntityId->rawID + 114514;
    auto randomUuid       = mce::UUID::random();
    pkt1.mUuid            = randomUuid;
    pl->sendNetworkPacket(pkt1);
    // Update Skin
    // auto skin = SerializedSkin(pl->getConnectionRequest());

    // auto pkt2                  = PlayerSkinPacket();
    // pkt2.mUUID                 = randomUuid;
    // pkt2.mSkin                 = skin;
    // pkt2.mLocalizedNewSkinName = "";
    // pkt2.mLocalizedOldSkinName = "";
    // pkt2.sendTo(*pl);


    // gmlib::network::GMBinaryStream bs;
    // bs.writePacketHeader(MinecraftPacketIds::PlayerSkin);
    // bs.writeUuid(randomUuid);
    // bs.writeSkin(skin);
    // bs.writeString("");
    // bs.writeString("");
    // bs.writeBool(true);
    // bs.sendTo(
    //     *(gmlib::world::actor::GMPlayer*)pl,
    //     NetworkPeer::Reliability::ReliableOrdered,
    //     Compressibility::Compressible
    // );
}

void DisableFreeCameraPacket(Player* pl) {
    auto pkt            = UpdatePlayerGameTypePacket();
    pkt.mPlayerGameType = pl->getPlayerGameType();
    pkt.mTargetPlayer   = pl->getOrCreateUniqueID();
    pkt.mTick->mValue   = ll::service::getLevel()->getCurrentTick().tickID;
    pkt.sendTo(*pl);
    // ((gmlib::world::actor::GMPlayer*)pl)->setClientGamemode(pl->getPlayerGameType());
    auto uniqueId  = pl->getOrCreateUniqueID();
    uniqueId.rawID = uniqueId.rawID + 114514;
    // RemoveActorPacket(uniqueId).sendTo(*pl);
    auto pkt2      = RemoveActorPacket();
    pkt2.mEntityId = uniqueId;
    pkt2.sendTo(*pl);
    UpdateAbilitiesPacket(pl->getOrCreateUniqueID(), pl->getAbilities()).sendTo(*pl);
}

/*
void SendActorLinkPacket(Player* pl) {
    auto links = pl->getLinks();
    for (auto& link : links) {
        GMLIB_BinaryStream bs;
        if (ll::service::getLevel()->getPlayer(link.A)) {
            bs.writeVarInt64(link.A.id + 114514);
        } else {
            bs.writeVarInt64(link.A.id);
        }
        if (ll::service::getLevel()->getPlayer(link.B)) {
            bs.writeVarInt64(link.B.id + 114514);
        } else {
            bs.writeVarInt64(link.B.id);
        }
        bs.writeUnsignedChar((uchar)link.type);
        bs.writeBool(link.mImmediate);
        bs.writeBool(link.mPassengerInitiated);
        GMLIB::Server::NetworkPacket<(int)MinecraftPacketIds::SetActorLink> pkt(bs.getAndReleaseData());
        pl->sendNetworkPacket(pkt);
    }
}
*/

void FreeCameraManager::EnableFreeCamera(Player* pl) {
    FreeCameraManager::getInstance().FreeCamList.insert(pl->getNetworkIdentifier().mGuid.g);
    EnableFreeCameraPacket(pl);
    SendFakePlayerPacket(pl);
    // SendActorLinkPacket(pl);
}

void FreeCameraManager::DisableFreeCamera(Player* pl) {
    auto pos   = pl->getFeetPos();
    auto dimid = pl->getDimensionId();
    // auto links = pl->getLinks();
    FreeCameraManager::getInstance().FreeCamList.erase(pl->getNetworkIdentifier().mGuid.g);
    DisableFreeCameraPacket(pl);
    pl->teleport(pos, dimid);
    // for (auto& link : links) {
    //     auto ride  = ll::service::getLevel()->fetchEntity(link.A);
    //     auto rider = ll::service::getLevel()->fetchEntity(link.B);
    //     if (ride && rider) {
    //        rider->startRiding(*ride);
    //    }
    //}
}

LL_TYPE_INSTANCE_HOOK(
    ServerPlayerMoveHandleEvent,
    ll::memory::HookPriority::Normal,
    ServerNetworkHandler,
    &ServerNetworkHandler::$handle,
    void,
    NetworkIdentifier const&     id,
    PlayerAuthInputPacket const& pkt
) {
    if (FreeCameraManager::getInstance().FreeCamList.contains(id.mGuid.g)) {
        return;
    } else {
        return origin(id, pkt);
    }
}

LL_TYPE_INSTANCE_HOOK(
    PlayerGamemodeChangeEvent,
    ll::memory::HookPriority::Normal,
    ServerPlayer,
    &ServerPlayer::$setPlayerGameType,
    void,
    ::GameType gamemode
) {
    origin(gamemode);
    if (FreeCameraManager::getInstance().FreeCamList.contains(getNetworkIdentifier().mGuid.g)) {
        FreeCameraManager::DisableFreeCamera(this);
    }
}

LL_TYPE_INSTANCE_HOOK(
    PlayerHurtEvent,
    ll::memory::HookPriority::Normal,
    Mob,
    &Mob::getDamageAfterResistanceEffect,
    float,
    class ActorDamageSource const& a1,
    float                          a2
) {
    auto res = origin(a1, a2);
    if (this->isType(ActorType::Player) && res != 0) {
        auto pl = (Player*)this;
        if ((pl->isSurvival() || pl->isAdventure())
            && FreeCameraManager::getInstance().FreeCamList.contains(pl->getNetworkIdentifier().mGuid.g)) {
            FreeCameraManager::DisableFreeCamera(pl);
        }
    }
    return res;
}

LL_TYPE_INSTANCE_HOOK(
    PlayerDieEvent,
    ll::memory::HookPriority::Normal,
    Player,
    &Player::$die,
    void,
    class ActorDamageSource const& a1
) {
    if (FreeCameraManager::getInstance().FreeCamList.contains(getNetworkIdentifier().mGuid.g)) {
        FreeCameraManager::DisableFreeCamera(this);
    }
    return origin(a1);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerLeftEvent,
    ll::memory::HookPriority::Normal,
    ServerPlayer,
    &ServerPlayer::disconnect,
    void
) {
    FreeCameraManager::getInstance().FreeCamList.erase(getNetworkIdentifier().mGuid.g);
    return origin();
}

struct Impl {
    ll::memory::HookRegistrar<
        ServerPlayerMoveHandleEvent,
        PlayerGamemodeChangeEvent,
        PlayerHurtEvent,
        PlayerDieEvent,
        PlayerLeftEvent>
        r;
};

std::unique_ptr<Impl> impl;

void FreeCameraManager::freecameraHook(bool enable) {
    if (enable) {
        if (!impl) impl = std::make_unique<Impl>();
    } else {
        impl.reset();
    }
};

} // namespace coral_fans::functions