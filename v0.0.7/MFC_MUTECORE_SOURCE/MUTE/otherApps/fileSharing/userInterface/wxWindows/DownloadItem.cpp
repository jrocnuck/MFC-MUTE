/*
 * Modification History
 *
 * 2003-October-27   Jason Rohrer
 * Created.
 *
 * 2003-October-28   Jason Rohrer
 * Fixed a deletion bug.
 * Added support for download item deletion.
 * Added better download stats.
 * Improved layout.
 *
 * 2003-November-2   Jason Rohrer
 * Switched to MS time resolution for download rate.
 *
 * 2003-November-6   Jason Rohrer
 * Switched to common formatUtils for size formatting.
 * Worked on deadlock when item destroyed while download in progress.
 *
 * 2003-November-7   Jason Rohrer
 * Worked more on quit-while-in-progress bugs.
 *
 * 2003-November-9   Jason Rohrer
 * Added function that checks active download status.
 * Removed excess print statements.
 *
 * 2003-November-11   Jason Rohrer
 * Added b flag to fopen of binary files.
 *
 * 2003-December-19   Jason Rohrer
 * Added check for local file open failing on download.
 *
 * 2003-December-22   Jason Rohrer
 * Changed to save incoming files temporarily in a separate directory.
 *
 * 2003-December-23   Jason Rohrer
 * Added UI support for hash mismatch.
 *
 * 2004-January-26   Jason Rohrer
 * Added support for download timeout setting.
 *
 * 2004-February-6   Jason Rohrer
 * Added Marius Sturm's patch for displaying time remaining.
 *
 * 2004-February-12   Jason Rohrer
 * Changed to use average transfer rate to compute ETA.
 *
 * 2004-February-16   Jason Rohrer
 * Fixed an integer rounding bug in ETA computation.
 *
 * 2004-February-21   Jason Rohrer
 * Changed to support new GetFile callback spec.
 *
 * 2004-February-22   Jason Rohrer
 * Added route quality gauge.
 *
 * 2004-February-26   Jason Rohrer
 * Fixed bugs in route quality guage, and changed starting point to 0.
 * Fixed bug in download rate when we hit a retry.
 * Fixed bugs in layout.
 *
 * 2004-March-8   Jason Rohrer
 * Fixed bugs in ETA when we haven't received any chunks yet.
 * Improved status displays.
 *
 * 2004-April-2   Jason Rohrer
 * Fixed potential deadlock.
 * Fixed layout of route quality bar.
 * Removed use of wxMutexGuiEnter, which was causing freezes on win32.
 *
 * 2004-December-8   Jason Rohrer
 * Added some support for resumable downloads.
 *
 * 2004-December-10   Jason Rohrer
 * Fixed bug in speed computation when a download is resumed.
 *
 * 2004-December-20   Jason Rohrer
 * Added check for downloads that are already in progress.
 */



#include "DownloadItem.h"

#include "MUTE/otherApps/fileSharing/userInterface/common/formatUtils.h"

#include "minorGems/util/SettingsManager.h"



// includes and definitions copied from wxWindows sample calendar app


// for all others, include the necessary headers
#ifndef WX_PRECOMP
//    #include "wx/stattext.h"
    #include "wx/app.h"
    #include "wx/thread.h"
    #include "wx/statline.h"
#endif


#include "minorGems/util/stringUtils.h"
#include "minorGems/util/TranslationManager.h"
#include "minorGems/io/file/Path.h"
#include "minorGems/system/Time.h"

#include "MUTE/otherApps/fileSharing/fileShare.h"

#include <time.h>



// IDs for various controls and event generators
enum {
    DOWNLOAD_PROGRESS_EVENT = 1,
    DOWNLOAD_RESULT_EVENT,
    DOWNLOAD_CHUNK_RETRY_EVENT,
    DOWNLOAD_CHUNK_RECEIVED_EVENT,
    BUTTON_CANCEL_CLEAR
    };



// event mapping table
BEGIN_EVENT_TABLE( DownloadItem, wxPanel )
    EVT_BUTTON( BUTTON_CANCEL_CLEAR,  DownloadItem::OnCancelClear )
    // dummy events posted by file handler to inform us of
    // download progress and other GUI updates in a thread-safe manner
    EVT_MENU( DOWNLOAD_PROGRESS_EVENT, DownloadItem::OnDownloadProgress )
    EVT_MENU( DOWNLOAD_RESULT_EVENT, DownloadItem::OnDownloadResult )
    EVT_MENU( DOWNLOAD_CHUNK_RETRY_EVENT, DownloadItem::OnChunkRetry )
    EVT_MENU( DOWNLOAD_CHUNK_RECEIVED_EVENT, DownloadItem::OnChunkReceived )
