/*
 * Modification History
 *
 * 2003-August-25   Jason Rohrer
 * Created.
 *
 * 2003-August-26   Jason Rohrer
 * Finished implementation.
 *
 * 2003-August-28   Jason Rohrer
 * Fixed a bugs in use of strstr.
 * Removed deletion of const string.
 * Fixed a double-deletion bug.
 * Added code to build map directory.
 * Fixed bug in hex-encoding message body.
 * Added ping handler.
 *
 * 2003-August-29   Jason Rohrer
 * Added internal tags to internal function names.
 * Added some support for contact info requests.
 * Added support for processing messages from unknown contacts.
 * Fix contact info request code.
 *
 * 2003-August-31   Jason Rohrer
 * Reversed order that handlers are invoked.
 * Added support for unknown contacts.
 * Added more return values.
 * Fixed for loop bug.
 * Added map file creation when auto-approve is on.
 *
 * 2003-September-1   Jason Rohrer
 * Removed all End... tags from high-level messages, since they are not needed.
 * Nullified all static data elements on Stop.
 *
 * 2003-September-4   Jason Rohrer
 * Fixed a deletion order bug.
 *
 * 2003-September-7   Jason Rohrer
 * Increased timeouts.
 *
 * 2003-September-8   Jason Rohrer
 * Added support for setting timeouts in API calls.
 *
 * 2003-September-15   Jason Rohrer
 * Changed to use new static AES functions.
 * Started work on AES key reuse.
 *
 * 2003-September-19   Jason Rohrer
 * Added session key reuse for received messages.
 * Added session key reuse for sending messages.
 * Fixed some bugs in key reuse code.  Fixed a memory leak.
 *
 * 2003-September-23   Jason Rohrer
 * Switched to base64 encoding for opaque message bodies.
 *
 * 2003-October-2   Jason Rohrer
 * Removed support for looking up contact info by virtual address.
 * Added support for ContactIDs.
 * Added support for key fingerprints and contactID encryption.
 *
 * 2003-October-3   Jason Rohrer
 * Fixed some bugs and memory leaks.
 * Added support for receiver key masks to hide reused keys.
 *
 * 2003-October-5   Jason Rohrer
 * Fixed memory leaks.
 * Improved contact ID map creation.
 * Added support for current virtual addresses.
 * Added randomized timed refreshes of current virtual address.
 * Changed to use a separate sendFrom address for each contact.
 *
 * 2003-October-7   Jason Rohrer
 * Parameterized address timeout for testing.
 * Fixed a variable naming bug.
 */



#include "pointToPointCommunicator.h"
#include "MUTE/layers/messageRouting/messageRouter.h"

#include "MUTE/common/CryptoUtils.h"
#include "MUTE/common/AESEncryptor.h"
#include "MUTE/common/AESDecryptor.h"

#include "minorGems/util/SettingsManager.h"
#include "minorGems/io/file/File.h"
#include "minorGems/util/SimpleVector.h"

#include "minorGems/util/log/AppLog.h"

#include "minorGems/system/Semaphore.h"

#include "minorGems/formats/encodingUtils.h"

#include "minorGems/crypto/hashes/sha1.h"

#include <time.h>
#include "minorGems/util/random/StdRandomSource.h"
#include "minorGems/util/random/RandomSource.h"



/**
 * Wrapper for message handler functions.
 *
 * @author Jason Rohrer
 */
class ContactMessageHandlerWrapper {
    public:
        int mID;
        char (*mHandlerFunction)( char, char *, char *, void * );
        void *mExtraHandlerArgument;
    };


// static data elements
char mutePointToPointStarted = false;
MutexLock *muteLock = NULL;
MutexLock *muteAutoApproveLock = NULL;
char muteUnknownContactAutoApprove = false;
RandomSource *muteTimeoutRandomSource = NULL;

// between half hour and hour
int muteMinAddressTimeout = 1800;
int muteMaxAddressTimeout = 3600;

char *muteLocalContactID = NULL;
char *muteReceiverKeyMask = NULL;
SimpleVector<ContactMessageHandlerWrapper *> *muteContactMessageHandlers =
    NULL;
int muteNextFreeContactHandlerID = 0;
int muteLowLevelMessageHandlerID = -1;

int mutePingHandlerID = -1;
int muteContactInfoHandlerID = -1;


// prototypes for internal functions
// inContactName can be NULL
void mute_internalSendMessageToContact( char *inContactName,
                                        char *inVirtualAddress,
                                        char *inPublicKey,
                                        char *inReceiverKeyMask,
                                        char *inMessage );



/**
 * Gets the current virtual address for a contact.
 *
 * @param inContactName the name of the contact.
 *   Must be destroyed by caller.
 *
 * @return the current virtual address, or NULL if we have no
 *   current address (in which case, the primary address, as returned
 *   by mute_internalGetContactInfo, should be used).
 *   Must be destroyed by caller.
 */
char *mute_internalGetCurrentVirtualAddress( char *inContactName ) {
    char *returnAddress = NULL;

    // we store map of each known contact name to a current virtual address
    // in a .mva file (MUTE virtual address)
    File *mapDir = new File( NULL, "virtualAddressMaps" );
    File *subDir = mapDir->getChildFile( "sendTo" );
    delete mapDir;
    
    if( subDir != NULL ) {
    
        char *mapFileName = autoSprintf( "%s.mva", inContactName );
        
        File *mapFile = subDir->getChildFile( mapFileName );
        delete [] mapFileName;
    
        delete subDir;
    
        if( mapFile != NULL ) {
        
            if( mapFile->exists() ) {
            
                char *currentAddress = mapFile->readFileContents();
                
                if( currentAddress != NULL ) {
                    returnAddress = currentAddress;
                    }
                else {
                    AppLog::error(
                        "pointToPoint",
                        "Failed to read from virtual address map file" );
                    }
                }
            else {
                AppLog::warning(
                    "pointToPoint",
                    "Virtual address map file does not exist for contact" );
                }
            
            delete mapFile;
            }
        else {
            AppLog::error(
                "pointToPoint",
                "Virtual address sendTo map directory does not exist?" );
            }
        }
    else {
        AppLog::error( "pointToPoint",
                       "Virtual address map directory does not exist?" );
        }

    return returnAddress;
    }



/**
 * Sets the current virtual address for a contact.
 *
 * @param inContactName the name of the contact.
 *   Must be destroyed by caller.
 * @param inCurrentAddress the current virtual address, or NULL
 *   to clear the address map for this contact.
 *   Must be destroyed by caller.
 *
 */
void mute_internalSetCurrentVirtualAddress( char *inContactName,
                                            char *inCurrentAddress ) {

    // we store map of each known contact name to a current virtual address
    // in a .mva file (MUTE virtual address)
    File *mapDir = new File( NULL, "virtualAddressMaps" );
    File *subDir = mapDir->getChildFile( "sendTo" );
    delete mapDir;
    
    if( subDir != NULL ) {
        
        char *mapFileName = autoSprintf( "%s.mva", inContactName );
        
        File *mapFile = subDir->getChildFile( mapFileName );
        delete [] mapFileName;
    
        delete subDir;
    
        if( mapFile != NULL ) {
            if( inCurrentAddress == NULL ) {
                mapFile->remove();
                }
            else {
                mapFile->writeToFile( inCurrentAddress );
                }
            delete mapFile;
            }
        else {
            AppLog::error(
                "pointToPoint",
                "Virtual address sendTo map directory does not exist?" );
            }
        }
    else {
        AppLog::error( "pointToPoint",
                       "Virtual address map directory does not exist?" );
        }

    }



/**
 * Gets the current virtual address that we should use as our
 * from address when we send to a contact.
 *
 * @param inContactName the name of the contact.
 *   Must be destroyed by caller.
 *
 * @return the virtual address, or NULL if we have no
 *   current address set.
 *   Must be destroyed by caller.
 */
