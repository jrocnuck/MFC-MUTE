/*
 * Modification History
 *
 * 2003-July-27   Jason Rohrer
 * Created.
 *
 * 2003-August-24   Jason Rohrer
 * Added seeding of random number generator.
 *
 * 2003-August-25   Jason Rohrer
 * Changed to use a message handler function.
 */



#include "MUTE/layers/messageRouting/messageRouter.h"

#include "minorGems/system/Thread.h"


#include <stdio.h>


void messageHandler( char *inFromAddress, char *inToAddress, char *inBody,
                     void *inExtraArgument ) {
    
    printf( "Got message from %s to %s:\n%s\n", inFromAddress,
            inToAddress, inBody );

    }


int main() {

    // for test only.
    // should always use user input as seed.
    muteSeedRandomGenerator( "44923sdj387sjf7889bjdrf" );
    
    muteStart( 9870 );

    int handlerID = muteAddMessageHandler( messageHandler, (void *)NULL );

    muteAddReceiveAddress( "nodeA" );
    
    muteAddHost( "localhost", 9871 );


    // sleep while we wait for messages to arive
    Thread::sleep( 20000 );
    /*
    int count = muteGetWaitingMessageCount( "nodeA" ); 
    if( count > 0 ) {

        printf( "Got a message\n" );

        char **messages;
        char **fromAddresses;
        
        count = muteGetReceivedMessages( "nodeA", count,
                                         &messages, &fromAddresses );

        for( int i=0; i<count; i++ ) {

            printf( "message %d, from %s:\n", i, fromAddresses[i] );
            printf( "%s\n", messages[i] );

            delete [] messages[i];
            delete [] fromAddresses[i];
            }

        delete [] messages;
        delete [] fromAddresses;
        }
    */

    muteRemoveMessageHandler( handlerID );

    muteStop();
    }
