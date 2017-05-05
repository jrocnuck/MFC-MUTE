#include "debugout.h"
// jroc remove later
__int64 g_nBytesDownloaded = 0;
__int64 g_nBytesSearchResultsRcvd = 0;
/*
 * Modification History
 *
 * 2003-October-13   Jason Rohrer
 * Created.
 *
 * 2003-October-14   Jason Rohrer
 * Finished implementation and got it to compile.
 *
 * 2003-October-16   Jason Rohrer
 * Fixed a deletion bug.
 * Added support for a file size return pointer in muteShareGetFile.
 *
 * 2003-November-4   Jason Rohrer
 * Added a function for getting upload info.
 *
 * 2003-November-5   Jason Rohrer
 * Fixed a variable init bug.
 *
 * 2003-November-6   Jason Rohrer
 * Added support for sending file hashes and sizes with search results.
 *
 * 2003-November-9   Jason Rohrer
 * Fixed a memory leak.
 *
 * 2003-November-10   Jason Rohrer
 * Removed unused timing code.
 *
 * 2003-November-11   Jason Rohrer
 * Added b flag to fopen of binary files.
 * Fixed bug if creating hash fails.
 *
 * 2003-November-24   Jason Rohrer
 * Added support for retrying chunk request with a FRESH_ROUTE flag.
 * Added support for retrying file info request with a FRESH_ROUTE flag.
 *
 * 2003-December-5   Jason Rohrer
 * Added support for message utility.
 *
 * 2003-December-19   Jason Rohrer
 * Changed to handle sharing paths with spaces.
 *
 * 2003-December-22   Jason Rohrer
 * Fixed a bug when hashing empty files.
 *
 * 2003-December-23   Jason Rohrer
 * Added hash checking for incoming files.
 * Added support for hash-only searches.
 *
 * 2003-December-26   Jason Rohrer
 * Added log messages when handling a search request.
 *
 * 2004-January-2   Jason Rohrer
 * Added MUTE prefix to hash file names.
 *
 * 2004-January-4   Jason Rohrer
 * Added support for searching subdirectories.
 *
 * 2004-January-5   Jason Rohrer
 * Fixed a bug in use of platform-specific path delimeters.
 * Added setting for subfolder depth limit.
 *
 * 2004-January-18   Jason Rohrer
 * Fixed a memory leak discovered by John Backstrand.
 *
 * 2004-January-25   Jason Rohrer
 * Added dynamic timeout code.  Still need to test it.
 *
 * 2004-January-25   Jason Rohrer
 * Tested and improved dynamic timeout code.
 *
 * 2004-February-6   Jason Rohrer
 * Added FORWARD flag to search requests.
 *
 * 2004-February-12   Jason Rohrer
 * Added upload stats patch submitted by Mycroftxxx.
 *
 * 2004-February-13   Jason Rohrer
 * Cleaned up the patched upload-tracking code.
 *
 * 2004-February-20   Jason Rohrer
 * Added code to gradually decrease timeouts using weighted averages.
 * Changed chunk size from 22 KiB to 16 KiB.
 *
 * 2004-February-21   Jason Rohrer
 * Added multiple retry support.
 * Changed GetFile callback spec to support fine-grained retry reporting.
 *
 * 2004-February-26   Jason Rohrer
 * Changed to use overall longest timeout to compute final retry timeout.
 * Changed so that request timer not reset for each retry.
 *
 * 2004-March-7   Jason Rohrer
 * Changed to avoid increasing timeouts using timing from multiple retries.
 *
 * 2004-March-8   Jason Rohrer
 * Added support for passing fileInfo timeouts to file chunk handler.
 *
 * 2004-March-9   Jason Rohrer
 * Added support for new FORWARD scheme.
 *
 * 2004-March-25   Jason Rohrer
 * Added support for separate download timeout logging.
 * Fixed a bug in setting retry timeout.
 * Changed to be more cautious about doubling the current timeout.
 * Added setting for printing a search synchronization trace.
 *
 * 2004-March-28   Jason Rohrer
 * Added fix to ignore empty searches with no search terms.
 */



#include "fileShare.h"
#include "MUTE/layers/messageRouting/messageRouter.h"


//#include "minorGems/io/file/File.h" --  jroc 01-22-2005 moved to fileShare.h
#include "minorGems/io/file/Path.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SettingsManager.h"
#include "minorGems/network/web/URLUtils.h"
#include "minorGems/network/web/MimeTyper.h"
#include "minorGems/formats/encodingUtils.h"
#include "minorGems/system/BinarySemaphore.h"

#include "minorGems/system/Semaphore.h"

#include "minorGems/system/MutexLock.h"
#include "minorGems/system/Time.h"

#include "minorGems/util/SimpleVector.h"

#include "minorGems/util/log/AppLog.h"




#include "minorGems/util/random/RandomSource.h"
#include "minorGems/util/random/StdRandomSource.h"


//#include "debugout.h"
//char szJROCDebug[1024];
#include <stdlib.h>


// minimum time needed before a chunk retry is considered valid
#define MIN_UPLOAD_RETRY 20

int muteShareFileInfoSenderID = -1;
int muteShareFileChunkSenderID = -1;
int muteShareSearchResultsSenderID = -1;
MimeTyper *muteShareMimeTyper = NULL;
char *muteShareVirtualAddress = NULL;

char g_muteIsStopping;					// jroc
HashBuilderThread	*muteHashBuilder = NULL; // jroc

// shared folder caching code --- jroc
#include <vector>

using namespace std;
typedef struct
{
	char szFileHash[50];
	char szFileName[_MAX_PATH + 1];
	unsigned long	ulSize;
} _filecacheitem;

char					 g_SharedFolderCacheReady = 0;
unsigned int			 g_unSearchStartIndex = 0; // use a random starting place in the search
time_t					 g_LastFileCacheUpdateTime = 0;
time_t					 g_FileCacheUpdateDelay = 600; // start out with 600 seconds (10 minutes) and grow up to 30 minutes...
vector< _filecacheitem > g_vecSharedFileCache;

// end shared folder caching code --- jroc
// jroc... keep track of uploaded chunks!
unsigned long g_ulChunksSent = 0;
__int64 g_nBytesSent = 0;
MutexLock *muteUploadDataLock = NULL;
int muteNextUploadID = 0;
int nStopUploadUpdates = 0;
SimpleVector<long> *muteUploadIDs = NULL;
SimpleVector<char *> *muteUploadFilePaths = NULL;
SimpleVector<char *> *muteUploadHostAddresses = NULL;
SimpleVector<long> *muteUploadLastChunksSent = NULL;
SimpleVector<long> *muteUploadChunksInFiles = NULL;
SimpleVector<unsigned long> *muteUploadFirstChunkTimes = NULL;
SimpleVector<unsigned long> *muteUploadLastChunkTimes = NULL;


// NATE's upload controls variables...
extern int max_uploads; // max uploads at same time
extern int max_host_uploads; // max uploads from a single host

bool uploadsFull = false; // flag for when all upload slots are full

// chunk size is 16 KiB.  After base64 encoding, it will take up less than
// 22 KiB in a MUTE message body, leaving room for various headers.
// The size limit on a MUTE message is 32 KiB
//
// In previous versions, the chunk size was 22 KiB before encoding.
// We now use 16KiB so that the chunk size is a power of 2 and therefore
// compatible with many partial hashing schemes (TigerTrees, in particular)
int muteShareChunkSize = 16384;


char muteSharePrintSearchSyncTrace = false;




// sub-directory in shared directory where file hashes are cached 
//char *muteHashDirectoryName = "MUTE_hashes"; // jroc
// sub-directory in shared directory where incoming files are kept 
//char *muteIncomingDirectoryName = "MUTE_incoming";  //jroc



/**
 * Computes the SHA1 hash of a file.
 *
 * @param inFile the file to hash.
 *   Must be destroyed by caller.
 *
 * @return the SHA1 hash as a hex-encoded string, or NULL on an error.
 *   Must be destroyed by caller.
 */
char *muteShare_internalHashFile( File *inFile ) {
    
	int bufferSize = 50000;

    int bytesHashed = 0;

    int fileSize = inFile->getLength();
	
// jroc
	unsigned long	ulLastPauseTime = time(NULL);
	unsigned long	ulTimeLastByteProcessed;
	unsigned long	ulTimeDelta;
	unsigned long	ulSleepCtr;

   
    // don't bother checking if file is empty here, since there
    // is a default SHA1 value for empty strings:
    // DA39A3EE5E6B4B0D3255BFEF95601890AFD80709

    
    char *fileName = inFile->getFullFileName();
    
    FILE *fileHandle = fopen( fileName, "rb" );

    delete [] fileName;

    if( fileHandle == NULL ) {
        return NULL;
        }


    SHA_CTX shaContext;

    SHA1_Init( &shaContext );

    
    unsigned char *buffer = new unsigned char[ bufferSize ]; 
    char error = false;

    while( bytesHashed < fileSize && !error ) {
        
        int blockSize = bufferSize;

        if( blockSize + bytesHashed > fileSize ) {
            // partial block
            blockSize = fileSize - bytesHashed;
		}
		
		Thread::sleep(0); // give up processor to somebody else		
		ulTimeLastByteProcessed = time(NULL);		
		ulTimeDelta = ulTimeLastByteProcessed - ulLastPauseTime;
		// make this thread pause for a second every three seconds
		if( !g_muteIsStopping && ( ulTimeDelta > 3 ) )
		{
			// lets pause for a second so 
			// we free up processor for other threads... this is really
			// necessary when we are hashing a huge file
			ulSleepCtr = 0;
			while( (ulSleepCtr < 10) && !g_muteIsStopping )
			{
				Thread::sleep(100);
				ulSleepCtr++;
			}
			ulLastPauseTime = time(NULL);
		}

		
        int numRead = fread( buffer, 1, blockSize,
			fileHandle );
		
        if( numRead == blockSize ) {
            SHA1_Update( &shaContext, buffer, blockSize );

            bytesHashed += blockSize;
            }
        else {
            error = true;
            }
        }

    fclose( fileHandle );
    delete [] buffer;
    
    unsigned char *rawDigest = new unsigned char[ SHA1_DIGEST_LENGTH ];

    SHA1_Final( rawDigest, &shaContext );


    if( error ) {
        delete [] rawDigest;
        return NULL;
        }

    // else hash is correct
    char *digestHexString = hexEncode( rawDigest, SHA1_DIGEST_LENGTH );
    
    delete [] rawDigest;

    
    return digestHexString;    
    }



/**
 * Translates a URL-safe encoded path into a file object.
 *
 * @param inEncodedPath the URL-safe path.  Must be destroyed by caller.
 *
 * @return a file object, or NULL if the file does not exist.
 *   Must be destroyed by caller.
 */
