/*
 * Modification History
 *
 * 2002-June-26   Jason Rohrer
 * Created.
 *
 * 2003-August-18   Jason Rohrer
 * Parameterized the key length.
 */



#include "hex.h"
#include "rsa.h"
#include "randpool.h"
#include "files.h"
#include <string.h>
#include <stdio.h>

USING_NAMESPACE( CryptoPP );



// Copied from the Crypto++ test suite
void GenerateRSAKey( unsigned int keyLength, const char *privFilename,
					 const char *pubFilename, const char *seed ) {
	RandomPool randPool;
	randPool.Put( (byte *)seed, strlen( seed ) );
   
	RSAES_OAEP_SHA_Decryptor priv( randPool, keyLength );
	HexEncoder privFile( new FileSink( privFilename ) );
	priv.DEREncode( privFile );
	privFile.MessageEnd();

	RSAES_OAEP_SHA_Encryptor pub( priv );
	HexEncoder pubFile( new FileSink( pubFilename ) );
	pub.DEREncode( pubFile );
	pubFile.MessageEnd();
	}



int main( int inNumArgs, char **inArgs ) {

    if( inNumArgs < 2 ) {
        printf( "First argument must be key length in bits\n" );
        return 0;
        }

    int keyLength = 512;

    int numRead = sscanf( inArgs[1], "%d", &keyLength );

    if( numRead != 1 ) {
        printf( "First argument, key length must be an integer.\n" );
        return 0;
        }
    
	printf(  "Generating a key of length %d...\n", keyLength );


	GenerateRSAKey( keyLength, "testPrivate.key", "testPublic.key",
					"adfm38034mdsf" );
	
	printf( "done.\n" );
	return 0;
	}
