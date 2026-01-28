#pragma once

#include <lstate.h>
#include <cstdint>
#include <string>

/*
Messy as fuck i know but welp
Luau Files modified:

* luaconf.h -> addresses of luauAPI funcs
#include "../../../../utils/utils.hpp"

namespace roblox::addresses {
    // luau VM
    constexpr std::uintptr_t luau_execute = 0x0; // luaD_call
    
    // luaD_* functions
    constexpr std::uintptr_t luaD_throw = 0x0; // lua_error <- luaL_error // B0 B5 02 AF 05 46 0C 20 0C 46 6D
    constexpr std::uintptr_t luaD_rawrunprotected = 0x0; // luaD_pcall
    
    // luaC_* functions
    // lua_newthread -> B0 B5 02 AF 04 46 00 69 D0
    constexpr std::uintptr_t luaC_step = 0x0; // lua_newthread // F0 B5 03 AF 2D E9 00 0F 81 B0 2D ED 02 8B 84 B0 04 69
    constexpr std::uintptr_t luaC_barriertable = 0x0; // 00 69 43 7D 02
    
    // luaV_* functions
    constexpr std::uintptr_t luaV_gettable = 0x0;
    constexpr std::uintptr_t luaV_settable = 0x0;
    
    constexpr std::uintptr_t luaO_nilobj = 0x0; // any luau push or get or to function
    constexpr std::uintptr_t dummynode = 0x0; // luaH_getnum <- usually inlined in luaH_get
}

* luaconf.h

* lobject.h
//#define luaO_nilobject (&luaO_nilobject_)
#define luaO_nilobject (TValue*)utils::memory::rebase( "libroblox.so", roblox::addresses::luaO_nilobj )
*lobject.h

* ltable.cpp
//#define dummynode (&luaH_dummynode)
#define dummynode (LuaNode*)utils::memory::rebase( "libroblox.so", roblox::addresses::dummynode )
*ltable.cpp

* lvmexecute.cpp -> luau_execute <- or change the name of the func if too lazy to remove "Moduleluau_execute"
void luau_execute(lua_State* L)
{
    static auto rluau_exec = *reinterpret_cast< decltype(luau_execute)* >( utils::memory::rebase( "libroblox.so", roblox::addresses::luau_execute ) );
    return rluau_exec( L );
}

* lvmutils.cpp -> luaV_gettable, luaV_settable
void luaV_gettable(lua_State* L, const TValue* t, TValue* key, StkId val)
{
    static auto rgettable = *reinterpret_cast< decltype(luaV_gettable)* >( utils::memory::rebase( "libroblox.so", roblox::addresses::luaV_gettable ) );
    return rgettable( L, t, key, val );
}

void luaV_settable(lua_State* L, const TValue* t, TValue* key, StkId val)
{
    static auto rsettable = *reinterpret_cast< decltype(luaV_settable)* >( utils::memory::rebase( "libroblox.so", roblox::addresses::luaV_settable ) );
    return rsettable( L, t, key, val );
}

* ldo.cpp -> luaD_throw, luaD_rawrunprotected
int luaD_rawrunprotected(lua_State* L, Pfunc f, void* ud)
{
    static auto rluau_rawrunprot = *reinterpret_cast< decltype(luaD_rawrunprotected)* >( utils::memory::rebase( "libroblox.so", roblox::addresses::luaD_rawrunprotected ) );
    return rluau_rawrunprot( L, f, ud );
}

l_noret luaD_throw(lua_State* L, int errcode)
{
    static auto rluau_throw = *reinterpret_cast< decltype(luaD_throw)* >( utils::memory::rebase( "libroblox.so", roblox::addresses::luaD_throw ) );
    rluau_throw( L, errcode );
}

* lgc.cpp -> luaC_step, luaC_barriertable
size_t luaC_step(lua_State* L, bool assist)
{
    static auto rluacvarier = *reinterpret_cast< decltype(luaC_step)* >( utils::memory::rebase( "libroblox.so", roblox::addresses::luaC_step ) );
    return rluacvarier( L, assist);
}

void luaC_barriertable(lua_State* L, Table* t, GCObject* v)
{
    static auto rluacvarier = *reinterpret_cast< decltype(luaC_barriertable)* >( utils::memory::rebase( "libroblox.so", roblox::addresses::luaC_barriertable ) );
    rluacvarier( L, t, v );
}

*/

/*
bytecode localscript + 268
modulescript + 240 
sharedstr + 16 = data

getbytecode func local/modulescript vftable + 232
vftable + 252 = getbytecodedata ( derefs the shared str too ) 
*/
namespace roblox {
    namespace addresses {
        // LuaState Encryption
        auto rLEnc( std::uintptr_t sc ) -> std::uintptr_t;
        