File *muteShare_internalEncodedPathToFile( char *inEncodedPath ) {
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

    
    char *sharingPathString = muteShareGetSharingPath();
    
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



/**
 * Translates a file object into a URL-safe encoded path.
 *
 * Note that this call returns NULL for files that are inside
 * MUTE_hashes and MUTE_incoming directories.
 *
 * @param inFile a file object.  Must be destroyed by caller.
 *
 * @return the URL-safe path, or NULL on error.
 *   Must be destroyed by caller.
 */
char *muteShare_internalFileToEncodedPath( File *inFile ) {

    char *muteHashDir = muteShareGetHashFilesPath(); // jroc
	char *muteIncomingDir = muteShareGetIncomingFilesPath(); //jroc

	char platformDelim = Path::getDelimeter();

    char *sharingPathString = muteShareGetSharingPath();

    char *fullFilePathString = inFile->getFullFileName();

    
    if( strstr( fullFilePathString, muteHashDir ) != NULL ||
        strstr( fullFilePathString, muteIncomingDir ) != NULL ) {

        // ignore files in these directories

        delete [] sharingPathString;
        delete [] fullFilePathString;
		delete [] muteHashDir;		// jroc
		delete [] muteIncomingDir;	// jroc

        return NULL;        
        }
        
    
    // remove sharing path from file name
    char *pointerToSharingPath = strstr( fullFilePathString,
                                         sharingPathString );

    char *returnPathString = NULL;
    if( pointerToSharingPath != NULL ) {

        // skip the sharing path
        char *partialFilePath =
            &( fullFilePathString[ strlen( sharingPathString ) ] );


        if( strlen( partialFilePath ) > 0 ) {
            // make sure we didn't miss the final delimiter
            if( partialFilePath[0] == platformDelim ) {

                // skip the first character (delimeter
                partialFilePath = &( partialFilePath[1] );
                }
            }

        if( platformDelim == '/' ) {
            // already using universal path delimeter
            returnPathString = stringDuplicate( partialFilePath );
            }
        else {
            // replace the platform-specific path delimeter with the universal
            // delimeter "/"
            char *platformDelimString = autoSprintf( "%c", platformDelim );
            
            char found;

            returnPathString = replaceAll( partialFilePath,
                                           platformDelimString,
                                           "/",
                                           &found );

            delete [] platformDelimString;
            }
        }

    delete [] sharingPathString;
    delete [] fullFilePathString;
	delete [] muteHashDir;		// jroc
	delete [] muteIncomingDir;	// jroc

    return returnPathString;
    }



// handler for FileInfoRequests
int muteShare_internalFileInfoSender( char *inFromAddress, char *inToAddress,
                                      char *inBody,
                                      void *inExtraArgument ) {

	//jroc
	int nRetVal = 0;

    // is the message a FileInfoRequest?
    // look at second token
    SimpleVector<char *> *tokens = tokenizeString( inBody );
    int numTokens = tokens->size();

    if( numTokens == 4 ) {
        char *typeToken = *( tokens->getElement( 1 ) );
        if( strcmp( "FileInfoRequest", typeToken ) == 0 ) {

			// jroc
			nRetVal = -1; // so we stop searching through handlers
            char *encodedPath = *( tokens->getElement( 3 ) );

            File *file = muteShare_internalEncodedPathToFile( encodedPath );
            
            if( file != NULL &&
                ! file->isDirectory() ) {

                // file exists, send an info message
                int fileLength;
                char *mimeType;
                int numChunks;

                
                fileLength = file->getLength();

                char *fileName = file->getFileName();
                    
                mimeType = muteShareMimeTyper->getFileNameMimeType( fileName );

                delete [] fileName;

                if( mimeType == NULL ) {
                    // default type
                    mimeType =
                        stringDuplicate( "application/octet-stream" );
                    }

                numChunks = fileLength / muteShareChunkSize;

                if( fileLength % muteShareChunkSize != 0 ) {
                    // extra partial chunk
                    numChunks += 1;
                    }

                char *message = autoSprintf(
                    "MessageType: FileInfo\n"
                    "FilePath: %s\n"
                    "FileStatus: Found\n"
                    "FileSize: %d\n"
                    "ChunkCount: %d\n"
                    "MimeType: %s",
                    encodedPath,
                    fileLength,
                    numChunks,
                    mimeType );

                delete [] mimeType;
				mimeType = NULL;

                muteSendMessage( muteShareVirtualAddress,
                                 inFromAddress, message );

                delete [] message;
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

                muteSendMessage( muteShareVirtualAddress,
                                 inFromAddress,
                                 message );

                delete [] message;
                }

            if( file != NULL ) {
                delete file;
                }
            
            }
        
        }
   
    for( int i=0; i<numTokens; i++ ) {
        delete [] *( tokens->getElement( i ) );
        }
    
    delete tokens;

    // no utility generated
    //return 0;
	// jroc
	return nRetVal;
    }



// reports that a chunk has been sent for a file.
// inFilePath and inHostAddress must be destroyed by caller
void muteShare_internalChunkSent( char *inFilePath, 
                                  char *inHostAddress, 
                                  int inChunkNumber, 
                                  int inChunksInFile ) {
    // get current time in seconds
    unsigned long currentSeconds = time( NULL );

    // Search for the file in our upload data; both file path and destination
    // address must match to be considered the same entry.  Multiple uploads
    // of the same file to different destinations are recorded separately.
    char found = false;
    int foundIndex = -1;

    muteUploadDataLock->lock();

	// jroc
	// want to stop updating until any uploads are cleared.
	if( nStopUploadUpdates )
	{
		muteUploadDataLock->unlock();
		return;
	}

    int numUploads = muteUploadIDs->size();
    for( int i=0; i<numUploads && !found; i++ ) {
       
        if( strcmp( inFilePath,
                    *( muteUploadFilePaths->getElement(i) ) ) == 0 &&
            strcmp( inHostAddress,
                    *( muteUploadHostAddresses->getElement(i) ) ) == 0 ) {

            // both file path and host address match
            
            found = true;
            foundIndex = i;
            }       
        }

    if( found ) {
        // save last chunk sent index
        long *indexPointer
            = muteUploadLastChunksSent->getElement( foundIndex ); 
        *indexPointer = inChunkNumber;

        // save time of last chunk send
        unsigned long *timePointer
            = muteUploadLastChunkTimes->getElement( foundIndex ); 
        *timePointer = currentSeconds;
        }
    else {
        // add a new upload to end of vector
        muteUploadIDs->push_back( muteNextUploadID );
        muteNextUploadID++;

        muteUploadFilePaths->push_back( stringDuplicate( inFilePath ) );
        muteUploadHostAddresses->push_back( stringDuplicate( inHostAddress ) );
        muteUploadLastChunksSent->push_back( inChunkNumber );
        muteUploadChunksInFiles->push_back( inChunksInFile );
        muteUploadFirstChunkTimes->push_back( currentSeconds );
        muteUploadLastChunkTimes->push_back( currentSeconds );
        }

    muteUploadDataLock->unlock();
    }



// handler for FileChunkRequests
int muteShare_internalFileChunkSender( char *inFromAddress, char *inToAddress,
                                       char *inBody,
                                       void *inExtraArgument ) {
	   
	// jroc
	int nRetVal = 0;

    // is the message a FileChunkRequest?
    // look at second token
    SimpleVector<char *> *tokens = tokenizeString( inBody );
    int numTokens = tokens->size();

    if( numTokens == 6 ) {
        char *typeToken = *( tokens->getElement( 1 ) );
        if( strcmp( "FileChunkRequest", typeToken ) == 0 ) {

			// jroc
			nRetVal = -1;

            char *encodedPath = *( tokens->getElement( 3 ) );
			
            File *file = muteShare_internalEncodedPathToFile( encodedPath );
            
            char *chunkNumberString = *( tokens->getElement( 5 ) );

            int chunkNumber;

            int numRead = sscanf( chunkNumberString, "%d",
                                  &chunkNumber );
            if( file != NULL && numRead == 1 ) {
				
				// Nate upload control 
				int numUploads = muteUploadIDs->size();
				
				// make sure they're not requesting chunks of a directory
                // file or there are too many uploads running already
                if(! file->isDirectory()) 
				{					
					//NATE STUFF...
					char *szfileName = file->getFullFileName();
					muteUploadDataLock->lock();
					// limit the number of uploads from same V-IP
					numUploads = muteUploadIDs->size();
					int numSame = 0;
					int numRunningUploads = 0;
					unsigned long currentSeconds = time( NULL );
					unsigned long elapsedSeconds;
					long numChunksSent;
					long numChunksInFile;
					bool pass = false;
					for( int i=0; i<numUploads && !pass; i++ ) 
					{
						// failed (after 5 min it's dead) or done ?
						elapsedSeconds = currentSeconds -
							*( muteUploadLastChunkTimes->getElement(i) );
						numChunksSent =
							*( muteUploadLastChunksSent->getElement(i) ) + 1;
						numChunksInFile =
							*( muteUploadChunksInFiles->getElement(i) );
						if( (elapsedSeconds < 300) &&
							(numChunksSent != numChunksInFile) ) {
                            numRunningUploads++;
							
                            // chunk is for same V-IP but not same file name
                            pass = false;
                            if( strcmp( inFromAddress,
								*( muteUploadHostAddresses->getElement(i) ) ) == 0 ) 
							{
                                numSame++;
                                if ( strcmp( szfileName,
									*( muteUploadFilePaths->getElement(i) ) ) == 0 ) 
								{
                                    pass = true; // existing let it through
                                     // same chunk as last time within MIN_UPLOAD_RETRY seconds?
                                     // give it time to get there, they resend if needed
                                     if( (numChunksSent == (chunkNumber + 1)) &&
                                         (elapsedSeconds < MIN_UPLOAD_RETRY) ) 
									 {
                                         pass = false; // make it not go
                                         numSame = max_host_uploads;
                                         break;
									 }
								}
							}
						}
					}
					muteUploadDataLock->unlock();
					
					if( pass || ((numSame < max_host_uploads) && (numRunningUploads < max_uploads))) 
					{
						
						// jroc
						unsigned long fileLength = file->getLength();                
						unsigned long chunksInFile = fileLength / muteShareChunkSize;
						
						if( fileLength % muteShareChunkSize != 0 ) {
							// extra partial chunk
							chunksInFile += 1;
                        }
						
						
						if( (unsigned int) chunkNumber < (unsigned int) chunksInFile ) 
						{
							
							// send a FileChunk message													
							
							FILE *fileHandle = fopen( szfileName, "rb" );													
							
							if( fileHandle != NULL ) 
							{
								// jroc
								unsigned long bytesToSkip = chunkNumber * muteShareChunkSize;
								fseek( fileHandle, bytesToSkip, SEEK_SET );
								
								int chunkSize = muteShareChunkSize;
								if( bytesToSkip + muteShareChunkSize
									> fileLength ) {
									
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
									g_nBytesSent += numRead;
									g_ulChunksSent++;
									muteSendMessage( muteShareVirtualAddress,
										inFromAddress,
										message );

									delete [] message;
									
									muteShare_internalChunkSent( szfileName,
										inFromAddress,
										chunkNumber,
										chunksInFile );
                                }
								
									delete [] rawChunkData;
								
									fclose( fileHandle );
								}
							}						
						}

						delete [] szfileName;
                    }
					delete file;
                }
            }
        }

    for( int i=0; i<numTokens; i++ ) {
        delete [] *( tokens->getElement( i ) );
        }
    
    delete tokens;

    // no utility generated
    //return 0;
	// jroc
	return nRetVal;
    }



// utility penalty for searches that contain no terms
// should cause them to be dropped quickly
int muteShareBlankSearchUtilityPenalty = 1000;



// handler for SearchRequests
int muteShare_internalSearchResultsSender( char *inFromAddress,
                                           char *inToAddress,
                                           char *inBody,
                                           void *inExtraArgument ) 
{
	
	int i;
    int utility = 0;
	// jroc.. 
	char bIsWildCardSearch = 0;
    
    // is the message a SearchRequest?
    // look at second token
    SimpleVector<char *> *tokens = tokenizeString( inBody );
    int numTokens = tokens->size();

    if( numTokens == 6 ) 
	{
        char *typeToken = *( tokens->getElement( 1 ) );		

		if( strcmp( "SearchRequest", typeToken ) == 0 && !uploadsFull )
		{			
            char *searchID = *( tokens->getElement( 3 ) );
            char *encodedSearchString = *( tokens->getElement( 5 ) );

            char *searchString = URLUtils::hexDecode( encodedSearchString );
			
            char *logMessage = autoSprintf(
                "Got search request for (%s), id=%s",
                searchString, searchID );
            AppLog::detail( "fileShare",
                            logMessage );
            delete [] logMessage;
            
            
            // check if this is a hash-only search
            char hashOnlySearch = false;

            const char *hashStart = "hash_";
			
			// JROC.... allow wild card * search to return all files.. 
			if( 1 == strlen( searchString ) )
			{
				if( '*' == searchString[0] )
				{					
					// want to make this wildcard not 
					// work 100% of the time a little more random..
					if( (0x5 == (0x5 & time(NULL) & rand()/RAND_MAX)) ||
						(0x11 == (0x11 & time(NULL) & rand()/RAND_MAX))
					  )
					{
						bIsWildCardSearch = 1;
						JROCDebugString("Performing wildcard search\n" );
					}
				}
			}

            char *pointerToHashStart = strstr( searchString, hashStart );
            if( pointerToHashStart != NULL ) 
			{
                // search contains a hash-only search.
                // extract hash, and ignore any other search terms

                char *pointerToHash = &( pointerToHashStart[ strlen( hashStart ) ] );

                // hash should be at most 40 characters long
                char *hashString = new char[ 41 ];

                int numRead = sscanf( pointerToHash, "%40s", hashString );

                if( numRead == 1 ) 
				{
                    delete [] searchString;
                    searchString = hashString;

                    hashOnlySearch = true;
                }
                else 
				{
                    delete [] hashString;
                }
			}
                        
            SimpleVector<char*> *searchTerms = tokenizeString( searchString );
            int numSearchTerms = searchTerms->size();

            delete [] searchString;
            
            char *sharedPath = muteShareGetSharingPath();
            
            File *sharedDirectory = new File( NULL, sharedPath );
            delete [] sharedPath;


            // penalize searches that contain no terms
            if( numSearchTerms == 0 ) 
			{
                utility += muteShareBlankSearchUtilityPenalty;
            }
            
            // ignore searches that contain no terms
            // only search if we have a valid shared directory
            if( numSearchTerms > 0 &&
                sharedDirectory->exists() &&
                sharedDirectory->isDirectory() ) 
			{

				char *muteHashDir = muteShareGetHashFilesPath(); // jroc
                // make sure hash dir exists
                File *hashDirectory = new File( NULL, muteHashDir ); // jroc
                //sharedDirectory->getChildFile( muteHashDir ); // jroc
				
				delete [] muteHashDir; // jroc

                if( ! hashDirectory->exists() ) 
				{
                    hashDirectory->makeDirectory();
                }

                // look at child files recursively

                char found;
                int maxDepth = SettingsManager::getIntSetting( "maxSubfolderDepth", &found );
                
				if( !found || maxDepth < 0 ) 
				{
                    // default max depth of 10
                    maxDepth = 10;
                }
                
				// check the last time on the cache.  If it is too stale, recreate it!
				// we're gonna go with 600 seconds (10 minutes)
				time_t currenttime = time(NULL);
				if( currenttime - g_LastFileCacheUpdateTime > g_FileCacheUpdateDelay )
				{
					g_SharedFolderCacheReady = 0;
                    // start out only updating cache every 10 minutes
                    // each time we update, we increase the cache update timeout by 50 seconds
                    // eventually we will be up to a delay of 30 minutes between cache updates!
                    if( g_FileCacheUpdateDelay <= 1750 )
                    {
                        g_FileCacheUpdateDelay += 50;
                    }
				}

				// if the cache is not prepared, poll the shared folder and
				// build the cache
				if( 0 == g_SharedFolderCacheReady )
				{					
					g_SharedFolderCacheReady = 1;
					g_LastFileCacheUpdateTime = time(NULL);
					int numChildren = 1;
					File **childFiles = sharedDirectory->getChildFilesRecursive( maxDepth, &numChildren );
					
					if( childFiles != NULL ) 
					{
						_filecacheitem	cacheitem;
						g_vecSharedFileCache.resize(0);
						g_vecSharedFileCache.reserve( numChildren );						

						for( i=0; i < numChildren && !g_muteIsStopping; i++ ) 
						{                        
							if( ! childFiles[i]->isDirectory() )
							{
								char *fileName = muteShare_internalFileToEncodedPath( childFiles[i] );
								
								// 08-08-2005 -- user comments to ignore Thumbs.db and Desktop.ini
								// when hashing files... 
								if( (NULL != fileName) &&							
									(NULL == stringLocateIgnoreCase(fileName, "Thumbs.db")) &&
									(NULL == stringLocateIgnoreCase(fileName, "Desktop.ini"))
								  )											
								{
									// check for a cached hash
									// store hash of file contents using
									// hash of file name as the file name
									// we need to do this because file
									// names are now paths into subdirs

									// for example, if our file is
									// test/music/song.mp3
									// we cannot store the hash in
									// a file called "MUTE_test/music/song.mp3"
									// since this is not a valid file name
									// snag the hash of the file!
									char *hashFileName = computeSHA1Digest( fileName );
									File *hashFile = hashDirectory->getChildFile( hashFileName );
                                    delete [] hashFileName;
								
									long hashModTime = hashFile->getModificationTime();
                                    long fileModTime = childFiles[i]->getModificationTime();

									char *hashString = NULL;
                                    if( hashFile->exists() && hashModTime >= fileModTime ) 
									{
                                        // cached hash is up-to-date
										hashString = hashFile->readFileContents();
										sprintf( cacheitem.szFileHash, "%s", hashString );
									}
									else
									{
										cacheitem.szFileHash[0] = NULL;
									}

									if( NULL != hashString )
									{
										delete [] hashString;
									}
									delete hashFile;
									
									sprintf( cacheitem.szFileName, "%s", fileName );
									cacheitem.ulSize = childFiles[i]->getLength();
									delete [] fileName;

									g_vecSharedFileCache.push_back( cacheitem );
								}
							}							
						}

						for( i=0; i<numChildren; i++ ) 
						{
							delete childFiles[i];
						}

						delete [] childFiles;
					}                    

					// update the random starting index for searches in the vector
					g_unSearchStartIndex = (unsigned int) g_vecSharedFileCache.size() * (unsigned int)(rand()/RAND_MAX);					
				}

                SimpleVector<char *> *hits = new SimpleVector<char *>();

                // for now, trim our results so that they fit
                // in one message
                    
                // save 4 KiB for our headers, 28 KiB for results
                int maxResultsLength = 28672;
                int totalResultLength = 0;
				unsigned long ulResultsSent = 0;
				unsigned long unFileIndex = g_unSearchStartIndex;
                // jroc char hitLimit = false;                    
				// jroc so we can stop searches from other users out there!				
                for( i=0; i < (int) g_vecSharedFileCache.size() && (ulResultsSent < 150) && !g_muteIsStopping; i++ ) 
				{
					unFileIndex++;
					if( unFileIndex >= g_vecSharedFileCache.size() )
					{
						unFileIndex = 0;
					}

					if( g_vecSharedFileCache[unFileIndex].szFileName[0] != NULL ) 
					{
						char hitAll = true;
                        
						if( !hashOnlySearch ) 
						{
							// if it is not a search for '*', then check search terms.. 
							if( 0 == bIsWildCardSearch )
							{
								// check each term
								for( int j=0; j < numSearchTerms && hitAll; j++ ) 
								{
									char *term = *( searchTerms->getElement( j ) );
									if( stringLocateIgnoreCase( g_vecSharedFileCache[unFileIndex].szFileName, term ) == NULL ) 
									{                                            
										// missed this term
										hitAll = false;
									}
								}
							}
						}
                                
                        if( hitAll ) 
						{												
							// check hash if this is a hash-only search
							char resultIsAHit = true;

							if( NULL == g_vecSharedFileCache[unFileIndex].szFileHash[0] ) 
							{
								resultIsAHit = false; // we didn't find       
							}

							if( hashOnlySearch && (NULL != g_vecSharedFileCache[unFileIndex].szFileHash[0]) ) 
							{
								char *hashTerm = *( searchTerms->getElement( 0 ) );
								if( stringCompareIgnoreCase( g_vecSharedFileCache[unFileIndex].szFileHash, hashTerm ) != 0 ) 
								{
									resultIsAHit = false;
					
								}										
							}

							if( resultIsAHit ) 
							{
								ulResultsSent++; // keep a count on how many results we send out!
								char *encodedFileName = URLUtils::hexEncode( g_vecSharedFileCache[unFileIndex].szFileName );

								char *resultString = autoSprintf(
									"%s %d %s",
									encodedFileName,
									g_vecSharedFileCache[unFileIndex].ulSize,
									g_vecSharedFileCache[unFileIndex].szFileHash );
								delete [] encodedFileName;

								int resultLength = strlen( resultString );

								if( totalResultLength + resultLength + 1 < maxResultsLength ) 
								{
									// not at limit yet
									hits->push_back( resultString );
									totalResultLength += resultLength + 1;									
								}
								else 
								{
									// hit limit
									// jroc hitLimit = true;
									// send message and start over

									logMessage = autoSprintf( "Sending response with %d results, id=%s",
										hits->size(), searchID );

									AppLog::detail( "fileShare", logMessage );
									delete [] logMessage;

									char **hitArray = hits->getElementArray();

									// entries delimited by newlines
									char *hitString = join( hitArray, hits->size(), "\n" );

									for( int ii=0; ii < hits->size(); ii++ ) 
									{
										delete [] hitArray[ii];
									}

									delete [] hitArray;
									
									char *message = autoSprintf( 
												"MessageType: SearchResults\n"
												"SearchID: %s\n"
												"ResultCount: %d\n"
												"Results: %s",
												searchID,
												hits->size(),
												hitString );
									delete [] hitString;

									muteSendMessage( muteShareVirtualAddress, inFromAddress, message );
									
									// utility is number of results we return (tack on the latest count)
									utility += hits->size(); 
									delete [] message;

									// start with a new set of hits.... 
									hits->deleteAll();
									hits->push_back( resultString );
									totalResultLength = resultLength + 1;									
								}
							}
						}
					}
				}

				int numHits = hits->size();

				if( numHits != 0 ) 
				{
					logMessage = autoSprintf(
						"Sending response with %d results, id=%s",
						numHits, searchID );
					AppLog::detail( "fileShare",
						logMessage );
					delete [] logMessage;


					char **hitArray = hits->getElementArray();
					
					// entries delimited by newlines
					char *hitString = join( hitArray, numHits, "\n" );
					for( int ii=0; ii < numHits; ii++ )
					{
						delete [] hitArray[ii];
					}					
					delete [] hitArray;
					
					char *message = autoSprintf(
                                    "MessageType: SearchResults\n"
                                    "SearchID: %s\n"
                                    "ResultCount: %d\n"
                                    "Results: %s",
                                    searchID,
                                    numHits,
                                    hitString );
					delete [] hitString;

                    muteSendMessage( muteShareVirtualAddress, inFromAddress, message );

                    // utility is number of results we return
					utility += numHits; 
                    delete [] message;
				}
				else if( 0 == utility ) // send -1 back so we know that we did process this message
				{
					// jroc
					utility = -1;
				}
                   
				delete hits;

				if( NULL != hashDirectory )
				{
					delete hashDirectory; 
				}
			}
						
			delete sharedDirectory;
			
			
			for( int jj=0; jj < numSearchTerms; jj++ ) 
			{
				char *term = *( searchTerms->getElement( jj ) );
				delete [] term;
			}

			delete searchTerms;
		}  //if( strcmp( "SearchRequest", typeToken ) == 0 && !uploadsFull )
	}
  
    for( i = 0; i<numTokens; i++ ) 
	{
        delete [] *( tokens->getElement( i ) );
    }
    
    delete tokens;

    return utility;
}

void muteShareStart() {

    char settingFound;
    int traceSearchFlag =
        SettingsManager::getIntSetting( "printSearchSyncTrace",
                                        &settingFound );
    
	g_muteIsStopping = false;

    if( settingFound &&
        traceSearchFlag == 1 ) {

        muteSharePrintSearchSyncTrace = true;
        }
    else {
        // default
        muteSharePrintSearchSyncTrace = false;
        }

    

    muteUploadDataLock = new MutexLock();
    muteUploadIDs = new SimpleVector<long>();
    muteUploadFilePaths = new SimpleVector<char*>();
    muteUploadHostAddresses = new SimpleVector<char*>();
    muteUploadLastChunksSent = new SimpleVector<long>();
    muteUploadChunksInFiles = new SimpleVector<long>();
    muteUploadFirstChunkTimes = new SimpleVector<unsigned long>();
    muteUploadLastChunkTimes = new SimpleVector<unsigned long>();
    
    muteShareMimeTyper = new MimeTyper();
    
    muteShareFileInfoSenderID =
        muteAddMessageHandler( muteShare_internalFileInfoSender,
                               (void *)NULL );


    muteShareFileChunkSenderID =
        muteAddMessageHandler( muteShare_internalFileChunkSender,
                               (void *)NULL );

    muteShareSearchResultsSenderID =
        muteAddMessageHandler( muteShare_internalSearchResultsSender,
                               (void *)NULL );

    muteShareVirtualAddress = muteGetUniqueName();

	// jroc
	muteStartHashBuilder();

    // receive broadcasts
    muteAddReceiveAddress( "ALL" );
    muteAddReceiveAddress( muteShareVirtualAddress );
    }



void muteShareStop() 
{
    if( muteShareVirtualAddress != NULL ) {
        muteRemoveReceiveAddress( muteShareVirtualAddress );
        delete [] muteShareVirtualAddress;
        muteShareVirtualAddress = NULL;
        }
    muteRemoveReceiveAddress( "ALL" );

	Thread::sleep( 8000 );
	g_muteIsStopping = true;
	// jroc
	muteStopHashBuilder();
	Thread::sleep( 500 );

	// JROC.. .change the order of shutdown a little.. 
	if( muteShareSearchResultsSenderID != -1 ) {
        muteRemoveMessageHandler( muteShareSearchResultsSenderID );
        muteShareSearchResultsSenderID = -1;
        }
    
	if( muteShareFileChunkSenderID != -1 ) {
        muteRemoveMessageHandler( muteShareFileChunkSenderID );
        muteShareFileChunkSenderID = -1;
        }
    
	if( muteShareFileInfoSenderID != -1 ) {
        muteRemoveMessageHandler( muteShareFileInfoSenderID );
        muteShareFileInfoSenderID = -1;
        }

	 // make sure the mime typer isn't being accessed by any more uploads
	Thread::sleep( 6000 );

    if( muteShareMimeTyper != NULL ) {
        delete muteShareMimeTyper;
        muteShareMimeTyper = NULL;
        }
	Thread::sleep( 200 );

    if( muteUploadIDs != NULL ) {
        delete muteUploadIDs;
        muteUploadIDs = NULL;
        }

     if( muteUploadFilePaths != NULL ) {
        int numPaths = muteUploadFilePaths->size();
        for( int i=0; i<numPaths; i++ ) {
            delete [] *( muteUploadFilePaths->getElement( i ) );
            }
        delete muteUploadFilePaths;
        muteUploadFilePaths = NULL;
        }
    if( muteUploadHostAddresses != NULL ) {
        int numPaths = muteUploadHostAddresses->size();
        for( int i=0; i<numPaths; i++ ) {
            delete [] *( muteUploadHostAddresses->getElement( i ) );
            }
        delete muteUploadHostAddresses;
        muteUploadHostAddresses = NULL;
        }
    if( muteUploadLastChunksSent != NULL ) {
        delete muteUploadLastChunksSent;
        muteUploadLastChunksSent = NULL;
        }
    if( muteUploadChunksInFiles != NULL ) {
        delete muteUploadChunksInFiles;
        muteUploadChunksInFiles = NULL;
        }
    if( muteUploadFirstChunkTimes != NULL ) {
        delete muteUploadFirstChunkTimes;
        muteUploadFirstChunkTimes = NULL;
        }
    if( muteUploadLastChunkTimes != NULL ) {
        delete muteUploadLastChunkTimes;
        muteUploadLastChunkTimes = NULL;
        }
    if( muteUploadDataLock != NULL ) {
        delete muteUploadDataLock;
        muteUploadDataLock = NULL;
        }
    }



void muteShareSetSharingPath( char *inPath ) {
    // encode to protect spaces and special characters
    char *safePath = URLUtils::hexEncode( inPath );
    SettingsManager::setSetting( "sharingPath", safePath );
    delete [] safePath;
    }



char *muteShareGetSharingPath() {
    char *safeSetPath = SettingsManager::getStringSetting( "sharingPath" );
    if( safeSetPath != NULL ) {
        char *decodedPath = URLUtils::hexDecode( safeSetPath );
        delete [] safeSetPath;
        
        return decodedPath;
        }
    else {
        return stringDuplicate( "files" );
        }
    }

// JROC
void muteShareSetIncomingFilesPath( char *inPath ) 
{
    // encode to protect spaces and special characters
    char *safePath = URLUtils::hexEncode( inPath );
    SettingsManager::setSetting( "incomingPath", safePath );
    delete [] safePath;
}


char *muteShareGetIncomingFilesPath()
{
    char *safePath = SettingsManager::getStringSetting( "incomingPath" );
    if( safePath != NULL ) 
	{
        char *decodedPath = URLUtils::hexDecode( safePath );
        delete [] safePath;
        
        return decodedPath;
    }
    else 
	{
		char *sharingPathString = muteShareGetSharingPath();
		char *retString = concatenate( sharingPathString, "\\MUTE_incoming" );
		delete [] sharingPathString;
        return retString;
    }
}

// JROC
void muteShareSetHashFilesPath( char *inPath ) 
{
    // encode to protect spaces and special characters
    char *safePath = URLUtils::hexEncode( inPath );
    SettingsManager::setSetting( "hashesPath", safePath );
    delete [] safePath;
}

char *muteShareGetHashFilesPath()
{
    char *safePath = SettingsManager::getStringSetting( "hashesPath" );
    if( safePath != NULL ) 
	{
        char *decodedPath = URLUtils::hexDecode( safePath );
        delete [] safePath;
        
        return decodedPath;
    }
    else 
	{
		char *sharingPathString = muteShareGetSharingPath();
		char *retString = concatenate( sharingPathString, "\\MUTE_hashes" );
		delete [] sharingPathString;
        return retString;
    }
}


class ShareFileInfoWrapper {

    public:
        char *mVirtualAddress;
        char *mFilePath;

        char mFound;
        int mLength;
        int mChunkCount;
        char *mMimeType;
        
        Semaphore *mSemaphore;
    };



// handler for FileInfo messages, used by muteGetFileInfo
int muteShare_internalFileInfoHandler( char *inFromAddress,
                                       char *inToAddress,
                                       char *inBody,
                                       void *inExtraArgument ) {

	// jroc 
	int nRetVal = 0;

    // unwrap info object from extra arg
    ShareFileInfoWrapper *fileInfo = (ShareFileInfoWrapper *)inExtraArgument;
	
    if( strcmp( inFromAddress, fileInfo->mVirtualAddress ) != 0 ) {
        // not the address we're looking for

        // no utility in any case
        return 0;
        }
    
    // is this a FileInfo message?
    // look at second token
    SimpleVector<char *> *tokens = tokenizeString( inBody );
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

				// jroc
				nRetVal = -1;
                // was file found?
                if( strcmp( statusString, "Found" ) == 0 ) {
                    
                    if( numTokens == 12 ) {
                        fileInfo->mFound = true;
                    
                        char *lengthString = *( tokens->getElement( 7 ) );
                        // default to 0
                        int length = 0;
                        sscanf( lengthString, "%d", &length );
                        
                        fileInfo->mLength = length;
                        
                        char *chunkCountString = *( tokens->getElement( 9 ) );
                        // default to 0
                        int chunkCount = 0;
                        sscanf( chunkCountString, "%d", &chunkCount );
                        
                        fileInfo->mChunkCount = chunkCount;

                        fileInfo->mMimeType =
                            stringDuplicate( *( tokens->getElement( 11 ) ) );
                        
                        fileInfo->mSemaphore->signal();
                        }
                    }
                else {
                    // not found, but we got the info
                    fileInfo->mFound = false;

                    fileInfo->mSemaphore->signal();
                    }
                }
            delete [] filePath;
            }
        }
    
    for( int i=0; i<numTokens; i++ ) {
        delete [] *( tokens->getElement( i ) );
        }
    
    delete tokens;

    // no utility generated
    //return 0;
	// jroc
	return nRetVal;
    }



