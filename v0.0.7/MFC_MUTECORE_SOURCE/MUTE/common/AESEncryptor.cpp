/*
 * Modification History
 *
 * 2003-August-21    Jason Rohrer
 * Created.
 *
 * 2003-September-15    Jason Rohrer
 * Added static functions for single-block encryption and decryption.
 *
 * 2003-October-3    Jason Rohrer
 * Added support for encrypting directly to/from hex encodings.
 */



#include "AESEncryptor.h"



#include "MUTE/crypto/hex.h"

#include "minorGems/formats/encodingUtils.h"

#include <string.h>



AESEncryptor::AESEncryptor( char *inKey,
                            char *inInitializationVector )
    : mAES( NULL ), mCFB( NULL ) {

    // decode key
    HexDecoder keyDecoder;

    keyDecoder.Put( (byte *)inKey, strlen( inKey ) );
    keyDecoder.MessageEnd();

    int decodedKeyLength = keyDecoder.MaxRetrievable();

    unsigned char *decodedKey = new unsigned char[ decodedKeyLength ];

    keyDecoder.Get( (byte *)decodedKey, decodedKeyLength ); 


    // now construct IV
    unsigned char *iv;
    char ivPassedIn = false;
    if( inInitializationVector != NULL ) {
        // use passed-in IV

        HexDecoder ivDecoder;

        ivDecoder.Put( (byte *)inInitializationVector,
                       strlen( inInitializationVector ) );
        ivDecoder.MessageEnd();

        int decodedIVLength = ivDecoder.MaxRetrievable();

        if( decodedIVLength == decodedKeyLength ) {
            ivPassedIn = true;
            iv = new unsigned char[ decodedIVLength ];
            
            ivDecoder.Get( (byte *)iv, decodedIVLength );
            }
        else {
            // iv not the same length as key
            ivPassedIn = false;
            }
        }

    if( !ivPassedIn ){
        // make all-zero IV that is same length as key
        iv = new unsigned char[ decodedKeyLength ];
        for( int i=0; i<decodedKeyLength; i++ ) {
            iv[ i ] = 0;
            }
        }
    
    
    mAES = new AESEncryption( (byte *)decodedKey, decodedKeyLength );
    
    mCFB = new CFBEncryption( *mAES, (byte *)iv );

    delete [] decodedKey;
    delete [] iv;
    }



AESEncryptor::~AESEncryptor() {

    delete mCFB;
    delete mAES;
    }



unsigned char *AESEncryptor::encryptData( unsigned char *inData,
                                          int inLength ) {

    unsigned char *returnData = new unsigned char[ inLength + 1 ];
    
    mCFB->ProcessString( (byte *)returnData,
                         (byte *)inData,
                         inLength );

    returnData[ inLength ] = '\0';

    return returnData;
    }



unsigned char *AESEncryptor::aesEncrypt(
    unsigned char *inData,
    int inDataLength,
    char *inKey,
    char *inInitializationVector ) {

    AESEncryptor *encryptor =
        new AESEncryptor( inKey, inInitializationVector );

    unsigned char *messageEncrypted =
        encryptor->encryptData( inData, inDataLength );

    delete encryptor;

    return messageEncrypted;
    }



char *AESEncryptor::aesEncryptToHex(
    unsigned char *inData,
    int inDataLength,
    char *inKey,
    char *inInitializationVector ) {

    unsigned char *rawEncryptedData =
        aesEncrypt( inData, inDataLength, inKey, inInitializationVector );

    char *returnString = hexEncode( rawEncryptedData, inDataLength );

    delete [] rawEncryptedData;

    return returnString;
    }

        
char *AESEncryptor::aesEncryptHexToHex(
    char *inHexData,
    char *inKey,
    char *inInitializationVector ) {

    unsigned char *rawData = hexDecode( inHexData );

    char *returnString = aesEncryptToHex( rawData,
                                          strlen( inHexData ) / 2,
                                          inKey,
                                          inInitializationVector );

    delete [] rawData;

    return returnString;
    }

