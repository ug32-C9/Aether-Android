#include "../environment.hpp"
#include <lua.h>
#include <lualib.h>
#include <string>
#include <vector>
#include "../../utils/utils.hpp"

// Simple Drawing API stub (functional but doesn't render yet as renderer is empty)
static int drawing_new(lua_State* ls)
{
    const char* type = luaL_checkstring(ls, 1);
    lua_newtable(ls);
    lua_pushstring(ls, type);
    lua_setfield(ls, -2, "Type");
    lua_pushboolean(ls, true);
    lua_setfield(ls, -2, "Visible");
    lua_pushnumber(ls, 1.0);
    lua_setfield(ls, -2, "Transparency");
    lua_pushinteger(ls, 0xFFFFFF);
    lua_setfield(ls, -2, "Color");
    
    if (strcmp(type, "Line") == 0) {
        lua_newtable(ls); lua_setfield(ls, -2, "From");
        lua_newtable(ls); lua_setfield(ls, -2, "To");
        lua_pushnumber(ls, 1.0); lua_setfield(ls, -2, "Thickness");
    }
    else if (strcmp(type, "Circle") == 0) {
        lua_newtable(ls); lua_setfield(ls, -2, "Position");
        lua_pushnumber(ls, 0.0); lua_setfield(ls, -2, "Radius");
        lua_pushnumber(ls, 16.0); lua_setfield(ls, -2, "NumSides");
    }
    else if (strcmp(type, "Square") == 0) {
        lua_newtable(ls); lua_setfield(ls, -2, "Position");
        lua_newtable(ls); lua_setfield(ls, -2, "Size");
    }
    else if (strcmp(type, "Image") == 0) {
        lua_pushstring(ls, ""); lua_setfield(ls, -2, "Data");
        lua_newtable(ls, 0); lua_setfield(ls, -2, "Position");
        lua_newtable(ls, 0); lua_setfield(ls, -2, "Size");
    }
    
    return 1;
}

static int drawing_clear(lua_State* ls)
{
    // Clear all drawing objects logic
    return 0;
}

static const luaL_Reg drawing_funcs[] = {
    {"new", drawing_new},
    {"clear", drawing_clear},
    {nullptr, nullptr}
};

auto exploit::environment::drawing(lua_State* ls) -> void
{
    lua_pushvalue(ls, LUA_GLOBALSINDEX);
    register_lib(ls, "Drawing", drawing_funcs);
    
    // Add Fonts table
    lua_getglobal(ls, "Drawing");
    lua_newtable(ls);
    lua_pushinteger(ls, 0); lua_setfield(ls, -2, "UI");
    lua_pushinteger(ls, 1); lua_setfield(ls, -2, "System");
    lua_pushinteger(ls, 2); lua_setfield(ls, -2, "Plex");
    lua_pushinteger(ls, 3); lua_setfield(ls, -2, "Monospace");
    lua_setfield(ls, -2, "Fonts");
    
    lua_pop(ls, 1);
}