END_EVENT_TABLE();



// callback for incoming files
char fileHandler( unsigned char *inChunk,
                  int inChunkLengthInBytes,
                  void *inExtraParam ) {

    // extra param is DownloadItem
    DownloadItem *item = (DownloadItem *)inExtraParam;

    char keepGoing = !( item->isCanceled() );
    
    if( keepGoing ) {
        // only send chunk event if we haven't been canceled

        if( inChunk != NULL ) {
            // we have been passed chunk data
            item->processChunk( inChunk, inChunkLengthInBytes );
            }
        else {
            // else chunk timed out, and API will retry

            // tell item about the retry
            item->processChunk( NULL, -1 );
            }
                
        }
        
    return keepGoing;
    }



DownloadThread::DownloadThread( char *inFromAddress, char *inFilePath,
                                char *inFileHash,
                                int *inDownloadSizePointer,
                                void *inDownloadItem,
                                char *inPartialFileName,
                                int inNumPartialBytes )
    : mFromAddress( stringDuplicate( inFromAddress ) ),
      mFilePath( stringDuplicate( inFilePath ) ),
      mFileHash( stringDuplicate( inFileHash ) ),
      mDownloadSizePointer( inDownloadSizePointer ),
      mDownloadItem( inDownloadItem ),
      mPartialFileName( inPartialFileName ),
      mNumPartialBytes( inNumPartialBytes ),
      mStopLock( new MutexLock() ),
      mStopped( false ) {

    start();
    }



DownloadThread::~DownloadThread() {
    // tell thread to stop (so it won't send its last event after the Yield)
    mStopLock->lock();
    mStopped = true;
    mStopLock->unlock();
    
    join();

    delete [] mFromAddress;
    delete [] mFilePath;
    delete [] mFileHash;

    if( mPartialFileName != NULL ) {
        delete [] mPartialFileName;
        }
    
    delete mStopLock;
    }



void DownloadThread::run() {

    DownloadItem *item = (DownloadItem *)mDownloadItem;

    // get the timeout setting
    char found;
    int downloadTimeoutMilliseconds =
        SettingsManager::getIntSetting( "downloadTimeoutMilliseconds",
                                        &found );
    if( !found || downloadTimeoutMilliseconds < 0 ) {
        // default to 60 seconds
        downloadTimeoutMilliseconds = 60000;
        }

    
    // start the file transfer
    int result = muteShareGetFile(
        mFromAddress,
        mFilePath,
        mFileHash,
        fileHandler,
        mDownloadItem,  // pass download item as handler arg
        &( item->mDownloadID ),
        mDownloadSizePointer,
        downloadTimeoutMilliseconds,
        mPartialFileName,
        mNumPartialBytes );
    
    mStopLock->lock();
    char stopped = mStopped;
    mStopLock->unlock();

    if( !stopped ) {
        // pass result back to parent item
        item->processDownloadResult( result );
        }

    // don't post event if stopped (avoid deadlock)
    }



