/*
 * Modification History
 *
 * 2003-August-18    Jason Rohrer
 * Created.
 *
 * 2003-August-21    Jason Rohrer
 * Added support for all AES key lengths.
 * Added missing termination to generated RSA keys.
 * Made getRandomHexString public and thread-safe.
 */



#include "CryptoUtils.h"

#include "MUTE/crypto/rsa.h"
#include "MUTE/crypto/hex.h"
#include "MUTE/crypto/rng.h"

// for some reason, it won't compile without this
#include "MUTE/crypto/default.h"

#include <string.h>


// instantiate static members
// E. Nuckols 04-03-2004 .. VisualC++ doesn't like assignment inside class def
#ifdef _MSC_VER
const int CryptoUtils::mPoolSize = 384;
const int CryptoUtils::AES_128_BIT = 0;
const int CryptoUtils::AES_192_BIT = 1;
const int CryptoUtils::AES_256_BIT = 2;
#endif


RandomPool CryptoUtils::mRandPool( mPoolSize );
char CryptoUtils::mPoolSeeded = false;
MutexLock CryptoUtils::mLock;
        


void CryptoUtils::seedRandomGenerator( char *inSeedString ) {
    mLock.lock();
    
    mRandPool.Put( (byte *)inSeedString, strlen( inSeedString ) );

    mPoolSeeded = true;

    mLock.unlock();
    }
                   



char CryptoUtils::hasRandomGeneratorBeenSeeded() {
    mLock.lock();

    char seeded = mPoolSeeded;

    mLock.unlock();

    return seeded;
    }



char *CryptoUtils::getRandomHexString( int inLength ) {
    unsigned char *block = new unsigned char[ inLength ];

    mLock.lock();
    mRandPool.GenerateBlock( (byte *)block, inLength );
    mLock.unlock();
    
    int hexStringLength = 2 * inLength;
    char *hexBlock = new char[ hexStringLength + 1 ];
    
    HexEncoder hexEncoder;
    hexEncoder.Put( (byte *)block, inLength );
    hexEncoder.MessageEnd();
    hexEncoder.Get( (byte *)hexBlock, hexStringLength );

    // terminate
    hexBlock[ hexStringLength ] = '\0';

    delete [] block;
    
    return hexBlock;
    }

        
        
char *CryptoUtils::getRandomGeneratorState() {
    return getRandomHexString( mPoolSize );
    }

        
        
char *CryptoUtils::generateAESKey( int inSize ) {
    int bitSize;
    if( inSize == AES_192_BIT ) {
        bitSize = 192;
        }
    else if( inSize == AES_256_BIT ) {
        bitSize = 256;
        }
    else {
        bitSize = 128;
        }
    return getRandomHexString( bitSize / 8 );
    } 



void CryptoUtils::generateRSAKey( int inKeyLengthInBits,
                                  char **outPublicKey,
                                  char **outPrivateKey ) {
    // lock around use of randpool
    mLock.lock();
    
    // first gen private key
    RSAES_OAEP_SHA_Decryptor priv( mRandPool, inKeyLengthInBits );

    mLock.unlock();
    

    // gen public key using private key
	RSAES_OAEP_SHA_Encryptor pub( priv );

        
    // now hex encode keys

    HexEncoder privateEncoder;
	priv.DEREncode( privateEncoder );
	privateEncoder.MessageEnd();
    int privateLength = privateEncoder.MaxRetrievable();

    *outPrivateKey = new char[ privateLength + 1 ];

    privateEncoder.Get( (byte *)(*outPrivateKey), privateLength );
    // terminate
    (*outPrivateKey)[ privateLength ] = '\0';
    

    HexEncoder publicEncoder;
	pub.DEREncode( publicEncoder );
	publicEncoder.MessageEnd();

    int publicLength = publicEncoder.MaxRetrievable();

    *outPublicKey = new char[ publicLength + 1 ];

    publicEncoder.Get( (byte *)(*outPublicKey), publicLength );

    // terminate
    (*outPublicKey)[ publicLength ] = '\0';
    }

        
        
int CryptoUtils::getMaximumEncryptionLength( char *inPublicKey ) {
    HexDecoder publicDecoder;
    publicDecoder.Put( (byte *)inPublicKey, strlen( inPublicKey ) );
    publicDecoder.MessageEnd();
    
    try {
        RSAES_OAEP_SHA_Encryptor pub( publicDecoder );
        return pub.MaxPlainTextLength();
        }
    catch( Exception inException ) {
        // bad key
        return -1;
        }
    }
        