char *mute_internalGetCurrentVirtualSendFromAddress( char *inContactName ) {
    char *returnAddress = NULL;

    // we store map of each known contact name to a current virtual address
    // in a .mva file (MUTE virtual address)
    File *mapDir = new File( NULL, "virtualAddressMaps" );
    File *subDir = mapDir->getChildFile( "sendFrom" );
    delete mapDir;
    
    if( subDir != NULL ) {
    
        char *mapFileName = autoSprintf( "%s.mva", inContactName );
        File *mapFile = subDir->getChildFile( mapFileName );
        delete [] mapFileName;

        char *oldMapFileName = autoSprintf( "%s.old.mva", inContactName );
        File *oldMapFile = subDir->getChildFile( oldMapFileName );
        delete [] oldMapFileName;

        // .mvt = "MUTE Virtual address timeout"
        char *timeoutFileName = autoSprintf( "%s.mvt", inContactName );
        File *timeoutFile = subDir->getChildFile( timeoutFileName );
        delete [] timeoutFileName;

        
        delete subDir;
    
        if( mapFile != NULL &&
            oldMapFile != NULL &&
            timeoutFile != NULL ) {
            
            if( mapFile->exists() ) {
                
                char *currentAddress = mapFile->readFileContents();
                
                if( currentAddress != NULL ) {

                    if( timeoutFile->exists() ) {

                        char *timeoutString = timeoutFile->readFileContents();

                        if( timeoutString != NULL ) {
                            long timeout;
                            int numRead = sscanf( timeoutString,
                                                  "%ld", &timeout );

                            if( numRead == 1 ) {
                                if( time( NULL ) > timeout ) {
                                    // current address has timed out

                                    if( oldMapFile->exists() ) {
                                        char *oldAddress =
                                            oldMapFile->readFileContents();
                                        
                                        if( oldAddress != NULL ) {
                                            muteRemoveReceiveAddress(
                                                oldAddress );

                                            delete [] oldAddress;
                                            }
                                        else {
                                            AppLog::error(
                                                "pointToPoint",
                                                "Failed to read from "
                                                "old address file" );
                                            }
                                        }
                                    
                                    oldMapFile->writeToFile( currentAddress );

                                    delete [] currentAddress;

                                    currentAddress = muteGetUniqueName();
                                    mapFile->writeToFile( currentAddress );

                                    muteAddReceiveAddress( currentAddress );
                                    
                                    
                                    // new timeout offset from now
                                    long newTimeout = time( NULL ) +
                                        muteTimeoutRandomSource->
                                            getRandomBoundedInt(
                                                muteMinAddressTimeout,
                                                muteMaxAddressTimeout );
                                    char *newTimeoutString =
                                        autoSprintf( "%d", newTimeout );

                                    timeoutFile->writeToFile(
                                        newTimeoutString );
                                    
                                    delete [] newTimeoutString;
                                    }
                                }
                            else {
                                AppLog::error(
                                    "pointToPoint",
                                    "Bad integer in timeout file" );
                                }
                            delete [] timeoutString;
                            }
                        else {
                            AppLog::error(
                                "pointToPoint",
                                "Failed to read from timeout file" );
                            }                        
                        }
                    else {
                        // no timeout?  create one
                        
                        // new timeout offset from now
                        long newTimeout = time( NULL ) +
                            muteTimeoutRandomSource->
                            getRandomBoundedInt( muteMinAddressTimeout,
                                                 muteMaxAddressTimeout );
                        char *newTimeoutString =
                            autoSprintf( "%d", newTimeout );
                        
                        timeoutFile->writeToFile(
                            newTimeoutString );
                        
                        delete [] newTimeoutString;
                        }
                    
                    
                    returnAddress = currentAddress;
                    }
                else {
                    AppLog::error(
                        "pointToPoint",
                        "Failed to read from virtual address map file" );
                    }
                }
            else {
                // generate a fresh address
                char *currentAddress = muteGetUniqueName();
                mapFile->writeToFile( currentAddress );

                returnAddress = currentAddress;

                muteAddReceiveAddress( currentAddress );
                
                // new timeout offset from now
                long newTimeout = time( NULL ) +
                    muteTimeoutRandomSource->
                    getRandomBoundedInt( muteMinAddressTimeout,
                                         muteMaxAddressTimeout );
                char *newTimeoutString =
                    autoSprintf( "%d", newTimeout );
                
                timeoutFile->writeToFile(
                    newTimeoutString );
                
                delete [] newTimeoutString;                
                }
            }
        else {
            AppLog::error(
                "pointToPoint",
                "Virtual address sendFrom map directory does not exist?" );
            }

        if( mapFile != NULL ) {
            delete mapFile;
            }
        if( oldMapFile != NULL ) {
            delete oldMapFile;
            }
        if( timeoutFile != NULL ) {
            delete timeoutFile;
            }
        }
    else {
        AppLog::error(
            "pointToPoint",
            "Virtual address map directory does not exist?" );
        }

    return returnAddress;
    }



/**
 * Clears the current virtual address that we should use as our
 * from address when we send to a contact.
 *
 * The next call to mute_internalGetCurrentVirtualSendFromAddress will return
 * a fresh sendFrom address for this contact.
 *
 * @param inContactName the name of the contact.
 *   Must be destroyed by caller.
 * @param inCurrentAddress the current virtual address, or NULL
 *   to clear the address map for this contact.
 *   Must be destroyed by caller.
 *
 */
void mute_internalClearCurrentVirtualSendFromAddress( char *inContactName ) {

    // we store map of each known contact name to a current virtual address
    // in a .mva file (MUTE virtual address)
    File *mapDir = new File( NULL, "virtualAddressMaps" );

    File *subDir = mapDir->getChildFile( "sendFrom" );
    delete mapDir;
    
    if( subDir != NULL ) {

        char *mapFileName = autoSprintf( "%s.mva", inContactName );
        File *mapFile = subDir->getChildFile( mapFileName );
        delete [] mapFileName;

        char *oldMapFileName = autoSprintf( "%s.old.mva", inContactName );
        File *oldMapFile = subDir->getChildFile( oldMapFileName );
        delete [] oldMapFileName;

        // .mvt = "MUTE Virtual address timeout"
        char *timeoutFileName = autoSprintf( "%s.mvt", inContactName );
        File *timeoutFile = subDir->getChildFile( timeoutFileName );
        delete [] timeoutFileName;
        
        delete subDir;
    
        if( mapFile != NULL &&
            oldMapFile != NULL &&
            timeoutFile != NULL ) {

            // stop receiving on our two most recent addresses
            if( mapFile->exists() ) {
                char *address =
                    mapFile->readFileContents();
                
                if( address != NULL ) {
                    muteRemoveReceiveAddress(
                        address );
                    
                    delete [] address;
                    }
                }            
            if( oldMapFile->exists() ) {
                char *address =
                    oldMapFile->readFileContents();
                
                if( address != NULL ) {
                    muteRemoveReceiveAddress(
                        address );
                    
                    delete [] address;
                    }
                }

            // delete the files
            mapFile->remove();
            timeoutFile->remove();
            oldMapFile->remove();
            }
        else {
            AppLog::error(
                "pointToPoint",
                "Virtual address sendFrom map directory does not exist?" );
            }

        if( mapFile != NULL ) {
            delete mapFile;
            }
        if( oldMapFile != NULL ) {
            delete oldMapFile;
            }
        if( timeoutFile != NULL ) {
            delete timeoutFile;
            }

        }
    else {
        AppLog::error( "pointToPoint",
                       "Virtual address map directory does not exist?" );
        }
    }



/**
 * Gets information about a contact.
 *
 * @param inContactName the name of the contact.
 *   Must be destroyed by caller.
 * @param outContactID pointer to where the contact ID
 *   should be returned.  Set to NULL on failure.
 *   Must be destroyed by caller.
 * @param outPrimaryVirtualAddress pointer to where the primary virtual address
 *   should be returned.  Set to NULL on failure.
 *   Must be destroyed by caller.
 * @param outPublicKey pointer to where the public key
 *   should be returned.  Set to NULL on failure.
 *   Must be destroyed by caller.
 * @param outReceiverKeyMask pointer to where the key mask
 *   should be returned.  Set to NULL on failure.
 *   Must be destroyed by caller.
 *
 * @return true if getting contact info succeeded, or false otherwise.
 */ 
char mute_internalGetContactInfo( char *inContactName,
                                  char **outContactID,
                                  char **outPrimaryVirtualAddress,
                                  char **outPublicKey,
                                  char **outReceiverKeyMask ) {

    *outPrimaryVirtualAddress = NULL;
    *outPublicKey = NULL;
    char returnValue = false;
    
    
    File *contactDir = new File( NULL, "contacts" );

    char *contactFileName = autoSprintf( "%s.mc", inContactName );
    
    File *contactFile = contactDir->getChildFile( contactFileName );
    delete [] contactFileName;

    delete contactDir;
    
    if( contactFile != NULL ) {

        if( contactFile->exists() ) {

            char *contactInfo = contactFile->readFileContents();

            if( contactInfo != NULL ) {

                SimpleVector<char *> *tokens = tokenizeString( contactInfo );

                int numTokens = tokens->size();
                if( numTokens == 10 ) {
                    // token 1 is name (ignore it)
                    
                    // token 3 is contact ID
                    *outContactID =
                        stringDuplicate( *( tokens->getElement( 3 ) ) );
                    // token 5 is virtual address
                    *outPrimaryVirtualAddress =
                        stringDuplicate( *( tokens->getElement( 5 ) ) );
                    // token 7 is public key
                    *outPublicKey =
                        stringDuplicate( *( tokens->getElement( 7 ) ) );
                    // token 9 is public key
                    *outReceiverKeyMask =
                        stringDuplicate( *( tokens->getElement( 9 ) ) );


                    
                    
                    returnValue = true;
                    }
                else {
                    char *logMesage = autoSprintf(
                        "Wrong number of tokens in contact file for %s",
                        inContactName );
                    AppLog::error( "pointToPoint", logMesage );
                    delete [] logMesage;
                    }


                for( int i=0; i<numTokens; i++ ) {
                    delete [] *( tokens->getElement( i ) );
                    }
                delete tokens;
                

                delete [] contactInfo;
                }
            else {
                char *logMesage =
                    autoSprintf( "Failed to read contact info for %s",
                                 inContactName );
                AppLog::error( "pointToPoint", logMesage );
                delete [] logMesage;
                }
            }
        else {
            char *logMesage = autoSprintf( "No contact file for %s",
                                           inContactName );
            AppLog::error( "pointToPoint", logMesage );
            delete [] logMesage;
            }
        
        delete contactFile;
        }
    else {
        AppLog::error( "pointToPoint", "Contact directory does not exist?" );
        }

    return returnValue;
    }



