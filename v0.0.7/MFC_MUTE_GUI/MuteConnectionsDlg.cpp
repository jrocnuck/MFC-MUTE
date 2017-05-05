// MuteConnectionsDlg.cpp : implementation file
//

/***************************************
- 01-26-2005 --> Added Nate's SaveConnections function
    to save the connections at close to the seedHosts.ini file
	for what ever it's worth.. 
***************************************/

#include "stdafx.h"
#include "winsock2.h"
#include "MFC_MUTE_GUI.h"
#include "MuteConnectionsDlg.h"

#include "minorGems/util/stringUtils.h"

#include "MUTE/layers/messageRouting/messageRouter.h"
#include "MUTE/otherApps/fileSharing/userInterface/common/formatUtils.h"
// from NATE's seedHost Updater!
#include "MUTE/otherApps/fileSharing/fileShare.h"
#include "MUTE/layers/messageRouting/messageRouter.h"
#include "minorGems/network/p2pParts/HostCatcher.h"
#include "minorGems/util/SettingsManager.h"
#include "minorGems/util/SimpleVector.h"
#include "minorGems/util/StringBufferOutputStream.h"

#include "MFC_MUTE_GUIDlg.h"
#include "ColorNames.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern HostCatcher *muteHostCatcher; // from HostCatcher.cpp


/////////////////////////////////////////////////////////////////////////////
// CMuteConnectionsDlg dialog


CMuteConnectionsDlg::CMuteConnectionsDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CMuteConnectionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMuteConnectionsDlg)
	m_strAddressEdit = _T("");
	m_strPortEdit = _T("");
	m_ulLastNumOfConnections = 0;
	//}}AFX_DATA_INIT
}


void CMuteConnectionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMuteConnectionsDlg)
	DDX_Control( pDX, IDC_STATIC_ADDHOST_GROUP, m_oAddHostGroup );
	DDX_Control( pDX, IDC_STATIC_UPDATEHOSTS, m_oHostUpdateGroup );
	DDX_Control( pDX, IDC_STATIC_MYINFO, m_oMyInfoGroup );
	DDX_Control(pDX, IDC_STATIC_STATUS_STR, m_staticStatusText);
	DDX_Control(pDX, IDC_STATIC_IN_OUT_STATS_LABEL_LEFT_1, m_oInOutStatsLabelLeft1);
	DDX_Control(pDX, IDC_STATIC_IN_OUT_STATS_LABEL_LEFT_2, m_oInOutStatsLabelLeft2);
	DDX_Control(pDX, IDC_STATIC_IN_OUT_STATS_LABEL_LEFT_3, m_oInOutStatsLabelLeft3);
	DDX_Control(pDX, IDC_STATIC_IN_OUT_STATS_LABEL_LEFT_4, m_oInOutStatsLabelLeft4);
	DDX_Control(pDX, IDC_STATIC_IN_OUT_STATS_LABEL_LEFT_5, m_oInOutStatsLabelLeft5);

	DDX_Control(pDX, IDC_STATIC_IN_OUT_STATS_LABEL_RIGHT_1, m_oInOutStatsLabelRight1);
	DDX_Control(pDX, IDC_STATIC_IN_OUT_STATS_LABEL_RIGHT_2, m_oInOutStatsLabelRight2);
	DDX_Control(pDX, IDC_STATIC_IN_OUT_STATS_LABEL_RIGHT_3, m_oInOutStatsLabelRight3);
	DDX_Control(pDX, IDC_STATIC_IN_OUT_STATS_LABEL_RIGHT_4, m_oInOutStatsLabelRight4);
	DDX_Control(pDX, IDC_STATIC_IN_OUT_STATS_LABEL_RIGHT_5, m_oInOutStatsLabelRight5);


	DDX_Control(pDX, IDC_CONNECTION_LIST, m_ConnectionList);
	DDX_Text(pDX, IDC_ADDRESS_STRING_EDIT, m_strAddressEdit);
	DDX_Text(pDX, IDC_PORT_STRING_EDIT, m_strPortEdit);	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMuteConnectionsDlg, CResizableDialog)
	//{{AFX_MSG_MAP(CMuteConnectionsDlg)	
	ON_MESSAGE( IN_OUT_STATS_UPDATE_EVENT, OnInOutStatsUpdate )
	ON_BN_CLICKED(IDC_ADD_HOST_BUTTON, OnAddHostButton )
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMuteConnectionsDlg message handlers