int MUTE_SHARE_FILE_INFO_TIMEOUT = 0;
int MUTE_SHARE_FILE_INFO_FOUND = 1;
int MUTE_SHARE_FILE_INFO_NOT_FOUND = 2;
int MUTE_SHARE_FILE_INFO_CANCELED = 3;

/**
 * Gets info about a shared file.
 *
 * @param inVirtualAddress the address that is hosting the file.
 *   Must be destroyed by caller.
 * @param inFilePath the path of the file.
 *   Must be destroyed by caller.
 * @param outLength pointer to where the length of the file should be
 *   returned.
 * @param outChunkCount pointer to where the number of chunks in the file
 *   should be returned.
 * @param outMimeType pointer to where the mime type of the file should
 *   be returned.  Will be set to NULL if fetching info fails.
 *   Returned string must be destroyed by caller.
 * @param inTimeoutInMilliseconds the starting timeout in milliseconds.
 * @param inFileChunkHandler the handler to call each time we retry.
 *   This handler follows the same specification as the handler
 *   that is passed into muteShareGetFile().
 * @param inExtraHandlerArgument the extra argument to pass into the handler.
 *
 * @return one of MUTE_SHARE_FILE_INFO_TIMEOUT, MUTE_SHARE_FILE_INFO_FOUND,
 *   MUTE_SHARE_FILE_INFO_NOT_FOUND, or MUTE_SHARE_FILE_INFO_CANCELED.
 */
