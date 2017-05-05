// JROC - 05/04/2004 --- put together for the folks out there in the free world... 
// I don't do all the geeky headers... steal this code put it where you want...
// take credit it for it, even, if you want... it's nothing you can't piece together
// from code pieces from all over the web...etc...just hooking you all up...
// MuteSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MFC_MUTE_GUI.h"
#include "MuteSettingsDlg.h"
#include "MuteOptionsDlg.h"
#include "MFC_MUTE_GUIDlg.h"  


#include "minorGems/io/file/Path.h"
#include "minorGems/io/file/File.h"
#include "minorGems/util/SettingsManager.h"
#include "MUTE/otherApps/fileSharing/fileShare.h"
#include "MUTE/layers/messageRouting/messageRouter.h"
#include "minorGems/util/stringUtils.h"

#include "ColorNames.h"
#include "WinFileSysUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMuteSettingsDlg dialog
// NATE's upload control variables 
int max_uploads; // max uploads at same time   
int max_host_uploads; // max uploads from a single host


#define DEFAULT_MAX_UPLOADS						(10) // number of default uploads allowed by default
#define DEFAULT_MAX_UPLOADS_MINIMUM_SETTING		(4) // minimum number of default uploads allowed by default
#define DEFAULT_MAX_UPLOADS_FROM_SAME_VIP		(3) // number of default uploads from same IP
#define DEFAULT_MAX_UPLOADS_FROM_SAME_VIP_MINIMUM_SETTING	(1) // minimum number of default uploads from same IP


CString CMuteSettingsDlg::m_static_strDir;

CMuteSettingsDlg::CMuteSettingsDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CMuteSettingsDlg::IDD, pParent)
{

	char * szSharePath = NULL;
	char * szIncomingPath = NULL;
	char * szHashPath = NULL;

    // get current settings
    double inboundLimit = muteGetInboundMessagePerSecondLimit();
    double outboundLimit = muteGetOutboundMessagePerSecondLimit();
    int targetConnectionCount = muteGetTargetNumberOfConnections();
    int maxConnectionCount = muteGetMaxNumberOfConnections();

    szSharePath = muteShareGetSharingPath();
	if( NULL != szSharePath )
	{
		m_strShareDirectory = szSharePath;
		delete [] szSharePath;
	}

	szIncomingPath = muteShareGetIncomingFilesPath();
	if( NULL != szIncomingPath )
	{
		m_strIncomingDirectory = szIncomingPath;
		delete [] szIncomingPath;
	}

	szHashPath = muteShareGetHashFilesPath();
	if( NULL != szHashPath )
	{
		m_strHashDirectory = szHashPath;
		delete [] szHashPath;
	}

    char found;

	int maxUploads =
        SettingsManager::getIntSetting( "maxUploads",
                                        &found );

    if( maxUploads < DEFAULT_MAX_UPLOADS_MINIMUM_SETTING ) {
        maxUploads = DEFAULT_MAX_UPLOADS_MINIMUM_SETTING; // default value limit
        }
    if( !found ) {
        maxUploads = DEFAULT_MAX_UPLOADS; // default value
        }
    max_uploads = maxUploads; // max uploads at same time
	m_strMaxUploads.Format("%d", maxUploads);


    int maxUploadsPerVip =
        SettingsManager::getIntSetting( "maxUploadsPerVip",
                                        &found );

    if( maxUploadsPerVip < DEFAULT_MAX_UPLOADS_FROM_SAME_VIP_MINIMUM_SETTING ) {
        maxUploadsPerVip = DEFAULT_MAX_UPLOADS_FROM_SAME_VIP_MINIMUM_SETTING; // default value limit
        }
    if( !found  ) {
        maxUploadsPerVip = DEFAULT_MAX_UPLOADS_FROM_SAME_VIP; // default value
        }
    max_host_uploads = maxUploadsPerVip; // max uploads from a single host
	m_strMaxUploadsPerVIP.Format("%d", maxUploadsPerVip);

    int downloadTimeoutMilliseconds =
        SettingsManager::getIntSetting( "downloadTimeoutMilliseconds",
                                        &found );
    if( !found || downloadTimeoutMilliseconds < 0 ) {
        // default to 60 seconds
        downloadTimeoutMilliseconds = 60000;
        }
    
    // construct the settings text fields    
    char *inboundLimitString = autoSprintf( "%d", (int)inboundLimit );
    m_strInboundLimit = inboundLimitString;
    delete [] inboundLimitString;

    char *outboundLimitString = autoSprintf( "%d", (int)outboundLimit );
    m_strOutboundLimit = outboundLimitString;
    delete [] outboundLimitString;


    int downloadTimeoutInSeconds = downloadTimeoutMilliseconds / 1000;
    
    char *downloadTimeoutString =
        autoSprintf( "%d", downloadTimeoutInSeconds );
    m_strInitialDownloadTimeout = downloadTimeoutString;
	delete [] downloadTimeoutString;

    
    char *targetConnectionCountString =
        autoSprintf( "%d", targetConnectionCount );
    m_strMaintainConnections = targetConnectionCountString;
	delete [] targetConnectionCountString;


    char *maxConnectionCountString =
        autoSprintf( "%d", maxConnectionCount );
    m_strAllowString = maxConnectionCountString;
    delete [] maxConnectionCountString;


    // check if we still need to ask user for a custom share path

    char valueFound;

	int nPortNumber = SettingsManager::getIntSetting( "port", &valueFound );
	if( valueFound )
	{
		m_strPort.Format("%d",nPortNumber);
	}
	else
	{
		m_strPort = "4900";
		SettingsManager::setSetting( "port", atoi( (LPCSTR) m_strPort) );	
	}

    int haveGoodSharePathFlag =
        SettingsManager::getIntSetting( "haveGoodSharePath",
                                        &valueFound );

    if( valueFound && haveGoodSharePathFlag == 1 ) 
	{
        // do nothing
    }
    else
	{
        // ask the user to specify a path

        OnBrowseSharedFolder();
		
		if( !m_strShareDirectory.IsEmpty() )
		{
			LPTSTR pszSharePath = m_strShareDirectory.GetBuffer( m_strShareDirectory.GetLength() + 1 );
			muteShareSetSharingPath( pszSharePath );
			m_strShareDirectory.ReleaseBuffer();
		}

        // never ask again
        SettingsManager::setSetting( "haveGoodSharePath", 1 );
    }


	//{{AFX_DATA_INIT(CMuteSettingsDlg)
	m_nDeleteFromIncoming = -1;
	m_strLanguageName = _T("");
	//}}AFX_DATA_INIT
}


void CMuteSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMuteSettingsDlg)			
	DDX_Text(pDX, IDC_SHARE_STRING_EDIT, m_strShareDirectory);
	DDX_Text(pDX, IDC_INCOMING_FOLDER_STRING_EDIT, m_strIncomingDirectory);	
	DDX_Text(pDX, IDC_HASH_FOLDER_STRING_EDIT, m_strHashDirectory);			
	DDX_Text(pDX,  IDC_INBOUND_LIMIT_STRING_EDIT, m_strInboundLimit);
	DDX_Control( pDX, IDC_INBOUND_LIMIT_STRING_EDIT, m_oInboundLimit );
	DDX_Text(pDX,  IDC_OUTBOUND_LIMIT_STRING_EDIT, m_strOutboundLimit );
	DDX_Control( pDX, IDC_OUTBOUND_LIMIT_STRING_EDIT, m_oOutboundLimit );
	DDX_Text(pDX,  IDC_INITIAL_DL_TO_STRING_EDIT, m_strInitialDownloadTimeout );
	DDX_Control( pDX, IDC_INITIAL_DL_TO_STRING_EDIT, m_oInitialDownloadTimeout );
	DDX_Text(pDX,  IDC_MAINTAIN_STRING_EDIT, m_strMaintainConnections );
	DDX_Control( pDX, IDC_MAINTAIN_STRING_EDIT, m_oMaintainConnections );
	DDX_Text(pDX,  IDC_ALLOW_STRING_EDIT, m_strAllowString );
	DDX_Control( pDX, IDC_ALLOW_STRING_EDIT, m_oAllowConnections );
	DDX_Control( pDX, IDC_MAX_UPLOADS_EDIT, m_oMaxUploads );
	DDX_Control( pDX, IDC_MAX_UPLOADS_PER_VIP_EDIT, m_oMaxUploadsPerVIP );
	DDX_Text( pDX, IDC_MAX_UPLOADS_EDIT, m_strMaxUploads );
	DDX_Text( pDX, IDC_MAX_UPLOADS_PER_VIP_EDIT, m_strMaxUploadsPerVIP );
	DDX_Control(pDX, IDC_COMBO_DELETE_CANCELED, m_ctrlComboDeleteCanceledDLs);
	DDX_CBIndex(pDX, IDC_COMBO_DELETE_CANCELED, m_nDeleteFromIncoming);
	DDX_Control( pDX, IDC_STATIC_MSG_LIMITS_GROUP, m_oMsgLimitsGroup );
	DDX_Control( pDX, IDC_STATIC_DL_GROUP, m_oDownloadsGroup );
	DDX_Control( pDX, IDC_STATIC_CONNECTIONS_GROUP, m_oConnectionsGroup );
	DDX_Control( pDX, IDC_STATIC_UL_GROUP, m_oUploadsGroup );		
	DDX_Control( pDX, IDC_STATIC_LANGUAGE_GROUP, m_oLanguageGroup );
	DDX_Control(pDX, IDC_COMBO_LANGUAGE, m_ctrlCBLanguage);	
	DDX_CBString(pDX, IDC_COMBO_LANGUAGE, m_strLanguageName);

	DDX_Control( pDX, IDC_STATIC_PORT_GROUP, m_oPortGroup );
	DDX_Text(pDX,  IDC_PORT_STRING_EDIT, m_strPort );
	DDX_Control( pDX, IDC_PORT_STRING_EDIT, m_oPort );
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMuteSettingsDlg, CResizableDialog)
	//{{AFX_MSG_MAP(CMuteSettingsDlg)
	ON_BN_CLICKED(IDC_BROWSE_SHARED_BUTTON, OnBrowseSharedFolder)
	ON_BN_CLICKED(IDC_INCOMING_BROWSE_BUTTON, OnBrowseIncomingFolder)
	ON_BN_CLICKED(IDC_HASH_BROWSE_BUTTON, OnBrowseHashFolder)
	ON_BN_CLICKED(IDC_SAVE_SETTINGS_BUTTON, OnSaveSettingsButton)
	ON_WM_SHOWWINDOW()	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMuteSettingsDlg message handlers

BOOL CMuteSettingsDlg::PreTranslateMessage(MSG* pMsg) 
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( (pMsg->wParam == VK_ESCAPE) || (pMsg->wParam == VK_RETURN) )
		{
			// Don't close the window if the user hits the ESC key.
			return FALSE;
		}
	}
	
	return CResizableDialog::PreTranslateMessage(pMsg);
}

//*******************************************************************
//  FUNCTION:   -	OnBrowseSharedFolder()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	Browse for Shared folder 
//*******************************************************************
void CMuteSettingsDlg::OnBrowseSharedFolder()
{
	LPITEMIDLIST		pidlRoot = NULL;
	LPITEMIDLIST		pidlSelected = NULL;
	BROWSEINFO          bi = {0};
	LPMALLOC			pMalloc = NULL;
	TCHAR               archPath[MAX_PATH];
	CString				strTemp;
	char				*szTitle;

	strTemp = m_ExtStr.LoadString( IDS_BROWSE_SHARING_FOLDER_CAPTION_ENG + g_unStringLanguageIdOffset );
	szTitle = new char[ strTemp.GetLength() + 1 ];
	sprintf( szTitle, "%s", (LPCSTR) strTemp );

	// THIS MAKES SURE WE DON'T OVERWRITE ANY OTHER 
	// SETTINGS THAT MAY HAVE BEEN CHANGED BEFORE CLICKING 
	// THE BROWSE BUTTON AND RETURNING TO THE UpdateData(FALSE) CALL
	if( ::IsWindow( this->m_hWnd ) )
	{
		UpdateData(TRUE);
	}

	SHGetMalloc( &pMalloc );
	
	bi.hwndOwner	= this->m_hWnd;
	bi.pidlRoot     = NULL;
	bi.lpszTitle	= szTitle;
	bi.ulFlags		= BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS; // BIF_EDITBOX | BIF_VALIDATE
#if defined (_MSC_VER) && (_MSC_VER == 1310)
	// for compiling with Visual Studio 2003
	bi.lpfn			= &CMuteSettingsDlg::BrowseCallbackProc;
#else
	bi.lpfn			= this->BrowseCallbackProc;
#endif
	bi.lParam		= 0;

	m_static_strDir = m_strShareDirectory;

	if ( NULL != (pidlSelected = ::SHBrowseForFolder(&bi)) )
	{
		if ( SHGetPathFromIDList( pidlSelected, archPath ) )
		{
			m_strShareDirectory = archPath;			
			if( ::IsWindow( GetSafeHwnd() ) )
			{
				UpdateData(FALSE);
			}
		}
	}
	
	if( pidlRoot )
	{
		pMalloc->Free(pidlRoot);
	}
	
	pMalloc->Release();
	delete [] szTitle;
}

