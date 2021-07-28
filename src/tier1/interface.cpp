//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//===========================================================================//
#include <windows.h>

#if !defined( DONT_PROTECT_FILEIO_FUNCTIONS )
#define DONT_PROTECT_FILEIO_FUNCTIONS // for protected_things.h
#endif

#if defined( PROTECTED_THINGS_ENABLE )
#undef PROTECTED_THINGS_ENABLE // from protected_things.h
#endif

#include <stdio.h>
#include "interface.h"
#include "basetypes.h"
#include "tier0/dbg.h"
#include <string.h>
#include <stdlib.h>
#include "tier1/strtools.h"
#include "tier0/icommandline.h"
#include "tier0/dbg.h"
#include "tier0/threadtools.h"
#include <direct.h> // getcwd

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// ------------------------------------------------------------------------------------ //
// InterfaceReg.
// ------------------------------------------------------------------------------------ //
InterfaceReg *InterfaceReg::s_pInterfaceRegs = NULL;

InterfaceReg::InterfaceReg( InstantiateInterfaceFn fn, const char *pName ) : m_pName( pName )
{
    m_CreateFn = fn;
    m_pNext = s_pInterfaceRegs;
    s_pInterfaceRegs = this;
}

// ------------------------------------------------------------------------------------ //
// CreateInterface.
// This is the primary exported function by a dll, referenced by name via dynamic binding
// that exposes an opqaue function pointer to the interface.
//
// We have the Internal variant so Sys_GetFactoryThis() returns the correct internal
// symbol under GCC/Linux/Mac as CreateInterface is DLL_EXPORT so its global so the loaders
// on those OS's pick exactly 1 of the CreateInterface symbols to be the one that is process wide and
// all Sys_GetFactoryThis() calls find that one, which doesn't work. Using the internal walkthrough here
// makes sure Sys_GetFactoryThis() has the dll specific symbol and GetProcAddress() returns the module specific
// function for CreateInterface again getting the dll specific symbol we need.
// ------------------------------------------------------------------------------------ //
void* CreateInterfaceInternal( const char *pName, int *pReturnCode )
{
    InterfaceReg *pCur;

    for( pCur = InterfaceReg::s_pInterfaceRegs; pCur; pCur = pCur->m_pNext ) {
        if( strcmp( pCur->m_pName, pName ) == 0 ) {
            if( pReturnCode ) {
                *pReturnCode = IFACE_OK;
            }
            return pCur->m_CreateFn();
        }
    }

    if( pReturnCode ) {
        *pReturnCode = IFACE_FAILED;
    }

    return NULL;
}

void *CreateInterface( const char *pName, int *pReturnCode )
{
    return CreateInterfaceInternal( pName, pReturnCode );
}

//-----------------------------------------------------------------------------
// Purpose: returns a pointer to a function, given a module
// Input  : pModuleName - module name
//          *pName - proc name
//-----------------------------------------------------------------------------
static void *Sys_GetProcAddress( const char *pModuleName, const char *pName )
{
    return (void *)GetProcAddress( (HMODULE)GetModuleHandle( pModuleName ), pName );
}

static void *Sys_GetProcAddress( HMODULE hModule, const char *pName )
{
    return (void *)GetProcAddress( hModule, pName );
}

void *Sys_GetProcAddress( CSysModule *pModule, const char *pszProcName )
{
    return (void *)GetProcAddress( (HMODULE)pModule, pszProcName );
}

bool Sys_IsDebuggerPresent()
{
    return Plat_IsInDebugSession();
}

struct ThreadedLoadLibaryContext_t {
    const char *m_pLibraryName;
    HMODULE m_hLibrary;
};

#ifdef _WIN32

