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

// The sound categories found in the weapon classname.txt files
// This needs to match the WeaponSound_t enum in weapon_parse.h
// @und: Saved this fuckers here to leave compatibility with original weapon_parse stuff.
const char *pWeaponSoundCategories[WPS_NUM_TYPES] = {
    "empty",
    "single_shot",
    "single_shot_npc",
    "double_shot",
    "double_shot_npc",
    "burst",
    "reload",
    "reload_npc",
    "melee_miss",
    "melee_hit",
    "melee_hit_world",
    "special1",
    "special2",
    "special3",
    "taunt",
    "deploy"
};

const WeaponSound_t GetWeaponSound( const char *pszString )
{
    for( int i = WPS_FIRST_DONTUSE; i < WPS_NUM_TYPES; i++ ) {
        if( Q_stricmp( pszString, pWeaponSoundCategories[i] ) == 0 ) {
            return (WeaponSound_t)i;
        }
    }
    return WPS_INVALID;
}

// Item flags that we're parsing out of the file.
struct ItemFlags_t {
    const char *    m_pszFlagName;
    int             m_iFlagValue;
};

// Item flags list...
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

static CUtlDict<FileWeaponInfo_t *, WeaponFileInfoHandle_t> m_WeaponInfoDatabase;

// used to track whether or not two weapons have been mistakenly assigned the wrong slot
bool g_bUsedWeaponSlots[MAX_WEAPON_SLOTS][MAX_WEAPON_POSITIONS] = { 0 };

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *name -
// Output : FileWeaponInfo_t
//-----------------------------------------------------------------------------
static WeaponFileInfoHandle_t FindWeaponInfoSlot( const char *pszName )
{
    WeaponFileInfoHandle_t lookup = m_WeaponInfoDatabase.Find( pszName );
    if( lookup != m_WeaponInfoDatabase.InvalidIndex() ) {
        return lookup;
    }

    FileWeaponInfo_t *insert = CreateWeaponInfo();
    lookup = m_WeaponInfoDatabase.Insert( pszName, insert );
    Assert( lookup != m_WeaponInfoDatabase.InvalidIndex() );
    return lookup;
}

// Find a weapon slot, assuming the weapon's data has already been loaded.
WeaponFileInfoHandle_t LookupWeaponInfoSlot( const char *pszName )
{
    return m_WeaponInfoDatabase.Find( pszName );
}

// FIXME, handle differently?
static FileWeaponInfo_t gNullWeaponInfo;

//-----------------------------------------------------------------------------
// Purpose:
// Input  : handle -
// Output : FileWeaponInfo_t
//-----------------------------------------------------------------------------

FileWeaponInfo_t * GetWeaponInfoFromHandle( WeaponFileInfoHandle_t handle )
{
    if( !m_WeaponInfoDatabase.IsValidIndex( handle ) ) {
        return &gNullWeaponInfo;
    }
    return m_WeaponInfoDatabase[handle];
}

//-----------------------------------------------------------------------------
// Purpose:
// Output : FileWeaponInfo_t
//-----------------------------------------------------------------------------
WeaponFileInfoHandle_t GetInvalidWeaponInfoHandle()
{
    return m_WeaponInfoDatabase.InvalidIndex();
}

