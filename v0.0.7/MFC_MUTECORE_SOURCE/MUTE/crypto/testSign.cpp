/*
 * Modification History
 *
 * 2002-June-27   Jason Rohrer
 * Created.
 * Added a test of the accumulation signing method.
 *
 * 2002-June-28   Jason Rohrer
 * Changed to use heap allocation.
 * Added output of signature on console for accumulation scheme.
 * Worked on a bug with passing blocks of characters to SignerFilter.
 *
 * 2002-July-2   Jason Rohrer
 * Discovered the source of the bug.  See testSignBug.cpp
 */



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



// Copied from the Crypto++ test suite
void RSASignFile( const char *privFilename,
				  const char *messageFilename,
				  const char *signatureFilename ){
	
	FileSource privFile(privFilename, true, new HexDecoder);

	RSASSA_PKCS1v15_SHA_Signer priv(privFile);

	// RSASSA_PKCS1v15_SHA_Signer ignores the rng.
	// Use a real RNG for other signature schemes!
	NullRNG rng;
	
	FileSource f(
		messageFilename, true,
		new SignerFilter(
			rng, priv,
			new HexEncoder( new FileSink( signatureFilename ) ) ) );
	}



void RSAAccumulateSignFile( const char *privFilename,
							const char *messageFilename,
							const char *signatureFilename ) {

	FileSource *privFile = new FileSource( privFilename,
                                           true, new HexDecoder() );

	// construct a hasher/signer using the private key
	RSASSA_PKCS1v15_SHA_Signer *priv =
        new RSASSA_PKCS1v15_SHA_Signer( *privFile );

	// RSASSA_PKCS1v15_SHA_Signer ignores the rng. 
	// Use a real RNG for other signature schemes!
	NullRNG *rng = new NullRNG();

	// a filter based on this signer
	// should put result into signature file
	SignerFilter *signer = new SignerFilter(
        *rng, 
        *priv, 
        new HexEncoder( new FileSink( signatureFilename ) ) );


	FILE *messageFile = fopen( messageFilename, "r" );

	if( messageFile != NULL ) {

        
		int charReadAsInt = getc( messageFile );
		
		while( charReadAsInt != EOF ) {
			
			unsigned char charReadAsChar = (unsigned char)charReadAsInt;

            printf( "Putting %c into sig\n", charReadAsChar );
            
			signer->Put( &charReadAsChar, 1 );
            
			charReadAsInt = getc( messageFile );
			}

		fclose( messageFile );
        
        /*        unsigned char *stringToPut = (unsigned char *)"testa";
        printf( "Putting string:\n%s\n", stringToPut );
        
        signer->Put( stringToPut, 5 );
        */
        // this should finish the signature AND force it out to file
		signer->MessageEnd( -1 );  // -1 means unlimited propagation

        
        // now print the signature to the console        
        FILE *sigFile = fopen( signatureFilename, "r" );

        if( sigFile != NULL ) {
            printf( "Signature:\n" );
            int charReadAsInt = getc( sigFile );
		
            while( charReadAsInt != EOF ) {
                printf( "%c", (char)charReadAsInt );
                charReadAsInt = getc( sigFile );
                }
            fclose( sigFile );
            printf( "\n" );
            }
        
                
        }

    
    delete privFile;
    delete priv;
    delete rng;
    delete signer;


	} 



void usage( char *inAppName ) {

	printf( "Usage:\n" );
	printf( "\t%s private_key_file message_file signature_file\n", inAppName );

	printf( "Example:\n" );
	printf( "\t%s testPriv.key message.txt signature.txt \n", inAppName );

	exit( 1 );
	}



int main( char inNumArgs, char **inArgs ) {

	if( inNumArgs != 4 ) {
		usage( inArgs[0] );
		}
			   
	
	printf(  "Signing with first method ...\n" );

	RSASignFile( inArgs[1], inArgs[2], inArgs[3] );

	
	
	char *secondSigFileName = new char[ strlen( inArgs[3] ) + 2 ];

	sprintf( secondSigFileName, "%s2", inArgs[3] );


	printf(  "Signing with second method (into %s) ...\n",
			 secondSigFileName );

	RSAAccumulateSignFile( inArgs[1], inArgs[2], secondSigFileName );
	
	printf( "done.\n" );

	
	delete [] secondSigFileName;

	
	return 0;
	}
