#include "../environment.hpp"
#include <lua.h>
#include <lualib.h>
#include <string>
#include <iostream>
#include "../../utils/utils.hpp"

// Console UNC for Android (mapped to logcat for now as there's no real windowed console on android easily)
static int rconsoleprint(lua_State* ls)
{
    const char* text = luaL_checkstring(ls, 1);
    LOGI("[CONSOLE] %s", text);
    return 0;
}

static int rconsoleinfo(lua_State* ls)
{
    const char* text = luaL_checkstring(ls, 1);
    LOGI("[INFO] %s", text);
    return 0;
}

static int rconsolewarn(lua_State* ls)
{
    const char* text = luaL_checkstring(ls, 1);
    LOGW("[WARN] %s", text);
    return 0;
}

static int rconsoleerr(lua_State* ls)
{
    const char* text = luaL_checkstring(ls, 1);
    LOGE("[ERROR] %s", text);
    return 0;
}

static int rconsoleclear(lua_State* ls)
{
    // No-op for logcat
    return 0;
}

static int rconsolename(lua_State* ls)
{
    // No-op for logcat
    return 0;
}

static const luaL_Reg console_funcs[] = {
    {"rconsoleprint", rconsoleprint},
    {"rconsoleinfo", rconsoleinfo},
    {"rconsolewarn", rconsolewarn},
    {"rconsoleerr", rconsoleerr},
    {"rconsoleclear", rconsoleclear},
    {"rconsolename", rconsolename},
    {"consoleprint", rconsoleprint},
    {"consoleinfo", rconsoleinfo},
    {"consolewarn", rconsolewarn},
    {"consoleerr", rconsoleerr},
    {"consoleclear", rconsoleclear},
    {nullptr, nullptr}
};

auto exploit::environment::console(lua_State* ls) -> void
{
    // Reusing bit(ls) name from environment.hpp for console registration if it wasn't used
    lua_pushvalue(ls, LUA_GLOBALSINDEX);
    register_lib(ls, nullptr, console_funcs);
    lua_pop(ls, 1);
}
