/*
 * Modification History
 *
 * 2003-October-26   Jason Rohrer
 * Created.
 *
 * 2003-October-27   Jason Rohrer
 * Added support for starting a download.
 *
 * 2003-November-6   Jason Rohrer
 * Added support for new result fields.
 *
 * 2003-November-9   Jason Rohrer
 * Removed excess print statements.
 *
 * 2003-December-16   Jason Rohrer
 * Fixed a scrolling bug in OS X.
 *
 * 2003-December-23   Jason Rohrer
 * Added UI support for hash mismatch.
 *
 * 2004-January-12   Jason Rohrer
 * Fixed a search locking bug.
 *
 * 2004-March-12   Jason Rohrer
 * Added several thread safety fixes and improvements.
 * Added search stop button and activity gauge.
 *
 * 2004-March-30   Jason Rohrer
 * Added search synch print traces.
 *
 * 2004-March-31   Jason Rohrer
 * Added a 5-second pause as part of the search sync trace.
 *
 * 2004-April-2   Jason Rohrer
 * Fixed potential deadlock.
 * Changed to unlock GUI mutex while joining search thread.
 * Removed use of wxMutexGuiEnter, which was causing freezes on win32.
 *
 * 2004-December-10   Jason Rohrer
 * Added a grab hash button.
 * Disabled download and grab hash buttons when no result is selected.
 */



#include "SearchPanel.h"

#include "MUTE/otherApps/fileSharing/userInterface/common/formatUtils.h"

#include "minorGems/util/SettingsManager.h"
#include "minorGems/util/TranslationManager.h"



// for non-precomp compilers, include the necessary headers
#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/thread.h"
//    #include "wx/frame.h"
    #include "wx/panel.h"
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
    #include "wx/button.h"
    #include "wx/app.h"
#endif


// IDs for various controls
enum {
    BUTTON_SEARCH = 1,
    BUTTON_STOP,
    BUTTON_DOWNLOAD,
    BUTTON_GRAB_HASH,
    RESULTS_LIST,
    FIELD_SEARCH,
    SEARCH_RESULTS_EVENT,
    SEARCH_TIMEOUT_EVENT
    };



// event mapping table
BEGIN_EVENT_TABLE( SearchPanel, wxPanel )
    EVT_BUTTON( BUTTON_SEARCH,  SearchPanel::OnSearch )
    EVT_BUTTON( BUTTON_STOP,  SearchPanel::OnStopSearch )
    EVT_BUTTON( BUTTON_DOWNLOAD,  SearchPanel::OnDownload )
    EVT_BUTTON( BUTTON_GRAB_HASH,  SearchPanel::OnGrabHash )
    EVT_TEXT_ENTER( FIELD_SEARCH, SearchPanel::OnSearch )
    // dummy events posted by results handler to inform us of
    // needed GUI updates in a thread-safe manner
    EVT_MENU( SEARCH_RESULTS_EVENT, SearchPanel::OnSearchResults )
    EVT_MENU( SEARCH_TIMEOUT_EVENT, SearchPanel::OnSearchTimeout )
    EVT_LIST_ITEM_SELECTED( RESULTS_LIST, SearchPanel::OnSelectionChange )
    EVT_LIST_ITEM_DESELECTED( RESULTS_LIST, SearchPanel::OnSelectionChange )
END_EVENT_TABLE();



