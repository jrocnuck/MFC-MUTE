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
 */



#ifndef DOWNLOAD_PANEL_INCLUDED
#define DOWNLOAD_PANEL_INCLUDED



// includes and definitions copied from wxWindows sample calendar app


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

// for all others, include the necessary headers
#ifndef WX_PRECOMP
    #include "wx/panel.h"
    #include "wx/notebook.h"
    #include "wx/sizer.h"
    #include "wx/scrolwin.h"
#endif



#include "DownloadItem.h"



#include "minorGems/util/SimpleVector.h"



/**
 * Panel in downloads tab.
 */
class DownloadPanel : public wxPanel {

    public:



        /**
         * Constructs a panel.
         *
         * @param inNotebook the notebook that this panel is in.
         * @param inNotebookPageIndex the index of this panel's page
         *   in the notebook.
         */
        DownloadPanel( wxNotebook *inNotebook,
                       int inNotebookPageIndex );

        
        
        ~DownloadPanel();

        
        
        /**
         * Adds a download to this panel.
         *
         * @param inFromAddress the virtual address of the host to download
         *   from.  Must be destroyed by caller.
         * @param inFilePath the file path to download.
         *   Must be destroyed by caller.
         * @param inFileHash the SHA1 hash of the file.
         *   Must be destroyed by caller.
         */
        void addDownload( char *inFromAddress, char *inFilePath,
                          char *inFileHash );



        /**
         * Gets whether any downloads in this panel are still active.
         *
         * If downloads are active, they should be canceled by the user
         * before this panel is destroyed (for example, before app shutdown).
         *
         * @return true if downloads are still active.
         */         
        char areDownloadsActive();

        
        
    private:
        wxNotebook *mNotebook;
        int mNotebookPageIndex;
        
        wxScrolledWindow *mScrollView;
        wxBoxSizer *mDownloadSizer;


        SimpleVector<DownloadItem *> *mDownloadItems;
        
    };



#endif


