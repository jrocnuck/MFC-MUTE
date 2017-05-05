/*
 * Modification History
 *
 * 2003-August-21   Jason Rohrer
 * Created.
 *
 * 2003-August-22   Jason Rohrer
 * Fixed bugs in header.
 */



#ifndef SECURE_STREAM_FACTOR_INCUDED
#define SECURE_STREAM_FACTOR_INCUDED




#include "minorGems/io/InputStream.h"
#include "minorGems/io/OutputStream.h"




/**
 * A factory that constructs secure streams.
 *
 * Note that the random pool in CryptoUtils must be seeded before this
 * factory is used.
 *
 * @author Jason Rohrer
 */
class SecureStreamFactory {


    public:
        
        /**
         * Establishes secure streams on top of existing streams.
         *
         * @param inInputStream the input stream to wrap.
         *   Will be destroyed when the returned input stream is destroyed.
         * @param inOutputStream the output stream to wrap.
         *   Will be destroyed when the returned output stream is destroyed.
         *   Must be distinct from inInputStream (in other words, they cannot
         *   be pointers to the same object).
         * @param inRSAPublicKey the public portion of our RSA key as a
         *   DER-encoded, hex-encoded sting.
         *   Must be destroyed by caller.
         * @param inRSAPrivateKey the private portion of our RSA key as a
         *   DER-encoded, hex-encoded sting.
         *   Must be destroyed by caller.
         * @param outSecureInputStream pointer to where the established
         *   input stream should be returned.
         *   Will be set to NULL if setting up streams fails.
         *   Returned stream must be destroyed by caller if non-NULL.
         * @param outSecureOutputStream pointer to where the established
         *   output stream should be returned.
         *   Will be set to NULL if setting up streams fails.
         *   Returned stream must be destroyed by caller if non-NULL.
         *
         * @return true if setting up streams succeeds, or false if it fails.
         */   
        static char establishStreams( InputStream *inInputStream,
                                      OutputStream *inOutputStream,
                                      char *inRSAPublicKey,
                                      char *inRSAPrivateKey,
                                      InputStream **outSecureInputStream,
                                      OutputStream **outSecureOutputStream );

    
    
    };




#endif
