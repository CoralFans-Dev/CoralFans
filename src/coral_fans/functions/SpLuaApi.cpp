#include "boost/algorithm/string/join.hpp"
#include "coral_fans/CoralFans.h"
#include "coral_fans/base/Macros.h"
#include "coral_fans/base/Mod.h"
#include "coral_fans/base/Utils.h"
#include "coral_fans/functions/SimPlayer.h"
#include "ll/api/base/StdInt.h"
#include "ll/api/service/Bedrock.h"
#include "mc/math/Vec2.h"
#include "mc/math/Vec3.h"
#include "mc/nbt/ByteArrayTag.h"
#include "mc/nbt/ByteTag.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/nbt/CompoundTagVariant.h"
#include "mc/nbt/FloatTag.h"
#include "mc/nbt/IntArrayTag.h"
#include "mc/nbt/IntTag.h"
#include "mc/nbt/ListTag.h"
#include "mc/nbt/ShortTag.h"
#include "mc/nbt/StringTag.h"
#include "mc/nbt/Tag.h"
#include "mc/network/packet/TextPacket.h"
#include "mc/util/ProfilerLite.h"
#include "mc/world/item/registry/ItemStack.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/Block.h"
#include <filesystem>
#include <memory>
#include <string>
#include <utility>
extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