DownloadItem::DownloadItem( wxWindow *inParent,
                            wxBoxSizer *inParentSizer,
                            char *inFromAddress, char *inFilePath,
                            char *inFileHash )
    : wxPanel( inParent, -1 ),
      mParentWindow( inParent ),
      mParentSizer( inParentSizer ),
      mDownloadStatusLock( new MutexLock() ),
      mDownloadActive( true ),
      mDownloadSizeInBytes( -1 ),
      mDownloadedSoFarInBytes( 0 ),
      mCurrentBlockStartTimeSeconds( 0 ),
      mCurrentBlockStartTimeMilliseconds( 0 ),
      mCurrentBlockStartSize( 0 ),
      mCurrentRate( 0 ),
      mCanceled( false ),
      mCleared( false ),
      mDownloadInfoFile( NULL ),
      mHashMapFile( NULL ),
      mFileHash( stringDuplicate( inFileHash ) ) {
    
    Time::getCurrentTime( &mDownloadStartTimeSeconds,
                          &mDownloadStartTimeMilliseconds );

    mCurrentBlockStartTimeSeconds = mDownloadStartTimeSeconds;
    mCurrentBlockStartTimeMilliseconds = mDownloadStartTimeMilliseconds;
    
    
    // stick before top slot (0) in download list
    mParentSizer->Insert( 0,
                          this,
                          0,
                          wxEXPAND | wxADJUST_MINSIZE,
                          0 );

    mPanelSizer = new wxBoxSizer( wxVERTICAL );

    SetSizer( mPanelSizer );


    // sizer to lay file name and cancel/clear button out on same line
    wxBoxSizer *fileNameAndButtonSizer = new wxBoxSizer( wxHORIZONTAL );

    // 10-pixel border on left and right of all lines
    // (look for  wxLEFT | wxRIGHT below)
    
    // 10-pixel border above first line
    mPanelSizer->Add( fileNameAndButtonSizer,
                      0,
                      wxEXPAND | wxTOP | wxLEFT | wxRIGHT,
                      10 );
    
    fileNameAndButtonSizer->Add( new wxStaticText( this, -1, inFilePath ),
                                 1,
                                 wxALIGN_CENTER,
                                 0 );

    mCancelClearButton = new wxButton(
        this, BUTTON_CANCEL_CLEAR,
        TranslationManager::translate( "cancel_download_button" ) );

    // separate sizer for button so that it can be right-aligned
    // and not expanded
    wxBoxSizer *buttonSizer = new wxBoxSizer( wxVERTICAL );
    buttonSizer->Add( mCancelClearButton,
                      0,
                      wxALIGN_RIGHT | wxADJUST_MINSIZE,
                      0 );
    
    fileNameAndButtonSizer->Add( buttonSizer,
                                 0,
                                 wxALIGN_CENTER,
                                 0 );

    
    mProgressGauge = new wxGauge( this, -1, 100,
                                  wxDefaultPosition, wxDefaultSize,
                                  wxGA_HORIZONTAL | wxGA_SMOOTH );
    mPanelSizer->Add( mProgressGauge,
                      0,
                      wxEXPAND | wxLEFT | wxRIGHT,
                      10 );

    
    
    mStatusLabel = new wxStaticText(
        this, -1,
        TranslationManager::translate( "file_fetching_info_message" ) );
    mPanelSizer->Add( mStatusLabel,
                      0,
                      wxALIGN_CENTER | wxADJUST_MINSIZE | wxLEFT | wxRIGHT,
                      10 );


    // sizer to lay host address and route quality bar out on same line
    wxBoxSizer *addressAndQualitySizer = new wxBoxSizer( wxHORIZONTAL );
    
    // 10-pixel border between last line and horizontal rule separator
    mPanelSizer->Add( addressAndQualitySizer,
                      0,
                      wxEXPAND | wxLEFT | wxRIGHT,
                      10 );

    
    

    
    char *addressString = autoSprintf(
        "%s  %s",
        TranslationManager::translate( "file_host_address_label" ),
        inFromAddress );
    addressAndQualitySizer->Add( new wxStaticText( this, -1, addressString ),
                                 0,
                                 wxALIGN_CENTER,
                                 0 );
    delete [] addressString;

    
    mRouteQualityGauge = new wxGauge( this, -1, 20,
                                      wxDefaultPosition, wxDefaultSize,
                                      wxGA_HORIZONTAL | wxGA_SMOOTH );
    // start in middle so that failed fileInfo requests can decrease gauge
    mRouteQualityGauge->SetValue( 10 );
    
    addressAndQualitySizer->Add(
        new wxStaticText(
            this, -1,
            TranslationManager::translate( "file_route_quality_label" ) ),
        0,
        wxALIGN_CENTER | wxLEFT,  // 20 pixel border on left
        20 );
    addressAndQualitySizer->Add( mRouteQualityGauge,
                                 1,
                                 wxALIGN_CENTER,
                                 0 );
    

    // finally, add a line as a divider
    // no space on left or right (goes to edge of panel)
    mPanelSizer->Add( new wxStaticLine( this, -1 ),
                      0,
                      wxEXPAND,
                      0 );


    // end of GUI layout
    
    
    // save the download to a file
                            
    
    char *lastSlash = inFilePath;
    char *nextSlash = inFilePath;
    while( nextSlash != NULL ) {
        // skip slash
        if( nextSlash[0] == '/' ) {
            lastSlash = &( nextSlash[1] );
            }
        
        nextSlash = strstr( lastSlash, "/" );
        }
    
    char *localFileName = lastSlash;


    char *sharePathString = muteShareGetSharingPath();
    File *shareDirectory = new File( NULL, sharePathString );
    delete [] sharePathString;

    File *incomingDirectory = shareDirectory->getChildFile( "MUTE_incoming" );
    delete shareDirectory;

    
    if( incomingDirectory != NULL && ! incomingDirectory->exists() ) {
        incomingDirectory->makeDirectory();
        }

    char fileOpenedForWriting = false;
    
    if( incomingDirectory != NULL ) {


        mDownloadFile = incomingDirectory->getChildFile( localFileName );

        char *localInfoFileName = autoSprintf( "%s.info", localFileName );
        mDownloadInfoFile =
            incomingDirectory->getChildFile( localInfoFileName );
        delete [] localInfoFileName;

        
        if( mDownloadFile != NULL ) {

            // look for an existing hash match for a partial file.  Thus,
            // we will resume files with identical contents even if their
            // names do not match.
            
            // hash map files are stored in files with names like SHA1.name,
            // where SHA1 is an SHA1 hash value.
            // Hash map files contain the name of the partial file that
            // corresponds to the hash.
            char *hashMapFileName = autoSprintf( "%s.name", mFileHash );
            
            mHashMapFile =
                incomingDirectory->getChildFile( hashMapFileName );
            delete [] hashMapFileName;

            if( mHashMapFile != NULL && mHashMapFile->exists() ) {
                // use the hash map to determine the name of an existing
                // partial file

                char *existingFileName = mHashMapFile->readFileContents();

                if( existingFileName != NULL ) {
                    if( strcmp( existingFileName, localFileName ) != 0 ) {
                        // existing file has a different name than the current
                        // download

                        // rename the existing file to match our current
                        // file name
                        File *existingFile = incomingDirectory->getChildFile(
                            existingFileName );
                        if( existingFile != NULL && existingFile->exists() ) {
                            existingFile->copy( mDownloadFile );
                            existingFile->remove();
                            }
                        if( existingFile != NULL ) {
                            delete existingFile;
                            }

                        // update the hash map
                        mHashMapFile->writeToFile( localFileName );


                        // rename the info file
                        if( mDownloadInfoFile != NULL ) {
                            char *existingInfoFileName =
                                autoSprintf( "%s.info", existingFileName );
                            
                            File *existingInfoFile =
                                incomingDirectory->getChildFile(
                                    existingInfoFileName );
                        
                            delete [] existingInfoFileName;

                            if( existingInfoFile != NULL &&
                                existingInfoFile->exists() ) {
                                
                                existingInfoFile->copy( mDownloadInfoFile );
                                existingInfoFile->remove();
                                }
                            if( existingInfoFile != NULL ) {
                                delete existingInfoFile;
                                }
                            }
                            
                        }
                    
                    delete [] existingFileName;
                    }
                }
            if( mHashMapFile != NULL &&
                ! mHashMapFile->exists() ) {
                // create a hash map for this download
                mHashMapFile->writeToFile( localFileName );
                }
                
            // at this point, we have renamed any matching existing partial
            // file (and corresponding meta info files) to match the name of
            // the current download

            // So, we can proceed as if any matching partial file has the
            // same name as the current download.

            char *fullFileName =
                mDownloadFile->getFullFileName();

            mDownloadFILE = NULL;

            char *partialFileName = NULL;
            int numPartialBytes = 0;

            if( mDownloadFile->exists() && mDownloadInfoFile->exists() ) {
                // a valid partial download exists
                
                char *infoFileName = mDownloadInfoFile->getFullFileName();
                FILE *infoFILE = fopen( infoFileName, "r" );

                delete [] infoFileName;
                
                if( infoFILE != NULL ) {
                    fscanf( infoFILE, "%d", &numPartialBytes );

                    fclose( infoFILE );
                    }

                if( numPartialBytes > 0 ) {
                    // preserve the existing file bytes
                    mDownloadFILE = fopen( fullFileName, "r+" );

                    fseek( mDownloadFILE, numPartialBytes, SEEK_SET );

                    
                    partialFileName = stringDuplicate( fullFileName );

                    mDownloadedSoFarInBytes = numPartialBytes;
                    mCurrentBlockStartSize = mDownloadedSoFarInBytes;
                    }
                }

            if( mDownloadFILE == NULL ) {
                // default to overwriting any existing bytes
                mDownloadFILE = fopen( fullFileName, "wb" );
                }

            delete [] fullFileName;


            // make sure that we could open the file for writing
            if( mDownloadFILE != NULL ) {
                
                mDownloadThread = new DownloadThread( inFromAddress,
                                                      inFilePath,
                                                      mFileHash,
                                                      &mDownloadSizeInBytes,
                                                      (void *)this,
                                                      partialFileName,
                                                      numPartialBytes );
                fileOpenedForWriting = true;
                }
            else {
                delete mDownloadFile;
                }        
            }
        delete incomingDirectory;
        }

    
    if( ! fileOpenedForWriting ) {
        mDownloadActive = false;
        mStatusLabel->SetLabel(
            TranslationManager::translate( "file_failed_to_write_message" ) );

        // now let them clear this download
        mCancelClearButton->SetLabel(
            TranslationManager::translate( "clear_download_button" ) );
        mCancelClearButton->Enable( true );
        
        mPanelSizer->Layout();

        mDownloadThread = NULL;
        }
    }