        // Objs
        constexpr std::uintptr_t tasksched = 0x41DC86C; 
        constexpr std::uintptr_t prop_table = 0x405F324;
        
        // Funcs
        constexpr std::uintptr_t scriptcontext_resume = 0x18F4760;
        constexpr std::uintptr_t rbxspawn = 0x1932D30;
        constexpr std::uintptr_t sandboxthreadandsetidentity = 0x18E51E8;
        constexpr std::uintptr_t rlua_pushinstance = 0x18A2668;
        constexpr std::uintptr_t pushinstance_registry = 0x18A20B4;
        static std::uintptr_t pushinstance_registry_rebased = 0x0;
        constexpr std::uintptr_t rbx_getthreadcontext = 0x18DE6B4;
        constexpr std::uintptr_t rlua_newthread = 0x36CC828;
        constexpr std::uintptr_t fireclickdetector = 0x23CF824;
        constexpr std::uintptr_t fireproximityprompt = 0x23D3E78;
        constexpr std::uintptr_t firetouchinterest = 0x2F44E84;
        
        // Func Hooks
        constexpr std::uintptr_t startscript = 0x1920834;
        constexpr std::uintptr_t ongameleave = 0x131ED10;
        constexpr std::uintptr_t ongameloaded = 0x131EAC8;
        constexpr std::uintptr_t jobstart = 0x3A4C48C;
        constexpr std::uintptr_t jobstop = 0x3A4C4FC;
    }
    
    namespace offsets {
        constexpr std::uintptr_t JobListStart = 312; // 64-bit standardized
        constexpr std::uintptr_t JobListEnd = 320;   // 64-bit standardized
        
        constexpr std::uintptr_t Whsj_ScriptCtxt = 432; // 64-bit standardized
        constexpr std::uintptr_t FpsCap = 208;
    }
    
    namespace structs {
        struct live_thread_ref
        {
            int unk_0; // 0
            lua_State* th; // 4
            int thread_id; // 8
        };
        
        struct weak_thread_ref_t
        {
            std::uint8_t pad_0[8];
            
            weak_thread_ref_t* previous; // 8
            weak_thread_ref_t* next; // 12
            live_thread_ref* livethreadref; // 16
        };
        
        struct ExtraSpace_t
        {
            struct Shared {
                int threadCount; // 0
                std::uintptr_t script_context; // 8
                ExtraSpace_t* allThreads; // 16
            };
            
            ExtraSpace_t* previous; // 0
            size_t count; // 8
            ExtraSpace_t* next; // 16
            std::shared_ptr<Shared> shared; // 24
            
            weak_thread_ref_t* Node; // 40
            int context_level; // 48
            
            std::uint8_t pad_0[12]; // 52
            
            std::uintptr_t script_context; // 64
            std::uintptr_t unk_0; // 72
            std::uintptr_t context_perms; // 80
            std::uintptr_t unk_1; // 88
            std::weak_ptr<uintptr_t> script; // 96
        };
    }
    
    namespace functions {
        auto init( ) -> void;
        auto get_tasksched( ) -> std::uintptr_t;
        auto set_identity(  lua_State* ls, int identity ) -> void;
        
        inline int ( *rbxspawn )( lua_State* rL ) = nullptr;
        inline lua_State* ( *rlua_newthread )( lua_State* rL ) = nullptr;
        inline std::uintptr_t ( *rbx_getthreadcontext )( lua_State* thread ) = nullptr;
        inline int ( *scriptcontext_resume )( int unk, std::uintptr_t sc, std::uintptr_t* ref, int nargs, int, int) = nullptr;
        inline std::uintptr_t ( *sandboxthreadandsetidentity )( lua_State* ls, std::uintptr_t identity, std::uintptr_t script ) = nullptr;
        inline std::uintptr_t ( *rlua_pushinstance )( lua_State* ls, std::uintptr_t inst ) = nullptr;
        inline std::uintptr_t ( *rlua_pushinstanceSP )( lua_State* ls, std::weak_ptr<uintptr_t> inst ) = nullptr;
        inline std::uintptr_t ( *fireclickdetector )( std::uintptr_t detector, float distance, std::uintptr_t player ) = nullptr;
        inline std::uintptr_t ( *fireproximityprompt )( std::uintptr_t prompt ) = nullptr;
        inline std::uintptr_t ( *firetouchinterest )( std::uintptr_t world, std::uintptr_t to_touch, std::uintptr_t transmitter, int untouch, int) = nullptr;
    }
}