SearchPanel::SearchPanel( wxNotebook *inNotebook,
                          DownloadPanel *inDownloadPanel )
    : wxPanel( inNotebook, -1 ),
      mSearchLock( new MutexLock() ),
      mSearchActive( false ),
      mSearchCanceled( false ),
      mResultLock( new MutexLock() ),
      mResultsFromAddresses( new SimpleVector<char *>() ),
      mResultsFilePaths( new SimpleVector<char *>() ),
      mResultsFileHashes( new SimpleVector<char *>() ),
      mResultsFileSizes( new SimpleVector<unsigned long>() ),
      mActiveGaugeMaxValue( 10 ),
      mActiveGaugeMotionDelta( 1 ),
      mSearchThread( NULL ),
      mSearchField( new wxTextCtrl( this, FIELD_SEARCH,
                                    "", wxDefaultPosition,
                                    wxDefaultSize,
                                    wxTE_PROCESS_ENTER ) ),
      mSearchResultsList( new wxListCtrl( this, RESULTS_LIST,
                                          wxDefaultPosition,
                                          wxDefaultSize,
                                          wxLC_REPORT | wxLC_VRULES ) ),
      mDownloadPanel( inDownloadPanel ) {

    
    char settingFound;
    int traceSearchFlag =
        SettingsManager::getIntSetting( "printSearchSyncTrace",
                                        &settingFound );
    
    if( settingFound &&
        traceSearchFlag == 1 ) {

        mPrintSearchSyncTrace = true;
        }
    else {
        // default
        mPrintSearchSyncTrace = false;
        }

    
    
    wxBoxSizer *panelSizer = new wxBoxSizer( wxVERTICAL );

    this->SetSizer( panelSizer );


    wxBoxSizer *searchFieldSizer = new wxBoxSizer( wxHORIZONTAL );

    panelSizer->Add( searchFieldSizer,
                     0,
                     wxEXPAND | wxALL,
                     10 );

    searchFieldSizer->Add( mSearchField,
                           4,    // 4 times larger that the activity gauge
                           wxALIGN_CENTER,
                           2 );

    mSearchButton = new wxButton(
        this, BUTTON_SEARCH,
        TranslationManager::translate( "search_button" ) );
    
    mStopButton = new wxButton(
        this, BUTTON_STOP, TranslationManager::translate( "stop_button" ) );
    
    mStopButton->Enable( false );
    
    searchFieldSizer->Add( mSearchButton,
                           0,
                           wxALIGN_CENTER,
                           2 );
    searchFieldSizer->Add( mStopButton,
                           0,
                           wxALIGN_CENTER,
                           2 );

    mSearchActiveGauge = new wxGauge( this, -1, mActiveGaugeMaxValue,
                                      wxDefaultPosition, wxDefaultSize,
                                      wxGA_HORIZONTAL | wxGA_SMOOTH );
    mSearchActiveGauge->SetValue( 0 );
    searchFieldSizer->Add( mSearchActiveGauge,
                           1,       //  1/4 the size of the search field
                           wxALIGN_CENTER,
                           2 );


    mSearchResultsList->InsertColumn(
        0, TranslationManager::translate( "result_header_file_name" ) );
    mSearchResultsList->InsertColumn(
        1, TranslationManager::translate( "result_header_size" ) );
    mSearchResultsList->InsertColumn(
        2, TranslationManager::translate( "result_header_hash" ) );
    mSearchResultsList->InsertColumn(
        3, TranslationManager::translate(
            "result_header_virtual_address" ) );
    
    mSearchResultsList->SetColumnWidth( 0, 200 );
    mSearchResultsList->SetColumnWidth( 1, 100 );
    mSearchResultsList->SetColumnWidth( 2, 200 );
    mSearchResultsList->SetColumnWidth( 3, 200 );
    
    
    panelSizer->Add( mSearchResultsList,
                     1,            // make vertically stretchable
                     wxEXPAND,     // make horizontally stretchable
                     2 );          // set border width to 2


    wxBoxSizer *bottomButtonSizer = new wxBoxSizer( wxHORIZONTAL );

    panelSizer->Add( bottomButtonSizer,
                     0,
                     wxEXPAND | wxALL,
                     10 );

    mGrabHashButton =
        new wxButton( this, BUTTON_GRAB_HASH,
                      TranslationManager::translate( "grab_hash_button" ) );

    bottomButtonSizer->Add( mGrabHashButton,
                            0,
                            wxALIGN_LEFT,
                            2 );


    // spacer panel
    bottomButtonSizer->Add( new wxPanel( this, -1 ),
                              1,
                              wxEXPAND,
                              2 );

    mDownloadButton =
        new wxButton( this, BUTTON_DOWNLOAD,
                      TranslationManager::translate( "download_button" ) );

    bottomButtonSizer->Add( mDownloadButton,
                                0,
                                wxALIGN_RIGHT,
                                2 );

    // nothing to grab our download
    mGrabHashButton->Enable( false );
    mDownloadButton->Enable( false );
    }



