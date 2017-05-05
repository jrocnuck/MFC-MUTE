/*
 * Modification History
 *
 * 2003-August-21   Jason Rohrer
 * Created.
 */


#include "MUTE/common/CryptoUtils.h"
#include "MUTE/common/AESEncryptor.h"
#include "MUTE/common/AESDecryptor.h"

#include <stdio.h>
#include <string.h>



int main() {
    char testFailed = false;

    
    if( CryptoUtils::hasRandomGeneratorBeenSeeded() ) {
        printf( "Generator already seeded?\n" );
        testFailed = true;
        }
    else {
        printf( "Generator not yet seeded\n" );
        }

    
    printf( "Seeding generator\n" );

    CryptoUtils::seedRandomGenerator( "this is a test seed sdfousdfju" );

    if( CryptoUtils::hasRandomGeneratorBeenSeeded() ) {
        printf( "Seeding succeeded\n" );
        }
    else {
        printf( "Seeding failed\n" );
        testFailed = true;
        }

    printf( "Generated AES key:\n" );
    char *aesKey = CryptoUtils::generateAESKey();

    printf( "Key:\n%s\n", aesKey );


    printf( "Constructing an encryptor with default IV\n" );
    AESEncryptor *encryptor = new AESEncryptor( aesKey );

    char *message = "this is a test";
    printf( "Encrypting message \"%s\"\n", message );
    unsigned char *encryptedMessage =
        encryptor->encryptData( (unsigned char *)message, strlen( message ) );


    printf( "Constructing a decryptor with default IV\n" );
    AESDecryptor *decryptor = new AESDecryptor( aesKey );

    printf( "Decrypting message\n" );
    unsigned char *decryptedMessage =
        decryptor->decryptData( encryptedMessage, strlen( message ) );

    // returned message should be \0-terminated
    printf( "Message:\n%s\n", decryptedMessage );

    if( strcmp( (char *)decryptedMessage, message ) != 0 ) {
        testFailed = true;
        }

    delete encryptor;
    delete decryptor;
    delete [] decryptedMessage;
    delete [] encryptedMessage;

    printf( "Generating a random IV\n" );
    char *iv = CryptoUtils::getRandomHexString( strlen( aesKey ) / 2 );
    printf( "IV:\n%s\n", iv );
    

    printf( "Constructing an encryptor with specific IV\n" );
    encryptor = new AESEncryptor( aesKey, iv );

    printf( "Encrypting message \"%s\"\n", message );
    encryptedMessage =
        encryptor->encryptData( (unsigned char *)message, strlen( message ) );


    printf( "Constructing a decryptor with specific IV\n" );
    decryptor = new AESDecryptor( aesKey, iv );

    printf( "Decrypting message\n" );
    decryptedMessage =
        decryptor->decryptData( encryptedMessage, strlen( message ) );

    // returned message should be \0-terminated
    printf( "Message:\n%s\n", decryptedMessage );

    if( strcmp( (char *)decryptedMessage, message ) != 0 ) {
        testFailed = true;
        }

    delete [] decryptedMessage;
    delete decryptor;
    

    printf( "Constructing a decryptor with default IV\n" );
    decryptor = new AESDecryptor( aesKey );

    printf(
        "Trying to decrypt specific-IV message with default-IV decryptor\n" );
    decryptedMessage =
        decryptor->decryptData( encryptedMessage, strlen( message ) );

    // returned message should be \0-terminated
    printf( "Message:\n%s\n", decryptedMessage );

    if( strcmp( (char *)decryptedMessage, message ) == 0 ) {
        testFailed = true;
        }


    delete encryptor;
    delete decryptor;
    delete [] decryptedMessage;
    delete [] encryptedMessage;

    delete [] iv;
    
    
    
    delete [] aesKey;


    
    if( testFailed ) {
        printf( "\nSome tests failed\n" );
        }
    else {
        printf( "\nAll tests passed\n" );
        }
    
    return 0;
    }
   
