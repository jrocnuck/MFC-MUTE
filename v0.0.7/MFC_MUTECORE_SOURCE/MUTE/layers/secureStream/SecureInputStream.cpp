/*
 * Modification History
 *
 * 2003-August-22   Jason Rohrer
 * Created.
 */



#include "SecureInputStream.h"


#include <string.h>



SecureInputStream::SecureInputStream( InputStream *inInputStream,
                                      AESDecryptor *inDecryptor )
    : mInputStream( inInputStream ),
      mDecryptor( inDecryptor ) {

    }



SecureInputStream::~SecureInputStream() {
    delete mInputStream;
    delete mDecryptor;
    }



long SecureInputStream::read( unsigned char *inBuffer, long inNumBytes ) {

    unsigned char *encryptedBuffer = new unsigned char[ inNumBytes ];
    int numRead = mInputStream->read( encryptedBuffer, inNumBytes );

    if( numRead <= 0 ) {
        delete [] encryptedBuffer;
        return numRead;
        }

    // else we at least read some
    
    unsigned char *decryptedBuffer =
        mDecryptor->decryptData( encryptedBuffer, numRead );

    delete [] encryptedBuffer;


    memcpy( (void *)inBuffer, (void *)decryptedBuffer, numRead );


    delete [] decryptedBuffer;

    return numRead;
    }
