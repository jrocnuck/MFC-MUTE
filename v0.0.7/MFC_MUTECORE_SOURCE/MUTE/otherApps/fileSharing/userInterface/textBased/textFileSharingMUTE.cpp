/*
 * Modification History
 *
 * 2003-October-14   Jason Rohrer
 * Created.
 *
 * 2003-October-16   Jason Rohrer
 * Added support for file transfer.
 * Added support for testing file size in muteShareGetFile.
 *
 * 2003-November-6   Jason Rohrer
 * Added support for new API features.
 *
 * 2003-November-11   Jason Rohrer
 * Added b flag to fopen of binary files.
 *
 * 2003-November-18   Jason Rohrer
 * Added runtime log level setting.
 *
 * 2003-November-24   Jason Rohrer
 * Fixed a memory leak.
 *
 * 2003-December-22   Jason Rohrer
 * Changed to save incoming files temporarily in a separate directory.
 *
 * 2003-December-23   Jason Rohrer
 * Added UI support for hash mismatch.
 *
 * 2003-December-30   Jason Rohrer
 * Fixed so that a saved random seed is used at most once.
 *
 * 2004-January-2   Jason Rohrer
 * Added startup question about firewall.
 *
 * 2004-January-11   Jason Rohrer
 * Fixed bug in reading user search string.
 * Fixed a locking bug.
 *
 * 2004-January-15   Jason Rohrer
 * Fixed looping bug when reading numerical user input.
 *
 * 2004-February-21   Jason Rohrer
 * Changed to support new GetFile callback spec.
 *
 * 2004-December-24   Jason Rohrer
 * Updated to match new GetFile spec.
 */



#include "MUTE/layers/messageRouting/messageRouter.h"

#include "MUTE/otherApps/fileSharing/fileShare.h"



#include "minorGems/util/SettingsManager.h"
#include "minorGems/util/SimpleVector.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/log/AppLog.h"

#include "minorGems/util/printUtils.h"
#include "minorGems/system/Time.h"
#include "minorGems/system/Thread.h"
#include "minorGems/system/MutexLock.h"
#include "minorGems/system/Semaphore.h"

#include "minorGems/io/file/File.h"
#include "minorGems/io/file/Path.h"

#include <stdio.h>



char *readUserString( char *inPrompt ) {

    int numRead = 0;

    char *buffer = new char[1000];
    
    while( numRead != 1 ) {
        printf( inPrompt );
        fflush( stdout );
        
        numRead = scanf( "%999s", buffer );
        }
    return buffer;
    }



char *readUserLine( char *inPrompt ) {
    printf( inPrompt );
    fflush( stdout );
    
    SimpleVector<char> *readChars = new SimpleVector<char>();


    
    int readInt = getc( stdin );

    // first, skip starting newlines
    while( readInt == '\n' || readInt == '\r' ) {
        readInt = getc( stdin );
        }
    
    while( readInt != '\n' && readInt != '\r' && readInt != EOF ) {
        readChars->push_back( (char)readInt );

        readInt = getc( stdin );
        }

    char *readLine = readChars->getElementString();

    delete readChars;

    return readLine;
    }



int readUserInt( char *inPrompt ) {

    int readInt;
    int numRead = 0;
    
    while( numRead != 1 ) {
        printf( inPrompt );
        fflush( stdout );
        
        numRead = scanf( "%d", &readInt );

        if( numRead != 1 ) {
            // the next character is not an int, so we failed to scanf it
            // skip the line
            char *line = readUserLine( "" );
            delete [] line;
            printf( "\n" );
            }
        }
    return readInt;
    }



double readUserDouble( char *inPrompt ) {

    double readDouble;
    int numRead = 0;
    
    while( numRead != 1 ) {
        printf( inPrompt );
        fflush( stdout );
        
        numRead = scanf( "%lf", &readDouble );
        if( numRead != 1 ) {
            // the next character is not a double, so we failed to scanf it
            // skip the line
            char *line = readUserLine( "" );
            delete [] line;
            printf( "\n" );
            }
        }
    return readDouble;
    }


