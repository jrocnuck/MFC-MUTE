// MuteSearchDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MFC_MUTE_GUI.h"
#include "MuteSearchDlg.h"
#include "MuteDownloadsDlg.h"

#include "MUTE/otherApps/fileSharing/userInterface/common/formatUtils.h"

#include "minorGems/util/SettingsManager.h"
#include <fstream>

#include "ColorNames.h"
#include "WinFileSysUtils.h"
#include ".\mutesearchdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMuteSearchDlg dialog


CMuteSearchDlg::CMuteSearchDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CMuteSearchDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMuteSearchDlg)
	m_strSearchEdit = _T("");
	m_strSearchExcludeEdit = _T("");
	m_strFileSizeEdit = _T("");
	m_nSearchBoolean = SEARCH_BOOLEAN_ENTIRE_PHRASE; // default to ENTIRE PHRASE for the search for words
	m_nFileSizeBoolean = SEARCH_FILESIZE_ATLEAST; // default to AT LEAST for the file size boolean
	//}}AFX_DATA_INIT
	m_oFileSizeEditCtrl.SetAllowNegative( false ); // no negative file sizes!

    mSearchLock =  new MutexLock();
	mSearchActive = false;
	mSearchCanceled = false;
	mResultLock = new MutexLock();
	mResultsFromAddresses = new SimpleVector<char *>();
	mResultsFilePaths = new SimpleVector<char *>();
	mResultsFileHashes = new SimpleVector<char *>();
	mResultsFileSizes = new SimpleVector<unsigned long>();
	//mActiveGaugeMaxValue( 10 ),
	//mActiveGaugeMotionDelta( 1 ),
	m_vecSearchThread.resize(0);
    
}

CMuteSearchDlg::~CMuteSearchDlg()
{
	// cancel any active search
    cancelActiveSearch();
    
    delete mSearchLock;
	
	
    int numResults = mResultsFromAddresses->size();
    for( int i=0; i<numResults; i++ ) {
        delete [] *( mResultsFromAddresses->getElement( i ) );
        delete [] *( mResultsFilePaths->getElement( i ) );
        delete [] *( mResultsFileHashes->getElement( i ) );
	}
    delete mResultsFromAddresses;
    delete mResultsFilePaths;
    delete mResultsFileHashes;
    delete mResultsFileSizes;    
    
    delete mResultLock;
}

void CMuteSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMuteSearchDlg)
	DDX_Control(pDX, IDC_COMBO_AND_OR, m_comboSearchBoolean);
	DDX_Control(pDX, IDC_COMBO_FILESIZE, m_comboFileSize);
	DDX_Control(pDX, IDC_SEARCHING_LED, m_oSearchingLED);
	DDX_Control(pDX, IDC_SEARCHING_TEXT, m_staticSearchingText);
	DDX_Control(pDX, IDC_SEARCHFOR_STATIC, m_oSearchForLabel);
	DDX_Control(pDX, IDC_EXCLUDE_STATIC, m_oExcludeLabel);
	DDX_Control(pDX, IDC_SEARCH_STRING_EDIT, m_SearchEdit);
	DDX_Control(pDX, IDC_SEARCH_LIST, m_SearchList);
	DDX_Text(pDX, IDC_SEARCH_STRING_EDIT, m_strSearchEdit);
	DDX_Text(pDX, IDC_SEARCH_STRING_EXCLUDE_EDIT, m_strSearchExcludeEdit );
	DDX_Control(pDX, IDC_FILESIZE_EDIT, m_oFileSizeEditCtrl );
	DDX_Text(pDX, IDC_FILESIZE_EDIT, m_strFileSizeEdit );
	DDX_CBIndex(pDX, IDC_COMBO_AND_OR, m_nSearchBoolean);
	DDX_CBIndex(pDX, IDC_COMBO_FILESIZE, m_nFileSizeBoolean);
	DDX_Control(pDX, IDC_PURGE_HASH_FOLDER, m_btnPurgeHashFolder);	
	DDX_Control(pDX, IDC_EXPORT_SEARCH_RESULTS_BUTTON, m_btnExportResultsButton);	
	DDX_Control(pDX, IDC_CLEAR_SEARCH_RESULTS_BUTTON, m_btnClearResultsButton);
	DDX_Control(pDX, IDC_DOWNLOAD_BUTTON, m_btnDownloadButton);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMuteSearchDlg, CResizableDialog)
	//{{AFX_MSG_MAP(CMuteSearchDlg)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_SEARCH_BUTTON, OnSearchButton)
	ON_BN_CLICKED(IDC_STOP_BUTTON, OnStopButton)
	ON_MESSAGE( SEARCH_TIMEOUT_EVENT, OnSearchTimeout )
	ON_MESSAGE( SEARCH_RESULTS_EVENT, OnSearchResults )	
	ON_MESSAGE( SEARCH_STOP_EVENT, DoStopSearch )
	ON_MESSAGE( MUTE_SEARCH_DO_DOWNLOAD_EVENT, DoDownload )
	ON_BN_CLICKED(IDC_EXPORT_SEARCH_RESULTS_BUTTON, OnExportSearchResultsButton)
	ON_BN_CLICKED(IDC_PURGE_HASH_FOLDER, OnPurgeHashFolder)
	ON_BN_CLICKED(IDC_DOWNLOAD_BUTTON, OnDownloadButton )
	ON_BN_CLICKED(IDC_CLEAR_SEARCH_RESULTS_BUTTON, OnClearResults)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMuteSearchDlg message handlers



//*******************************************************************
//  FUNCTION:   -	OnClose()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteSearchDlg::OnClose() 
{
	cancelActiveSearch();
}