/**
 * Gets the AES session key for a contact.
 *
 * @param inContactName the contact name.
 *   Must be destroyed by caller.
 * @param inType either "sending" or "receiving".
 *   Must be destroyed by caller.
 * @param outAESKey pointer to where the hex-encoded AES key
 *   should be returned.  Set to NULL on failure.
 *   Must be destroyed by caller.
 * @param outAESKeyEncrypted pointer to where the hex-encoded RSA-encrypted
 *   AES key should be returned.  Set to NULL on failure.
 *   Must be destroyed by caller.
 * @param outAESKeySignature pointer to where the hex-encoded RSA-signature
 *   of the  AES key should be returned.  Set to NULL on failure.
 *   Must be destroyed by caller.
 *
 * @return true if getting the key succeeded, or false otherwise.
 */
char mute_internalGetContactSessionKey(
    char *inContactName,
    char *inType,
    char **outAESKey,
    char **outAESKeyEncrypted,
    char **outAESKeySignature ) {

    *outAESKey = NULL;
    *outAESKeyEncrypted = NULL;
    *outAESKeySignature = NULL;

    char returnValue = false;
    
    
    File *sessionKeyDir = new File( NULL, "sessionKeys" );

    File *typeDir = sessionKeyDir->getChildFile( inType );

    delete sessionKeyDir;
    
    if( typeDir != NULL ) {

        char *keyFileName = autoSprintf( "%s_key.txt", inContactName );
        char *keyEncryptedFileName =
            autoSprintf( "%s_key_encrypted.txt", inContactName );
        char *keySignatureFileName =
            autoSprintf( "%s_key_signature.txt", inContactName );

        File *keyFile = typeDir->getChildFile( keyFileName );
        File *keyEncryptedFile = typeDir->getChildFile( keyEncryptedFileName );
        File *keySignatureFile = typeDir->getChildFile( keySignatureFileName );

        delete [] keyFileName;
        delete [] keyEncryptedFileName;
        delete [] keySignatureFileName;
        
        if( keyFile != NULL &&
            keyFile->exists() &&
            keyEncryptedFile != NULL &&
            keyEncryptedFile->exists() &&
            keySignatureFile != NULL &&
            keyEncryptedFile->exists() ) {

            char *key = keyFile->readFileContents();
            char *keyEncrypted = keyEncryptedFile->readFileContents();
            char *keySignature = keySignatureFile->readFileContents();

            if( key != NULL &&
                keyEncrypted != NULL &&
                keySignature != NULL ) {

                *outAESKey = key;
                *outAESKeyEncrypted = keyEncrypted;
                *outAESKeySignature = keySignature;
                returnValue = true;
                }
            else {
                if( key != NULL ) {
                    delete [] key;
                    }
                if( keyEncrypted != NULL ) {
                    delete [] keyEncrypted;
                    }
                if( keySignature != NULL ) {
                    delete [] keySignature;
                    }
                }
            }

        if( keyFile != NULL ) {
            delete keyFile;
            }
        if( keyEncryptedFile != NULL ) {
            delete keyEncryptedFile;
            }
        if( keySignatureFile != NULL ) {
            delete keySignatureFile;
            }

        delete typeDir;
        }

    return returnValue;
    }


/**
 * Gets the AES session sending key for a contact.
 *
 * See mute_internalGetContactSessionKey for parameter and return value
 * descriptions.
 */
char mute_internalGetContactSessionSendingKey(
    char *inContactName,
    char **outAESKey,
    char **outAESKeyEncrypted,
    char **outAESKeySignature ) {

    return mute_internalGetContactSessionKey( inContactName,
                                              "sending",
                                              outAESKey,
                                              outAESKeyEncrypted,
                                              outAESKeySignature );
    }



/**
 * Gets the AES session receiving key for a contact.
 *
 * See mute_internalGetContactSessionKey for parameter and return value
 * descriptions.
 */
char mute_internalGetContactSessionReceivingKey(
    char *inContactName,
    char **outAESKey,
    char **outAESKeyEncrypted,
    char **outAESKeySignature ) {

    return mute_internalGetContactSessionKey( inContactName,
                                              "receiving",
                                              outAESKey,
                                              outAESKeyEncrypted,
                                              outAESKeySignature );
    }



/**
 * Sets the AES session key for a contact.
 *
 * @param inContactName the contact name.
 *   Must be destroyed by caller.
 * @param inType either "sending" or "receiving".
 *   Must be destroyed by caller.
 * @param inAESKey the hex-encoded AES key.
 *   Must be destroyed by caller.
 * @param inAESKeyEncrypted the hex-encoded RSA-encrypted AES key.
 *   Must be destroyed by caller.
 * @param inAESKeySignature the hex-encoded RSA-signature
 *   of the  AES key.
 *   Must be destroyed by caller.
 *
 * @return true if setting the key succeeded, or false otherwise.
 */
char mute_internalSetContactSessionKey(
    char *inContactName,
    char *inType,
    char *inAESKey,
    char *inAESKeyEncrypted,
    char *inAESKeySignature ) {


    char returnValue = false;
    
    
    File *sessionKeyDir = new File( NULL, "sessionKeys" );

    File *typeDir = sessionKeyDir->getChildFile( inType );

    delete sessionKeyDir;
    
    if( typeDir != NULL ) {

        char *keyFileName = autoSprintf( "%s_key.txt", inContactName );
        char *keyEncryptedFileName =
            autoSprintf( "%s_key_encrypted.txt", inContactName );
        char *keySignatureFileName =
            autoSprintf( "%s_key_signature.txt", inContactName );

        File *keyFile = typeDir->getChildFile( keyFileName );
        File *keyEncryptedFile = typeDir->getChildFile( keyEncryptedFileName );
        File *keySignatureFile = typeDir->getChildFile( keySignatureFileName );

        delete [] keyFileName;
        delete [] keyEncryptedFileName;
        delete [] keySignatureFileName;
        
        if( keyFile != NULL &&
            keyEncryptedFile != NULL &&
            keySignatureFile != NULL ) {

            char success = keyFile->writeToFile( inAESKey );
            success = success &&
                keyEncryptedFile->writeToFile( inAESKeyEncrypted );
            success = success &&
                keySignatureFile->writeToFile( inAESKeySignature );

            returnValue = success;
            }

        if( keyFile != NULL ) {
            delete keyFile;
            }
        if( keyEncryptedFile != NULL ) {
            delete keyEncryptedFile;
            }
        if( keySignatureFile != NULL ) {
            delete keySignatureFile;
            }

        delete typeDir;
        }

    
    // create a map file for this key
    unsigned char *aesRaw = hexDecode( inAESKey );
    int aesLength = strlen( inAESKey ) / 2;
    char *keyFingerprint = computeSHA1Digest( aesRaw, aesLength );
    delete [] aesRaw;
    
    // we store map of each known key fingerprint to a contact name
    // in a .mkf file (MUTE key fingerprint)
    File *mapDir = new File( NULL, "keyFingerprintMaps" );
    
    char *mapFileName = autoSprintf( "%s.mkf", keyFingerprint );

    delete [] keyFingerprint;
    
    File *mapFile = mapDir->getChildFile( mapFileName );
    delete [] mapFileName;
    
    delete mapDir;
    
    if( mapFile != NULL ) {
        mapFile->writeToFile( inContactName );
        
        delete mapFile;
        }


    return returnValue;
    }


/**
 * Sets the AES session sending key for a contact.
 *
 * See mute_internalSetContactSessionKey for parameter and return value
 * descriptions.
 */
char mute_internalSetContactSessionSendingKey(
    char *inContactName,
    char *inAESKey,
    char *inAESKeyEncrypted,
    char *inAESKeySignature ) {

    return mute_internalSetContactSessionKey( inContactName,
                                              "sending",
                                              inAESKey,
                                              inAESKeyEncrypted,
                                              inAESKeySignature );
    }



/**
 * Sets the AES session receiving key for a contact.
 *
 * See mute_internalSetContactSessionKey for parameter and return value
 * descriptions.
 */
char mute_internalSetContactSessionReceivingKey(
    char *inContactName,
    char *inAESKey,
    char *inAESKeyEncrypted,
    char *inAESKeySignature ) {

    return mute_internalSetContactSessionKey( inContactName,
                                              "receiving",
                                              inAESKey,
                                              inAESKeyEncrypted,
                                              inAESKeySignature );
    }



/**
 * Sets the contact name to map a contact ID to.
 *
 * @param inContactName the name of the contact.
 *   Must be destroyed by caller.
 * @param inContactID the ID of the contact.
 *   Must be destroyed by caller.
 */
void mute_internalSetContactIDMap( char *inContactID,
                                   char *inContactName ) {
    char *mapFileName =
        autoSprintf( "%s.mci",
                     inContactID );
    File *mapDir = new File( NULL,
                             "contactIDMaps" );
            
    File *mapFile =
        mapDir->getChildFile( mapFileName );
    delete [] mapFileName;
    delete mapDir;
            
    if( mapFile != NULL ) {
        mapFile->writeToFile( inContactName );
        delete mapFile;
        }
    else {
        AppLog::error(
            "pointToPoint",
            "Failed to create contact ID map file" );
        }
    }



