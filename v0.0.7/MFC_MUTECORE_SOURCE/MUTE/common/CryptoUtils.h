/*
 * Modification History
 *
 * 2003-August-18    Jason Rohrer
 * Created.
 *
 * 2003-August-21    Jason Rohrer
 * Added support for all AES key lengths.
 * Made getRandomHexString public and thread-safe.
 */



#ifndef CRYPTO_UTILS_INCLUDED
#define CRYPTO_UTILS_INCLUDED



#include "minorGems/system/MutexLock.h"

#include "MUTE/crypto/randpool.h"



USING_NAMESPACE( CryptoPP )



/**
 * A collection of static cryptography utility functions, including
 * a static, secure random number generator.
 *
 * All functions are thread-safe.
 *
 * @author Jason Rohrer.
 */
class CryptoUtils {

        
        
    public:

        
        
        /**
         * Seeds the random number generator.
         *
         * Should be called before calling any other function (for example,
         * at program startup).
         */
        static void seedRandomGenerator( char *inSeedString );


        
        /**
         * Gets whether the generator has been seeded.
         *
         * @return true if seeded, or false if not.
         */
        static char hasRandomGeneratorBeenSeeded();

        
        
        /**
         * Gets a state representation string for the random number generator.
         *
         * This string can be used to securely re-seed the random number
         * generator later, for example, at the next system startup.
         *
         * @return a hex-encoded state string.
         *   Must be destroyed by caller.
         */
        static char *getRandomGeneratorState();



        /**
         * Gets a hex-encoded random string from the random pool.
         *
         * @param inLength the length of the random data in bytes BEFORE
         *   it has been hex-encoded.
         *
         * @return the hex-encoded string of length 2*inLength.
         *   Must be destroyed by caller.
         */
        static char *getRandomHexString( int inLength );



		// E. Nuckols 04-03-2004 .. VisualC++ doesn't like assignment inside class def
#ifdef _MSC_VER
		static const int AES_128_BIT;
        static const int AES_192_BIT;
        static const int AES_256_BIT;
#else
        static const int AES_128_BIT = 0;
        static const int AES_192_BIT = 1;
        static const int AES_256_BIT = 2;
#endif


        
        /**
         * Generates a random AES key.
         *
         * @param inSize one of AES_128_BIT, AES_192_BIT, or AES_256_BIT.
         *   Defaults to AES_128_BIT.
         *
         * @return the key as a hex-encoded string.
         *   Must be destroyed by caller.
         */
        static char *generateAESKey( int inSize = AES_128_BIT ); 



        /**
         * Generates a random RSA key pari.
         *
         * @param inKeyLengthInBits the key length in bits.
         * @param outPublicKey pointer to where the public key should be
         *   returned.  A DER and hex-encoded key is returned here.
         *   Must be destroyed by caller.
         * @param outPrivateKey pointer to where the private key should be
         *   returned.  A DER and hex-encoded key is returned here.
         *   Must be destroyed by caller.
         */
        static void generateRSAKey( int inKeyLengthInBits,
                                    char **outPublicKey,
                                    char **outPrivateKey );

        
        
        /**
         * Gets the maximum data length that is directly encryptable by
         * an RSA key.
         *
         * @param inPublicKey the public key as a DER and hex-encoded string.
         *   Must be destroyed by caller.
         *
         * @return the maximum encryptable data length in bytes, or -1
         *   if the key is not valid.
         */
        static int getMaximumEncryptionLength( char *inPublicKey );
        


        /**
         * Encrypts data with RSA (OAEP and SHA).
         *
         * @param inPublicKey the DER, hex-encoded public key string.
         *   Must be destroyed by caller.
         * @param inData the data to encrypt.
         *   Must be destroyed by caller.
         * @param inDataLength the length of the data.
         *
         * @return the encrypted data as a hex-encoded string, or NULL
         *   if encryption fails.
         *   Must be destroyed by caller if non-NULL.
         */
        static char *rsaEncrypt( char *inPublicKey,
                                 unsigned char *inData, int inDataLength );


        
        /**
         * Decrypts data with RSA (OAEP and SHA).
         *
         * @param inPrivateKey the DER, hex-encoded private key string.
         *   Must be destroyed by caller.
         * @param inEncryptedData the hex-encoded encrypted data string.
         *   Must be destroyed by caller.
         * @param outDecryptedDataLength a pointer to where the length
         *   of the data should be returned.
         *
         * @return the unencrypted data as a \0-terminated string, or NULL
         *   if encryption fails.
         *   Must be destroyed by caller if non-NULL.
         */
        static unsigned char *rsaDecrypt( char *inPrivateKey,
                                          char *inEncryptedData,
                                          int *outDecryptedDataLength );



        /**
         * Signs data with RSA (SSA PKCS1v15 SHA).
         *
         * @param inPrivateKey the DER, hex-encoded private key string.
         *   Must be destroyed by caller.
         * @param inData the data to sign.
         *   Must be destroyed by caller.
         * @param inDataLength the length of the data.
         *
         * @return the signature as a hex-encoded string, or NULL if signing
         *   fails.
         *   Must be destroyed by caller if non-NULL.
         */
        static char *rsaSign( char *inPrivateKey,
                              unsigned char *inData, int inDataLength );



        /**
         * Verifies a data signature with RSA (SSA PKCS1v15 SHA).
         *
         * @param inPublicKey the DER, hex-encoded public key string.
         *   Must be destroyed by caller.
         * @param inData the data that has been signed.
         *   Must be destroyed by caller.
         * @param inDataLength the length of the data.
         * @param inSignature the hex-encoded signature string.
         *   Must be destroyed by caller.
         *
         * @return true if the signature is valid, or false otherwise.
         */
        static char rsaVerify( char *inPublicKey,
                               unsigned char *inData, int inDataLength,
                               char *inSignature );

        

    private:


        // why is the pool size 384?  seems to be a standard size
		// E. Nuckols 04-03-2004 .. VisualC++ doesn't like assignment inside class def
#ifdef _MSC_VER
		const static int mPoolSize;
#else
		const static int mPoolSize = 384;
#endif
        static RandomPool mRandPool;
        static char mPoolSeeded;
        static MutexLock mLock;

        



        
    };
#endif