DownloadItem::~DownloadItem() {
    
    mDownloadStatusLock->lock();
    if( mDownloadActive ) {
        // cancel active download
        mCanceled = true;
        }
    mDownloadStatusLock->unlock();    
    
    
    // yield to allow any last event from fileHandler to come through
    wxYield();

    
    // wait for thread to finish and destroy it
    if( mDownloadThread != NULL ) {

        // make sure we unlock GUI mutex before joining thread to avoid
        // a deadlock (thread may be trying to post events)
        if( wxThread::IsMain() ) {
            wxMutexGuiLeave();
            }

        delete mDownloadThread;        
        
        mDownloadThread = NULL;
        }
    
    // clean up a partial download if we were destroyed while
    // the download was still active
    // (only happens when app shuts down mid-download)
    mDownloadStatusLock->lock();
    if( mDownloadActive ) {
        // thread stopped before posting result

        // flush and close file
        fclose( mDownloadFILE );

        if( mDownloadInfoFile == NULL ||
            ! mDownloadInfoFile->exists() ) {

            // no info about partial file
            // delete it
            if( mDownloadFile->exists() ) {
                mDownloadFile->remove();
                }
            }
        delete mDownloadFile;
        }
    
    if( mDownloadInfoFile != NULL ) {
        delete mDownloadInfoFile;
        mDownloadInfoFile = NULL;
        }

    if( mHashMapFile != NULL ) {
        delete mHashMapFile;
        mHashMapFile = NULL;
        }
    
    mDownloadStatusLock->unlock();

    
    delete mDownloadStatusLock;

    delete [] mFileHash;
    }



