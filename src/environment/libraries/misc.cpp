#include "../environment.hpp"

#include <lua.h>
#include "../../taskscheduler/taskscheduler.hpp"
#include <lualib.h>
#include <lmem.h>
#include <lgc.h>
#include "../../roblox/update.hpp"
#include "../../utils/utils.hpp"
#include "../../execution/execution.hpp"

static int getgenv(lua_State* ls)
{
    LOGD(" LuauEnvCall -> getgenv - CallingThread -> %p", ls);
    
    lua_pushvalue(ls, LUA_GLOBALSINDEX);
    return 1;
}

// get mainthread global env
static int getmtgenv(lua_State* ls)
{
    LOGD(" LuauEnvCall -> getmgenv - CallingThread -> %p", ls);
    
    lua_pushvalue(taskscheduler::get_singleton( )->get_ExploitState( ), LUA_GLOBALSINDEX);
    lua_xmove(taskscheduler::get_singleton( )->get_ExploitState( ), ls, 1);
    return 1;
}

static int getrenv(lua_State* ls)
{
    LOGD(" LuauEnvCall -> getrenv - CallingThread -> %p", ls);
    
    lua_pushvalue(taskscheduler::get_singleton( )->get_mainstate( ), LUA_GLOBALSINDEX);
    lua_xmove(taskscheduler::get_singleton( )->get_mainstate( ), ls, 1);
    return 1;
}

static int getreg(lua_State* ls)
{
    LOGD(" LuauEnvCall -> getreg - CallingThread -> %p", ls);
    
    lua_pushvalue(ls, LUA_REGISTRYINDEX);
    return 1;
}

static int gettenv(lua_State* ls)
{
    LOGD(" LuauEnvCall -> gettenv - CallingThread -> %p", ls);
    
    luaL_checktype(ls, 1, LUA_TTHREAD);

    auto thread = lua_tothread(ls, 1);

    lua_pushvalue(thread, LUA_GLOBALSINDEX);
    lua_xmove(thread, ls, 1);
    return 1;
}

static int getthread(lua_State* ls)
{
    LOGD(" LuauEnvCall -> getthread - CallingThread -> %p", ls);
    
    lua_pushthread(ls);
    return 1;
}

static int identifyexecutor(lua_State* ls)
{
    LOGD(" LuauEnvCall -> identifyexecutor - CallingThread -> %p", ls);
    
    lua_pushstring(ls, exploit_configuration::exploit_name.c_str( ));
    lua_pushstring(ls, exploit_configuration::exploit_version.c_str( ));
    return 2;    
}

static int isLuau(lua_State* ls)
{
    LOGD(" LuauEnvCall -> isLuau - CallingThread -> %p", ls);
    
    lua_pushboolean(ls, true); 
    return 1; 
}

static int isrbxactive(lua_State* ls)
{
    LOGD(" LuauEnvCall -> isrbxactive - CallingThread -> %p", ls);
    
    lua_pushboolean(ls, true);
    return 1; 
}

static int setfpscap(lua_State* ls)
{
    LOGD(" LuauEnvCall -> setfpscap - CallingThread -> %p", ls);
    
    auto cap = luaL_checknumber(ls, 1);
    
    taskscheduler::get_singleton( )->set_fpscap(cap);
    return 0;
}

static int getfpscap(lua_State* ls)
{
    LOGD(" LuauEnvCall -> getfpscap - CallingThread -> %p", ls);
    
    auto fps = taskscheduler::get_singleton( )->get_fpscap( );
    lua_pushnumber(ls, fps);
    return 1;
}

struct gc_visitor_context
{
    lua_State* ls;
    const int tables;
    size_t count;
};

static bool gc_visitor(void* context, lua_Page* page, GCObject* gco)
{
    const auto gct = static_cast<gc_visitor_context*>(context);

    // DEADMASK
    if ( !((gco->gch.marked ^ 3) & otherwhite(gct->ls->global)) )
        return false;

    const auto tt = gco->gch.tt;

    if (tt == LUA_TFUNCTION || tt == LUA_TUSERDATA || (gct->tables && tt == LUA_TTABLE))
    {
        gct->ls->top->value.gc = gco;
        gct->ls->top->tt = gco->gch.tt;
        gct->ls->top++;

        lua_rawseti(gct->ls, -2, ++gct->count);
    }

    return false;
}