/**
 * Maps a contact ID to a contact name.
 *
 * @param inContactID the ID to map.
 *   Must be destroyed by caller.
 *
 * @return the contact name, or NULL if mapping fails.
 *   Must be destroyed by caller if non-NULL.
 */
char *mute_internalMapContactIDToContactName( char *inContactID ) {
    char *returnName = NULL;

    // we store map of each known contact ID to a contact name
    // in a .mci file (MUTE contact ID)
    File *mapDir = new File( NULL, "contactIDMaps" );
    
    char *mapFileName = autoSprintf( "%s.mci", inContactID );
        
    File *mapFile = mapDir->getChildFile( mapFileName );
    delete [] mapFileName;
    
    delete mapDir;
    
    if( mapFile != NULL ) {
        
        if( mapFile->exists() ) {
            
            char *contactName = mapFile->readFileContents();

            if( contactName != NULL ) {
                returnName = contactName;
                }
            else {
                AppLog::error( "pointToPoint",
                               "Failed to read from contact ID map file" );
                }
            }
        else {
            AppLog::warning( "pointToPoint",
                             "Map file does not exist for contact ID" );


            // search through full contact list
            int numContacts;
            char **contacts = muteGetContactList( &numContacts );
            char found = false;

            for( int i=0; i<numContacts && !found; i++ ) {
                char *contactID;
                char *virtualAddress;
                char *publicKey;
                char *receiverKeyMask;
                
                char infoFound = mute_internalGetContactInfo(
                    contacts[i],
                    &contactID,
                    &virtualAddress,
                    &publicKey,
                    &receiverKeyMask );

                if( infoFound ) {
                    if( strcmp( contactID, inContactID ) == 0 ) {
                        found = true;
                        returnName = stringDuplicate( contacts[i] );
                        }

                    delete [] contactID;
                    delete [] virtualAddress;
                    delete [] publicKey;
                    delete [] receiverKeyMask;                    
                    }
                delete [] contacts[i];
                }
            delete [] contacts;
            

            if( found ) {
                // why doesn't a map file exist?  create one
                mute_internalSetContactIDMap( inContactID,
                                              returnName );
                }
            }
        
        delete mapFile;
        }
    else {
        AppLog::error( "pointToPoint",
                       "Contact ID map directory does not exist?" );
        }

    return returnName;
    }



/**
 * Maps an AES key fingerprint to a contact name.
 *
 * @param inKeyFingerprint the fingerprint to map.
 *   Must be destroyed by caller.
 *
 * @return the contact name, or NULL if mapping fails.
 *   Must be destroyed by caller if non-NULL.
 */
char *mute_internalMapKeyFingerprintToContactName( char *inKeyFingerprint ) {
    char *returnName = NULL;

    // we store map of each known key fingerprint to a contact name
    // in a .mkf file (MUTE key fingerprint)
    File *mapDir = new File( NULL, "keyFingerprintMaps" );
    
    char *mapFileName = autoSprintf( "%s.mkf", inKeyFingerprint );
        
    File *mapFile = mapDir->getChildFile( mapFileName );
    delete [] mapFileName;
    
    delete mapDir;
    
    if( mapFile != NULL ) {
        
        if( mapFile->exists() ) {
            
            char *contactName = mapFile->readFileContents();

            if( contactName != NULL ) {
                returnName = contactName;
                }
            else {
                AppLog::error( "pointToPoint",
                             "Failed to read from key fingerprint map file" );
                }
            }
        else {
            AppLog::error( "pointToPoint",
                           "Map file does not exist for key fingerprint" );
            }
        
        delete mapFile;
        }
    else {
        AppLog::error( "pointToPoint",
                       "Key fingerprint map directory does not exist?" );
        }

    return returnName;    
    }



// handler to receive all messages from the routing layer
// extra argument not used
int mute_internalLowLevelMessageHandler( char *inFromAddress,
                                          char *inToAddress,
                                          char *inBody,
                                          void *inExtraArgument ) {
    // tokenize it
    SimpleVector<char *> *tokens = tokenizeString( inBody );

    int numTokens = tokens->size();

    if( strstr( inBody, "Opaque" ) != NULL ) {

        if( numTokens == 16 ) {
                    
            char *initVector = *( tokens->getElement( 3 ) );
            char *aesEncryptedMasked = *( tokens->getElement( 5 ) );
            char *aesFingerprintMasked = *( tokens->getElement( 7 ) );
            char *aesSignatureMasked = *( tokens->getElement( 9 ) );
            char *contactIDEncryptedHex =  *( tokens->getElement( 11 ) );
            char *bodyEncryptedBase64 = *( tokens->getElement( 13 ) );
            char *bodySignature = *( tokens->getElement( 15 ) );


            // unmask the key elements
            char *aesEncrypted =
                AESDecryptor::aesDecryptHexToHex(
                    aesEncryptedMasked,
                    muteReceiverKeyMask,
                    initVector );
            char *aesFingerprint =
                AESDecryptor::aesDecryptHexToHex(
                    aesFingerprintMasked,
                    muteReceiverKeyMask,
                    initVector );
            char *aesSignature =
                AESDecryptor::aesDecryptHexToHex(
                    aesSignatureMasked,
                    muteReceiverKeyMask,
                    initVector );
            
            
            // try to map the key fingerprint to a contact
            char *contactName =
                mute_internalMapKeyFingerprintToContactName( aesFingerprint );

            char contactKnown = false;

            char *virtualAddress;
            char *publicKey;

            
            // try to get a session key for this contact            
            char sessionKeysFound = false;
            char *aesSessionKey;
            char *aesSessionKeyEncrypted;
            char *aesSessionKeySigned;

        
            if( contactName != NULL ) {
                char *tempContactID;
                char *tempReceiverKeyMask;
                char gotInfo =
                    mute_internalGetContactInfo( contactName,
                                                 &tempContactID,
                                                 &virtualAddress,
                                                 &publicKey,
                                                 &tempReceiverKeyMask);
                
                if( gotInfo ) {
                    contactKnown = true;
                    
                    
                    sessionKeysFound =
                        mute_internalGetContactSessionReceivingKey(
                            contactName,
                            &aesSessionKey,
                            &aesSessionKeyEncrypted,
                            &aesSessionKeySigned );

                    // ignore the contact ID and key mask
                    delete [] tempContactID;
                    delete [] tempReceiverKeyMask;
                    }
                }

            
            char *aesHex = NULL;

            if( sessionKeysFound ) {
                if( strcmp( aesEncrypted, aesSessionKeyEncrypted ) == 0 ) {
                    // session key matches key for message
                    // reuse the existing session key to avoid decrypting again
                    aesHex = stringDuplicate( aesSessionKey );
                    }
                }
                
            if( aesHex == NULL ) {
                // decrypt AES with our private key
                char *ourPrivateKey =
                    SettingsManager::getStringSetting( "privateKey" );
                    
                if( ourPrivateKey != NULL ) {
                        
                    int aesLength;
                    unsigned char *aesRaw =
                        CryptoUtils::rsaDecrypt( ourPrivateKey,
                                                 aesEncrypted,
                                                 &aesLength );
                    if( aesRaw != NULL ) {

                        if( !contactKnown ) {
                            // try to map the contactID to a contact name

                            char *aesTempHex = hexEncode( aesRaw,
                                                          aesLength );

                            // decrypt the contactID with the aes key
                            unsigned char *contactIDEncrypted
                                = hexDecode( contactIDEncryptedHex );
                            int contactIDEncryptedLength =
                                strlen( contactIDEncryptedHex ) / 2;

                            char *contactID = (char *)(
                                AESDecryptor::aesDecrypt(
                                    contactIDEncrypted,
                                    contactIDEncryptedLength,
                                    aesTempHex,
                                    initVector ) );
                            
                            delete [] aesTempHex;
                            delete [] contactIDEncrypted;
                            
                            contactName =
                                mute_internalMapContactIDToContactName(
                                    contactID );
                            delete [] contactID;
                            
                            if( contactName != NULL ) {
                                char *tempContactID;
                                char *tempReceiverKeyMask;
                                char gotInfo =
                                    mute_internalGetContactInfo(
                                        contactName,
                                        &tempContactID,
                                        &virtualAddress,
                                        &publicKey,
                                        &tempReceiverKeyMask );
                
                                if( gotInfo ) {
                                    contactKnown = true;
                                    
                                    // ignore the contact ID and key mask
                                    delete [] tempContactID;
                                    delete [] tempReceiverKeyMask;
                                    }
                                }
                            }
                        
                        if( contactKnown ) {
                            
                            char sigCorrect = CryptoUtils::rsaVerify(
                                publicKey,
                                aesRaw,
                                aesLength,
                                aesSignature );

                            if( sigCorrect ) {
                                aesHex = hexEncode( aesRaw,
                                                    aesLength );

                                // save the session key for later
                                mute_internalSetContactSessionReceivingKey(
                                    contactName,
                                    aesHex,
                                    aesEncrypted,
                                    aesSignature );
                                }
                            }
                        else {
                            // contact not known, so ignore the signature
                            // and don't save the session key
                            aesHex = hexEncode( aesRaw,
                                                aesLength );
                            }
                        delete [] aesRaw;
                        }
                    else {
                        AppLog::error(
                            "pointToPoint",
                            "Failed to decrypt the AES key" );
                        }
                    delete [] ourPrivateKey;
                    }
                else {
                    AppLog::error(
                        "pointToPoint",
                        "Failed to read our private key" );
                    }
                }

            if( aesHex != NULL ) {

                /*
        char *contactName = NULL;
        
        if( numTokens >= 12 ) {
            char *contactID = *( tokens->getElement( 11 ) );

            contactName =
                mute_internalMapContactIDToContactName( contactID );
            }
        


        char sessionKeysFound = false;
        char *aesSessionKey;
        char *aesSessionKeyEncrypted;
        char *aesSessionKeySigned;

        
        if( contactName != NULL ) {
            char *contactID;
            char gotInfo = mute_internalGetContactInfo( contactName,
                                                        &contactID,
                                                        &virtualAddress,
                                                        &publicKey );

            if( gotInfo ) {
                contactKnown = true;


                sessionKeysFound =
                    mute_internalGetContactSessionReceivingKey(
                        contactName,
                        &aesSessionKey,
                        &aesSessionKeyEncrypted,
                        &aesSessionKeySigned );

                // ignore the contact ID
                delete [] contactID;
                }
            }
                */
                if( contactName == NULL ) {
                    // use virtual address as contact name
                    contactName = stringDuplicate( inFromAddress );
                    }
                else {
                    // we have a valid contact name.
                    // we should update our virtual address map
                    // for this contact in case they have switched addresses

                    mute_internalSetCurrentVirtualAddress( contactName,
                                                           inFromAddress );
                    }

        
                // decrypt the message with the AES key
                int bodyLength;
                unsigned char *bodyEncrypted
                    = base64Decode( bodyEncryptedBase64, &bodyLength );
                    
                char *message = (char *)(
                    AESDecryptor::aesDecrypt(
                        bodyEncrypted,
                        bodyLength,
                        aesHex,
                        initVector ) );
                    
                delete [] bodyEncrypted;
                    
                    
                // verify the signature of the encrypted message
                // if we know the contact
                char sigCorrect = false;

                // sig was generated with
                // hexEncode( aesEncrypt( sha1( body ), aesKey, IV ) )

                unsigned char *bodyHash = computeRawSHA1Digest( message );

                unsigned char *encryptedBodyHash =
                    AESEncryptor::aesEncrypt(
                        bodyHash,
                        20,
                        aesHex,
                        initVector );
                delete [] bodyHash;
                delete [] aesHex;

                char *hexEncryptedBodyHash = hexEncode( encryptedBodyHash,
                                                        20 );
                delete [] encryptedBodyHash;
                
                if( strcmp( hexEncryptedBodyHash, bodySignature ) == 0 ) {
                    sigCorrect = true;
                    }
                delete [] hexEncryptedBodyHash;
                
                if( sigCorrect ) {
                    AppLog::info(
                        "pointToPoint",
                        "Received a valid Opaque message" );
                    AppLog::detail(
                        "pointToPoint",
                        message );
                                
                    muteLock->lock();
                    // pass to handlers until one processes message
                    char handled = false;
                    int numHandlers =
                        muteContactMessageHandlers->size();

                    // reverse order that handlers were registered
                    // (newest handler first)
                    for( int i=numHandlers - 1; i>=0 && !handled;  i-- ) {
                        
                        ContactMessageHandlerWrapper *wrapper =
                            *( muteContactMessageHandlers->
                               getElement( i ) );
                        
                        // pass to handler
                        handled =
                            wrapper->mHandlerFunction(
                                contactKnown,
                                contactName,
                                message,
                                wrapper->mExtraHandlerArgument );
                        }
                    if( handled ) {
                        AppLog::info(
                            "pointToPoint",
                            "Message consumed by a handler" );
                        }
                    else {
                        AppLog::info(
                            "pointToPoint",
                            "Message NOT consumed by a handler" );
                        }
                    muteLock->unlock();
                    }
                else {
                    AppLog::error(
                        "pointToPoint",
                        "Bad signature received" );
                    }
                delete [] message;
                }
            else {
                AppLog::error(
                    "pointToPoint",
                    "Failed to obtain an AES session key for message" );
                }

            if( contactKnown ) {
                delete [] virtualAddress;
                delete [] publicKey;
                }
            else {
                char *logMesage = autoSprintf(
                    "Failed to get contact info for %s",
                    contactName );
                AppLog::error( "pointToPoint", logMesage );
                delete [] logMesage;
                }
            delete [] contactName;
            
            if( sessionKeysFound ) {
                delete [] aesSessionKey;
                delete [] aesSessionKeyEncrypted;
                delete [] aesSessionKeySigned;
                }

            
            delete [] aesEncrypted;
            delete [] aesFingerprint;
            delete [] aesSignature;            
            }
        else {
            AppLog::error(
                "pointToPoint",
                "Expecting 16 tokens in an Opaque message" );
            }
        }
    else {
        AppLog::error( "pointToPoint", "Unknown message type received" );
        }

    for( int i=0; i<numTokens; i++ ) {
        delete [] *( tokens->getElement( i ) );
        }
    delete tokens;

	return false;
    }