char *CryptoUtils::rsaEncrypt( char *inPublicKey,
                               unsigned char *inData,
                               int inDataLength ) {

    if( getMaximumEncryptionLength( inPublicKey ) < inDataLength ) {
        return NULL;
        }

    HexDecoder publicDecoder;
    publicDecoder.Put( (byte *)inPublicKey, strlen( inPublicKey ) );
    publicDecoder.MessageEnd();
    
    try {
        // this will throw an exception if the key is bad
        RSAES_OAEP_SHA_Encryptor pub( publicDecoder );

        char *returnString = new char[ 2 * pub.CipherTextLength() + 1];
        
        // lock rand pool
        mLock.lock();
        pub.Encrypt( mRandPool,
                     (byte *)inData, inDataLength, (byte *)returnString );
        mLock.unlock();
        
    
        HexEncoder hexEncoder;
        hexEncoder.Put( (byte *)returnString, pub.CipherTextLength() );
        hexEncoder.MessageEnd();
        hexEncoder.Get( (byte *)returnString, 2 * pub.CipherTextLength() );
        
        returnString[ 2 * pub.CipherTextLength() ] = '\0';
        
        return returnString;    
        }
    catch( Exception inException ) {
        // bad key
        return NULL;
        }
    }


        
unsigned char *CryptoUtils::rsaDecrypt( char *inPrivateKey,
                                        char *inEncryptedData,
                                        int *outDecryptedDataLength ) {
    
    HexDecoder privateDecoder;
    privateDecoder.Put( (byte *)inPrivateKey, strlen( inPrivateKey ) );
    privateDecoder.MessageEnd();
    
    try {
        RSAES_OAEP_SHA_Decryptor priv( privateDecoder );

        HexDecoder hexDecoder;
        hexDecoder.Put( (byte *)inEncryptedData, strlen( inEncryptedData ) );
        hexDecoder.MessageEnd();

        if( hexDecoder.MaxRetrievable() != priv.CipherTextLength() ) {
            // bad encrypted message
            return NULL;
            }
        
        SecByteBlock buf( priv.CipherTextLength() );
        hexDecoder.Get( buf, priv.CipherTextLength() );
        
        unsigned char*returnString =
            new unsigned char[ priv.MaxPlainTextLength() + 1 ];
        unsigned messageLength = priv.Decrypt( buf, (byte *)returnString );
        returnString[ messageLength ] = '\0';
        
        *outDecryptedDataLength = messageLength;
        return returnString;
        }
    catch( Exception inException ) {
        // bad key
        return NULL;
        }
    }



char *CryptoUtils::rsaSign( char *inPrivateKey,
                            unsigned char *inData, int inDataLength ) {

    HexDecoder privateDecoder;
    privateDecoder.Put( (byte *)inPrivateKey, strlen( inPrivateKey ) );
    privateDecoder.MessageEnd();

    try {
        RSASSA_PKCS1v15_SHA_Signer priv( privateDecoder );

        // RSASSA_PKCS1v15_SHA_Signer ignores the rng.
        // Use a real RNG for other signature schemes!
        NullRNG rng;

        HexEncoder *signatureEncoder = new HexEncoder();

        // filter will push data through into hex encoder
        SignerFilter signatureFilter( rng, priv, signatureEncoder );

        signatureFilter.Put( (byte *)inData, inDataLength );

        // unlimited propagation... propagates to hex encoder
        signatureFilter.MessageEnd( -1 );


        // extract signature from hex encoder
        int sigLength = signatureEncoder->MaxRetrievable();
        char *signature = new char[ sigLength + 1 ];
        signatureEncoder->Get( (byte *)signature, sigLength );
        
        signature[ sigLength ] = '\0';

        return signature;        
        }
    catch( Exception inException ) {
        // bad key
        return NULL;
        }
    }



char CryptoUtils::rsaVerify( char *inPublicKey,
                             unsigned char *inData, int inDataLength,
                             char *inSignature ) {

    HexDecoder publicEncoder;
    publicEncoder.Put( (byte *)inPublicKey, strlen( inPublicKey ) );
    publicEncoder.MessageEnd();
    
    try {                   
        RSASSA_PKCS1v15_SHA_Verifier pub( publicEncoder );

        
        HexDecoder signatureDecoder;
        signatureDecoder.Put( (byte *)inSignature, strlen( inSignature ) );
        signatureDecoder.MessageEnd();

        if( signatureDecoder.MaxRetrievable() != pub.SignatureLength() ) {
            return false;
            }
    
        SecByteBlock signature( pub.SignatureLength() );
        signatureDecoder.Get( signature, signature.size );

        VerifierFilter verifierFilter( pub );
        verifierFilter.PutSignature( signature );

        verifierFilter.Put( (byte *)inData, inDataLength );
        // unlimited propagation
        verifierFilter.MessageEnd( -1 );

        
        byte result = 0;
        verifierFilter.Get( result );
        return ( result == 1 );
        }
    catch( Exception inException ) {
        // bad key
        return false;
        }
    }

