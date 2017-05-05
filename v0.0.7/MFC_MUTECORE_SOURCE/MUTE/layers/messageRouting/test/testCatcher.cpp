/*
 * Modification History
 *
 * 2003-August-8   Jason Rohrer
 * Created.
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
    muteSeedRandomGenerator( "s82390kkkf84msmbif" );

    
    muteStart( ourPort );
    muteSetTargetNumberOfConnections( 10 );

    // sleep forever
    while( true ) {
        Thread::sleep( 5000 );
        }

    
    muteStop();

    return 0;
    }
