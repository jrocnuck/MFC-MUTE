/*
 * Modification History
 *
 * 2003-October-24   Jason Rohrer
 * Created.
 *
 * 2003-October-26   Jason Rohrer
 * Got random seeding and key generation working.
 *
 * 2003-October-28   Jason Rohrer
 * Added a panel of active connections.
 *
 * 2003-November-4   Jason Rohrer
 * Added a settings panel.
 * Added an uploads panel.
 *
 * 2003-November-5   Jason Rohrer
 * Improved quitting behavior.
 * Fixed bugs in ending modal dialog boxes from threads.
 *
 * 2003-November-6   Jason Rohrer
 * Increased main window size.
 *
 * 2003-November-9   Jason Rohrer
 * Added check for active downloads before quit.
 *
 * 2003-November-11   Jason Rohrer
 * Changed custom dialogs to dynamically adjust their size.
 * Improved dialog messages/titles.
 * Changed mechanism for blocking dialogs while worker threads run.
 *
 * 2003-November-16   Jason Rohrer
 * Added runtime toggle for nice quit feature.
 *
 * 2003-November-18   Jason Rohrer
 * Added runtime setting for logging level.
 *
 * 2003-December-1   Jason Rohrer
 * Added mac-specific code for setting the working directory.
 *
 * 2003-December-7   Jason Rohrer
 * Fixed a memory leak.
 *
 * 2003-December-19   Jason Rohrer
 * Added an extra check when processing showNiceQuit setting.
 *
 * 2003-December-21   Jason Rohrer
 * Fixed quit menu issues on OS X.
 *
 * 2003-December-30   Jason Rohrer
 * Fixed so that a saved random seed is used at most once.
 *
 * 2004-January-2   Jason Rohrer
 * Added startup question about firewall.
 *
 * 2004-April-2   Jason Rohrer
 * Fixed potential deadlocks.
 * Removed use of wxMutexGuiEnter, which was causing freezes on win32.
 *
 * 2004-April-2   Jason Rohrer
 * Fixed potential deadlocks.
 *
 * 2004-October-7   Jason Rohrer
 * Added translation manager.
 *
 * 2004-December-24   Jason Rohrer
 * Added a connection quality gauge.
 */


// includes and definitions copied from wxWindows sample calendar app

#if defined(__GNUG__) && !defined(__APPLE__)
    #pragma implementation "GuiApp.cpp"
    #pragma interface "GuiApp.cpp"
#endif


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers
#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/thread.h"
    #include "wx/frame.h"
//  #include "wx/panel.h"
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
    #include "wx/button.h"
    #include "wx/gauge.h"
#endif


// Custom GUI component includes
#include "SearchPanel.h"
#include "DownloadPanel.h"
#include "ConnectionsPanel.h"
#include "SettingsPanel.h"
#include "UploadsPanel.h"



// MUTE includes
#include "MUTE/layers/messageRouting/messageRouter.h"
#include "MUTE/otherApps/fileSharing/fileShare.h"
#include "MUTE/common/CryptoUtils.h"

#include "minorGems/util/SettingsManager.h"
#include "minorGems/util/TranslationManager.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/system/Thread.h"
#include "minorGems/util/log/AppLog.h"



#include <unistd.h>



// class definitions



char showOnlyForceQuit = false;


/**
 * Entry point for the app.
 */
class GuiApp : public wxApp {

    public:

        // override the default init function
        virtual bool OnInit();

        // override the default exit function
        virtual int OnExit();
    };



/**
 * The main window.
 */
class GuiFrame : public wxFrame {

    public:

        /**
         * Contstructs a frame.
         */
        GuiFrame( const wxString& inTitle,
                  const wxPoint& inPosition,
                  const wxSize& inSize );


        
    private:
        
        // event handler
        void OnQuit( wxCommandEvent& inEvent );
        void OnForceQuit( wxCommandEvent& inEvent );
        

        
        // register frame to handle events 
        DECLARE_EVENT_TABLE();

        
        SearchPanel *mSearchPanel;
        DownloadPanel *mDownloadPanel;
        ConnectionsPanel *mConnectionsPanel;
        SettingsPanel *mSettingsPanel;
        UploadsPanel *mUploadsPanel;

