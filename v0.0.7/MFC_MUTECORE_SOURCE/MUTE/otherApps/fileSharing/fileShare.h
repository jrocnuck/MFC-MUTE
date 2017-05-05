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
 * 2003-November-6   Jason Rohrer
 * Added hashes and file sizes to search results.
 *
 * 2003-November-11   Jason Rohrer
 * Added b flag to fopen of binary files.
 *
 * 2003-December-23   Jason Rohrer
 * Added hash checking for incoming files.
 *
 * 2004-January-12   Jason Rohrer
 * Increased default timeout from 10 seconds to 30 seconds.
 *
 * 2004-February-12   Jason Rohrer
 * Added upload stats patch submitted by Mycroftxxx.
 *
 * 2004-February-13   Jason Rohrer
 * Cleaned up the patched upload-tracking code.
 *
 * 2004-February-21   Jason Rohrer
 * Changed GetFile callback spec to support fine-grained retry reporting.
 *
 * 2004-March-8   Jason Rohrer
 * Added support for passing fileInfo timeouts to file chunk handler.
 */



/**
 * The API for the file sharing layer of MUTE.
 *
 * All calls are thread-safe.
 *
 * All function parameters must be destroyed by caller.
 * All string parameters must be \0-terminated.
 *
 * Before calling this API, application should call
 * muteSeedRandomGenerator and muteStart
 * from the messageRouting layer
 *
 * After using this API, application should call
 * muteStop
 * from the messageRouting layer.
 * Application may also optionally call
 * muteGetRandomGeneratorState
 * from the messageRouting layer to save the state of the random generator.
 *
 * @author Jason Rohrer.
 */



#ifndef MUTE_FILE_SHARING_API
#define MUTE_FILE_SHARING_API

#include "minorGems/io/file/File.h" // 01-22-2005 moved here
#include "minorGems/system/Thread.h" // jroc
#include "minorGems/crypto/hashes/sha1.h"
#include <time.h>



/**
 * Starts the file sharing layer of this node.
 *
 * Must be called before using the file sharing API.
 *
 * Should be called after calling muteSeedRandomGenerator and muteStart
 * from the messageRouting layer.
 */
void muteShareStart();



/**
 * Stops the file sharing layer of this node.
 *
 * Must be called after using the file sharing API.
 *
 * Should be called before calling muteStop from the messageRouting layer.
 */
void muteShareStop();



/**
 * Sets the root path that this node will share files from.
 *
 * If no path is set, this node defaults to sharing files from the "files"
 * directory.
 *
 * This path is saved between startups by this API (in the settings folder).
 *
 * @param inPath the platform-dependent path, relative to the main directory
 *   of this node.
 *
 * Example: <PRE>
 * muteSetSharingPath( "temp/myFiles" );
 * </PRE> 
 */
void muteShareSetSharingPath( char *inPath );



/**
 * Gets the root path that this node is sharing files from.
 *
 * @return the platform-dependent path, relative to the main directory
 *   of this node.
 */
char *muteShareGetSharingPath();

//JROC... 
/**
 * Sets the root path that this node will download incoming files to.
 *
 * If no path is set, this node defaults to incoming files in the "files/MUTE_incoming"
 * directory.
 *
 * This path is saved between startups by this API (in the settings folder).
 *
 * @param inPath the platform-dependent path, relative to the main directory
 *   of this node.
 *
 * Example: <PRE>
 * muteShareSetIncomingFilesPath( "temp/myIncomingFiles" );
 * </PRE> 
 */
void muteShareSetIncomingFilesPath( char *inPath );

/**
 * Gets the root path that this node is using for incoming files.
 *
 * @return the platform-dependent path, relative to the main directory
 *   of this node for incoming files!
 */
char *muteShareGetIncomingFilesPath();

