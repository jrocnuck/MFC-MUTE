/*
 * Modification History
 *
 * 2003-August-18   Jason Rohrer
 * Created.
 */



#include "hex.h"
#include "randpool.h"
#include "files.h"
#include <string.h>
#include <stdio.h>

USING_NAMESPACE( CryptoPP );



// Copied from the Crypto++ test suite
void GenerateAESKey( unsigned int keyLength, const char *keyFilename,
                     const char *seed ) {
	RandomPool randPool;
	randPool.Put( (byte *)seed, strlen( seed ) );

    int keyLengthInBytes = keyLength / 8;
    
    unsigned char *key = new unsigned char[ keyLengthInBytes ];

    randPool.GenerateBlock( (byte *)key, keyLengthInBytes );

	HexEncoder keySink( new FileSink( keyFilename ) );
	keySink.Put( (byte *)key, keyLengthInBytes );
	keySink.MessageEnd();
	}



int main( int inNumArgs, char **inArgs ) {

    if( inNumArgs < 2 ) {
        printf( "First argument must be a random seed string.\n" );
        return 0;
        }
    
	printf(  "Generating a key of length 256...\n" );


	GenerateAESKey( 256, "testAES.key", inArgs[1] );
	
	printf( "done.\n" );
	return 0;
	}
