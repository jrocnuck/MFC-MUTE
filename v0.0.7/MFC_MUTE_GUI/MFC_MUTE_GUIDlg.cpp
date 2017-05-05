// MUTEMFC2Dlg.cpp : implementation file
//
// 02-17-2005 JROC - Changed tray icon tooltip from "MFC_MUTE_0.3" to "MFC MUTE"
#include "stdafx.h"
#include "MFC_MUTE_GUI.h"
#include "MFC_MUTE_GUIDlg.h"
#include "MuteDownloadsDlg.h"
#include <LIMITS.H > // JROC BUG FIX FOR MICRO$OFT DEBUG HEAP ALLOC CRT BUG
#include "minorGems/util/SettingsManager.h" 
#include "MUTE/otherApps/fileSharing/userInterface/common/formatUtils.h"
#include "StrParse.h"
#include "ColorNames.h"
#include ".\mfc_mute_guidlg.h"

extern __int64 g_nBytesSent;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define MUTE_TASK_TRAY_CB_ID			(WM_APP+1)
#define MUTE_TASK_TRAY_RESTORE_CMD_ID	(WM_USER+100) // ARBITRARY
#define MUTE_TASK_TRAY_QUIT_CMD_ID		(MUTE_TASK_TRAY_RESTORE_CMD_ID + 1) // ARBITRARY


AFX_STATIC const UINT _unTaskbarRestartMSGID = ::RegisterWindowMessage(TEXT("TaskbarCreated"));
BOOL (WINAPI *_TransparentBlt)(HDC, int, int, int, int, HDC, int, int, int, int, UINT)= NULL;

/////////////////////////////////////////////////////////////////////////////
// CMUTEMFC2Dlg dialog

CMUTEMFC2Dlg::CMUTEMFC2Dlg(CWnd* pParent /*=NULL*/)
	: CMUTEMFC2DialogBase(CMUTEMFC2Dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMUTEMFC2Dlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDI_MUTE_ICON);
	
	m_bQuitingAppDialogIsShowing	= false;
	m_bdoubleclicked				= false;
	m_bIsTrayIconActive				= false;
	m_ulLastNumOfConnections		= 0;
	m_timeStart						= time(NULL);
	
	//lastuprate = 0;
	//lastdownrate = 0;

	transicons[0] = (HICON)::LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_UP0DOWN0_ICON),IMAGE_ICON,16,16,0);
	transicons[1] = (HICON)::LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_UP0DOWN1_ICON),IMAGE_ICON,16,16,0);
	transicons[2] = (HICON)::LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_UP1DOWN0_ICON),IMAGE_ICON,16,16,0);
	transicons[3] = (HICON)::LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_UP1DOWN1_ICON),IMAGE_ICON,16,16,0);

	connicons[0] = (HICON)::LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_CONNECTEDNOTNOT_ICON),IMAGE_ICON,16,16,0);
	connicons[1] = (HICON)::LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_CONNECTEDLOWLOW_ICON),IMAGE_ICON,16,16,0);
	connicons[2] = (HICON)::LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_CONNECTEDHIGHHIGH_ICON),IMAGE_ICON,16,16,0);

	usericon = (HICON)::LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_USER_ICON),IMAGE_ICON,16,16,0);
}

	
	

void CMUTEMFC2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CMUTEMFC2DialogBase::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMUTEMFC2Dlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control( pDX, IDC_MUTE_MAIN_TAB, m_oTabCtrl );	
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMUTEMFC2Dlg, CMUTEMFC2DialogBase)
	//{{AFX_MSG_MAP(CMUTEMFC2Dlg)
	ON_COMMAND( MUTE_TASK_TRAY_RESTORE_CMD_ID, TrayRestore )
	ON_COMMAND( MUTE_TASK_TRAY_QUIT_CMD_ID, TrayQuit )
	ON_WM_SYSCOMMAND()
	ON_REGISTERED_MESSAGE( _unTaskbarRestartMSGID, AddTrayIcon )
	ON_MESSAGE( MUTE_TASK_TRAY_CB_ID, OnTrayMessage )
	ON_MESSAGE( MUTE_MAIN_GUI_UPDATE_CONNECTION_STAT_MSG, UpdateConnectionStatus )
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_WM_CLOSE()	
	ON_WM_QUERYENDSESSION()
    ON_WM_TIMER()
	ON_WM_SHOWWINDOW()
	ON_WM_WINDOWPOSCHANGING()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMUTEMFC2Dlg message handlers

