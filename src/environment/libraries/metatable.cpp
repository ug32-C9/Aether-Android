#include "../environment.hpp"
#include <lua.h>
#include <lualib.h>
#include <lapi.h>
#include <lobject.h>
#include <ltable.h>
#include "../../callcheck/callcheck.hpp"

// Functional metatable UNC
static int getrawmetatable(lua_State* ls)
{
    luaL_checkany(ls, 1);
    
    if (!lua_getmetatable(ls, 1))
        lua_pushnil(ls);
        
    return 1;
}

static int setrawmetatable(lua_State* ls)
{
    luaL_checkany(ls, 1);
    int t = lua_type(ls, 2);
    luaL_argcheck(ls, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table expected");
    
    lua_settop(ls, 2);
    lua_setmetatable(ls, 1);
    return 0;
}

static int hookmetamethod(lua_State* ls)
{
    luaL_checkany(ls, 1);
    const char* method = luaL_checkstring(ls, 2);
    luaL_checktype(ls, 3, LUA_TFUNCTION);
    
    if (!lua_getmetatable(ls, 1))
        luaL_error(ls, "Object has no metatable");
    
    lua_getfield(ls, -1, method);
    if (lua_isnoneornil(ls, -1))
    {
        lua_pop(ls, 2);
        luaL_error(ls, "Proxy does not have metamethod");
    }
    
    // Switch the metamethod
    lua_pushvalue(ls, 2); // copy method name
    lua_pushvalue(ls, 3); // copy new func
    lua_settable(ls, -4); // metatable[method] = new_func
    
    // Return original metamethod which is at -1
    return 1;
}

static int getnamecallmethod(lua_State* ls)
{
    // Luau internal namecall method
    if (ls->namecall)
    {
        lua_pushstring(ls, ls->namecall->data);
        return 1;
    }
    return 0;
}

static int setnamecallmethod(lua_State* ls)
{
    const char* method = luaL_checkstring(ls, 1);
    // This is a bit unsafe without proper string atom handling, but for now:
    // Actually we should just use lua_tostringatom potentially or similar
    // For now we just mock or return error if not easily settable
    lua_pushboolean(ls, false); 
    return 1;
}

static int setreadonly(lua_State* ls)
{
    luaL_checkany(ls, 1);
    bool readonly = luaL_checkboolean(ls, 2);
    lua_setreadonly(ls, 1, readonly);
    return 0;
}

static int isreadonly(lua_State* ls)
{
    luaL_checkany(ls, 1);
    lua_pushboolean(ls, lua_getreadonly(ls, 1));
    return 1;
}

static const luaL_Reg mt_funcs[] = {
    {"getrawmetatable", getrawmetatable},
    {"setrawmetatable", setrawmetatable},
    {"hookmetamethod", hookmetamethod},
    {"getnamecallmethod", getnamecallmethod},
    {"setnamecallmethod", setnamecallmethod},
    {"setreadonly", setreadonly},
    {"make_readonly", setreadonly},
    {"make_writeable", setreadonly}, // user can pass false
    {"isreadonly", isreadonly},
    {nullptr, nullptr}
};

auto exploit::environment::metatable(lua_State* ls) -> void
{
    lua_pushvalue(ls, LUA_GLOBALSINDEX);
    register_lib(ls, nullptr, mt_funcs);
    lua_pop(ls, 1);
}