        wxGauge *mConnectionQualityGauge;
    };



// set GuiApp as the app to launch when the program starts
IMPLEMENT_APP( GuiApp );


// IDs for various controls
enum {
    MENU_QUIT = 1,
    MENU_FORCE_QUIT
    };


// event mapping table
BEGIN_EVENT_TABLE( GuiFrame, wxFrame )
    EVT_MENU( MENU_QUIT,  GuiFrame::OnQuit )
    EVT_MENU( MENU_FORCE_QUIT,  GuiFrame::OnForceQuit )
END_EVENT_TABLE();


char *getRandomSeed() {

    
    wxTextEntryDialog *randomnessDialog =
        new wxTextEntryDialog( NULL,
                               "Enter some randomness",
                               "Encryption Seed",
                               "",
                               wxOK );

    randomnessDialog->ShowModal();

    char *returnValue =
        stringDuplicate( randomnessDialog->GetValue().c_str() );
    
    delete randomnessDialog;

    return returnValue;
    }



// gets input from user
// parameters and return value must be destroyed by caller
char *getUserString( const char *inWindowTitle, const char *inPrompt ) {


    wxTextEntryDialog *randomnessDialog =
        new wxTextEntryDialog( NULL,
                               inPrompt,
                               inWindowTitle,
                               "",
                               wxOK | wxCENTRE );

    randomnessDialog->ShowModal();

    char *returnValue =
        stringDuplicate( randomnessDialog->GetValue().c_str() );
    
    delete randomnessDialog;

    return returnValue;
    }


/**
 * Dialog that is displayed while key generation happens.
 */
class KeyGenDialog : public wxDialog {

    public:

        KeyGenDialog() 
            : wxDialog( NULL, -1,
                        TranslationManager::translate(
                            "first_startup_message_title" ),
                        wxDefaultPosition, wxDefaultSize,
                        wxDEFAULT_DIALOG_STYLE ) {

            mStartTime = time( NULL );
            mSizer = new wxGridSizer( 1 );
            SetSizer( mSizer );
        
            mTextMessage =
                new wxStaticText( this, -1,
                                  TranslationManager::translate(
                                      "key_generation_message" ) );
            mSizer->Add(
                mTextMessage, 0,
                wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL | wxALL |
                wxADJUST_MINSIZE,
                20 );


            mOkButton =
                new wxButton( this, wxID_OK,
                              "Ok" );    
            mSizer->Add(
                mOkButton, 0,
                wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL | wxALL |
                wxADJUST_MINSIZE,
                20 );

            mOkButton->Enable( false );
            
            // resize dialog to fit message
            mSizer->SetSizeHints( this );
            mSizer->Fit( this );
            Centre( wxBOTH );
            }

        
    protected:
        wxStaticText *mTextMessage;
        wxButton *mOkButton;
        wxGridSizer *mSizer;
        
        unsigned long mStartTime;
        
        // event handler
        // gets dummy event when thread is done
        void OnDone( wxCommandEvent& inEvent ) {
            long netTime = time( NULL ) - mStartTime;

            char *message = autoSprintf(
                TranslationManager::translate( "key_generation_done_message" ),
                netTime );
            
            mTextMessage->SetLabel( message );

            delete [] message;
            
            mOkButton->Enable( true );

            // resize/center to accommodate the new message
            mSizer->SetSizeHints( this );
            mSizer->Fit( this );
            Centre( wxBOTH );
            }

        
        // register frame to handle events 
        DECLARE_EVENT_TABLE();
        
    };


enum {
    DUMMY_KEY_GEN_DONE_EVENT = 1
    };



// event mapping table
BEGIN_EVENT_TABLE( KeyGenDialog, wxDialog )
    EVT_BUTTON( DUMMY_KEY_GEN_DONE_EVENT,  KeyGenDialog::OnDone )
END_EVENT_TABLE();




