/*
 * Modification History
 *
 * 2003-September-1   Jason Rohrer
 * Created.
 *
 * 2003-September-4   Jason Rohrer
 * Improved return value of muteGetFileInfo.
 *
 * 2003-September-8   Jason Rohrer
 * Added support for setting timeouts in API calls.
 *
 * 2003-September-25   Jason Rohrer
 * Changed to return immediately if file requested for unknown contact.
 *
 * 2003-October-13   Jason Rohrer
 * Improved handling of timeouts.
 *
 * 2003-November-11   Jason Rohrer
 * Added b flag to fopen of binary files.
 */



/**
 * The API for the file transfer layer of MUTE.
 *
 * All calls are thread-safe.
 *
 * All function parameters must be destroyed by caller.
 * All string parameters must be \0-terminated.
 *
 * Before calling this API, application should call
 * muteSeedRandomGenerator and muteStart
 * from the messageRouting layer and then call
 * mutePointToPointStart
 * from the pointToPoint layer.
 *
 * After using this API, application should call
 * mutePointToPointStop
 * from the pointToPoint layer and then call
 * muteStop
 * from the messageRouting layer.
 * Application may also optionally call
 * muteGetRandomGeneratorState
 * from the messageRouting layer to save the state of the random generator.
 *
 * @author Jason Rohrer.
 */



#ifndef MUTE_FILE_TRANSFER_API
#define MUTE_FILE_TRANSFER_API



/**
 * Starts the file transfer layer of this node.
 *
 * Must be called before using the file transfer API.
 *
 * Should be called after calling muteSeedRandomGenerator and muteStart
 * from the messageRouting layer and then mutePointToPointStart from the
 * pointToPoint layer.
 */
void muteFileTransferStart();



/**
 * Stops the file transfer layer of this node.
 *
 * Must be called after using the file transfer API.
 *
 * Should be called before calling mutePointToPointStop from the
 * pointToPoint layer and then muteStop from the messageRouting layer.
 */
void muteFileTransferStop();



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
void muteSetSharingPath( char *inPath );



/**
 * Gets the root path that this node is sharing files from.
 *
 * @return the platform-dependent path, relative to the main directory
 *   of this node.
 */
char *muteGetSharingPath();



// constants returned by muteGetFileInfo
const int MUTE_FILE_CONTACT_NOT_REACHABLE = 0;
const int MUTE_FILE_NOT_FOUND = 1;
const int MUTE_FILE_FOUND = 2;
const int MUTE_FILE_CONTACT_UNKNOWN = 3;


/**
 * Gets information about a contact's file.
 *
 * @param inContactName the name of the contact (from the pointToPoint layer).
 * @param inFilePath a /-delimited file path, relative to the contact's
 *   root sharing directory.
 * @param outIsDirectory pointer to where a directory flag should be returned.
 *   Set to true if the file is a directory, or false if the file is a data
 *   file.
 * @param outLength pointer to where the length of the file in bytes
 *   (if a data file) or the number of entries (if a directory file) should
 *   be returned.
 * @param outChunkCount pointer to where the number of chunks in the file
 *   should be returned.
 * @param outMimeType pointer to where the MIME type of the file should be
 *   returned.  Set to NULL if the file is not found or contact not reachable.
 * @param inTimeoutInMilliseconds the time to wait for the file info
 *   to return.  Defaults to 10 seconds (10,000 milliseconds).
 *
 * @return one of MUTE_FILE_FOUND, MUTE_FILE_NOT_FOUND,
 *   MUTE_FILE_CONTACT_NOT_REACHABLE, or MUTE_FILE_CONTACT_UNKNOWN
 *
 * Example: <PRE>
 * char isDirectory;
 * char *mimeType;
 * int length;
 * int chunkCount;
 * // specify a timeout of 20 seconds
 * int status = muteGetFileInfo( "cydonia", "songs/test.mp3",
 *                               &isDirectory, &length, &chunkCount,
 *                               &mimeType,
 *                               20000 );
 * if( status == MUTE_FILE_FOUND ) {
 *     delete [] mimeType;
 *     }
 * </PRE> 
 */
int muteGetFileInfo( char *inContactName, char *inFilePath,
                     char *outIsDirectory, int *outLength,
                     int *outChunkCount,
                     char **outMimeType,
                     int inTimeoutInMilliseconds = 10000 );



/**
 * Gets a data file from a contact.
 *
 * This call returns after the file has been completely fetched and
 * passed through the handler.
 *
 * @param inContactName the name of the contact (from the pointToPoint layer).
 * @param inFilePath a /-delimited file path, relative to the contact's
 *   root sharing directory.
 * @param inFileChunkHandler a callback function to pass received file
 *   chunks to. This function must return (void) and take the following
 *   arguments:
 *   (unsigned char *inChunk, int inChunkLengthInBytes,
 *    void *inExtraArgument ).
 * @param inExtraHandlerArgument pointer to an extra argument to be passed
 *   in to the handler function each time it is called.
 *   Must be destroyed by caller after the file transmission is complete.
 * @param inTimeoutInMilliseconds the time to wait for each file info
 *   and each chunk of file data to return.
 *   Defaults to 10 seconds (10,000 milliseconds).
 *
 * @return true if the transfer completed successfully, or false if it failed.
 *
 * Example: <PRE>
 * // define a handler function
 * void myChunkHandler( unsigned char *inChunk, int inChunkLengthInBytes,
 *                      void *inExtraArgument ) {
 *     // unwrap a file handle from our extra argument
 *     FILE *file = (FILE *)inExtraArgument;
 *     fwrite( inChunk, 1, inChunkLengthInBytes, file );
 *     }
 *
 * // elsewhere in code, register the handler function to fetch the file
 * // wrap file handle in extra argument
 * FILE *file = fopen( "receivedFile", "wb" );
 * // use default timeout of 10 seconds
 * char success = muteGetFile( "cydonia", "songs/test.mp3",
 *                              myChunkHandler, (void *)file );
 * // after it returns, the file transfer is complete
 * fclose( file );
 * if( !success ) {
 *     // handle a failure
 *     ...
 *     }
 * </PRE>
 */
char muteGetFile( char *inContactName, char *inFilePath,
                  void (*inFileChunkHandler)( unsigned char *, int, void * ),
                  void *inExtraHandlerArgument,
                  int inTimeoutInMilliseconds = 10000 );



/**
 * Gets a list of files in a contact's directory.
 *
 * @param inContactName the name of the contact (from the pointToPoint layer).
 * @param inDirectoryPath a /-delimited directory path, relative to the
 *   contact's root sharing directory.
 * @param outFileCount pointer to where the number of files in the directory
 *   should be returned.
 * @param inTimeoutInMilliseconds the time to wait for the file info
 *   and each directory chunk to return.
 *   Defaults to 10 seconds (10,000 milliseconds).
 *
 * @return an array of file names, relative to inDirectoryPath, or NULL
 *   if fetching the listing fails.
 *
 * Example: <PRE>
 * int fileCount;
 * // specify a timeout of 15 seconds
 * char **fileList = muteGetDirectoryListing( "cydonia", "songs/testSongs",
 *                                            &fileCount,
 *                                            15000 );
 *
 * if( fileList != NULL ) {
 *     // process file list here
 *     ...
 *     // destroy file list
 *     for( int i=0; i<numFiles; i++ ) {
 *         delete [] fileList[i];
 *         }
 *     delete [] fileList;
 *     }
 * </PRE> 
 */
char** muteGetDirectoryListing( char *inContactName, char *inDirectoryPath,
                                int *outFileCount,
                                int inTimeoutInMilliseconds = 10000);


    
#endif