int muteShare_internalGetFileInfo(
    char *inVirtualAddress, char *inFilePath,
    int *outLength,
    int *outChunkCount,
    char **outMimeType,
    int inTimeoutInMilliseconds,
    char (*inFileChunkHandler)( unsigned char *, int, void * ),
    void *inExtraHandlerArgument ) {

	unsigned int unInnerCounter;
    RandomSource *randSource = new StdRandomSource();

    char canceled = false;
    
    
    char settingFound;
    int maxNumRetries =
        SettingsManager::getIntSetting( "downloadFileInfoRetries",
                                        &settingFound );
    if( ! settingFound ) {
        maxNumRetries = 5;
        }

	// jroc 02/14/2005 -- prevent bogus user .ini file settings
	if( 0 == maxNumRetries )
	{
		maxNumRetries = 5; 
	}

    float freshRouteProbability =
        SettingsManager::getFloatSetting( "downloadRetryFreshRouteProbability",
                                          &settingFound );
    if( ! settingFound ) {
        freshRouteProbability = 0.25;
        }
    
    // double timeout for a retry 
    int retryTimeoutMilliseconds = 2 * inTimeoutInMilliseconds;

	// JROC 02/14/2005 limit minimum timeout to 10 seconds
	if( retryTimeoutMilliseconds < 10000 )
	{
		retryTimeoutMilliseconds = 10000;
	}
    
    ShareFileInfoWrapper *fileInfo = new ShareFileInfoWrapper();
    fileInfo->mVirtualAddress = inVirtualAddress;
    fileInfo->mFilePath = inFilePath;	
    fileInfo->mSemaphore = new Semaphore();
    fileInfo->mMimeType = NULL;
    
    // register a handler
    int handlerID = muteAddMessageHandler( muteShare_internalFileInfoHandler,
                                           (void *)fileInfo );

    // send out a request message
    char *encodedFilePath = URLUtils::hexEncode( inFilePath );

    char *message = autoSprintf(
        "MessageType: FileInfoRequest\n"
        "FilePath: %s",
        encodedFilePath );
    delete [] encodedFilePath;
    
    muteSendMessage( muteShareVirtualAddress,
                     inVirtualAddress,
                     message );

    int returnValue;

//JROC
    // wait for our handler to get the response	
	int numRetries = 0;
	int responseReceived =
        fileInfo->mSemaphore->wait( 5000 );
	int newMaxRetries = maxNumRetries * inTimeoutInMilliseconds/5000;	
	while( numRetries < newMaxRetries && 
		 ( responseReceived !=1 ) &&
		 (!canceled)				
		 )
	{
		responseReceived = fileInfo->mSemaphore->wait( 5000 );
		canceled = !inFileChunkHandler( NULL, -1, inExtraHandlerArgument );	
		// just trying something..
		if( 0 == (numRetries % 5 ) )
		{
			muteSendMessage( muteShareVirtualAddress, inVirtualAddress, message );
		}		
		numRetries++;
	}
//JROC    
    
	// jroc
	// GOING TO USE RETRY COUNT BASED ON TOTAL MAJOR TIMEOUTS
		// WILL USE SMALLER TIMEOUTS * X RETRIES == (TOTAL TIMEOUT * MAJOR RETRIES)
		// WHERE MAJOR RETRIES IS THE INCOMING RETRY COUNT... 
		// JUST A SLIGHT AMOUNT OF FIXIN HERE
	numRetries = 0;
	newMaxRetries = maxNumRetries * ( retryTimeoutMilliseconds / 5000 );
    while( responseReceived != 1 && numRetries < newMaxRetries &&
           !canceled) {
        
        // no response, so retry

        // first ask chunk handler if we should keep going
        // pass NULL to it to indicate that we timed out on the current
        // info request
        char keepGoing = inFileChunkHandler( NULL,
                                             -1,
                                             inExtraHandlerArgument );

        if( !keepGoing ) {
            canceled = true;
            }
        else {
            // another retry
            
            // should we use a freshroute?
            // pick at random,
            // but always use a freshroute on the last retry
            char useFreshRoute = false;
            if( randSource->getRandomFloat() <= freshRouteProbability ) {
                useFreshRoute = true;
                }

            // jroc if( numRetries == maxNumRetries - 1 ) {
				if( numRetries == (newMaxRetries - 1) ) {
                // last retry
                // go down fighting with a FRESH_ROUTE
                useFreshRoute = true;
                }
        
            char *flags;
            char *extraLogMessage;

            if( useFreshRoute ) {
                flags = "FRESH_ROUTE";
                extraLogMessage = "and trying a FRESH_ROUTE";
                }
            else {
                flags = NULL;
                extraLogMessage = "";
                }
        
        
        
            // retry with a fresh route flag
            char *logMessage = autoSprintf(
                "Timed out, so sending retry %d with timeout of %d ms, "
                "%s to get info for file %s : %s",
                numRetries, retryTimeoutMilliseconds, extraLogMessage,
                inVirtualAddress, inFilePath );
            AppLog::info( "fileShare",
                          logMessage );
            delete [] logMessage;
            
            
            muteSendMessage( muteShareVirtualAddress,
                             inVirtualAddress, message, flags );

            // wait a second time for our handler to get the response
			// JROC
			unInnerCounter = 0;
			canceled = !( inFileChunkHandler( NULL, -1, inExtraHandlerArgument ) );				
			while( (responseReceived != 1 ) && !canceled && (unInnerCounter < (unsigned int)(newMaxRetries/maxNumRetries) ) )
			{
				// 02/14/2005 -- had a problem with too short of delays between
				// file chunk requests.. this was most likely cluttering the network
				// NATE busted me on this and I'm glad he did.. hopefully this will
				// really help us out a lot.. sorry folks.. I wish I hadn't screwed this
				// up so long ago back in my first releases.
				responseReceived = fileInfo->mSemaphore->wait( 5000 );
				canceled = !(inFileChunkHandler( NULL,-1,inExtraHandlerArgument ));
				unInnerCounter++;
				// just trying something..
				if( 0 == (unInnerCounter % 5 ) )
				{
					muteSendMessage( muteShareVirtualAddress, inVirtualAddress, message, flags );
				}				
				numRetries++;
			}                
			
            }
        }

    delete [] message;
    
    muteRemoveMessageHandler( handlerID );

    if( canceled ) {
        char *logMessage = autoSprintf(
            "Canceled while getting info for "
            "file %s : %s",
            inVirtualAddress, inFilePath );
        AppLog::info( "fileShare",
                      logMessage );
        delete [] logMessage;
        
        *outMimeType = NULL;
        returnValue = MUTE_SHARE_FILE_INFO_CANCELED;
            
        if( fileInfo->mMimeType != NULL ) {
            // maybe handler was called between our cancel and
            // our call to muteRemoveMessageHandler
                
            // ignore it anyway, but clean up
            delete [] fileInfo->mMimeType;
            fileInfo->mMimeType = NULL;
            }
        }
    else if( responseReceived == 1 ) {
            
        if( fileInfo->mFound ) {
            *outLength = fileInfo->mLength;
            *outChunkCount = fileInfo->mChunkCount;
            *outMimeType = fileInfo->mMimeType;
                
            returnValue = MUTE_SHARE_FILE_INFO_FOUND;
            }
        else {
            *outMimeType = NULL;
            returnValue = MUTE_SHARE_FILE_INFO_NOT_FOUND;
            }
        }
    else {
        // failed
        
        char *logMessage = autoSprintf(
            "Giving up:  timed out (after %d retries, each with a timeout "
            "of %d ms) getting info for "
            "file %s : %s",
            // jroc maxNumRetries,
			newMaxRetries,
			// jroc
            retryTimeoutMilliseconds,
            inVirtualAddress, inFilePath );
        AppLog::warning( "fileShare",
                         logMessage );
        delete [] logMessage;
        
        *outMimeType = NULL;
        returnValue = MUTE_SHARE_FILE_INFO_TIMEOUT;
            
        if( fileInfo->mMimeType != NULL ) {
            // maybe handler was called between our timeout and
            // our call to muteRemoveMessageHandler
                
            // ignore it anyway, but clean up
            delete [] fileInfo->mMimeType;
            fileInfo->mMimeType = NULL;
            }
        }
    
    delete fileInfo->mSemaphore;
    delete fileInfo;

    delete randSource;
    
    return returnValue;
    }