/**
 * Thread that generates and RSA key pair in the background behind
 * a modal dialog box.
 */
class KeyGenThread : public Thread {

    public:

        /**
         * Constructs and starts a thread.
         *
         * @param inKeyLength the RSA key length in bits.
         * @param inDialog the modal dialog to send the end signal to
         *   when finished.
         */
        KeyGenThread( int inKeyLength,
                      wxDialog *inDialog )
            : mKeyLength( inKeyLength ),
              mNodePublicKey( NULL ),
              mNodePrivateKey( NULL ),
              mDialog( inDialog ) {

            start();
            }

        
        
        ~KeyGenThread() {
            delete [] mNodePublicKey;
            delete [] mNodePrivateKey;
            }

        

        // implements the Thread interface
        // when thread returns from run method (if joined)
        // then the key has been generated and is available.
        void run() {
            CryptoUtils::generateRSAKey( mKeyLength, &mNodePublicKey,
                                         &mNodePrivateKey );

            // close the modal dialog box

            // post event to the modal dialog box
            // so that the GUI thread can update UI
            wxCommandEvent event( wxEVT_COMMAND_BUTTON_CLICKED,
                                  DUMMY_KEY_GEN_DONE_EVENT );
        
            // send in a thread-safe way
            wxPostEvent( mDialog, event );


            // calling EndModal from this thread is not safe
            // mDialog->EndModal( wxID_OK );
            }


        
        /**
         * Gets the public key.
         *
         * @return the public key.
         *   Must be destroyed by caller.
         */
        char *getNodePublicKey() {
            return stringDuplicate( mNodePublicKey );
            }


        
        /**
         * Gets the private key.
         *
         * @return the private key.
         *   Must be destroyed by caller.
         */
        char *getNodePrivateKey() {
            return stringDuplicate( mNodePrivateKey );
            }


    protected:
        int mKeyLength;
        char *mNodePublicKey;
        char *mNodePrivateKey;
        wxDialog *mDialog;
            
    };



