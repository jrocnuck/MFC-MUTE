/*
 * Modification History
 *
 * 2003-August-18   Jason Rohrer
 * Created.
 */


#include "rsa.h"
#include "modes.h"
#include "rng.h"
#include "hex.h"
#include "rsa.h"
#include "randpool.h"
#include "files.h"
#include "filters.h"

// for some reason, it won't compile without this
#include "default.h"




#include <string.h>
#include <stdio.h>
#include <stdlib.h>



USING_NAMESPACE( CryptoPP );
USING_NAMESPACE( std );



// copied from crypto++ test program
char *RSAEncryptString( const char *inPubKeyFilename, const char *seed,
                        const char *message )
{
    FileSource pubFile(inPubKeyFilename, true, new HexDecoder);
    RSAES_OAEP_SHA_Encryptor pub(pubFile);

    printf( "Maximum message length for this key is %d\n",
            pub.MaxPlainTextLength() );
    
    if (strlen(message) > pub.MaxPlainTextLength())
    {
        cerr << "message too long for this key\n";
        abort();
    }

    // for padding
    RandomPool randPool;
    randPool.Put((byte *)seed, strlen(seed));

    char *outstr = new char[2*pub.CipherTextLength()+1];
    pub.Encrypt( randPool, (byte *)message, strlen(message), (byte *)outstr );

    HexEncoder hexEncoder;
    hexEncoder.Put((byte *)outstr, pub.CipherTextLength());
    hexEncoder.MessageEnd();
    hexEncoder.Get((byte *)outstr, 2*pub.CipherTextLength());

    outstr[2*pub.CipherTextLength()] = '\0';
    return outstr;
}



void usage( char *inAppName ) {

    printf( "Usage:\n" );
    printf( "\t%s pub_key_file seed_string message_string output_file\n",
            inAppName );

    printf( "Example:\n" );
    printf( "\t%s test.key sadfu48798f this_is_a_test_message out.txt\n",
            inAppName );

    exit( 1 );
    }



int main( char inNumArgs, char **inArgs ) {

    if( inNumArgs != 5 ) {
        usage( inArgs[0] );
        }
               
    
    printf(  "Encrypting...\n" );

    char *result = RSAEncryptString( inArgs[1], inArgs[2], inArgs[3] );
    
    printf( "Encrypted message:\n%s\n", result );

    FILE *outputFile = fopen( inArgs[4], "w" );

    if( outputFile != NULL ) {
        fprintf( outputFile, "%s", result );
        fclose( outputFile );
        }
    else {
        printf( "Failed to open output file for writing.\n" );
        }
    
    delete [] result;
    
    return 0;
    }
