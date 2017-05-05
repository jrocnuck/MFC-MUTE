/*
 * Modification History
 *
 * 2003-August-22   Jason Rohrer
 * Created.
 */



#ifndef SECURE_INPUT_STREAM_INCLUDED
#define SECURE_INPUT_STREAM_INCLUDED



#include "minorGems/io/InputStream.h"
#include "MUTE/common/AESDecryptor.h"



class SecureInputStream : public InputStream {


        
    public:


        
        /**
         * Constructs an input stream.
         *
         * @param inInputStream the input stream to wrap.
         *   Will be destroyed when this class is destroyed.
         * @param inDecryptor the decryptor to use when
         *   reading data from inInputStream.
         *   Will be destroyed when this class is destroyed.
         */ 
        SecureInputStream( InputStream *inInputStream,
                            AESDecryptor *inDecryptor );

        
        virtual ~SecureInputStream();
        

        
        // implements the InputStream interface
        virtual long read( unsigned char *inBuffer, long inNumBytes );


        
    private:

        InputStream *mInputStream;
        AESDecryptor *mDecryptor;


        
    };



#endif
