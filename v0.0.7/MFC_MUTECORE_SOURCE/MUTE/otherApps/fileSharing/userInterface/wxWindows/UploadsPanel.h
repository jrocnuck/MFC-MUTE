/*
 * Modification History
 *
 * 2003-November-4   Jason Rohrer
 * Created.
 *
 * 2004-February-13   Jason Rohrer
 * Cleaned up Mycroftxxx's patched code.
 */



#ifndef UPLOADS_PANEL_INCLUDED
#define UPLOADS_PANEL_INCLUDED



// includes and definitions copied from wxWindows sample calendar app


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

// for all others, include the necessary headers
#ifndef WX_PRECOMP
    #include "wx/panel.h"
    #include "wx/notebook.h"
    #include "wx/listctrl.h"
#endif



#include "minorGems/system/Thread.h"
#include "minorGems/system/MutexLock.h"
#include "minorGems/util/SimpleVector.h"



/**
 * Thread that triggers an update in the uploads display.
 */
class UploadUpdateThread : public Thread {

    public:

        

        /**
         * Constructs and starts a thread.
         *
         * @param inParentPanel the panel to fire update events to.
         *   Wrapped as void * to avoid class reference loop.
         */
        UploadUpdateThread( void *inParentPanel );

        
        
        /**
         * Stops, joins and destroys this thread.
         */
        ~UploadUpdateThread();

        

        // implements Thread interface
        void run();

        
        
    private:
        void *mParentPanel;
        MutexLock *mStopLock;
        char mStopped;
    };



/**
 * Panel in uploads tab.
 */
class UploadsPanel : public wxPanel {

    public:

        UploadsPanel( wxNotebook *inNotebook );

        ~UploadsPanel();

        
    private:
        
        wxListCtrl *mUploadList;
        UploadUpdateThread *mUpdateThread;
        SimpleVector<long> *mUploadListIndexVector;
        
        // handler for update event fired by thread
        void OnUploadsUpdate( wxCommandEvent& inEvent );
        
        
        // register to handle events 
        DECLARE_EVENT_TABLE();

        
        
    };



#endif


