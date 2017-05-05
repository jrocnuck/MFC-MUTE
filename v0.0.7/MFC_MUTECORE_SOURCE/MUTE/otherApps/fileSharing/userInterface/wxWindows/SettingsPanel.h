/*
 * Modification History
 *
 * 2003-November-2   Jason Rohrer
 * Created.
 *
 * 2003-December-19   Jason Rohrer
 * Added browsing feature.
 *
 * 2004-January-26   Jason Rohrer
 * Added UI for download timeout setting.
 *
 * 2004-March-23   Jason Rohrer
 * Added a maximum connection count.
 *
 * 2004-October-7   Jason Rohrer
 * Added translation manager.
 *
 * 2004-October-11   Jason Rohrer
 * Switched to combo box for language to work around aparent bug in wxMac.
 *
 * 2004-October-12   Jason Rohrer
 * Switched to choice box.
 */



#ifndef SETTINGS_PANEL_INCLUDED
#define SETTINGS_PANEL_INCLUDED



// includes and definitions copied from wxWindows sample calendar app


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

// for all others, include the necessary headers
#ifndef WX_PRECOMP
    #include "wx/panel.h"
    #include "wx/notebook.h"
    #include "wx/textctrl.h"
    #include "wx/choice.h"
#endif



/**
 * Panel in settings tab.
 */
class SettingsPanel : public wxPanel {

    public:

        SettingsPanel( wxNotebook *inNotebook );

        ~SettingsPanel();

        
    private:
        wxChoice *mNaturalLanguageChoiceBox;
        wxTextCtrl *mInboundLimitField;
        wxTextCtrl *mOutboundLimitField;
        wxTextCtrl *mDownloadTimeoutField;
        wxTextCtrl *mTargetConnectionCountField;
        wxTextCtrl *mMaxConnectionCountField;

        wxTextCtrl *mSharePathField;
        
        
        // event handlers
        void OnSaveSettings( wxCommandEvent& inEvent );
        void OnBrowse( wxCommandEvent& inEvent );

        // register to handle events 
        DECLARE_EVENT_TABLE();

        
        
    };



#endif


