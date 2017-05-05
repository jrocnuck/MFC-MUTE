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



#ifndef AES_ENCRYPTOR_INCLUDED
#define AES_ENCRYPTOR_INCLUDED


#include "MUTE/crypto/aes.h"
#include "MUTE/crypto/modes.h"



USING_NAMESPACE( CryptoPP )



/**
 * A class that can encrypt a series of data using AES.
 *
 * Uses CFB mode.
 *
 * @author Jason Rohrer.
 */
class AESEncryptor {

        
        
    public:



        /**
         * Constructs an encryptor.
         *
         * @param inKey the AES key as a hex-encoded string.
         *   Must be a valid AES length (128, 192, or 256 bits).
         *   Must be destroyed by caller.
         * @param inInitializationVector the initialization vector
         *   as a hex-encoded string, or NULL to use a default initialization
         *   vector (all zeros).
         *   If non-NULL, must be the same length as inKey.
         *   Must be destroyed by caller if non-NULL.
         */         
        AESEncryptor( char *inKey, char *inInitializationVector = NULL );


        ~AESEncryptor();


        
        /**
         * Encrypts a block of data.
         *
         * @param inData the data to encrypt.
         *   Must be destroyed by caller.
         * @param inLength the length of the data in bytes.
         *
         * @return the encrypted data.
         *   Returned buffer contains inLength bytes along with \0 termination.
         *   Must be destroyed by caller.
         */
        unsigned char *encryptData( unsigned char *inData, int inLength );



        /**
         * Encrypts a single block of data.
         *
         * This function is useful for encrypting data without explicitly
         * constructing an AESEncryptor object.
         *
         * @param inData the data to encrypt.
         *   Must be destroyed by caller.
         * @param inLength the length of the data in bytes.
         * @param inKey the AES key as a hex-encoded string.
         *   Must be a valid AES length (128, 192, or 256 bits).
         *   Must be destroyed by caller.
         * @param inInitializationVector the initialization vector
         *   as a hex-encoded string, or NULL to use a default initialization
         *   vector (all zeros).
         *   If non-NULL, must be the same length as inKey.
         *   Must be destroyed by caller if non-NULL.
         *
         * @return the encrypted data.
         *   Returned buffer contains inDataLength bytes along with
         *   \0 termination.
         *   Must be destroyed by caller.
         */
        static unsigned char *aesEncrypt(
            unsigned char *inData,
            int inDataLength,
            char *inKey,
            char *inInitializationVector = NULL );

        

        /**
         * Similar to aesEncrypt, except encrypted block is returned
         * as a hex-encoded string.
         *
         * @return the encrypted data as a \0-terminated, hex-encoded string.
         *   Must be destroyed by caller.
         */
        static char *aesEncryptToHex(
            unsigned char *inData,
            int inDataLength,
            char *inKey,
            char *inInitializationVector = NULL );

        

        /**
         * Similar to aesEncryptToHex, except data is passed in
         * as a hex-encoded string.
         *
         * @param inHexData the data to encrypt as a \0-terminated,
         *   hex-encoded string.
         *   Must be destroyed by caller.
         *
         * @return the encrypted data as a \0-terminated, hex-encoded string.
         *   Must be destroyed by caller.
         */
        static char *aesEncryptHexToHex(
            char *inHexData,
            char *inKey,
            char *inInitializationVector = NULL );
        

        
    private:

        AESEncryption *mAES;
        CFBEncryption *mCFB;


        
    };



#endif