namespace coral_fans::functions {

namespace sputils::lua_api {

namespace {

void compoundTagToLuaTable(lua_State* L, CompoundTagVariant tag) {
    if (tag.hold(Tag::Compound)) {
        lua_newtable(L);
        for (const auto& item : tag.get<CompoundTag>()) {
            lua_pushstring(L, item.first.c_str());
            compoundTagToLuaTable(L, item.second);
            lua_settable(L, -3);
        }
        return;
    }
    if (tag.hold(Tag::ByteArray)) {
        int idx = 1;
        lua_newtable(L);
        for (const auto& item : tag.get<ByteArrayTag>()) {
            lua_pushinteger(L, item);
            lua_seti(L, -2, idx);
            ++idx;
        }
        return;
    }
    if (tag.hold(Tag::IntArray)) {
        int idx = 1;
        lua_newtable(L);
        for (const auto& item : tag.get<IntArrayTag>()) {
            lua_pushinteger(L, item);
            lua_seti(L, -2, idx);
            ++idx;
        }
        return;
    }
    if (tag.hold(Tag::List)) {
        int idx = 1;
        lua_newtable(L);
        for (const auto& item : tag.get<ListTag>()) {
            compoundTagToLuaTable(L, item);
            lua_seti(L, -2, idx);
            ++idx;
        }
        return;
    }
    if (tag.hold(Tag::Byte)) {
        lua_pushinteger(L, tag.get<ByteTag>());
        return;
    }
    if (tag.hold(Tag::Double)) {
        lua_pushnumber(L, tag.get<DoubleTag>());
        return;
    }
    if (tag.hold(Tag::Float)) {
        lua_pushnumber(L, tag.get<FloatTag>());
        return;
    }
    if (tag.hold(Tag::Int)) {
        lua_pushinteger(L, tag.get<IntTag>());
        return;
    }
    if (tag.hold(Tag::Int64)) {
        lua_pushinteger(L, tag.get<Int64Tag>());
        return;
    }
    if (tag.hold(Tag::Short)) {
        lua_pushinteger(L, tag.get<ShortTag>());
        return;
    }
    if (tag.hold(Tag::String)) {
        lua_pushstring(L, tag.get<StringTag>().c_str());
        return;
    }
}

} // namespace

namespace {

LUAAPI(log) {
    int                    count = lua_gettop(L);
    std::list<std::string> lst;
    for (int i = 1; i <= count; ++i) lst.emplace_back(luaL_checkstring(L, i));
    coral_fans::CoralFans::getInstance().getSelf().getLogger().info("[CFSP-Lua] {}", boost::algorithm::join(lst, "\t"));
    return 0;
}

// userdata vec3 begin

LUAAPI(vec3_new) {
    LUA_ARG_COUNT_CHECK_C(3)
    float x = static_cast<float>(luaL_checknumber(L, 1));
    float y = static_cast<float>(luaL_checknumber(L, 2));
    float z = static_cast<float>(luaL_checknumber(L, 3));
    lua_settop(L, 0);
    Vec3* pos = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
    pos->x    = x;
    pos->y    = y;
    pos->z    = z;
    luaL_setmetatable(L, "vec3_mt");
    return 1;
}

LUAAPI(vec3_newHalf) {
    LUA_ARG_COUNT_CHECK_C(0)
    Vec3* pos = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
    *pos      = Vec3::HALF;
    luaL_setmetatable(L, "vec3_mt");
    return 1;
}

LUAAPI(vec3_newMax) {
    LUA_ARG_COUNT_CHECK_C(0)
    Vec3* pos = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
    *pos      = Vec3::MAX;
    luaL_setmetatable(L, "vec3_mt");
    return 1;
}

LUAAPI(vec3_newMin) {
    LUA_ARG_COUNT_CHECK_C(0)
    Vec3* pos = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
    *pos      = Vec3::MIN;
    luaL_setmetatable(L, "vec3_mt");
    return 1;
}

LUAAPI(vec3_newNegUnitX) {
    LUA_ARG_COUNT_CHECK_C(0)
    Vec3* pos = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
    *pos      = Vec3::NEG_UNIT_X;
    luaL_setmetatable(L, "vec3_mt");
    return 1;
}

LUAAPI(vec3_newNegUnitY) {
    LUA_ARG_COUNT_CHECK_C(0)
    Vec3* pos = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
    *pos      = Vec3::NEG_UNIT_Y;
    luaL_setmetatable(L, "vec3_mt");
    return 1;
}

LUAAPI(vec3_newNegUnitZ) {
    LUA_ARG_COUNT_CHECK_C(0)
    Vec3* pos = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
    *pos      = Vec3::NEG_UNIT_Z;
    luaL_setmetatable(L, "vec3_mt");
    return 1;
}

LUAAPI(vec3_newOne) {
    LUA_ARG_COUNT_CHECK_C(0)
    Vec3* pos = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
    *pos      = Vec3::ONE;
    luaL_setmetatable(L, "vec3_mt");
    return 1;
}

LUAAPI(vec3_newTwo) {
    LUA_ARG_COUNT_CHECK_C(0)
    Vec3* pos = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
    *pos      = Vec3::TWO;
    luaL_setmetatable(L, "vec3_mt");
    return 1;
}

LUAAPI(vec3_newUnitX) {
    LUA_ARG_COUNT_CHECK_C(0)
    Vec3* pos = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
    *pos      = Vec3::UNIT_X;
    luaL_setmetatable(L, "vec3_mt");
    return 1;
}

LUAAPI(vec3_newUnitY) {
    LUA_ARG_COUNT_CHECK_C(0)
    Vec3* pos = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
    *pos      = Vec3::UNIT_Y;
    luaL_setmetatable(L, "vec3_mt");
    return 1;
}

LUAAPI(vec3_newUnitZ) {
    LUA_ARG_COUNT_CHECK_C(0)
    Vec3* pos = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
    *pos      = Vec3::UNIT_Z;
    luaL_setmetatable(L, "vec3_mt");
    return 1;
}

LUAAPI(vec3_newZero) {
    LUA_ARG_COUNT_CHECK_C(0)
    Vec3* pos = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
    *pos      = Vec3::ZERO;
    luaL_setmetatable(L, "vec3_mt");
    return 1;
}

static const luaL_Reg lua_reg_vec3_c[] = {
    {"new",         lua_api_vec3_new        },
    {"newHalf",     lua_api_vec3_newHalf    },
    {"newMax",      lua_api_vec3_newMax     },
    {"newMin",      lua_api_vec3_newMin     },
    {"newNegUnitX", lua_api_vec3_newNegUnitX},
    {"newNegUnitY", lua_api_vec3_newNegUnitY},
    {"newNegUnitZ", lua_api_vec3_newNegUnitZ},
    {"newOne",      lua_api_vec3_newOne     },
    {"newTwo",      lua_api_vec3_newTwo     },
    {"newUnitX",    lua_api_vec3_newUnitX   },
    {"newUnitY",    lua_api_vec3_newUnitY   },
    {"newUnitZ",    lua_api_vec3_newUnitZ   },
    {"newZero",     lua_api_vec3_newZero    },
    {NULL,          NULL                    }
};

LUAAPI(vec3_get) {
    LUA_ARG_COUNT_CHECK_M(0)
    Vec3* pos = (Vec3*)luaL_checkudata(L, 1, "vec3_mt");
    luaL_argcheck(L, pos != nullptr, 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushnumber(L, pos->x);
    lua_pushnumber(L, pos->y);
    lua_pushnumber(L, pos->z);
    return 3;
}

LUAAPI(vec3_meta_tostring) {
    LUA_ARG_COUNT_CHECK_M(0)
    Vec3* pos = (Vec3*)luaL_checkudata(L, 1, "vec3_mt");
    luaL_argcheck(L, pos != nullptr, 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushstring(L, pos->toString().c_str());
    return 1;
}

LUAAPI(vec3_meta_add) {
    LUA_ARG_COUNT_CHECK_M(1)
    Vec3* pos1 = (Vec3*)luaL_checkudata(L, 1, "vec3_mt");
    luaL_argcheck(L, pos1 != nullptr, 1, "invalid userdata");
    Vec3* pos2 = (Vec3*)luaL_checkudata(L, 2, "vec3_mt");
    luaL_argcheck(L, pos2 != nullptr, 2, "invalid userdata");
    lua_settop(L, 0);
    Vec3* pos = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
    *pos      = (*pos1) + (*pos2);
    luaL_setmetatable(L, "vec3_mt");
    return 1;
}

LUAAPI(vec3_meta_sub) {
    LUA_ARG_COUNT_CHECK_M(1)
    Vec3* pos1 = (Vec3*)luaL_checkudata(L, 1, "vec3_mt");
    luaL_argcheck(L, pos1 != nullptr, 1, "invalid userdata");
    Vec3* pos2 = (Vec3*)luaL_checkudata(L, 2, "vec3_mt");
    luaL_argcheck(L, pos2 != nullptr, 2, "invalid userdata");
    lua_settop(L, 0);
    Vec3* pos = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
    *pos      = (*pos1) - (*pos2);
    luaL_setmetatable(L, "vec3_mt");
    return 1;
}

LUAAPI(vec3_meta_eq) {
    LUA_ARG_COUNT_CHECK_M(1)
    Vec3* pos1 = (Vec3*)luaL_checkudata(L, 1, "vec3_mt");
    luaL_argcheck(L, pos1 != nullptr, 1, "invalid userdata");
    Vec3* pos2 = (Vec3*)luaL_checkudata(L, 2, "vec3_mt");
    luaL_argcheck(L, pos2 != nullptr, 2, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, ((*pos1) == (*pos2)));
    return 1;
}

static const luaL_Reg lua_reg_vec3_m[] = {
    {"get",        lua_api_vec3_get          },
    {"__tostring", lua_api_vec3_meta_tostring},
    {"__add",      lua_api_vec3_meta_add     },
    {"__sub",      lua_api_vec3_meta_sub     },
    {"__eq",       lua_api_vec3_meta_eq      },
    {NULL,         NULL                      }
};

LUAAPI(open_vec3) {
    luaL_newmetatable(L, "vec3_mt");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, lua_reg_vec3_m, 0);
    luaL_newlib(L, lua_reg_vec3_c);
    return 1;
}

// userdata vec3 end

// userdata blockpos begin

LUAAPI(blockpos_new) {
    LUA_ARG_COUNT_CHECK_C(3)
    int x = static_cast<int>(luaL_checkinteger(L, 1));
    int y = static_cast<int>(luaL_checkinteger(L, 2));
    int z = static_cast<int>(luaL_checkinteger(L, 3));
    lua_settop(L, 0);
    BlockPos* bp = (BlockPos*)lua_newuserdata(L, sizeof(BlockPos));
    bp->x        = x;
    bp->y        = y;
    bp->z        = z;
    luaL_setmetatable(L, "blockpos_mt");
    return 1;
}

LUAAPI(blockpos_newMax) {
    LUA_ARG_COUNT_CHECK_C(0)
    BlockPos* bp = (BlockPos*)lua_newuserdata(L, sizeof(BlockPos));
    *bp          = BlockPos::MAX;
    luaL_setmetatable(L, "blockpos_mt");
    return 1;
}

LUAAPI(blockpos_newMin) {
    LUA_ARG_COUNT_CHECK_C(0)
    BlockPos* bp = (BlockPos*)lua_newuserdata(L, sizeof(BlockPos));
    *bp          = BlockPos::MIN;
    luaL_setmetatable(L, "blockpos_mt");
    return 1;
}

LUAAPI(blockpos_newOne) {
    LUA_ARG_COUNT_CHECK_C(0)
    BlockPos* bp = (BlockPos*)lua_newuserdata(L, sizeof(BlockPos));
    *bp          = BlockPos::ONE;
    luaL_setmetatable(L, "blockpos_mt");
    return 1;
}

LUAAPI(blockpos_newZero) {
    LUA_ARG_COUNT_CHECK_C(0)
    BlockPos* bp = (BlockPos*)lua_newuserdata(L, sizeof(BlockPos));
    *bp          = BlockPos::ZERO;
    luaL_setmetatable(L, "blockpos_mt");
    return 1;
}

static const luaL_Reg lua_reg_blockpos_c[] = {
    {"new",     lua_api_blockpos_new    },
    {"newMax",  lua_api_blockpos_newMax },
    {"newMin",  lua_api_blockpos_newMin },
    {"newOne",  lua_api_blockpos_newOne },
    {"newZero", lua_api_blockpos_newZero},
    {NULL,      NULL                    }
};

LUAAPI(blockpos_get) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockPos* bp = (BlockPos*)luaL_checkudata(L, 1, "blockpos_mt");
    luaL_argcheck(L, bp != nullptr, 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushinteger(L, bp->x);
    lua_pushinteger(L, bp->y);
    lua_pushinteger(L, bp->z);
    return 3;
}

LUAAPI(blockpos_bottomCenter) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockPos* bp = (BlockPos*)luaL_checkudata(L, 1, "blockpos_mt");
    luaL_argcheck(L, bp != nullptr, 1, "invalid userdata");
    lua_settop(L, 0);
    Vec3* pos = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
    *pos      = bp->bottomCenter();
    luaL_setmetatable(L, "vec3_mt");
    return 1;
}

LUAAPI(blockpos_center) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockPos* bp = (BlockPos*)luaL_checkudata(L, 1, "blockpos_mt");
    luaL_argcheck(L, bp != nullptr, 1, "invalid userdata");
    lua_settop(L, 0);
    Vec3* pos = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
    *pos      = bp->center();
    luaL_setmetatable(L, "vec3_mt");
    return 1;
}

LUAAPI(blockpos_toVec3) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockPos* bp = (BlockPos*)luaL_checkudata(L, 1, "blockpos_mt");
    luaL_argcheck(L, bp != nullptr, 1, "invalid userdata");
    lua_settop(L, 0);
    Vec3* pos = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
    *pos      = Vec3(*bp);
    luaL_setmetatable(L, "vec3_mt");
    return 1;
}

LUAAPI(blockpos_neighbor) {
    LUA_ARG_COUNT_CHECK_M(1)
    BlockPos* bp = (BlockPos*)luaL_checkudata(L, 1, "blockpos_mt");
    luaL_argcheck(L, bp != nullptr, 1, "invalid userdata");
    uchar facing = static_cast<uchar>(luaL_checkinteger(L, 2));
    lua_settop(L, 0);
    BlockPos* ret_bp = (BlockPos*)lua_newuserdata(L, sizeof(BlockPos));
    *ret_bp          = bp->neighbor(facing);
    luaL_setmetatable(L, "blockpos_mt");
    return 1;
}

LUAAPI(blockpos_meta_tostring) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockPos* bp = (BlockPos*)luaL_checkudata(L, 1, "blockpos_mt");
    luaL_argcheck(L, bp != nullptr, 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushstring(L, bp->toString().c_str());
    return 1;
}

LUAAPI(blockpos_meta_add) {
    LUA_ARG_COUNT_CHECK_M(1)
    BlockPos* bp1 = (BlockPos*)luaL_checkudata(L, 1, "blockpos_mt");
    luaL_argcheck(L, bp1 != nullptr, 1, "invalid userdata");
    BlockPos* bp2 = (BlockPos*)luaL_checkudata(L, 2, "blockpos_mt");
    luaL_argcheck(L, bp2 != nullptr, 2, "invalid userdata");
    lua_settop(L, 0);
    BlockPos* bp = (BlockPos*)lua_newuserdata(L, sizeof(BlockPos));
    *bp          = (*bp1) + (*bp2);
    luaL_setmetatable(L, "blockpos_mt");
    return 1;
}

LUAAPI(blockpos_meta_sub) {
    LUA_ARG_COUNT_CHECK_M(1)
    BlockPos* bp1 = (BlockPos*)luaL_checkudata(L, 1, "blockpos_mt");
    luaL_argcheck(L, bp1 != nullptr, 1, "invalid userdata");
    BlockPos* bp2 = (BlockPos*)luaL_checkudata(L, 2, "blockpos_mt");
    luaL_argcheck(L, bp2 != nullptr, 2, "invalid userdata");
    lua_settop(L, 0);
    BlockPos* bp = (BlockPos*)lua_newuserdata(L, sizeof(BlockPos));
    *bp          = (*bp1) - (*bp2);
    luaL_setmetatable(L, "blockpos_mt");
    return 1;
}

LUAAPI(blockpos_meta_eq) {
    LUA_ARG_COUNT_CHECK_M(1)
    BlockPos* bp1 = (BlockPos*)luaL_checkudata(L, 1, "blockpos_mt");
    luaL_argcheck(L, bp1 != nullptr, 1, "invalid userdata");
    BlockPos* bp2 = (BlockPos*)luaL_checkudata(L, 2, "blockpos_mt");
    luaL_argcheck(L, bp2 != nullptr, 2, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, ((*bp1) == (*bp2)));
    return 1;
}

static const luaL_Reg lua_reg_blockpos_m[] = {
    {"get",          lua_api_blockpos_get          },
    {"bottomCenter", lua_api_blockpos_bottomCenter },
    {"center",       lua_api_blockpos_center       },
    {"toVec3",       lua_api_blockpos_toVec3       },
    {"neighbor",     lua_api_blockpos_neighbor     },
    {"__tostring",   lua_api_blockpos_meta_tostring},
    {"__add",        lua_api_blockpos_meta_add     },
    {"__sub",        lua_api_blockpos_meta_sub     },
    {"__eq",         lua_api_blockpos_meta_eq      },
    {NULL,           NULL                          }
};

LUAAPI(open_blockpos) {
    luaL_newmetatable(L, "blockpos_mt");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, lua_reg_blockpos_m, 0);
    luaL_newlib(L, lua_reg_blockpos_c);
    return 1;
}