//JROC... 
/**
 * Sets the root path that this node will creat hash files in.
 *
 * If no path is set, this node defaults to incoming files in the "files/MUTE_hashes"
 * directory.
 *
 * This path is saved between startups by this API (in the settings folder).
 *
 * @param inPath the platform-dependent path, relative to the main directory
 *   of this node.
 *
 * Example: <PRE>
 * muteShareSetHashFilesPath( "temp/myHashFiles" );
 * </PRE> 
 */
void muteShareSetHashFilesPath( char *inPath );

/**
 * Gets the root path that this node is using for hash files.
 *
 * @return the platform-dependent path, relative to the main directory
 *   of this node for hash files!
 */
char *muteShareGetHashFilesPath();

/*
 01-22-2005
 jroc .. added declaration here so we can use the function elsewhere
*/
char *muteShare_internalFileToEncodedPath( File *inFile );

/**
 * Initiates a search on the network.
 *
 * This call returns after the search has been canceled by the handler.
 *
 * @param inSearchString a string of words to search for.
 * @param inResultHandler a callback function to pass received results to.
 *   This function must return (char) and take the following arguments:
 *   (char *inFileHostVirtualAddress, char *inFilePath,
 *    int inFileLengthInBytes, char *inFileSHA1Hash,
 *    void *inExtraArgument ).
 *   inFileHostVirtualAddress is the virtual address of the host
 *      returning the result.  Will be destroyed by handler's caller.
 *   inFilePath is the /-delimited file path of the result.
 *      Will be destroyed by handler's caller.
 *   inFileLengthInBytes is the length of the file in bytes.
 *   inFileSHA1Hash is the SHA1 hash of the file contents as a hex-encoded
 *      string.
 *      Will be destroyed by handler's caller.
 *   If no new results have been received after inTimeoutInMilliseconds
 *   passes since the last handler call, the handler will be called
 *   with inFileHostVirtualAddress, inFilePath, and inFileSHA1Hash set to NULL.
 *   The handler should return true to continue the search or false
 *   to cancel the search and ignore future results.
 * @param inExtraHandlerArgument pointer to an extra argument to be passed
 *   in to the handler function each time it is called.
 *   Must be destroyed by caller after the search has been canceled.
 * @param inTimeoutInMilliseconds the time to wait between results before
 *   calling the handler with NULL arguments.
 *   Defaults to 10 seconds (10,000 milliseconds).
 *
 * Example: <PRE>
 * // define a handler function (this one writes results to a file)
 * char myResultHandler( char *inFileHostVirtualAddress, char *inFilePath,
 *                       unsigned long inFileLength, char *inFileHash,
 *                       void *inExtraArgument ) {
 *     // unwrap a file handle from our extra argument
 *     FILE *file = (FILE *)inExtraArgument;
 *         
 *     // check that arguments are not NULL
 *     // (in other words, new results have arrived)
 *     if( inFileHostVirtualAddress != NULL && inFilePath != NULL ) {
 *         fprintf( file, "%s : %s\n", inFileHostVirtualAddress, inFilePath );
 *         return true;
 *         }
 *     else {
 *         // no new results, must have timed out
 *         fclose( file );
 *         // cancel this search
 *         return false;
 *         }
 *     }
 *
 * // elsewhere in code, register the handler function to catch the results
 * // wrap file handle in extra argument
 * FILE *file = fopen( "searchResults", "w" );
 *
 * // use default timeout of 10 seconds
 * muteShareSearch( "test tone mp3", myChunkHandler, (void *)file );
 * </PRE>
 */
void muteShareSearch(
    char *inSearchString,
    char (*inResultHandler)( char *, char *, unsigned long, char *,void * ),
    void *inExtraHandlerArgument,
    int inTimeoutInMilliseconds = 10000 );



// constants returned by muteGetSharedFile
const int MUTE_SHARE_FILE_NOT_FOUND = 0;
const int MUTE_SHARE_FILE_TRANSFER_FAILED = 1;
const int MUTE_SHARE_FILE_TRANSFER_CANCELED = 2;
const int MUTE_SHARE_FILE_TRANSFER_COMPLETE = 3;
const int MUTE_SHARE_FILE_TRANSFER_HASH_MISMATCH = 4;