BOOL CMuteConnectionsDlg::PreTranslateMessage(MSG* pMsg) 
{
	CWnd	*pWnd;
	if ( pMsg->message == WM_KEYDOWN )
	{
		if( pMsg->wParam == VK_RETURN )
		{
			pWnd = GetFocus();
			if( (GetDlgItem(IDC_ADDRESS_STRING_EDIT) == pWnd) || (GetDlgItem(IDC_PORT_STRING_EDIT) == pWnd) )
			{
				OnAddHostButton();
				return FALSE;
			}
			else
			{
				return FALSE;
			}
		}
		if( pMsg->wParam == VK_ESCAPE )
		{
			// Don't close the window if the user hits the ESC key.
			return FALSE;
		}
	}
	
	return CResizableDialog::PreTranslateMessage(pMsg);
}


/*
//*******************************************************************
//  FUNCTION:    -	OnClose
//  RETURNS:     -	
//  PARAMETERS:  -	
//  COMMENTS:	 -	
//*******************************************************************
*/
void CMuteConnectionsDlg::OnClose()
{	
	KillTimer(667);
	SaveConnections();
	Sleep(1000); // make sure any previous calls to update complete
}

/*
//*******************************************************************
//  FUNCTION:    -	OnTimer()
//  RETURNS:     -	
//  PARAMETERS:  -	
//  COMMENTS:	 -	
//*******************************************************************
*/
void CMuteConnectionsDlg::OnTimer(UINT nIDEvent) 
{
	if( 667 == nIDEvent )
	{	      
		KillTimer( 667 );
		OnConnectionsUpdate();
		SetTimer( 667, 5000, NULL );
	}
	else if( 668 == nIDEvent )
	{
		if( IsWindow( GetSafeHwnd() ) )
		{
			PostMessage( IN_OUT_STATS_UPDATE_EVENT, 0, 0 );
		}
	}
	
	CResizableDialog::OnTimer(nIDEvent);
}