// userdata blockpos end

// userdata vec2 begin

LUAAPI(vec2_new) {
    LUA_ARG_COUNT_CHECK_C(2)
    float x = static_cast<float>(luaL_checknumber(L, 1));
    float y = static_cast<float>(luaL_checknumber(L, 2));
    lua_settop(L, 0);
    Vec2* rot = (Vec2*)lua_newuserdata(L, sizeof(Vec2));
    rot->x    = x;
    rot->y    = y;
    luaL_setmetatable(L, "vec2_mt");
    return 1;
}

LUAAPI(vec2_newLowest) {
    LUA_ARG_COUNT_CHECK_C(0)
    Vec2* rot = (Vec2*)lua_newuserdata(L, sizeof(Vec2));
    *rot      = Vec2::LOWEST;
    luaL_setmetatable(L, "vec2_mt");
    return 1;
}

LUAAPI(vec2_newMax) {
    LUA_ARG_COUNT_CHECK_C(0)
    Vec2* rot = (Vec2*)lua_newuserdata(L, sizeof(Vec2));
    *rot      = Vec2::MAX;
    luaL_setmetatable(L, "vec2_mt");
    return 1;
}

LUAAPI(vec2_newMin) {
    LUA_ARG_COUNT_CHECK_C(0)
    Vec2* rot = (Vec2*)lua_newuserdata(L, sizeof(Vec2));
    *rot      = Vec2::MIN;
    luaL_setmetatable(L, "vec2_mt");
    return 1;
}

LUAAPI(vec2_newNegUnitX) {
    LUA_ARG_COUNT_CHECK_C(0)
    Vec2* rot = (Vec2*)lua_newuserdata(L, sizeof(Vec2));
    *rot      = Vec2::NEG_UNIT_X;
    luaL_setmetatable(L, "vec2_mt");
    return 1;
}

LUAAPI(vec2_newNegUnitY) {
    LUA_ARG_COUNT_CHECK_C(0)
    Vec2* rot = (Vec2*)lua_newuserdata(L, sizeof(Vec2));
    *rot      = Vec2::NEG_UNIT_Y;
    luaL_setmetatable(L, "vec2_mt");
    return 1;
}

LUAAPI(vec2_newOne) {
    LUA_ARG_COUNT_CHECK_C(0)
    Vec2* rot = (Vec2*)lua_newuserdata(L, sizeof(Vec2));
    *rot      = Vec2::ONE;
    luaL_setmetatable(L, "vec2_mt");
    return 1;
}

LUAAPI(vec2_newUnitX) {
    LUA_ARG_COUNT_CHECK_C(0)
    Vec2* rot = (Vec2*)lua_newuserdata(L, sizeof(Vec2));
    *rot      = Vec2::UNIT_X;
    luaL_setmetatable(L, "vec2_mt");
    return 1;
}

LUAAPI(vec2_newUnitY) {
    LUA_ARG_COUNT_CHECK_C(0)
    Vec2* rot = (Vec2*)lua_newuserdata(L, sizeof(Vec2));
    *rot      = Vec2::UNIT_Y;
    luaL_setmetatable(L, "vec2_mt");
    return 1;
}

LUAAPI(vec2_newZero) {
    LUA_ARG_COUNT_CHECK_C(0)
    Vec2* rot = (Vec2*)lua_newuserdata(L, sizeof(Vec2));
    *rot      = Vec2::ZERO;
    luaL_setmetatable(L, "vec2_mt");
    return 1;
}

static const luaL_Reg lua_reg_vec2_c[] = {
    {"new",         lua_api_vec2_new        },
    {"newLowest",   lua_api_vec2_newLowest  },
    {"newMax",      lua_api_vec2_newMax     },
    {"newMin",      lua_api_vec2_newMin     },
    {"newNegUnitX", lua_api_vec2_newNegUnitX},
    {"newNegUnitY", lua_api_vec2_newNegUnitY},
    {"newOne",      lua_api_vec2_newOne     },
    {"newUnitX",    lua_api_vec2_newUnitX   },
    {"newUnitY",    lua_api_vec2_newUnitY   },
    {"newZero",     lua_api_vec2_newZero    },
    {NULL,          NULL                    }
};