// callback function for Pings
// extra parameter is unused
char mute_internalPingHandler( char inContactKnown,
                               char *inFromAddress, char *inMessage,
                               void *inExtraArgument ) {

    if( !inContactKnown ) {
        // don't Pong unknown contacts...
        // we cannot send them secure messages
        return false;
        }
    
    char returnValue = false;
    
    // is the message a Ping?
    // look at second token
    SimpleVector<char *> *tokens = tokenizeString( inMessage );
    int numTokens = tokens->size();

    if( numTokens == 2 ) {
        char *typeToken = *( tokens->getElement( 1 ) );
        if( strcmp( "Ping", typeToken ) == 0 ) {

            // send back a pong
            char *message = "MessageType: Pong";

            muteSendMessageToContact( inFromAddress, message );
            
            returnValue =  true;
            }
        }

    for( int i=0; i<numTokens; i++ ) {
        delete [] *( tokens->getElement( i ) );
        }
    
    delete tokens;
    

    return returnValue;
    }



// default callback function for ContactInfo messages
// handles unsolicited contact info
// extra parameter is unused
char mute_internalContactInfoHandler( char inContactKnown,
                                      char *inFromAddress, char *inMessage,
                                      void *inExtraArgument ) {

    if( inContactKnown ) {

        // ingore contact info sent by known contacts, since
        // we already have their contact info
        return false;
        }
    
    char returnValue = false;
    
    // is the message a ContactInfo?
    // look at second token
    SimpleVector<char *> *tokens = tokenizeString( inMessage );
    int numTokens = tokens->size();

    if( numTokens >= 2 ) {
        char *typeToken = *( tokens->getElement( 1 ) );
        if( strcmp( "ContactInfo", typeToken ) == 0 ) {

            // strip info up to Contact: tag
            char *pointerToInfo = strstr( inMessage, "Contact:" );
            
            if( pointerToInfo != NULL ) {
                // found valid contact info

                if( numTokens >= 4 ) {
                    // token 3 is contact name
                    char *contactName = *( tokens->getElement( 3 ) );
                    
                    // create the contact file
                    char *fileName = autoSprintf( "%s.mc", contactName );

                    
                    File *contactsDir;

                    char autoApprove = muteGetUnknownContactAutoApprove();
                    
                    if( autoApprove ) {
                        // stick contact info right into contacts dir
                        contactsDir = new File( NULL, "contacts" );
                        }
                    else {
                        // stick info into unknown contacts dir, waiting
                        // for approval
                        contactsDir = new File( NULL, "unknownContacts" );
                        }
                    File *contactFile =
                        contactsDir->getChildFile( fileName );
                    delete [] fileName;
                    delete contactsDir;
                    
                    if( contactFile != NULL ) {
                        contactFile->writeToFile( pointerToInfo );
                        delete contactFile;

                        if( autoApprove ) {
                            
                            // make a map file

                            char *contactID;
                            char *virtualAddress;
                            char *publicKey;
                            char *receiverKeyMask;
                            
                            char gotInfo =
                                mute_internalGetContactInfo(
                                    contactName,
                                    &contactID,
                                    &virtualAddress,
                                    &publicKey,
                                    &receiverKeyMask );
                            if( gotInfo ) {

                                mute_internalSetContactIDMap( contactID,
                                                              contactName );

                                delete [] contactID;
                                delete [] virtualAddress;
                                delete [] publicKey;
                                delete [] receiverKeyMask;
                                }
                            else {
                                AppLog::error(
                            "pointToPoint - unsolicited info handler",
                            "Failed to fetch info after making contact file" );
                                }
                            }                        
                        }
                    else {
                        AppLog::error(
                            "pointToPoint - unsolicited info handler",
                            "Failed to create contact file" );
                        }
                    }
            
                returnValue =  true;
                }
            }
        }

    for( int i=0; i<numTokens; i++ ) {
        delete [] *( tokens->getElement( i ) );
        }
    
    delete tokens;
    

    return returnValue;
    }