//*******************************************************************
//  FUNCTION:   -	OnBrowseIncomingFolder()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	Browse for Incoming folder 
//*******************************************************************
void CMuteSettingsDlg::OnBrowseIncomingFolder() 
{
	LPITEMIDLIST		pidlRoot = NULL;
	LPITEMIDLIST		pidlSelected = NULL;
	BROWSEINFO          bi = {0};
	LPMALLOC			pMalloc = NULL;
	TCHAR               archPath[MAX_PATH];
	
	CString				strTemp;
	char				*szTitle;

	strTemp = m_ExtStr.LoadString( IDS_BROWSE_INCOMING_FOLDER_CAPTION_ENG + g_unStringLanguageIdOffset );
	szTitle = new char[ strTemp.GetLength() + 1 ];
	sprintf( szTitle, "%s", (LPCSTR) strTemp );
	
	// THIS MAKES SURE WE DON'T OVERWRITE ANY OTHER 
	// SETTINGS THAT MAY HAVE BEEN CHANGED BEFORE CLICKING 
	// THE BROWSE BUTTON AND RETURNING TO THE UpdateData(FALSE) CALL
	if( ::IsWindow( this->m_hWnd ) )
	{
		UpdateData(TRUE);
	}
	SHGetMalloc( &pMalloc );
	
	bi.hwndOwner	= this->m_hWnd;
	bi.pidlRoot     = NULL;
	bi.lpszTitle	= szTitle;
	bi.ulFlags		= BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS; // BIF_EDITBOX | BIF_VALIDATE
#if defined (_MSC_VER) && (_MSC_VER == 1310)
	// for compiling with Visual Studio 2003
	bi.lpfn			= &CMuteSettingsDlg::BrowseCallbackProc;
#else
	bi.lpfn			= this->BrowseCallbackProc;
#endif
	bi.lParam		= 0;

	m_static_strDir = m_strIncomingDirectory;

	if ( NULL != (pidlSelected = ::SHBrowseForFolder(&bi)) )
	{
		if ( SHGetPathFromIDList( pidlSelected, archPath ) )
		{
			m_strIncomingDirectory = archPath;
			
			if( ::IsWindow( GetSafeHwnd() ) )
			{
				UpdateData(FALSE);
			}
		}
	}
	
	if( pidlRoot )
	{
		pMalloc->Free(pidlRoot);
	}
	
	pMalloc->Release();
	delete [] szTitle;
}


//*******************************************************************
//  FUNCTION:   -	OnBrowseHashFolder()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	Browse for Hash folder 
//*******************************************************************
void CMuteSettingsDlg::OnBrowseHashFolder() 
{
	LPITEMIDLIST		pidlRoot = NULL;
	LPITEMIDLIST		pidlSelected = NULL;
	BROWSEINFO          bi = {0};
	LPMALLOC			pMalloc = NULL;
	TCHAR               archPath[MAX_PATH];
	CString				strTemp;
	char				*szTitle;

	strTemp = m_ExtStr.LoadString( IDS_BROWSE_HASH_FOLDER_CAPTION_ENG + g_unStringLanguageIdOffset );
	szTitle = new char[ strTemp.GetLength() + 1 ];
	sprintf( szTitle, "%s", (LPCSTR) strTemp );
	
	// THIS MAKES SURE WE DON'T OVERWRITE ANY OTHER 
	// SETTINGS THAT MAY HAVE BEEN CHANGED BEFORE CLICKING 
	// THE BROWSE BUTTON AND RETURNING TO THE UpdateData(FALSE) CALL
	if( ::IsWindow( this->m_hWnd ) )
	{
		UpdateData(TRUE);
	}
	SHGetMalloc( &pMalloc );
	
	bi.hwndOwner	= this->m_hWnd;
	bi.pidlRoot     = NULL;
	bi.lpszTitle	= szTitle;
	bi.ulFlags		= BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS; // BIF_EDITBOX | BIF_VALIDATE
#if defined (_MSC_VER) && (_MSC_VER == 1310)
	// for compiling with Visual Studio 2003
	bi.lpfn			= &CMuteSettingsDlg::BrowseCallbackProc;
#else
	bi.lpfn			= this->BrowseCallbackProc;
#endif
	bi.lParam		= 0;

	m_static_strDir = m_strHashDirectory;

	if ( NULL != (pidlSelected = ::SHBrowseForFolder(&bi)) )
	{
		if ( SHGetPathFromIDList( pidlSelected, archPath ) )
		{
			m_strHashDirectory = archPath;
			
			if( ::IsWindow( GetSafeHwnd() ) )
			{
				UpdateData(FALSE);
			}
		}
	}
	
	if( pidlRoot )
	{
		pMalloc->Free(pidlRoot);
	}
	
	pMalloc->Release();
	delete [] szTitle;
}

//**************************************************************************************
//  FUNCTION:	-	BrowseCallbackProc()
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:	-   This function helps us maintain our last browse folder...
//**************************************************************************************
int CALLBACK CMuteSettingsDlg::BrowseCallbackProc( HWND hwnd,  UINT uMsg, LPARAM lParam, LPARAM lpData )
{	
	if( BFFM_INITIALIZED == uMsg )
	{
		if( !m_static_strDir.IsEmpty() )
		{			
			char szLastPath[MAX_PATH];			
			sprintf( szLastPath, "%s", (LPCSTR) m_static_strDir );
			
			::SendMessage( hwnd, BFFM_SETSELECTION, TRUE, (long) &szLastPath[0] );
		}
	}

	return( 0 );
}