LUAAPI(vec2_get) {
    LUA_ARG_COUNT_CHECK_M(0)
    Vec2* rot = (Vec2*)luaL_checkudata(L, 1, "vec2_mt");
    luaL_argcheck(L, rot != nullptr, 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushnumber(L, rot->x);
    lua_pushnumber(L, rot->y);
    return 2;
}

LUAAPI(vec2_meta_tostring) {
    LUA_ARG_COUNT_CHECK_M(0)
    Vec2* rot = (Vec2*)luaL_checkudata(L, 1, "vec2_mt");
    luaL_argcheck(L, rot != nullptr, 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushstring(L, rot->toString().c_str());
    return 1;
}

LUAAPI(vec2_meta_add) {
    LUA_ARG_COUNT_CHECK_M(1)
    Vec2* rot1 = (Vec2*)luaL_checkudata(L, 1, "vec2_mt");
    luaL_argcheck(L, rot1 != nullptr, 1, "invalid userdata");
    Vec2* rot2 = (Vec2*)luaL_checkudata(L, 2, "vec2_mt");
    luaL_argcheck(L, rot2 != nullptr, 2, "invalid userdata");
    lua_settop(L, 0);
    Vec2* rot = (Vec2*)lua_newuserdata(L, sizeof(Vec2));
    *rot      = (*rot1) + (*rot2);
    luaL_setmetatable(L, "vec2_mt");
    return 1;
}

LUAAPI(vec2_meta_sub) {
    LUA_ARG_COUNT_CHECK_M(1)
    Vec2* rot1 = (Vec2*)luaL_checkudata(L, 1, "vec2_mt");
    luaL_argcheck(L, rot1 != nullptr, 1, "invalid userdata");
    Vec2* rot2 = (Vec2*)luaL_checkudata(L, 2, "vec2_mt");
    luaL_argcheck(L, rot2 != nullptr, 2, "invalid userdata");
    lua_settop(L, 0);
    Vec2* rot = (Vec2*)lua_newuserdata(L, sizeof(Vec2));
    *rot      = (*rot1) - (*rot2);
    luaL_setmetatable(L, "vec2_mt");
    return 1;
}

LUAAPI(vec2_meta_eq) {
    LUA_ARG_COUNT_CHECK_M(1)
    Vec2* rot1 = (Vec2*)luaL_checkudata(L, 1, "vec2_mt");
    luaL_argcheck(L, rot1 != nullptr, 1, "invalid userdata");
    Vec2* rot2 = (Vec2*)luaL_checkudata(L, 2, "vec2_mt");
    luaL_argcheck(L, rot2 != nullptr, 2, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, ((*rot1) == (*rot2)));
    return 1;
}

static const luaL_Reg lua_reg_vec2_m[] = {
    {"get",        lua_api_vec2_get          },
    {"__tostring", lua_api_vec2_meta_tostring},
    {"__add",      lua_api_vec2_meta_add     },
    {"__sub",      lua_api_vec2_meta_sub     },
    {"__eq",       lua_api_vec2_meta_eq      },
    {NULL,         NULL                      }
};

LUAAPI(open_vec2) {
    luaL_newmetatable(L, "vec2_mt");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, lua_reg_vec2_m, 0);
    luaL_newlib(L, lua_reg_vec2_c);
    return 1;
}

// userdata vec2 end

static const luaL_Reg lua_reg_null_c[] = {
    {NULL, NULL}
};

// userdata blockinfo begin

class BlockInfo {
public:
    std::string                  name;
    std::string                  type;
    int                          id;
    BlockPos                     pos;
    int                          dim;
    int                          variant;
    float                        translucency;
    float                        thickness;
    bool                         isAir;
    bool                         isBounceBlock;
    bool                         isButtonBlock;
    bool                         isCropBlock;
    bool                         isDoorBlock;
    bool                         isFallingBlock;
    bool                         isFenceBlock;
    bool                         isFenceGateBlock;
    bool                         isSlabBlock;
    bool                         isStemBlock;
    bool                         isThinFenceBlock;
    bool                         isUnbreakable;
    bool                         isWaterBlockingBlock;
    std::unique_ptr<CompoundTag> tag;

public:
    BlockInfo(BlockPos const bp, int d, Block const& bl)
    : name(bl.buildDescriptionName()),
      type(bl.getTypeName()),
      id(bl.getBlockItemId()),
      pos(bp),
      dim(d),
      variant(bl.getVariant()),
      translucency(bl.getTranslucency()),
      thickness(bl.getThickness()),
      isAir(bl.isAir()),
      isBounceBlock(bl.isBounceBlock()),
      isButtonBlock(bl.isButtonBlock()),
      isCropBlock(bl.isCropBlock()),
      isDoorBlock(bl.isDoorBlock()),
      isFallingBlock(bl.isFallingBlock()),
      isFenceBlock(bl.isFenceBlock()),
      isFenceGateBlock(bl.isFenceGateBlock()),
      isSlabBlock(bl.isSlabBlock()),
      isStemBlock(bl.isStemBlock()),
      isThinFenceBlock(bl.isThinFenceBlock()),
      isUnbreakable(bl.isUnbreakable()),
      isWaterBlockingBlock(bl.isWaterBlocking()),
      tag(std::make_unique<CompoundTag>(bl.getSerializationId())) {}

    bool operator==(const BlockInfo& bi) const {
        return name == bi.name && type == bi.type && id == bi.id && pos == bi.pos && dim == bi.dim
            && variant == bi.variant && translucency == bi.translucency && thickness == bi.thickness
            && isAir == bi.isAir && isBounceBlock == bi.isBounceBlock && isButtonBlock == bi.isButtonBlock
            && isCropBlock == bi.isCropBlock && isDoorBlock == bi.isDoorBlock && isFallingBlock == bi.isFallingBlock
            && isFenceBlock == bi.isFenceBlock && isFenceGateBlock == bi.isFenceGateBlock
            && isSlabBlock == bi.isSlabBlock && isStemBlock == bi.isStemBlock && isThinFenceBlock == bi.isThinFenceBlock
            && isUnbreakable == bi.isUnbreakable && isWaterBlockingBlock == bi.isWaterBlockingBlock
            && *tag == (*bi.tag);
    }
};

LUAAPI(blockinfo_getName) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushstring(L, (*binfo)->name.c_str());
    return 1;
}

LUAAPI(blockinfo_getType) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushstring(L, (*binfo)->type.c_str());
    return 1;
}

LUAAPI(blockinfo_getId) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushinteger(L, (*binfo)->id);
    return 1;
}

LUAAPI(blockinfo_getPos) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    BlockPos* bp = (BlockPos*)lua_newuserdata(L, sizeof(BlockPos));
    *bp          = (*binfo)->pos;
    luaL_setmetatable(L, "blockpos_mt");
    return 1;
}

LUAAPI(blockinfo_getDim) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushinteger(L, (*binfo)->dim);
    return 1;
}

LUAAPI(blockinfo_getVariant) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushinteger(L, (*binfo)->variant);
    return 1;
}

LUAAPI(blockinfo_getTranslucency) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushnumber(L, (*binfo)->translucency);
    return 1;
}

LUAAPI(blockinfo_getThickness) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushnumber(L, (*binfo)->thickness);
    return 1;
}

LUAAPI(blockinfo_isAir) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*binfo)->isAir);
    return 1;
}

LUAAPI(blockinfo_isBounceBlock) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*binfo)->isBounceBlock);
    return 1;
}

LUAAPI(blockinfo_isButtonBlock) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*binfo)->isButtonBlock);
    return 1;
}

LUAAPI(blockinfo_isCropBlock) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*binfo)->isCropBlock);
    return 1;
}

LUAAPI(blockinfo_isDoorBlock) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*binfo)->isDoorBlock);
    return 1;
}

LUAAPI(blockinfo_isFallingBlock) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*binfo)->isFallingBlock);
    return 1;
}

LUAAPI(blockinfo_isFenceBlock) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*binfo)->isFenceBlock);
    return 1;
}

LUAAPI(blockinfo_isFenceGateBlock) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*binfo)->isFenceGateBlock);
    return 1;
}

LUAAPI(blockinfo_isSlabBlock) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*binfo)->isSlabBlock);
    return 1;
}

LUAAPI(blockinfo_isStemBlock) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*binfo)->isStemBlock);
    return 1;
}

LUAAPI(blockinfo_isThinFenceBlock) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*binfo)->isThinFenceBlock);
    return 1;
}

LUAAPI(blockinfo_isUnbreakable) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*binfo)->isUnbreakable);
    return 1;
}

LUAAPI(blockinfo_isWaterBlockingBlock) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*binfo)->isWaterBlockingBlock);
    return 1;
}

LUAAPI(blockinfo_getNbtTable) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    compoundTagToLuaTable(L, *((*binfo)->tag));
    return 1;
}

LUAAPI(blockinfo_getSnbt) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushstring(L, (*binfo)->tag->toSnbt(SnbtFormat::Minimize).c_str());
    return 1;
}

LUAAPI(blockinfo_getSnbtWithPath) {
    LUA_ARG_COUNT_CHECK_M(1)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    std::string path = luaL_checkstring(L, 2);
    lua_settop(L, 0);
    const auto& rst = utils::getNbtFromTag(*((*binfo)->tag), path);
    lua_pushstring(L, rst.first.c_str());
    lua_pushboolean(L, rst.second);
    return 2;
}

LUAAPI(blockinfo_meta_eq) {
    LUA_ARG_COUNT_CHECK_M(1)
    BlockInfo** binfo1 = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo1 != nullptr) && ((*binfo1) != nullptr), 1, "invalid userdata");
    BlockInfo** binfo2 = (BlockInfo**)luaL_checkudata(L, 2, "blockinfo_mt");
    luaL_argcheck(L, (binfo2 != nullptr) && ((*binfo2) != nullptr), 2, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, ((**binfo1) == (**binfo2)));
    return 1;
}

