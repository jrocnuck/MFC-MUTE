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
 * Added support for decrypting directly to/from hex encodings.
 */



#ifndef AES_DECRYPTOR_INCLUDED
#define AES_DECRYPTOR_INCLUDED


#include "MUTE/crypto/aes.h"
#include "MUTE/crypto/modes.h"



USING_NAMESPACE( CryptoPP )



/**
 * A class that can decrypt a series of data using AES.
 *
 * Uses CFB mode.
 *
 * @author Jason Rohrer.
 */
class AESDecryptor {

        
        
    public:



        /**
         * Constructs an decryptor.
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
        AESDecryptor( char *inKey, char *inInitializationVector = NULL );


        ~AESDecryptor();


        
        /**
         * Decrypts a block of data.
         *
         * @param inData the data to decrypt.
         *   Must be destroyed by caller.
         * @param inLength the length of the data in bytes.
         *
         * @return the decrypted data.
         *   Returned buffer contains inLength bytes along with \0 termination.
         *   Must be destroyed by caller.
         */
        unsigned char *decryptData( unsigned char *inData, int inLength );



        /**
         * Decrypts a single block of data.
         *
         * This function is useful for decrypting data without explicitly
         * constructing an AESDecryptor object.
         *
         * @param inData the data to decrypt.
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
         * @return the decrypted data.
         *   Returned buffer contains inLength bytes along with \0 termination.
         *   Must be destroyed by caller.
         */
        static unsigned char *aesDecrypt(
            unsigned char *inData,
            int inDataLength,
            char *inKey,
            char *inInitializationVector = NULL );



        /**
         * Similar to aesDecrypt, except encrypted block is returned
         * as a hex-encoded string.
         *
         * @return the decrypted data as a \0-terminated, hex-encoded string.
         *   Must be destroyed by caller.
         */
        static char *aesDecryptToHex(
            unsigned char *inData,
            int inDataLength,
            char *inKey,
            char *inInitializationVector = NULL );

        

        /**
         * Similar to aesDecryptToHex, except data is passed in
         * as a hex-encoded string.
         *
         * @param inHexData the data to decrypt as a \0-terminated,
         *   hex-encoded string.
         *   Must be destroyed by caller.
         *
         * @return the decrypted data as a \0-terminated, hex-encoded string.
         *   Must be destroyed by caller.
         */
        static char *aesDecryptHexToHex(
            char *inHexData,
            char *inKey,
            char *inInitializationVector = NULL );

        
        
    private:

        AESEncryption *mAES;
        CFBDecryption *mCFB;


        
    };



#endif