BOOL CMUTEMFC2Dlg::OnInitDialog()
{
	CString	strTemp;
	int		nTabIndex;
	CSize	dlgSize;
	CRect	rc;

	CMUTEMFC2DialogBase::OnInitDialog();		
	
	// so we can Shrink the window even more
	// Tony motivated me to do this... 
	GetWindowRect( &rc );
	//dlgSize.cx = (long)(0.80 * rc.Width());
	//dlgSize.cy = (long)(0.80 * rc.Height());
	dlgSize.cx = 720;
	dlgSize.cy = 480;
	SetMinTrackSize( dlgSize ); //

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	// set statusbar
	m_Statusbar.Create(WS_CHILD|WS_VISIBLE|CCS_BOTTOM|SBARS_SIZEGRIP,CRect(0,0,0,0), this, IDC_STATUSBAR) ;
	m_Statusbar.EnableToolTips(true);
	CRect rect;
	m_Statusbar.GetClientRect(&rect);
	int widths[5] = { rect.right-675, rect.right-480, rect.right-250,rect.right-25, -1 };
	m_Statusbar.SetParts(5, widths);
	m_Statusbar.SetIcon(1,usericon);	
	m_Statusbar.SetIcon(2,transicons[0]);
	m_Statusbar.SetIcon(3,connicons[0]);

	AddAnchor( m_oTabCtrl, TOP_LEFT, BOTTOM_RIGHT );	
	AddAnchor( m_Statusbar, BOTTOM_LEFT, BOTTOM_RIGHT );
	
	m_TrayIconMenu.CreatePopupMenu();
	m_TrayIconMenu.AddMenuTitle("MUTE TRAY CONTROL", true);

	// 01-22-2005 -- just put a loop in instead of individual calls to
	// InsertItem
    m_oTabCtrl.SetImageList(&m_oTabCtrl.m_pImgLst); //*** MCoder 05.09.2005 - Zeile ergänzt
	for( nTabIndex = 0; nTabIndex < m_oTabCtrl.m_nNumberOfPages; nTabIndex++ )
	{
		m_oTabCtrl.InsertItem(nTabIndex,strTemp, nTabIndex);
	}

	m_oTabCtrl.Init();
	m_oTabCtrl.SetFocus();
	m_oTabCtrl.GotoPageIndex(3);

	SetTimer( 666, 30000, NULL ); // JROC BUG FIX FOR MICRO$OFT DEBUG HEAP ALLOC CRT BUG
	SetTimer( 667, 1000, NULL );
	LoadWindowPlacement();
	SetStrings();

	GetWindowText( m_strBaseWindowText );
	return FALSE;  // return TRUE  unless you set the focus to a control
}


//*******************************************************************
//  FUNCTION:   -	OnSysCommand
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMUTEMFC2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == SC_MINIMIZETRAY)
	{
		AddTrayIcon(0,0);
		ShowWindow(SW_HIDE);		
	}		
	else
	{		
		CMUTEMFC2DialogBase::OnSysCommand(nID, lParam);
	}
}

//*******************************************************************
//  FUNCTION:   -	OnPaint()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMUTEMFC2Dlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CMUTEMFC2DialogBase::OnPaint();
	}
}

HCURSOR CMUTEMFC2Dlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CMUTEMFC2Dlg::OnSize(UINT nType, int cx, int cy) 
{
	CMUTEMFC2DialogBase::OnSize(nType, cx, cy);	
}

//*******************************************************************
//  FUNCTION:   -	PreTranslateMessage()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
BOOL CMUTEMFC2Dlg::PreTranslateMessage(MSG* pMsg) 
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( (pMsg->wParam == VK_ESCAPE) || (pMsg->wParam == VK_RETURN) )
		{
			// Don't close the window if the user hits the ESC key.
			return FALSE;
		}
	}	
	return CMUTEMFC2DialogBase::PreTranslateMessage(pMsg);
}