//*******************************************************************
//  FUNCTION:   -	PreTranslateMessage()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
BOOL CMuteSearchDlg::PreTranslateMessage(MSG* pMsg) 
{
	if ( WM_KEYDOWN == pMsg->message )
	{		
		if( GetFocus() == (CWnd*) &m_SearchEdit )
		{
			// if user hits enter key within search edit box
			if( VK_RETURN == pMsg->wParam )
			{
				OnSearchButton();
				return FALSE;
			}
			else if( VK_ESCAPE == pMsg->wParam )
			{
				return FALSE;
			}
		}
		else if( GetFocus() == (CWnd*) &m_SearchList )
		{
			// if user hits enter key within the search list control
			if( VK_RETURN == pMsg->wParam )
			{
				OnDownloadButton();				
				return FALSE;
			}
			else if( VK_ESCAPE == pMsg->wParam )
			{
				return FALSE;
			}
		}
		else
		{
			if ( ( VK_ESCAPE == pMsg->wParam ) || ( VK_RETURN == pMsg->wParam ) )
			{
				// Don't close the window if the user hits the ESC key.
				return FALSE;
			}
		}
	}
	
	return CResizableDialog::PreTranslateMessage(pMsg);
}

//*******************************************************************
//  FUNCTION:   -	OnSearchButton()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteSearchDlg::OnSearchButton() 
{
	int				i;
	SearchThread	*pSearchThread;
	CString			strTemp;

	UpdateData(TRUE);

	m_strSearchEdit.TrimLeft();
	m_strSearchEdit.TrimRight();
	m_strSearchEdit.MakeLower();
	m_strSearchExcludeEdit.TrimLeft();
	m_strSearchExcludeEdit.TrimRight();
	m_strSearchExcludeEdit.MakeLower();
	m_strFileSizeEdit.TrimLeft();
	m_strFileSizeEdit.TrimRight();	

	if( m_strSearchEdit.IsEmpty() )
	{
		// don't do any searching if the search field is empty!
		return;
	}
	m_oSearchParser.vGo( (LPCSTR)m_strSearchEdit, ' ', FALSE );
	m_oSearchExcludeParser.vGo( (LPCSTR)m_strSearchExcludeEdit, ' ', FALSE );
                    
    // delete old results, if there are any
    mResultLock->lock();

    m_SearchList.DeleteAllItems();
	m_SearchList.ResetCaches();
	m_SearchList.Invalidate();
	m_vecSearchItems.resize(0);
	m_vecSearchItems.reserve(100000); // without this, the list will crash on sorting


    int numResults = mResultsFromAddresses->size();
    for( i = 0; i < numResults; i++ ) 
	{
        delete [] *( mResultsFromAddresses->getElement( i ) );
        delete [] *( mResultsFilePaths->getElement( i ) );
        delete [] *( mResultsFileHashes->getElement( i ) );
    }

    mResultsFromAddresses->deleteAll();
    mResultsFilePaths->deleteAll();
    mResultsFileHashes->deleteAll();
    mResultsFileSizes->deleteAll();
    
    mResultLock->unlock();

    // spawn thread for search
    mSearchLock->lock();
    mSearchCanceled = false;
    mSearchActive = true;
	for( i = 0; i < (int) m_vecSearchThread.size(); i++ )
	{
		if( NULL != m_vecSearchThread[i] )
		{
			delete m_vecSearchThread[i];
		}
	}
	m_vecSearchThread.resize(0);
	
	// need to have a search for each separate word in the search 
	// edit box
	if( SEARCH_BOOLEAN_ENTIRE_PHRASE == m_nSearchBoolean )
	{
		pSearchThread = new SearchThread( (LPCSTR) m_strSearchEdit, (void *)this );
		m_vecSearchThread.push_back( pSearchThread );
	}
	else
	{
		for( i = 0; i < m_oSearchParser.nGetNumFields(); i++ )
		{
			pSearchThread = new SearchThread( m_oSearchParser.m_strOutput[i].c_str(), (void *)this );
			m_vecSearchThread.push_back( pSearchThread );
		}
	}

    mSearchLock->unlock();
        

    GetDlgItem(IDC_SEARCH_BUTTON)->EnableWindow( FALSE );
	GetDlgItem(IDC_CLEAR_SEARCH_RESULTS_BUTTON)->EnableWindow( FALSE );
	GetDlgItem(IDC_DOWNLOAD_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_STOP_BUTTON)->EnableWindow( TRUE );    
	m_oSearchingLED.SetLed( CLed::LED_COLOR_GREEN, CLed::LED_OFF, CLed::LED_ROUND );
	strTemp = m_ExtStr.LoadString( IDS_SEARCH_SEARCHING_STATUS_ENG + g_unStringLanguageIdOffset );
	m_staticSearchingText.SetWindowText( strTemp );
    GetDlgItem(IDC_SEARCH_STRING_EDIT)->EnableWindow( FALSE );
	GetDlgItem(IDC_COMBO_AND_OR)->EnableWindow( FALSE );	
	GetDlgItem(IDC_FILESIZE_EDIT)->EnableWindow( FALSE );
	GetDlgItem(IDC_COMBO_FILESIZE)->EnableWindow( FALSE );
	GetDlgItem(IDC_SEARCH_STRING_EXCLUDE_EDIT)->EnableWindow( FALSE );
	GetDlgItem(IDC_EXPORT_SEARCH_RESULTS_BUTTON)->EnableWindow(FALSE);
	
	// make sure the ALT-XXX keys work for the appropriate buttons
	GetDlgItem(IDC_STOP_BUTTON)->SetFocus();

	SetStrings();
}

//*******************************************************************
//  FUNCTION:   -	OnStopButton()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteSearchDlg::OnStopButton() 
{
	// do this so we don't lock up the GUI in case the
	// search semaphore locks up... which happens and we are yet
	// to figure out why 7-19-04
	PostMessage( SEARCH_STOP_EVENT, 0, 0 );
	mSearchCanceled = true; // don't worry about the semaphore... 	
}