MutexLock *searchLock;
char searchActive;
char searchCanceled;

MutexLock *printMessageLock;

MutexLock *resultLock;
char resultsExist;
SimpleVector<char *> *resultFromAddresses;
SimpleVector<char *> *resultFilePaths;
SimpleVector<char *> *resultFileHashes;

int resultCount;





// callback for incoming search results
char searchResultHandler( char *inFromAddress, char *inFilePath,
                          unsigned long inFileLength,
                          char *inFileHash,
                          void *inExtraParam ) {

    if( inFromAddress != NULL ) {
        resultLock->lock();

        resultFromAddresses->push_back( stringDuplicate( inFromAddress ) );
        resultFilePaths->push_back( stringDuplicate( inFilePath ) );
        resultFileHashes->push_back( stringDuplicate( inFileHash ) );
        
        printMessageLock->lock();

        float kiByteSize = inFileLength / 1024.0;
        
        printf( "#%5d   %s\n"
                "         From: %s\n"
                "         Size: %f KiB\n"
                "         Hash: %s\n",
                resultCount,
                inFilePath, inFromAddress, kiByteSize, inFileHash );
        printMessageLock->unlock();

        resultCount++;
        resultsExist = true;

        resultLock->unlock();
        }

    char returnValue = true;
        
    searchLock->lock();

    if( searchCanceled ) {
        returnValue = false;
        }
    searchLock->unlock();

    return returnValue;
    }


class SearchThread : public Thread {

    public:
        // constructs and starts a search thread
        // inSearchString must be destroyed by caller
        SearchThread( char *inSearchString )
            : mSearchString( stringDuplicate( inSearchString ) ) {
            start();
            }

        // joins and destroys this search thread
        ~SearchThread() {
            join();

            delete [] mSearchString;
            }

        // implements Thread interface
        void run() {
            searchLock->lock();
            searchActive = true;
            searchLock->unlock();

            muteShareSearch( mSearchString, searchResultHandler, NULL,
                             1000 );

            printMessageLock->lock();

            printMessageLock->unlock();
            
            searchLock->lock();
            searchActive = false;
            searchLock->unlock();
            }
        
    private:
        char *mSearchString;

    };


SearchThread *searchThread;


MutexLock *downloadLock;
char downloadActive;
char downloadCanceled;
int downloadSize;
int downloadedSoFar;

File *downloadFile;
FILE *downloadFILE;


// callback for incoming files
char fileHandler( unsigned char *inChunk,
                  int inChunkLengthInBytes,
                  void *inExtraParam ) {

    // extra param is file stream
    FILE *outputFile = (FILE *)inExtraParam;

    if( inChunk != NULL ) {
        // we have chunk data
        fwrite( inChunk, 1, inChunkLengthInBytes, outputFile );

        // progress bar
        printf( "-" );
        fflush( stdout );
        }
    else {
        // this chunk timed out and will be retried... no chunk data yet

        // indicate this visually with a drop in the progress bar
        printf( "." );
        fflush( stdout );
        }
        
    downloadLock->lock();
    
    downloadedSoFar += inChunkLengthInBytes;
    
    char keepGoing = ! downloadCanceled;
    downloadLock->unlock();
    
    return keepGoing;
    }



class DownloadThread : public Thread {

    public:
        // constructs and starts a cancel thread
        DownloadThread( char *inFromAddress, char *inFilePath,
                        char *inFileHash )
            : mFromAddress( stringDuplicate( inFromAddress ) ),
              mFilePath( stringDuplicate( inFilePath ) ),
              mFileHash( stringDuplicate( inFileHash ) ) {
            
            start();
            }

        // joins, and destroys this search thread
        ~DownloadThread() {
            join();

            delete [] mFromAddress;
            delete [] mFilePath;
            delete [] mFileHash;
            }