/*
//*******************************************************************
//  FUNCTION:    -	OnInitDialog
//  RETURNS:     -	
//  PARAMETERS:  -	
//  COMMENTS:	 -	
//*******************************************************************
*/
BOOL CMuteConnectionsDlg::OnInitDialog() 
{
	CString	strTemp;
	CResizableDialog::OnInitDialog();

	ShowSizeGrip(FALSE);

	AddAnchor( IDC_CONNECTION_LIST, TOP_LEFT, BOTTOM_RIGHT );
	AddAnchor( IDC_STATIC_STATUS_STR, BOTTOM_LEFT );		
	AddAnchor( IDC_STATIC_ADDRESS, TOP_RIGHT );
	AddAnchor( IDC_ADDRESS_STRING_EDIT, TOP_RIGHT );
	AddAnchor( IDC_STATIC_PORT, TOP_RIGHT );
	AddAnchor( IDC_PORT_STRING_EDIT, TOP_RIGHT );
	AddAnchor( IDC_ADD_HOST_BUTTON, TOP_RIGHT );	
	AddAnchor( IDC_DD2, TOP_RIGHT );
	AddAnchor( IDC_HOSTWEBCACHEURL, TOP_RIGHT );
	AddAnchor( IDC_UPDATEHOSTWEBCACHEFROMURL, TOP_RIGHT );
	AddAnchor( m_oAddHostGroup, TOP_RIGHT );
	AddAnchor( m_oHostUpdateGroup, TOP_RIGHT );
	AddAnchor( m_oMyInfoGroup, TOP_RIGHT );	
	AddAnchor( IDC_STATIC_IN_OUT_STATS_LABEL, TOP_RIGHT );	

	AddAnchor( m_oInOutStatsLabelLeft1, TOP_RIGHT );
	AddAnchor( m_oInOutStatsLabelLeft2, TOP_RIGHT );
	AddAnchor( m_oInOutStatsLabelLeft3, TOP_RIGHT );
	AddAnchor( m_oInOutStatsLabelLeft4, TOP_RIGHT );
	AddAnchor( m_oInOutStatsLabelLeft5, TOP_RIGHT );
	AddAnchor( m_oInOutStatsLabelRight1, TOP_RIGHT );
	AddAnchor( m_oInOutStatsLabelRight2, TOP_RIGHT );
	AddAnchor( m_oInOutStatsLabelRight3, TOP_RIGHT );
	AddAnchor( m_oInOutStatsLabelRight4, TOP_RIGHT );
	AddAnchor( m_oInOutStatsLabelRight5, TOP_RIGHT );


	m_ConnectionList.Init();
	
	/////////////////////////////////////////////////////////////////////////////
	// LOAD THE GUI RESOURCES FROM THE STRING TABLE BASED ON LANGUAGE... 
/*
	strTemp = m_ExtStr.LoadString( IDS_CONNECTIONS_TAB_STARTUP_ENG + g_unStringLanguageIdOffset );
	m_staticStatusText.SetWindowText( strTemp );

	strTemp = m_ExtStr.LoadString( IDS_CONNECTIONS_TAB_ADDRESS_TEXT_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_STATIC_ADDRESS )->SetWindowText( strTemp );

	strTemp = m_ExtStr.LoadString( IDS_CONNECTIONS_TAB_PORT_TEXT_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_STATIC_PORT )->SetWindowText( strTemp );
	
	strTemp = m_ExtStr.LoadString( IDS_CONNECTIONS_TAB_ADDHOST_BTN_TEXT_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_ADD_HOST_BUTTON )->SetWindowText( strTemp );
*/	
	// LOAD THE GUI RESOURCES FROM THE STRING TABLE BASED ON LANGUAGE... 
	/////////////////////////////////////////////////////////////////////////////	

	m_oAddHostGroup.Init( IDI_ADD_HOST_ICON );	
	m_oHostUpdateGroup.Init( IDI_HOSTUPDATE_ICON );	
	m_oMyInfoGroup.Init( IDI_MYINFO_ICON );		
	
	SetTimer( 667, 5000, NULL );
	SetTimer( 668, 1000, NULL );

	m_oInOutStatsLabelLeft1.SetText( "Out:" );
	m_oInOutStatsLabelLeft2.SetText( "In:" );
	m_oInOutStatsLabelLeft3.SetText( "DL'd:" );
	m_oInOutStatsLabelLeft4.SetText( "Search Results:" );
	m_oInOutStatsLabelLeft5.SetText( "OUT TO ALL:" );

	SetStrings();
	((CStatic*)GetDlgItem(IDC_HOSTS_ICO))->SetIcon((HICON)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_HOSTLIST_ICON), IMAGE_ICON, 16, 16, 0));
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/*
//*******************************************************************
//  FUNCTION:    -	OnConnectionsUpdate()
//  RETURNS:     -	
//  PARAMETERS:  -	
//  COMMENTS:	 -	
//*******************************************************************
*/
void CMuteConnectionsDlg::OnConnectionsUpdate()
{
	CMUTEConnectionListItem		listItem;
	CMUTEMFC2Dlg				*pParent;

	pParent = (CMUTEMFC2Dlg *) (GetParent()->GetParent());

    m_ConnectionList.DeleteAllItems();


    char			**addresses;
    int 			*ports;
    int 			*sentCounts;
    int 			*queueCounts;
    int 			*dropCounts;
	unsigned long	*startTimes;
	CString			strTemp;

    int numHosts = muteGetConnectedHostList( &addresses, &ports, &sentCounts,
                                             &queueCounts, &dropCounts, &startTimes );

	m_ulLastNumOfConnections = numHosts;
	strTemp.Format( "%s (%d)", 
		m_ExtStr.LoadString( IDS_TAB_TITLE_CONNECTIONS_TEXT_ENG + g_unStringLanguageIdOffset ),
		m_ulLastNumOfConnections
		);
	GetDlgItem( IDC_HOST_TEXT )->SetWindowText( strTemp );
	
	if( NULL != pParent )
	{
		if( ::IsWindow( pParent->GetSafeHwnd() ) )
		{
			pParent->PostMessage( MUTE_MAIN_GUI_UPDATE_CONNECTION_STAT_MSG, 0, numHosts );			
		}
	}

	m_vecListItems.resize(numHosts);
    for( int i=0; i<numHosts; i++ ) 
	{        
		m_vecListItems[i].m_strAddress = addresses[i];
		m_vecListItems[i].m_unPort = ports[i];
		m_vecListItems[i].m_unSent = sentCounts[i];
		m_vecListItems[i].m_unQueued = queueCounts[i];
		m_vecListItems[i].m_unDropped = dropCounts[i];
        
		m_ConnectionList.AddConnectionItem( &m_vecListItems[i] );
        delete [] addresses[i];
    }

    delete [] addresses;
    delete [] ports;
    delete [] sentCounts;
    delete [] queueCounts;
    delete [] dropCounts;
	delete [] startTimes;


    char *currentAttemptAddress;
    int currentAttemptPort;
    char attempting = muteGetCurrentConnectionAttempt( &currentAttemptAddress,
                                                       &currentAttemptPort );

    if( attempting ) 
	{
		CString	strTemp,strStatus;

		strTemp = m_ExtStr.LoadString( IDS_TRYING_TO_CONNECT_ENG + g_unStringLanguageIdOffset );
		strStatus.Format( strTemp, currentAttemptAddress, currentAttemptPort );		
		          
        delete [] currentAttemptAddress;
		
        m_staticStatusText.SetWindowText( strStatus );

    }
    else 
	{
        m_staticStatusText.SetWindowText( "" );
    }	
}

