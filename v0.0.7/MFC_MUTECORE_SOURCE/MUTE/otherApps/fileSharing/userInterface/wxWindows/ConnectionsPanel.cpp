/*
 * Modification History
 *
 * 2003-October-28   Jason Rohrer
 * Created.
 *
 * 2003-November-5   Jason Rohrer
 * Fixed destruction/stopping behavior of update thread.
 *
 * 2004-February-20   Jason Rohrer
 * Added info about current connection attempt.
 *
 * 2004-April-2   Jason Rohrer
 * Fixed potential deadlock.
 * Removed use of wxMutexGuiEnter, which was causing freezes on win32.
 *
 * 2004-December-24   Jason Rohrer
 * Added a connection quality gauge.
 */



#include "ConnectionsPanel.h"

#include "minorGems/util/stringUtils.h"
#include "minorGems/util/TranslationManager.h"




// for non-precomp compilers, include the necessary headers
#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/thread.h"
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
#endif



#include "MUTE/otherApps/fileSharing/fileShare.h"
#include "MUTE/layers/messageRouting/messageRouter.h"



#include "minorGems/util/stringUtils.h"



// IDs for various controls
enum {
    BUTTON_ADD_HOST = 1,
    FIELD_ADD_HOST_ADDRESS,
    FIELD_ADD_HOST_PORT,
    CONNECTIONS_UPDATE_EVENT
    };



// event mapping table
BEGIN_EVENT_TABLE( ConnectionsPanel, wxPanel )
    EVT_BUTTON( BUTTON_ADD_HOST,  ConnectionsPanel::OnAddHost )
    EVT_TEXT_ENTER( FIELD_ADD_HOST_ADDRESS, ConnectionsPanel::OnAddHost )
    EVT_TEXT_ENTER( FIELD_ADD_HOST_PORT, ConnectionsPanel::OnAddHost )

    // dummy events sent by thread to trigger an update
    EVT_MENU( CONNECTIONS_UPDATE_EVENT, ConnectionsPanel::OnConnectionsUpdate )
END_EVENT_TABLE();



ConnectionsPanel::ConnectionsPanel( wxNotebook *inNotebook,
                                    wxGauge *inConnectionQualityGauge )
    : wxPanel( inNotebook, -1 ),
      mConnectionQualityGauge( inConnectionQualityGauge ),
      mAddHostAddressField( new wxTextCtrl( this, FIELD_ADD_HOST_ADDRESS,
                                            "", wxDefaultPosition,
                                            wxDefaultSize,
                                            wxTE_PROCESS_ENTER ) ),
      mAddHostPortField( new wxTextCtrl( this, FIELD_ADD_HOST_PORT,
                                            "4900", wxDefaultPosition,
                                            wxDefaultSize,
                                            wxTE_PROCESS_ENTER ) ),
      mConnectionList( new wxListCtrl( this, -1,
                                       wxDefaultPosition,
                                       wxDefaultSize,
                                       wxLC_REPORT | wxLC_VRULES ) ),
      mUpdateThread( NULL ) {

    
    wxBoxSizer *panelSizer = new wxBoxSizer( wxVERTICAL );

    this->SetSizer( panelSizer );


    wxBoxSizer *addFieldSizer = new wxBoxSizer( wxHORIZONTAL );

    panelSizer->Add( addFieldSizer,
                     0,
                     wxEXPAND | wxALL,
                     10 );

    addFieldSizer->Add(
        new wxStaticText(
            this, -1, TranslationManager::translate( "add_address_label" ) ),
        0,
        wxALIGN_CENTER,
        0 );

    addFieldSizer->Add( mAddHostAddressField,
                        1,
                        wxALIGN_CENTER,
                        0 );

    addFieldSizer->Add(
        new wxStaticText(
            this, -1, TranslationManager::translate( "add_port_label" ) ),
        0,
        wxALIGN_CENTER,
        0 );

    addFieldSizer->Add( mAddHostPortField,
                        0,
                        wxALIGN_CENTER,
                        0 );

    wxButton *addHostButton =
        new wxButton( this, BUTTON_ADD_HOST,
                      TranslationManager::translate( "add_host_button" ) );

    addFieldSizer->Add( addHostButton,
                        0,
                        wxALIGN_CENTER,
                        0 );

    
    

    mConnectionList->InsertColumn(
        0, TranslationManager::translate( "connections_header_address" ) );
    mConnectionList->InsertColumn(
        1, TranslationManager::translate( "connections_header_port" ) );
    mConnectionList->InsertColumn(
        2, TranslationManager::translate( "connections_header_sent" ) );
    mConnectionList->InsertColumn(
        3, TranslationManager::translate( "connections_header_queued" ) );
    mConnectionList->InsertColumn(
        4, TranslationManager::translate( "connections_header_dropped" ) );
    mConnectionList->SetColumnWidth( 0, 150 );
    mConnectionList->SetColumnWidth( 1, 75 );
    mConnectionList->SetColumnWidth( 2, 75 );
    mConnectionList->SetColumnWidth( 3, 75 );
    mConnectionList->SetColumnWidth( 4, 75 );
    
    panelSizer->Add( mConnectionList,
                     1,            // make vertically stretchable
                     wxEXPAND,     // make horizontally stretchable
                     0 );



    wxBoxSizer *statusSizer = new wxBoxSizer( wxHORIZONTAL );

    panelSizer->Add( statusSizer,
                     0,
                     wxEXPAND | wxALL,
                     0 );

    mConnectionStatus = new wxStaticText(
        this, -1,
        TranslationManager::translate( "connection_status_starting" ) );
    statusSizer->Add( mConnectionStatus,
                      0,
                      wxALIGN_CENTER,
                      0 );

    
    // start our thread
    mUpdateThread = new ConnectionUpdateThread( (void *)this );
    }