//*******************************************************************
//  FUNCTION:   -	DoStopSearch()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
LRESULT CMuteSearchDlg::DoStopSearch(WPARAM wParam, LPARAM lParam)
{
	CWaitCursor wait;
	CString		strTemp;

    // cancel the previous search, if there is one active
    cancelActiveSearch();

    GetDlgItem(IDC_STOP_BUTTON)->EnableWindow( FALSE );
    GetDlgItem(IDC_SEARCH_BUTTON)->EnableWindow( TRUE );
    GetDlgItem(IDC_SEARCH_STRING_EDIT)->EnableWindow( TRUE );
	GetDlgItem(IDC_SEARCH_STRING_EXCLUDE_EDIT)->EnableWindow( TRUE );
	GetDlgItem(IDC_COMBO_AND_OR)->EnableWindow( TRUE );
	GetDlgItem(IDC_FILESIZE_EDIT)->EnableWindow( TRUE );
	GetDlgItem(IDC_COMBO_FILESIZE)->EnableWindow( TRUE );
	if( m_SearchList.GetItemCount() > 0 )
	{
		GetDlgItem(IDC_EXPORT_SEARCH_RESULTS_BUTTON)->EnableWindow(TRUE);
		GetDlgItem(IDC_CLEAR_SEARCH_RESULTS_BUTTON)->EnableWindow(TRUE);
		GetDlgItem(IDC_DOWNLOAD_BUTTON)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_EXPORT_SEARCH_RESULTS_BUTTON)->EnableWindow(FALSE);
		GetDlgItem(IDC_CLEAR_SEARCH_RESULTS_BUTTON)->EnableWindow(FALSE);
		GetDlgItem(IDC_DOWNLOAD_BUTTON)->EnableWindow(FALSE);
	}
	m_oSearchingLED.SetLed( CLed::LED_COLOR_RED, CLed::LED_OFF, CLed::LED_ROUND ); 
	strTemp = m_ExtStr.LoadString( IDS_SEARCH_IDLE_STATUS_ENG + g_unStringLanguageIdOffset );
	m_staticSearchingText.SetWindowText( strTemp );
 
	// make sure the ALT-XXX keys work for the appropriate buttons
	GetDlgItem(IDC_SEARCH_LIST)->SetFocus();

	SetStrings();
	return (LRESULT) TRUE;
}

//*******************************************************************
//  FUNCTION:   -	DoDownload()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
LRESULT CMuteSearchDlg::DoDownload(WPARAM wParam, LPARAM lParam)
{
	CWaitCursor	wait;
	CString		strFromAddress;
	CString		strFilePath;
	CString		strFileHash;
	
	mResultLock->lock();

    int count = m_SearchList.GetItemCount();	

    for( int i=0; i<count; i++ ) 
	{
        if( m_SearchList.GetItemState( i, LVIS_SELECTED ) ) 
		{
			CMuteSearchListItem * pListItem = (CMuteSearchListItem *) m_SearchList.GetItemData( i );
			strFromAddress = pListItem->m_strHostVirtualAddress;
			strFilePath = pListItem->m_strFileName;
			strFileHash = pListItem->m_strHash;
			
			if( NULL != m_pDownloadDialog )
			{
				CString str;								
				( (CMuteDownloadsDlg *)m_pDownloadDialog )->addDownloadFromSearch( (LPCSTR)strFromAddress , (LPCSTR) strFilePath, (LPCSTR) strFileHash );
				pListItem->m_bIsDownloading = true;
				str = m_ExtStr.LoadString( IDS_ADDED_TO_DLOADS_PREFIX_ENG + g_unStringLanguageIdOffset );				

				if( -1 == pListItem->m_strFileName.Find(str) )
				{
					// don't add "Downloading more than once!!
					str += pListItem->m_strFileName;					
					m_SearchList.SetItemText( i,0, str);
				}				
			}
		}
	}	

    mResultLock->unlock();
	return (LRESULT) TRUE;
}

