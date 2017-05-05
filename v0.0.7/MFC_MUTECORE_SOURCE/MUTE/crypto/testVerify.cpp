/*
 * Modification History
 *
 * 2002-July-3   Jason Rohrer
 * Created.
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
char RSAVerifyFile( const char *pubFilename,
                    const char *messageFilename,
                    const char *signatureFilename ) {
    FileSource pubFile(pubFilename, true, new HexDecoder);
    RSASSA_PKCS1v15_SHA_Verifier pub(pubFile);

    FileSource signatureFile(signatureFilename, true, new HexDecoder);
    if (signatureFile.MaxRetrievable() != pub.SignatureLength())
        return false;
    SecByteBlock signature(pub.SignatureLength());
    signatureFile.Get(signature, signature.size);

    VerifierFilter *verifierFilter = new VerifierFilter(pub);
    verifierFilter->PutSignature(signature);
    FileSource f(messageFilename, true, verifierFilter);

    
    byte result = 0;
    verifierFilter->Get(result);
    return result == 1;
    
    }



char RSAAccumulateVerifyFile( const char *pubFilename,
                              const char *messageFilename,
                              const char *signatureFilename ) {
    FileSource pubFile(pubFilename, true, new HexDecoder);
    RSASSA_PKCS1v15_SHA_Verifier pub(pubFile);

    FileSource signatureFile(signatureFilename, true, new HexDecoder);
    if (signatureFile.MaxRetrievable() != pub.SignatureLength())
        return false;
    SecByteBlock signature(pub.SignatureLength());
    signatureFile.Get(signature, signature.size);

    VerifierFilter *verifierFilter = new VerifierFilter(pub);


    FILE *messageFile = fopen( messageFilename, "r" );

    if( messageFile != NULL ) {

        
        int charReadAsInt = getc( messageFile );
        
        while( charReadAsInt != EOF ) {
            
            unsigned char charReadAsChar = (unsigned char)charReadAsInt;
            
            verifierFilter->Put( &charReadAsChar, 1 );
            
            charReadAsInt = getc( messageFile );
            }

        fclose( messageFile );

        // add signature before message end
        verifierFilter->PutSignature(signature);
        
        // unlimited propagation
        verifierFilter->MessageEnd( -1 );
        }
    
    byte result = 0;
    verifierFilter->Get(result);


    delete verifierFilter;

    
    return result == 1;
    }



void usage( char *inAppName ) {

    printf( "Usage:\n" );
    printf( "\t%s public_key_file message_file signature_file\n", inAppName );

    printf( "Example:\n" );
    printf( "\t%s testPub.key message.txt signature.txt \n", inAppName );

    exit( 1 );
    }



int main( char inNumArgs, char **inArgs ) {

    if( inNumArgs != 4 ) {
        usage( inArgs[0] );
        }
               
    
    printf(  "Verifying with first method ...\n" );

    char result = RSAVerifyFile( inArgs[1], inArgs[2], inArgs[3] );

    if( result ) {
        printf( "Signature is correct.\n" );
        }
    else {
        printf( "Signature is NOT correct.\n" );
        }


    printf(  "Verifying with second method ...\n" );

    result = RSAAccumulateVerifyFile( inArgs[1], inArgs[2], inArgs[3] );

    if( result ) {
        printf( "Signature is correct.\n" );
        }
    else {
        printf( "Signature is NOT correct.\n" );
        }

    
    return 0;
    }