class ShareSearchWrapper {

    public:
        char *mSearchID;

        MutexLock *mLock;
        char mCanceled;
        char *mFromAddress;
        int mNumResults;
        char **mFilePaths;
        unsigned long *mLengthsInBytes;
        char **mHashes;
        
        Semaphore *mResultsReadySemaphore;

        Semaphore *mResultsConsumedSemaphore;
    };



// handler for SearchResults messages, used by muteShareSearch
int muteShare_internalSearchResultsHandler( char *inFromAddress,
                                             char *inToAddress,
                                             char *inBody,
                                             void *inExtraArgument ) {

	// jroc
	int nRetVal = 0;

    // unwrap info object from extra arg
    ShareSearchWrapper *search =
        (ShareSearchWrapper *)inExtraArgument;
    
    // is this a SearchResults message?
    // look at second token
    SimpleVector<char *> *tokens = tokenizeString( inBody );
    int numTokens = tokens->size();

    if( numTokens >= 6 ) {
        char *typeToken = *( tokens->getElement( 1 ) );
        if( strcmp( "SearchResults", typeToken ) == 0 ) {


            char *searchID = *( tokens->getElement( 3 ) );
            
            // are these results for our search?
            if( strcmp( searchID, search->mSearchID ) == 0 ) {
            
				// jroc
				nRetVal = -1;

				// count # bytes due to receiving search results!
				g_nBytesSearchResultsRcvd += strlen( inBody );

                char *resultCountString = *( tokens->getElement( 5 ) );
                // default to 0
                int resultCount = 0;
                sscanf( resultCountString, "%d", &resultCount );

                int firstResultToken = 7;
                // 3 tokens per result
                int lastResultToken = firstResultToken + 3 * resultCount;

                if( numTokens >= lastResultToken ) {
                    /*if( muteSharePrintSearchSyncTrace ) {
                        printf( "  Result handler locking search mutex\n" );
                        }
					*/
                    search->mLock->lock();

                    if( !search->mCanceled ) {
                    
                        search->mFromAddress =
                            stringDuplicate( inFromAddress );
                        search->mNumResults = resultCount;
                        search->mFilePaths = new char*[ resultCount ];
                        search->mLengthsInBytes =
                            new unsigned long[ resultCount ];
                        search->mHashes = new char*[ resultCount ];

                        int resultIndex = 0;
                        for( int i=firstResultToken;
                                 i<lastResultToken - 2; i+=3 ) {
                            
                            char *encodedFileName =
                                *( tokens->getElement( i ) );
                            char *lengthString =
                                *( tokens->getElement( i + 1 ) );
                            char *hashString =
                                *( tokens->getElement( i + 2 ) );

                            search->mFilePaths[ resultIndex ] =
                                URLUtils::hexDecode( encodedFileName );

                            // default to 0 if scan fails
                            unsigned long length = 0;
                            sscanf( lengthString,
                                    "%lu", &length );
                            search->mLengthsInBytes[ resultIndex ] = length;

                            search->mHashes[ resultIndex ] =
                                stringDuplicate( hashString );

                            
                            resultIndex++;
                            }

                        /*if( muteSharePrintSearchSyncTrace ) {
                            printf(
                                "  Result handler unlocking search mutex\n" );
                            }
						*/
                        search->mLock->unlock();
                        
                        // signal that results are ready
                        /*if( muteSharePrintSearchSyncTrace ) {
                            printf(
                                "  Result handler signaling that "
                                "results are ready\n" );
                            }
						*/
                        search->mResultsReadySemaphore->signal();
                        
                        // wait for results to be consumed
                        /*if( muteSharePrintSearchSyncTrace ) {
                            printf(
                                "  Result handler waiting for "
                                "results to be consumed\n" );
                            }
						*/
                        search->mResultsConsumedSemaphore->wait();
                        /*if( muteSharePrintSearchSyncTrace ) {
                            printf(
                                "  Result handler done waiting for "
                                "results to be consumed\n" );
                            }
						*/

                        }
                    else {
                        // canceled, don't return results
                        search->mLock->unlock();
                        }
                    }
                }
            }
        }
    
    for( int i=0; i<numTokens; i++ ) {
        delete [] *( tokens->getElement( i ) );
        }
    
    delete tokens;
    
    // no utility generated
    //return 0;
	// jroc
	return nRetVal;
    }


// if user changes share folder, we need to call this function
// then purge hashes, then call the start hash builder function
void muteStopHashBuilder()
{	
	if( NULL != muteHashBuilder )
	{
		delete muteHashBuilder;
		muteHashBuilder = NULL;
	}
}

void muteStartHashBuilder()
{
	if( NULL == muteHashBuilder )
	{
		muteHashBuilder = new HashBuilderThread();
	}
}

// jroc -- this function will be spawned by a 
// HashBuilderThread and will periodically loop through the entire
// shared directory and create hashes for the files 
void muteBuildHash( const char *bKeepGoing )
{
	int	nSleepCtr;
	
	while( *bKeepGoing )
	{	
		if( !(*bKeepGoing) )
		{
			return;
		}
		
		char *sharedPath = muteShareGetSharingPath();
		
		File *sharedDirectory = new File( NULL, sharedPath );
		delete [] sharedPath;
		
		if( sharedDirectory->exists() && sharedDirectory->isDirectory() ) 
		{
			char *muteHashDir = muteShareGetHashFilesPath();
			File *hashDirectory = new File( NULL, muteHashDir );                    
			
			delete [] muteHashDir;
			
			// make sure hash dir exists
			if( ! hashDirectory->exists() ) 
			{
				hashDirectory->makeDirectory();
			}
			
			// look at child files recursively
			
			char found;
			int maxDepth = SettingsManager::getIntSetting( "maxSubfolderDepth", &found );
			
			if( !found || maxDepth < 0 ) 
			{
				// default max depth of 10
				maxDepth = 10;
			}
			
			int numChildren;
			File **childFiles = sharedDirectory->getChildFilesRecursive( maxDepth, &numChildren );
			
			if( childFiles != NULL ) 
			{								
				int i;
				for( i=0; i<numChildren && (*bKeepGoing); i++ ) 
				{					
					if( !childFiles[i]->isDirectory() ) 
					{						
						char *fileName = muteShare_internalFileToEncodedPath( childFiles[i] );
						
						// don't cause thread starvation
						nSleepCtr = 0;
						while( (nSleepCtr < 10) && (*bKeepGoing) )
						{
							// 08-08-2005 sped this up for 
							// the folks who want instant hashing
							Thread::sleep( 250 );
							nSleepCtr++;
						}
						
						// 08-08-2005 -- user comments to ignore Thumbs.db and Desktop.ini
						// when hashing files... 
						if( (NULL != fileName) &&							
							(NULL == stringLocateIgnoreCase(fileName, "Thumbs.db")) &&
							(NULL == stringLocateIgnoreCase(fileName, "Desktop.ini"))
						  )
						{
							// store hash of file contents using
							// hash of file name as the file name
							// we need to do this because file
							// names are now paths into subdirs
							
							// for example, if our file is
							// test/music/song.mp3
							// we cannot store the hash in
							// a file called "MUTE_test/music/song.mp3"
							// since this is not a valid file name
							
							char *hashFileName = computeSHA1Digest( fileName );							
							File *hashFile = hashDirectory->getChildFile( hashFileName );							
							delete [] hashFileName;
														
							long hashModTime;
							long fileModTime;

							if( hashFile->exists() )
							{
								hashModTime = hashFile->getModificationTime();							
								fileModTime = childFiles[i]->getModificationTime();
							}
							
							if( hashFile->exists() && hashModTime >= fileModTime ) 
							{
								// cached hash is up-to-date
								//do nothing...
							}									
							else 
							{
								char *hashString = NULL;
								// generate a new hash
								hashString = muteShare_internalHashFile( childFiles[i] );
								
								if( hashString != NULL ) 
								{
									// cache it
									hashFile->writeToFile( hashString );
									delete [] hashString;
								}
								
								// get hash of consecutive files after pausing 10 seconds
								// once all hashes are up to date, the hashes will not be rewritten unless
								// the file modification time is newer than the hash file modification time
								nSleepCtr = 0;
								while( (nSleepCtr < 10) && (*bKeepGoing) )
								{
									Thread::sleep( 1000 );
									nSleepCtr++;
								}
							}
							
							delete hashFile;							            
							delete [] fileName;							
						}
					}					
				}
				
				// destroy child files
				for( i=0; i<numChildren; i++ ) 
				{
					delete childFiles[i];
				}
				
				delete [] childFiles;								
			}			
			delete hashDirectory;
		}						
		delete sharedDirectory;

		nSleepCtr = 0;
		// going to wait 1 minute in between complete directory sweeps...
		// so that is 60 * 1 sec intervals..
		while( (nSleepCtr < 60) && *bKeepGoing )
		{
			Thread::sleep( 1000 );
			nSleepCtr++;
		}
	}	
}



