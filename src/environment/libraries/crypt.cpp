#include "../environment.hpp"
#include <lua.h>
#include <lualib.h>
#include <string>
#include "../../utils/utils.hpp"
#include "../../dependencies/base64.hpp"

static int crypt_base64_encode(lua_State* ls)
{
    size_t len;
    const char* data = luaL_checklstring(ls, 1, &len);
    std::string encoded = base64::encode(reinterpret_cast<const unsigned char*>(data), len);
    lua_pushlstring(ls, encoded.c_str(), encoded.size());
    return 1;
}

static int crypt_base64_decode(lua_State* ls)
{
    std::string data = luaL_checkstring(ls, 1);
    std::string decoded = base64::decode(data);
    lua_pushlstring(ls, decoded.c_str(), decoded.size());
    return 1;
}

static int crypt_lz4compress(lua_State* ls)
{
    size_t len;
    const char* data = luaL_checklstring(ls, 1, &len);
    std::string compressed = utils::lz4::compress(std::string(data, len));
    lua_pushlstring(ls, compressed.c_str(), compressed.size());
    return 1;
}

static int crypt_lz4decompress(lua_State* ls)
{
    size_t len;
    const char* data = luaL_checklstring(ls, 1, &len);
    int size = luaL_checkinteger(ls, 2);
    std::string decompressed = utils::lz4::decompress(std::string(data, len), size);
    lua_pushlstring(ls, decompressed.c_str(), decompressed.size());
    return 1;
}

static int crypt_hash(lua_State* ls)
{
    size_t len;
    const char* data = luaL_checklstring(ls, 1, &len);
    // Simple FNV-1a hash for demonstration if real algorithms aren't available
    uint64_t hash = 0xcbf29ce484222325;
    for (size_t i = 0; i < len; ++i) {
        hash ^= (uint8_t)data[i];
        hash *= 0x100000001b3;
    }
    char buf[17];
    sprintf(buf, "%016llx", hash);
    lua_pushstring(ls, buf);
    return 1;
}

static const luaL_Reg crypt_funcs[] = {
    {"base64_encode", crypt_base64_encode},
    {"base64_decode", crypt_base64_decode},
    {"lz4compress", crypt_lz4compress},
    {"lz4decompress", crypt_lz4decompress},
    {"hash", crypt_hash},
    {nullptr, nullptr}
};

auto exploit::environment::crypt(lua_State* ls) -> void
{
    lua_pushvalue(ls, LUA_GLOBALSINDEX);
    register_lib(ls, "crypt", crypt_funcs);
    
    // Aliases
    lua_getglobal(ls, "crypt");
    lua_getfield(ls, -1, "base64_encode");
    lua_setfield(ls, LUA_GLOBALSINDEX, "base64_encode");
    lua_getfield(ls, -2, "base64_decode");
    lua_setfield(ls, LUA_GLOBALSINDEX, "base64_decode");
    
    lua_pop(ls, 1);
}