//*******************************************************************
//  FUNCTION:   -	OnClose()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMUTEMFC2Dlg::OnClose() 
{
	SaveWindowPlacement();
	if( ! ((CMuteDownloadsDlg *) m_oTabCtrl.m_tabPages[1])->areDownloadsActive() )
	{
		CleanupAndExit();
	}
	else
	{	
		CString	strCaption, strText;

		int nMBReturnVal;
		m_bQuitingAppDialogIsShowing = true; // prevents the task tray from popping up while displaying this message box
		
		strCaption = m_ExtStr.LoadString( IDS_MUTE_CLOSE_MSG_CAPTION_ENG + g_unStringLanguageIdOffset );
		strText = m_ExtStr.LoadString( IDS_MUTE_CLOSE_MSG_TEXT_ENG + g_unStringLanguageIdOffset );
		
		nMBReturnVal = MessageBox( strText, strCaption, MB_ICONQUESTION | MB_YESNO);

		if( IDYES == nMBReturnVal )
		{
			CleanupAndExit();
		}

		m_bQuitingAppDialogIsShowing = false;
	}
}


//*******************************************************************
//  FUNCTION:   -	OnQueryEndSession()	
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	Called by frame work when a user is logging off or shutting down.
//*******************************************************************
BOOL CMUTEMFC2Dlg::OnQueryEndSession()
{
	CleanupAndExit();
	return TRUE;
}


//**************************************************************************************
//  FUNCTION:	-	CleanupAndExit()
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		Clears up any remnants of Tray Icons... 
//**************************************************************************************
void CMUTEMFC2Dlg::CleanupAndExit()
{	
	CWaitCursor			wait;
	CMuteDownloadsDlg	*pDlg;
		
	for( int i = 0; i < m_oTabCtrl.m_nNumberOfPages; i++ )
	{
		pDlg = (CMuteDownloadsDlg *)m_oTabCtrl.m_tabPages[i];
		pDlg->SendMessage( WM_CLOSE, 0, 0);		
	}

	RemoveTrayIcon();
	
	m_TrayIconMenu.DestroyMenu();

    KillTimer(666);	
	AfxGetApp()->DoWaitCursor(1);
	Sleep(2000);
	EndDialog(IDCANCEL);
}

//**************************************************************************************
//  FUNCTION:   -	OnTrayMessage()
//  RETURNS:    -	
//  PARAMETERS: -	WPARAM wparam, LPARAM lparam  -- see the Mouse messages in windows help
//  COMMENTS:   -	This function is a call back that is called when the user uses the mouse
//					on the tray icon.
//**************************************************************************************
LONG CMUTEMFC2Dlg::OnTrayMessage( WPARAM wparam, LPARAM lparam )
{   
	if( m_bQuitingAppDialogIsShowing )
	{
		// get out now, because we are showing the quit dialog!
		return -1;
	}
	// The tray icon sent us a message.  Let's see what it is
    switch ( lparam )
    {
		case WM_LBUTTONDBLCLK:
			m_bdoubleclicked = true;
			break;		
        case WM_CONTEXTMENU:
        case WM_RBUTTONDOWN:
            {
				// The user clicked the right mouse button.
                CMenu	oMenu;
				CPoint oPoint;
                 
				// Figure out where the mouse is so we                
				// can display the menu near it.
				GetCursorPos( &oPoint );
                SetForegroundWindow();
											
				m_TrayIconMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, oPoint.x, oPoint.y, this);
            }
            break;
		case WM_LBUTTONUP:
		if(m_bdoubleclicked)
		{
			PostMessage( WM_COMMAND, MUTE_TASK_TRAY_RESTORE_CMD_ID, 0);
			m_bdoubleclicked = false;
		}
		break;
		default:
			break;
    }

    return 0;
}


