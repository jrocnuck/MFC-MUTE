/*
 * Modification History
 *
 * 2003-November-2   Jason Rohrer
 * Created.
 *
 * 2003-December-19   Jason Rohrer
 * Added browsing feature.
 * Added check for bad file path.
 *
 * 2003-December-21   Jason Rohrer
 * Added first-startup message box explaining directory chooser on OS X.
 *
 * 2004-January-26   Jason Rohrer
 * Fixed the starting value for the Outbound Limit field.
 * Added UI for download timeout setting.
 *
 * 2004-January-27   Jason Rohrer
 * Fixed a memory leak.
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
 * Switched to choice box, but fixed choice array.
 */



#include "SettingsPanel.h"

#include "minorGems/io/file/Path.h"
#include "minorGems/io/file/File.h"
#include "minorGems/util/SettingsManager.h"
#include "minorGems/util/TranslationManager.h"




// for non-precomp compilers, include the necessary headers
#ifndef WX_PRECOMP
    #include "wx/app.h"
//  #include "wx/log.h"
//    #include "wx/frame.h"
//    #include "wx/panel.h"
    #include "wx/stattext.h"
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
    #include "wx/dirdlg.h"
    #include "wx/msgdlg.h"
#endif



#include "MUTE/otherApps/fileSharing/fileShare.h"
#include "MUTE/layers/messageRouting/messageRouter.h"



#include "minorGems/util/stringUtils.h"



// IDs for various controls
enum {
    BUTTON_SAVE_SETTINGS = 1,
    BUTTON_BROWSE
    };



// event mapping table
BEGIN_EVENT_TABLE( SettingsPanel, wxPanel )
    EVT_BUTTON( BUTTON_SAVE_SETTINGS,  SettingsPanel::OnSaveSettings )
    EVT_BUTTON( BUTTON_BROWSE,  SettingsPanel::OnBrowse )
END_EVENT_TABLE();