void DownloadItem::processChunk( unsigned char *inChunk,
                                 int inChunkLengthInBytes ) {

    if( inChunk != NULL ) {
        mDownloadStatusLock->lock();
        
        fwrite( inChunk, 1, inChunkLengthInBytes, mDownloadFILE );
        
        mDownloadedSoFarInBytes += inChunkLengthInBytes;

        if( mDownloadInfoFile != NULL ) {
            // save info about this partial download
            char *infoFileName = mDownloadInfoFile->getFullFileName();

            FILE *infoFILE = fopen( infoFileName, "w" );

            delete [] infoFileName;
            
            if( infoFILE != NULL ) {
                fprintf( infoFILE, "%d", mDownloadedSoFarInBytes );
                fclose( infoFILE );
                }
            }
            
        mDownloadStatusLock->unlock();
        
                
        // post event so that GUI thread can increase quality gauge
        wxCommandEvent eventReceived( wxEVT_COMMAND_MENU_SELECTED,
                                      DOWNLOAD_CHUNK_RECEIVED_EVENT );
        // send in a thread-safe way
        wxPostEvent( this, eventReceived );
        }
    else {
        // event to decrease quality gauge
        wxCommandEvent eventRetry( wxEVT_COMMAND_MENU_SELECTED,
                                   DOWNLOAD_CHUNK_RETRY_EVENT );
        wxPostEvent( this, eventRetry );
        }

    // event to update progress and ETA UI
    wxCommandEvent eventProgress( wxEVT_COMMAND_MENU_SELECTED,
                                  DOWNLOAD_PROGRESS_EVENT );
    
    wxPostEvent( this, eventProgress );
    }