void PrecacheFileWeaponInfoDatabase( IFileSystem *pFilesystem, const unsigned char *pICEKey )
{
    // Already did!
    if( m_WeaponInfoDatabase.Count() != 0 ) {
        return;
    }

    KeyValues *pManifestKV = new KeyValues( "WeaponScripts" );
    if( pManifestKV->LoadFromFile( pFilesystem, "scripts/weapon_manifest.txt", "GAME" ) ) {
        for( KeyValues *pSub = pManifestKV->GetFirstSubKey(); pSub; pSub = pSub->GetNextKey() ) {
            if( !Q_stricmp( pSub->GetName(), "file" ) ) {
                char file_base[512] = { 0 };
                Q_FileBase( pSub->GetString(), file_base, sizeof( file_base ) );
                
                WeaponFileInfoHandle_t handle;
                if( ReadWeaponDataFromFileForSlot( pFilesystem, file_base, handle, pICEKey ) ) {
                #if defined(CLIENT_DLL)
                    gWR.LoadWeaponSprites( handle );
                #endif
                }
            }
            // otherwise - do nothing.
        }
    }
    pManifestKV->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose: Read data on weapon from script file
// Output:  true  - if data2 successfully read
//          false - if data load fails
//-----------------------------------------------------------------------------

bool ReadWeaponDataFromFileForSlot( IFileSystem *pFilesystem, const char *pszWeaponName, WeaponFileInfoHandle_t &handle, const unsigned char *pICEKey )
{
    handle = FindWeaponInfoSlot( pszWeaponName );
    FileWeaponInfo_t *pFileInfo = GetWeaponInfoFromHandle( handle );
    AssertMsg( !!pFileInfo, "ReadWeaponDataFromFileForSlot: pFileInfo is NULL!" );
    if( pFileInfo->bWasTryingToParse ) {
        return true;
    }

    // TODO: make it scripts/weapons/%s
    char script_name[128] = { 0 };
    Q_snprintf( script_name, sizeof( script_name ), "scripts/%s", pszWeaponName );
    KeyValues *pWeaponScript = ReadEncryptedKVFile( pFilesystem, script_name, pICEKey, false );
    if( !pWeaponScript ) {
        return false;
    }

    pFileInfo->Parse( pWeaponScript, pszWeaponName );
    pWeaponScript->deleteThis();
    return true;
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

//-----------------------------------------------------------------------------
// FileWeaponInfo_t implementation.
//-----------------------------------------------------------------------------
FileWeaponInfo_t::FileWeaponInfo_t()
{
    // error C2536: 'FileWeaponInfo::szANYFUCKINGARRAY': cannot specify explicit initializer for arrays
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

void FileWeaponInfo_t::Parse( KeyValues *pKeyValuesData, const char *pszWeaponName )
{
    
    // Okay, we tried at least once to look this up...
    bWasTryingToParse = true;

    // @und: I prefer not to touch this code -- this is incredibly good for valve code.
    Q_strncpy( szClassName, pszWeaponName, MAX_WEAPON_STRING );
    Q_strncpy( szPrintName, pKeyValuesData->GetString( "printname", WEAPON_PRINTNAME_MISSING ), MAX_WEAPON_STRING );

    Q_strncpy( szViewModel, pKeyValuesData->GetString( "viewmodel" ), MAX_WEAPON_STRING );
    Q_strncpy( szWorldModel, pKeyValuesData->GetString( "playermodel" ), MAX_WEAPON_STRING );
    Q_strncpy( szAnimationPrefix, pKeyValuesData->GetString( "anim_prefix" ), MAX_WEAPON_PREFIX );

    iSlot = pKeyValuesData->GetInt( "bucket", 0 );
    iPosition = pKeyValuesData->GetInt( "bucket_position", 0 );

#if defined(CLIENT_DLL)
    // Imitate x360 slots.
    if( hud_fastswitch.GetInt() == 2 ) {
        iSlot = pKeyValuesData->GetInt( "bucket_360", iSlot );
        iPosition = pKeyValuesData->GetInt( "bucket_position_360", iPosition );
    }
#endif

    iMaxClip1       = pKeyValuesData->GetInt( "clip_size", WEAPON_NOCLIP );     // Max primary clips gun can hold (assume they don't use clips by default)
    iMaxClip2       = pKeyValuesData->GetInt( "clip2_size", WEAPON_NOCLIP );    // Max secondary clips gun can hold (assume they don't use clips by default)
    iDefaultClip1   = pKeyValuesData->GetInt( "default_clip", iMaxClip1 );      // amount of primary ammo placed in the primary clip when it's picked up
    iDefaultClip2   = pKeyValuesData->GetInt( "default_clip2", iMaxClip2 );     // amount of secondary ammo placed in the secondary clip when it's picked up
    iWeight         = pKeyValuesData->GetInt( "weight", 0 );
    iRumbleEffect   = pKeyValuesData->GetInt( "rumble", -1 );

    // LAME old way to specify item flags.
    // Weapon scripts should use the flag names.
    iFlags = pKeyValuesData->GetInt( "item_flags", ITEM_FLAG_LIMITINWORLD );
    for( int i = 0; i < ARRAYSIZE( g_ItemFlags ); i++ ) {
        int v = pKeyValuesData->GetInt( g_ItemFlags[i].m_pszFlagName, -1 );
        if( v != -1 ) { // oh fuck
            iFlags = !!v
                ? ( iFlags & ~g_ItemFlags[i].m_iFlagValue )
                : ( iFlags |  g_ItemFlags[i].m_iFlagValue );
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
        const char *pszSoundName = pWeaponSoundKV->GetString( pWeaponSoundCategories[i] );
        if( pszSoundName && pszSoundName[0] ) {
            Q_strncpy( m_weaponSounds[i], pszSoundName, sizeof( m_weaponSounds[i] ) );
        }
    }
}
