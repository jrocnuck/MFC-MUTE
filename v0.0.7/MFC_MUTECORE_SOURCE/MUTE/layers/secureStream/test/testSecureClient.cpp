/*
 * Modification History
 *
 * 2003-August-22   Jason Rohrer
 * Created.
 * Fixed message exchange.
 *
 * 2003-August-23   Jason Rohrer
 * Added output when secure streams established.
 */



#include <stdio.h>


#include "minorGems/network/Socket.h"
#include "minorGems/network/SocketClient.h"
#include "minorGems/network/SocketStream.h"
#include "minorGems/network/HostAddress.h"

#include "MUTE/common/CryptoUtils.h"
#include "MUTE/layers/secureStream/SecureStreamFactory.h"
#include "minorGems/io/InputStream.h"
#include "minorGems/io/OutputStream.h"

#include "minorGems/util/stringUtils.h"

#include "minorGems/network/p2pParts/protocolUtils.h"




int main( int inNumArgs, char **inArgs ) {

    if( inNumArgs != 2 ) {
        printf( "First arg must be connection port\n" );
        return 0;
        }

    int port;

    int numRead = sscanf( inArgs[1], "%d", &port );

    if( numRead != 1 ) {
        printf( "Port must be an integer\n" );
        return 0;
        }


    printf( "Generating our RSA keypair...\n" );
    // seed the rand generator
    CryptoUtils::seedRandomGenerator( "f8488sd7bjj477" );
    

    // make our key pair
    char *pubKey;
    char *privKey;
    CryptoUtils::generateRSAKey( 1024, &pubKey, &privKey );

    printf( "...done\n" );
    

    printf( "Connecting to localhost:%d\n", port );
    
    HostAddress *serverAddress =
        new HostAddress( stringDuplicate( "localhost" ),
                         port );

    Socket *sock = SocketClient::connectToServer( serverAddress );

    delete serverAddress;

    if( sock == NULL ) {
        printf( "Failed to connect to local server\n" );
        return 0;
        }

    printf( "Connection established.  Trying to establish secure streams.\n" );
    
    // separate streams
    SocketStream *inputStream = new SocketStream( sock );
    SocketStream *outputStream = new SocketStream( sock );

    
    

    InputStream *secureInput;
    OutputStream *secureOutput;

    char established = SecureStreamFactory::establishStreams( inputStream,
                                                              outputStream,
                                                              pubKey,
                                                              privKey,
                                                              &secureInput,
                                                              &secureOutput );

        
    delete [] pubKey;
    delete [] privKey;


    
    if( established ) {
        printf( "Secure streams established\n" );
        
        // send a message
        secureOutput->writeString( "This is a test from the client" );
        secureOutput->writeString( "\nEndMessage\n" );

        
        // get a message
        char *message = readStreamUpToTag( secureInput,
                                           "EndMessage",
                                           5000 );

        if( message != NULL ) {
            printf( "Got message:\n%s\n", message );
            delete [] message;
            }
        
        delete secureInput;
        delete secureOutput;
        }
    else {
        printf( "Failed to establish secure streams\n" );

        delete inputStream;
        delete outputStream;
        }
    
    delete sock;
    

    
    
    
        
    return 0;
    }