SettingsPanel::SettingsPanel( wxNotebook *inNotebook )
    : wxPanel( inNotebook, -1 ) {

    // get current settings
    double inboundLimit = muteGetInboundMessagePerSecondLimit();
    double outboundLimit = muteGetOutboundMessagePerSecondLimit();
    int targetConnectionCount = muteGetTargetNumberOfConnections();
    int maxConnectionCount = muteGetMaxNumberOfConnections();
    char *sharePath = muteShareGetSharingPath();

    char found;
    int downloadTimeoutMilliseconds =
        SettingsManager::getIntSetting( "downloadTimeoutMilliseconds",
                                        &found );
    if( !found || downloadTimeoutMilliseconds < 0 ) {
        // default to 60 seconds
        downloadTimeoutMilliseconds = 60000;
        }

    char *naturalLanguage = SettingsManager::getStringSetting( "language" );

    if( naturalLanguage == NULL ) {
        naturalLanguage = stringDuplicate( "English" );
        }


    int numLanguages;
    char **availableLanguages =
        TranslationManager::getAvailableLanguages( &numLanguages );

    // make a wxString array for our choices
    wxString *languageChoices = new wxString[ numLanguages ];

    for( int i=0; i<numLanguages; i++ ) {
        // wxString assignment copies cstring data
        languageChoices[i] = availableLanguages[i];
        
        delete [] availableLanguages[i];
        }
    delete [] availableLanguages;
    
    mNaturalLanguageChoiceBox = new wxChoice(
        this, -1,
        wxDefaultPosition,
        wxDefaultSize,
        numLanguages,
        languageChoices );

    delete [] languageChoices;

    
    mNaturalLanguageChoiceBox->SetStringSelection( naturalLanguage );

    delete [] naturalLanguage;

    
    
    // construct the settings text fields    
    char *inboundLimitString = autoSprintf( "%f", inboundLimit );
    mInboundLimitField = new wxTextCtrl( this, -1,
                                         inboundLimitString,
                                         wxDefaultPosition,
                                         wxDefaultSize,
                                         wxTE_PROCESS_ENTER );
    delete [] inboundLimitString;

    char *outboundLimitString = autoSprintf( "%f", outboundLimit );
    mOutboundLimitField = new wxTextCtrl( this, -1,
                                          outboundLimitString,
                                          wxDefaultPosition,
                                          wxDefaultSize,
                                          wxTE_PROCESS_ENTER );
    delete [] outboundLimitString;


    int downloadTimeoutInSeconds = downloadTimeoutMilliseconds / 1000;
    
    char *downloadTimeoutString =
        autoSprintf( "%d", downloadTimeoutInSeconds );
    mDownloadTimeoutField = new wxTextCtrl( this, -1,
                                            downloadTimeoutString,
                                            wxDefaultPosition,
                                            wxDefaultSize,
                                            wxTE_PROCESS_ENTER );
    delete [] downloadTimeoutString;

    
    char *targetConnectionCountString =
        autoSprintf( "%d", targetConnectionCount );
    mTargetConnectionCountField = new wxTextCtrl( this, -1,
                                                  targetConnectionCountString,
                                                  wxDefaultPosition,
                                                  wxDefaultSize,
                                                  wxTE_PROCESS_ENTER );
    delete [] targetConnectionCountString;


    char *maxConnectionCountString =
        autoSprintf( "%d", maxConnectionCount );
    mMaxConnectionCountField = new wxTextCtrl( this, -1,
                                                  maxConnectionCountString,
                                                  wxDefaultPosition,
                                                  wxDefaultSize,
                                                  wxTE_PROCESS_ENTER );
    delete [] maxConnectionCountString;


    mSharePathField = new wxTextCtrl( this, -1,
                                      sharePath,
                                      wxDefaultPosition,
                                      wxDefaultSize,
                                      wxTE_PROCESS_ENTER );
    delete [] sharePath;

    

    wxBoxSizer *panelSizer = new wxBoxSizer( wxVERTICAL );

    this->SetSizer( panelSizer );

    
    
    // a 3-column grid sizer, 10-pixel gaps
    wxGridSizer *gridSizer = new wxGridSizer( 3, 10, 10 );

    // expand the grid horizontally but not vertically
    panelSizer->Add( gridSizer,
                     0,
                     wxEXPAND,
                     0 );


    
    gridSizer->Add(
        new wxStaticText(
            this, -1,
            TranslationManager::translate( "natural_language_label" ) ),
        0,
        wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL,
        0 );

    gridSizer->Add( mNaturalLanguageChoiceBox,
                    0,
                    wxEXPAND,
                    0 );
    
    gridSizer->Add(
        new wxStaticText(
            this, -1,
            TranslationManager::translate( "language_restart_tip" ) ),
        0,
        wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL,
        0 );
    
    
    
    gridSizer->Add(
        new wxStaticText(
            this, -1,
            TranslationManager::translate( "inbound_limit_label" ) ),
        0,
        wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL,
        0 );

    gridSizer->Add( mInboundLimitField,
                    0,
                    wxEXPAND,
                    0 );
    
    gridSizer->Add(
        new wxStaticText(
            this, -1,
            TranslationManager::translate( "message_per_second_units" ) ),
        0,
        wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL,
        0 );


    
    gridSizer->Add( new wxStaticText( this, -1, "" ),
                    0,
                    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL,
                    0 );

    gridSizer->Add(
        new wxStaticText(
            this, -1,
            TranslationManager::translate( "no_limit_tip" ) ),
        0,
        wxALIGN_CENTER,
        0 );
    
    gridSizer->Add( new wxStaticText( this, -1, "" ),
                     0,
                     wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL,
                     0 );

    
    

    gridSizer->Add(
        new wxStaticText(
            this, -1,
            TranslationManager::translate( "outbound_limit_label" ) ),
        0,
        wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL,
        0 );

    gridSizer->Add( mOutboundLimitField,
                     0,
                     wxEXPAND,
                     0 );
    
    gridSizer->Add(
        new wxStaticText(
            this, -1,
            TranslationManager::translate( "message_per_second_units" ) ),
        0,
        wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL,
        0 );

    

    gridSizer->Add( new wxStaticText( this, -1, "" ),
                    0,
                    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL,
                    0 );

    gridSizer->Add(
        new wxStaticText(
            this, -1,
            TranslationManager::translate( "no_limit_tip" ) ),
        0,
        wxALIGN_CENTER,
        0 );
    
    gridSizer->Add( new wxStaticText( this, -1, "" ),
                     0,
                     wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL,
                     0 );


    
    gridSizer->Add(
        new wxStaticText(
            this, -1,
            TranslationManager::translate( "initial_timeout_label" ) ),
        0,
        wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL,
        0 );

    gridSizer->Add( mDownloadTimeoutField,
                     1,
                     wxEXPAND,
                     0 );
    
    gridSizer->Add(
        new wxStaticText(
            this, -1,
            TranslationManager::translate( "seconds_unit" ) ),
        0,
        wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL,
        0 );


    
    gridSizer->Add(
        new wxStaticText(
            this, -1,
            TranslationManager::translate( "connection_at_least_label" ) ),
        0,
        wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL,
        0 );

    gridSizer->Add( mTargetConnectionCountField,
                     1,
                     wxEXPAND,
                     0 );
    
    gridSizer->Add(
        new wxStaticText(
            this, -1,
            TranslationManager::translate( "connections_unit" ) ),
        0,
        wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL,
        0 );


    gridSizer->Add(
        new wxStaticText(
            this, -1,
            TranslationManager::translate( "connection_at_most_label" ) ),
        0,
        wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL,
        0 );

    gridSizer->Add( mMaxConnectionCountField,
                     1,
                     wxEXPAND,
                     0 );
    
    gridSizer->Add(
        new wxStaticText(
            this, -1,
            TranslationManager::translate( "connections_unit" ) ),
        0,
        wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL,
        0 );


    gridSizer->Add(
        new wxStaticText(
            this, -1,
            TranslationManager::translate( "share_path_label" ) ),
        0,
        wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL,
        0 );
    
    gridSizer->Add( mSharePathField,
                     1,
                     wxEXPAND,
                     0 );

    wxButton *browseButton =
        new wxButton( this, BUTTON_BROWSE,
                      TranslationManager::translate( "browse_button" ) );

    
    gridSizer->Add( browseButton,
                    0,
                    wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL,
                    0 );

    

    wxButton *saveButton =
        new wxButton(
            this, BUTTON_SAVE_SETTINGS,
            TranslationManager::translate( "save_settings_button" ) );

    // add two spacers before button
    gridSizer->Add( new wxStaticText( this, -1, "" ),
                     0,
                     wxALIGN_LEFT,
                     0 );

    gridSizer->Add( new wxStaticText( this, -1, "" ),
                     0,
                     wxALIGN_LEFT,
                     0 );

    
    gridSizer->Add( saveButton,
                     0,
                     wxALIGN_CENTER,
                     0 );



    // check if we still need to ask user for a custom share path

    char valueFound;
    int haveGoodSharePathFlag =
        SettingsManager::getIntSetting( "haveGoodSharePath",
                                        &valueFound );

    if( valueFound && haveGoodSharePathFlag == 1 ) {
        // do nothing
        }
    else {
        // ask the user to specify a path

        #ifdef __WXMAC__
            // our browse box has no title in OS X, so show a message
            // box first to let the user know what the browse box is for

            wxMessageDialog *aboutToBrowseBox =
                new wxMessageDialog(
                    NULL,
                    TranslationManager::translate( "file_browse_explain" ),
                    TranslationManager::translate(
                        "first_startup_message_title" ),
                    wxOK );  // only OK button

            // wait for user OK
            aboutToBrowseBox->ShowModal();
            delete aboutToBrowseBox;
        #endif

            
        wxCommandEvent event( wxEVT_COMMAND_BUTTON_CLICKED,
                              BUTTON_BROWSE );
        OnBrowse( event );

        // never ask again
        SettingsManager::setSetting( "haveGoodSharePath", 1 );
        }
    }