bool GuiApp::OnInit() {


    // ack... platform-specific code
    // but the Mac's app launcher is odd...
    #ifdef __WXMAC__
        // make sure working directory is the same as the directory
        // that the app resides in
        // this is especially important on the mac platform, which
        // doesn't set a proper working directory for double-clicked
        // app bundles

        // app arguments provided as a wxApp member
        // arg 0 is the path to the app executable
        char *appDirectoryPath = stringDuplicate( wxApp::argv[0] );
    
        char *appNamePointer = strstr( appDirectoryPath,
                                       "fileSharingMUTE.app" );
        
        // terminate full app path to get parent directory
        appNamePointer[0] = '\0';
        
        chdir( appDirectoryPath );
        
        delete [] appDirectoryPath;
        
    #endif

        
    char *languageName = SettingsManager::getStringSetting( "language" );

    if( languageName == NULL ) {
        // default
        languageName = stringDuplicate( "English" );
        }
    TranslationManager::setLanguage( languageName );

    delete [] languageName;

    
    
    // check if firewall setting exists
    char valueFound;
    SettingsManager::getIntSetting( "behindFirewall",
                                    &valueFound );
    if( ! valueFound ) {
        // ask user about firewall status
        int answer =
            wxMessageBox( TranslationManager::translate( "firewall_question" ),
                          TranslationManager::translate(
                              "first_startup_message_title" ),
                          wxYES_NO );
        
        // save the firewall setting
        if( answer == wxYES ) {
            SettingsManager::setSetting( "behindFirewall", 1 );
            }
        else {
            SettingsManager::setSetting( "behindFirewall", 0 );
            }
        }

    
    
    char *randomSeed = SettingsManager::getStringSetting( "randomSeed" );
    
    if( randomSeed == NULL ) {
        randomSeed = getUserString(
            TranslationManager::translate( "gathering_randomness_title" ),
            TranslationManager::translate( "gathering_randomness_message" ) );
        }
    else {
        printf( "Using randomness saved from last time\n" );
        }

    muteSeedRandomGenerator( randomSeed );
    delete [] randomSeed;

    
    // we've used this seed, so mark it as blank
    // (will be NULL if getStringSetting called again) 
    SettingsManager::setSetting( "randomSeed", "" );

    

    int portNumber = SettingsManager::getIntSetting( "port", &valueFound );

    if( ! valueFound ) {
        portNumber = 4900;
        }



    // make sure node RSA keys exist before starting node
    // the node will generate keys itself upon startup, but then
    // we cannot display a proper progress dialog
    
    char *nodePublicKey =
        SettingsManager::getStringSetting( "nodePublicKey" );
    char *nodePrivateKey =
        SettingsManager::getStringSetting( "nodePrivateKey" );

    if( nodePublicKey == NULL || strcmp( nodePublicKey, "" ) == 0 ||
        nodePrivateKey == NULL || strcmp( nodePrivateKey, "" ) == 0 ) {

        if( nodePublicKey != NULL ) {
            delete [] nodePublicKey;
            }
        if( nodePrivateKey != NULL ) {
            delete [] nodePrivateKey;
            }

        char keyLengthFound;
        int keyLength = SettingsManager::getIntSetting( "nodeKeySize",
                                                        &keyLengthFound );

        if( !keyLengthFound ) {
            // ask user about key size
            const wxString choices[] =
                { TranslationManager::translate( "512_bit_choice" ),
                  TranslationManager::translate( "1024_bit_choice" ),
                  TranslationManager::translate( "1536_bit_choice" ),
                  TranslationManager::translate( "2048_bit_choice" ),
                  TranslationManager::translate( "4096_bit_choice" ) };
            
            wxSingleChoiceDialog *keyChoiceBox =
                new wxSingleChoiceDialog(
                    NULL,
                    TranslationManager::translate( "select_key_size_message" ),
                    TranslationManager::translate(
                        "first_startup_message_title" ),
                    5,
                    choices,
                    NULL,      // no client data
                    wxOK | wxCENTRE | wxCAPTION );
            
            // defaults to 1024
            keyChoiceBox->SetSelection( 1 );

            keyChoiceBox->ShowModal();

            int selection = keyChoiceBox->GetSelection();

            switch( selection ) {
                case 0:
                    keyLength = 512;
                    break;
                case 1:
                    keyLength = 1024;
                    break;
                case 2:
                    keyLength = 1536;
                    break;
                case 3:
                    keyLength = 2048;
                    break;
                case 4:
                    keyLength = 4096;
                    break;
                default:
                    keyLength = 1024;
                    break;
                }

            delete keyChoiceBox;
            }
        
        KeyGenDialog *generatingBox = new KeyGenDialog();

        KeyGenThread *thread = new KeyGenThread( keyLength, generatingBox );
        
        // block until thread enables Ok button and user closes box
        generatingBox->ShowModal();
        delete generatingBox;

        // make sure we unlock GUI mutex before joining thread to avoid
        // a deadlock (thread may be trying to post events)
        if( wxThread::IsMain() ) {
            wxMutexGuiLeave();
            }

        thread->join();

        nodePublicKey = thread->getNodePublicKey();
        nodePrivateKey = thread->getNodePrivateKey();
        
        delete thread;        
        
        
        SettingsManager::setSetting( "nodePublicKey", nodePublicKey );
        SettingsManager::setSetting( "nodePrivateKey", nodePrivateKey );
        }

    delete [] nodePublicKey;
    delete [] nodePrivateKey;

    muteStart( portNumber );
        
    muteShareStart();


    char *logLevelString = SettingsManager::getStringSetting( "logLevel" );

    if( logLevelString != NULL ) {
        // change the default log level

        int newLogLevel = -1;
        
        if( strcmp( logLevelString, "DEACTIVATE_LEVEL" ) == 0 ) {
            newLogLevel = Log::DEACTIVATE_LEVEL;
            }
        else if( strcmp( logLevelString, "CRITICAL_ERROR_LEVEL" ) == 0 ) {
            newLogLevel = Log::CRITICAL_ERROR_LEVEL;
            }
        else if( strcmp( logLevelString, "ERROR_LEVEL" ) == 0 ) {
            newLogLevel = Log::ERROR_LEVEL;
            }
        else if( strcmp( logLevelString, "WARNING_LEVEL" ) == 0 ) {
            newLogLevel = Log::WARNING_LEVEL;
            }
        else if( strcmp( logLevelString, "INFO_LEVEL" ) == 0 ) {
            newLogLevel = Log::INFO_LEVEL;
            }
        else if( strcmp( logLevelString, "DETAIL_LEVEL" ) == 0 ) {
            newLogLevel = Log::DETAIL_LEVEL;
            }
        else if( strcmp( logLevelString, "TRACE_LEVEL" ) == 0 ) {
            newLogLevel = Log::TRACE_LEVEL;
            }

        if( newLogLevel != -1 ) {
            AppLog::setLoggingLevel( newLogLevel );
            }

        delete [] logLevelString;
        }

    
    GuiFrame *frame = new GuiFrame(
        TranslationManager::translate( "main_window_title" ),
        wxPoint( 50, 50 ),
        wxSize( 700, 550 ) );

    
    frame->Show( true );

    SetTopWindow( frame );
    
    return true;
    }



