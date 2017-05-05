/*
 * Modification History
 *
 * 2003-August-8   Jason Rohrer
 * Created.
 *
 * 2003-August-11   Jason Rohrer
 * Added a user quit signal.
 *
 * 2003-August-24   Jason Rohrer
 * Added seeding of random number generator.
 */



#include "MUTE/layers/messageRouting/messageRouter.h"

#include "minorGems/system/Thread.h"


#include <stdio.h>



int main( int inNumArgs, char **inArgs ) {

    int ourPort = 9860;


    if( inNumArgs > 1 ) {
        // if scan fails, ourPort will remain at default value
        sscanf( inArgs[1], "%d", &ourPort );
        }
 

    // for test only.
    // should always use user input as seed.
    muteSeedRandomGenerator( "38sdf84jjkkb884rj7761uj" );

    
    muteStart( ourPort );
    muteSetTargetNumberOfConnections( 3 );


    // wait for quit signal
    char readChar;
    int numRead = scanf( "%c", &readChar );

    while( readChar != 'q' && readChar != 'Q' && numRead == 1 ) {
        numRead = scanf( "%c", &readChar );
        }
    
    
    muteStop();

    return 0;
    }
