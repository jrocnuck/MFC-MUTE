/*
 * Modification History
 *
 * 2003-September-1   Jason Rohrer
 * Created.
 *
 * 2003-September-2   Jason Rohrer
 * Worked on implementation more.
 *
 * 2003-September-4   Jason Rohrer
 * Added code for getting directory listings.
 * Improved return value of muteGetFileInfo.
 * Fixed a return value bug.
 * Fixed an autoSprintf bug in formatting outbound messages.
 * Fixed bug in handling of root path.
 * Fixed a bug in muteGetFileInfo.
 * Added log messages.
 *
 * 2003-September-5   Jason Rohrer
 * Fixed bugs in GetFile and added log messages.
 * Fixed memory errors and leaks.
 *
 * 2003-September-7   Jason Rohrer
 * Increased timeouts.
 *
 * 2003-September-8   Jason Rohrer
 * Added support for setting timeouts in API calls.
 *
 * 2003-September-9   Jason Rohrer
 * Changed to send index.html files (if they exist) for directories.
 * Fixed a memory leak.
 *
 * 2003-September-23   Jason Rohrer
 * Switched to base64 encoding for bulk data.  Increased chunk size.
 * Fixed bug when path starts with /  .
 *
 * 2003-September-24   Jason Rohrer
 * Changed to block paths that contain .. (parent directory).
 *
 * 2003-September-25   Jason Rohrer
 * Changed to return immediately if file requested for unknown contact.
 *
 * 2003-October-13   Jason Rohrer
 * Improved handling of timeouts.
 *
 * 2003-November-11   Jason Rohrer
 * Added b flag to fopen of binary files.
 *
 * 2004-April-4    Jason Rohrer
 * Fixed redefs of variable i.  Thanks to jrocnuck.
 */



#include "fileTransmitter.h"
#include "MUTE/layers/pointToPoint/pointToPointCommunicator.h"


#include "minorGems/io/file/File.h"
#include "minorGems/io/file/Path.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SettingsManager.h"
#include "minorGems/network/web/URLUtils.h"
#include "minorGems/network/web/MimeTyper.h"
#include "minorGems/formats/encodingUtils.h"

#include "minorGems/system/Semaphore.h"

#include "minorGems/util/log/AppLog.h"



int muteFileInfoSenderID = -1;
int muteFileChunkSenderID = -1;
int muteDirectoryChunkSenderID = -1;
MimeTyper *muteMimeTyper = NULL;



// chunk size is 15 KiB.  After base64 encoding twice (once as the data block,
// and again when the file chunk message is encrypted), it will take up
// 28 KiB in a MUTE message body, leaving 4 KiB for various headers.
// The size limit on a MUTE message is 32 KiB
int muteChunkSize = 15360;



/**
 * Translates a URL-safe encoded path into a file object.
 *
 * @param inEncodedPath the URL-safe path.  Must be destroyed by caller.
 *
 * @return a file object, or NULL if the file does not exist.
 */
File *mute_internalEncodedPathToFile( char *inEncodedPath ) {
    char *path = URLUtils::hexDecode( inEncodedPath );

    if( strstr( path, ".." ) ) {
        // path may be trying to escape from our file directory
        // block it

        // note:  this may also block file names with "..", like,
        // test..txt
        // but who really names files like this anyway?
        // should really look for /.. or ../, but this might not be as
        // safe (what if, on windows, someone requests \.. directly,
        //       even though this isn't a valid MUTE path)
        
        delete [] path;
        return NULL;
        }
    
    char *pathPointer = path;
    
    if( pathPointer[ strlen( pathPointer ) - 1 ] == '/' ) {
        // path ends with /, remove it
        pathPointer[ strlen( pathPointer ) - 1 ] = '\0';
        }
    if( pathPointer[0] == '/' ) {
        // path starts with /, skip it
        pathPointer = &( pathPointer[1] );
        }
    
    
    char *platformPath;
            
    // transform it into a platform-specific path
    char platformDelim = Path::getDelimeter();

    if( platformDelim == '/' ) {
        // already in platform-specific form
        platformPath = stringDuplicate( pathPointer );
        }
    else {
        char found;

        char *platformDelimString = autoSprintf( "%c", platformDelim );
        
        // replace protocol delimeters with platform-specific delimiters
        platformPath = replaceAll( pathPointer, "/",
                                   platformDelimString, &found );

        delete [] platformDelimString;
        }

    delete [] path;

    
    char *sharingPathString = muteGetSharingPath();
    
    File *file;

    if( strlen( platformPath ) == 0 ) {
        // root directory
        file = new File( NULL, sharingPathString );
        }
    else {
        Path *sharingPath = new Path( sharingPathString );
        file = new File( sharingPath, platformPath );
        }
    delete [] sharingPathString;
    
    delete [] platformPath;
    
    if( file->exists() ) {
        return file;
        }
    else {
        AppLog::error(
            "fileTransfer",
            "File does not exist" );
        delete file;
        return NULL;
        }
    }