void DownloadItem::processDownloadResult( int inResult ) {
    mDownloadStatusLock->lock();
    
    mDownloadResult = inResult;
    
    mDownloadStatusLock->unlock();

    // post event so that GUI thread can update progress UI
    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED,
                          DOWNLOAD_RESULT_EVENT );

    // send in a thread-safe way
    wxPostEvent( this, event );
    }



char DownloadItem::isCanceled() {
    mDownloadStatusLock->lock();
    char returnValue = mCanceled;
    mDownloadStatusLock->unlock();

    return returnValue;
    }



char DownloadItem::isCleared() {
    mDownloadStatusLock->lock();
    char returnValue = mCleared;
    mDownloadStatusLock->unlock();

    return returnValue;
    }



char DownloadItem::isActive() {
    mDownloadStatusLock->lock();
    char returnValue = mDownloadActive;
    mDownloadStatusLock->unlock();

    return returnValue;
    }



char *DownloadItem::getFileHash() {
    
    return stringDuplicate( mFileHash );
    }



void DownloadItem::OnCancelClear( wxCommandEvent &event ) {
    // disable button until download result is returned
    mCancelClearButton->Enable( false );

    mDownloadStatusLock->lock();
    if( mDownloadActive ) {
        // cancel active download        
        mCanceled = true;
        
        mDownloadStatusLock->unlock();    
        
        mStatusLabel->SetLabel(
            TranslationManager::translate( "file_canceling_message" ) );

        
        // interrupt the download so that it checks for our cancel signal
        // immediately
        muteShareInterruptDownload( mDownloadID );
        }
    else {
        // not active, so clear this item
        mCleared = true;
        mDownloadStatusLock->unlock();    

        mParentSizer->Show( this, false );
        mParentSizer->Layout();

        // scrolling of parent window may need to be adjusted
        mParentWindow->FitInside();
        }
        
    
    mPanelSizer->Layout();
    }



void DownloadItem::OnDownloadProgress( wxCommandEvent& event ) {
    mDownloadStatusLock->lock();
    
    
    float fractionDone =
        (float)mDownloadedSoFarInBytes / (float)mDownloadSizeInBytes;

    int percentDone = (int)( fractionDone * 100 );

    long timeDelta =
        Time::getMillisecondsSince( mCurrentBlockStartTimeSeconds,
                                    mCurrentBlockStartTimeMilliseconds ); 
    if( timeDelta >= 1000 ) {
        // 1 second has passed since last progress update

        int sizeDelta = mDownloadedSoFarInBytes - mCurrentBlockStartSize;

        if( sizeDelta > 0 ) {
            // we have received more data since the last progress update
            
            mCurrentRate = ( (float)sizeDelta / (float)timeDelta ) * 1000;
            
            mCurrentBlockStartSize = mDownloadedSoFarInBytes;
            
            Time::getCurrentTime( &mCurrentBlockStartTimeSeconds,
                                  &mCurrentBlockStartTimeMilliseconds );
            }
        }
        

    char *sizeString = formatDataSizeWithUnits( mDownloadSizeInBytes );
    
    char *rateString = formatDataSizeWithUnits( (unsigned long)mCurrentRate ); 

    
    // compute the average rate over the entire transfer so far
    long totalMillisecondsSoFar =
        Time::getMillisecondsSince( mDownloadStartTimeSeconds,
                                    mDownloadStartTimeMilliseconds );
    double averageBytePerSecondRate =
        ( (double)mDownloadedSoFarInBytes / (double)totalMillisecondsSoFar )
        * 1000;

    char *statusText;
    if( averageBytePerSecondRate > 0 ) { 
        // compute the ETA using the average rate
        long bytesRemaining = mDownloadSizeInBytes - mDownloadedSoFarInBytes;
        double secondsRemaining = bytesRemaining / averageBytePerSecondRate;

        
        char *timeRemainingString =
            formatTimeIntervalWithUnits( secondsRemaining );
    
        statusText = autoSprintf(
            TranslationManager::translate( "file_progress_message" ),
                                  percentDone,
                                  sizeString,
                                  rateString,
                                  timeRemainingString );
        delete [] timeRemainingString;
        }
    else {
        // don't compute or show time remaining, since our rate is 0

        statusText = stringDuplicate( 
            TranslationManager::translate( "file_fetching_info_message" )  );
        }

    delete [] sizeString;
    delete [] rateString;
    
    
    
    mDownloadStatusLock->unlock();

    mStatusLabel->SetLabel( statusText );

    delete [] statusText;

    mProgressGauge->SetValue( percentDone );

    mPanelSizer->Layout();

    // FIXME:  need to update size of parent scroll view here also 
    }



