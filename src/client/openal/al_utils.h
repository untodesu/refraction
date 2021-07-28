#ifndef AL_UTILS_H
#define AL_UTILS_H 1

class CBaseALDecoder;

CBaseALDecoder *AL_OpenFile( const char *pszFilename );
void AL_CloseFile( CBaseALDecoder *pDecoder );

#endif
