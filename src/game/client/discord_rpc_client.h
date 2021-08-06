#ifndef DISCORD_RPC_CLIENT_H
#define DISCORD_RPC_CLIENT_H
#ifdef _WIN32
#pragma once
#endif
#include "discord_rpc.h"
#include "igamesystem.h"

class IDiscordRPC : public CBaseGameSystemPerFrame {
public:
    virtual void SetPresenceState( const char *pszFmt, ... ) = 0;
    virtual void SetPresenceDetails( const char *pszFmt, ... ) = 0;
};
extern IDiscordRPC *g_pDiscordRPC;

#endif