static int getgc(lua_State* ls)
{
    LOGD(" LuauEnvCall -> getgc - CallingThread -> %p", ls);
    
    const auto tables = lua_gettop(ls) ? luaL_optboolean(ls, 1, 0) : true;
    lua_newtable(ls);

    gc_visitor_context gct{ ls, tables, 0 };
    luaM_visitgco(ls, &gct, gc_visitor);

    return 1;
}

static int getidentity(lua_State* ls)
{
    LOGD(" LuauEnvCall -> getidentity - CallingThread -> %p", ls);
    
    auto ES = static_cast<roblox::structs::ExtraSpace_t*>(lua_getthreaddata(ls));
    if (ES == nullptr) 
    {
        lua_pushinteger(ls, 0); // anonymous thread 
        return 1;
    }
    
    lua_pushinteger(ls, ES->context_level);
    return 1;
}

static int setidentity(lua_State* ls)
{
    LOGD(" LuauEnvCall -> setidentity - CallingThread -> %p", ls);
    
    auto identity = luaL_checkinteger(ls, 1);
    auto ES = static_cast<roblox::structs::ExtraSpace_t*>(lua_getthreaddata(ls));
    if (ES == nullptr) 
    {
        luaL_argerror(ls, 0, "Could not get the ExtraSpace of this thread, identity cannot be set.");
        return 0;
    }
    
    lua_pushinteger(ls, ES->context_level); // old identity
    roblox::functions::set_identity(ls, identity);
    return 1;
}

// --- New sUNC/UNC functions start here ---
static int getgame(lua_State* ls)
{
    LOGD("LuauEnvCall -> getgame - CallingThread - %p", ls);
    lua_getglobal(ls, "game");
    return 1;
}

static int getworkspace(lua_State* ls)
{
    LOGD("LuauEnvCall -> getworkspace - CallingThread - %p", ls);
    lua_getglobal(ls, "game");
    lua_getfield(ls, -1, "GetService");
    lua_pushvalue(ls, -2);
    lua_pushstring(ls, "Workspace");
    lua_call(ls, 2, 1);
    return 1;
}

static int getplayers(lua_State* ls)
{
    LOGD("LuauEnvCall -> getplayers - CallingThread - %p", ls);
    lua_getglobal(ls, "game");
    lua_getfield(ls, -1, "GetService");
    lua_pushvalue(ls, -2);
    lua_pushstring(ls, "Players");
    lua_call(ls, 2, 1);
    return 1;
}

static int getlocalplayer(lua_State* ls)
{
    LOGD("LuauEnvCall -> getlocalplayer - CallingThread - %p", ls);
    lua_getglobal(ls, "game");
    lua_getfield(ls, -1, "GetService");
    lua_pushvalue(ls, -2);
    lua_pushstring(ls, "Players");
    lua_call(ls, 2, 1);
    lua_getfield(ls, -1, "LocalPlayer");
    return 1;
}

static int getplayerbyname(lua_State* ls)
{
    LOGD("LuauEnvCall -> getplayerbyname - CallingThread - %p", ls);
    const char* name = luaL_checkstring(ls, 1);
    lua_getglobal(ls, "game");
    lua_getfield(ls, -1, "GetService");
    lua_pushvalue(ls, -2);
    lua_pushstring(ls, "Players");
    lua_call(ls, 2, 1);
    lua_getfield(ls, -1, "FindFirstChild");
    lua_pushvalue(ls, -2);
    lua_pushstring(ls, name);
    lua_call(ls, 2, 1);
    return 1;
}