SearchPanel::~SearchPanel() {
    // cancel any active search
    cancelActiveSearch();
    
    delete mSearchLock;


    int numResults = mResultsFromAddresses->size();
    for( int i=0; i<numResults; i++ ) {
        delete [] *( mResultsFromAddresses->getElement( i ) );
        delete [] *( mResultsFilePaths->getElement( i ) );
        delete [] *( mResultsFileHashes->getElement( i ) );
        }
    delete mResultsFromAddresses;
    delete mResultsFilePaths;
    delete mResultsFileHashes;
    delete mResultsFileSizes;    
    
    delete mResultLock;
    }



void SearchPanel::cancelActiveSearch() {
    if( mPrintSearchSyncTrace ) {
        printf( "            cancelActiveSearch locking search mutex\n" );
        }
    mSearchLock->lock();
    if( mSearchActive ) {
        mSearchCanceled = true;

        if( mPrintSearchSyncTrace ) {
            printf(
                "            cancelActiveSearch unlocking search mutex\n" );
            }

        mSearchLock->unlock();

        
        if( mPrintSearchSyncTrace ) {
            printf(
                "            cancelActiveSearch destroying search thread\n" );
            }
        

        // make sure we unlock GUI mutex before joining thread to avoid
        // a deadlock (thread may be trying to post events)
        if( wxThread::IsMain() ) {
            wxMutexGuiLeave();
            }

        // wait for search thread to return
        delete mSearchThread;

        
        mSearchThread = NULL;

        if( mPrintSearchSyncTrace ) {
            printf(
                "            cancelActiveSearch search thread destroyed\n" );
            }


        if( mPrintSearchSyncTrace ) {
            printf( "            cancelActiveSearch locking search mutex\n" );
            }        
        mSearchLock->lock();
        mSearchActive = false;

        if( mPrintSearchSyncTrace ) {
            printf(
                "            cancelActiveSearch unlocking search mutex\n" );
            }        
        mSearchLock->unlock();
        }
    else {
        if( mPrintSearchSyncTrace ) {
            printf(
                "            cancelActiveSearch unlocking search mutex\n" );
            }        
        mSearchLock->unlock();
        }
    }



