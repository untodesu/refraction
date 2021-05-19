#include "tier1/strtools.h"
#include "tier0/icommandline.h"
#include "vraddll.h"

int main( int argc, char **argv )
{
    CommandLine()->CreateCmdLine( argc, argv );
    
    int iBothArg = 0;
    for(int i = 1; i < argc; i++) {
        if( !Q_stricmp( argv[i], "-both" ) ) {
            iBothArg = i;
            break;
        }
    }
    
    int iReturnValue = 0;
    
    for( int i = 0; i < 2; i++ ) {
        if( i && !iBothArg )
            continue;
        
        if( iBothArg )
            Q_strcpy( argv[iBothArg], i ? "-hdr" : "-ldr" );
        
        iReturnValue = g_pVRadDLL->main( argc, argv );
    }
    
    return iReturnValue;
}
