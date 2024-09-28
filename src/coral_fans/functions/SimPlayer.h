#pragma once

#include "boost/serialization/version.hpp"
#include "mc/math/Vec2.h"
#include "mc/math/Vec3.h"
#include "mc/server/SimulatedPlayer.h"
#include <boost/serialization/access.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/version.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace coral_fans::functions {

class SimPlayerManager {
private:
    enum SimPlayerStatus : int {
        Offline,
        Alive,
        Dead,
    };
    struct SimPlayerInfo {
        std::string                     name;
        std::string                     xuid;
        std::string                     ownerUuid;
        std::unordered_set<std::string> groups;
        int                             status;
        float                           offlinePosX;
        float                           offlinePosY;
        float                           offlinePosZ;
        int                             offlineDim;
        float                           offlineRotX;
        float                           offlineRotY;
        std::string                     offlineGameType;
        bool                            offlineEmptyInv;
        SimulatedPlayer*                simPlayer;
        template <typename Archive>
        void serialize(Archive& ar, const unsigned int version) {
            if (version == 1) {
                ar & name;
                ar & xuid;
                ar & ownerUuid;
                ar & groups;
                ar & status;
                ar & offlinePosX;
                ar & offlinePosY;
                ar & offlinePosZ;
                ar & offlineDim;
                ar & offlineRotX;
                ar & offlineRotY;
                ar & offlineGameType;
                ar & offlineEmptyInv;
            }
        }
    };
    std::unordered_map<std::string, SimPlayerInfo>                   mNameSimPlayerMap;
    std::unordered_map<std::string, std::unordered_set<std::string>> mOwnerNameMap;
    std::unordered_map<std::string, std::unordered_set<std::string>> mGroupNameMap;
    std::unordered_map<std::string, std::unordered_set<std::string>> mGroupAdminMap;
    unsigned long long                                               mOnlineCount = 0;
    unsigned long long                                               mSpawnCount  = 0;
    bool                                                             autorespawn  = false;
    bool                                                             autojoin     = false;

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version) {
        if (version == 1) {
            ar & mNameSimPlayerMap;
            ar & mOwnerNameMap;
            ar & mGroupNameMap;
            ar & mGroupAdminMap;
            ar & autorespawn;
            ar & autojoin;
        }
    }

private:
    void refreshSoftEnum();

public:
    void save();
    void load();

public:
    std::string listSimPlayer();
    std::string listGroup();

public:
    inline void setAutoRespawn(bool isopen) { this->autorespawn = isopen; }
    inline void setAutoJoin(bool isopen) { this->autojoin = isopen; }
    inline bool getAutoRespawn() { return this->autorespawn; }
    inline bool getAutoJoin() { return this->autojoin; }

public:
    std::pair<std::string, bool> createGroup(Player*, std::string const&);
    std::pair<std::string, bool> deleteGroup(Player*, std::string const&);
    std::pair<std::string, bool> addSpToGroup(Player*, std::string const&, std::string const&);
    std::pair<std::string, bool> rmSpFromGroup(Player*, std::string const&, std::string const&);
    std::pair<std::string, bool> addAdminToGroup(Player*, std::string const&, Player*);
    std::pair<std::string, bool> rmAdminFromGroup(Player*, std::string const&, Player*);

public:
    std::pair<std::string, bool> spawnSimPlayer(Player*, std::string const&, Vec3 const&, Vec2 const&);
    std::pair<std::string, bool> spawnGroup(Player*, std::string const&);
    std::pair<std::string, bool> despawnSimPlayer(Player*, std::string const&, bool);
    std::pair<std::string, bool> despawnGroup(Player*, std::string const&);
    std::pair<std::string, bool> rmSimPlayer(Player*, std::string const&, bool);
    std::pair<std::string, bool> rmGroup(Player*, std::string const&);
    std::pair<std::string, bool> respawnSimPlayer(Player*, std::string const&, bool);
    std::pair<std::string, bool> respawnGroup(Player*, std::string const&);

public:
    void setDead(std::string const&);
};

} // namespace coral_fans::functions

BOOST_CLASS_VERSION(coral_fans::functions::SimPlayerManager, 1)
BOOST_CLASS_VERSION(coral_fans::functions::SimPlayerManager::SimPlayerInfo, 1)