// callback for incoming search results
char searchResultHandler( char *inFromAddress, char *inFilePath,
                          unsigned long inFileSize,
                          char *inFileHash,
                          void *inExtraParam ) {
    // unwrap panel from extra parameter
    SearchPanel *parentPanel = (SearchPanel *)inExtraParam;

    
    char printTrace = parentPanel->mPrintSearchSyncTrace;
    
    
    char timeout = true;
    
    if( inFromAddress != NULL ) {
        // got results, no timeout
        timeout = false;

        if( printTrace ) {
            printf( "    GUI result handler locking result mutex\n" );
            }
        parentPanel->mResultLock->lock();


        if( printTrace ) {
            printf( "    GUI result handler adding results\n" );
            }        
        parentPanel->mResultsFromAddresses->push_back(
            stringDuplicate( inFromAddress ) );
        parentPanel->mResultsFilePaths->push_back(
            stringDuplicate( inFilePath ) );
        parentPanel->mResultsFileHashes->push_back(
            stringDuplicate( inFileHash ) );
        parentPanel->mResultsFileSizes->push_back(
            inFileSize );

        if( printTrace ) {
            printf( "    GUI result handler unlocking result mutex\n" );
            }

        parentPanel->mResultLock->unlock();
        }
    // else we timed out waiting for more results

    char keepGoing = true;

    if( printTrace ) {
        printf( "    GUI result handler locking search mutex\n" );
        }
    parentPanel->mSearchLock->lock();

    if( parentPanel->mSearchCanceled ) {
        keepGoing = false;
        }

    if( printTrace ) {
        printf( "    GUI result handler unlocking search mutex\n" );
        }
    parentPanel->mSearchLock->unlock();


    // only post event if we are not canceled
    // this prevents a GUI thread deadlock
    if( keepGoing ) {
        if( printTrace ) {            
            printf( "    GUI result handler posting events to GUI\n" );
            }
        
        // post event so that GUI thread can update results display
        if( timeout ) {
            wxCommandEvent eventResultsTimeout( wxEVT_COMMAND_MENU_SELECTED,
                                                 SEARCH_TIMEOUT_EVENT );
            // send in a thread-safe way
            wxPostEvent( parentPanel, eventResultsTimeout );
            }
        else {                        
            wxCommandEvent eventResultsReceived( wxEVT_COMMAND_MENU_SELECTED,
                                                 SEARCH_RESULTS_EVENT );
            // send in a thread-safe way
            wxPostEvent( parentPanel, eventResultsReceived );
            }
        }

    if( printTrace ) {
        printf( "    GUI result handler returning\n" );
        }        
    return keepGoing;
    }



void SearchPanel::OnSearchResults( wxCommandEvent& inEvent ) {
    if( mPrintSearchSyncTrace ) {
        printf( "        GUI result event handler called\n" );
        }
    mResultLock->lock();

    int firstItemToAdd = mSearchResultsList->GetItemCount();

    int lastItemToAdd = mResultsFilePaths->size() - 1;

    if( firstItemToAdd <= lastItemToAdd ) {
        // add each item that we're missing

        for( int i=firstItemToAdd; i<=lastItemToAdd; i++ ) {
            char *sizeString = formatDataSizeWithUnits(
                *( mResultsFileSizes->getElement( i ) ) );
        
            mSearchResultsList->InsertItem(
                i, *( mResultsFilePaths->getElement( i ) ) );
        
            mSearchResultsList->SetItem( i, 1, sizeString );

            mSearchResultsList->SetItem(
                i, 2, *( mResultsFileHashes->getElement( i ) ) );

            mSearchResultsList->SetItem(
                i, 3, *( mResultsFromAddresses->getElement( i ) ) );
    
            delete [] sizeString;
            }
        }

    
    mResultLock->unlock();
    if( mPrintSearchSyncTrace ) {
        printf( "        GUI result event handler returning\n" );
        }
    }



void SearchPanel::OnSearchTimeout( wxCommandEvent& inEvent ) {
    if( mPrintSearchSyncTrace ) {
        printf( "        GUI timeout event handler called\n" );
        }

    // only update our gauge if we haven't been canceled
    // (ignores any lingering events that come through after we are canceled)
    char shouldUpdate;
    mSearchLock->lock();
    shouldUpdate = !mSearchCanceled;
    mSearchLock->unlock();

    if( shouldUpdate ) {
        // animate our activity gauge to indicate that we are still waiting
        // for results (that we timed out)
    
        int oldValue = mSearchActiveGauge->GetValue();
        int newValue = oldValue + mActiveGaugeMotionDelta;
        
        if( newValue >= mActiveGaugeMaxValue ) {
            newValue = mActiveGaugeMaxValue;
            mActiveGaugeMotionDelta = -mActiveGaugeMotionDelta;
            }
        else if( newValue <= 0 ) {
            newValue = 0;
            mActiveGaugeMotionDelta = -mActiveGaugeMotionDelta;
            }
        
        mSearchActiveGauge->SetValue( newValue );
        }
    
    if( mPrintSearchSyncTrace ) {
        printf( "        GUI timeout event handler returning\n" );
        }
    }



