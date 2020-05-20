//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Weapon data file parsing, shared by game & client dlls.
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include <KeyValues.h>
#include <tier0/mem.h>
#include "filesystem.h"
#include "utldict.h"
#include "ammodef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CUtlDict<WeaponInfo_t *, WeaponInfoHandle_t> WeaponParse::m_WeaponInfoDatabase;

#if defined(CLIENT_DLL)
#define LOGPREFIX "CLIENT"
#else
#define LOGPREFIX "SERVER"
#endif

const char * WeaponParse::m_aszWeaponSoundTable[WPS_NUM_TYPES] = {
    "empty",
    "primary",
    "primary_npc",
    "secondary",
    "secondary_npc",
    "burst",
    "reload",
    "reload_npc",
    "melee_swing",
    "melee_hit",
    "melee_hit_world",
    "special1",
    "special2",
    "special3",
    "taunt",
    "deploy"
};

#if defined(CLIENT_DLL)
// TODO: Call it every time hud_fastswitch changes!
CON_COMMAND(cl_reloadweapondata, "")
{
    WeaponParse::Initialize( filesystem, g_pGameRules->GetEncryptionKey() );
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Returns handle to new weapon info/found one.
//-----------------------------------------------------------------------------
WeaponInfoHandle_t WeaponParse::FindWeaponInfo( const char *pszName )
{
    WeaponInfoHandle_t lookup = m_WeaponInfoDatabase.Find( pszName );
    if( lookup != m_WeaponInfoDatabase.InvalidIndex() ) {
        return lookup;
    }

    WeaponInfo_t *insert = CreateWeaponInfo();
    lookup = m_WeaponInfoDatabase.Insert( pszName, insert );
    AssertMsg( m_WeaponInfoDatabase.IsValidIndex( lookup ), "Failed to add new weapon info lookup!" );

    return lookup;
}

//-----------------------------------------------------------------------------
// Purpose: Returns pointer to weapon sound script names table.
//-----------------------------------------------------------------------------
const char ** WeaponParse::GetWeaponSoundTable()
{
    // Just like that. Workaround.
    return m_aszWeaponSoundTable;
}

//-----------------------------------------------------------------------------
// Purpose: Converts string to WeaponSound_t enum.
//-----------------------------------------------------------------------------
const WeaponSound_t WeaponParse::FindWeaponSound( const char *pszName )
{
    for( int i = WPS_FIRST_DONTUSE; i < WPS_NUM_TYPES; i++ ) {
        if( Q_stricmp( pszName, m_aszWeaponSoundTable[i] ) == 0 ) {
            return (WeaponSound_t)i;
        }
    }
    return WPS_INVALID;
}

//-----------------------------------------------------------------------------
// Purpose: returns handle to weapon info for specified classname.
//-----------------------------------------------------------------------------
WeaponInfoHandle_t WeaponParse::LookupWeaponInfo( const char *pszName )
{
    return m_WeaponInfoDatabase.Find( pszName );
}

//-----------------------------------------------------------------------------
// Purpose: Returns invalid handle to weapon info.
//-----------------------------------------------------------------------------
WeaponInfoHandle_t WeaponParse::GetInvalidWeaponInfo()
{
    return m_WeaponInfoDatabase.InvalidIndex();
}

//-----------------------------------------------------------------------------
// Purpose: Returns pointer to weapon info extracted from handle to it.
//-----------------------------------------------------------------------------
WeaponInfo_t * WeaponParse::GetWeaponInfo( WeaponInfoHandle_t hWpnInfo )
{
    static WeaponInfo_t s_invalidInfo;
    if( !m_WeaponInfoDatabase.IsValidIndex( hWpnInfo ) ) {
        return &s_invalidInfo;
    }
    return m_WeaponInfoDatabase[hWpnInfo];
}

//-----------------------------------------------------------------------------
// Purpose: Finds weapon info with specified classname.
//          Returns true if found one.
//-----------------------------------------------------------------------------
bool WeaponParse::GetWeaponInfo( const char *pszName, WeaponInfoHandle_t &hWpnInfo )
{
    hWpnInfo = m_WeaponInfoDatabase.Find( pszName );
    return m_WeaponInfoDatabase.IsValidIndex( hWpnInfo );
}

//-----------------------------------------------------------------------------
// Purpose: Loads weapons.res script and parses all defined weapons.
//          Now there are less shit in "scripts/" than before.
//-----------------------------------------------------------------------------
void WeaponParse::Initialize( IFileSystem *pFilesystem, const unsigned char *pICEKey )
{
    KeyValues *pWeaponScriptsKV = new KeyValues( "WpnScripts" );
    if( pWeaponScriptsKV->LoadFromFile( pFilesystem, "scripts/weapons.res", "GAME" ) ) {
        int found = 0;
        for( KeyValues *pSub = pWeaponScriptsKV->GetFirstTrueSubKey(); pSub; pSub = pSub->GetNextTrueSubKey() ) {
            const char *pszWpnName = pSub->GetName();
            WeaponInfoHandle_t hWpnInfo = FindWeaponInfo( pszWpnName );
            WeaponInfo_t *pWpnInfo = GetWeaponInfo( hWpnInfo ); // <-- Should return good!
            pWpnInfo->Parse( pSub, pszWpnName );

        #if defined(CLIENT_DLL)
            gWR.LoadWeaponSprites( hWpnInfo );
        #endif

            found++;
        }

        ConColorMsg( Color( 255, 255, 0, 255 ), "WeaponParse/%s: Loaded %d weapons\n", LOGPREFIX, found );
    }
}

//-----------------------------------------------------------------------------
// Purpose: Item flags that we're parsing out of the file.
//-----------------------------------------------------------------------------
struct ItemFlags_t {
    const char *    m_pszFlagName;
    int             m_iFlagValue;
};

//-----------------------------------------------------------------------------
// Purpose: String->Enum conversion table.
//-----------------------------------------------------------------------------
ItemFlags_t g_ItemFlags[8] = {
    { "ITEM_FLAG_SELECTONEMPTY",        ITEM_FLAG_SELECTONEMPTY },
    { "ITEM_FLAG_NOAUTORELOAD",         ITEM_FLAG_NOAUTORELOAD },
    { "ITEM_FLAG_NOAUTOSWITCHEMPTY",    ITEM_FLAG_NOAUTOSWITCHEMPTY },
    { "ITEM_FLAG_LIMITINWORLD",         ITEM_FLAG_LIMITINWORLD },
    { "ITEM_FLAG_EXHAUSTIBLE",          ITEM_FLAG_EXHAUSTIBLE },
    { "ITEM_FLAG_DOHITLOCATIONDMG",     ITEM_FLAG_DOHITLOCATIONDMG },
    { "ITEM_FLAG_NOAMMOPICKUPS",        ITEM_FLAG_NOAMMOPICKUPS },
    { "ITEM_FLAG_NOITEMPICKUP",         ITEM_FLAG_NOITEMPICKUP }
};

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//          Also fills internal arrays with zeros because fuck you c++
//-----------------------------------------------------------------------------
WeaponInfo_t::WeaponInfo_t()
{
    // error C2536: 'WeaponInfo_t::szANYFUCKINGARRAY': cannot specify explicit initializer for arrays
    Q_memset( szClassName,          0, sizeof( szClassName ) );
    Q_memset( szPrintName,          0, sizeof( szPrintName ) );
    Q_memset( szViewModel,          0, sizeof( szViewModel ) );
    Q_memset( szWorldModel,         0, sizeof( szWorldModel ) );
    Q_memset( szAnimationPrefix,    0, sizeof( szAnimationPrefix ) );
    Q_memset( szAmmo1,              0, sizeof( szAmmo1 ) );
    Q_memset( szAmmo2,              0, sizeof( szAmmo2 ) );
    Q_memset( m_weaponSounds,       0, sizeof( m_weaponSounds ) );
}

#ifdef CLIENT_DLL
extern ConVar hud_fastswitch;
#endif

//-----------------------------------------------------------------------------
// Purpose: Parses input KeyValues.
//          If God wanted me to live, he would not have created a C++98
//-----------------------------------------------------------------------------
void WeaponInfo_t::Parse( KeyValues *pKeyValuesData, const char *pszWeaponName )
{
    // @und: I prefer not to touch this code -- this is incredibly good for valve code.
    Q_strncpy( szClassName,         pszWeaponName, MAX_WEAPON_STRING );
    Q_strncpy( szPrintName,         pKeyValuesData->GetString( "printname", WEAPON_PRINTNAME_MISSING ), MAX_WEAPON_STRING );
    Q_strncpy( szViewModel,         pKeyValuesData->GetString( "viewmodel" ),   MAX_WEAPON_STRING );
    Q_strncpy( szWorldModel,        pKeyValuesData->GetString( "playermodel" ), MAX_WEAPON_STRING );
    Q_strncpy( szAnimationPrefix,   pKeyValuesData->GetString( "anim_prefix" ), MAX_WEAPON_PREFIX );

    iSlot       = pKeyValuesData->GetInt( "bucket", 0 );
    iPosition   = pKeyValuesData->GetInt( "bucket_position", 0 );

#if defined(CLIENT_DLL)
    // Emulate x360 slots.
    // Looks cool...
    if( hud_fastswitch.GetInt() == 2 ) {
        iSlot       = pKeyValuesData->GetInt( "bucket_360",             iSlot );
        iPosition   = pKeyValuesData->GetInt( "bucket_position_360",    iPosition );
    }
#endif

    iMaxClip1       = pKeyValuesData->GetInt( "clip_size",      WEAPON_NOCLIP );    // Max primary clips gun can hold (assume they don't use clips by default)
    iMaxClip2       = pKeyValuesData->GetInt( "clip2_size",     WEAPON_NOCLIP );    // Max secondary clips gun can hold (assume they don't use clips by default)
    iDefaultClip1   = pKeyValuesData->GetInt( "default_clip",   iMaxClip1 );        // amount of primary ammo placed in the primary clip when it's picked up
    iDefaultClip2   = pKeyValuesData->GetInt( "default_clip2",  iMaxClip2 );        // amount of secondary ammo placed in the secondary clip when it's picked up
    iWeight         = pKeyValuesData->GetInt( "weight",         0 );
    iRumbleEffect   = pKeyValuesData->GetInt( "rumble",        -1 );

    iFlags = ITEM_FLAG_LIMITINWORLD;

    iFlags = 0;
    for( int i = 0; i < ARRAYSIZE( g_ItemFlags ); i++ ) {
        int iFlag = pKeyValuesData->GetInt( g_ItemFlags[i].m_pszFlagName, -1 );
        if( iFlag != -1 ) { // oh fuck
            iFlags = !!iFlag ? ( iFlags & ~g_ItemFlags[i].m_iFlagValue ) : ( iFlags | g_ItemFlags[i].m_iFlagValue );
        }
    }

    // bool x = i ? true : false;
    // what the fuck? isn't just easier to do !!i?
    bAutoSwitchTo       = !!pKeyValuesData->GetInt( "autoswitchto", 1 );
    bAutoSwitchFrom     = !!pKeyValuesData->GetInt( "autoswitchfrom", 1 );
    m_bBuiltRightHanded = !!pKeyValuesData->GetInt( "BuiltRightHanded", 1 );
    m_bAllowFlipping    = !!pKeyValuesData->GetInt( "AllowFlipping", 1 );
    m_bMeleeWeapon      = !!pKeyValuesData->GetInt( "MeleeWeapon", 0 );
    
    // Primary ammo
    const char *pszAmmo1 = pKeyValuesData->GetString( "primary_ammo", "None" );
    Q_strncpy( szAmmo1, !Q_stricmp( pszAmmo1, "None" ) ? "" : pszAmmo1, sizeof( szAmmo1 ) );
    iAmmoType = GetAmmoDef()->Index( szAmmo1 );

    // Secondary ammo
    const char *pszAmmo2 = pKeyValuesData->GetString( "secondary_ammo", "None" );
    Q_strncpy( szAmmo2, !Q_stricmp( pszAmmo2, "None" ) ? "" : pszAmmo2, sizeof( szAmmo2 ) );
    iAmmo2Type = GetAmmoDef()->Index( szAmmo2 );
    
    // Weapon sounds
    Q_memset( m_weaponSounds, 0, sizeof( m_weaponSounds ) );
    KeyValues *pWeaponSoundKV = pKeyValuesData->FindKey( "SoundData", true );
    for( int i = WPS_FIRST_DONTUSE; i < WPS_NUM_TYPES; i++ ) {
        const char *pszSoundName = pWeaponSoundKV->GetString( WeaponParse::GetWeaponSoundTable()[i] );
        if( pszSoundName && pszSoundName[0] ) {
            Q_strncpy( m_weaponSounds[i], pszSoundName, sizeof( m_weaponSounds[i] ) );
        }
    }
}

KeyValues * ReadEncryptedKVFile( IFileSystem *pFilesystem, const char *pszFilename, const unsigned char *pICEKey, bool bForceReadEncryptedFile /*= false*/ )
{
    AssertMsg( Q_strrchr( pszFilename, '.' ) == NULL, "ReadEncryptedFile: I SAID WITHOUT FUCKING EXTENSION" );

    char szFullName[512];
    Q_snprintf( szFullName, sizeof( szFullName ), "%s.txt", pszFilename );

    // Open the weapon data file, and abort if we can't
    KeyValues *pWeaponScript = new KeyValues( "WeaponDataFile" );
    const char *pszSearchPath = pICEKey ? "MOD" : "GAME";
    if( bForceReadEncryptedFile || !pWeaponScript->LoadFromFile( pFilesystem, szFullName, pszSearchPath ) ) {
        if( pICEKey ) {
            Q_snprintf( szFullName, sizeof( szFullName ), "%s.ctx", pszFilename );
            FileHandle_t file = pFilesystem->Open( szFullName, "rb", pszSearchPath );
            if( !file ) {
                pWeaponScript->deleteThis();
                return NULL;
            }

            int fileSize = pFilesystem->Size( file );
            char *pBuffer = (char *)MemAllocScratch( fileSize + 1 );
            AssertMsg( pBuffer, "MemAllocScratch failed." );

            pFilesystem->Read( pBuffer, fileSize, file );
            pBuffer[fileSize] = '\0';
            pFilesystem->Close( file );

            UTIL_DecodeICE( (unsigned char *)pBuffer, fileSize, (const unsigned char *)pICEKey );
            bool ok = pWeaponScript->LoadFromBuffer( szFullName, pBuffer, pFilesystem );
            MemFreeScratch();
            if( ok ) {
                return pWeaponScript;
            }
        }

        pWeaponScript->deleteThis();
        return NULL;
    }

    return pWeaponScript;
}
