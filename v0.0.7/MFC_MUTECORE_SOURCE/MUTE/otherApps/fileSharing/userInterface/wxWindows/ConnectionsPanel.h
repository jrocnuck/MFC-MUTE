/*
 * Modification History
 *
 * 2003-October-28   Jason Rohrer
 * Created.
 *
 * 2004-February-20   Jason Rohrer
 * Added info about current connection attempt.
 *
 * 2004-December-24   Jason Rohrer
 * Added a connection quality gauge.
 */



#ifndef CONNECTIONS_PANEL_INCLUDED
#define CONNECTIONS_PANEL_INCLUDED



// includes and definitions copied from wxWindows sample calendar app


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

// for all others, include the necessary headers
#ifndef WX_PRECOMP
    #include "wx/panel.h"
    #include "wx/notebook.h"
    #include "wx/listctrl.h"
    #include "wx/textctrl.h"
    #include "wx/stattext.h"
    #include "wx/gauge.h"
#endif



#include "minorGems/system/Thread.h"
#include "minorGems/system/MutexLock.h"



/**
 * Thread that triggers an update in the connections display.
 */
class ConnectionUpdateThread : public Thread {

    public:

        

        /**
         * Constructs and starts a thread.
         *
         * @param inParentPanel the panel to fire update events to.
         *   Wrapped as void * to avoid class reference loop.
         */
        ConnectionUpdateThread( void *inParentPanel );

        
        
        /**
         * Stops, joins and destroys this thread.
         */
        ~ConnectionUpdateThread();

        

        // implements Thread interface
        void run();

        
        
    private:
        void *mParentPanel;
        MutexLock *mStopLock;
        char mStopped;
    };



/**
 * Panel in connections tab.
 */
class ConnectionsPanel : public wxPanel {

    public:

        /**
         * Constructs a panel.
         *
         * @param inNotebook the parent notebook of this panel.
         * @param inConnectionQualityGauge the gauge to report our
         *   connection quality on.
         */
        ConnectionsPanel( wxNotebook *inNotebook,
                          wxGauge *inConnectionQualityGauge );

        ~ConnectionsPanel();

        
    private:
        wxGauge *mConnectionQualityGauge;
        wxTextCtrl *mAddHostAddressField;
        wxTextCtrl *mAddHostPortField;
        
        wxListCtrl *mConnectionList;
        ConnectionUpdateThread *mUpdateThread;

        wxStaticText *mConnectionStatus;
        
        // event handlers
        void OnAddHost( wxCommandEvent& inEvent );
        
        // handler for update event fired by thread
        void OnConnectionsUpdate( wxCommandEvent& inEvent );
        
        
        // register to handle events 
        DECLARE_EVENT_TABLE();

        
        
    };



#endif