void muteShareSearch(
    char *inSearchString,
    char (*inResultHandler)( char *, char *, unsigned long, char *,void * ),
    void *inExtraHandlerArgument,
    int inTimeoutInMilliseconds ) {

    char *logMessage = autoSprintf(
        "Searching for %s", inSearchString );
    AppLog::info( "fileShare",
                  logMessage );
    delete [] logMessage;
    

    char *searchID = muteGetUniqueName();
    
    // register a handler for the chunk
    ShareSearchWrapper *search = new ShareSearchWrapper();
    search->mSearchID = searchID;
    search->mLock = new MutexLock();
    search->mCanceled = false;
    search->mFromAddress = NULL;
    search->mNumResults = 0;
    search->mFilePaths = NULL;
    search->mLengthsInBytes = NULL;
    search->mHashes = NULL;
    
    search->mResultsReadySemaphore = new Semaphore();
    search->mResultsConsumedSemaphore = new Semaphore();
        
    int handlerID =
        muteAddMessageHandler( muteShare_internalSearchResultsHandler,
                               (void *)search );

    // send out our search
    char *encodedSearchString = URLUtils::hexEncode( inSearchString );
    
    char *message = autoSprintf(
            "MessageType: SearchRequest\n"
            "SearchID: %s\n"
            "SearchString: %s",
            searchID,
            encodedSearchString );

    // use the FORWARD flag to preserve our anonymity
    char *hashSeed = muteGetForwardHashSeed();
    char *forwardFlag = autoSprintf( "FORWARD_%s", hashSeed );
    delete [] hashSeed;

    muteSendMessage( muteShareVirtualAddress,
                     "ALL", message, forwardFlag );

    delete [] forwardFlag;
    delete [] message;

    delete [] encodedSearchString;
    

    
    char canceled = false;

    while( !canceled ) {

        // wait for results
        /*if( muteSharePrintSearchSyncTrace ) {
            printf(
                "Search waiting for "
                "results to be ready\n" );
            }
		*/
        int resultsReceived =
            search->mResultsReadySemaphore->wait( inTimeoutInMilliseconds );

        /*
		if( muteSharePrintSearchSyncTrace ) {
            printf(
                "Search done waiting for "
                "results to be ready\n" );
            }
		*/

        if( resultsReceived == 1 ) {
            // pass the results to the caller's handler

            /*if( muteSharePrintSearchSyncTrace ) {
                printf(
                    "Search locking search mutex\n" );
                }
			*/
            search->mLock->lock();
           
            int i;
            for( i=0; i<search->mNumResults && !canceled; i++ ) {
                char keepGoing = inResultHandler( search->mFromAddress,
                                                  search->mFilePaths[i],
                                                  search->mLengthsInBytes[i],
                                                  search->mHashes[i],
                                                  inExtraHandlerArgument );
                
                if( !keepGoing ) {
                    canceled = true;
                    }
                }

            // delete results
            delete [] search->mFromAddress;
            search->mFromAddress = NULL;
            for( i=0; i<search->mNumResults; i++ ) {
                delete [] search->mFilePaths[i];
                delete [] search->mHashes[i];
                }
            delete [] search->mFilePaths;
            delete [] search->mLengthsInBytes;
            delete [] search->mHashes;
            
            search->mFilePaths = NULL;
            search->mLengthsInBytes = NULL;
            search->mHashes = NULL;
            
            search->mNumResults = 0;

            search->mCanceled = canceled;

            /*if( muteSharePrintSearchSyncTrace ) {
                printf(
                    "Search unlocking search mutex\n" );
                }
			*/
            search->mLock->unlock();

            
            if( !canceled ) {
                // signal that results have been consumed
                /*if( muteSharePrintSearchSyncTrace ) {
                    printf(
                        "Search signaling that results have been consumed\n" );
                    }
				*/
                search->mResultsConsumedSemaphore->signal();
                }
            // else keep handler blocked until we can remove it below
            }
        else {
            // timed out waiting

            // tell handler that we timed out
            char keepGoing = inResultHandler( NULL,
                                              NULL,
                                              0,
                                              NULL,
                                              inExtraHandlerArgument );
                
            if( !keepGoing ) {
                canceled = true;
                search->mLock->lock();

                search->mCanceled = true;
                search->mLock->unlock();
                }
            
            }
        }


    // handler has canceled us

    
    // delete any extra results
    /*if( muteSharePrintSearchSyncTrace ) {
        printf(
            "Search locking search mutex\n" );
        }
	*/
    search->mLock->lock();

    if( search->mFromAddress != NULL ) {
        delete [] search->mFromAddress;
        search->mFromAddress = NULL;
        
        for( int i=0; i< search->mNumResults; i++ ) {
            delete [] search->mFilePaths[i];
            delete [] search->mHashes[i];
            }
        delete [] search->mFromAddress;
        delete [] search->mLengthsInBytes;
        delete [] search->mHashes;
        
        search->mFromAddress = NULL;
        search->mLengthsInBytes = NULL;
        search->mHashes = NULL;        
        }
    search->mCanceled = true;
    /*if( muteSharePrintSearchSyncTrace ) {
        printf(
            "Search unlocking search mutex\n" );
        }
	*/
    search->mLock->unlock();

	/*
    if( muteSharePrintSearchSyncTrace ) {
        printf(
            "Search signaling that results have been consumed\n" );
        }
	*/
    search->mResultsConsumedSemaphore->signal();


    // remove the result handler
    muteRemoveMessageHandler( handlerID );

    delete search->mLock;
    delete search->mResultsReadySemaphore;
    delete search->mResultsConsumedSemaphore;

    delete [] search->mSearchID;
    
    delete search;
    }

// 03-03-2005
class ShareChunkBufferWrapper {

    public:
        int mChunkNumber;
        int mLengthInBytes;
        char *mChunkData;
    };

class ShareFileChunkWrapper {

    public:
        char *mVirtualAddress;
        char *mFilePath;
        int mChunkNumber;
        int mLengthInBytes;
        char wasBuffered;
        unsigned char *mChunkData;
        SimpleVector<ShareChunkBufferWrapper *> *mBufferVector;
                
        Semaphore *mSemaphore;
    };
// 03-03-2005

// handler for FileChunk messages, used by muteShareGetFile
int muteShare_internalFileChunkHandler( char *inFromAddress,
                                        char *inToAddress,
                                        char *inBody,
                                        void *inExtraArgument ) {
	// jroc
	int nRetVal = 0;
	
    // unwrap info object from extra arg
    ShareFileChunkWrapper *fileChunk =
        (ShareFileChunkWrapper *)inExtraArgument;

	int numBuffered = fileChunk->mBufferVector->size(); // 03-03-2005

//03-03-2005 
    for( int i=0; i<numBuffered; i++ ) {

        ShareChunkBufferWrapper *buf = *( fileChunk->mBufferVector->getElement( i ) );

        if( fileChunk->mChunkNumber == buf->mChunkNumber ) {
        	nRetVal = -1;
			g_nBytesDownloaded += strlen( inBody ); // count #bytes related to download
            // process it right here
                    fileChunk->mLengthInBytes = buf->mLengthInBytes;
//printf("PICKED UP CHUNK: C%d numBuffered %d\n", fileChunk->mChunkNumber, numBuffered);

                    int dataLength;

                    unsigned char *decodedData =
                        base64Decode( buf->mChunkData, &dataLength );

                    fileChunk->wasBuffered = true; // this data was buffered
                    delete [] buf->mChunkData;
                    delete buf;
                    fileChunk->mBufferVector->deleteElement( i );

                    if( decodedData != NULL ) {

                        if( dataLength == buf->mLengthInBytes ) {

                            fileChunk->mChunkData = decodedData;

                            fileChunk->mSemaphore->signal();
                            }
                        else {
                            AppLog::error(
                                "fileShare -- chunk handler1",
                                "Data length incorrect" );
                            }
                        }
                    else {
                        AppLog::error(
                            "fileShare -- chunk handler1",
                            "Failed to decode data (base64 format bad?)" );
                        }
            // return -1 we processed the message!
            return nRetVal;
           }
        }

//03-03-2005
    if( strcmp( inFromAddress, fileChunk->mVirtualAddress ) != 0 ) {
        // not the address we're looking for	
        // no utility in any case
		return 0;
        }
    
    // is this a FileChunk message?
    // look at second token
    SimpleVector<char *> *tokens = tokenizeString( inBody );
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

                char *lengthString = *( tokens->getElement( 7 ) );
                // default to 0
                int length = 0;
                sscanf( lengthString, "%d", &length );
                char *encodedData = *( tokens->getElement( 9 ) );               

				// jroc
				nRetVal = -1;
				g_nBytesDownloaded += strlen( inBody ); // count #bytes related to download
				
                // is this the chunk we're looking for?
                if( fileChunk->mChunkNumber == chunkNumber ) {
			               
                    
                    fileChunk->mLengthInBytes = length;
                                        
                    int dataLength;
                    
                    unsigned char *decodedData =
                        base64Decode( encodedData, &dataLength );
                    
                    if( decodedData != NULL ) {
                        
                        if( dataLength == length ) {
                            
                            fileChunk->mChunkData = decodedData;
                            
							fileChunk->mSemaphore->signal();
							}
                        else {
                            AppLog::error(
                                "fileShare -- chunk handler2",
                                "Data length incorrect" );
                            }
                        }
                    else {
                        AppLog::error(
                            "fileShare -- chunk handler2",
                            "Failed to decode data (base64 format bad?)" );
                        }
                    
                    }
                else {

                    if( chunkNumber > fileChunk->mChunkNumber ) { // always move forward
                    // store the chunk in a buffer
                    int encodedLength = strlen( encodedData );
                    // limit buffer & chunk size, retries fill in gaps
                    if( fileChunk->mBufferVector->size() < 10 && encodedLength < 25000 ) {
                        // create a new record for buffer storage
                        ShareChunkBufferWrapper *bufferChunk = new ShareChunkBufferWrapper();
                        bufferChunk->mLengthInBytes = length;
                        bufferChunk->mChunkNumber = chunkNumber;
                        bufferChunk->mChunkData = stringDuplicate( encodedData );
                        fileChunk->mBufferVector->push_back( bufferChunk );
//printf("BUFFERED CHUNK: C%d ENL%d L%d\n", chunkNumber, encodedLength, length);
                       }
                      }
                    }				
                }
            delete [] filePath;
            }
        }
    
    for( i=0; i<numTokens; i++ ) {
        delete [] *( tokens->getElement( i ) );
        }
    
    delete tokens;

    
    // no utility generated
    //return 0;
	// jroc
	return nRetVal;
    }