//**************************************************************************************
//  FUNCTION:   -	AddTrayIcon()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:   -	This function adds the icon to the task tray.
//**************************************************************************************
LRESULT CMUTEMFC2Dlg::AddTrayIcon(WPARAM, LPARAM lParam)
{
    // We don't use ShowIcon here because 
    // ShowIcon relies on the tray icon already
    // having been created. This creates the icon 
    // in the tray.
    NOTIFYICONDATA strNIData;

    strNIData.cbSize = sizeof( NOTIFYICONDATA );
    strNIData.hWnd   = GetSafeHwnd();
    strNIData.uID    = MUTE_TASK_TRAY_CB_ID;
    strNIData.uCallbackMessage = MUTE_TASK_TRAY_CB_ID;
    _tcscpy( strNIData.szTip, "MFC MUTE" );
    strNIData.hIcon = m_hIcon;//::LoadIcon( AfxGetResourceHandle(), MAKEINTRESOURCE( IDR_MAINFRAME ) );
    strNIData.uFlags = NIF_MESSAGE | NIF_TIP | NIF_ICON;
    Shell_NotifyIcon(NIM_ADD, &strNIData);
#if _WIN32_IE >= 0x0500
    // If this isn't compiled under the correct version
    // of the SDK and IE, this code will not compile.
    strNIData.uVersion = 0;
    Shell_NotifyIcon(NIM_SETVERSION, &strNIData);
#endif // _WIN32_IE >= 0x0500	

	m_bIsTrayIconActive = true;
	return (LRESULT) TRUE;
}

//**************************************************************************************
//  FUNCTION:   -	RemoveTrayIcon()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:   -	This function removes the icon from the tray
//**************************************************************************************
void CMUTEMFC2Dlg::RemoveTrayIcon()
{
	NOTIFYICONDATA	strNIData;   
    /* kill Tray Icon */
	strNIData.cbSize = sizeof( NOTIFYICONDATA );
    strNIData.hWnd   = GetSafeHwnd();
    strNIData.uID    = MUTE_TASK_TRAY_CB_ID;
	strNIData.uCallbackMessage = MUTE_TASK_TRAY_CB_ID;
    Shell_NotifyIcon( NIM_DELETE, &strNIData ); 
	m_bIsTrayIconActive = false;
}

//*******************************************************************
//  FUNCTION:   -	TrayRestore()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	When user chooses to restore from tray icon.
//*******************************************************************
void CMUTEMFC2Dlg::TrayRestore()
{
	RemoveTrayIcon();
	ShowWindow(SW_SHOW);
	SetForegroundWindow();
}

//*******************************************************************
//  FUNCTION:   -	TrayQuit()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	When user chooses to quit from tray icon.
//*******************************************************************
void CMUTEMFC2Dlg::TrayQuit()
{
	OnClose();
}