void mutePointToPointStart() {
    muteLock = new MutexLock();
    muteAutoApproveLock = new MutexLock();
    
    // add our message handler
    muteLowLevelMessageHandlerID =
        muteAddMessageHandler( mute_internalLowLevelMessageHandler,
                               (void *)NULL );
    

    muteContactMessageHandlers =
        new SimpleVector<ContactMessageHandlerWrapper *>();

    char *autoApproveSetting =
        SettingsManager::getStringSetting( "autoApprove" );
    if( autoApproveSetting != NULL ) {
        if( strcmp( autoApproveSetting, "true" ) == 0 ) {
            muteUnknownContactAutoApprove = true;
            }
        else {
            muteUnknownContactAutoApprove = false;
            }
        delete [] autoApproveSetting;
        }
    
    
    // add our primary address
    char *primaryVirtualAddress =
        SettingsManager::getStringSetting( "primaryVirtualAddress" );       
    if( primaryVirtualAddress != NULL ) {        
        muteAddReceiveAddress( primaryVirtualAddress );

        delete [] primaryVirtualAddress;
        }

    
    if( muteTimeoutRandomSource == NULL ) {
        muteTimeoutRandomSource = new StdRandomSource();
        }

    
    // get our local contact ID
    char *localContactID =
        SettingsManager::getStringSetting( "localContactID" );

    if( muteLocalContactID != NULL ) {
        delete [] muteLocalContactID;
        muteLocalContactID = NULL;
        }
    
    if( localContactID != NULL ) {
        
        muteLocalContactID = stringDuplicate( localContactID );
        delete [] localContactID;
        }
    else {
        // create a new contact ID
        muteLocalContactID = muteGetUniqueName();
        }


    
    // get our local receiver key mask
    char *receiverKeyMask =
        SettingsManager::getStringSetting( "receiverKeyMask" );

    if( muteReceiverKeyMask != NULL ) {
        delete [] muteReceiverKeyMask;
        muteReceiverKeyMask = NULL;
        }
    
    if( receiverKeyMask != NULL ) {
        muteReceiverKeyMask = stringDuplicate( receiverKeyMask );
        delete [] receiverKeyMask;
        }
    else {
        // create a new mask
        muteReceiverKeyMask =
            CryptoUtils::generateAESKey( CryptoUtils::AES_128_BIT );
        }


    // construct ID maps for each contact, and
    // clear the virtual address maps
    int numContacts;
    char **contacts = muteGetContactList( &numContacts );
    for( int i=0; i<numContacts; i++ ) {

        mute_internalSetCurrentVirtualAddress( contacts[i], NULL );
        mute_internalClearCurrentVirtualSendFromAddress( contacts[i] );
        
        char *contactID;
        char *virtualAddress;
        char *key;
        char *receiverKeyMask;
        char found = mute_internalGetContactInfo( contacts[i],
                                                  &contactID,
                                                  &virtualAddress, &key,
                                                  &receiverKeyMask );

        if( found ) {
            mute_internalSetContactIDMap( contactID,
                                          contacts[i] );
            
            delete [] contacts[i];

            delete [] contactID;
            delete [] virtualAddress;
            delete [] key;
            delete [] receiverKeyMask;
            }
        }
    delete [] contacts;

    // add a ping handler
    mutePingHandlerID =
        muteAddContactMessageHandler( mute_internalPingHandler, (void *)NULL );

    // and handler for unsolicited contact info
    muteContactInfoHandlerID =
        muteAddContactMessageHandler( mute_internalContactInfoHandler,
                                      (void *)NULL );

    mutePointToPointStarted = true;
    }



void mutePointToPointStop() {
    mutePointToPointStarted = false;
    
    if( muteAutoApproveLock != NULL ) {
        delete muteAutoApproveLock;
        muteAutoApproveLock = NULL;
        }
    if( muteContactInfoHandlerID != -1 ) {
        muteRemoveContactMessageHandler( muteContactInfoHandlerID );
        muteContactInfoHandlerID = -1;
        }
    if( mutePingHandlerID != -1 ) {
        muteRemoveContactMessageHandler( mutePingHandlerID );
        mutePingHandlerID = -1;
        }
    if( muteLowLevelMessageHandlerID != -1 ) {
        muteRemoveMessageHandler( muteLowLevelMessageHandlerID );
        muteLowLevelMessageHandlerID = -1;
        }
    if( muteLock != NULL ) {
        delete muteLock;
        muteLock = NULL;
        }
    if( muteTimeoutRandomSource != NULL ) {
        delete muteTimeoutRandomSource;
        muteTimeoutRandomSource = NULL;
        }
    if( muteLocalContactID != NULL ) {
        delete [] muteLocalContactID;
        muteLocalContactID = NULL;
        }
    if( muteReceiverKeyMask != NULL ) {
        delete [] muteReceiverKeyMask;
        muteReceiverKeyMask = NULL;
        }
    if( muteContactMessageHandlers != NULL ) {
        int numHandlers = muteContactMessageHandlers->size();
        for( int i=0; i<numHandlers; i++ ) {
            delete *( muteContactMessageHandlers->getElement( i ) );
            }
        delete muteContactMessageHandlers;
        muteContactMessageHandlers = NULL;
        }
    }



void muteGenerateLocalContactInformation( char *inContactName,
                                          int inKeyLength ) {

    // make sure we stop receiving on our old primary address
    char *primaryVirtualAddress =
        SettingsManager::getStringSetting( "primaryVirtualAddress" );       
    if( primaryVirtualAddress != NULL ) {        
        muteRemoveReceiveAddress( primaryVirtualAddress );

        delete [] primaryVirtualAddress;
        }
    
    char *contactID = muteGetUniqueName();
    char *virtualAddress = muteGetUniqueName();
    char *publicKey;
    char *privateKey;
    CryptoUtils::generateRSAKey( inKeyLength, &publicKey, &privateKey );
    char *receiverKeyMask =
        CryptoUtils::generateAESKey( CryptoUtils::AES_128_BIT );
    
    SettingsManager::setSetting( "localContactName", inContactName );
    SettingsManager::setSetting( "localContactID", contactID );
    SettingsManager::setSetting( "publicKey", publicKey );
    SettingsManager::setSetting( "privateKey", privateKey );
    SettingsManager::setSetting( "primaryVirtualAddress",
                                 virtualAddress );
    SettingsManager::setSetting( "receiverKeyMask",
                                 receiverKeyMask );

    // if layer is already started, start receiving on new primary address
    if( mutePointToPointStarted ) {
        muteAddReceiveAddress( virtualAddress );
        }
    
    // update static values (will be non-NULL if this layer has already
    // been started)
    if( muteLocalContactID != NULL ) {
        delete [] muteLocalContactID;
        muteLocalContactID = stringDuplicate( contactID );
        }
    if( muteReceiverKeyMask != NULL ) {
        delete [] muteReceiverKeyMask;
        muteReceiverKeyMask = stringDuplicate( receiverKeyMask );
        }
    
    // output a contact file for us in the main directory
    char *fileName = autoSprintf( "%s.mc", inContactName );
    FILE *contactFile = fopen( fileName, "w" );
    delete [] fileName;
    
    if( contactFile != NULL ) {
        
        fprintf( contactFile,
                 "Contact: %s\n"
                 "ContactID: %s\n"
                 "PrimaryVirtualAddress: %s\n"
                 "PublicKey: %s\n"
                 "AESMaskKey: %s",
                 inContactName,
                 contactID,
                 virtualAddress,
                 publicKey,
                 receiverKeyMask );
                 
        fclose( contactFile );
        }
    else {
        AppLog::error( "pointToPoint",
                       "Failed to open our contact file for writing" );
        }
    delete [] virtualAddress;
    delete [] contactID;
    delete [] publicKey;
    delete [] privateKey;
    delete [] receiverKeyMask;
    }



char mutePushLocalContactInfoToContact( char *inContactName ) {

    char returnValue = false;
    
    char *ourName =
        SettingsManager::getStringSetting( "localContactName" );
                
    if( ourName != NULL ) {

        char *ourContactFileName = autoSprintf( "%s.mc", ourName );
        
        File *ourContactFile = new File( NULL, ourContactFileName );
        delete [] ourContactFileName;
        
        if( ourContactFile->exists() ) {
            
            char *ourContactInfo = ourContactFile->readFileContents();
            
            if( ourContactInfo != NULL ) {
                
                char *messageBody = autoSprintf(
                    "MessageType: ContactInfo\n"
                    "%s",
                    ourContactInfo );

                char sent = muteSendMessageToContact( inContactName,
                                                      messageBody );
                delete [] messageBody;
                
                delete [] ourContactInfo;

                returnValue = sent;
                }
            else {
                AppLog::error(
                    "pointToPoint",
                    "Faile to read from local contact info file" );
                }
            }
        else {
            AppLog::error(
                "pointToPoint",
                "Local contact info file not found" );
            }
        delete ourContactFile;
        
        delete [] ourName;
        }
    else {
        AppLog::error(
            "pointToPoint",
            "Local contact name not found" );
        }
    
    return returnValue;
    }