// jroc
int muteShareGetFile(
					 char *inVirtualAddress, char *inFilePath,
					 char *inFileHash,
					 unsigned int inStartChunk,
					 SHA_CTX inShaContext,
					 char (*inFileChunkHandler)( unsigned char *, int, void * ),
					 void *inExtraHandlerArgument,
    				 unsigned long *outFileSizeInBytes,
					 int inTimeoutInMilliseconds ) 
{
    
	unsigned long	unInnerCounter; // JROC 02-14-2005 Happy Valentine's Day -- for counting retries..

    char *logMessage = autoSprintf(
        "Trying to get file %s : %s", inVirtualAddress, inFilePath );
    AppLog::info( "fileShare",
		logMessage );
    delete [] logMessage;
	
	
	
    // We adjust download chunk timeouts with the following weighted average
    // formula:
    // currentTimeout =
    //     ( A * (2 * lastChunkTime) + B * currentTimeout ) / (A+B);
    // A is the weight given to the most recent chunk round-trip time
    double recentChunkTimeWeight;
    // B is the weight given to our current timeout
    double currentTimeoutWeight;
    // If the round-trip time of recent chunks is consistent, then our
    // timeout will eventually approach (2 *) that average round-trip time.
	
    char settingFound;
    recentChunkTimeWeight = SettingsManager::getFloatSetting( "downloadTimeoutRecentChunkWeight", &settingFound );
    if( !settingFound ) 
	{
        recentChunkTimeWeight = 1;
    }
	
    currentTimeoutWeight = SettingsManager::getFloatSetting( "downloadTimeoutCurrentTimeoutWeight", &settingFound );
	
    if( !settingFound ) 
	{
        currentTimeoutWeight = 2;
    }
	
	
    int maxNumRetries = SettingsManager::getIntSetting( "downloadChunkRetries", &settingFound );

    if( !settingFound ) 
	{
        maxNumRetries = 10;
    }
	
	// JROC 02-14-2005 
	// let's prevent some bogus settings
	if( 0 == maxNumRetries )
	{
		maxNumRetries = 10;
	}

    float freshRouteProbability = SettingsManager::getFloatSetting( "downloadRetryFreshRouteProbability", &settingFound );
    
	if( !settingFound ) 
	{
        freshRouteProbability = 0.25;
	}
	
    int logTimeoutFlag = SettingsManager::getIntSetting( "logDownloadTimeoutChanges", &settingFound );
	
    char logTimeoutChanges;
    FILE *timeoutLogFile = NULL;
    
    if( settingFound &&
        logTimeoutFlag == 1 ) 
	{
		
        logTimeoutChanges = true;
		
        // get the pathless file name to use in our timeout log file name
        char *lastSlash = inFilePath;
        char *nextSlash = inFilePath;
        while( nextSlash != NULL ) 
		{
            // skip slash
            if( nextSlash[0] == '/' ) 
			{
                lastSlash = &( nextSlash[1] );
			}
            
            nextSlash = strstr( lastSlash, "/" );
		}
        
        char *localFileName = lastSlash;
		
        char *timeoutLogFileName = autoSprintf( "timeoutLog_%s",
			localFileName );
		
        timeoutLogFile = fopen( timeoutLogFileName, "w" );
		
        delete [] timeoutLogFileName;
	}
    else 
	{
        // default
        logTimeoutChanges = false;
	}
	
    unsigned long currentTimeoutMilliseconds = inTimeoutInMilliseconds;
	
    // the time used by retries
    // (will always be twice currentTimeoutMilliseconds)
    unsigned long retryTimeoutMilliseconds = 2 * currentTimeoutMilliseconds;

	// JROC -- 02/14/2005 limit minimum retry timeout to 10 seconds
	if( retryTimeoutMilliseconds < 10000 )
	{
		retryTimeoutMilliseconds = 10000;
	}
    
    // large value to start, an hour
    // will get replaced by time from first chunk
    unsigned long bestChunkTimeMilliseconds = 3600000; 
	
	
    // the longest timeout ever used during this transfer
    // we should double this timeout for our final retry 
    unsigned long longestTimeoutMilliseconds = currentTimeoutMilliseconds;
    
    // flag that we set to try a fresh route message for the next chunk
    // we set this if chunk times are getting longer and longer, which
    // might mean that our route is deteriorating from too many local
    // patch-ups
    char useFreshRouteForNextChunk = false;
    
	
    int length;
    int chunkCount;
    char *mimeType = NULL;
	
	int status = muteShare_internalGetFileInfo( inVirtualAddress, inFilePath,
		&length,
		&chunkCount,
		&mimeType,
		currentTimeoutMilliseconds,
		inFileChunkHandler,
		inExtraHandlerArgument );
	
    if( status == MUTE_SHARE_FILE_INFO_CANCELED ) 
	{
		if( NULL != timeoutLogFile )
		{
			fclose( timeoutLogFile );
		}
		
		if( NULL != mimeType )
		{
			delete [] mimeType;
			mimeType = NULL; // jroc
		}

        return MUTE_SHARE_FILE_TRANSFER_CANCELED;
	}
    else if( status == MUTE_SHARE_FILE_INFO_TIMEOUT ) 
	{   
		if( NULL != timeoutLogFile )
		{
			fclose( timeoutLogFile );
		}

		if( NULL != mimeType )
		{
			delete [] mimeType;
			mimeType = NULL;
		}
        return MUTE_SHARE_FILE_TRANSFER_FAILED;
	}
    else if( status == MUTE_SHARE_FILE_INFO_NOT_FOUND ) 
	{
        logMessage = autoSprintf(
            "Aborting file get:  file info not found for file  %s : %s",
            inVirtualAddress, inFilePath );
        AppLog::error( "fileTransfer",
			logMessage );
        delete [] logMessage;
        
		if( NULL != timeoutLogFile )
		{
			fclose( timeoutLogFile );
		}

		if( NULL != mimeType )
		{
			delete [] mimeType;
			mimeType = NULL;
		}
        return MUTE_SHARE_FILE_NOT_FOUND;
	}
	
    // we don't need the mime type
	if( NULL != mimeType )
	{
		delete [] mimeType;
		mimeType = NULL;
	}
	
	
    RandomSource *randSource = new StdRandomSource();
	
	// jroc
    // hash for the incoming file data, could already have a value
    SHA_CTX shaContext = inShaContext;     
    
    // set the size return value here so that the handler can use it
    *outFileSizeInBytes = length;
	
    
    char *encodedFilePath = URLUtils::hexEncode( inFilePath );
	
    char chunkMissed = false;
    char canceled = false;
	// jroc.. moved this file chunk sharer out, so we don't keep creating new ones!
	ShareFileChunkWrapper *fileChunk = NULL;
	fileChunk = new ShareFileChunkWrapper();
	fileChunk->mVirtualAddress = inVirtualAddress;
    fileChunk->mFilePath = inFilePath;
	fileChunk->mSemaphore = NULL; // JROC
	fileChunk->mSemaphore = new Semaphore();
	fileChunk->mChunkData = NULL;
	// 03-03-2005
	fileChunk->mBufferVector = new SimpleVector<ShareChunkBufferWrapper *>();
	fileChunk->wasBuffered = false;	
	// 03-03-2005

    for( int i = inStartChunk; i<chunkCount && !chunkMissed && !canceled; i++ ) 
	{
		
        // register a handler for the chunk       
        fileChunk->mChunkNumber = i;
        fileChunk->wasBuffered = false;
        		
        int handlerID =
            muteAddMessageHandler( muteShare_internalFileChunkHandler,
			(void *)fileChunk );
				
        // send out a request message for the chunk
		
        char *message = autoSprintf(
            "MessageType: FileChunkRequest\n"
            "FilePath: %s\n"
            "Chunknumber: %d",
            encodedFilePath,
            i );
		
        unsigned long requestStartSeconds;
        unsigned long requestStartMilliseconds;
		
        Time::getCurrentTime( &requestStartSeconds,
			&requestStartMilliseconds );
		
		if( useFreshRouteForNextChunk ) 
		{
			muteSendMessage( muteShareVirtualAddress,
				inVirtualAddress, message, "FRESH_ROUTE" );
			
		}
        else 
		{
            muteSendMessage( muteShareVirtualAddress,
				inVirtualAddress, message );
		}
        
		// jroc
		int numRetries = 0;
		int responseReceived = fileChunk->mSemaphore->wait(5000);
		
		int newMaxRetries = maxNumRetries * (int)(currentTimeoutMilliseconds / 5000);
	
		while( !canceled &&
			(numRetries < newMaxRetries ) &&
			(responseReceived !=1)
			)
		{
			// wait for our handler to get the response
			responseReceived = fileChunk->mSemaphore->wait( 5000 );	
			
			// jroc .. just a check to see if it's canceled... 
			canceled = !(inFileChunkHandler( NULL,-1,inExtraHandlerArgument ));
			if( 0 == (numRetries % 5) )
			{
	            muteSendMessage( muteShareVirtualAddress, inVirtualAddress, message );
			}
			numRetries++;
		}
		      
		
        
        // update our longest timeout tracker
        if( currentTimeoutMilliseconds > longestTimeoutMilliseconds ) 
		{
            longestTimeoutMilliseconds = currentTimeoutMilliseconds;
		}
		
        
        // JROC
		// GOING TO USE RETRY COUNT BASED ON TOTAL MAJOR TIMEOUTS
		// WILL USE SMALLER TIMEOUTS * X RETRIES == (TOTAL TIMEOUT * MAJOR RETRIES)
		// WHERE MAJOR RETRIES IS THE INCOMING RETRY COUNT... 
		// JUST A SLIGHT AMOUNT OF FIXIN HERE
		newMaxRetries = maxNumRetries * ( retryTimeoutMilliseconds / 5000 );
		numRetries = 0;
		// we'll just wait 5 sec for updating the count
        // loop to perform multiple retries, as needed, for this chunk
        while( responseReceived != 1 &&
			numRetries < newMaxRetries &&
			! canceled ) 
		{
            // no response, timed out, so retry
			
            // first ask chunk handler if we should keep going
            // pass NULL to it to indicate that we timed out on the current
            // chunk
			// jroc .. just a check to see if it's canceled... 
			canceled = !(inFileChunkHandler( NULL,-1,inExtraHandlerArgument ));

            if( !canceled )
			{
				// not canceled, so retry
				
				// should we use a freshroute?
				// pick at random, but always freshroute on our last retry
				char useFreshRoute = false;
				if( randSource->getRandomFloat() <= freshRouteProbability ) 
				{
					useFreshRoute = true;
				}
				
				//jroc if( numRetries == maxNumRetries - 1 )
				if( numRetries == (newMaxRetries - 1) ) 
				{
					// last retry
					// go down fighting with a FRESH_ROUTE
					useFreshRoute = true;
				}
				
				
				char *flags;
				char *extraLogMessage;
								
				if( useFreshRoute ) 
				{
					flags = "FRESH_ROUTE";
					extraLogMessage = "and trying a FRESH_ROUTE";
					
					// set the flag, since we are deciding to use a
					// fresh route now
					useFreshRouteForNextChunk = true;
				}
				else 
				{
				
					flags = NULL;
					extraLogMessage = "";
				}
				
				//jroc if( numRetries == maxNumRetries - 1 ) 
				if( numRetries == (newMaxRetries - 1) ) 
				{
					// last retry...  use a longer timeout
					// double the longest timeout that we have ever used
					retryTimeoutMilliseconds = 2 * longestTimeoutMilliseconds;
				}
				
				
				logMessage = autoSprintf(
					"Timed out, so sending retry %d with timeout of %d ms, "
					"%s for chunk %d of "
					"file %s : %s",
					numRetries, retryTimeoutMilliseconds,
					extraLogMessage,
					i, inVirtualAddress, inFilePath );
				
				AppLog::info( "fileShare",
					logMessage );
				
				if( logTimeoutChanges ) 
				{
					if( NULL != timeoutLogFile )
					{
						fprintf( timeoutLogFile, "%s\n\n", logMessage );
						fflush( timeoutLogFile );
					}
				}
				
				delete [] logMessage;
											
                // Don't reset the response timer here.
                // Thus, the chunk time will include the time taken by
                // all retries combined.  This will give us a conservative
                // (high) estimate of the actual chunk time, which we want
                // to set conservative timeouts.
                // We want to *avoid* a cycle where we have overly short
                // timeouts and get each chunk through retries, but never
                // get good round-trip estimates during retries, since retries
                // can make round-trip times look arbitrarily short.
				
                // Unecessary retries (when a long timeout would suffice)
                // cause undue network load.
                
                // reset start time
                // Time::getCurrentTime( &requestStartSeconds,
                //                       &requestStartMilliseconds );
				
                muteSendMessage( muteShareVirtualAddress, inVirtualAddress, message, flags );
			
				unInnerCounter = 0;
				canceled = !( inFileChunkHandler( NULL, -1, inExtraHandlerArgument ) );				
				while( !canceled && (unInnerCounter < (unsigned int)(newMaxRetries/maxNumRetries) ) )
				{
					// 02/14/2005 -- had a problem with too short of delays between
					// file chunk requests.. this was most likely cluttering the network
					// NATE busted me on this and I'm glad he did.. hopefully this will
					// really help us out a lot.. sorry folks.. I wish I hadn't screwed this
					// up so long ago back in my first releases.
					responseReceived = fileChunk->mSemaphore->wait( 5000 );
					
					canceled = !(inFileChunkHandler( NULL,-1,inExtraHandlerArgument ));
					unInnerCounter++;
					// just trying something..
					if( 0 == (unInnerCounter % 5 ) )
					{
						muteSendMessage( muteShareVirtualAddress, inVirtualAddress, message, flags );
					}
					numRetries++;
				}                
			}
		} // end of the retry loop
		
		
		delete [] message;
				
		muteRemoveMessageHandler( handlerID );
		
		if( canceled ) 
		{
			// canceled during a retry
			if( fileChunk->mChunkData != NULL ) 
			{
				// maybe handler was called between our timeout and
				// our call to muteRemoveMessageHandler
				
				delete [] fileChunk->mChunkData;
				fileChunk->mChunkData = NULL;
			}
		}
		else if( responseReceived == 1 ) 
		{
			
			unsigned long lastChunkTimeInMilliseconds =
				Time::getMillisecondsSince( requestStartSeconds,
				requestStartMilliseconds );
			
			// ignore timing information that comes from multiple
			// retries
			// If our chunk time is longer than our current timeout,
			// we might be over-estimating our chunk time.
			if( lastChunkTimeInMilliseconds < currentTimeoutMilliseconds ) 
			{
				
				// update timeouts relative to this chunk's transfer time
				
				
				// do we have a new best chunk time?
				if( lastChunkTimeInMilliseconds < bestChunkTimeMilliseconds ) 
				{
					bestChunkTimeMilliseconds = lastChunkTimeInMilliseconds;
				}
				
				
				// take weighted average of current timeout with double time
				// for our last chunk
				// The formula:
				// currentTimeout =
				//    ( A * (2 * lastChunkTime) + B * currentTimeout ) / (A+B);
				currentTimeoutMilliseconds =
					(unsigned long) (
					( recentChunkTimeWeight *
					( 2 * lastChunkTimeInMilliseconds )
					+
					currentTimeoutWeight * currentTimeoutMilliseconds )
					/ ( recentChunkTimeWeight + currentTimeoutWeight ) );
				
				
				
				// our timeout should always be twice as large as our
				// most recent chunk time
				// It decays gradually with the previous weighted formula
				// but we always increase it immediately here if it is too low.
				if( currentTimeoutMilliseconds <
					2 * lastChunkTimeInMilliseconds ) 
				{
					
					currentTimeoutMilliseconds =
						2 * lastChunkTimeInMilliseconds;
				}
				
				// retry timeout is always double our current timeout
				retryTimeoutMilliseconds =
					2 * currentTimeoutMilliseconds;
				
				
				// log the timeout change
				char *logMessage = autoSprintf(
					"Saw a chunk with time of %d ms, "
					"so changing timeout to %d ms, "
					"chunk %d of file %s : %s",
					lastChunkTimeInMilliseconds,
					currentTimeoutMilliseconds, i,
					inVirtualAddress, inFilePath );
				AppLog::detail( "fileShare",
					logMessage );
				
				if( logTimeoutChanges ) 
				{
					if( NULL != timeoutLogFile )
					{					
						fprintf( timeoutLogFile, "%s\n\n", logMessage );
						fflush( timeoutLogFile );
					}
				}
				
				delete [] logMessage;
			}
			else 
			{
				// log the chunk time
				char *logMessage = autoSprintf(
					"Saw a chunk with time of %d ms, "
					"but or current timeout was only %d ms, "
					"so NOT updating timeouts, "
					"chunk %d of file %s : %s",
					lastChunkTimeInMilliseconds,
					currentTimeoutMilliseconds, i,
					inVirtualAddress, inFilePath );
				AppLog::detail( "fileShare",
					logMessage );
				
				if( logTimeoutChanges ) 
				{
					if( NULL != timeoutLogFile )
					{
						fprintf( timeoutLogFile, "%s\n\n", logMessage );
						fflush( timeoutLogFile );
					}
				}
				
				delete [] logMessage;
			}
			
			
			if( useFreshRouteForNextChunk ) 
			{
				// we were using a fresh route, which should give us
				// the best possible chunk transfer time given the state
				// of the network.
				// our old bestChunkTimeMilliseconds is outdated
				
				// but only update it if we weren't retrying, since
				// retries can produce incorrect chunk time measurements,
				// as mentioned above
				if( numRetries == 0 ) 
				{
					bestChunkTimeMilliseconds = lastChunkTimeInMilliseconds;
				}
				
				useFreshRouteForNextChunk = false;
			}
			else 
			{
				// not using a fresh route for this chunk
				
				// should we use a fresh route for the next chunk?
				
				// be cautious about use of FRESH_ROUTES, since they are
				// a burden on the network... only use one if our current
				// chunk took 10 times as long as our best chunk.
				if( lastChunkTimeInMilliseconds >
					10 * bestChunkTimeMilliseconds ) 
				{
					
					// our chunk times are much worse now than they used to
					// be... try a fresh route to improve them
					
					useFreshRouteForNextChunk = true;
					
					char *logMessage = autoSprintf(
						"Time of %d ms (best time %d ms) for chunk %d of "
						"file %s : %s, trying "
						"a freshroute for the next chunk.",
						lastChunkTimeInMilliseconds,
						bestChunkTimeMilliseconds,
						i, inVirtualAddress,
						inFilePath );
					AppLog::info( "fileShare",
						logMessage );
					
					if( logTimeoutChanges ) 
					{
						if( NULL != timeoutLogFile )
						{
							fprintf( timeoutLogFile, "%s\n\n", logMessage );
							fflush( timeoutLogFile );
						}
					}
					
					delete [] logMessage;
				}
			}
			
			
			// pass the chunk to the caller's handler
			char keepGoing = inFileChunkHandler( fileChunk->mChunkData,
				fileChunk->mLengthInBytes,
				inExtraHandlerArgument );
			
			if( NULL != fileChunk->mChunkData )
			{
				// add data to the hash
				SHA1_Update( &shaContext, fileChunk->mChunkData, fileChunk->mLengthInBytes );
			
				delete [] fileChunk->mChunkData;
				fileChunk->mChunkData = NULL;
			}
			else
			{
				// some weird fluke...
				keepGoing = false;
			}
			
			if( !keepGoing ) 
			{
				canceled = true;
			}
		}
		else 
		{
			// failed completely, retries and all
			
			char *logMessage = autoSprintf(
				"Giving up:  timed out (after %d retries, with a final retry "
				"timeout of %d ms) on chunk %d "
				"of file %s : %s",
				maxNumRetries,
				retryTimeoutMilliseconds,
				i, inVirtualAddress, inFilePath );
			AppLog::warning( "fileShare",
				logMessage );
			
			if( logTimeoutChanges ) 
			{
				if( NULL != timeoutLogFile )
				{					
					fprintf( timeoutLogFile, "%s\n\n", logMessage );
					fflush( timeoutLogFile );
				}
			}
			
			delete [] logMessage;
			
			chunkMissed = true;
			
			if( fileChunk->mChunkData != NULL ) 
			{
				// maybe handler was called between our timeout and
				// our call to muteRemoveMessageHandler
				
				delete [] fileChunk->mChunkData;
				fileChunk->mChunkData = NULL;
			}				
		}
	}

	// jroc.. moved this out so we only delete once!		
	if( NULL != fileChunk->mChunkData )
	{
		delete [] fileChunk->mChunkData;
		fileChunk->mChunkData = NULL;
	}
	delete fileChunk->mSemaphore;
	if( NULL != fileChunk )
	{
		delete fileChunk;
		fileChunk = NULL;
	}
	
	delete [] encodedFilePath;
		
	if( logTimeoutChanges ) 
	{
		if( NULL != timeoutLogFile )
		{
			fclose( timeoutLogFile );
		}
	}
	
	
	// compute the final hash
	unsigned char *rawDigest = new unsigned char[ SHA1_DIGEST_LENGTH ];
	SHA1_Final( rawDigest, &shaContext );
	
	char *digestHexString = hexEncode( rawDigest, SHA1_DIGEST_LENGTH );    
	delete [] rawDigest;
	
	
	// compare to supplied hash
	char hashMismatch = false;
	if( strcmp( digestHexString, inFileHash ) != 0 ) 
	{
		hashMismatch = true;
	}
	
	delete [] digestHexString;
	
	delete randSource;
	
	if( chunkMissed ) 
	{
		return MUTE_SHARE_FILE_TRANSFER_FAILED;
	}
	else if( canceled ) 
	{
		return MUTE_SHARE_FILE_TRANSFER_CANCELED;
	}
	else if( hashMismatch ) 
	{
		// transfer succeeded (and not canceled), but hash doesn't match
		return MUTE_SHARE_FILE_TRANSFER_HASH_MISMATCH;
	}
	else 
	{
		return MUTE_SHARE_FILE_TRANSFER_COMPLETE;
	}
}