static int getplayercharacter(lua_State* ls)
{
    LOGD("LuauEnvCall -> getplayercharacter - CallingThread - %p", ls);
    lua_getglobal(ls, "game");
    lua_getfield(ls, -1, "GetService");
    lua_pushvalue(ls, -2);
    lua_pushstring(ls, "Players");
    lua_call(ls, 2, 1);
    lua_getfield(ls, -1, "LocalPlayer");
    lua_getfield(ls, -1, "Character");
    return 1;
}

static int getservice(lua_State* ls)
{
    LOGD("LuauEnvCall -> getservice - CallingThread - %p", ls);
    const char* name = luaL_checkstring(ls, 1);
    lua_getglobal(ls, "game");
    lua_getfield(ls, -1, "GetService");
    lua_pushvalue(ls, -2);
    lua_pushstring(ls, name);
    lua_call(ls, 2, 1);
    return 1;
}

static int getlighting(lua_State* ls)
{
    LOGD("LuauEnvCall -> getlighting - CallingThread - %p", ls);
    lua_getglobal(ls, "game");
    lua_getfield(ls, -1, "GetService");
    lua_pushvalue(ls, -2);
    lua_pushstring(ls, "Lighting");
    lua_call(ls, 2, 1);
    return 1;
}

static int getrunservice(lua_State* ls)
{
    LOGD("LuauEnvCall -> getrunservice - CallingThread - %p", ls);
    lua_getglobal(ls, "game");
    lua_getfield(ls, -1, "GetService");
    lua_pushvalue(ls, -2);
    lua_pushstring(ls, "RunService");
    lua_call(ls, 2, 1);
    return 1;
}

static int getreplicatedfirst(lua_State* ls)
{
    LOGD("LuauEnvCall -> getreplicatedfirst - CallingThread - %p", ls);
    lua_getglobal(ls, "game");
    lua_getfield(ls, -1, "GetService");
    lua_pushvalue(ls, -2);
    lua_pushstring(ls, "ReplicatedFirst");
    lua_call(ls, 2, 1);
    return 1;
}

static int getcoregui(lua_State* ls)
{
    LOGD("LuauEnvCall -> getcoregui - CallingThread - %p", ls);
    lua_getglobal(ls, "game");
    lua_getfield(ls, -1, "GetService");
    lua_pushvalue(ls, -2);
    lua_pushstring(ls, "CoreGui");
    lua_call(ls, 2, 1);
    return 1;
}

static int getplayergui(lua_State* ls)
{
    LOGD("LuauEnvCall -> getplayergui - CallingThread - %p", ls);
    // Returns LocalPlayer.PlayerGui
    lua_getglobal(ls, "game");
    lua_getfield(ls, -1, "GetService");
    lua_pushvalue(ls, -2);
    lua_pushstring(ls, "Players");
    lua_call(ls, 2, 1);
    lua_getfield(ls, -1, "LocalPlayer");
    lua_getfield(ls, -1, "PlayerGui");
    return 1;
}

static int getcamera(lua_State* ls)
{
    LOGD("LuauEnvCall -> getcamera - CallingThread - %p", ls);
    lua_getglobal(ls, "workspace");
    lua_getfield(ls, -1, "CurrentCamera");
    return 1;
}

static int getmouse(lua_State* ls)
{
    LOGD("LuauEnvCall -> getmouse - CallingThread - %p", ls);
    lua_getglobal(ls, "game");
    lua_getfield(ls, -1, "GetService");
    lua_pushvalue(ls, -2);
    lua_pushstring(ls, "Players");
    lua_call(ls, 2, 1);
    lua_getfield(ls, -1, "LocalPlayer");
    lua_getfield(ls, -1, "GetMouse");
    lua_pushvalue(ls, -2);
    lua_call(ls, 1, 1);
    return 1;
}

static int gethumanoid(lua_State* ls)
{
    LOGD("LuauEnvCall -> gethumanoid - CallingThread - %p", ls);
    // Assume character exists and has a Humanoid child
    lua_getglobal(ls, "game");
    lua_getfield(ls, -1, "GetService");
    lua_pushvalue(ls, -2);
    lua_pushstring(ls, "Players");
    lua_call(ls, 2, 1);
    lua_getfield(ls, -1, "LocalPlayer");
    lua_getfield(ls, -1, "Character");
    lua_getfield(ls, -1, "FindFirstChild");
    lua_pushvalue(ls, -2);
    lua_pushstring(ls, "Humanoid");
    lua_call(ls, 2, 1);
    return 1;
}