//*******************************************************************
//  FUNCTION:   -	OnInitDialog()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	Called upon creation of the dialog window.
//*******************************************************************
BOOL CMuteSearchDlg::OnInitDialog() 
{
	CString	strTemp;

	CResizableDialog::OnInitDialog();
	ShowSizeGrip(FALSE);
		
	m_SearchList.SetParent((CWnd *) this );
    m_SearchList.Init();
	
    m_oSearchingLED.LoadBitmapResource( IDB_LED_BITMAPS );
	m_oSearchingLED.SetLed( CLed::LED_COLOR_RED, CLed::LED_OFF, CLed::LED_ROUND );

	// make search for text green
	m_oSearchForLabel.SetTextColor( colGreen );
	m_oSearchForLabel.SetFontBold(TRUE);

	// make exclude text red
	m_oExcludeLabel.SetTextColor( colIndianRed );
	m_oExcludeLabel.SetFontBold(TRUE);

	GetDlgItem(IDC_STOP_BUTTON)->EnableWindow( FALSE );
	GetDlgItem(IDC_EXPORT_SEARCH_RESULTS_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_CLEAR_SEARCH_RESULTS_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_DOWNLOAD_BUTTON)->EnableWindow(FALSE);    
    AddAnchor( IDC_SEARCH_STRING_EDIT, TOP_LEFT, TOP_RIGHT );
	AddAnchor( IDC_SEARCH_LIST, TOP_LEFT, BOTTOM_RIGHT );
	AddAnchor( IDC_SEARCH_BUTTON, BOTTOM_LEFT );
	AddAnchor( IDC_STOP_BUTTON, BOTTOM_LEFT );
	AddAnchor( IDC_SEARCHING_LED, BOTTOM_LEFT );
	AddAnchor( IDC_SEARCHING_TEXT, BOTTOM_LEFT );
	AddAnchor( m_btnClearResultsButton, BOTTOM_RIGHT );
	AddAnchor( m_btnDownloadButton, BOTTOM_RIGHT );		
	AddAnchor( IDC_PURGE_HASH_FOLDER, BOTTOM_RIGHT );
	AddAnchor( m_btnExportResultsButton, BOTTOM_RIGHT );
	AddAnchor( IDC_STATIC_RECTANGLE, TOP_RIGHT );
	AddAnchor( IDC_COMBO_AND_OR, TOP_RIGHT );
	AddAnchor( IDC_SEARCHFOR_STATIC, TOP_LEFT );
	AddAnchor( IDC_SEARCH_BOOLEAN_STATIC, TOP_RIGHT );
	AddAnchor( IDC_EXCLUDE_STATIC, TOP_LEFT );
	AddAnchor( IDC_SEARCH_STRING_EXCLUDE_EDIT, TOP_LEFT, TOP_RIGHT );
	AddAnchor( IDC_FILESIZE_STATIC, TOP_RIGHT );
	AddAnchor( IDC_FILESIZE_EDIT, TOP_RIGHT );
	AddAnchor( IDC_FILESIZE_BOOLEAN_STATIC, TOP_RIGHT );
	AddAnchor( IDC_COMBO_FILESIZE, TOP_RIGHT );

	m_btnPurgeHashFolder.SetIcon( IDI_TRASH_ICON, NULL );
	m_btnPurgeHashFolder.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, 200);
	strTemp = m_ExtStr.LoadString( IDS_TOOLTIP_PURGE_HASH_FILES_ENG + g_unStringLanguageIdOffset );
	m_btnPurgeHashFolder.SetTooltipText(strTemp);	
	strTemp = m_ExtStr.LoadString( IDS_SEARCH_PURGE_HASH_BUTTON_TEXT_ENG + g_unStringLanguageIdOffset );
	m_btnPurgeHashFolder.SetWindowText( strTemp );

	strTemp = m_ExtStr.LoadString( IDS_SEARCH_SEARCH_BUTTON_TEXT_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_SEARCH_BUTTON )->SetWindowText( strTemp );

	strTemp = m_ExtStr.LoadString( IDS_SEARCH_STOP_BUTTON_TEXT_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_STOP_BUTTON )->SetWindowText( strTemp );

	strTemp = m_ExtStr.LoadString( IDS_SEARCH_EXPORT_BUTTON_TEXT_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_EXPORT_SEARCH_RESULTS_BUTTON )->SetWindowText( strTemp );

	strTemp = m_ExtStr.LoadString( IDS_SEARCH_CLEARRESULTS_BUTTON_TEXT_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_CLEAR_SEARCH_RESULTS_BUTTON )->SetWindowText( strTemp );
	
	strTemp = m_ExtStr.LoadString( IDS_SEARCH_DLOAD_BUTTON_TEXT_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_DOWNLOAD_BUTTON )->SetWindowText( strTemp );

	strTemp = m_ExtStr.LoadString( IDS_SEARCH_SEARCHFOR_TEXT_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_SEARCHFOR_STATIC )->SetWindowText( strTemp );

	strTemp = m_ExtStr.LoadString( IDS_SEARCH_EXPORT_SEARCH_BOOLEAN_TAG_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_SEARCH_BOOLEAN_STATIC )->SetWindowText( strTemp );

	strTemp = m_ExtStr.LoadString( IDS_SEARCH_SEARCHEXCLUDE_TEXT_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_EXCLUDE_STATIC )->SetWindowText( strTemp );

	strTemp = m_ExtStr.LoadString( IDS_SEARCH_FILE_SIZE_LIMITS_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_FILESIZE_STATIC )->SetWindowText( strTemp );

	strTemp = m_ExtStr.LoadString( IDS_SEARCH_FILE_SIZE_BOOLEAN_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_FILESIZE_BOOLEAN_STATIC )->SetWindowText( strTemp );

	strTemp = m_ExtStr.LoadString( IDS_SEARCH_BOOLEAN_ATLEAST_ENG + g_unStringLanguageIdOffset );
	m_comboFileSize.AddString( strTemp );
	strTemp = m_ExtStr.LoadString( IDS_SEARCH_BOOLEAN_ATMOST_ENG + g_unStringLanguageIdOffset );
	m_comboFileSize.AddString( strTemp );

	strTemp = m_ExtStr.LoadString( IDS_SEARCH_BOOLEAN_AND_ENG + g_unStringLanguageIdOffset );
	m_comboSearchBoolean.AddString( strTemp );

	strTemp = m_ExtStr.LoadString( IDS_SEARCH_BOOLEAN_OR_ENG + g_unStringLanguageIdOffset );
	m_comboSearchBoolean.AddString( strTemp );

	strTemp = m_ExtStr.LoadString( IDS_SEARCH_BOOLEAN_ENTIRE_PHRASE_ENG + g_unStringLanguageIdOffset );
	m_comboSearchBoolean.AddString( strTemp );

	strTemp = m_ExtStr.LoadString( IDS_SEARCH_IDLE_STATUS_ENG + g_unStringLanguageIdOffset );
	m_staticSearchingText.SetWindowText( strTemp );
	((CStatic*)GetDlgItem(IDC_SEARCHLST_ICO))->SetIcon((HICON)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SEARCH2_ICON), IMAGE_ICON, 16, 16, 0));

	UpdateData(FALSE); // GET COMBO BOXES RIGHT... 

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//*******************************************************************
//  FUNCTION:   -	cancelActiveSearch()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteSearchDlg::cancelActiveSearch() {

	int		i;
	
	mSearchLock->lock();
    if( mSearchActive ) {
        mSearchCanceled = true;

        mSearchLock->unlock();
        
		Sleep(500);

        // wait for search thread to return
		for( i = 0; i < (int) m_vecSearchThread.size(); i++ )
		{
			if( NULL != m_vecSearchThread[i] )
			{
				delete m_vecSearchThread[i];				
			}
		}
		m_vecSearchThread.resize(0);

        mSearchLock->lock();
        mSearchActive = false;

        mSearchLock->unlock();
        }
    else {
        mSearchLock->unlock();
        }
}