/**
 * Gets a file from a node on the network.
 *
 * This call returns after the file has been completely fetched and
 * passed through the handler.
 *
 * @param inVirtualAddress the virtual address of the file host.
 * @param inFilePath a /-delimited file path, relative to the host's
 *   root sharing directory.
 * @param inFileHash the SHA1 hash of the file as a hex-encoded
 *   ascii string.  The received file will be checked against this hash.
 * @param inStartChunk - file chunk starting point for receiving file
 * @param inShaContext - HASH context for downloadable file.
 * @param inFileChunkHandler a callback function to pass received file
 *   chunks to. This function must return (char) and take the following
 *   arguments:
 *   (unsigned char *inChunk, int inChunkLengthInBytes,
 *    void *inExtraArgument ).
 *   inChunk is the raw bytes of chunk data, or NULL to indicate that
 *      the current chunk (or file info request) has timed out and will be
 *      retried.
 *      Will be destroyed by handler's caller.
 *   inChunkLengthInBytes is the length of the chunk in bytes, or -1 if
 *      the current chunk (or file info request) has timed out and will be
 *      retried.
 *      Will be destroyed by handler's caller.
 *   The handler should return true to continue the transfer or false
 *   to cancel the transfer.
 * @param inExtraHandlerArgument pointer to an extra argument to be passed
 *   in to the handler function each time it is called.
 *   Must be destroyed by caller after the file transmission is complete.
 * @param outFileSizeInBytes pointer to where the file size should be
 *   returned.  This value is set before the handler is called, so the
 *   handler can use this value.
 * @param inTimeoutInMilliseconds the time to wait for the file info
 *   to return.  Defaults to 30 seconds (30,000 milliseconds).
 *
 * @return one of MUTE_SHARE_FILE_NOT_FOUND,
 *   MUTE_SHARE_FILE_TRANSFER_CANCELED, MUTE_SHARE_FILE_TRANSFER_FAILED,
 *   MUTE_SHARE_FILE_TRANSFER_COMPLETE,
 *   or MUTE_SHARE_FILE_TRANSFER_HASH_MISMATCH.
 *
 * Example: <PRE>
 * // define a handler function
 * char myChunkHandler( unsigned char *inChunk, int inChunkLengthInBytes,
 *                      void *inExtraArgument ) {
 *     // unwrap a file handle from our extra argument
 *     FILE *file = (FILE *)inExtraArgument;
 *     if( inChunk != NULL ) {
 *         // we have real data for this chunk, write it to file
 *         fwrite( inChunk, 1, inChunkLengthInBytes, file );
 *         }
 *     else {
 *         // chunk timed out, no data yet, do nothing
 *         }
 *     // do not cancel
 *     return true;
 *     }
 *
 * // elsewhere in code, register the handler function to fetch the file
 * // wrap file handle in extra argument
 * FILE *file = fopen( "receivedFile", "wb" );
 * int fileSize;
 * // use default timeout of 10 seconds
 * int result = muteShareGetFile( "C0D62D331359BF2A27BEA46693F1CD32E3B7519E",
 *                                "songs/test.mp3",
 *                                "9E6D1F25D46633BA1EC296A1D3813D887EBB8417",
 *                                myChunkHandler, (void *)file, &fileSize );
 * // after it returns, the file transfer is complete
 * fclose( file );
 * if( result != MUTE_SHARE_FILE_TRANSFER_COMPLETE ) {
 *     // handle a failure
 *     ...
 *     }
 * </PRE>
 */
 // jroc modified
int muteShareGetFile(
    char *inVirtualAddress, char *inFilePath,
    char *inFileHash, 
    unsigned int inStartChunk,
    SHA_CTX inShaContext, 
    char (*inFileChunkHandler)( unsigned char *, int, void * ),
    void *inExtraHandlerArgument,
    unsigned long *outFileSizeInBytes,
    int inTimeoutInMilliseconds = 30000 );



