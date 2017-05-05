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
 * 2003-December-23   Jason Rohrer
 * Added UI support for hash mismatch.
 *
 * 2004-March-12   Jason Rohrer
 * Added several thread safety fixes and improvements.
 * Added search stop button and activity gauge.
 *
 * 2004-March-30   Jason Rohrer
 * Added search synch print traces.
 *
 * 2004-April-2   Jason Rohrer
 * Fixed potential deadlock.
 *
 * 2004-December-10   Jason Rohrer
 * Disabled download and grab hash buttons when no result is selected.
 */



#ifndef SEARCH_PANEL_INCLUDED
#define SEARCH_PANEL_INCLUDED



// includes and definitions copied from wxWindows sample calendar app


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

// for all others, include the necessary headers
#ifndef WX_PRECOMP
    #include "wx/panel.h"
    #include "wx/notebook.h"
    #include "wx/textctrl.h"
    #include "wx/listctrl.h"
    #include "wx/gauge.h"
#endif


#include "DownloadPanel.h"



#include "minorGems/system/MutexLock.h"
#include "minorGems/system/Thread.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SimpleVector.h"



#include "MUTE/otherApps/fileSharing/fileShare.h"



// callback for incoming search results
char searchResultHandler( char *inFromAddress, char *inFilePath,
                          unsigned long inFileSize,
                          char *inFileHash,
                          void *inExtraParam );



/**
 * Thread that runs the MUTE search.
 */
class SearchThread : public Thread {

    public:
        /**
         * Constructs and starts a search thread
         * @param inSearchString must be destroyed by caller.
         * @param inExtraHandlerParam parameter to pass to results handler.
         */
        SearchThread( char *inSearchString, void *inExtraHandlerParam )
            : mSearchString( stringDuplicate( inSearchString ) ),
              mExtraHandlerParam( inExtraHandlerParam ) {
            
            start();
            }

        /**
         * Joins and destroys this search thread.
         */
        ~SearchThread() {
            join();

            delete [] mSearchString;
            }

        // implements Thread interface
        void run() {
            muteShareSearch( mSearchString, searchResultHandler,
                             mExtraHandlerParam,
                             1000 );
            }
        
    private:
        char *mSearchString;
        void *mExtraHandlerParam;
        
    };



/**
 * Panel in search tab.
 */
class SearchPanel : public wxPanel {

    public:

        SearchPanel( wxNotebook *inNotebook, DownloadPanel *inDownloadPanel );

        ~SearchPanel();


        
        /**
         * Cancels any active search.
         *
         * Must be called before shutting down MUTE filesharing layer.
         */
        void cancelActiveSearch();


        
        char mPrintSearchSyncTrace;

        
        
        MutexLock *mSearchLock;
        char mSearchActive;
        char mSearchCanceled;

        MutexLock *mResultLock;
        SimpleVector<char *> *mResultsFromAddresses;
        SimpleVector<char *> *mResultsFilePaths;
        SimpleVector<char *> *mResultsFileHashes;
        SimpleVector<unsigned long> *mResultsFileSizes;
        
        wxListCtrl *mSearchResultsList;


        
    private:
        // event handlers
        void OnSearch( wxCommandEvent& inEvent );
        void OnStopSearch( wxCommandEvent& inEvent );
        void OnDownload( wxCommandEvent& inEvent );
        void OnGrabHash( wxCommandEvent& inEvent );
        void OnSearchResults( wxCommandEvent& inEvent );
        void OnSearchTimeout( wxCommandEvent& inEvent );
        void OnSelectionChange( wxListEvent& inEvent );
        
        // register to handle events 
        DECLARE_EVENT_TABLE();
        

        wxTextCtrl *mSearchField;
        wxButton *mSearchButton;
        wxButton *mStopButton;
        wxButton *mGrabHashButton;
        wxButton *mDownloadButton;
        wxGauge *mSearchActiveGauge;

        int mActiveGaugeMaxValue;
        int mActiveGaugeMotionDelta;

        
        SearchThread *mSearchThread;

        DownloadPanel *mDownloadPanel;
        
        
    };



#endif


