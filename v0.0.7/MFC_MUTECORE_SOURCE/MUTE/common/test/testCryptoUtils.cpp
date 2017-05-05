/*
 * Modification History
 *
 * 2003-August-19   Jason Rohrer
 * Created.
 *
 * 2003-August-21   Jason Rohrer
 * Added missing test of rand pool state extraction.
 * Added check for bad RSA-decrypted message.
 *
 * 2004-January-26   Jason Rohrer
 * Added extra test cases.
 */


#include "MUTE/common/CryptoUtils.h"

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

    delete [] aesKey;



    int keyLength = 1024;
    char *pubKey;
    char *privKey;

    printf( "Generating %d-bit RSA key:\n", keyLength );
    CryptoUtils::generateRSAKey( keyLength, &pubKey, &privKey );
    
    printf( "Public: %s\nPrivate: %s\n",
            pubKey, privKey );

    int maxLength = CryptoUtils::getMaximumEncryptionLength( pubKey );
    
    printf( "Max encryption length is %d bytes\n", maxLength );

    if( maxLength < 1 ) {
        testFailed = true;
        }

    maxLength = CryptoUtils::getMaximumEncryptionLength( "AE00FF87" );
    printf( "Encryption length of bad key is %d bytes\n", maxLength );

    if( maxLength != -1 ) {
        testFailed = true;
        }


    maxLength = CryptoUtils::getMaximumEncryptionLength( "" );
    printf( "Encryption length of empty key is %d bytes\n", maxLength );

    if( maxLength != -1 ) {
        testFailed = true;
        }


    maxLength = CryptoUtils::getMaximumEncryptionLength( "null" );
    printf( "Encryption length of bad-hex key is %d bytes\n", maxLength );

    if( maxLength != -1 ) {
        testFailed = true;
        }

    

    char *message = "a test message";

    printf( "Encrypting message (\"%s\") using public key\n", message );

    char *encryptedMessage = CryptoUtils::rsaEncrypt( pubKey,
                                                      (unsigned char*)message,
                                                      strlen( message ) );

    if( encryptedMessage != NULL ) {
        printf( "Encrypted message:\n%s\n", encryptedMessage );

        printf( "Decrypting message\n" );
        int length;
        char *decryptedMessage =
            (char *)( CryptoUtils::rsaDecrypt( privKey,
                                            encryptedMessage,
                                            &length ) );

        if( decryptedMessage != NULL ) {
            printf( "message: \"%s\"\n", decryptedMessage );

            if( strcmp( decryptedMessage, message ) != 0 ) {
                testFailed = true;
                }
            
            delete [] decryptedMessage;
            }
        else {
            printf( "Failed\n" );
            testFailed = true;
            }


        printf( "Decrypting with a bad key\n" );
        length;
        char *result = (char *)( CryptoUtils::rsaDecrypt( "A88",
                                                          encryptedMessage,
                                                          &length ) );
        if( result == NULL ) {
            printf( "Failed\n" );
            }
        else {
            printf( "Succeeded?\n" );
            
            delete [] result;
            testFailed = true;
            }
        
        delete [] encryptedMessage;
        }
    else {
        printf( "Encryption failed\n" );
        testFailed = true;
        }
    


    char *longMessage = "a very very very very very very long long "
        "very very very very very very long long test message"
        "very very very very very very long long test message";

    printf( "Encrypting over-long message (\"%s\") using public key\n",
            longMessage );

    char *encryptedLongMessage =
        CryptoUtils::rsaEncrypt( pubKey,
                                 (unsigned char *)longMessage,
                                 strlen( longMessage ) );

    if( encryptedLongMessage != NULL ) {
        printf( "Encrypted message:\n%s\n", encryptedLongMessage );

        delete [] encryptedLongMessage;
        testFailed = true;
        }
    else {
        printf( "Encryption failed (as expected)\n" );
        }


    printf( "Encrypting with a bad key\n" );
    char *result = CryptoUtils::rsaEncrypt( "A88",
                                            (unsigned char *)message,
                                            strlen( message ) );
    if( result == NULL ) {
        printf( "Failed\n" );
        }
    else {
        printf( "Succeeded?\n" );

        delete [] result;
        testFailed = true;
        }


    printf( "Decrypting a bad message with a good key\n" );
    int length;
    result = (char *)( CryptoUtils::rsaDecrypt( privKey,
                                                "AFF8",
                                                &length ) );
    if( result == NULL ) {
        printf( "Failed\n" );
        }
    else {
        printf( "Succeeded?\n" );

        delete [] result;
        testFailed = true;
        }


    printf( "Signing message\n" );
    char *signature = CryptoUtils::rsaSign( privKey,
                                            (unsigned char *)message,
                                            strlen( message ) );

    if( signature != NULL ) {
        printf( "Signature: %s\n", signature );

        printf( "Verifying signature\n" ); 
        char verified = CryptoUtils::rsaVerify( pubKey,
                                                (unsigned char *)message,
                                                strlen( message ),
                                                signature );
        if( verified ) {
            printf( "Signature good\n" );
            }
        else {
            printf( "Signature bad\n" );
            testFailed = true;
            }


        printf( "Verifying signature with a bad key\n" ); 
        verified = CryptoUtils::rsaVerify( "A8003F",
                                           (unsigned char *)message,
                                           strlen( message ),
                                           signature );
        if( verified ) {
            printf( "Signature good?n" );
            testFailed = true;
            }
        else {
            printf( "Signature bad\n" );
            }
        
        
        delete [] signature;
        }
    else {
        printf( "Failed\n" );
        testFailed = true;
        }


    printf( "Signing with a bad key\n" );
    signature = CryptoUtils::rsaSign( "AE009",
                                      (unsigned char *)message,
                                      strlen( message ) );
    if( signature == NULL ) {
        printf( "Failed\n" );
        }
    else {
        printf( "Succeeded?\n" );
        testFailed = true;
        }


    printf( "Verifying a bad signature\n" ); 
    char verified = CryptoUtils::rsaVerify( pubKey,
                                            (unsigned char *)message,
                                            strlen( message ),
                                            "AE9FA" );
    if( verified ) {
        printf( "Signature good?n" );
        testFailed = true;
        }
    else {
        printf( "Signature bad\n" );
        }


    delete [] pubKey;
    delete [] privKey;


    char *poolState = CryptoUtils::getRandomGeneratorState();

    printf( "Random pool state:\n%s\n", poolState );

    delete [] poolState;

    if( testFailed ) {
        printf( "\nSome tests failed\n" );
        }
    else {
        printf( "\nAll tests passed\n" );
        }
    
    return 0;
    }
   
