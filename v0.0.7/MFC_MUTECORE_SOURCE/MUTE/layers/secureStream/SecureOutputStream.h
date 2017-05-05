/*
 * Modification History
 *
 * 2003-August-22   Jason Rohrer
 * Created.
 */



#ifndef SECURE_OUTPUT_STREAM_INCLUDED
#define SECURE_OUTPUT_STREAM_INCLUDED



#include "minorGems/io/OutputStream.h"
#include "MUTE/common/AESEncryptor.h"



class SecureOutputStream : public OutputStream {


        
    public:


        
        /**
         * Constructs an output stream.
         *
         * @param inOutputStream the output stream to wrap.
         *   Will be destroyed when this class is destroyed.
         * @param inEncryptor the encryptor to use when
         *   writing data to inOutputStream.
         *   Will be destroyed when this class is destroyed.
         */ 
        SecureOutputStream( OutputStream *inOutputStream,
                            AESEncryptor *inEncryptor );

        
        virtual ~SecureOutputStream();
        

        
        // implements the OutputStream interface
        virtual long write( unsigned char *inBuffer, long inNumBytes );


        
    private:

        OutputStream *mOutputStream;
        AESEncryptor *mEncryptor;


        
    };



#endif
