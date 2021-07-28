#include "cbase.h"
#include "filesystem.h"
#include "openal/al_decoder.h"
#include "openal/al_decoder_mp3.h"
#include "openal/al_decoder_wav.h"
#include "openal/al_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CBaseALDecoder *AL_OpenFile( const char *pszFilename )
{
    const char *pszExtension = V_GetFileExtension( pszFilename );
    if( pszExtension ) {
        if( !Q_stricmp( pszExtension, "mp3" ) ) {
            CALDecoder_MP3 *pDecoder = new CALDecoder_MP3( filesystem );
            if( pDecoder->OpenFile( pszFilename ) )
                return pDecoder;
            delete pDecoder;
            return NULL;
        }

        if( !Q_stricmp( pszExtension, "wav" ) ) {
            CALDecoder_WAV *pDecoder = new CALDecoder_WAV( filesystem );
            if( pDecoder->OpenFile( pszFilename ) )
                return pDecoder;
            delete pDecoder;
            return NULL;
        }

        // TODO: .flac
        // TODO: .mod and .xm (PoC)

        DevWarning( "AL_OpenFile: unsupported extension %s\n", pszExtension );
        return NULL;
    }

    DevWarning( "AL_OpenFile: unable to parse file extension for %s\n", pszFilename );
    return NULL;
}

void AL_CloseFile( CBaseALDecoder *pDecoder )
{
    pDecoder->GetFilesystem()->Close( pDecoder->GetFileHandle() );
    delete pDecoder;
}