//*******************************************************************
//  FUNCTION:   -	OnInitDialog()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
BOOL CMuteSettingsDlg::OnInitDialog() 
{
	CString		strTemp;
	char		cValueFound;

	CResizableDialog::OnInitDialog();

	ShowSizeGrip(FALSE);

	AddAnchor(IDC_STATIC_LANGUAGE_GROUP,BOTTOM_LEFT);
	AddAnchor(IDC_COMBO_LANGUAGE,BOTTOM_LEFT);
    AddAnchor(IDC_SAVE_SETTINGS_BUTTON,BOTTOM_RIGHT);
	AddAnchor(IDC_STATIC_PORT_GROUP,BOTTOM_LEFT);
	AddAnchor(IDC_PORT_STRING_EDIT,BOTTOM_LEFT);
  
	AddAnchor(IDC_SHARE_STRING_EDIT,MIDDLE_LEFT);
    AddAnchor(IDC_INCOMING_FOLDER_STRING_EDIT,MIDDLE_LEFT);
    AddAnchor(IDC_HASH_FOLDER_STRING_EDIT,MIDDLE_LEFT);
    
	AddAnchor(IDC_STATIC_SHARE_LABEL,MIDDLE_LEFT);
    AddAnchor(IDC_STATIC_INCOMING_LABEL,MIDDLE_LEFT);
    AddAnchor(IDC_STATIC_HASH_LABEL,MIDDLE_LEFT);

	AddAnchor(IDC_BROWSE_SHARED_BUTTON,MIDDLE_LEFT);
    AddAnchor(IDC_INCOMING_BROWSE_BUTTON,MIDDLE_LEFT);
    AddAnchor(IDC_HASH_BROWSE_BUTTON,MIDDLE_LEFT);

	m_oPort.SetAllowDecimal(false);
	m_oPort.SetAllowNegative(false);
	m_oPort.SetLimitText(5);

	m_oInitialDownloadTimeout.SetAllowNegative(false);
	m_oMaintainConnections.SetAllowNegative(false);	
	m_oAllowConnections.SetAllowNegative(false);
	m_oMaxUploads.SetAllowNegative(false);
	m_oMaxUploadsPerVIP.SetAllowNegative(false);

	m_oPortGroup.Init( IDI_NETWORK_ICON );
	m_oMsgLimitsGroup.Init( IDI_MESSAGE_ICON );	
	m_oDownloadsGroup.Init( IDI_DOWNLOAD_ICON );
	m_oConnectionsGroup.Init( IDI_NETWORK_ICON );
	m_oUploadsGroup.Init( IDI_UPLOAD_ICON );
	m_oLanguageGroup.Init( IDI_FLAG_USA_ICON + g_unStringLanguageIdOffset );

	m_ctrlCBLanguage.AddString( "ENGLISH" );
	m_ctrlCBLanguage.SetIcon( 0, IDI_FLAG_USA_ICON );
	m_ctrlCBLanguage.AddString( "ITALIAN" );
	m_ctrlCBLanguage.SetIcon( 1, IDI_FLAG_ITALY_ICON );
	m_ctrlCBLanguage.AddString( "GERMAN" );
	m_ctrlCBLanguage.SetIcon( 2, IDI_FLAG_GERMANY_ICON );
	m_ctrlCBLanguage.AddString( "FRENCH" );
	m_ctrlCBLanguage.SetIcon( 3, IDI_FLAG_FRANCE_ICON );
	m_ctrlCBLanguage.AddString( "SPANISH" );
	m_ctrlCBLanguage.SetIcon( 4, IDI_FLAG_SPAIN_ICON );
	m_ctrlCBLanguage.AddString( "DANISH" );
	m_ctrlCBLanguage.SetIcon( 5, IDI_FLAG_DENMARK_ICON );
	m_ctrlCBLanguage.AddString( "LITHUANIAN" );
	m_ctrlCBLanguage.SetIcon( 6, IDI_FLAG_LITHUANIA_ICON );
	m_ctrlCBLanguage.AddString( "TURKISH" );
	m_ctrlCBLanguage.SetIcon( 7, IDI_FLAG_TURKEY_ICON );
		
	InitCurrentLanguage();
	SetStrings();
	
	m_nDeleteFromIncoming = SettingsManager::getIntSetting( "deleteCanceled", &cValueFound );
	if( !cValueFound )
	{
		m_nDeleteFromIncoming = 0; // default is to not delete files that are canceled!
	}

	if( m_nDeleteFromIncoming > 1 )
	{
		m_nDeleteFromIncoming = 1;
	}
	else if( m_nDeleteFromIncoming < 0 )
	{
		m_nDeleteFromIncoming = 0;
	}
	
	UpdateData(FALSE);
		

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


//*******************************************************************
//  FUNCTION:   -	OnSaveSettingsButton()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteSettingsDlg::OnSaveSettingsButton() 
{
	CString	strOldLanguage = m_strLanguageName;
	CString strExternalLanguageString;
	bExternSel = false;
	CString	strOldShareFolder = m_strShareDirectory;
	CString	strOldHashFolder = m_strHashDirectory; // 01-22-2005 JROC -- gotta restart hash builder if this dir changes!
	CString strOldPort = m_strPort;

	UpdateData(TRUE); // grab the changes on the GUI...

	//CString strTemp;
	bExternSel = true;

	CString strTemp = m_strLanguageName;
	strTemp.MakeUpper();

	m_ctrlCBLanguage.ResetContent();
	m_ctrlCBLanguage.AddString( "ENGLISH" );
	m_ctrlCBLanguage.SetIcon( 0, IDI_FLAG_USA_ICON );
	m_ctrlCBLanguage.AddString( "ITALIAN" );
	m_ctrlCBLanguage.SetIcon( 1, IDI_FLAG_ITALY_ICON );
	m_ctrlCBLanguage.AddString( "GERMAN" );
	m_ctrlCBLanguage.SetIcon( 2, IDI_FLAG_GERMANY_ICON );
	m_ctrlCBLanguage.AddString( "FRENCH" );
	m_ctrlCBLanguage.SetIcon( 3, IDI_FLAG_FRANCE_ICON );
	m_ctrlCBLanguage.AddString( "SPANISH" );
	m_ctrlCBLanguage.SetIcon( 4, IDI_FLAG_SPAIN_ICON );
	m_ctrlCBLanguage.AddString( "DANISH" );
	m_ctrlCBLanguage.SetIcon( 5, IDI_FLAG_DENMARK_ICON );
	m_ctrlCBLanguage.AddString( "LITHUANIAN" );
	m_ctrlCBLanguage.SetIcon( 6, IDI_FLAG_LITHUANIA_ICON );
	m_ctrlCBLanguage.AddString( "TURKISH" );
	m_ctrlCBLanguage.SetIcon( 7, IDI_FLAG_TURKEY_ICON );

	if(strTemp == "ENGLISH") bExternSel = false;
	if(strTemp == "ITALIAN") bExternSel = false;
	if(strTemp == "GERMAN")  bExternSel = false;
	if(strTemp == "FRENCH")  bExternSel = false;
	if(strTemp == "SPANISH")  bExternSel = false;
	if(strTemp == "DANISH")  bExternSel = false;
	if(strTemp == "LITHUANIAN")  bExternSel = false;
	if(strTemp == "TURKISH")  bExternSel = false;
	
	if (bExternSel == true)		
	{		
		strExternalLanguageString = m_strLanguageName;
		// the name is not enough. Here we need a path		
		m_strLanguageName = GetLanguageFilePath(m_strLanguageName);		
		
		CFileFind file;
		CString IconFilePath = m_strLanguageName;
		IconFilePath.Replace(".lng",".ico");
		
		if(file.FindFile(IconFilePath,0))
		{				
			m_oLanguageGroup.SetIcon(IconFilePath);			
			m_ctrlCBLanguage.SetIcon(m_ctrlCBLanguage.FindString(0, m_strExtLanguageName) ,1);
		}
		else
		{
			m_oLanguageGroup.SetIcon(IDI_EXTERN_LANG_ICON);
		}		
	}	

	ListExternLanguages();
	
	
	// take affect
	if( strOldLanguage != m_strLanguageName )
	{
		char *szLanguage = NULL;
		
		if( !m_strLanguageName.IsEmpty() )
		{
			szLanguage = new char[ m_strLanguageName.GetLength() + 1 ];
			if( NULL != szLanguage )
			{
				sprintf( szLanguage, "%s", (LPCSTR) m_strLanguageName );				
			}
		}
		else
		{
			szLanguage = new char[ strOldLanguage.GetLength() + 1 ];
			if( NULL != szLanguage )
			{		
				sprintf( szLanguage, "%s", (LPCSTR) strOldLanguage );			
			}
		}
			
		if( NULL != szLanguage )
		{
			SettingsManager::setSetting( "language", szLanguage );
			delete [] szLanguage;
		}		
	}

	// If the selected language is not external the CString Array must be empty 
	if (bExternSel==false ) { InitCurrentLanguage(); strArray.RemoveAll();} 
	if (bExternSel==true)
	{
		if( !m_strLanguageName.IsEmpty() )
		{
			g_unStringLanguageIdOffset=0;
			// Load the external strings 
			m_ExtStr.LoadStringsFromFile(m_strLanguageName); 
			m_strLanguageName = strExternalLanguageString;	
		}
		else
		{
			g_unStringLanguageIdOffset = MUTE_ENGLISH;
			m_strLanguageName = "ENGLISH";
			m_oLanguageGroup.SetIcon(IDI_FLAG_USA_ICON);
			UpdateData(FALSE);
		}
	} 

	
	CMUTEMFC2Dlg* m_MUTEMFC2Dlg=(CMUTEMFC2Dlg*)GetParentOwner();
	m_MUTEMFC2Dlg->SetStrings(); // item in the tab dialog

    // This window is a child so we must update it's  parent too; 
	CWnd* wnd=GetOwner();
	((CMuteOptionsDlg*)wnd)->SetStrings();
	
    char *inboundString =
        stringDuplicate( (LPCSTR) m_strInboundLimit );
    char *outboundString =
        stringDuplicate( (LPCSTR) m_strOutboundLimit );
    char *downloadTimeoutString =
        stringDuplicate( (LPCSTR) m_strInitialDownloadTimeout );
    char *targetCountString =
        stringDuplicate( (LPCSTR) m_strMaintainConnections );
    char *maxCountString =
        stringDuplicate( (LPCSTR) m_strAllowString );
    char *sharePath =
        stringDuplicate( (LPCSTR) m_strShareDirectory );
	char *incomingPath = stringDuplicate( (LPCSTR) m_strIncomingDirectory );
	char *hashPath = stringDuplicate( (LPCSTR) m_strHashDirectory );
	char *maxUploadsString =
        stringDuplicate( (LPCSTR) m_strMaxUploads );
    char *maxUploadsPerVipString =
        stringDuplicate( (LPCSTR) m_strMaxUploadsPerVIP );
    double	inbound;
    double	outbound;
    int		downloadTimeoutSeconds;
    int		targetCount;
    int		maxCount;

    int numRead = sscanf( inboundString, "%lf", &inbound );

    if( numRead == 1 && ( inbound > 0 || inbound == -1 ) ) {
        muteSetInboundMessagePerSecondLimit( inbound );
        }
    else {
        // bad entry, return field to correct value
        double value = muteGetInboundMessagePerSecondLimit();
        char *valueString = autoSprintf( "%f", value );

        m_strInboundLimit = valueString;
		UpdateData(FALSE);

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

        m_strOutboundLimit = valueString;
		UpdateData(FALSE);
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

        m_strInitialDownloadTimeout = valueString;
		UpdateData(FALSE);

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

        m_strMaintainConnections = valueString;
		UpdateData(FALSE);

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
        
		m_strAllowString = valueString;
		UpdateData(FALSE);

        delete [] valueString;
        }

	char found;
	int maxUploads = SettingsManager::getIntSetting( "maxUploads", &found );

	numRead = sscanf( maxUploadsString, "%d", &maxUploads );

    if( numRead == 1 && maxUploads >= DEFAULT_MAX_UPLOADS_MINIMUM_SETTING ) {
        SettingsManager::setSetting( "maxUploads",
                                     maxUploads );
        }
    else {
        // bad entry, return field to correct value
        char found;
        maxUploads =
        SettingsManager::getIntSetting( "maxUploads",
                                        &found );

        if( maxUploads < DEFAULT_MAX_UPLOADS_MINIMUM_SETTING ) {
            maxUploads = DEFAULT_MAX_UPLOADS_MINIMUM_SETTING; // default value limit
            }
        if( !found ) {
            maxUploads = DEFAULT_MAX_UPLOADS; // default value
            }
        }
    max_uploads = maxUploads; // max uploads at same time
	m_strMaxUploads.Format("%d", maxUploads );

	int maxUploadsPerVip;
    numRead = sscanf( maxUploadsPerVipString, "%d", &maxUploadsPerVip );

    if( numRead == 1 && maxUploadsPerVip >= DEFAULT_MAX_UPLOADS_FROM_SAME_VIP_MINIMUM_SETTING ) {
        SettingsManager::setSetting( "maxUploadsPerVip",
                                     maxUploadsPerVip );
        }
    else {
        // bad entry, return field to correct value
        char found;
        maxUploadsPerVip =
        SettingsManager::getIntSetting( "maxUploadsPerVip",
                                        &found );

        if( maxUploadsPerVip < DEFAULT_MAX_UPLOADS_FROM_SAME_VIP_MINIMUM_SETTING ) {
            maxUploadsPerVip = DEFAULT_MAX_UPLOADS_FROM_SAME_VIP_MINIMUM_SETTING; // default value limit
            }
        if( !found ) {
            maxUploadsPerVip = DEFAULT_MAX_UPLOADS_FROM_SAME_VIP; // default value
            }

        char *valueString = autoSprintf( "%d", maxUploadsPerVip );        
		m_strMaxUploadsPerVIP = valueString;

        delete [] valueString;
        }
    max_host_uploads = maxUploadsPerVip; // max uploads from a single host
	m_strMaxUploadsPerVIP.Format("%d", maxUploadsPerVip );
	

    File *DirFile = new File( NULL, sharePath );

    if( DirFile->exists() ) 
	{
        muteShareSetSharingPath( sharePath );
    }
    else 
	{
		CString	strCaption, strMsg;

		strCaption = m_ExtStr.LoadString( IDS_MUTE_ERROR_CAPTION_ENG + g_unStringLanguageIdOffset );
		strMsg = m_ExtStr.LoadString( IDS_MUTE_SHARE_FOLDER_DOESNT_EXIST_ENG + g_unStringLanguageIdOffset );
		MessageBox( strMsg, strCaption, MB_ICONERROR );
        
        // reset back to good path
        char *goodPath = muteShareGetSharingPath();
        m_strShareDirectory = goodPath;		
		UpdateData(FALSE);

        delete [] goodPath;
    }
    delete DirFile;

	DirFile = new File( NULL, incomingPath );

	if( DirFile->exists() )
	{    
		muteShareSetIncomingFilesPath( incomingPath );
	}
	else
	{
		CString	strCaption, strMsg;

		strCaption = m_ExtStr.LoadString( IDS_MUTE_ERROR_CAPTION_ENG + g_unStringLanguageIdOffset );
		strMsg = m_ExtStr.LoadString( IDS_MUTE_INCOMING_FOLDER_DOESNT_EXIST_ENG + g_unStringLanguageIdOffset );
		MessageBox( strMsg, strCaption, MB_ICONERROR );
        
        // reset back to good path
        char *goodPath = muteShareGetIncomingFilesPath();		
		if( NULL != goodPath )
		{
			m_strIncomingDirectory = goodPath;
			delete [] goodPath;
		}		
		UpdateData(FALSE);        
	}
	delete DirFile;
	
	DirFile = new File( NULL, hashPath );

	if( DirFile->exists() )	
	{
		muteShareSetHashFilesPath( hashPath );
	}
	else
	{
		CString	strCaption, strMsg;

		strCaption = m_ExtStr.LoadString( IDS_MUTE_ERROR_CAPTION_ENG + g_unStringLanguageIdOffset );
		strMsg = m_ExtStr.LoadString( IDS_MUTE_HASH_FOLDER_DOESNT_EXIST_ENG + g_unStringLanguageIdOffset );
		MessageBox( strMsg, strCaption, MB_ICONERROR );		
        
        // reset back to good path
        char *goodPath = muteShareGetHashFilesPath();        
		if( NULL != goodPath )
		{
			m_strHashDirectory = goodPath;
			delete [] goodPath;
		}		
		UpdateData(FALSE);        
	}
    delete DirFile;

    delete [] inboundString;
    delete [] outboundString;
    delete [] downloadTimeoutString;
    delete [] targetCountString;
    delete [] maxCountString;
    delete [] sharePath;
	delete [] incomingPath;
	delete [] hashPath;
	delete [] maxUploadsString;
    delete [] maxUploadsPerVipString;

	// delete files on cancel setting
    if(m_ctrlComboDeleteCanceledDLs.GetCurSel()==0) m_nDeleteFromIncoming=0;
	if(m_ctrlComboDeleteCanceledDLs.GetCurSel()==1) m_nDeleteFromIncoming=1;	
	SettingsManager::setSetting( "deleteCanceled", m_nDeleteFromIncoming );	
	
	// In some instances if a user puts an invalid setting in to a box and clicks
	// the Save Settings button, this function will over ride the user and reset
	// the value.  Therefore this next function call will show correct values 
	// in edit boxes (in case some changed back)
	UpdateData(FALSE);	

// 01-22-2005 JROC.. also need to account for changing HASH directory.
	if( (strOldShareFolder != m_strShareDirectory) ||
		(strOldHashFolder != m_strHashDirectory)
	  )
	{
		CWaitCursor wait;
			
		// stop hash builder
		muteStopHashBuilder();

		// purge hashes
		EmptyDirectory( (LPCSTR) m_strHashDirectory, TRUE, TRUE );

		// restart hash builder
		muteStartHashBuilder();
	}
	
	SettingsManager::setSetting( "port", atoi( (LPCSTR) m_strPort) );	
	if( strOldPort != m_strPort )
	{
		CString	strCaption, strMsg;

		strCaption = m_ExtStr.LoadString( IDS_SETTINGS_PORT_CHANGE_RESTART_CAPTION_ENG + g_unStringLanguageIdOffset );
		strMsg = m_ExtStr.LoadString( IDS_SETTINGS_PORT_CHANGE_RESTART_TEXT_ENG + g_unStringLanguageIdOffset );
		MessageBox( strMsg, strCaption, MB_ICONERROR );		
	}
    // In every case we must set the new strings in all the 
	// the visible windows. Other windows will be automatically
	// updated by handling the ON_WM_SHOWWINDOW() message.
		 
	SetStrings(); // Update strings in this windows


}

//*******************************************************************
//  FUNCTION:   -	InitCurrentLanguage()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	Search only among the embedded languages
//*******************************************************************

void CMuteSettingsDlg::InitCurrentLanguage()
{
	m_strLangGroupIconPath.IsEmpty();
    ListExternLanguages();
	  
    g_unStringLanguageIdOffset=0;
	CString IconFilePath;
	char *szLanguage = SettingsManager::getStringSetting( "language" );


	if( NULL != szLanguage )
	{
		m_strLanguageName = szLanguage;
		delete [] szLanguage;

        // Additional language resource file detection 
		if (m_strLanguageName.Right(4)==".lng")	
		{		
		
			CFileFind file;
			if(file.FindFile(m_strLanguageName,0))
			{
				file.FindNextFile();
				IconFilePath=m_strLanguageName;
				m_strExtLanguageName=file.GetFileTitle();
                m_strLanguageName=m_strExtLanguageName;				
			}
		}

		
		if( m_strLanguageName.Find( "ENGLISH" ) > - 1 )
		{			
			g_unStringLanguageIdOffset = MUTE_ENGLISH;
			m_oLanguageGroup.SetIcon(IDI_FLAG_USA_ICON);
		}
		if( m_strLanguageName.Find( "ITALIAN" ) > - 1 )
		{			
			g_unStringLanguageIdOffset = MUTE_ITALIAN;
			m_oLanguageGroup.SetIcon(IDI_FLAG_ITALY_ICON);
		}
		if( m_strLanguageName.Find( "GERMAN" ) > - 1 )
		{		
			g_unStringLanguageIdOffset = MUTE_GERMAN;
			m_oLanguageGroup.SetIcon(IDI_FLAG_GERMANY_ICON);
		}
		if( m_strLanguageName.Find( "FRENCH" ) > - 1 )
		{			
			g_unStringLanguageIdOffset = MUTE_FRENCH;
			m_oLanguageGroup.SetIcon(IDI_FLAG_FRANCE_ICON);
		}
		if( m_strLanguageName.Find( "SPANISH" ) > - 1 )
		{			
			g_unStringLanguageIdOffset = MUTE_SPANISH;
			m_oLanguageGroup.SetIcon(IDI_FLAG_SPAIN_ICON);
		}
		else if( m_strLanguageName.Find( "DANISH" ) > - 1 )
		{
			g_unStringLanguageIdOffset = MUTE_DANISH;
			m_oLanguageGroup.SetIcon(IDI_FLAG_DENMARK_ICON);
		}
		else if( m_strLanguageName.Find( "LITHUANIAN" ) > - 1 )
		{
			g_unStringLanguageIdOffset = MUTE_LITHUANIAN;
			m_oLanguageGroup.SetIcon(IDI_FLAG_LITHUANIA_ICON);
		}
		else if( m_strLanguageName.Find( "TURKISH" ) > - 1 )
		{
			g_unStringLanguageIdOffset = MUTE_TURKISH;
			m_oLanguageGroup.SetIcon(IDI_FLAG_TURKEY_ICON);
		}

		// EXTERN LANGUAGE		
		if( m_strLanguageName == m_strExtLanguageName)
		{			
			g_unStringLanguageIdOffset=0;
			m_strLanguageName = m_strExtLanguageName;

			CFileFind file;			
            IconFilePath.Replace(".lng",".ico");

			if(file.FindFile(IconFilePath,0))   // If extern language icon found
			{				
				m_oLanguageGroup.SetIcon(IconFilePath);
				m_strLangGroupIconPath=IconFilePath;
			}
			else								// Default icon
			{
				m_oLanguageGroup.SetIcon(IDI_EXTERN_LANG_ICON);
				m_strLangGroupIconPath.IsEmpty();
			}
		}

	}
	else
	{		
		// default to ENGLISH!
		m_strLanguageName = "ENGLISH";
		g_unStringLanguageIdOffset = MUTE_ENGLISH;
		m_oLanguageGroup.SetIcon(IDI_FLAG_USA_ICON);
	}
}

//*******************************************************************
//  FUNCTION:   -	SetStrings()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteSettingsDlg::SetStrings()
{
	InitCurrentLanguage();
    CString	strTemp;
	char cValueFound;

	strTemp = m_ExtStr.LoadString( IDS_SETTINGS_MESSAGE_LIMIT_GROUP_ENG + g_unStringLanguageIdOffset );
	m_oMsgLimitsGroup.SetText( strTemp );
	
	strTemp = m_ExtStr.LoadString( IDS_SETTINGS_DLOAD_SETTINGS_GROUP_ENG + g_unStringLanguageIdOffset );
	m_oDownloadsGroup.SetText( strTemp );

	strTemp = m_ExtStr.LoadString( IDS_SETTINGS_CONNECTIONS_GROUP_ENG + g_unStringLanguageIdOffset );
	m_oConnectionsGroup.SetText( strTemp );

	strTemp = m_ExtStr.LoadString( IDS_SETTINGS_UPLOAD_CONTROLS_GROUP_ENG + g_unStringLanguageIdOffset );
	m_oUploadsGroup.SetText( strTemp );

	strTemp = m_ExtStr.LoadString( IDS_SETTINGS_LANGUAGE_GROUP_ENG + g_unStringLanguageIdOffset );

	if (m_strLangGroupIconPath.IsEmpty())
	{	
		m_oLanguageGroup.SetText( strTemp );
	}
	else
	{
		m_oLanguageGroup.SetText( strTemp,m_strLangGroupIconPath );
	}
	m_strLangGroupIconPath.Empty();

	strTemp = m_ExtStr.LoadString( IDS_CONNECTIONS_TAB_PORT_TEXT_ENG + g_unStringLanguageIdOffset );
	m_oPortGroup.SetText( strTemp );

	strTemp = m_ExtStr.LoadString( IDS_SETTINGS_INBOUND_LIMIT_ENG + g_unStringLanguageIdOffset );	
	GetDlgItem( IDC_STATIC_INBOUND_LABEL )->SetWindowText( strTemp );
	
	strTemp = m_ExtStr.LoadString( IDS_SETTINGS_OUTBOUND_LIMIT_ENG + g_unStringLanguageIdOffset );	
	GetDlgItem( IDC_STATIC_OUTBOUND_LABEL )->SetWindowText( strTemp );
	
	strTemp = m_ExtStr.LoadString( IDS_SETTINGS_MSG_SEC_LIMIT_ENG + g_unStringLanguageIdOffset );	
	GetDlgItem( IDC_STATIC_INBOUND_UNITS_LABEL )->SetWindowText( strTemp );
	
	strTemp = m_ExtStr.LoadString( IDS_SETTINGS_MSG_SEC_LIMIT_ENG + g_unStringLanguageIdOffset );	
	GetDlgItem( IDC_STATIC_OUTBOUND_UNITS_LABEL )->SetWindowText( strTemp );
	
	strTemp = m_ExtStr.LoadString( IDS_SETTINGS_INITIAL_DL_TIMEOUT_ENG + g_unStringLanguageIdOffset );	
	GetDlgItem( IDC_STATIC_INITIAL_DL_TO_LABEL )->SetWindowText( strTemp );
	
	strTemp = m_ExtStr.LoadString( IDS_SETTINGS_DELETE_CANCELED_DL_ENG + g_unStringLanguageIdOffset );	
	GetDlgItem( IDC_STATIC_DELETECANCELED_LABEL )->SetWindowText( strTemp );
	
	strTemp = m_ExtStr.LoadString( IDS_SETTINGS_MAINTAIN_ATLEAST_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_STATIC_MAINTAIN_LABEL )->SetWindowText( strTemp );
	
	strTemp = m_ExtStr.LoadString( IDS_SETTINGS_NEIGHBORCONNECTIONS_ENG + g_unStringLanguageIdOffset );	
	GetDlgItem( IDC_STATIC_NEIGHBORS_LABEL )->SetWindowText( strTemp );
	
	strTemp = m_ExtStr.LoadString( IDS_SETTINGS_ALLOW_ATMOST_ENG + g_unStringLanguageIdOffset );	
	GetDlgItem( IDC_STATIC_ALLOW_LABEL )->SetWindowText( strTemp );
	
	strTemp = m_ExtStr.LoadString( IDS_SETTINGS_NEIGHBORCONNECTIONS_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_STATIC_NEIGHBORS_LABEL_2 )->SetWindowText( strTemp );
	
	strTemp = m_ExtStr.LoadString( IDS_SETTINGS_MAX_UPLOADS_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_STATIC_MAX_UPLOADS )->SetWindowText( strTemp );
	
	strTemp = m_ExtStr.LoadString( IDS_SETTINGS_MAX_UPLOADS_PER_VIP_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_STATIC_MAX_UPLOADS_PER_VIP )->SetWindowText( strTemp );
	
	strTemp = m_ExtStr.LoadString( IDS_SETTINGS_SHAREFILESFROM_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_STATIC_SHARE_LABEL )->SetWindowText( strTemp );
	
	strTemp = m_ExtStr.LoadString( IDS_BTN_BROWSE_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_BROWSE_SHARED_BUTTON )->SetWindowText( strTemp );
	
	strTemp = m_ExtStr.LoadString( IDS_SETTINGS_INCOMING_FOLDER_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_STATIC_INCOMING_LABEL )->SetWindowText( strTemp );
	
	strTemp = m_ExtStr.LoadString( IDS_BTN_BROWSE_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_INCOMING_BROWSE_BUTTON )->SetWindowText( strTemp );
	
	strTemp = m_ExtStr.LoadString( IDS_SETTINGS_HASH_FOLDER_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_STATIC_HASH_LABEL )->SetWindowText( strTemp );
	
	strTemp = m_ExtStr.LoadString( IDS_BTN_BROWSE_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_HASH_BROWSE_BUTTON )->SetWindowText( strTemp );
	
	strTemp = m_ExtStr.LoadString( IDS_SETTINGS_SAVE_SETTINGS_BTN_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_SAVE_SETTINGS_BUTTON )->SetWindowText( strTemp );

	if(m_ctrlComboDeleteCanceledDLs.GetCount()>1)m_ctrlComboDeleteCanceledDLs.ResetContent();
	m_ctrlComboDeleteCanceledDLs.Clear();

	m_nDeleteFromIncoming=SettingsManager::getIntSetting( "deleteCanceled", &cValueFound );		
	
	strTemp = m_ExtStr.LoadString( IDS_NO_ENG + g_unStringLanguageIdOffset );
	m_ctrlComboDeleteCanceledDLs.InsertString(0, strTemp );	

	strTemp = m_ExtStr.LoadString( IDS_YES_ENG + g_unStringLanguageIdOffset );
	m_ctrlComboDeleteCanceledDLs.InsertString(1, strTemp );

    if(m_nDeleteFromIncoming==0) m_ctrlComboDeleteCanceledDLs.SetCurSel(0);
    if(m_nDeleteFromIncoming==1) m_ctrlComboDeleteCanceledDLs.SetCurSel(1);
}



void CMuteSettingsDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CResizableDialog::OnShowWindow(bShow, nStatus);	

	InitCurrentLanguage();
	SetStrings();
}


