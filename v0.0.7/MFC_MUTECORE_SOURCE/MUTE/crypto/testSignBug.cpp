/*
 * Modification History
 *
 * 2002-July-2   Jason Rohrer
 * Created.
 */


// This demonstrates that there is no bug in terms of different
// output from a block input and a byte-by-byte input to an
// accumulation signer.


#include "rng.h"
#include "hex.h"
#include "rsa.h"
#include "randpool.h"
#include "files.h"
#include "filters.h"

#include "default.h"



#include <string.h>
#include <stdio.h>
#include <stdlib.h>



USING_NAMESPACE( CryptoPP );
USING_NAMESPACE( std );



// prototypes:
void signStringByteByByte( const char *inString );
void signStringWholeBlock( const char *inString );
void printSignature();



const char *signatureFileName = "signature.txt";



void signStringByteByByte( const char *inString ) {

    // source for our private key file
    FileSource privFile( "privateKey.txt", true, new HexDecoder );
    
    // construct a hasher/signer using the private key
    RSASSA_PKCS1v15_SHA_Signer priv( privFile );
    
    // RSASSA_PKCS1v15_SHA_Signer ignores the rng. 
    // Use a real RNG for other signature schemes!
    NullRNG rng;

    // a filter based on this signer
    // should put result into signature file
    SignerFilter signer( rng, 
                         priv, 
                         new HexEncoder(
                             new FileSink( signatureFileName ) ) );


    // pass data blocks through filter, one char at a time
    int numBytes = strlen( inString );

    byte *data = (unsigned char *)inString;
    
    for( int i=0; i<numBytes; i++ ) {
        signer.Put( &( data[i] ), 1 );
        }

    // unlimited propagate here to flush file
    signer.MessageEnd( -1 );
    
    }



void signStringWholeBlock( const char *inString ) {

    // source for our private key file
    FileSource privFile( "privateKey.txt", true, new HexDecoder );
    
    // construct a hasher/signer using the private key
    RSASSA_PKCS1v15_SHA_Signer priv( privFile );
    
    // RSASSA_PKCS1v15_SHA_Signer ignores the rng. 
    // Use a real RNG for other signature schemes!
    NullRNG rng;

    // a filter based on this signer
    // should put result into signature file
    SignerFilter signer( rng, 
                         priv, 
                         new HexEncoder(
                             new FileSink( signatureFileName ) ) );


    // pass data blocks through filter as one large block
    int numBytes = strlen( inString );
    
    signer.Put( (unsigned char *)inString, numBytes );

    // unlimited propagate here to flush file
    signer.MessageEnd( -1 );
    }



void printSignature() {
    // now print the signature to the console        
    FILE *sigFile = fopen( signatureFileName, "r" );
    
    if( sigFile != NULL ) {
        int charReadAsInt = getc( sigFile );
		
        while( charReadAsInt != EOF ) {
            printf( "%c", (char)charReadAsInt );
            charReadAsInt = getc( sigFile );
                }
        fclose( sigFile );
        printf( "\n" );
        }
    else{
        printf( "Opening signature file failed.\n" );
        }

    }



int main() {

    char *stringToSign = "test";

    signStringWholeBlock( stringToSign );

    printf( "Signature for string signed as block:\n" );

    printSignature();


    signStringByteByByte( stringToSign );

    printf( "Signature for string signed byte-by-byte:\n" );

    printSignature();
    
    }
