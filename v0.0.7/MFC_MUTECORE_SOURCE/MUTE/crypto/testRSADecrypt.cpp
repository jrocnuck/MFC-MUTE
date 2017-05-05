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
char *RSADecryptString(const char *privFilename, const char *ciphertext)
{
    FileSource privFile(privFilename, true, new HexDecoder);
    RSAES_OAEP_SHA_Decryptor priv(privFile);

    HexDecoder hexDecoder;
    hexDecoder.Put((byte *)ciphertext, strlen(ciphertext));
    hexDecoder.MessageEnd();
    SecByteBlock buf(priv.CipherTextLength());
    hexDecoder.Get(buf, priv.CipherTextLength());

    char *outstr = new char[priv.MaxPlainTextLength()+1];
    unsigned messageLength = priv.Decrypt(buf, (byte *)outstr);
    outstr[messageLength] = 0;
    return outstr;
}



void usage( char *inAppName ) {

    printf( "Usage:\n" );
    printf( "\t%s priv_key_file encrypted_file\n", inAppName );

    printf( "Example:\n" );
    printf( "\t%s test.key encrypted.txt \n", inAppName );

    exit( 1 );
    }



int main( char inNumArgs, char **inArgs ) {

    if( inNumArgs != 3 ) {
        usage( inArgs[0] );
        }

    FILE *messageFile = fopen( inArgs[2], "r" );
    if( messageFile == NULL ) {
        printf( "Failed to open encrypted message file\n" );
        return 0;
        }

    char *message = new char[ 400 ];

    int numRead = fscanf( messageFile, "%s", message );
    fclose( messageFile );
    
    if( numRead != 1 ) {
        delete [] message;

        printf( "Failed to read encrypted message file\n" );
        return 0;
        }
        
    
    
    printf(  "Decrypting...\n" );

    char *result = RSADecryptString( inArgs[1], message );
    delete [] message;
    
    printf( "Decrypted message:\n%s\n", result );

    delete [] result;
    
    return 0;
    }
