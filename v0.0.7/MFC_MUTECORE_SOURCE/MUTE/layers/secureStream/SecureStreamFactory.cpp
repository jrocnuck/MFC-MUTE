/*
 * Modification History
 *
 * 2003-August-21   Jason Rohrer
 * Created.
 *
 * 2003-August-22   Jason Rohrer
 * Added key-establishment portion of protocol.
 * Added construction of secure stream objects.
 * Fixed a few bugs.
 * Added missing stream start tag.
 *
 * 2003-August-23   Jason Rohrer
 * Fixed a memory leak.
 *
 * 2004-January-26   Jason Rohrer
 * Added partial work-around for an exception-handling bug in older versions
 * of gcc.
 */



#include "SecureStreamFactory.h"
#include "SecureInputStream.h"
#include "SecureOutputStream.h"



#include "MUTE/common/CryptoUtils.h"
#include "MUTE/common/AESEncryptor.h"
#include "MUTE/common/AESDecryptor.h"

#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SimpleVector.h"
#include "minorGems/formats/encodingUtils.h"

#include "minorGems/network/p2pParts/protocolUtils.h"

#include "minorGems/util/log/AppLog.h"

#include <string.h>



char SecureStreamFactory::establishStreams(
    InputStream *inInputStream,
    OutputStream *inOutputStream,
    char *inRSAPublicKey,
    char *inRSAPrivateKey,
    InputStream **outSecureInputStream,
    OutputStream **outSecureOutputStream ) {

    char *loggerName = "SecureStreamFactory";
    

    *outSecureInputStream = NULL;
    *outSecureOutputStream = NULL;
    
    
    
    // send our public key
    char *publicKeyString = autoSprintf( "PublicKey: %s\nEndPublicKey\n",
                                         inRSAPublicKey );

    int expectedSent = strlen( publicKeyString );
    int numSent = inOutputStream->writeString( publicKeyString );
    
    delete [] publicKeyString;

    if( numSent != expectedSent ) {
        AppLog::error( loggerName, "Failed to send our public key" );
        return false;
        }
    

    // receive their public key
    // key is second token (index 1)
    char *theirPublicKey = readStreamUpToTagAndGetToken( inInputStream,
                                                         "EndPublicKey",
                                                         5000,
                                                         1 );

    
    // non-valid RSA keys are handled properly by CryptoUtils
    // by catching exceptions thrown by Crypto++

    // however, in older versions of gcc (2.95 and older),
    // the across-module exception handling is broken for larger linked apps
    // so a bad RSA key results in an app abort.

    // This was being triggered regularly by keys that were showing up as
    // the string "(null)", which must have been generated at the remote
    // node when printing a NULL key to the settings file... why would
    // this happen?

    // as a partial work-around, reject keys that are too short, which
    // will include "(null)" key strings.
    if( theirPublicKey != NULL ) {
        if( strlen( theirPublicKey ) < 10 ) {
            // key too short to be a valid RSA key
            char *logMessage =
                autoSprintf( "Their public key too short to be valid: "
                             "%s",
                             theirPublicKey );
            AppLog::error( loggerName, logMessage );
            delete [] logMessage;
            
            delete [] theirPublicKey;

            return false;
            }
        }

    
    if( theirPublicKey == NULL ) {
        AppLog::error( loggerName, "Failed to receive their public key" );
        return false;
        }
    
    
    // generate a fresh AES key that we will use for sending
    char *ourAESKeyHex =
        CryptoUtils::generateAESKey( CryptoUtils::AES_128_BIT );

    // assume that hex encoding produced by CryptoUtils is correct 
    unsigned char *ourRawAESKey = hexDecode( ourAESKeyHex );
    int ourRawAESKeyLength = strlen( ourAESKeyHex ) / 2;
    
    // encrypt our raw AES key with their public key, producing hex encoding
    char *ourRawAESKeyEncrypted =
        CryptoUtils::rsaEncrypt( theirPublicKey,
                                 ourRawAESKey,
                                 ourRawAESKeyLength );
    delete [] ourRawAESKey;
    delete [] theirPublicKey;
    

    if( ourRawAESKeyEncrypted == NULL ) {
        AppLog::error(
            loggerName,
            "Failed to encrypt our AES key with their public RSA key" );

        delete [] ourAESKeyHex;
        return false;
        }
    
    // send our AES key
    char *ourAESKeyString = autoSprintf( "AESKey: %s\nEndAESKey\n",
                                         ourRawAESKeyEncrypted );

    delete [] ourRawAESKeyEncrypted;

    expectedSent = strlen( ourAESKeyString );
    numSent = inOutputStream->writeString( ourAESKeyString );

    delete [] ourAESKeyString;

    if( numSent != expectedSent ) {
        AppLog::error( loggerName, "Failed to send our AES key" );

        delete [] ourAESKeyHex;
        return false;
        }


    // receive their AES key
    // key is second token (index 1)
    char *theirRawAESKeyEncrypted =
        readStreamUpToTagAndGetToken( inInputStream,
                                      "EndAESKey",
                                      5000,
                                      1 );

    if( theirRawAESKeyEncrypted == NULL ) {
        AppLog::error( loggerName, "Failed to receive their AES key" );

        delete [] ourAESKeyHex;
        return false;
        }
    

    // decrypt their AES key with our private key

    int theirRawAESKeyLength;
    unsigned char *theirRawAESKey =
        CryptoUtils::rsaDecrypt( inRSAPrivateKey,
                                 theirRawAESKeyEncrypted,
                                 &theirRawAESKeyLength );

    delete [] theirRawAESKeyEncrypted;
    
    if( theirRawAESKey == NULL || theirRawAESKeyLength != 16 ) {
        // decryption failed or their key is not 128 bits long
        if( theirRawAESKey != NULL ) {
            AppLog::error( loggerName, "Their AES key not 128 bits long" );
            delete [] theirRawAESKey;
            }
        else {
            AppLog::error( loggerName, "Failed to decrypt their AES key" );
            }
        delete [] ourAESKeyHex;
        return false;
        }

    
    char *theirAESKeyHex = hexEncode( theirRawAESKey, theirRawAESKeyLength );
    delete [] theirRawAESKey;


    // send the stream start tag
    inOutputStream->writeString( "Stream:" );

    // read their stream start tag
    char *upToStreamStart = readStreamUpToTag( inInputStream,
                                               "Stream:",
                                               100 );

    if( upToStreamStart == NULL ) {
        AppLog::error( loggerName,
                       "Failed to read the start of their stream" );

        delete [] ourAESKeyHex;
        delete [] theirAESKeyHex;
        return false;
        }

    delete [] upToStreamStart;

    
    // now we have both AES keys (one for sending, and one for receiving)
    // we are done with the key exchange portion of the protocol

    AESEncryptor *sendingEncryptor = new AESEncryptor( ourAESKeyHex );
    delete [] ourAESKeyHex;

    AESDecryptor *receivingDecryptor = new AESDecryptor( theirAESKeyHex );
    delete [] theirAESKeyHex;



    *outSecureOutputStream = new SecureOutputStream( inOutputStream,
                                                     sendingEncryptor );
    *outSecureInputStream = new SecureInputStream( inInputStream,
                                                   receivingDecryptor );

    return true;    
    }