// callback for incoming search results
char searchResultHandler( char *inFromAddress, char *inFilePath,
                          unsigned long inFileSize,
                          char *inFileHash,
                          void *inExtraParam ) {
    // unwrap panel from extra parameter
    CMuteSearchDlg			*pSearchDlg = (CMuteSearchDlg *)inExtraParam;
	CMuteSearchListItem		searchItem;       
    
    
    char timeout = true;
	char keepGoing = true;

	if( pSearchDlg->mSearchCanceled ) 
	{
        keepGoing = false;
		return keepGoing;
    }
    
    if( inFromAddress != NULL ) 
	{
        // got results, no timeout
        timeout = false;

        pSearchDlg->mResultLock->lock();

		searchItem.m_strFileName = inFilePath;
		searchItem.m_unFileSize = inFileSize;		
        searchItem.m_strHash = inFileHash;
		searchItem.m_strHostVirtualAddress = inFromAddress;
			
		if( pSearchDlg->DoWeKeepSearchResult( searchItem ) )
		{
			pSearchDlg->mResultsFromAddresses->push_back(
				stringDuplicate( inFromAddress ) );
			pSearchDlg->mResultsFilePaths->push_back(
				stringDuplicate( inFilePath ) );
			pSearchDlg->mResultsFileHashes->push_back(
				stringDuplicate( inFileHash ) );
			pSearchDlg->mResultsFileSizes->push_back(
				inFileSize );
		}
    
		pSearchDlg->mResultLock->unlock();
    }
    // else we timed out waiting for more results    


	pSearchDlg->mSearchLock->lock();

    if( pSearchDlg->mSearchCanceled ) 
	{
        keepGoing = false;		
    }

    pSearchDlg->mSearchLock->unlock();

    // only post event if we are not canceled
    // this prevents a GUI thread deadlock
    if( keepGoing ) {
        
        // post event so that GUI thread can update results display
        if( timeout ) {
			if( ::IsWindow( pSearchDlg->GetSafeHwnd() ) )
			{
				pSearchDlg->PostMessage(SEARCH_TIMEOUT_EVENT,0,0);				
			}
        }
        else {
			if( NULL != pSearchDlg )
			{
				if( ::IsWindow( pSearchDlg->GetSafeHwnd() ) )
				{
					pSearchDlg->PostMessage(SEARCH_RESULTS_EVENT,0,0);					
				}
			}
        }
     }

    return keepGoing;
}

//*******************************************************************
//  FUNCTION:   -	
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
LRESULT CMuteSearchDlg::OnSearchTimeout(WPARAM wParam, LPARAM lParam)
{
    // only update our gauge if we haven't been canceled
    // (ignores any lingering events that come through after we are canceled)
    char shouldUpdate;
    mSearchLock->lock();
    shouldUpdate = !mSearchCanceled;
    mSearchLock->unlock();

	
    if( shouldUpdate ) {
        // animate our activity gauge to indicate that we are still waiting
        // for results (that we timed out)
		m_oSearchingLED.Ping(200);
        }
   
	return (LRESULT) 0;
}

//*******************************************************************
//  FUNCTION:   -	
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
LRESULT CMuteSearchDlg::OnSearchResults(WPARAM wParam, LPARAM lParam)
{
	CMuteSearchListItem		searchItem;
	bool					bToggle = false;
	MSG						msg;
	CString					strTemp;

	mResultLock->lock();


    int firstItemToAdd = m_vecSearchItems.size();

    int lastItemToAdd = mResultsFilePaths->size() - 1;

	
    if( firstItemToAdd <= lastItemToAdd ) 
	{
        // add each item that we're missing

        for( int i=firstItemToAdd; i<=lastItemToAdd; i++ ) 
		{
			if( bToggle )
			{
				bToggle = false;
				m_oSearchingLED.SetLed(CLed::LED_COLOR_GREEN,CLed::LED_ON,CLed::LED_ROUND);
			}
			else
			{
				bToggle = true;
				m_oSearchingLED.SetLed(CLed::LED_COLOR_GREEN,CLed::LED_OFF,CLed::LED_ROUND);
			}
            char *sizeString = formatDataSizeWithUnits(
                *( mResultsFileSizes->getElement( i ) ) );
        
			searchItem.m_strFileName = *( mResultsFilePaths->getElement(i) );
			searchItem.m_unFileSize = *( mResultsFileSizes->getElement(i) );
			searchItem.m_strFileSize = sizeString;
            searchItem.m_strHash = *( mResultsFileHashes->getElement(i) );
			searchItem.m_strHostVirtualAddress = *( mResultsFromAddresses->getElement(i) );
			
			m_vecSearchItems.push_back(searchItem);
			m_SearchList.AddSearchResult( (CMuteSearchListItem *) &(m_vecSearchItems[m_vecSearchItems.size()-1]) );
			GetDlgItem(IDC_DOWNLOAD_BUTTON)->EnableWindow(TRUE);

			// JROC -- 02-20-2005
			// 0xFF == WM_INPUT
			while( ::PeekMessage( &msg, NULL, 0xff, 0xff, PM_REMOVE ) )
			{
				::DispatchMessage( &msg );
			}
			delete [] sizeString;
        }		
	}
	
	strTemp.Format( "%s (%d)",
                    m_ExtStr.LoadString( IDS_SEARCH_RESULTS_LBL_ENG + g_unStringLanguageIdOffset),
                    m_SearchList.GetItemCount() );
	GetDlgItem( IDC_RESULTS_LBL )->SetWindowText(strTemp);


    mResultLock->unlock();
    if( mResultsFilePaths->size() > 90000 )
	{
		// we've done enough.. let's stop the search!!
		OnStopButton();		
	}

	return (LRESULT) 0;
}


