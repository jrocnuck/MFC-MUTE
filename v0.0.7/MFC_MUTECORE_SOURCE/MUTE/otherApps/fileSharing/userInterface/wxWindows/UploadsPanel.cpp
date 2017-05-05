/*
 * Modification History
 *
 * 2003-November-4   Jason Rohrer
 * Created.
 *
 * 2003-November-5   Jason Rohrer
 * Fixed destruction/stopping behavior of update thread.
 *
 * 2004-February-13   Jason Rohrer
 * Re-added upload stats patch submitted by Mycroftxxx.
 * Cleaned up patched code.
 *
 * 2004-April-2   Jason Rohrer
 * Fixed potential deadlock.
 * Removed use of wxMutexGuiEnter, which was causing freezes on win32.
 */



#include "UploadsPanel.h"

#include "MUTE/otherApps/fileSharing/userInterface/common/formatUtils.h"



// for non-precomp compilers, include the necessary headers
#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/thread.h"
//    #include "wx/frame.h"
//    #include "wx/panel.h"
//    #include "wx/stattext.h"
//    #include "wx/menu.h"
//  #include "wx/layout.h"
//    #include "wx/dialog.h"
//    #include "wx/msgdlg.h"
    #include "wx/sizer.h"
//    #include "wx/textdlg.h"
//  #include "wx/progdlg.h"
//    #include "wx/utils.h"
//    #include "wx/choicdlg.h"
//    #include "wx/button.h"
#endif



#include "MUTE/otherApps/fileSharing/fileShare.h"



#include "minorGems/util/stringUtils.h"
#include "minorGems/util/TranslationManager.h"
#include <time.h>
#include <stdio.h>



// IDs for various controls
enum {
    UPLOADS_UPDATE_EVENT = 1
    };



// event mapping table
BEGIN_EVENT_TABLE( UploadsPanel, wxPanel )
    // dummy events sent by thread to trigger an update
    EVT_MENU( UPLOADS_UPDATE_EVENT, UploadsPanel::OnUploadsUpdate )
END_EVENT_TABLE();



UploadsPanel::UploadsPanel( wxNotebook *inNotebook )
    : wxPanel( inNotebook, -1 ),
      mUploadList( new wxListCtrl( this, -1,
                                       wxDefaultPosition,
                                       wxDefaultSize,
                                       wxLC_REPORT | wxLC_VRULES ) ),
      mUpdateThread( NULL ) {

    
    wxBoxSizer *panelSizer = new wxBoxSizer( wxVERTICAL );

    this->SetSizer( panelSizer );


    // vector shadows the visible UploadList, maintaining an upload ID for
    // each list entry.
    mUploadListIndexVector = new SimpleVector<long>();

    mUploadList->InsertColumn(
        0,
        TranslationManager::translate( "upload_header_virtual_address" ) );
    mUploadList->InsertColumn(
        1,
        TranslationManager::translate( "upload_header_file_path" ) );
    mUploadList->InsertColumn(
        2,
        TranslationManager::translate( "upload_header_chunks" ) );
    mUploadList->InsertColumn(
        3,
        TranslationManager::translate( "upload_header_status" ) );
    
    mUploadList->SetColumnWidth( 0, 200 );
    mUploadList->SetColumnWidth( 1, 200 );
    mUploadList->SetColumnWidth( 2, 125 );
    mUploadList->SetColumnWidth( 3, 150 );
    
    panelSizer->Add( mUploadList,
                     1,            // make vertically stretchable
                     wxEXPAND,     // make horizontally stretchable
                     0 );

    // start our thread
    mUpdateThread = new UploadUpdateThread( (void *)this );
    }



UploadsPanel::~UploadsPanel() {
    // make sure we unlock GUI mutex before joining thread to avoid
    // a deadlock (thread may be trying to post events)
    if( wxThread::IsMain() ) {
        wxMutexGuiLeave();
        }

    delete mUpdateThread;
    
    delete mUploadListIndexVector;
    }






