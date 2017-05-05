/*
 * Modification History
 *
 * 2003-August-22   Jason Rohrer
 * Created.
 * Fixed a bug.
 */



#include "SecureOutputStream.h"



SecureOutputStream::SecureOutputStream( OutputStream *inOutputStream,
                                        AESEncryptor *inEncryptor )
    : mOutputStream( inOutputStream ),
      mEncryptor( inEncryptor ) {

    }



SecureOutputStream::~SecureOutputStream() {
    delete mOutputStream;
    delete mEncryptor;
    }



long SecureOutputStream::write( unsigned char *inBuffer, long inNumBytes ) {
    
    unsigned char *encryptedBuffer =
        mEncryptor->encryptData( inBuffer, inNumBytes );

    int numWritten = mOutputStream->write( encryptedBuffer, inNumBytes );

    delete [] encryptedBuffer;

    return numWritten;
    }