//*******************************************************************
//  FUNCTION:   -	OnDownloadButton()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteSearchDlg::OnDownloadButton()
{
	CWaitCursor	wait;
	PostMessage( MUTE_SEARCH_DO_DOWNLOAD_EVENT,0,0);
}

//*******************************************************************
//  FUNCTION:   -	OnClearResults()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteSearchDlg::OnClearResults()
{
	// delete old results, if there are any
    mResultLock->lock();
	
	m_SearchList.DeleteAllItems();
	m_SearchList.ResetCaches();

	m_SearchList.Invalidate();
	m_vecSearchItems.resize(0);
	m_vecSearchItems.reserve(100000); // without this, the list will crash on sorting
	int numResults = mResultsFromAddresses->size();
    for( int i=0; i<numResults; i++ ) {
        delete [] *( mResultsFromAddresses->getElement( i ) );
        delete [] *( mResultsFilePaths->getElement( i ) );
        delete [] *( mResultsFileHashes->getElement( i ) );
        }
    mResultsFromAddresses->deleteAll();
    mResultsFilePaths->deleteAll();
    mResultsFileHashes->deleteAll();
    mResultsFileSizes->deleteAll();
    
    mResultLock->unlock();

	m_strSearchEdit.Empty();
	m_strSearchExcludeEdit.Empty();
	m_strFileSizeEdit.Empty();
	m_nSearchBoolean = SEARCH_BOOLEAN_ENTIRE_PHRASE;// default to ENTIRE PHRASE for the search for words
	m_nFileSizeBoolean = SEARCH_FILESIZE_ATLEAST; // default to AT LEAST for the file size boolean
	UpdateData(FALSE);	
	GetDlgItem(IDC_EXPORT_SEARCH_RESULTS_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_CLEAR_SEARCH_RESULTS_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_DOWNLOAD_BUTTON)->EnableWindow(FALSE);

	SetStrings();
}


