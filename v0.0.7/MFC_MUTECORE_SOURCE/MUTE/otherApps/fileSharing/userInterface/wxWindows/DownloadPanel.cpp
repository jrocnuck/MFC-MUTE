/*
 * Modification History
 *
 * 2003-October-26   Jason Rohrer
 * Created.
 *
 * 2003-October-27   Jason Rohrer
 * Added support for download item deletion.
 *
 * 2003-November-9   Jason Rohrer
 * Added function that checks for active downloads.
 *
 * 2003-December-23   Jason Rohrer
 * Added UI support for hash mismatch.
 *
 * 2004-December-20   Jason Rohrer
 * Added check for downloads that are already in progress.
 */



#include "DownloadPanel.h"
#include "DownloadItem.h"

#include "minorGems/util/TranslationManager.h"


// for non-precomp compilers, include the necessary headers
#ifndef WX_PRECOMP
    #include "wx/app.h"
//  #include "wx/log.h"
    #include "wx/frame.h"
    #include "wx/panel.h"
    #include "wx/stattext.h"
    #include "wx/menu.h"
    #include "wx/notebook.h"
//  #include "wx/layout.h"
    #include "wx/dialog.h"
    #include "wx/msgdlg.h"
    #include "wx/sizer.h"
    #include "wx/textdlg.h"
//  #include "wx/progdlg.h"
    #include "wx/utils.h"
    #include "wx/choicdlg.h"
    #include "wx/scrolwin.h"
    #include "wx/button.h"
#endif



#include "minorGems/util/stringUtils.h"



DownloadPanel::DownloadPanel( wxNotebook *inNotebook,
                              int inNotebookPageIndex )
    : wxPanel( inNotebook, -1 ),
      mNotebook( inNotebook ),
      mNotebookPageIndex( inNotebookPageIndex ),
      mDownloadItems( new SimpleVector<DownloadItem *>() ) {


    wxBoxSizer *panelSizer = new wxBoxSizer( wxVERTICAL );

    this->SetSizer( panelSizer );

    mScrollView = new wxScrolledWindow( this, -1 );
    mScrollView->SetScrollRate( 5, 5 );
        
    panelSizer->Add( mScrollView,
                     1,
                     wxEXPAND,
                     0 );


    // a box layout inside the scroll view
    mDownloadSizer = new wxBoxSizer( wxVERTICAL );

    mScrollView->SetSizer( mDownloadSizer );

    /*
    for( int i=0; i<40; i++ ) {
        char *buttonText = autoSprintf( "button %d", i );
        
        wxButton *testButton = new wxButton( mScrollView, -1,
                                             buttonText );
        delete [] buttonText;

        mDownloadSizer->Add( testButton,
                            0,
                            wxALIGN_LEFT,
                            0 );
        }
    */
    }



DownloadPanel::~DownloadPanel() {
    // items in vector will be deleted by wxWindows destruction process
    delete mDownloadItems;
    }



void DownloadPanel::addDownload( char *inFromAddress, char *inFilePath,
                                 char *inFileHash ) {
    // bring our page to the front before modifying the GUI
    // mNotebook->SetSelection( mNotebookPageIndex );

    // make sure that an existing download item is not already fetching
    // a file with the same hash
    int numItems = mDownloadItems->size();
    
    // jump out as soon as we find one
    char hashFound = false;
    char *activeName = NULL;
    for( int i=0; i<numItems && !hashFound; i++ ) {
        DownloadItem *item = *( mDownloadItems->getElement( i ) );

        if( item->isActive() ) {
            char *hash = item->getFileHash();

            if( strcmp( hash, inFileHash ) == 0 ) {
                hashFound = true;
                
                }
            delete [] hash;
            }
        }

    if( !hashFound ) {
    
        DownloadItem *addedItem = new DownloadItem( mScrollView,
                                                    mDownloadSizer,
                                                    inFromAddress, inFilePath,
                                                    inFileHash );
        
        mScrollView->FitInside();
        
        mDownloadItems->push_back( addedItem );
        }
    else {

        // do not re-download the active file
        const char *messageTemplate = TranslationManager::translate(
            "download_already_active_message" );

        char *message = autoSprintf( messageTemplate,
                                     inFilePath );

        wxMessageDialog *downloadActiveBox = new wxMessageDialog(
            NULL,
            message,
            TranslationManager::translate( "download_already_active_title" ),
            wxOK );  // only OK button

        delete [] message;
        
        // wait for user OK
        downloadActiveBox->ShowModal();
        delete downloadActiveBox;
        }
    
    // check to see if any of our items are ready to be destroyed
    // this is delayed destruction, but it will prevent leaks because
    // we check for needed destruction each time a new download is added.

    // thus, only the last-added download represents potentially unclaimed
    // space, and it will be reclaimed anyway at program destruction

    char found = true;

    while( found ) {
        int numItems = mDownloadItems->size();

        // jump out as soon as we find one
        found = false;        
        for( int i=0; i<numItems && !found; i++ ) {

            DownloadItem *item = *( mDownloadItems->getElement( i ) );

            if( item->isCleared() ) {
                // destroy it

                // this will remove it from its sizer and from this panel,
                // though if it is cleared, it isn't visible anymore anyway
                item->Destroy();

                mDownloadItems->deleteElement( i );

                found = true;
                }
            }
        }

    
    }



char DownloadPanel::areDownloadsActive() {

    int numItems = mDownloadItems->size();
    
    for( int i=0; i<numItems; i++ ) {
        
        DownloadItem *item = *( mDownloadItems->getElement( i ) );

        if( item->isActive() ) {
            return true;
            }
        }

    // no actives found
    return false;
    }