void SearchPanel::OnStopSearch( wxCommandEvent& inEvent ) {
    if( mPrintSearchSyncTrace ) {
        printf( "        GUI stop handler called\n" );
        }
    // cancel the previous search, if there is one active
    cancelActiveSearch();

    mStopButton->Enable( false );
    mSearchButton->Enable( true );
    mSearchField->Enable( true );

    mActiveGaugeMotionDelta = 1;
    mSearchActiveGauge->SetValue( 0 );

    if( mPrintSearchSyncTrace ) {
        printf( "        GUI stop handler returning\n" );
        }
    }



void SearchPanel::OnSearch( wxCommandEvent& inEvent ) {
    
    char *searchString = stringDuplicate( mSearchField->GetValue().c_str() );
                    
    // delete old results, if there are any
    mResultLock->lock();

    // scroll back to item 0 before deleting items to fix a bug
    // in OS X behavior
    if( mSearchResultsList->GetItemCount() > 0 ) {
        mSearchResultsList->EnsureVisible( 0 );
        }
    
    mSearchResultsList->DeleteAllItems();

    int numResults = mResultsFromAddresses->size();
    for( int i=0; i<numResults; i++ ) {
        delete [] *( mResultsFromAddresses->getElement( i ) );
        delete [] *( mResultsFilePaths->getElement( i ) );
        delete [] *( mResultsFileHashes->getElement( i ) );
        }
    mResultsFromAddresses->deleteAll();
    mResultsFilePaths->deleteAll();
    mResultsFileHashes->deleteAll();
    mResultsFileSizes->deleteAll();
    
    mResultLock->unlock();


    // spawn thread for search
    mSearchLock->lock();
    mSearchCanceled = false;
    mSearchActive = true;
    mSearchThread = new SearchThread( searchString,
                                      (void *)this );
    mSearchLock->unlock();
    
    delete [] searchString;

    mSearchButton->Enable( false );
    mStopButton->Enable( true );
    mSearchField->Enable( false );    
    }



void SearchPanel::OnDownload( wxCommandEvent& inEvent ) {

    mResultLock->lock();

    int count = mSearchResultsList->GetItemCount();

    for( int i=0; i<count; i++ ) {
        if( mSearchResultsList->GetItemState( i, wxLIST_STATE_SELECTED ) ) {

            char *fromAddress = *( mResultsFromAddresses->getElement( i ) );
            char *filePath = *( mResultsFilePaths->getElement( i ) );
            char *fileHash = *( mResultsFileHashes->getElement( i ) );

            
            mDownloadPanel->addDownload( fromAddress, filePath, fileHash );
            }
        }

    
    
    mResultLock->unlock();
    
    }



void SearchPanel::OnGrabHash( wxCommandEvent& inEvent ) {

    mResultLock->lock();

    int count = mSearchResultsList->GetItemCount();

    char *foundHash = NULL;

    for( int i=0; i<count && foundHash == NULL; i++ ) {

        if( mSearchResultsList->GetItemState( i, wxLIST_STATE_SELECTED ) ) {

            foundHash =
                stringDuplicate( *( mResultsFileHashes->getElement( i ) ) );
            }
        }
    mResultLock->unlock();

    if( foundHash != NULL ) {
        // stop the active search if there is one
        OnStopSearch( inEvent );

        char *searchString = autoSprintf( "hash_%s", foundHash );

        delete [] foundHash;

        mSearchField->SetValue( searchString );

        delete [] searchString;
        }
    }



void SearchPanel::OnSelectionChange( wxListEvent& inEvent ) {
    if( mSearchResultsList->GetSelectedItemCount() == 0 ) {
        // disable buttons
        mGrabHashButton->Enable( false );
        mDownloadButton->Enable( false );
        }
    else {
        // we now have something to grab or download
        mGrabHashButton->Enable( true );
        mDownloadButton->Enable( true );
        }
    }


