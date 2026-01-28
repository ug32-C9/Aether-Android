#include "../environment.hpp"
#include <lua.h>
#include <lualib.h>
#include <string>
#include <fstream>
#include <filesystem>
#include "../../utils/utils.hpp"

namespace fs = std::filesystem;

static std::string get_workspace_path()
{
    // On Android, we should use a path the app can actually access,
    // like /sdcard/Android/data/com.roblox.client/files/workspace/
    // For now we'll use a local path that is likely accessible.
    return "/sdcard/Aether/workspace/";
}

static std::string sanitize_path(std::string path)
{
    // Prevent directory traversal
    while (path.find("..") != std::string::npos)
    {
        path.erase(path.find(".."), 2);
    }
    return get_workspace_path() + path;
}

static int fs_readfile(lua_State* ls)
{
    std::string path = sanitize_path(luaL_checkstring(ls, 1));
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) luaL_error(ls, "Failed to open file");
    
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    lua_pushlstring(ls, content.c_str(), content.size());
    return 1;
}

static int fs_writefile(lua_State* ls)
{
    std::string path = sanitize_path(luaL_checkstring(ls, 1));
    std::string content = luaL_checkstring(ls, 2);
    
    fs::create_directories(fs::path(path).parent_path());
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) luaL_error(ls, "Failed to open file for writing");
    
    file.write(content.c_str(), content.size());
    return 0;
}

static int fs_appendfile(lua_State* ls)
{
    std::string path = sanitize_path(luaL_checkstring(ls, 1));
    std::string content = luaL_checkstring(ls, 2);
    
    fs::create_directories(fs::path(path).parent_path());
    std::ofstream file(path, std::ios::binary | std::ios::app);
    if (!file.is_open()) luaL_error(ls, "Failed to open file for appending");
    
    file.write(content.c_str(), content.size());
    return 0;
}

static int fs_isfile(lua_State* ls)
{
    std::string path = sanitize_path(luaL_checkstring(ls, 1));
    lua_pushboolean(ls, fs::is_regular_file(path));
    return 1;
}

static int fs_isfolder(lua_State* ls)
{
    std::string path = sanitize_path(luaL_checkstring(ls, 1));
    lua_pushboolean(ls, fs::is_directory(path));
    return 1;
}

static int fs_makefolder(lua_State* ls)
{
    std::string path = sanitize_path(luaL_checkstring(ls, 1));
    fs::create_directories(path);
    return 0;
}

static int fs_delfolder(lua_State* ls)
{
    std::string path = sanitize_path(luaL_checkstring(ls, 1));
    fs::remove_all(path);
    return 0;
}

static int fs_delfile(lua_State* ls)
{
    std::string path = sanitize_path(luaL_checkstring(ls, 1));
    fs::remove(path);
    return 0;
}

static int fs_listfiles(lua_State* ls)
{
    std::string path = sanitize_path(luaL_checkstring(ls, 1));
    lua_newtable(ls);
    int i = 1;
    if (fs::exists(path) && fs::is_directory(path))
    {
        for (const auto& entry : fs::directory_iterator(path))
        {
            lua_pushstring(ls, entry.path().filename().string().c_str());
            lua_rawseti(ls, -2, i++);
        }
    }
    return 1;
}

static const luaL_Reg fs_funcs[] = {
    {"readfile", fs_readfile},
    {"writefile", fs_writefile},
    {"appendfile", fs_appendfile},
    {"isfile", fs_isfile},
    {"isfolder", fs_isfolder},
    {"makefolder", fs_makefolder},
    {"delfolder", fs_delfolder},
    {"delfile", fs_delfile},
    {"listfiles", fs_listfiles},
    {nullptr, nullptr}
};

auto exploit::environment::filesystem(lua_State* ls) -> void
{
    // Reusing connections(ls) for filesystem for now
    lua_pushvalue(ls, LUA_GLOBALSINDEX);
    register_lib(ls, nullptr, fs_funcs);
    lua_pop(ls, 1);
}