// handler for FileInfoRequests
char mute_internalFileInfoSender( char inContactKnown, char *inFromContact,
                                  char *inMessage,
                                  void *inExtraArgument ) {

    char returnValue = false;
    
    // ignore requests from unknown contacts
    if( !inContactKnown ) {
        return false;
        }

    // is the message a FileInfoRequest?
    // look at second token
    SimpleVector<char *> *tokens = tokenizeString( inMessage );
    int numTokens = tokens->size();

    if( numTokens == 4 ) {
        char *typeToken = *( tokens->getElement( 1 ) );
        if( strcmp( "FileInfoRequest", typeToken ) == 0 ) {

            char *encodedPath = *( tokens->getElement( 3 ) );

            File *file = mute_internalEncodedPathToFile( encodedPath );
            
            if( file != NULL ) {

                // file exists, send an info message

                char *fileType;
                int fileLength;
                char *mimeType;
                int numChunks;

                if( file->isDirectory() ) {
                    // look for an index file.
                    File *indexFile = file->getChildFile( "index.html" );
                    if( indexFile != NULL &&
                        indexFile->exists() &&
                        ! indexFile->isDirectory() ) {

                        // use index file instead of directory itself
                        delete file;
                        file = indexFile;
                        }
                    else {
                        if( indexFile != NULL ) {
                            delete indexFile;
                            }
                        }
                    }

                
                if( file->isDirectory() ) {
                    fileType = "Directory";
                    File **childFiles = file->getChildFiles( &fileLength );

                    // total size of all entries
                    // used to determine chunk count.
                    int totalEntrySize = 0;
                    
                    if( childFiles != NULL ) {
                        for( int i=0; i<fileLength; i++ ) {
                            char *fileName = childFiles[i]->getFileName();

                            char *encodedName =
                                URLUtils::hexEncode( fileName );

                            totalEntrySize += strlen( encodedName ) + 1;

                            delete [] fileName;
                            delete [] encodedName;
                            
                            delete childFiles[i];
                            }
                        delete [] childFiles;
                        }
                    numChunks = totalEntrySize / muteChunkSize;

                    if( totalEntrySize % muteChunkSize != 0 ) {
                        // extra partial chunk
                        numChunks += 1;
                        }
                    
                    mimeType = stringDuplicate( "application/octet-stream" );
                    }
                else {
                    fileType = "Data";

                    fileLength = file->getLength();

                    char *fileName = file->getFileName();
                    
                    mimeType = muteMimeTyper->getFileNameMimeType( fileName );

                    delete [] fileName;

                    if( mimeType == NULL ) {
                        // default type
                        mimeType =
                            stringDuplicate( "application/octet-stream" );
                        }

                    numChunks = fileLength / muteChunkSize;

                    if( fileLength % muteChunkSize != 0 ) {
                        // extra partial chunk
                        numChunks += 1;
                        }
                    }

                char *message = autoSprintf(
                    "MessageType: FileInfo\n"
                    "FilePath: %s\n"
                    "FileStatus: Found\n"
                    "FileType: %s\n"
                    "FileSize: %d\n"
                    "ChunkCount: %d\n"
                    "MimeType: %s",
                    encodedPath,
                    fileType,
                    fileLength,
                    numChunks,
                    mimeType );

                delete [] mimeType;
                
                delete file;

                muteSendMessageToContact( inFromContact, message );

                delete [] message;
                
                returnValue =  true;
                }
            else {
                AppLog::error(
                    "fileTransfer",
                    "FileInfoRequest does not contain a valid file path" );

                // send back a not found message
                char *message = autoSprintf(
                    "MessageType: FileInfo\n"
                    "FilePath: %s\n"
                    "FileStatus: NotFound",
                    encodedPath );

                muteSendMessageToContact( inFromContact, message );

                delete [] message;
                
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



// handler for FileChunkRequests
char mute_internalFileChunkSender( char inContactKnown, char *inFromContact,
                                   char *inMessage,
                                   void *inExtraArgument ) {
    // ignore requests from unknown contacts
    if( !inContactKnown ) {
        return false;
        }


    char returnValue = false;
    
    // is the message a FileChunkRequest?
    // look at second token
    SimpleVector<char *> *tokens = tokenizeString( inMessage );
    int numTokens = tokens->size();

    if( numTokens == 6 ) {
        char *typeToken = *( tokens->getElement( 1 ) );
        if( strcmp( "FileChunkRequest", typeToken ) == 0 ) {

            char *encodedPath = *( tokens->getElement( 3 ) );

            File *file = mute_internalEncodedPathToFile( encodedPath );
            
            if( file != NULL ) {

                // if they're asking for chunks of a directory file,
                // perhaps it has an index file associated with it.
                if( file->isDirectory() ) {
                    // look for an index file.
                    File *indexFile = file->getChildFile( "index.html" );
                    if( indexFile != NULL &&
                        indexFile->exists() &&
                        ! indexFile->isDirectory() ) {
                        
                        // use index file instead of directory itself
                        delete file;
                        file = indexFile;
                        }
                    else {
                        if( indexFile != NULL ) {
                            delete indexFile;
                            }
                        }
                    }

                // make sure they're not requesting chunks of a directory
                // file that has no index file
                if( ! file->isDirectory() ) {
                
                    int fileLength = file->getLength();
                
                    int chunksInFile = fileLength / muteChunkSize;

                    if( fileLength % muteChunkSize != 0 ) {
                        // extra partial chunk
                        chunksInFile += 1;
                        }

                    char *chunkNumberString = *( tokens->getElement( 5 ) );

                    int chunkNumber;

                    int numRead = sscanf( chunkNumberString, "%d",
                                          &chunkNumber );

                    if( numRead == 1 && chunkNumber < chunksInFile ) {

                        // send a FileChunk message

                        char *fileName = file->getFullFileName();
                    
                        FILE *fileHandle = fopen( fileName, "rb" );

                        delete [] fileName;


                        if( fileHandle != NULL ) {
                            int bytesToSkip = chunkNumber * muteChunkSize;
                            fseek( fileHandle, bytesToSkip, SEEK_SET );

                            int chunkSize = muteChunkSize;
                            if( bytesToSkip + muteChunkSize > fileLength ) {
                                // partial chunk
                                chunkSize = fileLength - bytesToSkip;
                                }

                            unsigned char *rawChunkData =
                                new unsigned char[ chunkSize ];

                            numRead = fread( rawChunkData, 1, chunkSize,
                                             fileHandle );

                            if( numRead == chunkSize ) {

                                // base64 encode with no line breaks
                                char *encodedChunkData =
                                    base64Encode( rawChunkData, chunkSize,
                                                  false );

                                char *message = autoSprintf(
                                    "MessageType: FileChunk\n"
                                    "FilePath: %s\n"
                                    "ChunkNumber: %d\n"
                                    "ChunkLength: %d\n"
                                    "ChunkData: %s",
                                    encodedPath,
                                    chunkNumber,
                                    chunkSize,
                                    encodedChunkData);

                                delete [] encodedChunkData;

                                muteSendMessageToContact( inFromContact,
                                                          message );
                                delete [] message;

                                returnValue = true;
                                }
                        
                            delete [] rawChunkData;

                            fclose( fileHandle );
                            }
                        }
                    }
                delete file;
                }
            }
        }

    for( int i=0; i<numTokens; i++ ) {
        delete [] *( tokens->getElement( i ) );
        }
    
    delete tokens;
    

    return returnValue;
    }



// handler for DirectoryChunkRequests
char mute_internalDirectoryChunkSender( char inContactKnown,
                                        char *inFromContact,
                                        char *inMessage,
                                        void *inExtraArgument ) {
    // ignore requests from unknown contacts
    if( !inContactKnown ) {
        return false;
        }


    char returnValue = false;
    
    // is the message a DirectoryChunkRequest?
    // look at second token
    SimpleVector<char *> *tokens = tokenizeString( inMessage );
    int numTokens = tokens->size();

    if( numTokens == 6 ) {
        char *typeToken = *( tokens->getElement( 1 ) );
        if( strcmp( "DirectoryChunkRequest", typeToken ) == 0 ) {

            char *encodedPath = *( tokens->getElement( 3 ) );

            File *file = mute_internalEncodedPathToFile( encodedPath );
            
            if( file != NULL ) {

                if( file->isDirectory() ) {
                    int numChildren;
                    File **childFiles = file->getChildFiles( &numChildren );


                    if( childFiles != NULL ) {

                        char *chunkNumberString = *( tokens->getElement( 5 ) );

                        int chunkNumber;
                        
                        int numRead = sscanf( chunkNumberString,
                                              "%d", &chunkNumber );

                        if( numRead == 1 ) {

                            SimpleVector<char *> *entriesInChunk =
                                new SimpleVector<char *>();
                            
                            int bytesToSkip = chunkNumber * muteChunkSize;

                            // total size of all entries
                            // used to determine chunk count.
                            int totalEntrySize = 0;

                            int i;
                            for( i=0; i<numChildren; i++ ) {
                                char *fileName = childFiles[i]->getFileName();

                                char *encodedName =
                                    URLUtils::hexEncode( fileName );

                                int nameLength = strlen( encodedName ) + 1;
                                if( totalEntrySize + nameLength > bytesToSkip
                                    &&
                                    totalEntrySize + nameLength <
                                    bytesToSkip + muteChunkSize ) {

                                    // entry is in our chunk
                                    entriesInChunk->push_back(
                                        stringDuplicate( encodedName ) );
                                    }
                                totalEntrySize += nameLength;
                                
                                delete [] fileName;
                                delete [] encodedName;
                            
                                delete childFiles[i];
                                }
                            delete [] childFiles;

                            // get entries as an array
                            int numEntries = entriesInChunk->size();
                            char **entryArray =
                                entriesInChunk->getElementArray();

                            // entries delimited by newlines
                            char *entryString = join( entryArray,
                                                      numEntries,
                                                      "\n" );
                            for( i=0; i<numEntries; i++ ) {
                                delete [] entryArray[i];
                                }
                            delete [] entryArray;
                            delete entriesInChunk;

                            char *message = autoSprintf(
                                "MessageType: DirectoryChunk\n"
                                "FilePath: %s\n"
                                "ChunkNumber: %d\n"
                                "EntryCount: %d\n"
                                "Entries: %s",
                                encodedPath,
                                chunkNumber,
                                numEntries,
                                entryString );

                            delete [] entryString;


                            muteSendMessageToContact( inFromContact, message );
                            delete [] message;

                            returnValue = true;
                            }
                        }
                    else {
                        char *fileName = file->getFullFileName();
                        char *message = autoSprintf(
                            "Failed to get children of directory: %s",
                            fileName );
                        AppLog::error( "fileTransfer",
                                       message );
                        delete [] message;
                        delete [] fileName;
                        }
                    }
                else {
                    AppLog::error(
                        "fileTransfer",
                        "Got DirectoryChunkRequest for a non-directory file" );
                    }
                delete file;
                }
            else {
                AppLog::error(
                    "fileTransfer",
                 "DirectoryChunkRequest does not contain a valid file path" );
                }
            }
        }

    for( int i=0; i<numTokens; i++ ) {
        delete [] *( tokens->getElement( i ) );
        }
    
    delete tokens;
    

    return returnValue;
    }



void muteFileTransferStart() {
    muteMimeTyper = new MimeTyper();
    
    muteFileInfoSenderID =
        muteAddContactMessageHandler( mute_internalFileInfoSender,
                                      (void *)NULL );
    muteFileChunkSenderID =
        muteAddContactMessageHandler( mute_internalFileChunkSender,
                                      (void *)NULL );
    muteDirectoryChunkSenderID =
        muteAddContactMessageHandler( mute_internalDirectoryChunkSender,
                                      (void *)NULL );
    }



void muteFileTransferStop() {

    if( muteFileInfoSenderID != -1 ) {
        muteRemoveContactMessageHandler( muteFileInfoSenderID );
        muteFileInfoSenderID = -1;
        }    
    if( muteFileChunkSenderID != -1 ) {
        muteRemoveContactMessageHandler( muteFileChunkSenderID );
        muteFileChunkSenderID = -1;
        }
    if( muteDirectoryChunkSenderID != -1 ) {
        muteRemoveContactMessageHandler( muteDirectoryChunkSenderID );
        muteDirectoryChunkSenderID = -1;
        }
    if( muteMimeTyper != NULL ) {
        delete muteMimeTyper;
        muteMimeTyper = NULL;
        }
    }



void muteSetSharingPath( char *inPath ) {
    SettingsManager::setSetting( "sharingPath", inPath );
    }



char *muteGetSharingPath() {
    char *setPath = SettingsManager::getStringSetting( "sharingPath" );
    if( setPath != NULL ) {
        return setPath;
        }
    else {
        return stringDuplicate( "files" );
        }
    }



class FileInfoWrapper {

    public:
        char *mContactName;
        char *mFilePath;
        char mFound;
        char mIsDirectory;
        int mLength;
        int mChunkCount;
        char *mMimeType;
        
        Semaphore *mSemaphore;
    };



// handler for FileInfo messages, used by muteGetFileInfo
char mute_internalFileInfoHandler( char inContactKnown, char *inFromContact,
                                   char *inMessage,
                                   void *inExtraArgument ) {

    char returnValue = false;
    
    // ignore info from unknown contacts
    if( !inContactKnown ) {
        return false;
        }

    // unwrap info object from extra arg
    FileInfoWrapper *fileInfo = (FileInfoWrapper *)inExtraArgument;

    if( strcmp( inFromContact, fileInfo->mContactName ) != 0 ) {
        // not the contact we're looking for
        return false;
        }
    
    // is this a FileInfo message?
    // look at second token
    SimpleVector<char *> *tokens = tokenizeString( inMessage );
    int numTokens = tokens->size();

    if( numTokens >= 6 ) {
        char *typeToken = *( tokens->getElement( 1 ) );
        if( strcmp( "FileInfo", typeToken ) == 0 ) {


            char *encodedPath = *( tokens->getElement( 3 ) );
            char *filePath =
                URLUtils::hexDecode( encodedPath );

            // is this info about our file?
            if( strcmp( filePath, fileInfo->mFilePath ) == 0 ) {

                char *statusString = *( tokens->getElement( 5 ) );

                // was file found?
                if( strcmp( statusString, "Found" ) == 0 ) {
                    fileInfo->mFound = true;
                    
                    if( numTokens == 14 ) {
                        char *typeString = *( tokens->getElement( 7 ) );
                        
                        if( strcmp( typeString, "Directory" ) == 0 ) {
                            fileInfo->mIsDirectory = true;
                            }
                        else {
                            fileInfo->mIsDirectory = false;
                            }
                
                        char *lengthString = *( tokens->getElement( 9 ) );
                        // default to 0
                        int length = 0;
                        sscanf( lengthString, "%d", &length );
                        
                        fileInfo->mLength = length;
                        
                        char *chunkCountString = *( tokens->getElement( 11 ) );
                        // default to 0
                        int chunkCount = 0;
                        sscanf( chunkCountString, "%d", &chunkCount );
                        
                        fileInfo->mChunkCount = chunkCount;

                        fileInfo->mMimeType =
                            stringDuplicate( *( tokens->getElement( 11 ) ) );
                        
                        fileInfo->mSemaphore->signal();
                        returnValue = true;
                        }
                    }
                else {
                    // not found, but we got the info
                    fileInfo->mFound = false;

                    fileInfo->mSemaphore->signal();
                    returnValue = true;
                    }
                }
            delete [] filePath;
            }
        }
    
    for( int i=0; i<numTokens; i++ ) {
        delete [] *( tokens->getElement( i ) );
        }
    
    delete tokens;
    

    return returnValue;
    }



int muteGetFileInfo( char *inContactName, char *inFilePath,
                     char *outIsDirectory, int *outLength,
                     int *outChunkCount,
                     char **outMimeType,
                     int inTimeoutInMilliseconds ) {

    FileInfoWrapper *fileInfo = new FileInfoWrapper();
    fileInfo->mContactName = inContactName;
    fileInfo->mFilePath = inFilePath;
    fileInfo->mSemaphore = new Semaphore();
    fileInfo->mMimeType = NULL;
    
    // register a handler
    int handlerID = muteAddContactMessageHandler( mute_internalFileInfoHandler,
                                                  (void *)fileInfo );

    // send out a request message
    char *encodedFilePath = URLUtils::hexEncode( inFilePath );

    char *message = autoSprintf(
        "MessageType: FileInfoRequest\n"
        "FilePath: %s",
        encodedFilePath );
    delete [] encodedFilePath;
    
    char contactKnown = muteSendMessageToContact( inContactName, message );
    delete [] message;


    int returnValue;
    
    if( !contactKnown ) {
        muteRemoveContactMessageHandler( handlerID );

        *outMimeType = NULL;
        
        returnValue = MUTE_FILE_CONTACT_UNKNOWN;
        }
    else {    
        // wait for our handler to get the response
        int responseReceived =
            fileInfo->mSemaphore->wait( inTimeoutInMilliseconds );

        muteRemoveContactMessageHandler( handlerID );

        if( responseReceived == 1 ) {
            
            if( fileInfo->mFound ) {
                *outIsDirectory = fileInfo->mIsDirectory;
                *outLength = fileInfo->mLength;
                *outChunkCount = fileInfo->mChunkCount;
                *outMimeType = fileInfo->mMimeType;
                
                returnValue = MUTE_FILE_FOUND;
                }
            else {
                *outMimeType = NULL;
                returnValue = MUTE_FILE_NOT_FOUND;
                }
            }
        else {
            *outMimeType = NULL;
            returnValue = MUTE_FILE_CONTACT_NOT_REACHABLE;
            
            if( fileInfo->mMimeType != NULL ) {
                // maybe handler was called between our timeout and
                // our call to muteRemoveContactMessageHandler
                
                // ignore it anyway, but clean up
                delete [] fileInfo->mMimeType;
                fileInfo->mMimeType = NULL;
                }
            }
        }

    
    delete fileInfo->mSemaphore;
    delete fileInfo;
    
    return returnValue;
    }



class FileChunkWrapper {

    public:
        char *mContactName;
        char *mFilePath;

        int mChunkNumber;
        int mLengthInBytes;
        unsigned char *mChunkData;
        
        Semaphore *mSemaphore;
    };



// handler for FileChunk messages, used by muteGetFile
char mute_internalFileChunkHandler( char inContactKnown, char *inFromContact,
                                    char *inMessage,
                                    void *inExtraArgument ) {
    char returnValue = false;
    
    // ignore info from unknown contacts
    if( !inContactKnown ) {
        return false;
        }

    // unwrap info object from extra arg
    FileChunkWrapper *fileChunk = (FileChunkWrapper *)inExtraArgument;

    if( strcmp( inFromContact, fileChunk->mContactName ) != 0 ) {
        // not the contact we're looking for
        return false;
        }
    
    // is this a FileChunk message?
    // look at second token
    SimpleVector<char *> *tokens = tokenizeString( inMessage );
    int numTokens = tokens->size();

    if( numTokens == 10 ) {
        char *typeToken = *( tokens->getElement( 1 ) );
        if( strcmp( "FileChunk", typeToken ) == 0 ) {


            char *encodedPath = *( tokens->getElement( 3 ) );
            char *filePath =
                URLUtils::hexDecode( encodedPath );

            // is this info about our file?
            if( strcmp( filePath, fileChunk->mFilePath ) == 0 ) {
            
                char *chunkNumberString = *( tokens->getElement( 5 ) );
                // default to 0
                int chunkNumber = 0;
                sscanf( chunkNumberString, "%d", &chunkNumber );

                // is this the chunk we're looking for?
                if( fileChunk->mChunkNumber == chunkNumber ) {
                                                                    
                    char *lengthString = *( tokens->getElement( 7 ) );
                    // default to 0
                    int length = 0;
                    sscanf( lengthString, "%d", &length );
                    
                    fileChunk->mLengthInBytes = length;
                    
                    char *encodedData = *( tokens->getElement( 9 ) );
                    int dataLength;
                    
                    unsigned char *decodedData =
                        base64Decode( encodedData, &dataLength );
                    
                    if( decodedData != NULL ) {
                        
                        if( dataLength == length ) {
                            
                            fileChunk->mChunkData = decodedData;
                            
                            fileChunk->mSemaphore->signal();
                            returnValue = true;
                            }
                        else {
                            AppLog::error(
                                "fileTransmitter -- chunk handler",
                                "Data length incorrect" );
                            }
                        }
                    else {
                        AppLog::error(
                            "fileTransmitter -- chunk handler",
                            "Failed to decode data (hex format bad?)" );
                        }
                    
                    }
                }
            delete [] filePath;
            }
        }

        for( int i=0; i<numTokens; i++ ) {
        delete [] *( tokens->getElement( i ) );
        }
    
    delete tokens;
    

    return returnValue;
    }



char muteGetFile( char *inContactName, char *inFilePath,
                  void (*inFileChunkHandler)( unsigned char *, int, void * ),
                  void *inExtraHandlerArgument,
                  int inTimeoutInMilliseconds ) {

    char *logMessage = autoSprintf(
        "Trying to get file %s : %s", inContactName, inFilePath );
    AppLog::info( "fileTransfer",
                  logMessage );
    delete [] logMessage;
    
    
    char isDirectory;
    int length;
    int chunkCount;
    char *mimeType;

    int status = muteGetFileInfo( inContactName, inFilePath,
                                  &isDirectory, &length,
                                  &chunkCount,
                                  &mimeType,
                                  inTimeoutInMilliseconds );

    if( status != MUTE_FILE_FOUND ) {
        logMessage = autoSprintf(
            "Aborting file get:  failed to fetch info for file  %s : %s",
            inContactName, inFilePath );
        AppLog::error( "fileTransfer",
                       logMessage );
        delete [] logMessage;
        
        return false;
        }

    // we don't need the mime type
    delete [] mimeType;


    // ignore requests for directories
    if( isDirectory ) {
        logMessage = autoSprintf(
            "Aborting file get:  file is a directory  %s : %s",
            inContactName, inFilePath );
        AppLog::error( "fileTransfer",
                       logMessage );
        delete [] logMessage;
        

        return false;
        }

    char *encodedFilePath = URLUtils::hexEncode( inFilePath );

    char chunkMissed = false;
    for( int i=0; i<chunkCount && !chunkMissed; i++ ) {

        // register a handler for the chunk
        FileChunkWrapper *fileChunk = new FileChunkWrapper();
        fileChunk->mContactName = inContactName;
        fileChunk->mFilePath = inFilePath;
        fileChunk->mChunkNumber = i;
        fileChunk->mSemaphore = new Semaphore();
        fileChunk->mChunkData = NULL;
        
        int handlerID =
            muteAddContactMessageHandler( mute_internalFileChunkHandler,
                                          (void *)fileChunk );

        

        // send out a request message for the chunk

        char *message = autoSprintf(
            "MessageType: FileChunkRequest\n"
            "FilePath: %s\n"
            "Chunknumber: %d",
            encodedFilePath,
            i );
        
        muteSendMessageToContact( inContactName, message );
        delete [] message;
        

        // wait for our handler to get the response
        int responseReceived =
            fileChunk->mSemaphore->wait( inTimeoutInMilliseconds );

        muteRemoveContactMessageHandler( handlerID );

        if( responseReceived == 1 ) {
            // pass the chunk to the caller's handler
            inFileChunkHandler( fileChunk->mChunkData,
                                fileChunk->mLengthInBytes,
                                inExtraHandlerArgument );
                
            delete [] fileChunk->mChunkData;
            }
        else {
            char *logMessage = autoSprintf(
                "Timed out on chunk %d of file %s : %s",
                i, inContactName, inFilePath );
            AppLog::error( "fileTransfer",
                           logMessage );
            delete [] logMessage;
            
            chunkMissed = true;

            if( fileChunk->mChunkData != NULL ) {
                // maybe handler was called between our timeout and
                // our call to muteRemoveContactMessageHandler

                delete [] fileChunk->mChunkData;
                fileChunk->mChunkData = NULL;
                }
            }

        delete fileChunk->mSemaphore;
        delete fileChunk;
        }

    delete [] encodedFilePath;


    if( chunkMissed ) {
        return false;
        }
    else {
        return true;
        }
    }



class DirectoryChunkWrapper {

    public:
        char *mContactName;
        char *mDirectoryPath;

        int mChunkNumber;

        SimpleVector<char *> *mDirectoryEntries;
        
        Semaphore *mSemaphore;
    };



// handler for DirectoryChunk messages, used by muteGetFile
char mute_internalDirectoryChunkHandler( char inContactKnown,
                                         char *inFromContact,
                                         char *inMessage,
                                         void *inExtraArgument ) {

    char returnValue = false;
    
    // ignore info from unknown contacts
    if( !inContactKnown ) {
        return false;
        }

    // unwrap info object from extra arg
    DirectoryChunkWrapper *dirChunk = (DirectoryChunkWrapper *)inExtraArgument;

    if( strcmp( inFromContact, dirChunk->mContactName ) != 0 ) {
        // not the contact we're looking for
        return false;
        }
    
    // is this a DirectoryChunk message?
    // look at second token
    SimpleVector<char *> *tokens = tokenizeString( inMessage );
    int numTokens = tokens->size();

    if( numTokens >= 9 ) {
        char *typeToken = *( tokens->getElement( 1 ) );
        if( strcmp( "DirectoryChunk", typeToken ) == 0 ) {


            char *encodedPath = *( tokens->getElement( 3 ) );
            char *filePath =
                URLUtils::hexDecode( encodedPath );

            // is this info about our file?
            if( strcmp( filePath, dirChunk->mDirectoryPath ) == 0 ) {
            
                char *chunkNumberString = *( tokens->getElement( 5 ) );
                // default to 0
                int chunkNumber = 0;
                sscanf( chunkNumberString, "%d", &chunkNumber );

                // is this the chunk we're looking for?
                if( dirChunk->mChunkNumber == chunkNumber ) {
                                                                    
                    char *entryCountString = *( tokens->getElement( 7 ) );
                    // default to 0
                    int entryCount = 0;
                    sscanf( entryCountString, "%d", &entryCount );

                    // are there enough tokens left to meet the entryCount?
                    if( entryCount == numTokens - 9 ) {
                        
                        // push each decoded entry onto the dirChunk vector
                        for( int i=9; i<numTokens; i++ ) {
                            char *encodedEntry = *( tokens->getElement( i ) );
                            char *decodedEntry =
                                URLUtils::hexDecode( encodedEntry ); 

                            dirChunk->mDirectoryEntries->push_back(
                                 decodedEntry );
                            }
                        
                        // we received a valid chunk
                        dirChunk->mSemaphore->signal();
                        returnValue = true;
                        }
                    }
                }
            delete [] filePath;                
            }
        }

    for( int i=0; i<numTokens; i++ ) {
        delete [] *( tokens->getElement( i ) );
        }
    
    delete tokens;
    

    return returnValue;
    }



char **muteGetDirectoryListing( char *inContactName, char *inDirectoryPath,
                                int *outFileCount,
                                int inTimeoutInMilliseconds ) {

    char isDirectory;
    int length;
    int chunkCount;
    char *mimeType;

    int status = muteGetFileInfo( inContactName, inDirectoryPath,
                                  &isDirectory, &length,
                                  &chunkCount,
                                  &mimeType,
                                  inTimeoutInMilliseconds );

    if( status != MUTE_FILE_FOUND ) {
        return NULL;
        }

    // ignore mime type
    delete [] mimeType;

    
    // ignore requests for data files
    if( !isDirectory ) {
        return NULL;
        }


    if( length == 0 ) {
        // an empty directory

        *outFileCount = 0;

        return new char*[0];
        }

    
    char *encodedFilePath = URLUtils::hexEncode( inDirectoryPath );

    char chunkMissed = false;
    SimpleVector<char *> *entries = new SimpleVector<char *>();
    
    for( int i=0; i<chunkCount && !chunkMissed; i++ ) {

        // register a handler for the chunk
        DirectoryChunkWrapper *dirChunk = new DirectoryChunkWrapper();
        dirChunk->mContactName = inContactName;
        dirChunk->mDirectoryPath = inDirectoryPath;
        dirChunk->mChunkNumber = i;
        dirChunk->mDirectoryEntries = entries;
        dirChunk->mSemaphore = new Semaphore();

        int handlerID =
            muteAddContactMessageHandler( mute_internalDirectoryChunkHandler,
                                          (void *)dirChunk );


        // send out a request message for the chunk

        char *message = autoSprintf(
            "MessageType: DirectoryChunkRequest\n"
            "FilePath: %s\n"
            "Chunknumber: %d",
            encodedFilePath,
            i );
        
        muteSendMessageToContact( inContactName, message );
        delete [] message;
        

        // wait for our handler to get the response
        int responseReceived =
            dirChunk->mSemaphore->wait( inTimeoutInMilliseconds );

        muteRemoveContactMessageHandler( handlerID );

        if( responseReceived != 1 ) {
            chunkMissed = true;
            }

        delete dirChunk->mSemaphore;
        delete dirChunk;
        }

    delete [] encodedFilePath;

    char **returnArray;
    
    if( chunkMissed ) {
        returnArray = NULL;
        *outFileCount = 0;

        int numEntries = entries->size();
        for( int i=0; i<numEntries; i++ ) {
            char *entry = *( entries->getElement( i ) ); 
            delete [] entry;
            }        
        }
    else {
        returnArray = entries->getElementArray();
        *outFileCount = entries->size();
        }

    delete entries;
    
    return returnArray;
    }