        // implements Thread interface
        void run() {
            
            
            downloadLock->lock();
            downloadActive = true;
            downloadedSoFar = 0;
            downloadLock->unlock();



            unsigned long seconds;
            unsigned long ms;
            Time::getCurrentTime( &seconds, &ms );
                    
            int downloadID;
            
            int result = muteShareGetFile(
                mFromAddress,
                mFilePath,
                mFileHash,
                fileHandler,
                (void *)downloadFILE,
                &downloadID,
                &downloadSize );

            unsigned long deltaMS
                = Time::getMillisecondsSince( seconds, ms );

            downloadLock->lock();
            fclose( downloadFILE );
                            
            if( result == MUTE_SHARE_FILE_TRANSFER_COMPLETE ) {
                

                // move file into main directory
                char *partialFileName = downloadFile->getFileName();

                char *sharePathString = muteShareGetSharingPath();
                File *shareDirectory = new File( NULL, sharePathString );
                delete [] sharePathString;
        
                File *finalDownloadFile =
                    shareDirectory->getChildFile( partialFileName );

                delete shareDirectory;
                delete [] partialFileName;

        
                if( finalDownloadFile != NULL ) {
                    // move
                    downloadFile->copy( finalDownloadFile );
                    downloadFile->remove();
                    
                    delete finalDownloadFile;
                    }

                float rate = downloadSize / ( (float)deltaMS / 1000 );
                rate = rate / 1024;

                printf( "\nFile transfer complete in %d ms [%.2f KiB/s]\n",
                        (int)deltaMS, rate );
                }
            else if( result ==
                     MUTE_SHARE_FILE_TRANSFER_HASH_MISMATCH ) {
                printf( "\nFile transfer corrupted\n" );
                
                if( downloadFile->exists() ) {
                    // remove partial file
                    downloadFile->remove();
                    }
                }
            else if( result ==
                     MUTE_SHARE_FILE_TRANSFER_FAILED ) {
                printf( "\nFile transfer failed\n" );
                
                if( downloadFile->exists() ) {
                    // remove partial file
                    downloadFile->remove();
                    }
                }
            else if( result ==
                     MUTE_SHARE_FILE_TRANSFER_CANCELED ) {
                printf( "\nFile transfer canceled\n" );
                if( downloadFile->exists() ) {
                    // remove partial file
                    downloadFile->remove();
                    }
                }
            else if( result ==
                     MUTE_SHARE_FILE_NOT_FOUND ) {
                printf( "\nFile not found\n" );
                if( downloadFile->exists() ) {
                    // remove partial file
                    downloadFile->remove();
                    }
                }
            
            delete downloadFile;
            downloadActive = false;
            downloadLock->unlock();
            }
        
    private:
        char *mFromAddress;
        char *mFilePath;
        char *mFileHash;
    };


DownloadThread *downloadThread = NULL;



