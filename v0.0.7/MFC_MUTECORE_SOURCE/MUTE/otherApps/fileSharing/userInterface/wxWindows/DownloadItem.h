/*
 * Modification History
 *
 * 2003-October-27   Jason Rohrer
 * Created.
 *
 * 2003-October-27   Jason Rohrer
 * Added support for download item deletion.
 * Added better download stats.
 * Improved layout.
 *
 * 2003-November-2   Jason Rohrer
 * Switched to MS time resolution for download rate.
 *
 * 2003-November-6   Jason Rohrer
 * Worked on deadlock when item destroyed while download in progress.
 *
 * 2003-November-9   Jason Rohrer
 * Added function that checks active download status.
 *
 * 2003-December-23   Jason Rohrer
 * Added UI support for hash mismatch.
 *
 * 2004-February-12   Jason Rohrer
 * Changed to use average transfer rate to compute ETA.
 *
 * 2004-February-22   Jason Rohrer
 * Added route quality gauge.
 *
 * 2004-December-8   Jason Rohrer
 * Added some support for resumable downloads.
 *
 * 2004-December-20   Jason Rohrer
 * Added check for downloads that are already in progress.
 */



#ifndef DOWNLOAD_ITEM_INCLUDED
#define DOWNLOAD_ITEM_INCLUDED



// includes and definitions copied from wxWindows sample calendar app


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

// for all others, include the necessary headers
#ifndef WX_PRECOMP
    #include "wx/panel.h"
    #include "wx/window.h"
    #include "wx/sizer.h"
    #include "wx/stattext.h"
    #include "wx/gauge.h"
    #include "wx/button.h"
#endif


#include "minorGems/system/Thread.h"
#include "minorGems/system/MutexLock.h"
#include "minorGems/io/file/File.h"

#include <stdio.h>



class DownloadThread : public Thread {

    public:
        /**
         * Constructs and starts a download thread.
         *
         * @param inFromAddress the virtual address of the host to download
         *   from.  Must be destroyed by caller.
         * @param inFilePath the file path to download.
         *   Must be destroyed by caller.
         * @param inFileHash the SHA1 hash of the file.
         *   Must be destroyed by caller.
         * @param inDownloadSizePointer pointer to where the download
         *   size should be set when it is known.
         * @param inDownloadItem the parent item panel, wrapped
         *   as a void * to avoid a class reference loop.
         * @param inPartialFileName the platform-dependent path to
         *   an existing partial download, or NULL if no partial file exists.
         *   Defaults to NULL.
         *   Will be destroyed when this thread is destroyed.
         * @param inNumPartialBytes the number of bytes that have already
         *   been downloaded as a partial transfer, or 0 if no partial
         *   file exists.  Defaults to 0.
         */
        DownloadThread( char *inFromAddress, char *inFilePath,
                        char *inFileHash,
                        int *inDownloadSizePointer,
                        void *inDownloadItem,
                        char *inPartialFileName = NULL,
                        int inNumPartialBytes = 0 );

        // joins, and destroys this search thread
        ~DownloadThread();

        // implements Thread interface
        void run();

        
    private:
        char *mFromAddress;
        char *mFilePath;
        char *mFileHash;
        int *mDownloadSizePointer;
        void *mDownloadItem;
        char *mPartialFileName;
        int mNumPartialBytes;
        
        MutexLock *mStopLock;
        char mStopped;
        
    };



/**
 * Panel for a download item.
 */
class DownloadItem : public wxPanel {

    public:


        /**
         * Constructs an item.
         *
         * @param inParent the parent window.
         * @param inParentSizer the sizer that this item should add
         *   itself to.  This item will also remove itself once it has
         *   be canceled.
         * @param inFromAddress the virtual address of the host to download
         *   from.  Must be destroyed by caller.
         * @param inFilePath the file path to download.
         *   Must be destroyed by caller.
         * @param inFileHash the SHA1 hash of the file.
         *   Must be destroyed by caller.
         */
        DownloadItem( wxWindow *inParent,
                      wxBoxSizer *inParentSizer,
                      char *inFromAddress, char *inFilePath,
                      char *inFileHash );

        
        