//*******************************************************************
//  FUNCTION:   -	OnAddHostButton() 
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteConnectionsDlg::OnAddHostButton() 
{
	HOSTENT *ip;

	UpdateData(TRUE);

	// check for valid ip address
	ip = gethostbyname((LPCSTR) m_strAddressEdit);

	if( NULL == ip )
	{
		CString	strCaption, strMsg;

		strCaption = m_ExtStr.LoadString( IDS_ADDHOST_ERROR_MSGBOX_CAPTION_ENG + g_unStringLanguageIdOffset );
		strMsg = m_ExtStr.LoadString( IDS_ADDHOST_ERROR_MSGBOX_MSG_ENG + g_unStringLanguageIdOffset );
		MessageBox( strMsg, strCaption, MB_ICONEXCLAMATION);
		return;
	}

	char *address = stringDuplicate( (LPCSTR) m_strAddressEdit );
    char *portString = stringDuplicate( (LPCSTR) m_strPortEdit );

    if( strcmp( address, "" ) != 0 ) 
	{

        // port defaults to 4900
        int port = 4900;
        int numRead = sscanf( portString, "%d", &port );

        if( numRead == 1 ) 
		{
            // clear address field
            m_strAddressEdit.Empty();

            // leave port field
			m_strPortEdit.Empty();
        }
        else 
		{
            // replace bad port string with default
            m_strPortEdit = "4900";
        }
		UpdateData(FALSE);

        muteAddHost( address, port );            
	}

    delete [] address;
    delete [] portString;    	
}

//*******************************************************************
//  FUNCTION:   -	SetStrings()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteConnectionsDlg::SetStrings()
{
	CString strTemp;	

	strTemp = m_ExtStr.LoadString( IDS_CONNECTIONS_TAB_ADDHOST_BTN_TEXT_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_ADD_HOST_BUTTON )->SetWindowText( strTemp );		
	strTemp.Remove('&');
	m_oAddHostGroup.SetText( strTemp );	
	
	strTemp = m_ExtStr.LoadString( IDS_TRAFFIC_TEXT_ENG + g_unStringLanguageIdOffset );
	m_oMyInfoGroup.SetText(strTemp);

	strTemp = m_ExtStr.LoadString( IDS_CONNECTIONS_TAB_ADDRESS_TEXT_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_STATIC_ADDRESS )->SetWindowText( strTemp );

	strTemp = m_ExtStr.LoadString( IDS_CONNECTIONS_TAB_PORT_TEXT_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_STATIC_PORT )->SetWindowText( strTemp );		

	strTemp.Format( "%s (%d)", 
		m_ExtStr.LoadString( IDS_TAB_TITLE_CONNECTIONS_TEXT_ENG + g_unStringLanguageIdOffset ),
		m_ulLastNumOfConnections
		);
	GetDlgItem( IDC_HOST_TEXT )->SetWindowText( strTemp );
}


//*******************************************************************
//  FUNCTION:   -	OnShowWindow
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	Initiates strings update
//*******************************************************************

void CMuteConnectionsDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CResizableDialog::OnShowWindow(bShow, nStatus);			
	m_ConnectionList.SetStrings();	
	SetStrings();
}