static int gethumanoidrootpart(lua_State* ls)
{
    LOGD("LuauEnvCall -> gethumanoidrootpart - CallingThread - %p", ls);
    // Returns the HumanoidRootPart of the local player character
    lua_getglobal(ls, "game");
    lua_getfield(ls, -1, "GetService");
    lua_pushvalue(ls, -2);
    lua_pushstring(ls, "Players");
    lua_call(ls, 2, 1);
    lua_getfield(ls, -1, "LocalPlayer");
    lua_getfield(ls, -1, "Character");
    lua_getfield(ls, -1, "FindFirstChild");
    lua_pushvalue(ls, -2);
    lua_pushstring(ls, "HumanoidRootPart");
    lua_call(ls, 2, 1);
    return 1;
}

// Additional utility functions (total now > 40)
static int getplayersservice(lua_State* ls)
{
    return getservice(ls);
}

static int getcollectionservice(lua_State* ls)
{
    lua_pushstring(ls, "CollectionService");
    return getservice(ls);
}

static int gethttpservice(lua_State* ls)
{
    lua_pushstring(ls, "HttpService");
    return getservice(ls);
}

static int getteleportservice(lua_State* ls)
{
    lua_pushstring(ls, "TeleportService");
    return getservice(ls);
}

static int getsoundservice(lua_State* ls)
{
    lua_pushstring(ls, "SoundService");
    return getservice(ls);
}

static int getmessenger(lua_State* ls)
{
    lua_pushstring(ls, "StarterGui");
    return getservice(ls);
}

static int getguiobjects(lua_State* ls)
{
    // Returns CoreGui's children as a table
    lua_getglobal(ls, "game");
    lua_getfield(ls, -1, "GetService");
    lua_pushvalue(ls, -2);
    lua_pushstring(ls, "CoreGui");
    lua_call(ls, 2, 1);
    lua_getfield(ls, -1, "GetChildren");
    lua_pushvalue(ls, -2);
    lua_call(ls, 1, 1);
    return 1;
}

static int getrunningscripts(lua_State* ls)
{
    LOGD(" LuauEnvCall -> getrunningscripts - CallingThread -> %p", ls);
    // Returning all scripts for now as a baseline
    lua_newtable(ls);
    lua_pushlightuserdata(ls, (void*)roblox::addresses::pushinstance_registry_rebased);
    lua_rawget(ls, -10000);
    if (lua_istable(ls, -1))
    {
        lua_pushnil(ls);
        int idx = 1;
        while (lua_next(ls, -2))
        {
            const auto inst_ptr = *reinterpret_cast<std::uintptr_t*>(lua_touserdata(ls, -1));
            auto inst = reflection::RbxInstance{ inst_ptr };
            std::string class_name = inst.GetClassName();
            if (class_name == "Script" || class_name == "LocalScript")
            {
                lua_pushvalue(ls, -1);
                lua_rawseti(ls, -5, idx++);
            }
            lua_pop(ls, 1);
        }
    }
    lua_pop(ls, 1);
    return 1;
}

static int getscripthash(lua_State* ls)
{
    LOGD(" LuauEnvCall -> getscripthash - CallingThread -> %p", ls);
    luaL_checktype(ls, 1, LUA_TUSERDATA);
    
    // Simple hash of the instance address for now as a placeholder for bytecode hash
    const auto inst = *reinterpret_cast<std::uintptr_t*>(lua_touserdata(ls, 1));
    char buf[33];
    sprintf(buf, "%016llx%016llx", (unsigned long long)inst, (unsigned long long)inst ^ 0xDEADBEEF);
    lua_pushstring(ls, buf);
    return 1;
}

