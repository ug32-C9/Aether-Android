#include "../environment.hpp"
#include <lua.h>
#include <lualib.h>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static int http_get(lua_State* ls)
{
    std::string url = luaL_checkstring(ls, 1);
    
    // We should probably do this in a separate thread to avoid blocking the game thread,
    // but for now let's keep it simple as requested for "leftovers".
    auto response = cpr::Get(cpr::Url{url});
    
    if (response.status_code == 200)
    {
        lua_pushlstring(ls, response.text.c_str(), response.text.size());
    }
    else
    {
        lua_pushnil(ls);
    }
    return 1;
}

static int http_request(lua_State* ls)
{
    luaL_checktype(ls, 1, LUA_TTABLE);
    
    lua_getfield(ls, 1, "Url");
    std::string url = luaL_checkstring(ls, -1);
    lua_pop(ls, 1);
    
    lua_getfield(ls, 1, "Method");
    std::string method = luaL_optstring(ls, -1, "GET");
    lua_pop(ls, 1);
    
    cpr::Header headers;
    lua_getfield(ls, 1, "Headers");
    if (lua_istable(ls, -1))
    {
        lua_pushnil(ls);
        while (lua_next(ls, -2))
        {
            headers[luaL_checkstring(ls, -2)] = luaL_checkstring(ls, -1);
            lua_pop(ls, 1);
        }
    }
    lua_pop(ls, 1);
    
    lua_getfield(ls, 1, "Body");
    std::string body = luaL_optstring(ls, -1, "");
    lua_pop(ls, 1);
    
    cpr::Response response;
    if (method == "GET") response = cpr::Get(cpr::Url{url}, headers);
    else if (method == "POST") response = cpr::Post(cpr::Url{url}, headers, cpr::Body{body});
    else if (method == "PUT") response = cpr::Put(cpr::Url{url}, headers, cpr::Body{body});
    else if (method == "DELETE") response = cpr::Delete(cpr::Url{url}, headers, cpr::Body{body});
    else response = cpr::Get(cpr::Url{url}, headers);
    
    lua_newtable(ls);
    lua_pushstring(ls, response.text.c_str());
    lua_setfield(ls, -2, "Body");
    
    lua_pushinteger(ls, response.status_code);
    lua_setfield(ls, -2, "StatusCode");
    
    lua_pushstring(ls, response.status_line.c_str());
    lua_setfield(ls, -2, "StatusMessage");
    
    lua_newtable(ls);
    for (auto& header : response.header)
    {
        lua_pushstring(ls, header.second.c_str());
        lua_setfield(ls, -2, header.first.c_str());
    }
    lua_setfield(ls, -2, "Headers");
    
    lua_pushboolean(ls, response.status_code >= 200 && response.status_code < 300);
    lua_setfield(ls, -2, "Success");
    
    return 1;
}

static int get_custom_asset(lua_State* ls)
{
    std::string path = luaL_checkstring(ls, 1);
    // On Android, this usually involves prefixing with a special scheme or path
    lua_pushstring(ls, ("rbxasset://../Aether/workspace/" + path).c_str());
    return 1;
}

static const luaL_Reg funcs[] = {
    {"request", http_request},
    {"HttpGet", http_get},
    {"getcustomasset", get_custom_asset},
    {nullptr, nullptr}
};

auto exploit::environment::http(lua_State* ls) -> void
{
    lua_pushvalue(ls, LUA_GLOBALSINDEX);
    register_lib(ls, "http", funcs);
    
    // Also register HttpGet as a global
    lua_pushcfunction(ls, http_get, "HttpGet");
    lua_setfield(ls, -2, "HttpGet");
    
    lua_pop(ls, 1);
}