// wraps LoadLibraryEx() since 360 doesn't support that
static HMODULE InternalLoadLibrary( const char *pName, Sys_Flags flags )
{
    if( flags & SYS_NOLOAD )
        return GetModuleHandle( pName );
    else
        return LoadLibraryEx( pName, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
}

unsigned ThreadedLoadLibraryFunc( void *pParam )
{
    ThreadedLoadLibaryContext_t *pContext = (ThreadedLoadLibaryContext_t*)pParam;
    pContext->m_hLibrary = InternalLoadLibrary( pContext->m_pLibraryName, SYS_NOFLAGS );
    return 0;
}

#endif // _WIN32

HMODULE Sys_LoadLibrary( const char *pLibraryName, Sys_Flags flags )
{
    char str[1024];
    // Note: DLL_EXT_STRING can be "_srv.so" or "_360.dll". So be careful
    //  when using the V_*Extension* routines...
    const char *pDllStringExtension = V_GetFileExtension( DLL_EXT_STRING );
    const char *pModuleExtension = pDllStringExtension ? ( pDllStringExtension - 1 ) : DLL_EXT_STRING;

    Q_strncpy( str, pLibraryName, sizeof( str ) );

    // always force the final extension to be .dll
    V_SetExtension( str, pModuleExtension, sizeof( str ) );

    Q_FixSlashes( str );

    ThreadedLoadLibraryFunc_t threadFunc = GetThreadedLoadLibraryFunc();
    if( !threadFunc )
        return InternalLoadLibrary( str, flags );

    // We shouldn't be passing noload while threaded.
    Assert( !( flags & SYS_NOLOAD ) );

    ThreadedLoadLibaryContext_t context;
    context.m_pLibraryName = str;
    context.m_hLibrary = 0;

    ThreadHandle_t h = CreateSimpleThread( ThreadedLoadLibraryFunc, &context );

    unsigned int nTimeout = 0;
    while( ThreadWaitForObject( h, true, nTimeout ) == TW_TIMEOUT ) {
        nTimeout = threadFunc();
    }

    ReleaseThreadHandle( h );
    return context.m_hLibrary;
}

static bool s_bRunningWithDebugModules = false;

//-----------------------------------------------------------------------------
// Purpose: Loads a DLL/component from disk and returns a handle to it
// Input  : *pModuleName - filename of the component
// Output : opaque handle to the module (hides system dependency)
//-----------------------------------------------------------------------------
CSysModule *Sys_LoadModule( const char *pModuleName, Sys_Flags flags /* = SYS_NOFLAGS (0) */ )
{
    // If using the Steam filesystem, either the DLL must be a minimum footprint
    // file in the depot (MFP) or a filesystem GetLocalCopy() call must be made
    // prior to the call to this routine.
    char szCwd[1024];
    HMODULE hDLL = NULL;

    if( !Q_IsAbsolutePath( pModuleName ) ) {
        // full path wasn't passed in, using the current working dir
        _getcwd( szCwd, sizeof( szCwd ) );
        if( szCwd[strlen( szCwd ) - 1] == '/' || szCwd[strlen( szCwd ) - 1] == '\\' ) {
            szCwd[strlen( szCwd ) - 1] = 0;
        }

        char szAbsoluteModuleName[1024];
        size_t cCwd = strlen( szCwd );
        if( strstr( pModuleName, "bin/" ) == pModuleName || ( szCwd[cCwd - 1] == 'n'  && szCwd[cCwd - 2] == 'i' && szCwd[cCwd - 3] == 'b' ) ) {
            // don't make bin/bin path
            Q_snprintf( szAbsoluteModuleName, sizeof( szAbsoluteModuleName ), "%s/%s", szCwd, pModuleName );
        }
        else {
            Q_snprintf( szAbsoluteModuleName, sizeof( szAbsoluteModuleName ), "%s/bin/%s", szCwd, pModuleName );
        }
        hDLL = Sys_LoadLibrary( szAbsoluteModuleName, flags );
    }

    if( !hDLL ) {
        hDLL = Sys_LoadLibrary( pModuleName, flags );
#if defined( _DEBUG )
        if( !hDLL ) {
            // So you can see what the error is in the debugger...
            char *lpMsgBuf;
            FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPTSTR)&lpMsgBuf, 0, NULL );
            LocalFree( (HLOCAL)lpMsgBuf );
        }
#endif // DEBUG
    }

    // If running in the debugger, assume debug binaries are okay, otherwise they must run with -allowdebug
    if( hDLL && Sys_GetProcAddress( hDLL, "BuiltDebug" ) ) {
        if( !CommandLine()->FindParm( "-allowdebug" ) && !Sys_IsDebuggerPresent() ) {
            Error( "Module %s is a debug build\n", pModuleName );
        }

        DevWarning( "Module %s is a debug build\n", pModuleName );

        if( !s_bRunningWithDebugModules ) {
            s_bRunningWithDebugModules = true;
        }
    }

    return (CSysModule *)hDLL;
}