//*******************************************************************
//  FUNCTION:   -	ListExternLanguages()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-   Search for external languages files and add the
//					langages names in the language combo                 
//*******************************************************************//



void CMuteSettingsDlg::ListExternLanguages()
{
	int  nb=0;
	CString line;	
	CString LanguageFilePath;


	 // Get the setting directory
	
	 TCHAR szDirectory[MAX_PATH] = "";
     ::GetCurrentDirectory(sizeof(szDirectory) - 1, szDirectory);
	 CString CurrentDir = szDirectory;
	 CString SettingsDir= szDirectory;

	
     SettingsDir=CurrentDir + "\\settings\\";
	 

	 // List files in the settings directory with lang extension

	 CFileFind file;      
     SetCurrentDirectory (SettingsDir);
     
     BOOL bWorking=file.FindFile ("*.*"); 
     
     while (bWorking) 
	 { 
		bWorking = file.FindNextFile(); 
		if (!file.IsDirectory ())          // exclude directory from search            
		{		
			if (file.GetFileName().Right(4) ==".lng")
			{
				LanguageFilePath = SettingsDir + file.GetFileName();
				
				// if a language resource file is found ...
				if(LanguageFilePath.Right(4)==".lng")
				// let's get it's name...
				m_strExtLanguageName=file.GetFileName().Left(file.GetFileName().GetLength()-4);
				
				// if it don't still exist we add it to the language combo list
				
				if(m_ctrlCBLanguage.FindString(0, m_strExtLanguageName)==CB_ERR )
				{
					m_ctrlCBLanguage.AddString(m_strExtLanguageName);
				}
				
				// don't forget to display the the language icon if available 				
				CFileFind file;
				CString IconFilePath=LanguageFilePath;
				IconFilePath.Replace(".lng",".ico");

				if(file.FindFile(IconFilePath,0))   // If extern language icon found
				{				
					m_ctrlCBLanguage.SetIcon(m_ctrlCBLanguage.FindString(0, m_strExtLanguageName) ,IconFilePath);			
				}
				else								// Default icon
				{
					m_ctrlCBLanguage.SetIcon(m_ctrlCBLanguage.FindString(0, m_strExtLanguageName),IDI_EXTERN_LANG_ICON );
				}		

				nb++;
			}
		} 
	 } 
    
    file.Close ();
	SetCurrentDirectory (CurrentDir);
}