LUAAPI(blockinfo_meta_gc) {
    LUA_ARG_COUNT_CHECK_M(0)
    BlockInfo** binfo = (BlockInfo**)luaL_checkudata(L, 1, "blockinfo_mt");
    luaL_argcheck(L, (binfo != nullptr) && ((*binfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    delete *binfo;
    return 0;
}

static const luaL_Reg lua_reg_blockinfo_m[] = {
    {"getName",              lua_api_blockinfo_getName             },
    {"getType",              lua_api_blockinfo_getType             },
    {"getId",                lua_api_blockinfo_getId               },
    {"getPos",               lua_api_blockinfo_getPos              },
    {"getDim",               lua_api_blockinfo_getDim              },
    {"getVariant",           lua_api_blockinfo_getVariant          },
    {"getTranslucency",      lua_api_blockinfo_getTranslucency     },
    {"getThickness",         lua_api_blockinfo_getThickness        },
    {"isAir",                lua_api_blockinfo_isAir               },
    {"isBounceBlock",        lua_api_blockinfo_isBounceBlock       },
    {"isButtonBlock",        lua_api_blockinfo_isButtonBlock       },
    {"isCropBlock",          lua_api_blockinfo_isCropBlock         },
    {"isDoorBlock",          lua_api_blockinfo_isDoorBlock         },
    {"isFallingBlock",       lua_api_blockinfo_isFallingBlock      },
    {"isFenceBlock",         lua_api_blockinfo_isFenceBlock        },
    {"isFenceGateBlock",     lua_api_blockinfo_isFenceGateBlock    },
    {"isSlabBlock",          lua_api_blockinfo_isSlabBlock         },
    {"isStemBlock",          lua_api_blockinfo_isStemBlock         },
    {"isThinFenceBlock",     lua_api_blockinfo_isThinFenceBlock    },
    {"isUnbreakable",        lua_api_blockinfo_isUnbreakable       },
    {"isWaterBlockingBlock", lua_api_blockinfo_isWaterBlockingBlock},
    {"getNbtTable",          lua_api_blockinfo_getNbtTable         },
    {"getSnbt",              lua_api_blockinfo_getSnbt             },
    {"getSnbtWithPath",      lua_api_blockinfo_getSnbtWithPath     },
    {"__eq",                 lua_api_blockinfo_meta_eq             },
    {"__gc",                 lua_api_blockinfo_meta_gc             },
    {NULL,                   NULL                                  }
};

LUAAPI(open_blockinfo) {
    luaL_newmetatable(L, "blockinfo_mt");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, lua_reg_blockinfo_m, 0);
    luaL_newlib(L, lua_reg_null_c);
    return 1;
}

// userdata blockinfo end

// userdata blocksource begin

LUAAPI(blocksource_getBlockInfo) {
    LUA_ARG_COUNT_CHECK_M(1)
    int* dimid = (int*)luaL_checkudata(L, 1, "blocksource_mt");
    luaL_argcheck(L, dimid != nullptr, 1, "invalid userdata");
    BlockPos* bp = (BlockPos*)luaL_checkudata(L, 2, "blockpos_mt");
    luaL_argcheck(L, bp != nullptr, 2, "invalid userdata");
    lua_settop(L, 0);
    auto level = ll::service::getLevel();
    if (!level.has_value()) {
        lua_pushnil(L);
        lua_pushboolean(L, false);
        return 2;
    }
    const auto& block = level->getDimension(*dimid)->getBlockSourceFromMainChunkSource().getBlock(*bp);
    BlockInfo** binfo = (BlockInfo**)lua_newuserdata(L, sizeof(BlockInfo*));
    *binfo            = new BlockInfo(*bp, *dimid, block);
    luaL_setmetatable(L, "blockinfo_mt");
    lua_pushboolean(L, true);
    return 2;
}

static const luaL_Reg lua_reg_blocksource_m[] = {
    {"getBlockInfo", lua_api_blocksource_getBlockInfo},
    {NULL,           NULL                            }
};

LUAAPI(open_blocksource) {
    luaL_newmetatable(L, "blocksource_mt");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, lua_reg_blocksource_m, 0);
    luaL_newlib(L, lua_reg_null_c);
    return 1;
}

// userdata blocksource end

// userdata level begin

LUAAPI(level_getDayTime) {
    LUA_ARG_COUNT_CHECK_M(0)
    luaL_checkudata(L, 1, "level_mt");
    lua_settop(L, 0);
    auto level = ll::service::getLevel();
    if (!level.has_value()) {
        lua_pushinteger(L, -1);
        return 1;
    }
    lua_pushinteger(L, level->getTime() % 24000);
    return 1;
}

LUAAPI(level_getGameTime) {
    LUA_ARG_COUNT_CHECK_M(0)
    luaL_checkudata(L, 1, "level_mt");
    lua_settop(L, 0);
    auto level = ll::service::getLevel();
    if (!level.has_value()) {
        lua_pushinteger(L, -1);
        return 1;
    }
    lua_pushinteger(L, level->getCurrentTick().t);
    return 1;
}

LUAAPI(level_getDay) {
    LUA_ARG_COUNT_CHECK_M(0)
    luaL_checkudata(L, 1, "level_mt");
    lua_settop(L, 0);
    auto level = ll::service::getLevel();
    if (!level.has_value()) {
        lua_pushinteger(L, -1);
        return 1;
    }
    lua_pushinteger(L, level->getTime() / 24000);
    return 1;
}

LUAAPI(level_getMspt) {
    LUA_ARG_COUNT_CHECK_M(0)
    luaL_checkudata(L, 1, "level_mt");
    lua_settop(L, 0);
    lua_pushnumber(L, static_cast<double>(ProfilerLite::gProfilerLiteInstance.getServerTickTime().count() / 1000000.0));
    return 1;
}

const static luaL_Reg lua_reg_level_m[] = {
    {"getDayTime",  lua_api_level_getDayTime },
    {"getGameTime", lua_api_level_getGameTime},
    {"getDay",      lua_api_level_getDay     },
    {"getMspt",     lua_api_level_getMspt    },
    {NULL,          NULL                     }
};

LUAAPI(open_level) {
    luaL_newmetatable(L, "level_mt");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, lua_reg_level_m, 0);
    luaL_newlib(L, lua_reg_null_c);
    return 1;
}

// userdata level end

// userdata iteminfo begin

class ItemInfo {
public:
    std::string                  name;
    std::string                  type;
    int                          id;
    int                          count;
    int                          aux;
    int                          damage;
    int                          attackDamage;
    int                          maxDamage;
    int                          maxStackSize;
    std::vector<std::string>     lore;
    bool                         isArmorItem;
    bool                         isBlock;
    bool                         isDamageableItem;
    bool                         isDamaged;
    bool                         isEnchanted;
    bool                         isEnchantingBook;
    bool                         isFireResistant;
    bool                         isFullStack;
    bool                         isGlint;
    bool                         isHorseArmorItem;
    bool                         isLiquidClipItem;
    bool                         isMusicDiscItem;
    bool                         isOffhandItem;
    bool                         isPotionItem;
    bool                         isStackable;
    bool                         isWearableItem;
    std::unique_ptr<CompoundTag> tag;

public:
    ItemInfo(ItemStack const& item)
    : name(item.getCustomName().empty() ? item.getName() : item.getCustomName()),
      type(item.getTypeName()),
      id(item.getId()),
      count(item.mCount),
      aux(item.getAuxValue()),
      damage(item.getDamageValue()),
      attackDamage(item.getAttackDamage()),
      maxDamage(item.getMaxDamage()),
      maxStackSize(item.getMaxStackSize()),
      lore(item.getCustomLore()),
      isArmorItem(item.isArmorItem()),
      isBlock(item.isBlock()),
      isDamageableItem(item.isDamageableItem()),
      isDamaged(item.isDamaged()),
      isEnchanted(item.isEnchanted()),
      isEnchantingBook(item.isEnchantingBook()),
      isFireResistant(item.isFireResistant()),
      isFullStack(item.isFullStack()),
      isGlint(item.isGlint()),
      isHorseArmorItem(item.isHorseArmorItem()),
      isLiquidClipItem(item.isLiquidClipItem()),
      isMusicDiscItem(item.isMusicDiscItem()),
      isOffhandItem(item.isOffhandItem()),
      isPotionItem(item.isPotionItem()),
      isStackable(item.isStackable()),
      isWearableItem(item.isHumanoidWearableItem()),
      tag(item.save()) {}
    bool operator==(const ItemInfo& rt) const {
        return name == rt.name && type == rt.type && id == rt.id && count == rt.count && aux == rt.aux
            && damage == rt.damage && attackDamage == rt.attackDamage && maxDamage == rt.maxDamage
            && maxStackSize == rt.maxStackSize && lore == rt.lore && isArmorItem == rt.isArmorItem
            && isBlock == rt.isBlock && isDamageableItem == rt.isDamageableItem && isDamaged == rt.isDamaged
            && isEnchanted == rt.isEnchanted && isEnchantingBook == rt.isEnchantingBook
            && isFireResistant == rt.isFireResistant && isFullStack == rt.isFullStack && isGlint == rt.isGlint
            && isHorseArmorItem == rt.isHorseArmorItem && isLiquidClipItem == rt.isLiquidClipItem
            && isMusicDiscItem == rt.isMusicDiscItem && isOffhandItem == rt.isOffhandItem
            && isPotionItem == rt.isPotionItem && isStackable == rt.isStackable && isWearableItem == rt.isWearableItem
            && (*tag) == (*rt.tag);
    }
};

LUAAPI(iteminfo_getName) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushstring(L, (*info)->name.c_str());
    return 1;
}

LUAAPI(iteminfo_getType) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushstring(L, (*info)->type.c_str());
    return 1;
}

LUAAPI(iteminfo_getId) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushinteger(L, (*info)->id);
    return 1;
}

LUAAPI(iteminfo_getCount) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushinteger(L, (*info)->count);
    return 1;
}