        ~DownloadItem();
        

        
        /**
         * Processes an incoming chunk of this download.
         *
         * @param inChunk the chunk data, or NULL to indicate that the
         *   current chunk has failed and will be retried.
         *   Must be destroyed by caller.
         * @param inChunkLengthInBytes the length of the chunk data in bytes,
         *   or -1 if the current chunk has failed and will be retried.
         */
        void processChunk( unsigned char *inChunk,
                           int inChunkLengthInBytes );

        

        /**
         * Processes the final result of a download.
         *
         * @param inResult the return value of muteShareGetFile.
         */
        void processDownloadResult( int inResult );
        
        
        
        /**
         * Gets whether this download has been canceled by the user.
         *
         * @return true if canceled, or false otherwise.
         */
        char isCanceled();



        /**
         * Gets whether this item has been cleared from view and is ready for
         * explicit destruction.
         *
         * Uncleared windows will be destroyed automatically at program
         * termination by the standard wxWindows destruction process.
         *
         * @return true if this item has been cleared.
         */
        char isCleared();


        
        /**
         * Gets whether this download is still active (incoming).
         *
         * @return true if this download is still active.
         */
        char isActive();
        


        /**
         * Gets the hash of the file being downloaded.
         *
         * @return the SHA1 hash (hex-encoded).  Must be destroyed by caller.
         */
        char *getFileHash();


        
        // ID to use when interrupting a download to cancel it
        // public so that our download thread can access it
        int mDownloadID;

        
        
    private:
        
        /**
         * Event handler for cancel/clear button.
         */
        void OnCancelClear( wxCommandEvent &event );



        // event handlers for download progress

        
        /**
         * Called for any kind of download progress to update
         * progress bar and ETA display.
         */
        void OnDownloadProgress( wxCommandEvent &event );

        
        /**
         * Called when a chunk is retried.
         */ 
        void OnChunkRetry( wxCommandEvent &event );

        
        /**
         * Called when a chunk is received.
         */ 
        void OnChunkReceived( wxCommandEvent &event );

        
        /**
         * Called when a download finishes (completes, is canceled, or fails).
         */
        void OnDownloadResult( wxCommandEvent &event );


        
        // register to handle events 
        DECLARE_EVENT_TABLE();

        
        wxWindow *mParentWindow;
        wxBoxSizer *mParentSizer;
        wxBoxSizer *mPanelSizer;

        MutexLock *mDownloadStatusLock;

        char mDownloadActive;
        int mDownloadSizeInBytes;
        int mDownloadedSoFarInBytes;

        // remember the start time so we can compute the ETA
        // using the average transfer rate instead of just the most
        // recent rate.
        unsigned long mDownloadStartTimeSeconds;
        unsigned long mDownloadStartTimeMilliseconds;

        
        // measure the rate with blocks of time that
        // are at least 1 second long
        unsigned long mCurrentBlockStartTimeSeconds;
        unsigned long mCurrentBlockStartTimeMilliseconds;
        
        long mCurrentBlockStartSize;
        
        float mCurrentRate;
        
        int mDownloadResult;
        
        wxStaticText *mStatusLabel;
        wxGauge *mProgressGauge;
        wxGauge *mRouteQualityGauge;
        wxButton *mCancelClearButton;
        
        DownloadThread *mDownloadThread;

        char mCanceled;
        char mCleared;
        
        FILE *mDownloadFILE;
        File *mDownloadFile;

        // for storing info about the number of bytes successfully received
        File *mDownloadInfoFile;

        // for storing a map from a hash to a downloaded partial 
        File *mHashMapFile;

        char *mFileHash;
    };



#endif