GuiFrame::GuiFrame( const wxString& inTitle,
                    const wxPoint& inPosition,
                    const wxSize& inSize )
    : wxFrame( (wxFrame *)NULL, -1, inTitle, inPosition, inSize ) {


    wxMenuBar *frameMenuBar = new wxMenuBar();


    char valueFound;
    int showNiceQuitFlag =
        SettingsManager::getIntSetting( "showNiceQuit",
                                        &valueFound );
        
    if( valueFound && showNiceQuitFlag == 1 ) {
        showOnlyForceQuit = false;
        }
    else {
        showOnlyForceQuit = true;
        }

    
    // don't show file menu on OS X
    // since the file menu only contains "Quit", and the the app-specific
    // menu already has a quit item
    #ifndef __WXMAC__
        wxMenu *fileMenu = new wxMenu();

        if( ! showOnlyForceQuit ) {
            // show both "Quit" (nice) and "Force Quit"
            
            // Q is accellerator
            fileMenu->Append(
                MENU_QUIT,
                TranslationManager::translate( "quit_menu_item" ) );

            fileMenu->Append(
                MENU_FORCE_QUIT,
                TranslationManager::translate( "force_quit_menu_item" ) );
            }
        else {
            // show only "Quit" (force quit)
            fileMenu->Append(
                MENU_FORCE_QUIT,
                TranslationManager::translate( "quit_menu_item" ) );
            }

        frameMenuBar->Append( fileMenu,
                              TranslationManager::translate( "file_menu" ) );
    #endif

    
    SetMenuBar( frameMenuBar );

    wxBoxSizer *frameSizer = new wxBoxSizer( wxVERTICAL );
    this->SetSizer( frameSizer );


    // gauge is passed into ConnectionsPanel, so construct it here
    mConnectionQualityGauge = new wxGauge( this, -1, 10,
                                       wxDefaultPosition, wxDefaultSize,
                                       wxGA_HORIZONTAL | wxGA_SMOOTH );
    mConnectionQualityGauge->SetValue( 0 );

    
    wxNotebook *notebook = new wxNotebook( this, -1 );


    // download panel is second page in notebook
    // index = 1
    mDownloadPanel = new DownloadPanel( notebook,
                                        1 );
    
    mSearchPanel = new SearchPanel( notebook, mDownloadPanel );

    mUploadsPanel = new UploadsPanel( notebook );
    
    mConnectionsPanel = new ConnectionsPanel( notebook,
                                              mConnectionQualityGauge );
    mSettingsPanel = new SettingsPanel( notebook );
    
    notebook->AddPage( mSearchPanel,
                       TranslationManager::translate( "search_tab" ) );
    notebook->AddPage( mDownloadPanel,
                       TranslationManager::translate( "downloads_tab" ) );
    notebook->AddPage( mUploadsPanel,
                       TranslationManager::translate( "uploads_tab" ) );
    notebook->AddPage( mConnectionsPanel,
                       TranslationManager::translate( "connections_tab" ) );
    notebook->AddPage( mSettingsPanel,
                       TranslationManager::translate( "settings_tab" ) );

    frameSizer->Add( notebook,
                     1,            // make vertically stretchable
                     wxEXPAND |    // make horizontally stretchable
                     wxALL,        //   and make border all around
                     2 );          // set border width to 2

    
    wxBoxSizer *statusSizer = new wxBoxSizer( wxHORIZONTAL );

    frameSizer->Add( statusSizer,
                     0,
                     wxEXPAND | wxALL,
                     2 );

     wxStaticText *testLabel =
         new wxStaticText( this, -1,
                           TranslationManager::translate(
                               "connection_quality_label" ) );

     statusSizer->Add( testLabel,
                       0,     // default size
                       wxALIGN_CENTER,
                       2 );
     
     statusSizer->Add( mConnectionQualityGauge,
                       1,       //  1/4 of remaining space
                       wxALIGN_CENTER,
                       2 );

     // add one more empty panel for spacing
     wxBoxSizer *statusSpacer = new wxBoxSizer( wxVERTICAL );

     statusSizer->Add( statusSpacer,
                       3,       //  3/4 of remaining space
                       wxALIGN_CENTER,
                       2 );

    //frameSizer->Fit( this );
    }