ConnectionsPanel::~ConnectionsPanel() {
    // make sure we unlock GUI mutex before joining thread to avoid
    // a deadlock (thread may be trying to post events)
    if( wxThread::IsMain() ) {
        wxMutexGuiLeave();
        }
    
    delete mUpdateThread;
    }






void ConnectionsPanel::OnConnectionsUpdate( wxCommandEvent& inEvent ) {

    mConnectionList->DeleteAllItems();


    char **addresses;
    int *ports;
    int *sentCounts;
    int *queueCounts;
    int *dropCounts;
    int numHosts = muteGetConnectedHostList( &addresses, &ports, &sentCounts,
                                             &queueCounts, &dropCounts );

    for( int i=0; i<numHosts; i++ ) {

        char *portString = autoSprintf( "%d", ports[i] );
        char *sentString = autoSprintf( "%d", sentCounts[i] );
        char *queueString = autoSprintf( "%d", queueCounts[i] );
        char *dropString = autoSprintf( "%d", dropCounts[i] );
        
        mConnectionList->InsertItem( i, addresses[i] );
        mConnectionList->SetItem( i, 1, portString );
        mConnectionList->SetItem( i, 2, sentString );
        mConnectionList->SetItem( i, 3, queueString );
        mConnectionList->SetItem( i, 4, dropString );

        delete [] addresses[i];
        delete [] portString;
        delete [] sentString;
        delete [] queueString;
        delete [] dropString;
        }

    delete [] addresses;
    delete [] ports;
    delete [] sentCounts;
    delete [] queueCounts;
    delete [] dropCounts;


    char *currentAttemptAddress;
    int currentAttemptPort;
    char attempting = muteGetCurrentConnectionAttempt( &currentAttemptAddress,
                                                       &currentAttemptPort );

    if( attempting ) {
        char *statusString = autoSprintf(
            TranslationManager::translate( "connection_status_trying" ),
            currentAttemptAddress, currentAttemptPort );
        delete [] currentAttemptAddress;

        mConnectionStatus->SetLabel( statusString );

        delete [] statusString;
        }
    else {
        mConnectionStatus->SetLabel( "" );
        }

    mConnectionQualityGauge->SetRange( muteGetTargetNumberOfConnections() );
    mConnectionQualityGauge->SetValue( muteGetConnectionCount() );
    }



void ConnectionsPanel::OnAddHost( wxCommandEvent& inEvent ) {
    char *address =
        stringDuplicate( mAddHostAddressField->GetValue().c_str() );
    char *portString =
        stringDuplicate( mAddHostPortField->GetValue().c_str() );

    if( strcmp( address, "" ) != 0 ) {

        // port defaults to 4900
        int port = 4900;
        int numRead = sscanf( portString, "%d", &port );

        if( numRead == 1 ) {

            // clear address field
            mAddHostAddressField->SetValue( "" );

            // leave port field

            }
        else {
            // replace bad port string with default
            mAddHostPortField->SetValue( "4900" );
            }

        muteAddHost( address, port );            
        }

    delete [] address;
    delete [] portString;    
    }



ConnectionUpdateThread::ConnectionUpdateThread( void *inParentPanel )
    : mParentPanel( inParentPanel ),
      mStopLock( new MutexLock() ),
      mStopped( false ) {

    start();
    }



ConnectionUpdateThread::~ConnectionUpdateThread() {

    mStopLock->lock();
    mStopped = true;
    mStopLock->unlock();

    
    join();

    delete mStopLock;
    }



void ConnectionUpdateThread::run() {
    mStopLock->lock();
    char isStopped = mStopped;
    mStopLock->unlock();

    ConnectionsPanel *panel = (ConnectionsPanel *)mParentPanel;
    
    while( ! isStopped ) {

        // fire an update event

        // post event so that GUI thread can update UI
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED,
                              CONNECTIONS_UPDATE_EVENT );
        
        // send in a thread-safe way
        wxPostEvent( panel, event );

        // sleep for 5 seconds in 1-second increments
        for( int i=0; i<5 && !isStopped; i++ ) {
            sleep( 1000 );


            // check if stopped
            mStopLock->lock();
            isStopped = mStopped;
            mStopLock->unlock();
            }
        }
    }