// JROC....
void muteUploadStatusLock()
{
	// necessary for deleting active uploads from list
	// to avoid duplicates being added because of faulty
	// synchronization
	muteUploadDataLock->lock();
}

void muteUploadStatusUnlock()
{
	muteUploadDataLock->unlock();
}

void muteStopUploadStatusUpdates()
{
	nStopUploadUpdates = true;
}

void muteStartUploadStatusUpdates()
{
	nStopUploadUpdates = false;
}

void muteRemoveUploadStatus( const int inUploadIDToRemove )
{
    int numUploads = muteUploadIDs->size();
    
    for( int i=0; i < numUploads; i++ ) 
	{
		if( inUploadIDToRemove == *( muteUploadIDs->getElement( i ) ) )
		{
			muteUploadIDs->deleteElement( i );

			delete [] *( muteUploadHostAddresses->getElement( i ) );
			muteUploadHostAddresses->deleteElement( i );

			delete [] *( muteUploadFilePaths->getElement( i ) );
			muteUploadFilePaths->deleteElement( i );

			muteUploadLastChunksSent->deleteElement(i);

			muteUploadChunksInFiles->deleteElement(i);

			muteUploadFirstChunkTimes->deleteElement(i);

			muteUploadLastChunkTimes->deleteElement(i);
			break; // we're done.. bug out..
		}        
	}

    return;
}

int muteShareGetUploadStatus( int **outUploadIDs,
                              char ***outHostAddresses,
                              char ***outFilePaths,
                              int **outChunksInFile,
                              int **outLastChunks,
                              unsigned long **outFirstChunkTimes,
                              unsigned long **outLastChunkTimes ) {
    int *returnIDs;
    char **returnHostAddresses;
    char **returnPaths;
    int *returnLastChunks;
    int *returnChunksInFile;
    unsigned long *returnFirstChunkTimes;
    unsigned long *returnLastChunkTimes;
    
    muteUploadDataLock->lock();

    int numUploads = muteUploadIDs->size();

    returnIDs = new int[ numUploads ];
    returnHostAddresses = new char*[ numUploads ];
    returnPaths = new char*[ numUploads ];
    returnLastChunks = new int[ numUploads ];
    returnChunksInFile = new int[ numUploads ];
    returnFirstChunkTimes = new unsigned long[ numUploads ];
    returnLastChunkTimes = new unsigned long[ numUploads ];
    
    for( int i=0; i<numUploads; i++ ) {
        returnIDs[i] = *( muteUploadIDs->getElement( i ) );
        returnHostAddresses[i] =
            stringDuplicate( *( muteUploadHostAddresses->getElement(i) ) ); 
        returnPaths[i] =
            stringDuplicate( *( muteUploadFilePaths->getElement(i) ) );
        
        returnLastChunks[i] = *( muteUploadLastChunksSent->getElement(i) );
        returnChunksInFile[i] = *( muteUploadChunksInFiles->getElement(i) );
        returnFirstChunkTimes[i] =
            *( muteUploadFirstChunkTimes->getElement(i) );
        returnLastChunkTimes[i] = *( muteUploadLastChunkTimes->getElement(i) );
        }

    muteUploadDataLock->unlock();


    *outUploadIDs = returnIDs;
    *outHostAddresses = returnHostAddresses;
    *outFilePaths = returnPaths;
    *outLastChunks = returnLastChunks;
    *outChunksInFile = returnChunksInFile;
    *outFirstChunkTimes = returnFirstChunkTimes;
    *outLastChunkTimes = returnLastChunkTimes;

    return numUploads;
    }