LUAAPI(iteminfo_getAux) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushinteger(L, (*info)->aux);
    return 1;
}

LUAAPI(iteminfo_getDamage) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushinteger(L, (*info)->damage);
    return 1;
}

LUAAPI(iteminfo_getAttackDamage) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushinteger(L, (*info)->attackDamage);
    return 1;
}

LUAAPI(iteminfo_getMaxDamage) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushinteger(L, (*info)->maxDamage);
    return 1;
}

LUAAPI(iteminfo_getMaxStackSize) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushinteger(L, (*info)->maxStackSize);
    return 1;
}

LUAAPI(iteminfo_getLore) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    int idx = 1;
    lua_newtable(L);
    for (const auto& item : (*info)->lore) {
        lua_pushstring(L, item.c_str());
        lua_seti(L, -2, idx);
        ++idx;
    }
    return 1;
}

LUAAPI(iteminfo_isArmorItem) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*info)->isArmorItem);
    return 1;
}

LUAAPI(iteminfo_isBlock) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*info)->isBlock);
    return 1;
}

LUAAPI(iteminfo_isDamageableItem) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*info)->isDamageableItem);
    return 1;
}

LUAAPI(iteminfo_isDamaged) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*info)->isDamaged);
    return 1;
}

LUAAPI(iteminfo_isEnchanted) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*info)->isEnchanted);
    return 1;
}

LUAAPI(iteminfo_isEnchantingBook) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*info)->isEnchantingBook);
    return 1;
}

LUAAPI(iteminfo_isFireResistant) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*info)->isFireResistant);
    return 1;
}

LUAAPI(iteminfo_isFullStack) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*info)->isFullStack);
    return 1;
}

LUAAPI(iteminfo_isGlint) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*info)->isGlint);
    return 1;
}

LUAAPI(iteminfo_isHorseArmorItem) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*info)->isHorseArmorItem);
    return 1;
}

LUAAPI(iteminfo_isLiquidClipItem) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*info)->isLiquidClipItem);
    return 1;
}

LUAAPI(iteminfo_isMusicDiscItem) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*info)->isMusicDiscItem);
    return 1;
}

LUAAPI(iteminfo_isOffhandItem) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*info)->isOffhandItem);
    return 1;
}

LUAAPI(iteminfo_isPotionItem) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*info)->isPotionItem);
    return 1;
}

LUAAPI(iteminfo_isStackable) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*info)->isStackable);
    return 1;
}

LUAAPI(iteminfo_isWearableItem) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, (*info)->isWearableItem);
    return 1;
}

LUAAPI(iteminfo_getNbtTable) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    compoundTagToLuaTable(L, *((*info)->tag));
    return 1;
}

LUAAPI(iteminfo_getSnbt) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    lua_pushstring(L, (*info)->tag->toSnbt(SnbtFormat::Minimize).c_str());
    return 1;
}

LUAAPI(iteminfo_getSnbtWithPath) {
    LUA_ARG_COUNT_CHECK_M(1)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    std::string path = luaL_checkstring(L, 2);
    lua_settop(L, 0);
    const auto& rst = utils::getNbtFromTag(*((*info)->tag), path);
    lua_pushstring(L, rst.first.c_str());
    lua_pushboolean(L, rst.second);
    return 2;
}

LUAAPI(iteminfo_meta_eq) {
    LUA_ARG_COUNT_CHECK_M(1)
    ItemInfo** info1 = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info1 != nullptr) && ((*info1) != nullptr), 1, "invalid userdata");
    ItemInfo** info2 = (ItemInfo**)luaL_checkudata(L, 2, "iteminfo_mt");
    luaL_argcheck(L, (info2 != nullptr) && ((*info2) != nullptr), 2, "invalid userdata");
    lua_settop(L, 0);
    lua_pushboolean(L, ((**info1) == (**info2)));
    return 1;
}