//*******************************************************************
//  FUNCTION:   -	OnExportSearchResultsButton()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	Open up a dialog to help folks save the current
//					search results to a text file.
//*******************************************************************
void CMuteSearchDlg::OnExportSearchResultsButton() 
{	
	CString		strFileNameWithPath;
	CFileDialog oFileDlg( FALSE, "", NULL, NULL ,"Text Files (*.txt)|*.txt||", NULL );
	ofstream	out;
	int			i;	
	CString		strTemp, strAtLeast, strAtMost;
		
	if( m_vecSearchItems.size() <= 0 )
		return;

	strTemp = m_ExtStr.LoadString( IDS_EXPORT_SEARCH_DLG_CAPTION_ENG + g_unStringLanguageIdOffset );
	oFileDlg.m_ofn.lpstrTitle = (LPCSTR) strTemp;
	
	if( IDOK != oFileDlg.DoModal() )
	{
		return;
	}
	
	strFileNameWithPath = oFileDlg.GetPathName();
	
	out.open( (LPCSTR) strFileNameWithPath );
	if( out.is_open() )
	{
		strTemp = m_ExtStr.LoadString( IDS_SEARCH_EXPORT_SEARCH_TEXT_TAG_ENG + g_unStringLanguageIdOffset );
		out << (LPCSTR) strTemp << " """ << (LPCSTR) m_strSearchEdit << """" << endl;
		strTemp = m_ExtStr.LoadString( IDS_SEARCH_EXPORT_SEARCH_BOOLEAN_TAG_ENG + g_unStringLanguageIdOffset );
		out << (LPCSTR) strTemp;

		if( SEARCH_BOOLEAN_AND == m_nSearchBoolean )
		{
			strTemp = m_ExtStr.LoadString( IDS_SEARCH_BOOLEAN_AND_ENG + g_unStringLanguageIdOffset );			
		}
		else if( SEARCH_BOOLEAN_OR == m_nSearchBoolean )
		{
			strTemp = m_ExtStr.LoadString( IDS_SEARCH_BOOLEAN_OR_ENG + g_unStringLanguageIdOffset );			
		}
		else
		{
			strTemp = m_ExtStr.LoadString( IDS_SEARCH_BOOLEAN_ENTIRE_PHRASE_ENG + g_unStringLanguageIdOffset );
		}

		out << " " << (LPCSTR) strTemp << endl;


		strTemp = m_ExtStr.LoadString( IDS_SEARCH_EXCLUDE_TEXT_ENG + g_unStringLanguageIdOffset );
		out << (LPCSTR) strTemp << " """ << (LPCSTR) m_strSearchExcludeEdit << """" << endl;
		
		strTemp = m_ExtStr.LoadString( IDS_SEARCH_FILE_SIZE_LIMITS_ENG + g_unStringLanguageIdOffset );
		out << (LPCSTR) strTemp << " """ << (LPCSTR) m_strFileSizeEdit << """" << endl;
		
		strAtLeast=m_ExtStr.LoadString( IDS_SEARCH_BOOLEAN_ATLEAST_ENG + g_unStringLanguageIdOffset );
		strAtMost=m_ExtStr.LoadString( IDS_SEARCH_BOOLEAN_ATMOST_ENG + g_unStringLanguageIdOffset );
		
		strTemp = m_ExtStr.LoadString( IDS_SEARCH_FILE_SIZE_BOOLEAN_ENG + g_unStringLanguageIdOffset );
		out << (LPCSTR) strTemp << " """ << ( (SEARCH_FILESIZE_ATLEAST == m_nFileSizeBoolean ) ? (LPCSTR) strAtLeast : (LPCSTR) strAtMost ) << """" << endl;
		
		strTemp = m_ExtStr.LoadString( IDS_SEARCH_EXPORT_FILE_COL_HEADINGS_ENG + g_unStringLanguageIdOffset );
		out << (LPCSTR) strTemp << endl;

		for( i = 0; i < (int) m_vecSearchItems.size(); i++ )
		{			
			out << (LPCSTR) m_vecSearchItems[i].m_strFileName;
			out << " , ";
			out << (LPCSTR) m_vecSearchItems[i].m_strFileSize;
			out << " , ";
			out << (LPCSTR) m_vecSearchItems[i].m_strHash;
			out << " , ";
			out << (LPCSTR) m_vecSearchItems[i].m_strHostVirtualAddress;
			out << endl;		 
		}

		out.close();

		// reset current path // for some reason this export search
		// results causes future downloads to say cannot create
		// apparently the CFileDialog class screws stuff up by
		// changing the current directory.. theoretically... 
		// this program should use absolute directories rather than
		// relative dirs.. and this wouldn't be a problem.
		if( ! ( (CMUTEMFC2App *) AfxGetApp() )->m_strStartingPath.IsEmpty() )
		{
			SetCurrentDirectory( (LPCSTR) ( (CMUTEMFC2App *) AfxGetApp() )->m_strStartingPath );
		}
	}
}



//*******************************************************************
//  FUNCTION:   -	DoWeKeepSearchResult()
//  RETURNS:    -	bool - true == keep the search result item
//  PARAMETERS: -	
//  COMMENTS:	-	This function compares/checks the input search item
//					against the user's search filters.  If it meets the requirements
//					then the function returns true
//*******************************************************************
bool CMuteSearchDlg::DoWeKeepSearchResult( CMuteSearchListItem& item )
{
	int				i;
	int				nFileSize;
	bool			bAnyFound; // used for the OR search in the search field
	

	// ignore all booleans when first part of search is hash_
	if( "hash_" == m_strSearchEdit.Left(5) )
	{
		return true;
	}

	// for wild card seach.. (searching all files.. )
	if( "*" == m_strSearchEdit )
	{
		return true;
	}

	item.m_strFileName.MakeLower();

	// check the file size stuff first
	if( !m_strFileSizeEdit.IsEmpty() )
	{
		nFileSize = atoi( (LPCSTR) m_strFileSizeEdit );
		nFileSize *= 1024; // file size on gui in kilobytes	
		if( SEARCH_FILESIZE_ATLEAST == m_nFileSizeBoolean )
		{
			if( (int)item.m_unFileSize < nFileSize )
			{
				return( false );
			}
		}
		else
		{
			if( (int)item.m_unFileSize > nFileSize )
			{
				return( false );
			}
		}
	}

	// check the exclusion stuff next!
	if( !m_strSearchExcludeEdit.IsEmpty() )
	{
		for( i = 0; i < m_oSearchExcludeParser.nGetNumFields(); i++ )
		{	
			if( item.m_strFileName.Find( m_oSearchExcludeParser.m_strOutput[i].c_str(), 0 ) >= 0 )
			{
				return( false );
			}
		}
	}

	// check to see if the file should be filtered based on the
	// search edit box and the search boolean
	// note, we don't have to worry about the box being empty here
	// because we don't allow empty search edit boxes to search
	if( SEARCH_BOOLEAN_AND == m_nSearchBoolean )
	{
		// we have to check for all of the words...
		// if any one of them isn't found, we quit
		for( i = 0; i < m_oSearchParser.nGetNumFields(); i++ )
		{
			if( -1 == item.m_strFileName.Find( m_oSearchParser.m_strOutput[i].c_str() ) )
			{
				return( false );
			}
		}
	}
	else if( SEARCH_BOOLEAN_OR == m_nSearchBoolean )
	{
		bAnyFound = false;

		// we have to check for any of the words...
		// if any one of them is found, we quit searching
		for( i = 0; i < m_oSearchParser.nGetNumFields(); i++ )
		{
			if( item.m_strFileName.Find( m_oSearchParser.m_strOutput[i].c_str() ) >= 0 ) 
			{
				// we found one.. lets quit looking anymore
				bAnyFound = true;
				break;
			}
		}

		if( !bAnyFound )
		{
			return( false );
		}
	}
	else
	{
		if( -1 == item.m_strFileName.Find( m_strSearchEdit ) )
		{
			return( false );
		}
	}
	

	// last check for duplicates.. if already in list 
	// i.e. same filename, hash, and same virt address don't add again!
	for( i = 0; i < (int) m_vecSearchItems.size(); i++ )
	{
		// have to use no case comparison, because we make the 
		// filename all lower sometimes.. 
		if( 0 == m_vecSearchItems[i].m_strFileName.CompareNoCase( item.m_strFileName ) )
		{
			if( item.m_strHash == m_vecSearchItems[i].m_strHash )
			{
				if( item.m_strHostVirtualAddress == m_vecSearchItems[i].m_strHostVirtualAddress )
				{
					return( false );
				}
			}
		}
	}

	return true;
}


//*******************************************************************
//  FUNCTION:   -	OnPurgeHashFolder()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	Deletes all the hash files in your MUTE hashes folder
//					folder... this is good from time to time so that
//					if you move files you will reduce the possibility
//					of hash files being incorrect.
//*******************************************************************
void CMuteSearchDlg::OnPurgeHashFolder() 
{
	CString	strCaption, strMsg;
	char	*szHashPath = NULL;	
	
	strCaption = m_ExtStr.LoadString( IDS_PURGE_HASHES_MSGBOX_CAPTION_ENG + g_unStringLanguageIdOffset );
	strMsg = m_ExtStr.LoadString( IDS_PURGE_HASHES_MSGBOX_TEXT_ENG + g_unStringLanguageIdOffset );
	if( IDYES == MessageBox( strMsg, strCaption, MB_ICONQUESTION | MB_YESNO ) )
	{
		CWaitCursor	wait;
		szHashPath = muteShareGetHashFilesPath();
		
		if( NULL != szHashPath )
		{		
			EmptyDirectory( (LPCSTR) szHashPath, TRUE, TRUE );
			delete [] szHashPath;
		}	
	}
}

//*******************************************************************
//  FUNCTION:   -	SetStrings()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	Handles drawing the right language on the screen!
//*******************************************************************
void CMuteSearchDlg::SetStrings()
{
	CString strTemp;

	strTemp = m_ExtStr.LoadString(IDS_SEARCH_SEARCH_BUTTON_TEXT_ENG + g_unStringLanguageIdOffset);
	GetDlgItem(IDC_SEARCH_BUTTON)->SetWindowText(strTemp);
	
	strTemp = m_ExtStr.LoadString(IDS_SEARCH_STOP_BUTTON_TEXT_ENG + g_unStringLanguageIdOffset);
	GetDlgItem(IDC_STOP_BUTTON)->SetWindowText(strTemp);	
    
	m_btnExportResultsButton.SetIcon( IDI_EXPORT_RESULTS_ICON, NULL );
	m_btnExportResultsButton.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, 200);
	strTemp = m_ExtStr.LoadString(IDS_SEARCH_EXPORT_BUTTON_TEXT_ENG + g_unStringLanguageIdOffset);
	m_btnExportResultsButton.SetWindowText( strTemp );
	
	m_btnClearResultsButton.SetIcon( IDI_CLEAR_RESULTS_ICON, NULL );
	m_btnClearResultsButton.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, 200);
	strTemp = m_ExtStr.LoadString(IDS_SEARCH_CLEARRESULTS_BUTTON_TEXT_ENG + g_unStringLanguageIdOffset);
	m_btnClearResultsButton.SetWindowText( strTemp );
	
	m_btnDownloadButton.SetIcon( IDI_DOWNLOAD4_ICON, NULL );
	m_btnDownloadButton.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, 200);
	strTemp = m_ExtStr.LoadString(IDS_SEARCH_DLOAD_BUTTON_TEXT_ENG + g_unStringLanguageIdOffset);
	m_btnDownloadButton.SetWindowText( strTemp );	
	
	strTemp = m_ExtStr.LoadString(IDS_SEARCH_SEARCHFOR_TEXT_ENG + g_unStringLanguageIdOffset);
	GetDlgItem(IDC_SEARCHFOR_STATIC)->SetWindowText(strTemp);
    
	strTemp = m_ExtStr.LoadString(IDS_SEARCH_EXPORT_SEARCH_BOOLEAN_TAG_ENG + g_unStringLanguageIdOffset);
	GetDlgItem( IDC_SEARCH_BOOLEAN_STATIC )->SetWindowText(strTemp);
	
	strTemp = m_ExtStr.LoadString(IDS_SEARCH_SEARCHEXCLUDE_TEXT_ENG + g_unStringLanguageIdOffset);
	GetDlgItem( IDC_EXCLUDE_STATIC )->SetWindowText(strTemp);
	
	strTemp = m_ExtStr.LoadString(IDS_SEARCH_FILE_SIZE_LIMITS_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_FILESIZE_STATIC )->SetWindowText(strTemp);	
	
	strTemp = m_ExtStr.LoadString(IDS_SEARCH_FILE_SIZE_BOOLEAN_ENG + g_unStringLanguageIdOffset);
	GetDlgItem( IDC_FILESIZE_BOOLEAN_STATIC )->SetWindowText(strTemp);
	
	strTemp.Format( "%s (%d)",
                    m_ExtStr.LoadString( IDS_SEARCH_RESULTS_LBL_ENG + g_unStringLanguageIdOffset),
                    m_SearchList.GetItemCount() );
	GetDlgItem( IDC_RESULTS_LBL )->SetWindowText(strTemp);
	

	m_comboFileSize.ResetContent();
	m_comboFileSize.AddString(m_ExtStr.LoadString(IDS_SEARCH_BOOLEAN_ATLEAST_ENG + g_unStringLanguageIdOffset));	
	m_comboFileSize.AddString(m_ExtStr.LoadString(IDS_SEARCH_BOOLEAN_ATMOST_ENG + g_unStringLanguageIdOffset));
    
	m_comboSearchBoolean.ResetContent();	
	m_comboSearchBoolean.AddString(m_ExtStr.LoadString(IDS_SEARCH_BOOLEAN_AND_ENG + g_unStringLanguageIdOffset));	
	m_comboSearchBoolean.AddString(m_ExtStr.LoadString(IDS_SEARCH_BOOLEAN_OR_ENG + g_unStringLanguageIdOffset));	
	m_comboSearchBoolean.AddString(m_ExtStr.LoadString(IDS_SEARCH_BOOLEAN_ENTIRE_PHRASE_ENG + g_unStringLanguageIdOffset));	;
	
	// if searching need searching, otherwise, idle
	if( !mSearchActive )
	{
		m_staticSearchingText.SetWindowText(m_ExtStr.LoadString(IDS_SEARCH_IDLE_STATUS_ENG + g_unStringLanguageIdOffset));
	}
	else
	{
		m_staticSearchingText.SetWindowText(m_ExtStr.LoadString(IDS_SEARCH_SEARCHING_STATUS_ENG + g_unStringLanguageIdOffset));
	}

	m_btnPurgeHashFolder.SetTooltipText(m_ExtStr.LoadString(IDS_TOOLTIP_PURGE_HASH_FILES_ENG + g_unStringLanguageIdOffset));	
	m_btnPurgeHashFolder.SetWindowText(m_ExtStr.LoadString(IDS_SEARCH_PURGE_HASH_BUTTON_TEXT_ENG + g_unStringLanguageIdOffset));

	UpdateData(FALSE);
}


void CMuteSearchDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CResizableDialog::OnShowWindow(bShow, nStatus);

	SetStrings();
	m_SearchList.SetStrings();	
}