/**
 * Gets a list of upload status information.
 *
 * @param outUploadIDs pointer to location where an array of unique IDs
 *   for the uploads should be returned.
 *   The array must be destroyed by caller.
 * @param outHostAddresses pointer to location where an array of host virutal
 *   addresses strings, one for each upload, should be returned.
 *   The array and the strings it contains must be destroyed by caller.
 * @param outFilePaths pointer to location where an array of file path
 *   strings, one for each upload, should be returned.
 *   The array and the strings it contains must be destroyed by caller.
 * @param outChunksInFile pointer to location where an array of chunk counts,
 *   one for each upload, should be returned.
 *   The returned values indicate how many chunks are in each file.
 *   The array must be destroyed by caller.
 * @param outLastChunks pointer to location where an array of indexes of last
 *   chunk sent, one for each upload should be returned.
 *   These values give an estimate of how close an upload is to completion.
 *   The array must be destroyed by caller.
 * @param outFirstChunkTimes pointer to location where an array of
 *   first chunk times, one for each upload should be returned.
 *   These values are times in seconds as returned by the system call
 *   time( NULL ).
 *   The array must be destroyed by caller.
 * @param outLastChunkTimes pointer to location where an array of
 *   last chunk times, one for each upload should be returned.
 *   These values are times in seconds as returned by the system call
 *   time( NULL ).
 *   The array must be destroyed by caller.
 *
 * @return the number of uploads.
 *   (in other words, the length of each returned array).
 *
 * Example: <PRE>
 * int *uploadIDs;
 * char **hostAddresses;
 * char **filePaths;
 * int *chunkCounts;
 * int *lastChunks;
 * unsigned long *firstChunkTimes;
 * unsigned long *lastChunkTimes;
 * int numUploads = muteShareGetUploadStatus( &uploadIDs, &hostAddresses,
 *                                            &filePaths, &chunkCounts,
 *                                            &lastChunks, &firstChunkTimes,
 *                                            &lastChunkTimes );
 *
 * // process information here
 * ...
 * // now destroyed returned arrays
 * for( int i=0; i<numUploads; i++ ) {
 *     delete [] filePaths[i];
 *     delete [] hostAddresses[i];
 *     }
 * delete [] uploadIDs;
 * delete [] hostAddresses;
 * delete [] filePaths;
 * delete [] chunkCounts;
 * delete [] lastChunks;
 * delete [] firstChunkTimes;
 * delete [] lastChunkTimes;
 * </PRE>
 */
int muteShareGetUploadStatus( int **outUploadIDs,
                              char ***outHostAddresses,
                              char ***outFilePaths,
                              int **outChunksInFile,
                              int **outLastChunks,
                              unsigned long **outFirstChunkTimes,
                              unsigned long **outLastChunkTimes );



/* JROC --- attempt to remove an upload from the list 
and keep track of chunks sent! */
extern unsigned long g_ulChunksSent;
extern __int64 g_nBytesSent;
void muteUploadStatusLock();
void muteUploadStatusUnlock();
void muteStopUploadStatusUpdates();
void muteStartUploadStatusUpdates();
void muteRemoveUploadStatus( const int inUploadIDToRemove );
void muteBuildHash( const char *bKeepGoing );
void muteStopHashBuilder();
void muteStartHashBuilder();

/**
 * Thread that runs periodically creates Hashes for all files
 *	-- we only need hashes for people searching by hash...
 *  -- not a lot of people doing that most likely... 
 */
class HashBuilderThread : public Thread {

    public:
        /**
         * Constructs and starts a search thread
         * @param inSearchString must be destroyed by caller.
         * @param inExtraHandlerParam parameter to pass to results handler.
         */
        HashBuilderThread()
		{       
			m_bKeepGoing = true;
            start();
        }

        /**
         * Joins and destroys this search thread.
         */
        ~HashBuilderThread() 
		{
			m_bKeepGoing = false;
            join();       
        }

        // implements Thread interface
        void run() {
            muteBuildHash( &m_bKeepGoing );
            }
        
    private:
		char	m_bKeepGoing;
        
    };
#endif