//*******************************************************************
//  FUNCTION:   -	SetStrings()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	Get current language and set appropriate strings 
//*******************************************************************
void CMUTEMFC2Dlg::SetStrings()
{
	CString			strTemp;
	CString			strTemp2;	
	int				nTabIndex;
	TC_ITEM			ti;
	unsigned int	i;

    ti.mask = TCIF_TEXT;
	
	strTemp2.Empty();
	for(i = 0; i < m_oTabCtrl.GetIconWidth()/4; i++ )
	{
		strTemp2 += " ";
	}

	// search tab 
	nTabIndex = 0;
	strTemp = m_ExtStr.LoadString(IDS_TAB_TITLE_SEARCH_TEXT_ENG + g_unStringLanguageIdOffset );	
	strTemp.Insert(0,strTemp2);
	strTemp+=strTemp2;	
	ti.pszText = strTemp.GetBuffer(MAX_PATH);
	m_oTabCtrl.SetItem(nTabIndex,&ti);
	strTemp.ReleaseBuffer();
	nTabIndex++;

	// downloads tab
	strTemp = m_ExtStr.LoadString( IDS_TAB_TITLE_DLOAD_TEXT_ENG + g_unStringLanguageIdOffset );
	strTemp.Insert(0,strTemp2);
	strTemp+=strTemp2;	
	ti.pszText =strTemp.GetBuffer(MAX_PATH);
	m_oTabCtrl.SetItem(nTabIndex,&ti);
	strTemp.ReleaseBuffer();
	nTabIndex++;

	// uploads tab
	strTemp = m_ExtStr.LoadString( IDS_TAB_TITLE_ULOAD_TEXT_ENG + g_unStringLanguageIdOffset );
	strTemp.Insert(0,strTemp2);
	strTemp+=strTemp2;	
	ti.pszText =strTemp.GetBuffer(MAX_PATH);
	m_oTabCtrl.SetItem(nTabIndex,&ti);
	strTemp.ReleaseBuffer();
	nTabIndex++;

	// connections tab
	strTemp = m_ExtStr.LoadString( IDS_TAB_TITLE_CONNECTIONS_TEXT_ENG + g_unStringLanguageIdOffset );
	strTemp.Insert(0,strTemp2);
	strTemp+=strTemp2;	
	ti.pszText = strTemp.GetBuffer(MAX_PATH);
	m_oTabCtrl.SetItem(nTabIndex,&ti);
	strTemp.ReleaseBuffer();
	nTabIndex++;

	// shared files tab	";
	// connections tab
	strTemp = m_ExtStr.LoadString( IDS_TAB_TITLE_SHAREDFILES_TEXT_ENG + g_unStringLanguageIdOffset );
	strTemp.Insert(0,strTemp2);
	strTemp+=strTemp2;	
	ti.pszText = strTemp.GetBuffer(MAX_PATH);
	m_oTabCtrl.SetItem(nTabIndex,&ti);
	strTemp.ReleaseBuffer();
	nTabIndex++;
	
	// settings tab
	strTemp = m_ExtStr.LoadString( IDS_TAB_TITLE_SETTINGS_TEXT_ENG + g_unStringLanguageIdOffset );
	strTemp.Insert(0,strTemp2);
	strTemp+=strTemp2;	
	ti.pszText =strTemp.GetBuffer(MAX_PATH);
	m_oTabCtrl.SetItem(nTabIndex,&ti);
	strTemp.ReleaseBuffer();
	nTabIndex++;	
	

	// MENU
	m_TrayIconMenu.DeleteMenu(MUTE_TASK_TRAY_RESTORE_CMD_ID,MF_BYCOMMAND);
	strTemp = m_ExtStr.LoadString( IDS_TASK_TRAY_MENU_RESTORE_ENG + g_unStringLanguageIdOffset );
	m_TrayIconMenu.AppendMenu(MF_STRING,MUTE_TASK_TRAY_RESTORE_CMD_ID, strTemp, MAKEINTRESOURCE(IDI_RESTORE_WINDOW_ICON) );

	m_TrayIconMenu.DeleteMenu(MUTE_TASK_TRAY_QUIT_CMD_ID,MF_BYCOMMAND);
	strTemp = m_ExtStr.LoadString( IDS_TASK_TRAY_MENU_QUIT_ENG + g_unStringLanguageIdOffset );
	m_TrayIconMenu.AppendMenu(MF_STRING,MUTE_TASK_TRAY_QUIT_CMD_ID, strTemp, MAKEINTRESOURCE(IDI_EXIT_MUTE_ICON) );			

	strTemp.Format( "%s:%d ",
		m_ExtStr.LoadString( IDS_TAB_TITLE_CONNECTIONS_TEXT_ENG + g_unStringLanguageIdOffset ),
		m_ulLastNumOfConnections );	
	m_Statusbar.SetText( strTemp, 1, 0 );

	char *szTemp = formatDataSizeWithUnits( g_nBytesSent );
	char *szTemp2 = formatDataSizeWithUnits( g_nBytesDownloaded );	
    
    strTemp.Format( m_ExtStr.LoadString( IDS_UPDOWN_ENG + g_unStringLanguageIdOffset ), szTemp, szTemp2 );	
    delete [] szTemp;
	delete [] szTemp2;
	m_Statusbar.SetText( strTemp, 2, 0 );

	if( 0 == m_ulLastNumOfConnections )
	{			
		m_Statusbar.SetIcon(3,connicons[0]);				
		strTemp =  m_ExtStr.LoadString( IDS_MUTENET_ENG + g_unStringLanguageIdOffset );
		strTemp += ":";
		strTemp += m_ExtStr.LoadString( IDS_NOTCONNECTED_ENG + g_unStringLanguageIdOffset );
		m_Statusbar.SetText( strTemp, 3, 0 );
	}
	else if( 1 == m_ulLastNumOfConnections )
	{			
		m_Statusbar.SetIcon(3,connicons[1]);		
		strTemp =  m_ExtStr.LoadString( IDS_MUTENET_ENG + g_unStringLanguageIdOffset );
		strTemp += ":";
		strTemp += m_ExtStr.LoadString( IDS_CONNECTED_ENG + g_unStringLanguageIdOffset );
		m_Statusbar.SetText( strTemp, 3, 0 );		
	}
	else
	{						
		m_Statusbar.SetIcon(3,connicons[2]);
		strTemp =  m_ExtStr.LoadString( IDS_MUTENET_ENG + g_unStringLanguageIdOffset );
		strTemp += ":";
		strTemp += m_ExtStr.LoadString( IDS_CONNECTED_ENG + g_unStringLanguageIdOffset );
		m_Statusbar.SetText( strTemp, 3, 0 );		
	}
    Invalidate();
}

