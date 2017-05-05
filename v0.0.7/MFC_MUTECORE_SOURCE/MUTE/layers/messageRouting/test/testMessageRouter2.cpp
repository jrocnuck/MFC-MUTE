/*
 * Modification History
 *
 * 2003-July-27   Jason Rohrer
 * Created.
 *
 * 2003-August-24   Jason Rohrer
 * Added seeding of random number generator.
 */



#include "MUTE/layers/messageRouting/messageRouter.h"

#include "minorGems/system/Thread.h"


#include <stdio.h>



int main() {

    // for test only.
    // should always use user input as seed.
    muteSeedRandomGenerator( "f89329dskksdfi948kfk8" );
    

    muteStart( 9871 );

    muteAddReceiveAddress( "nodeB" );

    Thread::sleep( 10000 );

    muteSendMessage( "nodeB", "nodeA", "test message" );    

    Thread::sleep( 20000 );
    
    muteStop();
    }