LUAAPI(iteminfo_meta_gc) {
    LUA_ARG_COUNT_CHECK_M(0)
    ItemInfo** info = (ItemInfo**)luaL_checkudata(L, 1, "iteminfo_mt");
    luaL_argcheck(L, (info != nullptr) && ((*info) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    delete *info;
    return 0;
}

static const luaL_Reg lua_reg_iteminfo_m[] = {
    {"getName",          lua_api_iteminfo_getName         },
    {"getType",          lua_api_iteminfo_getType         },
    {"getId",            lua_api_iteminfo_getId           },
    {"getCount",         lua_api_iteminfo_getCount        },
    {"getAux",           lua_api_iteminfo_getAux          },
    {"getDamage",        lua_api_iteminfo_getDamage       },
    {"getAttackDamage",  lua_api_iteminfo_getAttackDamage },
    {"getMaxDamage",     lua_api_iteminfo_getMaxDamage    },
    {"getMaxStackSize",  lua_api_iteminfo_getMaxStackSize },
    {"getLore",          lua_api_iteminfo_getLore         },
    {"isArmorItem",      lua_api_iteminfo_isArmorItem     },
    {"isBlock",          lua_api_iteminfo_isBlock         },
    {"isDamageableItem", lua_api_iteminfo_isDamageableItem},
    {"isDamaged",        lua_api_iteminfo_isDamaged       },
    {"isEnchanted",      lua_api_iteminfo_isEnchanted     },
    {"isEnchantingBook", lua_api_iteminfo_isEnchantingBook},
    {"isFireResistant",  lua_api_iteminfo_isFireResistant },
    {"isFullStack",      lua_api_iteminfo_isFullStack     },
    {"isGlint",          lua_api_iteminfo_isGlint         },
    {"isHorseArmorItem", lua_api_iteminfo_isHorseArmorItem},
    {"isLiquidClipItem", lua_api_iteminfo_isLiquidClipItem},
    {"isMusicDiscItem",  lua_api_iteminfo_isMusicDiscItem },
    {"isOffhandItem",    lua_api_iteminfo_isOffhandItem   },
    {"isPotionItem",     lua_api_iteminfo_isPotionItem    },
    {"isStackable",      lua_api_iteminfo_isStackable     },
    {"isWearableItem",   lua_api_iteminfo_isWearableItem  },
    {"getNbtTable",      lua_api_iteminfo_getNbtTable     },
    {"getSnbt",          lua_api_iteminfo_getSnbt         },
    {"getSnbtWithPath",  lua_api_iteminfo_getSnbtWithPath },
    {"__eq",             lua_api_iteminfo_meta_eq         },
    {"__gc",             lua_api_iteminfo_meta_gc         },
    {NULL,               NULL                             }
};

LUAAPI(open_iteminfo) {
    luaL_newmetatable(L, "iteminfo_mt");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, lua_reg_iteminfo_m, 0);
    luaL_newlib(L, lua_reg_null_c);
    return 1;
}

// userdata iteminfo end

// userdata simplayer begin

LUAAPI(simplayer_getName) {
    LUA_ARG_COUNT_CHECK_M(0)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    lua_settop(L, 0);
    lua_pushstring(L, (*spinfo)->getName().c_str());
    return 1;
}

LUAAPI(simplayer_getXuid) {
    LUA_ARG_COUNT_CHECK_M(0)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    lua_settop(L, 0);
    lua_pushstring(L, (*spinfo)->getXuid().c_str());
    return 1;
}

LUAAPI(simplayer_getStatus) {
    LUA_ARG_COUNT_CHECK_M(0)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    lua_settop(L, 0);
    lua_pushinteger(L, (*spinfo)->getStatus());
    return 1;
}

LUAAPI(simplayer_getPos) {
    LUA_ARG_COUNT_CHECK_M(0)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    lua_settop(L, 0);
    Vec3* pos = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
    *pos      = (*spinfo)->getPos();
    luaL_setmetatable(L, "vec3_mt");
    return 1;
}

LUAAPI(simplayer_getFeetPos) {
    LUA_ARG_COUNT_CHECK_M(0)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    lua_settop(L, 0);
    Vec3* pos = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
    *pos      = (*spinfo)->getFeetPos();
    luaL_setmetatable(L, "vec3_mt");
    return 1;
}

LUAAPI(simplayer_getStandingOn) {
    LUA_ARG_COUNT_CHECK_M(0)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    lua_settop(L, 0);
    BlockPos* pos = (BlockPos*)lua_newuserdata(L, sizeof(BlockPos));
    *pos          = (*spinfo)->getStandingOn();
    luaL_setmetatable(L, "blockpos_mt");
    return 1;
}

LUAAPI(simplayer_getRot) {
    LUA_ARG_COUNT_CHECK_M(0)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    lua_settop(L, 0);
    Vec2* pos = (Vec2*)lua_newuserdata(L, sizeof(Vec2));
    *pos      = (*spinfo)->getRot();
    luaL_setmetatable(L, "vec2_mt");
    return 1;
}

LUAAPI(simplayer_getHealth) {
    LUA_ARG_COUNT_CHECK_M(0)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    lua_settop(L, 0);
    lua_pushinteger(L, (*spinfo)->getHealth());
    return 1;
}

LUAAPI(simplayer_getHunger) {
    LUA_ARG_COUNT_CHECK_M(0)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    lua_settop(L, 0);
    lua_pushnumber(L, (*spinfo)->getHunger());
    return 1;
}

LUAAPI(simplayer_sneaking) {
    LUA_ARG_COUNT_CHECK_M(1)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    luaL_argcheck(
        L,
        lua_isboolean(L, 2),
        2,
        std::string{"boolean expected, got " + std::string{lua_typename(L, lua_type(L, 2))}}.c_str()
    );
    lua_pushboolean(L, (*spinfo)->sneaking(lua_toboolean(L, 2)));
    return 1;
}

LUAAPI(simplayer_swimming) {
    LUA_ARG_COUNT_CHECK_M(1)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    luaL_argcheck(
        L,
        lua_isboolean(L, 2),
        2,
        std::string{"boolean expected, got " + std::string{lua_typename(L, lua_type(L, 2))}}.c_str()
    );
    (*spinfo)->swimming(lua_toboolean(L, 2));
    return 0;
}

LUAAPI(simplayer_attack) {
    LUA_ARG_COUNT_CHECK_M(0)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    lua_settop(L, 0);
    lua_pushboolean(L, (*spinfo)->attack());
    return 1;
}

LUAAPI(simplayer_chat) {
    LUA_ARG_COUNT_CHECK_M(1)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    std::string msg = luaL_checkstring(L, 2);
    lua_settop(L, 0);
    (*spinfo)->chat(msg);
    return 0;
}

LUAAPI(simplayer_destroy) {
    LUA_ARG_COUNT_CHECK_M(0)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    lua_settop(L, 0);
    lua_pushboolean(L, (*spinfo)->destroy());
    return 1;
}

LUAAPI(simplayer_dropSelectedItem) {
    LUA_ARG_COUNT_CHECK_M(0)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    lua_settop(L, 0);
    lua_pushboolean(L, (*spinfo)->dropSelectedItem());
    return 1;
}

LUAAPI(simplayer_dropInv) {
    LUA_ARG_COUNT_CHECK_M(0)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    lua_settop(L, 0);
    lua_pushboolean(L, (*spinfo)->dropInv());
    return 1;
}

LUAAPI(simplayer_runCmd) {
    LUA_ARG_COUNT_CHECK_M(1)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    std::string cmd = luaL_checkstring(L, 2);
    lua_settop(L, 0);
    lua_pushboolean(L, (*spinfo)->runCmd(cmd));
    return 1;
}

LUAAPI(simplayer_getBlockPosFromView) {
    LUA_ARG_COUNT_CHECK_M(0)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    lua_settop(L, 0);
    const auto& rst = (*spinfo)->getBlockPosFromView();
    BlockPos*   pos = (BlockPos*)lua_newuserdata(L, sizeof(BlockPos));
    *pos            = rst.first;
    luaL_setmetatable(L, "blockpos_mt");
    lua_pushboolean(L, rst.second);
    return 2;
}

LUAAPI(simplayer_searchInInvWithId) {
    int count = lua_gettop(L);
    if (count < 2 || count > 3) return luaL_error(L, "1 or 2 args expected (without \"self\")");
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    int id    = static_cast<int>(luaL_checkinteger(L, 2));
    int start = 0;
    if (count == 3) start = static_cast<int>(luaL_checkinteger(L, 3));
    lua_settop(L, 0);
    lua_pushinteger(L, (*spinfo)->searchInInvWithId(id, start));
    return 1;
}

LUAAPI(simplayer_searchInInvWithName) {
    int count = lua_gettop(L);
    if (count < 2 || count > 3) return luaL_error(L, "1 or 2 args expected (without \"self\")");
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    std::string name  = luaL_checkstring(L, 2);
    int         start = 0;
    if (count == 3) start = static_cast<int>(luaL_checkinteger(L, 3));
    lua_settop(L, 0);
    lua_pushinteger(L, (*spinfo)->searchInInvWithName(name, start));
    return 1;
}

LUAAPI(simplayer_selectSlot) {
    LUA_ARG_COUNT_CHECK_M(1)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    int slot = static_cast<int>(luaL_checkinteger(L, 2));
    lua_settop(L, 0);
    lua_pushboolean(L, (*spinfo)->selectSlot(slot));
    return 1;
}

LUAAPI(simplayer_select) {
    LUA_ARG_COUNT_CHECK_M(1)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    int id = static_cast<int>(luaL_checkinteger(L, 2));
    lua_settop(L, 0);
    lua_pushboolean(L, (*spinfo)->select(id));
    return 1;
}

LUAAPI(simplayer_getItemFromInv) {
    LUA_ARG_COUNT_CHECK_M(1)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    int slot = static_cast<int>(luaL_checkinteger(L, 2));
    lua_settop(L, 0);
    ItemInfo** info = (ItemInfo**)lua_newuserdata(L, sizeof(ItemInfo*));
    *info           = new ItemInfo((*spinfo)->getItemFromInv(slot));
    luaL_setmetatable(L, "iteminfo_mt");
    return 1;
}

LUAAPI(simplayer_interact) {
    LUA_ARG_COUNT_CHECK_M(0)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    lua_settop(L, 0);
    lua_pushboolean(L, (*spinfo)->interact());
    return 1;
}

LUAAPI(simplayer_jump) {
    LUA_ARG_COUNT_CHECK_M(0)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    lua_settop(L, 0);
    lua_pushboolean(L, (*spinfo)->jump());
    return 1;
}

LUAAPI(simplayer_useItem) {
    LUA_ARG_COUNT_CHECK_M(1)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    int delay = static_cast<int>(luaL_checkinteger(L, 2));
    lua_settop(L, 0);
    (*spinfo)->useItem(delay);
    return 0;
}

LUAAPI(simplayer_startBuild) {
    LUA_ARG_COUNT_CHECK_M(0)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    lua_settop(L, 0);
    (*spinfo)->startBuild();
    return 0;
}

LUAAPI(simplayer_lookAt) {
    LUA_ARG_COUNT_CHECK_M(1)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    Vec3* pos = (Vec3*)luaL_checkudata(L, 2, "vec3_mt");
    luaL_argcheck(L, pos != nullptr, 2, "invalid userdata");
    lua_settop(L, 0);
    (*spinfo)->lookAt(*pos);
    return 0;
}

LUAAPI(simplayer_moveTo) {
    LUA_ARG_COUNT_CHECK_M(1)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    Vec3* pos = (Vec3*)luaL_checkudata(L, 2, "vec3_mt");
    luaL_argcheck(L, pos != nullptr, 2, "invalid userdata");
    lua_settop(L, 0);
    (*spinfo)->moveTo(*pos);
    return 0;
}

LUAAPI(simplayer_navigateTo) {
    LUA_ARG_COUNT_CHECK_M(1)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    Vec3* pos = (Vec3*)luaL_checkudata(L, 2, "vec3_mt");
    luaL_argcheck(L, pos != nullptr, 2, "invalid userdata");
    lua_settop(L, 0);
    (*spinfo)->navigateTo(*pos);
    return 0;
}

LUAAPI(simplayer_isTaskFree) {
    LUA_ARG_COUNT_CHECK_M(0)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    lua_settop(L, 0);
    lua_pushboolean(L, (*spinfo)->isTaskFree());
    return 1;
}

LUAAPI(simplayer_stopAction) {
    LUA_ARG_COUNT_CHECK_M(0)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(
        L,
        (spinfo != nullptr) && ((*spinfo) != nullptr) && ((*spinfo)->simPlayer != nullptr),
        1,
        "invalid userdata"
    );
    lua_settop(L, 0);
    (*spinfo)->stopAction();
    return 0;
}

LUAAPI(simplayer_meta_gc) {
    LUA_ARG_COUNT_CHECK_M(0)
    SimPlayerManager::SimPlayerInfo** spinfo = (SimPlayerManager::SimPlayerInfo**)luaL_checkudata(L, 1, "simplayer_mt");
    luaL_argcheck(L, (spinfo != nullptr) && ((*spinfo) != nullptr), 1, "invalid userdata");
    lua_settop(L, 0);
    delete *spinfo;
    return 0;
}

const static luaL_Reg lua_reg_simplayer_m[] = {
    {"getName",             lua_api_simplayer_getName            },
    {"getXuid",             lua_api_simplayer_getXuid            },
    {"getStatus",           lua_api_simplayer_getStatus          },
    {"getPos",              lua_api_simplayer_getPos             },
    {"getFeetPos",          lua_api_simplayer_getFeetPos         },
    {"getStandingOn",       lua_api_simplayer_getStandingOn      },
    {"getRot",              lua_api_simplayer_getRot             },
    {"getHealth",           lua_api_simplayer_getHealth          },
    {"getHunger",           lua_api_simplayer_getHunger          },
    {"sneaking",            lua_api_simplayer_sneaking           },
    {"swimming",            lua_api_simplayer_swimming           },
    {"attack",              lua_api_simplayer_attack             },
    {"chat",                lua_api_simplayer_chat               },
    {"destroy",             lua_api_simplayer_destroy            },
    {"dropSelectedItem",    lua_api_simplayer_dropSelectedItem   },
    {"dropInv",             lua_api_simplayer_dropInv            },
    {"runCmd",              lua_api_simplayer_runCmd             },
    {"getBlockPosFromView", lua_api_simplayer_getBlockPosFromView},
    {"searchInInvWithId",   lua_api_simplayer_searchInInvWithId  },
    {"searchInInvWithName", lua_api_simplayer_searchInInvWithName},
    {"selectSlot",          lua_api_simplayer_selectSlot         },
    {"select",              lua_api_simplayer_select             },
    {"getItemFromInv",      lua_api_simplayer_getItemFromInv     },
    {"interact",            lua_api_simplayer_interact           },
    {"jump",                lua_api_simplayer_jump               },
    {"useItem",             lua_api_simplayer_useItem            },
    {"startBuild",          lua_api_simplayer_startBuild         },
    {"lookAt",              lua_api_simplayer_lookAt             },
    {"moveTo",              lua_api_simplayer_moveTo             },
    {"navigateTo",          lua_api_simplayer_navigateTo         },
    {"isTaskFree",          lua_api_simplayer_isTaskFree         },
    {"stopAction",          lua_api_simplayer_stopAction         },
    {"__gc",                lua_api_simplayer_meta_gc            },
    {NULL,                  NULL                                 }
};

LUAAPI(open_simplayer) {
    luaL_newmetatable(L, "simplayer_mt");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, lua_reg_simplayer_m, 0);
    luaL_newlib(L, lua_reg_null_c);
    return 1;
}