void CMUTEMFC2Dlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CMUTEMFC2DialogBase::OnShowWindow(bShow, nStatus);		
	SetStrings();
}	

void CMUTEMFC2Dlg::OnTimer(UINT nIDEvent) 
{
	switch( nIDEvent )
	{

	case 666:
#ifdef _DEBUG
		// reference 
		// http://www.experts-exchange.com/Programming/Programming_Languages/MFC/Q_21088390.html
		// JROC BUG FIX FOR MICRO$OFT DEBUG HEAP ALLOC CRT BUG
        long lRequest;
        long NewCrtBreakAlloc;
		void *my_pointer;

        my_pointer = malloc(10);
        _CrtIsMemoryBlock(my_pointer, 10, &lRequest, NULL, NULL);
        free(my_pointer);
        
        if (lRequest > -1)
          NewCrtBreakAlloc = LONG_MIN / 2;
        else
          NewCrtBreakAlloc = LONG_MAX / 2;
        
        _CrtSetBreakAlloc(NewCrtBreakAlloc);       
#endif
		break;
	case 667:
		// update caption with run time
		CString str;
		time_t currTime = time(NULL);
		CTimeSpan ts( currTime - m_timeStart );
		str = m_strBaseWindowText;
		str += ts.Format( " %D:%H:%M:%S" );
		SetWindowText( str );
		break;
	}

	CMUTEMFC2DialogBase::OnTimer(nIDEvent);
}

//*******************************************************************
//  FUNCTION:   -	SaveWindowPlacement
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	Saves window placement info, so we can bring app
//					up in the exact place it was when closed.
//*******************************************************************
void CMUTEMFC2Dlg::SaveWindowPlacement()
{	
	char			value[ 100 ];
	int				show;
	int				x;
	int				y;
	int				right;
	int				bottom;
	WINDOWPLACEMENT info;
	
	info.length = sizeof(WINDOWPLACEMENT);
	if ( FALSE != GetWindowPlacement( &info ) )
	{
		if( m_bIsTrayIconActive )
		{			
			show = SW_HIDE;
		}
		else
		{
			show = info.showCmd;	
		}

		x		= info.rcNormalPosition.left;
		y		= info.rcNormalPosition.top;
		right	= info.rcNormalPosition.right;
		bottom	= info.rcNormalPosition.bottom;
		sprintf( value, "%d,%d,%d,%d,%d", show, x, y, right, bottom );
		SettingsManager::setSetting( "mfcWindowPlacement", value );
	}
}