SettingsPanel::~SettingsPanel() {
    }






void SettingsPanel::OnSaveSettings( wxCommandEvent& inEvent ) {

    char *naturalLanguageString = stringDuplicate(
        mNaturalLanguageChoiceBox->GetStringSelection().c_str() );
    
    char *inboundString =
        stringDuplicate( mInboundLimitField->GetValue().c_str() );
    char *outboundString =
        stringDuplicate( mOutboundLimitField->GetValue().c_str() );
    char *downloadTimeoutString =
        stringDuplicate( mDownloadTimeoutField->GetValue().c_str() );
    char *targetCountString =
        stringDuplicate( mTargetConnectionCountField->GetValue().c_str() );
    char *maxCountString =
        stringDuplicate( mMaxConnectionCountField->GetValue().c_str() );
    char *sharePath =
        stringDuplicate( mSharePathField->GetValue().c_str() );


    SettingsManager::setSetting( "language", naturalLanguageString );
    delete [] naturalLanguageString;

    double inbound;
    double outbound;
    int downloadTimeoutSeconds;
    int targetCount;
    int maxCount;

    int numRead = sscanf( inboundString, "%lf", &inbound );

    if( numRead == 1 && ( inbound > 0 || inbound == -1 ) ) {
        muteSetInboundMessagePerSecondLimit( inbound );
        }
    else {
        // bad entry, return field to correct value
        double value = muteGetInboundMessagePerSecondLimit();
        char *valueString = autoSprintf( "%f", value );

        mInboundLimitField->SetValue( valueString );

        delete [] valueString;
        }


    numRead = sscanf( outboundString, "%lf", &outbound );

    if( numRead == 1 && ( outbound > 0 || outbound == -1 )  ) {
        muteSetOutboundMessagePerSecondLimit( outbound );
        }
    else {
        // bad entry, return field to correct value
        double value = muteGetOutboundMessagePerSecondLimit();
        char *valueString = autoSprintf( "%f", value );

        mOutboundLimitField->SetValue( valueString );

        delete [] valueString;
        }


    numRead = sscanf( downloadTimeoutString, "%d", &downloadTimeoutSeconds );

    if( numRead == 1 && downloadTimeoutSeconds >= 0 ) {
        int downloadTimeoutMilliseconds = 1000 * downloadTimeoutSeconds;

        SettingsManager::setSetting( "downloadTimeoutMilliseconds",
                                     downloadTimeoutMilliseconds );
        }
    else {
        // bad entry, return field to correct value
        char found;
        int value =
            SettingsManager::getIntSetting( "downloadTimeoutMilliseconds",
                                            &found );
        if( !found || value < 0 ) {
            // default to 60 seconds
            value = 60000;
            }
        
        char *valueString = autoSprintf( "%d", value );

        mDownloadTimeoutField->SetValue( valueString );

        delete [] valueString;
        }

    

    numRead = sscanf( targetCountString, "%d", &targetCount );

    if( numRead == 1 && targetCount >= 0 ) {
        muteSetTargetNumberOfConnections( targetCount );
        }
    else {
        // bad entry, return field to correct value
        int value = muteGetTargetNumberOfConnections();
        char *valueString = autoSprintf( "%d", value );

        mTargetConnectionCountField->SetValue( valueString );

        delete [] valueString;
        }


    numRead = sscanf( maxCountString, "%d", &maxCount );

    if( numRead == 1 && maxCount >= 0 && maxCount >= targetCount ) {
        muteSetMaxNumberOfConnections( maxCount );
        }
    else {
        // bad entry, return field to correct value
        int value = muteGetMaxNumberOfConnections();
        char *valueString = autoSprintf( "%d", value );

        mMaxConnectionCountField->SetValue( valueString );

        delete [] valueString;
        }


    File *shareDirFile = new File( NULL, sharePath );

    if( shareDirFile->exists() ) {
        muteShareSetSharingPath( sharePath );
        }
    else {
        wxMessageDialog *badPathBox =
            new wxMessageDialog(
                NULL,
                TranslationManager::translate( "browse_error_message" ),
                TranslationManager::translate( "error_message_title" ),
                wxOK );  // only OK button

        // wait for user OK
        badPathBox->ShowModal();
        delete badPathBox;

        // reset back to good path
        char *goodPath = muteShareGetSharingPath();
        mSharePathField->SetValue( goodPath );

        delete [] goodPath;
        }

    delete shareDirFile;
    
    
    delete [] inboundString;
    delete [] outboundString;
    delete [] downloadTimeoutString;
    delete [] targetCountString;
    delete [] maxCountString;
    delete [] sharePath;
    }



void SettingsPanel::OnBrowse( wxCommandEvent& inEvent ) {

    char *sharePath =
        stringDuplicate( mSharePathField->GetValue().c_str() );

    if( !wxIsAbsolutePath( sharePath ) ) {
        char *cwd = stringDuplicate( wxGetCwd().c_str() );

        char pathDelim = Path::getDelimeter();
        // relative, make absolute
        char *absSharePath = autoSprintf( "%s%c%s",
                                          cwd, pathDelim, sharePath );

        delete [] sharePath;
        delete [] cwd;

        sharePath = absSharePath;
        }
        
    wxDirDialog *dirPicker =
        new wxDirDialog(
            this,
            TranslationManager::translate( "browse_window_title" ),
            sharePath );
    
    if( dirPicker->ShowModal() == wxID_OK ) {
        char *newSharePath = stringDuplicate( dirPicker->GetPath().c_str() );

        mSharePathField->SetValue( newSharePath );
        muteShareSetSharingPath( newSharePath );

        delete [] newSharePath;

        // force the settings to be saved
        OnSaveSettings( inEvent );
        }

    delete [] sharePath;
    }