//-----------------------------------------------------------------------------
// Purpose: Determine if any debug modules were loaded
//-----------------------------------------------------------------------------
bool Sys_RunningWithDebugModules()
{
    return s_bRunningWithDebugModules;
}


//-----------------------------------------------------------------------------
// Purpose: Unloads a DLL/component from
// Input  : *pModuleName - filename of the component
// Output : opaque handle to the module (hides system dependency)
//-----------------------------------------------------------------------------
void Sys_UnloadModule( CSysModule *pModule )
{
    if( !pModule )
        return;
    FreeLibrary( (HMODULE)pModule );
}

//-----------------------------------------------------------------------------
// Purpose: returns a pointer to a function, given a module
// Input  : module - windows HMODULE from Sys_LoadModule()
//          *pName - proc name
// Output : factory for this module
//-----------------------------------------------------------------------------
CreateInterfaceFn Sys_GetFactory( CSysModule *pModule )
{
    if( !pModule )
        return NULL;
    return (CreateInterfaceFn)GetProcAddress( (HMODULE)pModule, CREATEINTERFACE_PROCNAME );
}

//-----------------------------------------------------------------------------
// Purpose: returns the instance of this module
// Output : interface_instance_t
//-----------------------------------------------------------------------------
CreateInterfaceFn Sys_GetFactoryThis( void )
{
    return &CreateInterfaceInternal;
}

//-----------------------------------------------------------------------------
// Purpose: returns the instance of the named module
// Input  : *pModuleName - name of the module
// Output : interface_instance_t - instance of that module
//-----------------------------------------------------------------------------
CreateInterfaceFn Sys_GetFactory( const char *pModuleName )
{
    return (CreateInterfaceFn)Sys_GetProcAddress( pModuleName, CREATEINTERFACE_PROCNAME );
}

//-----------------------------------------------------------------------------
// Purpose: get the interface for the specified module and version
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool Sys_LoadInterface(
    const char *pModuleName,
    const char *pInterfaceVersionName,
    CSysModule **pOutModule,
    void **pOutInterface )
{
    CSysModule *pMod = Sys_LoadModule( pModuleName );
    if( !pMod )
        return false;

    CreateInterfaceFn fn = Sys_GetFactory( pMod );
    if( !fn ) {
        Sys_UnloadModule( pMod );
        return false;
    }

    *pOutInterface = fn( pInterfaceVersionName, NULL );
    if( !( *pOutInterface ) ) {
        Sys_UnloadModule( pMod );
        return false;
    }

    if( pOutModule )
        *pOutModule = pMod;

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Place this as a singleton at module scope (e.g.) and use it to get the factory from the specified module name.
//
// When the singleton goes out of scope (.dll unload if at module scope),
//  then it'll call Sys_UnloadModule on the module so that the refcount is decremented
//  and the .dll actually can unload from memory.
//-----------------------------------------------------------------------------
CDllDemandLoader::CDllDemandLoader( char const *pchModuleName ) : m_pchModuleName( pchModuleName ), m_hModule( 0 ), m_bLoadAttempted( false )
{
}

CDllDemandLoader::~CDllDemandLoader()
{
    Unload();
}

CreateInterfaceFn CDllDemandLoader::GetFactory()
{
    if( !m_hModule && !m_bLoadAttempted ) {
        m_bLoadAttempted = true;
        m_hModule = Sys_LoadModule( m_pchModuleName );
    }

    if( !m_hModule ) {
        return NULL;
    }

    return Sys_GetFactory( m_hModule );
}

void CDllDemandLoader::Unload()
{
    if( m_hModule ) {
        Sys_UnloadModule( m_hModule );
        m_hModule = 0;
    }
}