//*******************************************************************
//  FUNCTION:   -	LoadWindowPlacement
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	Loads window placement info so app pops up in same
//					place as it was closed.
//*******************************************************************
void CMUTEMFC2Dlg::LoadWindowPlacement()
{
	WINDOWPLACEMENT info;
	char *mfcWindowPlacement = SettingsManager::getStringSetting( "mfcWindowPlacement" );

	info.length = sizeof(WINDOWPLACEMENT);
	if( NULL != mfcWindowPlacement )
	{
		CStringParser parser( mfcWindowPlacement, ',' );
		if( 5 == parser.nGetNumFields() )
		{
			GetWindowPlacement( &info );			
			info.showCmd = atoi( parser.m_strOutput[0].c_str() );			
			info.rcNormalPosition.left = atoi(parser.m_strOutput[1].c_str() );
			info.rcNormalPosition.top = atoi(parser.m_strOutput[2].c_str() );
			info.rcNormalPosition.right = atoi(parser.m_strOutput[3].c_str() );
			info.rcNormalPosition.bottom = atoi(parser.m_strOutput[4].c_str() );
			SetWindowPlacement( &info );
			if( SW_HIDE == info.showCmd )
			{
				AddTrayIcon(0,0);				
			}
		}

		delete [] mfcWindowPlacement;
	}
}

//**************************************************************************************
//  FUNCTION:   -	OnWindowPosChanging()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:   -	Allows the program to start with a hidden dialog without a taskbar icon.
//**************************************************************************************
void CMUTEMFC2Dlg::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos) 
{
	if( m_bIsTrayIconActive )
	{
		// Make the window hidden without the need of a timer.
        lpwndpos->flags &= ~SWP_SHOWWINDOW;
	}

	CDialog::OnWindowPosChanging( lpwndpos );
}

//**************************************************************************************
//  FUNCTION:   -	UpdateConnectionStatus()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:   -	Draws the right stuff on the status bar for connections..
//**************************************************************************************
LRESULT CMUTEMFC2Dlg::UpdateConnectionStatus(WPARAM, LPARAM lParam)
{
	CString str;						
	m_ulLastNumOfConnections = (unsigned long) lParam;

	char *szTemp = formatDataSizeWithUnits( g_nBytesSent );
	char *szTemp2 = formatDataSizeWithUnits( g_nBytesDownloaded );	
	str.Format( m_ExtStr.LoadString( IDS_UPDOWN_ENG + g_unStringLanguageIdOffset ), 
		szTemp,		
		szTemp2 );
	delete [] szTemp;
	delete [] szTemp2;
	m_Statusbar.SetText( str, 2, 0 );

	str.Format( "%s:%d ",
		m_ExtStr.LoadString( IDS_TAB_TITLE_CONNECTIONS_TEXT_ENG + g_unStringLanguageIdOffset ),
		m_ulLastNumOfConnections );	
	m_Statusbar.SetText( str, 1, 0 );
	
	if( 0 == m_ulLastNumOfConnections )
	{			
		m_Statusbar.SetIcon(3,connicons[0]);				
		str =  m_ExtStr.LoadString( IDS_MUTENET_ENG + g_unStringLanguageIdOffset );
		str += ":";
		str += m_ExtStr.LoadString( IDS_NOTCONNECTED_ENG + g_unStringLanguageIdOffset );
		m_Statusbar.SetText( str, 3, 0 );
	}
	else if( 1 == m_ulLastNumOfConnections )
	{			
		m_Statusbar.SetIcon(3,connicons[1]);		
		str =  m_ExtStr.LoadString( IDS_MUTENET_ENG + g_unStringLanguageIdOffset );
		str += ":";
		str += m_ExtStr.LoadString( IDS_CONNECTED_ENG + g_unStringLanguageIdOffset );
		m_Statusbar.SetText( str, 3, 0 );		
	}
	else
	{						
		m_Statusbar.SetIcon(3,connicons[2]);
		str =  m_ExtStr.LoadString( IDS_MUTENET_ENG + g_unStringLanguageIdOffset );
		str += ":";
		str += m_ExtStr.LoadString( IDS_CONNECTED_ENG + g_unStringLanguageIdOffset );
		m_Statusbar.SetText( str, 3, 0 );		
	}
	return (LRESULT) TRUE;
}