/**
 * Dialog that is displayed while MUTE shutdown happens.
 */
class ShutdownDialog : public wxDialog {

    public:

        ShutdownDialog() 
            : wxDialog( NULL, -1,
                        TranslationManager::translate( "quit_message_title" ),
                        wxDefaultPosition, wxDefaultSize,
                        wxDEFAULT_DIALOG_STYLE ) {
            mSizer = new wxGridSizer( 1 );
            SetSizer( mSizer );
        
            mTextMessage = new wxStaticText(
                this, -1,
                TranslationManager::translate( "shutdown_waiting_message" ) );
            
            mSizer->Add(
                mTextMessage, 0,
                wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL | wxALL |
                wxADJUST_MINSIZE,
                20 );


            wxGridSizer *buttonSizer = new wxGridSizer( 2 );

            mSizer->Add(
                buttonSizer, 0,
                wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL | wxADJUST_MINSIZE );

            
            mOkButton = new wxButton(
                this, wxID_OK,
                TranslationManager::translate( "quit_ok_button" ) );    

            mForceQuitButton = new wxButton(
                this, wxID_CANCEL,
                TranslationManager::translate( "quit_force_quit_button" ) );

            buttonSizer->Add(
                mOkButton, 0,
                wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL | wxALL |
                wxADJUST_MINSIZE,
                20 );
            buttonSizer->Add(
                mForceQuitButton, 0,
                wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL | wxALL |
                wxADJUST_MINSIZE,
                20 );

            mOkButton->Enable( false );
            
            // resize dialog to fit message
            mSizer->SetSizeHints( this );
            mSizer->Fit( this );
            Centre( wxBOTH );
            }

        
    protected:
        wxStaticText *mTextMessage;
        wxButton *mOkButton;
        wxButton *mForceQuitButton;
        wxGridSizer *mSizer;
        
        // event handler
        // gets dummy event when thread is done
        void OnDone( wxCommandEvent& inEvent ) {
            
            mTextMessage->SetLabel(
                TranslationManager::translate( "shutdown_done_message" ) );
            
            mOkButton->Enable( true );
            mForceQuitButton->Enable( false );

            // don't resize box, since new message is smaller than original
            }

        
        // register frame to handle events 
        DECLARE_EVENT_TABLE();
        
    };


enum {
    DUMMY_SHUTDOWN_DONE_EVENT = 1
    };



// event mapping table
BEGIN_EVENT_TABLE( ShutdownDialog, wxDialog )
    EVT_BUTTON( DUMMY_SHUTDOWN_DONE_EVENT,  ShutdownDialog::OnDone )
END_EVENT_TABLE();



/**
 * Thread that shuts down this MUTE node in the background behind
 * a modal dialog box.
 */