// gets a contact list, specifying a folder name to look in
// inFolderName must be destroyed by caller
char **mute_internalGetContactList( char *inFolderName, int *outNumContacts ) {

    SimpleVector<char *> *contacts = new SimpleVector<char *>();
    
    
    File *contactDir = new File( NULL, inFolderName );

    int numFiles;
    File **contactFiles = contactDir->getChildFiles( &numFiles );

    if( contactFiles != NULL ) {

        char *logMessage =
            autoSprintf( "Found %d files in contacts directory", numFiles );
        
        AppLog::info( "pointToPoint", logMessage );
        delete [] logMessage;

        
        for( int i=0; i<numFiles; i++ ) {

            char *name = contactFiles[i]->getFileName();

            char *pointerToExtension = strstr( name, ".mc" ); 
            if( pointerToExtension != NULL ) {

                // file is a contact file

                // terminate it
                pointerToExtension[0] = '\0';

                contacts->push_back( stringDuplicate( name ) );                
                }

            delete [] name;

            delete contactFiles[i];
            }

        delete [] contactFiles;
        }
    else {
        AppLog::error( "pointToPoint", "Failed to open contacts directory" );
        }
             

    delete contactDir;


    *outNumContacts = contacts->size();

    char *logMessage =
        autoSprintf( "Found %d contacts", *outNumContacts );
        
    AppLog::info( "pointToPoint", logMessage );
    delete [] logMessage;

    
    char **returnArray = contacts->getElementArray();

    delete contacts;

    return returnArray;
    }



char **muteGetContactList( int *outNumContacts ) {
    return mute_internalGetContactList( "contacts", outNumContacts );
    }



char **muteGetUnknownContactList( int *outNumContacts ) {
    return mute_internalGetContactList( "unknownContacts", outNumContacts );
    }



char muteApproveUnknownContact( char *inContactName ) {

    char returnValue = false;
    
    
    char *logMessage = autoSprintf( "Approving unknown contact: %s",
                                    inContactName );
    AppLog::info( "pointToPoint", logMessage );
    delete [] logMessage;

    
    char *fileName = autoSprintf( "%s.mc", inContactName );
    File *unknownContactsDir = new File( NULL,
                                         "unknownContacts" );
    File *unknownContactFile =
        unknownContactsDir->getChildFile( fileName );
    
    delete unknownContactsDir;
    
    if( unknownContactFile != NULL ) {
        if( unknownContactFile->exists() ) {

            // open corresponding file in contacts directory
            File *contactsDir = new File( NULL,
                                         "contacts" );
            File *contactFile =
                contactsDir->getChildFile( fileName );
            
            delete contactsDir;

            if( contactFile != NULL ) {
                // copy unknown contact info into contact file
                unknownContactFile->copy( contactFile );
                
                delete contactFile;
                
                // delete the unknown file
                unknownContactFile->remove();


                // make a map file

                char *contactID;
                char *virtualAddress;
                char *publicKey;
                char *receiverKeyMask;
                
                char gotInfo = mute_internalGetContactInfo( inContactName,
                                                            &contactID,
                                                            &virtualAddress,
                                                            &publicKey,
                                                            &receiverKeyMask );
                if( gotInfo ) {
                    mute_internalSetContactIDMap( contactID,
                                                  inContactName );

                    delete [] contactID;
                    delete [] virtualAddress;
                    delete [] publicKey;
                    delete [] receiverKeyMask;
                    
                    returnValue = true;
                    }
                else {
                    AppLog::error(
                            "pointToPoint",
                            "Failed to fetch info after making contact file" );
                    }
                }
            else {
                AppLog::error( "pointToPoint",
                               "contacts folder does not exist" );
                }
            
            }
        else {
            AppLog::error( "pointToPoint",
                           "unknown contact file does not exist" );
            }
        delete unknownContactFile;
        }
    else {
        AppLog::error( "pointToPoint",
                       "unknown contacts folder does not exist" );
        }

    delete [] fileName;

    return returnValue;
    }


void muteSetUnknownContactAutoApprove( char inAutoApproveOn ) {
    muteAutoApproveLock->lock();
    muteUnknownContactAutoApprove = inAutoApproveOn;

    if( muteUnknownContactAutoApprove ) {
        SettingsManager::setSetting( "autoApprove", "true" );
        }
    else {
        SettingsManager::setSetting( "autoApprove", "false" );
        }
    
    muteAutoApproveLock->unlock();
    }



char muteGetUnknownContactAutoApprove() {
    muteAutoApproveLock->lock();
    char returnValue = muteUnknownContactAutoApprove;
    muteAutoApproveLock->unlock();

    return returnValue;
    }



char muteSendMessageToContact( char *inContactName, char *inMessage ) {
    char returnValue = false;

    char *contactID;
    char *virtualAddress;
    char *publicKey;
    char *receiverKeyMask;
    char gotInfo = mute_internalGetContactInfo( inContactName,
                                                &contactID,
                                                &virtualAddress, &publicKey,
                                                &receiverKeyMask );

    if( gotInfo ) {

        // use current address, if we have one
        char *currentAddress =
            mute_internalGetCurrentVirtualAddress( inContactName );

        if( currentAddress != NULL ) {
            delete [] virtualAddress;
            virtualAddress = currentAddress;
            }
        // otherwise, we default to the primary address
        
        
        mute_internalSendMessageToContact( inContactName,
                                           virtualAddress,
                                           publicKey, receiverKeyMask,
                                           inMessage );

        delete [] contactID;
        delete [] virtualAddress;
        delete [] publicKey;
        delete [] receiverKeyMask;

        returnValue = true;
        }
    else {
        char *logMesage = autoSprintf(
            "Failed to get contact info for %s",
            inContactName );
        AppLog::error( "pointToPoint", logMesage );
        delete [] logMesage;
        }

    return returnValue;
    }



void mute_internalSendMessageToContact( char *inContactName,
                                        char *inVirtualAddress,
                                        char *inPublicKey,
                                        char *inReceiverKeyMask,
                                        char *inMessage ) {

    char *aesHex = NULL;
    char *aesEncryptedHex = NULL;
    char *aesFingerprintHex = NULL;
    char *aesSignatureHex = NULL;

    char gotKeys = false;

    char sessionKeysFound = false;
    
    if( inContactName != NULL ) {

        sessionKeysFound =
            mute_internalGetContactSessionSendingKey(
                inContactName,
                &aesHex,
                &aesEncryptedHex,
                &aesSignatureHex );
        if( sessionKeysFound ) {
            gotKeys = true;
            }
        }

    if( !sessionKeysFound ) {
        // generate an AES key
        aesHex = CryptoUtils::generateAESKey( CryptoUtils::AES_128_BIT );

        unsigned char *aesRaw = hexDecode( aesHex );
        int aesLength = strlen( aesHex ) / 2;
        
        // encrypt it with their public key
        aesEncryptedHex =
            CryptoUtils::rsaEncrypt( inPublicKey,
                                     aesRaw, aesLength );

        if( aesEncryptedHex != NULL ) {

            // sign it with our private key
            
            char *ourPrivateKey =
                SettingsManager::getStringSetting( "privateKey" );
            
            if( ourPrivateKey != NULL ) {
                
                aesSignatureHex =
                    CryptoUtils::rsaSign( ourPrivateKey,
                                          aesRaw,
                                          aesLength );
                
                if( aesSignatureHex != NULL ) {

                    gotKeys = true;

                    if( inContactName != NULL ) {
                        // save our generated keys for next time

                        mute_internalSetContactSessionSendingKey(
                            inContactName,
                            aesHex,
                            aesEncryptedHex,
                            aesSignatureHex );
                        }
                    
                    }
                else {
                    AppLog::error(
                        "pointToPoint",
                        "Failed to sign the AES key with our private key"  );
                    }

                delete [] ourPrivateKey;
                }
            else {
                AppLog::error( "pointToPoint",
                               "Failed to read our private key"  );
                }

            }
        else {
            AppLog::error(
                "pointToPoint",
                "Failed to encrypt the AES key with their public key"  );
            }

        delete [] aesRaw;
        }

    
    if( gotKeys ) {

        unsigned char *aesRaw = hexDecode( aesHex );
        int aesLength = strlen( aesHex ) / 2;
        aesFingerprintHex = computeSHA1Digest( aesRaw, aesLength ); 

        delete [] aesRaw;

        
        // create a random init vector
        // can use a new AES key for this
        char *initVector =
            CryptoUtils::generateAESKey( CryptoUtils::AES_128_BIT );

        
        unsigned char *messageEncrypted =
            AESEncryptor::aesEncrypt( (unsigned char *)inMessage,
                                      strlen( inMessage ),
                                      aesHex,
                                      initVector );

        // base64 encode with no line breaks
        char *messageEncryptedBase64 =
            base64Encode( messageEncrypted, strlen( inMessage ), false );
        delete [] messageEncrypted;



        // sig is generated with
        // hexEncode( aesEncrypt( sha1( body ), aesKey, IV ) )
        
        unsigned char *bodyHash = computeRawSHA1Digest( inMessage );
        
        unsigned char *encryptedBodyHash =
            AESEncryptor::aesEncrypt(
                bodyHash,
                20,
                aesHex,
                initVector );
        delete [] bodyHash;
                
        char *hexEncryptedBodyHash = hexEncode( encryptedBodyHash,
                                                20 );
        delete [] encryptedBodyHash;

        
        unsigned char *contactIDEncrypted =
            AESEncryptor::aesEncrypt( (unsigned char *)muteLocalContactID,
                                      strlen( muteLocalContactID ),
                                      aesHex,
                                      initVector );
        char *contactIDEncryptedHex =
            hexEncode( contactIDEncrypted, strlen( muteLocalContactID ) );
        delete [] contactIDEncrypted;

        // mask reused AES key elements with the receiver's key mask
        char *aesEncryptedHexMasked =
            AESEncryptor::aesEncryptHexToHex( aesEncryptedHex,
                                              inReceiverKeyMask,
                                              initVector );
        char *aesFingerprintHexMasked =
            AESEncryptor::aesEncryptHexToHex( aesFingerprintHex,
                                              inReceiverKeyMask,
                                              initVector );
        char *aesSignatureHexMasked =
            AESEncryptor::aesEncryptHexToHex( aesSignatureHex,
                                              inReceiverKeyMask,
                                              initVector );
        
        char *messageBody = autoSprintf(
            "MessageType: Opaque\n"
            "InitializationVector: %s\n"
            "AESKey: %s\n"
            "AESKeyFingerprint: %s\n"
            "AESKeySignature: %s\n"
            "ContactID: %s\n"
            "Body: %s\n"
            "BodySignature: %s",
            initVector,
            aesEncryptedHexMasked,
            aesFingerprintHexMasked,
            aesSignatureHexMasked,
            contactIDEncryptedHex,
            messageEncryptedBase64,
            hexEncryptedBodyHash );

        delete [] aesEncryptedHexMasked;
        delete [] aesFingerprintHexMasked;
        delete [] aesSignatureHexMasked;
        
        delete [] hexEncryptedBodyHash;
        delete [] contactIDEncryptedHex;


        char *sendFromAddress = NULL;
        if( inContactName != NULL ) {
            // sending to a known contact
            sendFromAddress =
                mute_internalGetCurrentVirtualSendFromAddress( inContactName );
            }
        
        if( sendFromAddress == NULL ) {
            char *primaryVirtualAddress =
                SettingsManager::getStringSetting( "primaryVirtualAddress" );
            
            if( primaryVirtualAddress != NULL ) {
                // send from our primary address
                sendFromAddress = primaryVirtualAddress;
                }
            else {
                // else use a fresh, one-time address
                sendFromAddress = muteGetUniqueName();

                AppLog::warning( "pointToPoint",
                                 "We have no primary address set"  );
                }
            }
        muteSendMessage(
            sendFromAddress,
            inVirtualAddress,
            messageBody );

        delete [] sendFromAddress;
                    
        delete [] messageBody;
        
        delete [] messageEncryptedBase64;

        delete [] initVector;
        }
    else {
        AppLog::error( "pointToPoint",
                       "Failed to get AES keys"  );
        }

    if( aesHex != NULL ) {
        delete [] aesHex;
        }
    if( aesEncryptedHex != NULL ) {
        delete [] aesEncryptedHex;
        }
    if( aesFingerprintHex != NULL ) {
        delete [] aesFingerprintHex;
        }
    if( aesSignatureHex != NULL ) {
        delete [] aesSignatureHex;
        }
    
    }