void UploadsPanel::OnUploadsUpdate( wxCommandEvent& inEvent ) {
    int *uploadIDs;
    char **hostAddresses;
    char **filePaths;
    int *chunksInFile;
    int *lastChunksSent;
    unsigned long *firstChunkTimes;
    unsigned long *lastChunkTimes;

    // The displayed list starts empty, and new items are added as required
    // when new uploads are started.  It can't be sorted or cleared (yet).
    // Sorting could be supported later by storing the
    // vector index for each item in that item's data field, and changing
    // the update loop below to work through the list items sequentially
    // instead of the vector items.  Clearing could be
    // supported by deleting both the vector contents and list items, and
    // letting both accumulate new entries from scratch.
    // It might be best to clear out only Done or Failed uploads, to prevent
    // silly ETA times from treating continued uploads as new.
    int numListItems = mUploadList->GetItemCount();
    int numUploads = muteShareGetUploadStatus( &uploadIDs,
                                               &hostAddresses,
                                               &filePaths,
                                               &chunksInFile,
                                               &lastChunksSent,
                                               &firstChunkTimes,
                                               &lastChunkTimes );

    // get current time in seconds using API-prescribed time(NULL) call
    unsigned long currentSeconds = time( NULL );

    // update the list item corresponding to each upload info vector entry
    for( int i=0; i<numUploads; i++ ) {

        char alreadyInList = false;
        int existingListIndex = -1;

        for( int j=0; j<numListItems && !alreadyInList; j++ ) {
            if( uploadIDs[i] ==
                *( mUploadListIndexVector->getElement( j ) ) ) {

                alreadyInList = true;
                existingListIndex = j;
                }
            }

        if( ! alreadyInList ) {
            // append to the end of the list
            existingListIndex = numListItems;

            mUploadListIndexVector->push_back( uploadIDs[i] );
            mUploadList->InsertItem( existingListIndex, hostAddresses[i] );
            
            numListItems++;
            }

        mUploadList->SetItem( existingListIndex, 1, filePaths[i] );


        
        const char *oldCountString = NULL;
        const char *oldStatusString = NULL;

        if( alreadyInList ) {
            // grab last status and count strings so we can check for changes
            // before updating UI

            wxListItem itemInfo;
            itemInfo.m_itemId = existingListIndex;
            itemInfo.m_col = 2;
            if( mUploadList->GetItem( itemInfo ) ) {
                oldCountString = itemInfo.m_text;
                }
            
            itemInfo.m_col = 3;
            if( mUploadList->GetItem( itemInfo ) ) {
                oldStatusString = itemInfo.m_text;
                }
            }

        
        
        // determine the upload's current status
        unsigned long elapsedSeconds = currentSeconds - lastChunkTimes[i];
        int numChunksSent = lastChunksSent[i] + 1;

        char *statusString;
        if( numChunksSent == chunksInFile[i] ) {
            // all chunks uploaded: done
            statusString = stringDuplicate(
                TranslationManager::translate( "upload_status_done" ) );
            }
        else if( elapsedSeconds >= 15 * 60 ) {
            // no progress for 15 minutes: failed
            statusString = stringDuplicate( 
                TranslationManager::translate( "upload_status_failed" ) );
            }
        else if( elapsedSeconds >= 60 ) {
            // no progress for 1 minute: stalled
            statusString = stringDuplicate(
                TranslationManager::translate( "upload_status_stalled" ) );
            }
        else {
            // actively sending: status shows time remaining
            
            if( numChunksSent == 1 ) {
                // don't try to compute time remaining, since we don't
                // have enough information from only one chunk

                statusString = stringDuplicate( 
                    TranslationManager::translate(
                        "upload_status_starting" ) );
                }
            else {
                double secondsPerChunk =
                    (double)( currentSeconds - firstChunkTimes[i] ) /
                    (double)( numChunksSent - 1 );
                
            

                double timeRemainingSeconds =
                    secondsPerChunk * ( chunksInFile[i] - numChunksSent );
                
                char *timeRemainingString =
                    formatTimeIntervalWithUnits( timeRemainingSeconds );
                
                statusString = autoSprintf(
                    TranslationManager::translate(
                        "upload_status_time_remaining" ),
                    timeRemainingString );
                
                delete [] timeRemainingString;
                }
            }


        // update the info columns only if they have changed; this minimizes
        // that annoying screen flashing and probably saves a good bit of
        // CPU once you've got a couple hundred uploads posted

        // format a new count string
        int percentDone = numChunksSent * 100 / chunksInFile[i];
        char *countString = autoSprintf( "%d%%  %d/%d",
                                         percentDone,
                                         numChunksSent,
                                         chunksInFile[i] );
        if( ( oldCountString != NULL &&
              strcmp( oldCountString, countString ) != 0 )
            || oldCountString == NULL ) {
            // count changed.  Update the UI
            mUploadList->SetItem( existingListIndex, 2, countString );
            }
        delete [] countString;
        
        if( ( oldStatusString != NULL &&
              strcmp( oldStatusString, statusString ) != 0 )
            || oldStatusString == NULL ) {
            // status changed.  Update the UI
            mUploadList->SetItem( existingListIndex, 3, statusString );
            }
        delete [] statusString;

        delete [] hostAddresses[i];
        delete [] filePaths[i];
        }

    delete [] uploadIDs;
    delete [] hostAddresses;
    delete [] filePaths;
    delete [] chunksInFile;
    delete [] lastChunksSent;
    delete [] firstChunkTimes;
    delete [] lastChunkTimes;
    }



UploadUpdateThread::UploadUpdateThread( void *inParentPanel )
    : mParentPanel( inParentPanel ),
      mStopLock( new MutexLock() ),
      mStopped( false ) {

    start();
    }



UploadUpdateThread::~UploadUpdateThread() {
    mStopLock->lock();
    mStopped = true;
    mStopLock->unlock();
    
    join();

    delete mStopLock;
    }



void UploadUpdateThread::run() {
    mStopLock->lock();
    char isStopped = mStopped;
    mStopLock->unlock();

    UploadsPanel *panel = (UploadsPanel *)mParentPanel;
    
    while( ! isStopped ) {

        // fire an update event

        // post event so that GUI thread can update UI
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED,
                              UPLOADS_UPDATE_EVENT );
        
        // send in a thread-safe way
        wxPostEvent( panel, event );

        // sleep for 5 seconds in 1-second increments
        for( int i=0; i<5 && !isStopped; i++ ) {
            sleep( 1000 );


            // check if stopped
            mStopLock->lock();
            isStopped = mStopped;
            mStopLock->unlock();
            }
        }
    }