void DownloadItem::OnChunkRetry( wxCommandEvent& event ) {
    int currentQuality = mRouteQualityGauge->GetValue();
    currentQuality--;
    if( currentQuality < 0 ) {
        currentQuality = 0;
        }

    mRouteQualityGauge->SetValue( currentQuality );
    }



void DownloadItem::OnChunkReceived( wxCommandEvent& event ) {
    int currentQuality = mRouteQualityGauge->GetValue();
    currentQuality++;
    if( currentQuality > 20 ) {
        currentQuality = 20;
        }

    mRouteQualityGauge->SetValue( currentQuality );
    }



void DownloadItem::OnDownloadResult( wxCommandEvent& event ) {
    
    mDownloadStatusLock->lock();
    mDownloadActive = false;
    
    int result = mDownloadResult;

    // flush and close file
    fclose( mDownloadFILE );

    mDownloadStatusLock->unlock();
    
    char complete = false;
    
    switch( result ) {
        case MUTE_SHARE_FILE_NOT_FOUND:
            mStatusLabel->SetLabel(
                TranslationManager::translate( "file_not_found_message" ) );
            break;
        case MUTE_SHARE_FILE_TRANSFER_FAILED:
            mStatusLabel->SetLabel( 
                TranslationManager::translate( "file_failed_message" ) );
            break;            
        case MUTE_SHARE_FILE_TRANSFER_CANCELED:
            mStatusLabel->SetLabel( 
                TranslationManager::translate( "file_canceled_message" ) );
            break;            
        case MUTE_SHARE_FILE_TRANSFER_HASH_MISMATCH:
            mStatusLabel->SetLabel( 
                TranslationManager::translate( "file_corrupted_message" ) );
            break;            
        case MUTE_SHARE_FILE_TRANSFER_COMPLETE: {
            char *sizeString = formatDataSizeWithUnits( mDownloadSizeInBytes );
            
            char *statusText = autoSprintf(
                TranslationManager::translate( "file_completed_message" ),
                sizeString);
            delete [] sizeString;
            
            mStatusLabel->SetLabel( statusText );
            delete [] statusText;
            
            complete = true;
            break;
            }
        default:
            mStatusLabel->SetLabel(
                TranslationManager::translate( "file_failed_message" ) );
            break;
        }

    
    mDownloadStatusLock->lock();
    if( !complete ) {
        if( mDownloadInfoFile == NULL ||
            ! mDownloadInfoFile->exists() ) {
            // no info about partial file
            // delete it
            if( mDownloadFile->exists() ) {
                mDownloadFile->remove();
                }
            }
        }
    else {
        // move file into main directory
        char *partialFileName = mDownloadFile->getFileName();

        char *sharePathString = muteShareGetSharingPath();
        File *shareDirectory = new File( NULL, sharePathString );
        delete [] sharePathString;
        
        File *finalDownloadFile =
            shareDirectory->getChildFile( partialFileName );

        delete shareDirectory;
        delete [] partialFileName;

        
        if( finalDownloadFile != NULL ) {
            // move
            mDownloadFile->copy( finalDownloadFile );
            mDownloadFile->remove();

            if( mDownloadInfoFile != NULL ) {
                // do not need info file anymore
                mDownloadInfoFile->remove();
                }

            if( mHashMapFile != NULL ) {
                // do not need hash map anymore
                mHashMapFile->remove();
                }
            
            delete finalDownloadFile;
            }
        }
        
    delete mDownloadFile;

    mDownloadStatusLock->unlock();


    // now let them clear this download
    mCancelClearButton->SetLabel(
        TranslationManager::translate( "clear_download_button" ) );
    mCancelClearButton->Enable( true );

    mPanelSizer->Layout();
    }


