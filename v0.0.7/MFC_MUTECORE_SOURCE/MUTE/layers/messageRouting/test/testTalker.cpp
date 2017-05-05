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

    char *ourVirtualAddress;
    char *partnerVirtualAddress;
    int ourPort = 9880;

    if( inNumArgs > 1 ) {
        ourVirtualAddress = inArgs[1];
        }
    else {
        ourVirtualAddress = "talkerA";
        }

    if( inNumArgs > 2 ) {
        // if scan fails, ourPort will remain at default value
        sscanf( inArgs[2], "%d", &ourPort );
        }
 
    if( inNumArgs > 3 ) {
        partnerVirtualAddress = inArgs[3];
        }
    else {
        partnerVirtualAddress = "talkerB";
        }


    // for test only.
    // should always use user input as seed.
    muteSeedRandomGenerator( "sd8487jsduf74unby" );
    
    
    muteStart( ourPort );
    muteSetTargetNumberOfConnections( 3 );
    
    muteAddReceiveAddress( ourVirtualAddress );



    while( true ) {
        int count = 0; 
        while( count == 0 ) {
            // send a message
            muteSendMessage( ourVirtualAddress, partnerVirtualAddress,
                         "test message" );
            
            // look for responses
            count = muteGetWaitingMessageCount( ourVirtualAddress );
            Thread::sleep( 5000 );
            }

        // get and discard received messages
        char **messages;
        char **fromAddresses;
        
        count = muteGetReceivedMessages( "nodeA", count,
                                         &messages, &fromAddresses );

        for( int i=0; i<count; i++ ) {
            delete [] messages[i];
            delete [] fromAddresses[i];
            }

        delete [] messages;
        delete [] fromAddresses;


        }

    muteStop();

    return 0;
    }