// userdata simplayer end

static const luaL_Reg lua_libs[] = {
    {"vec3",        lua_api_open_vec3       },
    {"blockpos",    lua_api_open_blockpos   },
    {"vec2",        lua_api_open_vec2       },
    {"blockinfo",   lua_api_open_blockinfo  },
    {"blocksource", lua_api_open_blocksource},
    {"level",       lua_api_open_level      },
    {"iteminfo",    lua_api_open_iteminfo   },
    {"simplayer",   lua_api_open_simplayer  },
    {NULL,          NULL                    }
};

} // namespace

inline void broadcastErr(std::filesystem::path const& path, std::string const& name, std::string const& msg) {
    std::string data = std::format("[CFSP-Lua] ({}@{})\n{}", path.string(), name, msg);
    TextPacket::createRawMessage(data).sendToClients();
    coral_fans::CoralFans::getInstance().getSelf().getLogger().error(data);
}

std::pair<std::string, bool>
execLuaScript(std::string const& fileName, int interval, SimPlayerManager::SimPlayerInfo& spinfo) {
    auto  path = coral_fans::CoralFans::getInstance().getSelf().getDataDir() / "simplayer" / "scripts" / fileName;
    auto& mod  = coral_fans::mod();
    // new state
    std::shared_ptr<lua_State> L(luaL_newstate(), [](lua_State* L) {
        if (L) lua_close(L);
    });
    if (!L.get()) return {"Cannot open lua state", false};
    // open libs
    luaL_openlibs(L.get());
    // load log function
    lua_register(L.get(), "log", lua_api_log);
    // do preload
    int ret = luaL_dostring(L.get(), mod.getConfig().simPlayer.luaPreload.c_str());
    if (ret != LUA_OK) return {lua_tostring(L.get(), -1), false};
    lua_settop(L.get(), 0);
    // load built-in types and vars
    const luaL_Reg* libs = lua_libs;
    for (; libs->func; ++libs) {
        luaL_requiref(L.get(), libs->name, libs->func, 1);
        lua_pop(L.get(), 1);
    }
    // load BlockSource
    int* dimid = (int*)lua_newuserdata(L.get(), sizeof(int));
    *dimid     = (int)spinfo.simPlayer->getDimensionId();
    luaL_setmetatable(L.get(), "blocksource_mt");
    lua_setglobal(L.get(), "BlockSource");
    // load Level
    lua_newuserdata(L.get(), sizeof(void*));
    luaL_setmetatable(L.get(), "level_mt");
    lua_setglobal(L.get(), "Level");
    // load SimPlayer
    SimPlayerManager::SimPlayerInfo** luaspinfo =
        (SimPlayerManager::SimPlayerInfo**)lua_newuserdata(L.get(), sizeof(SimPlayerManager::SimPlayerInfo*));
    *luaspinfo = new SimPlayerManager::SimPlayerInfo(spinfo);
    luaL_setmetatable(L.get(), "simplayer_mt");
    lua_setglobal(L.get(), "SimPlayer");
    // set require path
    int ty = lua_getglobal(L.get(), "package");
    if (ty != LUA_TTABLE) return {"\"package\" is not a table", false};
    lua_getfield(L.get(), -1, "path");
    std::string pkgPath = lua_tostring(L.get(), -1);
    lua_pop(L.get(), 1);
    pkgPath += std::format(";{0}\\?.lua;{0}\\?\\init.lua", std::filesystem::absolute(path).parent_path().string());
    lua_pushstring(L.get(), pkgPath.c_str());
    lua_setfield(L.get(), -2, "path");
    lua_getfield(L.get(), -1, "cpath");
    std::string pkgCPath = lua_tostring(L.get(), -1);
    lua_pop(L.get(), 1);
    pkgCPath += std::format(";{0}\\?.dll;{0}\\?\\init.dll", std::filesystem::absolute(path).parent_path().string());
    lua_pushstring(L.get(), pkgCPath.c_str());
    lua_setfield(L.get(), -2, "cpath");
    lua_setglobal(L.get(), "package");
    // load and do file
    ret = luaL_dofile(L.get(), path.string().c_str());
    if (ret != LUA_OK) return {lua_tostring(L.get(), -1), false};
    lua_settop(L.get(), 0);
    // exec Init func
    int fType = lua_getglobal(L.get(), "Init");
    if (fType != LUA_TFUNCTION) return {"\"Init\" is not a function", false};
    ret = lua_pcall(L.get(), 0, 1, 0);
    if (ret != LUA_OK) return {lua_tostring(L.get(), -1), false};
    // get Init func return
    if (lua_isboolean(L.get(), -1)) {
        if (!lua_toboolean(L.get(), -1)) return {"\"Init\" function returned false. break", true}; // exit
    } else return {"function \"Init\" did not return a boolean type value", false};
    lua_settop(L.get(), 0);
    // run Tick func once
    fType = lua_getglobal(L.get(), "Tick");
    if (fType != LUA_TFUNCTION) return {"\"Tick\" is not a function", false};
    ret = lua_pcall(L.get(), 0, 1, 0);
    if (ret != LUA_OK) return {lua_tostring(L.get(), -1), false};
    if (lua_isboolean(L.get(), -1)) {
        if (!lua_toboolean(L.get(), -1)) return {"\"Tick\" function returned false. break", true}; // exit
    } else return {"function \"Tick\" did not return a boolean type value", false};
    lua_settop(L.get(), 0);
    // run Tick in scheduler
    spinfo.scriptid = spinfo.scheduler->add(interval, [L, path, spinfo](unsigned long long) {
        lua_settop(L.get(), 0);
        int fType = lua_getglobal(L.get(), "Tick");
        if (fType != LUA_TFUNCTION) {
            broadcastErr(path, spinfo.name, "\"Tick\" is not a function");
            return false;
        }
        int ret = lua_pcall(L.get(), 0, 1, 0);
        if (ret != LUA_OK) {
            broadcastErr(path, spinfo.name, lua_tostring(L.get(), -1));
            return false;
        }
        if (lua_isboolean(L.get(), -1)) return (bool)lua_toboolean(L.get(), -1);
        else {
            broadcastErr(path, spinfo.name, "function \"Tick\" did not return a boolean type value");
            return false;
        }
    });
    // return
    return {"Success", true};
}

} // namespace sputils::lua_api

} // namespace coral_fans::functions