int main() {

    printf( "textFileSharingMUTE\n"
            "a text-based user interface for MUTE file sharing\n\n" );

    printf( "http://mute-net.sourceforge.net\n\n" );


    // check if firewall setting exists
    char valueFound;
    SettingsManager::getIntSetting( "behindFirewall",
                                    &valueFound );
    if( ! valueFound ) {
        // ask user about firewall status
        char *answer =
            readUserString( "Are you behind a firewall that might\n"
                            "block inbound connections to MUTE [yes/no]? " );
        
        // save the firewall setting
        if( stringLocateIgnoreCase( answer, "y" ) != NULL ) {
            SettingsManager::setSetting( "behindFirewall", 1 );
            }
        else {
            SettingsManager::setSetting( "behindFirewall", 0 );
            }
        
        delete [] answer;
        }

    
    
    // check for a random seed saved last time
    char *randomSeed = SettingsManager::getStringSetting( "randomSeed" );
    
    if( randomSeed == NULL ) {
        randomSeed = readUserString( "Enter some randomness: " );
        }
    else {
        printf( "Using randomness saved from last time\n" );
        }

    muteSeedRandomGenerator( randomSeed );
    delete [] randomSeed;

    
    // we've used this seed, so mark it as blank
    // (will be NULL if getStringSetting called again) 
    SettingsManager::setSetting( "randomSeed", "" );


    
    int portNumber = SettingsManager::getIntSetting( "port", &valueFound );

    if( ! valueFound ) {

        portNumber = readUserInt( "Enter port number to listen on: " );

        SettingsManager::setSetting( "port", portNumber );
        }

    
    printf( "Listening for MUTE connections on port %d\n", portNumber );
    muteStart( portNumber );
        
    muteShareStart();

    
    char *logLevelString = SettingsManager::getStringSetting( "logLevel" );

    if( logLevelString != NULL ) {
        // change the default log level

        int newLogLevel = -1;
        
        if( strcmp( logLevelString, "DEACTIVATE_LEVEL" ) == 0 ) {
            newLogLevel = Log::DEACTIVATE_LEVEL;
            }
        else if( strcmp( logLevelString, "CRITICAL_ERROR_LEVEL" ) == 0 ) {
            newLogLevel = Log::CRITICAL_ERROR_LEVEL;
            }
        else if( strcmp( logLevelString, "ERROR_LEVEL" ) == 0 ) {
            newLogLevel = Log::ERROR_LEVEL;
            }
        else if( strcmp( logLevelString, "WARNING_LEVEL" ) == 0 ) {
            newLogLevel = Log::WARNING_LEVEL;
            }
        else if( strcmp( logLevelString, "INFO_LEVEL" ) == 0 ) {
            newLogLevel = Log::INFO_LEVEL;
            }
        else if( strcmp( logLevelString, "DETAIL_LEVEL" ) == 0 ) {
            newLogLevel = Log::DETAIL_LEVEL;
            }
        else if( strcmp( logLevelString, "TRACE_LEVEL" ) == 0 ) {
            newLogLevel = Log::TRACE_LEVEL;
            }

        if( newLogLevel != -1 ) {
            AppLog::setLoggingLevel( newLogLevel );
            }

        delete [] logLevelString;
        }

    
    searchLock = new MutexLock();
    searchActive = false;
    searchCanceled = false;
    searchThread = NULL;
    
    resultLock = new MutexLock();

    resultsExist = false;
    resultFromAddresses = new SimpleVector<char *>();
    resultFilePaths = new SimpleVector<char *>();
    resultFileHashes = new SimpleVector<char *>();
    resultCount = 0;
    
    printMessageLock = new MutexLock();


    downloadLock = new MutexLock();
    downloadActive = false;
    downloadCanceled = false;

    printf( "Enter ? to show a list of available commands\n" );
    
    char quitting = false;
    char *readCharBuffer = new char[2];
    // print the prompt 
    printf( "> " );
    fflush( stdout );

    while( !quitting ) {


        scanf( "%1s", readCharBuffer );

        switch( readCharBuffer[0] ) {
            case '?':
                printf( "Commands:\n" );
                
                printf( "  MUTE node management:\n" );
                printf( "    l -- set message rate limits\n" );
                printf( "    c -- print active connection list\n" );
                printf( "    h -- add host to pool of possible "
                        "connections\n" );

                printf( "  Searching:\n" );
                printf( "    s -- search network for file\n" );
                printf( "    d -- download a file from recent search\n" );
                
                printf( "  General:\n" );
                printf( "    ? -- print this help message\n" );
                printf( "    q -- quit\n" );
                break;
            case 'l':
            case 'L': {
                double outboundLimit = muteGetOutboundMessagePerSecondLimit();
                double inboundLimit = muteGetInboundMessagePerSecondLimit();

                if( outboundLimit != -1 ) {
                    printf( "Outbound limit:  %f messages/second\n",
                            outboundLimit );
                    }
                else {
                    printf( "Outbound limit:  -1 (unlimited)\n" );
                    }
                if( inboundLimit != -1 ) {
                    printf( "Inbound limit:  %f messages/second\n",
                            inboundLimit );
                    }
                else {
                    printf( "Inbound limit:  -1 (unlimited)\n" );
                    }

                printf( "\n" );
                
                outboundLimit = readUserDouble(
                    "Enter outbound limit in messages/second"
                    " (-1 = no limit): " );
                inboundLimit = readUserDouble(
                    "Enter inbound limit in messages/second"
                    " (-1 = no limit): " );

                if( outboundLimit < 0 ) {
                    outboundLimit = -1;
                    }
                if( inboundLimit < 0 ) {
                    inboundLimit = -1;
                    }
                
                muteSetOutboundMessagePerSecondLimit( outboundLimit );
                muteSetInboundMessagePerSecondLimit( inboundLimit );
                
                break;
                }
            case 'c':
            case 'C': {
                char **addresses;
                int *ports;
                int *sentCounts;
                int *queuedCounts;
                int *droppedCounts;
                
                int connectionCount =
                    muteGetConnectedHostList( &addresses,
                                              &ports,
                                              &sentCounts,
                                              &queuedCounts,
                                              &droppedCounts );
                if( connectionCount > 0 ) {
                    printf( "Host:                     "
                            "Sent: "
                            "Queued: "
                            "Dropped:\n" );
                    for( int i=0; i<connectionCount; i++ ) {
                        printf( "%15s : %5d %7d %7d  %7d\n",
                                addresses[i], ports[i], sentCounts[i],
                                queuedCounts[i], droppedCounts[i] );
                        delete [] addresses[i];
                        }
                    }
                else {
                    printf( "No connections\n" );
                    }
                delete [] addresses;
                delete [] ports;
                delete [] sentCounts;
                delete [] queuedCounts;
                delete [] droppedCounts;
                break;
                }
            case 'h':
            case 'H': {
                char *address = readUserString( "Enter IP address: " );
                int port = readUserInt( "Enter port number: " );

                muteAddHost( address, port );

                printf( "Host added\n" );
                delete [] address;
                break;
                }
            case 's':
            case 'S': {
                searchLock->lock();
                if( searchActive ) {
                    printf( "Canceling active search...\n" );
                    // cancel the search
                    searchCanceled = true;
                    searchLock->unlock();
                    
                    // wait for the thread to return
                    delete searchThread;
                    }
                else {
                    searchLock->unlock();
                    }
                searchCanceled = false;
                    
                // delete old results, if there are any
                resultLock->lock();
                if( resultsExist ) {
                    int numResults = resultFromAddresses->size();
                    for( int i=0; i<numResults; i++ ) {
                        delete []
                            *( resultFromAddresses->getElement( i ) );
                        delete [] *( resultFilePaths->getElement( i ) );
                        delete [] *( resultFileHashes->getElement( i ) );
                        }
                    resultFromAddresses->deleteAll();
                    resultFilePaths->deleteAll();
                    resultFileHashes->deleteAll();
                    resultCount = 0;
                    }
                resultsExist = false;
                resultCount = 0;
                resultLock->unlock();
                
                char *searchString =
                    readUserLine( "Enter search string: " );
                
                // spawn thread for search
                searchThread = new SearchThread( searchString );
                delete [] searchString;
                
                break;
                }
            case 'd':
            case 'D': {
                downloadLock->lock();

                if( downloadActive ) {
                    // cancel active download
                    downloadCanceled = true;

                    downloadLock->unlock();
                    
                    // wait for download thread to return
                    delete downloadThread;
                    downloadThread = NULL;
                    }
                else {
                    if( downloadThread != NULL ) {
                        // a lingering DL thread exists
                        delete downloadThread;
                        downloadThread = NULL;
                        }
                    
                    // start a new download
                    downloadLock->unlock();
                    
                    resultLock->lock();

                    if( resultsExist ) {
                        int itemNumber =
                            readUserInt( "Enter search result number: # " );
                        
                        if( itemNumber < resultCount ) {
                            char *fromAddress = stringDuplicate(
                                *( resultFromAddresses->getElement(
                                    itemNumber ) ) );
                            char *filePath = stringDuplicate(
                                *( resultFilePaths->getElement(
                                    itemNumber ) ) );
                            char *fileHash = stringDuplicate(
                                *( resultFileHashes->getElement(
                                    itemNumber ) ) );
                            
                            char *sharePathString = muteShareGetSharingPath();
                            
                            printf( "Downloading %s from %s into folder %s\n",
                                    filePath, fromAddress, sharePathString );

                            char *lastSlash = filePath;
                            char *nextSlash = filePath;
                            while( nextSlash != NULL ) {
                                // skip slash
                                if( nextSlash[0] == '/' ) {
                                    lastSlash = &( nextSlash[1] );
                                    }
                                
                                nextSlash = strstr( lastSlash, "/" );
                                }
                            
                            char *localFileName = lastSlash;




                            File *shareDirectory = new File( NULL,
                                                             sharePathString );
                                                    
                            File *incomingDirectory =
                                shareDirectory->getChildFile(
                                    "MUTE_incoming" );
                            delete shareDirectory;

    
                            if( incomingDirectory != NULL && !
                                incomingDirectory->exists() ) {
                                incomingDirectory->makeDirectory();
                                }

                            downloadFILE = NULL;
                            
                            if( incomingDirectory != NULL ) {
                                downloadFile =
                                    incomingDirectory->getChildFile(
                                        localFileName );
    

                                if( downloadFile != NULL ) {
                                    char *fullFileName =
                                        downloadFile->getFullFileName();
                        
                                    downloadFILE = fopen( fullFileName,
                                                          "wb" );
                                    
                                    delete [] fullFileName;
                                    }
                                delete incomingDirectory;
                                }

               
                            if( downloadFILE == NULL ) {
                                printf( "Failed to open file %s for writing"
                                        " in incoming folder\n",
                                        localFileName );
                            }
                            else {
                                downloadLock->lock();
                                downloadCanceled = false;
                                downloadLock->unlock();

                                
                                // spawn a thread for this download
                                printf(
                                    "Enter d again to stop the download\n" );
                                downloadThread = new DownloadThread(
                                    fromAddress, filePath, fileHash );
                                
                                }
                            
                            delete [] fromAddress;
                            delete [] filePath;
                            delete [] fileHash;
                            delete [] sharePathString;
                            }
                    
                        }

                    resultLock->unlock();
                    }
                break;
                }
            case 'q':
            case 'Q':
                quitting = true;
                break;
            default:
                printf( "Unknown command: %c\n", readCharBuffer[0] );
                break;
            }

        // print the next prompt 
        printf( "> " );
        fflush( stdout );
        }
    delete [] readCharBuffer;

    threadPrintF( "Qutting...\n" );

    
    // cancel any active download
    downloadLock->lock();

    if( downloadActive ) {
        // cancel active download
        downloadCanceled = true;
        
        downloadLock->unlock();
        
        // wait for download thread to return
        delete downloadThread;
        downloadThread = NULL;
        }
    else {
        if( downloadThread != NULL ) {
            // a lingering DL thread exists
            delete downloadThread;
            downloadThread = NULL;
            }
        downloadLock->unlock();
        }
    delete downloadLock;

    
    // cancel any active search
    searchLock->lock();
    if( searchActive ) {
        searchCanceled = true;
        searchLock->unlock();

        // wait for search thread to return
        delete searchThread;
        }
    else {
        searchLock->unlock();
        }

    delete searchLock;

    // delete last results
    resultLock->lock();
    if( resultsExist ) {
        int numResults = resultFromAddresses->size();
        for( int i=0; i<numResults; i++ ) {
            delete [] *( resultFromAddresses->getElement( i ) );
            delete [] *( resultFilePaths->getElement( i ) );
            delete [] *( resultFileHashes->getElement( i ) );
            }
        resultFromAddresses->deleteAll();
        resultFilePaths->deleteAll();
        resultFileHashes->deleteAll();
        resultCount = 0;
        resultsExist = false;
        }
    resultLock->unlock();

    
    delete resultLock;

    delete resultFromAddresses;
    delete resultFilePaths;
    delete resultFileHashes;
    
    delete printMessageLock;


    

    threadPrintF( "Stopping file sharing layer\n" );
    muteShareStop();

    

    threadPrintF( "Saving randomness for use at next startup\n" );
    char *randState = muteGetRandomGeneratorState();

    SettingsManager::setSetting( "randomSeed", randState );
    delete [] randState;

    threadPrintF( "Stopping message routing layer\n" );
    muteStop();


    threadPrintF( "All layers are stopped, exiting.\n" );
    }