class ShutDownThread : public Thread {

    public:

        /**
         * Constructs and starts a thread.
         * @param inDialog the modal dialog to send the end signal to
         *   when finished.
         */
        ShutDownThread( wxDialog *inDialog )
            : mDialog( inDialog ) {

            start();
            }

        
        
        ~ShutDownThread() {
            }

        

        // implements the Thread interface
        // when thread returns from run method (if joined)
        // then all layers of this MUTE node will have been shut down.
        void run() {

            
            printf( "Stopping file sharing layer\n" );
            muteShareStop();
            
            printf( "Saving randomness for use at next startup\n" );
            char *randState = muteGetRandomGeneratorState();
            
            SettingsManager::setSetting( "randomSeed", randState );
            delete [] randState;
            
            printf( "Stopping message routing layer\n" );
            muteStop();
    

            printf( "All layers are stopped, exiting.\n" );

            
            // close the modal dialog box
            
            // post event to the modal dialog box
            // so that the GUI thread can update UI
            wxCommandEvent event( wxEVT_COMMAND_BUTTON_CLICKED,
                                  DUMMY_SHUTDOWN_DONE_EVENT );
        
            // send in a thread-safe way
            wxPostEvent( mDialog, event );

            
            // calling EndModal from this thread is not safe
            // mDialog->EndModal( wxID_OK );
            }


    protected:
        wxDialog *mDialog;
            
    };



void GuiFrame::OnQuit( wxCommandEvent& WXUNUSED( event ) ) {
    if( mDownloadPanel->areDownloadsActive() ) {
        // don't quit while downloads are active

        wxMessageDialog *cannotQuitBox = new wxMessageDialog(
            NULL,
            TranslationManager::translate(
                "quit_blocked_by_downloads_message" ),
            TranslationManager::translate( "quit_message_title" ),
            wxOK );  // only OK button

        // wait for user OK before starting
        cannotQuitBox->ShowModal();
        delete cannotQuitBox;
        }
    else {
        mSearchPanel->cancelActiveSearch();
        
        this->Close( TRUE );
        }
    }



void GuiFrame::OnForceQuit( wxCommandEvent& WXUNUSED( event ) ) {

    if( showOnlyForceQuit ) {
        // this is their only way to quit

        // at least save the random seed
        printf( "Saving randomness for use at next startup\n" );
        char *randState = muteGetRandomGeneratorState();
        
        SettingsManager::setSetting( "randomSeed", randState );
        delete [] randState;
        }
    // otherwise, just exit
    
    exit( 0 );
    }



int GuiApp::OnExit() {

    // on OS X, the OS puts its own Quit menu item in an app-specific
    // menu (in addition to the one in the file menu).
    // This apparently calls OnExit directly
    // We still want to let this force-quit if it's set
    if( showOnlyForceQuit ) {

        // at least save the random seed
        printf( "Saving randomness for use at next startup\n" );
        char *randState = muteGetRandomGeneratorState();
        
        SettingsManager::setSetting( "randomSeed", randState );
        delete [] randState;
        }
    else {
        // nice quit
        
        ShutdownDialog *shuttingDownMessageBox = new ShutdownDialog();

        // start shutdown thread
        ShutDownThread *thread = new ShutDownThread( shuttingDownMessageBox );

        // wait for user Cancel, or for thread to enable Ok button and user
        // to click Ok
        int result = shuttingDownMessageBox->ShowModal();
        delete shuttingDownMessageBox;
        
        
        // if thread closes box, it sends OK event
        if( result == wxID_OK ) {

            // make sure we unlock GUI mutex before joining thread to avoid
            // a deadlock (thread may be trying to post events)
            if( wxThread::IsMain() ) {
                wxMutexGuiLeave();
                }

            thread->join();

            delete thread;
            }
        else {
            // force-quit from the shutdown box
            // do not join/delete thread
            // this is a memory leak
            // (along with whatever MUTE layers have not been
            // shut down yet), but when force-quitting, we can ignore leaks 
            }
        }

    return 0;
    }