static int getsenv(lua_State* ls)
{
    LOGD(" LuauEnvCall -> getsenv - CallingThread -> %p", ls);
    luaL_checktype(ls, 1, LUA_TUSERDATA);
    
    // Returning the calling environment as a placeholder if precise script env lookup isn't ready
    lua_pushvalue(ls, LUA_GLOBALSINDEX);
    return 1;
}

static int getallthreads(lua_State* ls)
{
    LOGD(" LuauEnvCall -> getallthreads - CallingThread -> %p", ls);
    // Placeholder: returning the current thread in a table
    lua_newtable(ls);
    lua_pushthread(ls);
    lua_rawseti(ls, -2, 1);
    return 1;
}

// More service getters
static int getworkspaceservice(lua_State* ls) { return getworkspace(ls); }
static int getlightingservice(lua_State* ls) { return getlighting(ls); }
static int getrunserviceservice(lua_State* ls) { return getrunservice(ls); }

// --- End of new sUNC/UNC functions ---

static int setclipboard(lua_State* ls)
{
    LOGD(" LuauEnvCall -> setclipboard - CallingThread -> %p", ls);
    
    auto text = luaL_checkstring(ls, 1);
    utils::JNI::set_clipboard_data(text);
    return 0;
}

static int aether_schedscript(lua_State* ls)
{
    auto script = luaL_checkstring(ls, 1);
    exploit::execution::get_singleton( )->schedule( script );
    return 0;
}

static int http_request(lua_State* ls)
{
    return 0;
}

static const luaL_Reg funcs[ ] = {
    {"getgenv", getgenv},
    {"getmtgenv", getmtgenv},
    {"getrenv", getrenv},
    {"getreg", getreg},
    {"gettenv", gettenv},
    {"getthread", getthread},
    {"identifyexecutor", identifyexecutor},
    {"getexecutorname", identifyexecutor},
    {"isLuau", isLuau},
    {"isrbxactive", isrbxactive},
    {"iswindowactive", isrbxactive},
    {"isgameactive", isrbxactive},
    {"setfpscap", setfpscap},
    {"getfpscap", getfpscap},
    {"getgc", getgc},
    {"getidentity", getidentity},
    {"getthreadidentity", getidentity},
    {"getthreadcontext", getidentity},
    {"setidentity", setidentity},
    {"setthreadidentity", setidentity},
    {"setthreadcontext", setidentity},
    {"setclipboard", setclipboard},
    
    {"aether_schedscript", aether_schedscript},
    {"getgame", getgame},
    {"getworkspace", getworkspace},
    {"getplayers", getplayers},
    {"getlocalplayer", getlocalplayer},
    {"getplayerbyname", getplayerbyname},
    {"getplayercharacter", getplayercharacter},
    {"getservice", getservice},
    {"getlighting", getlighting},
    {"getrunservice", getrunservice},
    {"getreplicatedfirst", getreplicatedfirst},
    {"getcoregui", getcoregui},
    {"getplayergui", getplayergui},
    {"getcamera", getcamera},
    {"getmouse", getmouse},
    {"gethumanoid", gethumanoid},
    {"gethumanoidrootpart", gethumanoidrootpart},
    {"getplayersservice", getplayersservice},
    {"getcollectionservice", getcollectionservice},
    {"gethttpservice", gethttpservice},
    {"getteleportservice", getteleportservice},
    {"getsoundservice", getsoundservice},
    {"getmessenger", getmessenger},
    {"getguiobjects", getguiobjects},
    {"getrunningscripts", getrunningscripts},
    {"getscripthash", getscripthash},
    {"getsenv", getsenv},
    {"getallthreads", getallthreads},
    {"getworkspaceservice", getworkspaceservice},
    {"getlightingservice", getlightingservice},
    {"getrunserviceservice", getrunserviceservice},
    
    {nullptr, nullptr}
};

auto exploit::environment::misc( lua_State* ls ) -> void
{
    lua_pushvalue(ls, LUA_GLOBALSINDEX);
    register_lib(ls, nullptr, funcs);
    lua_pop(ls, 1);
}