// NATE whipped this up to save connections for quick restart!
// save the HostCatcher list to the seedHosts.ini file
void CMuteConnectionsDlg::SaveConnections() 
{	
	char **addresses;
	int *ports;
	int *sentCounts;
	int *queueCounts;
	int *dropCounts;
	unsigned long *startTimes;
	char hostString[1024];

	int hostCount = 0;

	StringBufferOutputStream *tempHostListStream = new StringBufferOutputStream();

	// get current connected host list and write them to stream first
	// it seems to pick from the top of the list first but I didn't do
	// a lot of research to confirm that
	int numHosts = muteGetConnectedHostList( &addresses, &ports, &sentCounts, &queueCounts, &dropCounts, &startTimes );

	for( int i=0; i < numHosts; i++ ) 
	{
		sprintf( hostString, "%s %d\n", addresses[i], ports[i] );
		tempHostListStream->writeString( hostString );
		hostCount++;
		delete [] addresses[i]; // jroc.. we have to delete each address allocation
	}

	delete [] addresses;
	delete [] ports;
	delete [] sentCounts;
	delete [] queueCounts;
	delete [] dropCounts;
	delete [] startTimes;
	if( muteHostCatcher != NULL ) 
	{
		SimpleVector<HostAddress *> *hostList = muteHostCatcher->getHostList( 50, NULL );
		int numHosts = hostList->size();

		for( int i=0; i < numHosts; i++ ) 
		{
			HostAddress *currentHost = *( hostList->getElement( i ) );
            
            // poor man's way of avoiding a buffer overrun...
            // I'm assuming here that the port cannot have more than 10 or so digits
            // and the last time I checked, a port # will never be more than 32 bits unsigned (usually they're 16)
            // so that means the max # could be 4294967296 which is 10 digits... 
            if( strlen( currentHost->mAddressString) + 15 < 1024 )
            {
			    sprintf( hostString,  "%s %d\n", currentHost->mAddressString, currentHost->mPort );
			    tempHostListStream->writeString( hostString );
            }
			delete currentHost;
			hostCount++;
		}

		delete hostList;
	}

	char *hostListString = tempHostListStream->getString();

	if( hostCount > 0 ) 
	{	
		//printf( "Saving connections list for use at next startup\n" );
		SettingsManager::setSetting( "seedHosts", hostListString );
	}

	delete [] hostListString;
	delete tempHostListStream;
}


//*******************************************************************
//  FUNCTION:   -	OnInOutStatsUpdate()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	Fired through windows message by Window Timer
//*******************************************************************
LRESULT CMuteConnectionsDlg::OnInOutStatsUpdate(WPARAM wParam, LPARAM lParam)
{
	CString str;
	char szTempOut[1024];
	char *szTemp = formatDataSizeWithUnits( g_nMFCMuteBytesOut );
	char *szTemp2 = formatDataSizeWithUnits( g_nMFCMuteBytesIn );	
	char *szTemp3 = formatDataSizeWithUnits( g_nBytesDownloaded );
	char *szTemp4 = formatDataSizeWithUnits( g_nBytesSearchResultsRcvd );
	char *szTemp5 = formatDataSizeWithUnits( g_nBytesSentToAll );

	sprintf( szTempOut, "%s  %I64d B\n", szTemp, g_nMFCMuteBytesOut );
	m_oInOutStatsLabelRight1.SetText( szTempOut );

	sprintf( szTempOut, "%s  %I64d B\n", szTemp2, g_nMFCMuteBytesIn );
	m_oInOutStatsLabelRight2.SetText( szTempOut );

	sprintf( szTempOut, "%s  %I64d B\n", szTemp3, g_nBytesDownloaded );
	m_oInOutStatsLabelRight3.SetText( szTempOut );
	
	sprintf( szTempOut, "%s  %I64d B\n", szTemp4, g_nBytesSearchResultsRcvd );
	m_oInOutStatsLabelRight4.SetText( szTempOut );
	
	sprintf( szTempOut, "%s  %I64d B", szTemp5, g_nBytesSentToAll );
	m_oInOutStatsLabelRight5.SetText( szTempOut );
	
	delete [] szTemp;
	delete [] szTemp2;
	delete [] szTemp3;
	delete [] szTemp4;
	delete [] szTemp5;
	
	return (LRESULT) TRUE;
}