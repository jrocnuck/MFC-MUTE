/*
 * Modification History
 *
 * 2003-August-15   Jason Rohrer
 * Created.
 *
 * 2003-August-21   Jason Rohrer
 * Fixed IV size.
 */


#include "aes.h"
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


void AESDecryptFile( const char *keyFilename,
                     const char *messageFilename,
                     const char *outputFilename ) {

   // hex decoding halvs length
    int bufferLength = 256;
    char *keyBuffer = new char[ bufferLength + 1 ];

    // add string termination
    keyBuffer[ bufferLength ] = '\0';
    

    // read in the key
    FileSource keyFile( keyFilename, true,
                        new HexDecoder(
                            new ArraySink( (unsigned char*)keyBuffer,
                                           bufferLength ) ) );

    // iv is all-zero
    char *iv = new char[ 32 ];
    for( int i=0; i<32; i++ ) {
        iv[i] = 0;
        }
    
    AESEncryption aesEncryption( (byte *)keyBuffer, 32 );
    CFBDecryption cfbDecryption( aesEncryption, (byte *)iv );


    FILE *messageFile = fopen( messageFilename, "r" );
    FILE *outputFile = fopen( outputFilename, "wb" );
    
	if( messageFile != NULL && outputFile != NULL ) {

        char *oneCharInputString = new char[2];
        oneCharInputString[1] = '\0';

        char *oneCharOutputString = new char[2];
        oneCharOutputString[1] = '\0';

        
		int charReadAsInt = getc( messageFile );
		
		while( charReadAsInt != EOF ) {
			
			unsigned char charReadAsChar = (unsigned char)charReadAsInt;

            oneCharInputString[0] = charReadAsChar;
           
            cfbDecryption.ProcessString( (byte *)oneCharOutputString,
                                         (byte *)oneCharInputString, 1 );

            // FIXME:  need to hex-encode output
            fprintf( outputFile, "%s", oneCharOutputString );

            
			charReadAsInt = getc( messageFile );
			}
        
		fclose( messageFile );
        fclose( outputFile );

        delete [] oneCharOutputString;
        delete [] oneCharInputString;
        }

    delete [] keyBuffer;
    delete [] iv;
    }



void usage( char *inAppName ) {

	printf( "Usage:\n" );
	printf( "\t%s key_file message_file output_file\n", inAppName );

	printf( "Example:\n" );
	printf( "\t%s test.key message.txt output.txt \n", inAppName );

	exit( 1 );
	}



int main( char inNumArgs, char **inArgs ) {

	if( inNumArgs != 4 ) {
		usage( inArgs[0] );
		}
			   
	
	printf(  "Decrypting...\n" );

	AESDecryptFile( inArgs[1], inArgs[2], inArgs[3] );
	
	printf( "done.\n" );
	
	return 0;
	}
