/*
 * Modification History
 *
 * 2003-August-28   Jason Rohrer
 * Created.
 */



#include "MUTE/layers/messageRouting/messageRouter.h"

#include "minorGems/system/Thread.h"


#include <stdio.h>



char *readUserString( char *inPrompt ) {

    int numRead = 0;

    char *buffer = new char[1000];
    
    while( numRead != 1 ) {
        printf( inPrompt );
        fflush( stdout );
        
        numRead = scanf( "%999s", buffer );
        }
    return buffer;
    }


int main( int inNumArgs, char **inArgs ) {

    int ourPort = 9860;


    if( inNumArgs > 1 ) {
        // if scan fails, ourPort will remain at default value
        sscanf( inArgs[1], "%d", &ourPort );
        }
 
    char *randomSeed = readUserString( "Enter some randomness: " );
    muteSeedRandomGenerator( randomSeed );

    delete [] randomSeed;

    printf( "Listening for connections on port %d\n", ourPort );
    
    muteStart( ourPort );
    muteSetTargetNumberOfConnections( 0 );

    // sleep forever
    while( true ) {
        Thread::sleep( 5000 );
        }

    
    muteStop();

    return 0;
    }