class PongHandlerArgument {
    public:
        char *mFromAddress;
        Semaphore *mSemaphore;
    };



// callback function for Pongs
// extra parameter is a semaphore/fromAddress structure
char mute_internalPongHandler( char inContactKnown,
                               char *inFromAddress, char *inMessage,
                               void *inExtraArgument ) {

    char returnValue = false;
    
    // unwrap argument
    PongHandlerArgument *argument = (PongHandlerArgument *)inExtraArgument;

    if( strcmp( inFromAddress, argument->mFromAddress ) == 0 ) {
        // addresses match

        // is the message a Pong?
        // look at second token
        SimpleVector<char *> *tokens = tokenizeString( inMessage );
        int numTokens = tokens->size();

        if( numTokens == 2 ) {
            char *typeToken = *( tokens->getElement( 1 ) );
            if( strcmp( "Pong", typeToken ) == 0 ) {

                // signal the semaphore
                argument->mSemaphore->signal();        
                
                returnValue =  true;
                }
            }

        for( int i=0; i<numTokens; i++ ) {
            delete [] *( tokens->getElement( i ) );
            }

        delete tokens;
        }

    return returnValue;
    }



char mutePingContact( char *inContactName,
                      int inTimeoutInMilliseconds ) {
    // first, register a handler for the response

    PongHandlerArgument *argument = new PongHandlerArgument();
    argument->mFromAddress = inContactName;
    argument->mSemaphore = new Semaphore(  );  // starts in unsignaled state

    int id = muteAddContactMessageHandler( mute_internalPongHandler,
                                           (void *)argument );

    // now, send out a ping message
    char *message = "MessageType: Ping";

    muteSendMessageToContact( inContactName, message );
    
    // now wait for our semaphore to be signaled by the pong handler
    int waitResult = argument->mSemaphore->wait( inTimeoutInMilliseconds );

    // remove the handler
    muteRemoveContactMessageHandler( id );

    // clean up argument
    delete argument->mSemaphore;
    delete argument;

    if( waitResult == 1 ) {
        // we got pong
        return true;
        }
    else {
        // we timed out waiting

        // should we try their primary address?
        char *currentAddress =
            mute_internalGetCurrentVirtualAddress( inContactName );

        if( currentAddress != NULL ) {
            delete [] currentAddress;
            
            // a current address exists, so we must have used it last time
            
            // revert to using the primary address
            mute_internalSetCurrentVirtualAddress( inContactName, NULL );

            // try pinging again
            return mutePingContact( inContactName,
                                    inTimeoutInMilliseconds );
            }
        else {
            // we were already pinging them at their primary virtual address
            return false;
            }
        }    
    }



class FetchedContactInfoHandlerArgument {
    public:
        char *mVirtualAddress;
        Semaphore *mSemaphore;
        char *mFoundContactName;
    };



// callback function for fetched contact info
// extra parameter is a semaphore/fromAddress structure
char mute_internalFetchedContactInfoHandler( char inContactKnown,
                                             char *inFromContact,
                                             char *inMessage,
                                             void *inExtraArgument ) {
    char returnValue = false;
    
    // unwrap argument
    FetchedContactInfoHandlerArgument *argument =
        (FetchedContactInfoHandlerArgument *)inExtraArgument;

    if( strcmp( inFromContact, argument->mVirtualAddress ) == 0 ) {
        // addresses match

        // is the message a ContactInfo message?
        // look at second token
        SimpleVector<char *> *tokens = tokenizeString( inMessage );
        int numTokens = tokens->size();

        if( numTokens >= 2 ) {
            char *typeToken = *( tokens->getElement( 1 ) );
            if( strcmp( "ContactInfo", typeToken ) == 0 ) {

                // strip info up to Contact: tag
                char *pointerToInfo = strstr( inMessage, "Contact:" );

                if( pointerToInfo != NULL ) {
                    // found valid contact info

                    if( numTokens >= 4 ) {
                        // token 3 is contact name
                        char *contactName = *( tokens->getElement( 3 ) );

                        // stick contact name into our argument
                        if( argument->mFoundContactName == NULL ) {
                            argument->mFoundContactName =
                                stringDuplicate( contactName );
                            }

                        // create the contact file
                        char *fileName = autoSprintf( "%s.mc", contactName );
                        File *contactsDir = new File( NULL, "contacts" );
                        File *contactFile =
                            contactsDir->getChildFile( fileName );
                        delete [] fileName;
                        delete contactsDir;
                        
                        if( contactFile != NULL ) {
                            contactFile->writeToFile( pointerToInfo );
                            delete contactFile;
                            }
                        else {
                            AppLog::error(
                                "pointToPoint - fetched info handler",
                                "Failed to create contact file" );
                            }

                        // create a map file

                        fileName = autoSprintf( "%s.mva",
                                                argument->mVirtualAddress );
                        File *mapDir = new File( NULL, "addressMaps" );

                        File *mapFile =
                            mapDir->getChildFile( fileName );
                        delete [] fileName;
                        delete mapDir;
                        
                        if( mapFile != NULL ) {
                            mapFile->writeToFile( contactName );
                            delete mapFile;
                            }
                        else {
                            AppLog::error(
                                "pointToPoint - fetched info handler",
                                "Failed to create address map file" );
                            }
                        
                        
                        // signal the semaphore
                        argument->mSemaphore->signal();

                        returnValue = true;
                        }
                    }
                }
            }

        for( int i=0; i<numTokens; i++ ) {
            delete [] *( tokens->getElement( i ) );
            }

        delete tokens;
        }

    return returnValue;
    }



int muteAddContactMessageHandler(
    char (*inHandlerFunction)( char, char *, char *, void * ),
    void *inExtraHandlerArgument ) {

    ContactMessageHandlerWrapper *wrapper = new ContactMessageHandlerWrapper();

    muteLock->lock();

    wrapper->mID = muteNextFreeContactHandlerID;
    muteNextFreeContactHandlerID++;
    
    wrapper->mHandlerFunction = inHandlerFunction;
    wrapper->mExtraHandlerArgument = inExtraHandlerArgument;

    muteContactMessageHandlers->push_back( wrapper );

    muteLock->unlock();

    return wrapper->mID;
    }



void muteRemoveContactMessageHandler( int inHandlerID ) {

    char found = false;

    muteLock->lock();
    int numHandlers = muteContactMessageHandlers->size();
    for( int i=0; i<numHandlers && !found; i++ ) {

        ContactMessageHandlerWrapper *wrapper =
            *( muteContactMessageHandlers->getElement( i ) );

        if( wrapper->mID == inHandlerID ) {
            delete wrapper;
            muteContactMessageHandlers->deleteElement( i );

            found = true;
            }
        }

    muteLock->unlock();
    }