//*******************************************************************
//  FUNCTION:   -	GetLanguageFilePath
//  RETURNS:    -	CString LanguageFilePath
//  PARAMETERS: -	
//  COMMENTS:	-   Retrieve the pathname of an additional language                  
//*******************************************************************//
CString CMuteSettingsDlg::GetLanguageFilePath(CString strLangageName)
{	
	int  nb=0;
	CString line;

	// Get the setting directory

	TCHAR szDirectory[MAX_PATH] = "";
	::GetCurrentDirectory(sizeof(szDirectory) - 1, szDirectory);

	CString CurrentDir = szDirectory;
	CString SettingsDir= szDirectory;
	CString LanguageFilePath;
	CString strLanguage;

	LanguageFilePath.Empty();
	SettingsDir=CurrentDir + "\\settings\\";	 

	// List files in the settings directory with str extension

	CFileFind file;

	SetCurrentDirectory (SettingsDir);     
	BOOL bWorking=file.FindFile ("*.*");      

	while (bWorking) 
	{ 
		bWorking = file.FindNextFile(); 
		if (!file.IsDirectory ()) // exclude directory from search            
		{			
			if (file.GetFileName().Find(strLangageName) > -1)
			{
				if(file.GetFileName().Right(4)==".lng")
				{
					LanguageFilePath = SettingsDir+file.GetFileName();				
				}
				nb++;
			}
		} 
	} 

	file.Close ();
	SetCurrentDirectory (CurrentDir);

	return LanguageFilePath;
}