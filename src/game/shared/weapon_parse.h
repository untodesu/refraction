//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Weapon data file parsing, shared by game & client dlls.
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_PARSE_H
#define WEAPON_PARSE_H
#ifdef _WIN32
#pragma once
#endif

#include "shareddefs.h"
#include "utldict.h"

class IFileSystem;

typedef unsigned short WeaponInfoHandle_t;

// -----------------------------------------------------------
// Weapon sound types
// Used to play sounds defined in the weapon's classname.txt file
// This needs to match pWeaponSoundCategories in weapon_parse.cpp
// ------------------------------------------------------------
enum WeaponSound_t {
    WPS_INVALID = -1,
    WPS_FIRST_DONTUSE = 0,

    WPS_EMPTY = WPS_FIRST_DONTUSE,
    WPS_PRIMARY,
    WPS_PRIMARY_NPC,
    WPS_SECONDARY,
    WPS_SECONDARY_NPC,
    WPS_BURST,  // what the fuck is this?
    WPS_RELOAD,
    WPS_RELOAD_NPC,
    WPS_MELEE_SWING,
    WPS_MELEE_HIT,
    WPS_MELEE_HIT_WORLD,
    WPS_SPECIAL1,
    WPS_SPECIAL2,
    WPS_SPECIAL3,
    WPS_TAUNT,  // what the fuck?
    WPS_DEPLOY,

    WPS_NUM_TYPES,
};

#define MAX_SHOOT_SOUNDS            16  // Maximum number of shoot sounds per shoot type
#define MAX_WEAPON_STRING           80
#define MAX_WEAPON_PREFIX           16
#define MAX_WEAPON_AMMO_NAME        32
#define WEAPON_PRINTNAME_MISSING    "!!! Missing printname on weapon"

class CHudTexture;
class KeyValues;

//-----------------------------------------------------------------------------
// Purpose: Contains the data read from the weapon's script file.
// It's cached so we only read each weapon's script file once.
// Each game provides a CreateWeaponInfo function so it can have game-specific
// data (like CS move speeds) in the weapon script.
//-----------------------------------------------------------------------------
class WeaponInfo_t {
public:
    WeaponInfo_t();
    virtual void Parse( KeyValues *pKeyValuesData, const char *pszWeaponName );

public:
    bool    bLoadedHudElements = false;

// SHARED
    char    szClassName[MAX_WEAPON_STRING];
    char    szPrintName[MAX_WEAPON_STRING];                 // Name for showing in HUD, etc.

    char    szViewModel[MAX_WEAPON_STRING];                 // View model of this weapon
    char    szWorldModel[MAX_WEAPON_STRING];                // Model of this weapon seen carried by the player
    char    szAnimationPrefix[MAX_WEAPON_PREFIX];           // Prefix of the animations that should be used by the player carrying this weapon
    int     iSlot = 0;                                      // inventory slot.
    int     iPosition = 0;                                  // position in the inventory slot.
    int     iMaxClip1 = 0;                                  // max primary clip size (-1 if no clip)
    int     iMaxClip2 = 0;                                  // max secondary clip size (-1 if no clip)
    int     iDefaultClip1 = 0;                              // amount of primary ammo in the gun when it's created
    int     iDefaultClip2 = 0;                              // amount of secondary ammo in the gun when it's created
    int     iWeight = 0;                                    // this value used to determine this weapon's importance in autoselection.
    int     iRumbleEffect = 0;                              // Which rumble effect to use when fired? (xbox)
    bool    bAutoSwitchTo = false;                          // whether this weapon should be considered for autoswitching to
    bool    bAutoSwitchFrom = false;                        // whether this weapon can be autoswitched away from when picking up another weapon or ammo
    int     iFlags = 0;                                     // miscellaneous weapon flags
    char    szAmmo1[MAX_WEAPON_AMMO_NAME];                  // "primary" ammo type
    char    szAmmo2[MAX_WEAPON_AMMO_NAME];                  // "secondary" ammo type
    int     iAmmoType = 0;
    int     iAmmo2Type = 0;
    bool    m_bMeleeWeapon = false;                         // Melee weapons can always "fire" regardless of ammo.

    // This tells if the weapon was built right-handed (defaults to true).
    // This helps cl_righthand make the decision about whether to flip the model or not.
    bool    m_bBuiltRightHanded = false;
    bool    m_bAllowFlipping = false;                       // False to disallow flipping the model, regardless of whether
                                                            // it is built left or right handed.

    char    m_weaponSounds[WPS_NUM_TYPES][MAX_WEAPON_STRING];

    // CLIENT DLL
    // Sprite data, read from the data file
    int             iSpriteCount;
    CHudTexture *   iconActive;
    CHudTexture *   iconInactive;
    CHudTexture *   iconAmmo;
    CHudTexture *   iconAmmo2;
    CHudTexture *   iconCrosshair;
    CHudTexture *   iconAutoaim;
    CHudTexture *   iconZoomedCrosshair;
    CHudTexture *   iconZoomedAutoaim;
    CHudTexture *   iconSmall;
};

class WeaponParse {
private:
    static CUtlDict<WeaponInfo_t *, WeaponInfoHandle_t> m_WeaponInfoDatabase;
    static const char * m_aszWeaponSoundTable[WPS_NUM_TYPES];

private:
    static WeaponInfoHandle_t   FindWeaponInfo( const char *pszName );

public:
    static const char **        GetWeaponSoundTable();
    static const WeaponSound_t  FindWeaponSound( const char *pszName );
    
    static WeaponInfoHandle_t   LookupWeaponInfo( const char *pszName );
    static WeaponInfoHandle_t   GetInvalidWeaponInfo();
    
    static WeaponInfo_t *       GetWeaponInfo( WeaponInfoHandle_t hWpnInfo );
    static bool                 GetWeaponInfo( const char *pszName, WeaponInfoHandle_t &hWpnInfo );
    
    static void                 Initialize( IFileSystem *pFilesystem, const unsigned char *pICEKey );
};

//
// Read a possibly-encrypted KeyValues file in.
// If pICEKey is NULL, then it appends .txt to the filename and loads it as an unencrypted file.
// If pICEKey is non-NULL, then it appends .ctx to the filename and loads it as an encrypted file.
//
// (This should be moved into a more appropriate place).
//
KeyValues* ReadEncryptedKVFile( IFileSystem *filesystem, const char *szFilenameWithoutExtension, const unsigned char *pICEKey, bool bForceReadEncryptedFile = false );

// Each game implements this. It can return a derived class and override Parse() if it wants.
extern WeaponInfo_t* CreateWeaponInfo();


#endif // WEAPON_PARSE_H
