/* 02-14-2005 JROC -->

    -- On the Uploads screen, when files are selected, the DELETE key performs 
    the cancel/clear function.
*/

#include "stdafx.h"
#include "MFC_MUTE_GUI.h"
#include "MuteUploadsDlg.h"

#include "MUTE/otherApps/fileSharing/userInterface/common/formatUtils.h"
#include "MUTE/otherApps/fileSharing/fileShare.h"
#include "minorGems/util/stringUtils.h"
#include <time.h>
#include <stdio.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMuteUploadsDlg dialog


CMuteUploadsDlg::CMuteUploadsDlg(CWnd* pParent /*=NULL*/)
: CResizableDialog(CMuteUploadsDlg::IDD, pParent)
{

	//{{AFX_DATA_INIT(CMuteUploadsDlg)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT	
	mUploadListIndexVector = NULL;
	m_nDontUpdateCtr = 0;
	m_ulChunksUploaded = 0;	
}

//*******************************************************************
//  FUNCTION:   -	~CMuteUploadsDlg
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	 
//*******************************************************************
CMuteUploadsDlg::~CMuteUploadsDlg()
{
	if( NULL != mUploadListIndexVector )
	{		
		while( mUploadListIndexVector->size() > 0 )
		{
			mUploadListIndexVector->deleteElement(0);
		}
		delete mUploadListIndexVector;
		mUploadListIndexVector = NULL;
	}
}

void CMuteUploadsDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMuteUploadsDlg)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_UPLOADS_LIST, m_UploadList);
	DDX_Control(pDX, IDC_UPLOADS_TOTAL_SHOWN_LABEL, m_oUploadsCntLabel);
	DDX_Control(pDX, IDC_UPLOADS_CHUNKS_UPLOADED_LABEL, m_oChunksCntLabel );
	DDX_Control(pDX, IDC_REMOVE_ALL_UPLOAD_BUTTON, m_btnRemoveAllUpload );
	DDX_Control(pDX, IDC_CANCEL_BUTTON, m_btnClearSelectedUpload );
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMuteUploadsDlg, CResizableDialog)
//{{AFX_MSG_MAP(CMuteUploadsDlg)
	ON_WM_CLOSE()
	ON_MESSAGE( UPLOADS_UPDATE_EVENT, OnUploadsUpdate )
	ON_MESSAGE( UPLOAD_CANCEL_EVENT, OnUploadCancel )
	ON_MESSAGE( UPLOAD_REMOVE_ALL_EVENT, OnUploadRemoveAll )
	ON_BN_CLICKED(IDC_REMOVE_ALL_UPLOAD_BUTTON, OnRemoveAllUploadsButton)
	ON_BN_CLICKED(IDC_CANCEL_BUTTON, OnCancelButton)
	ON_WM_TIMER()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMuteUploadsDlg message handlers

BOOL CMuteUploadsDlg::PreTranslateMessage(MSG* pMsg) 
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( (pMsg->wParam == VK_ESCAPE) || (pMsg->wParam == VK_RETURN) )
		{
			// Don't close the window if the user hits the ESC key.
			return FALSE;
		}

		if( VK_DELETE == pMsg->wParam )
		{
			PostMessage( UPLOAD_CANCEL_EVENT, NULL, NULL );
			return FALSE;
		}
	}
	
	return CResizableDialog::PreTranslateMessage(pMsg);
}


//*******************************************************************
//  FUNCTION:   -	OnInitDialog()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
BOOL CMuteUploadsDlg::OnInitDialog()
{
	CString	str;
	CResizableDialog::OnInitDialog();
	ShowSizeGrip(FALSE);
	// vector shadows the visible UploadList, maintaining an upload ID for
    // each list entry.
	m_UploadList.SetParent((CWnd *) this );
	m_UploadList.Init();
	
	AddAnchor( IDC_UPLOADS_LIST, TOP_LEFT, BOTTOM_RIGHT );
	AddAnchor( m_btnRemoveAllUpload, BOTTOM_CENTER );
	AddAnchor( m_btnClearSelectedUpload, BOTTOM_CENTER );	
	AddAnchor( IDC_UPLOADS_TOTAL_SHOWN_LABEL, BOTTOM_LEFT );
	AddAnchor( IDC_UPLOADS_CHUNKS_UPLOADED_LABEL, BOTTOM_LEFT );

	m_btnRemoveAllUpload.SetIcon( IDI_CLEAR_RESULTS_ICON, NULL );
	m_btnRemoveAllUpload.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, 200);
	str = m_ExtStr.LoadString( IDS_ULOAD_BTN_REMOVALL_ENG + g_unStringLanguageIdOffset );
	m_btnRemoveAllUpload.SetWindowText( str );

	m_btnClearSelectedUpload.SetIcon( IDI_CANCEL_SELECTED_ICON, NULL );
	m_btnClearSelectedUpload.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, 200);
	str = m_ExtStr.LoadString( IDS_ULOAD_BTN_CANCELCLEAR_ENG + g_unStringLanguageIdOffset );
	m_btnClearSelectedUpload.SetWindowText( str );	


    mUploadListIndexVector = new SimpleVector<long>();
	m_vecUploadItems = new SimpleVector<CMuteUploadListItem *>();		

    ((CStatic*)GetDlgItem(IDC_UPLOADS_ICO))->SetIcon((HICON)::LoadImage(AfxGetInstanceHandle(),
            MAKEINTRESOURCE(IDI_UPLOAD2_ICON), IMAGE_ICON, 16, 16, 0));


	SetTimer( 666, 5000, NULL );
	return TRUE; // return TRUE unless you set the focus to a control
}



/*
//*******************************************************************
//  FUNCTION:    -	OnClose
//  RETURNS:     -	
//  PARAMETERS:  -	
//  COMMENTS:	 -	
//*******************************************************************
*/
void CMuteUploadsDlg::OnClose()
{			
	CMuteUploadListItem *item;

	KillTimer(666);
	
	m_UploadList.DeleteAllItems();

	if( NULL != mUploadListIndexVector )
	{		
		while( mUploadListIndexVector->size() > 0 )
		{
			mUploadListIndexVector->deleteElement(0);
			item = *( m_vecUploadItems->getElement( 0 ) );
			delete item;
			m_vecUploadItems->deleteElement(0);
		}
		delete mUploadListIndexVector;
		delete m_vecUploadItems;
		mUploadListIndexVector = NULL;
	}	
}

//*******************************************************************
//  FUNCTION:   -	OnTimer()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	Timer function
//*******************************************************************
void CMuteUploadsDlg::OnTimer( UINT_PTR nIDEvent )
{
	if( 666 == nIDEvent )
	{
		if( IsWindow( GetSafeHwnd() ) )
		{
			PostMessage( UPLOADS_UPDATE_EVENT, 0, 0 );
		}
	}
}

//*******************************************************************
//  FUNCTION:   -	OnUploadsUpdate()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	Fired through windows message by Window Timer
//*******************************************************************
LRESULT CMuteUploadsDlg::OnUploadsUpdate(WPARAM wParam, LPARAM lParam)
{
    int					*uploadIDs;
    char				**hostAddresses;
    char				**filePaths;
    int					*chunksInFile;
    int					*lastChunksSent;
    unsigned long		*firstChunkTimes;
    unsigned long		*lastChunkTimes;
	CMuteUploadListItem	*pUploadListItem = NULL;
	CString				str,str2;
	
	if( !m_UploadList.LockList(100000) )
	{
		return (LRESULT) FALSE;
	}	

	if( m_nDontUpdateCtr > 0 )
	{
		m_nDontUpdateCtr--;		
		m_UploadList.UnlockList();
		return (LRESULT) FALSE;
	}

	muteStartUploadStatusUpdates();

    // The displayed list starts empty, and new items are added as required
    // when new uploads are started.  It can't be sorted or cleared (yet).
    // Sorting could be supported later by storing the
    // vector index for each item in that item's data field, and changing
    // the update loop below to work through the list items sequentially
    // instead of the vector items.  Clearing could be
    // supported by deleting both the vector contents and list items, and
    // letting both accumulate new entries from scratch.
    // It might be best to clear out only Done or Failed uploads, to prevent
    // silly ETA times from treating continued uploads as new.
	if( NULL == mUploadListIndexVector )
	{
		m_UploadList.UnlockList();
		return (LRESULT) FALSE;
	}

    int numListItems = mUploadListIndexVector->size();
    int numUploads = muteShareGetUploadStatus( &uploadIDs,
		&hostAddresses,
		&filePaths,
		&chunksInFile,
		&lastChunksSent,
		&firstChunkTimes,
		&lastChunkTimes );
	
    // get current time in seconds using API-prescribed time(NULL) call
    unsigned long currentSeconds = time( NULL );
	
	// update the list item corresponding to each upload info vector entry
    for( int i=0; i<numUploads; i++ ) 
	{
		
        char alreadyInList = false;
        int existingListIndex = -1;
		
        for( int j=0; j<numListItems && !alreadyInList; j++ ) 
		{
            if( uploadIDs[i] == *( mUploadListIndexVector->getElement( j ) ) ) {
                alreadyInList = true;
                existingListIndex = j;
			}	
		}
		
        if( ! alreadyInList ) 
		{
			CString	str;

            // append to the end of the list
            existingListIndex = numListItems;
			
            mUploadListIndexVector->push_back( uploadIDs[i] );
			pUploadListItem = new CMuteUploadListItem;
			if( NULL != pUploadListItem )
			{
				//mUploadList->InsertItem( existingListIndex, hostAddresses[i] );
				pUploadListItem->m_nMUTEUploadIndex = uploadIDs[i];
				pUploadListItem->m_strHostVirtualAddress = hostAddresses[i];
				pUploadListItem->m_strFileName = filePaths[i];
									
				pUploadListItem->m_unChunksUploaded =  0;
				pUploadListItem->m_unItemState = E_UL_ITEM_STARTINGUP;
				pUploadListItem->m_strStatus = m_ExtStr.LoadString( IDS_ULOAD_LIST_STARTINGUP_ENG + g_unStringLanguageIdOffset );
				m_vecUploadItems->push_back( pUploadListItem );
				m_UploadList.AddUploadItem( *( m_vecUploadItems->getElement( m_vecUploadItems->size() - 1 ) ) );
				numListItems++;
			}			            
        }
		
          
        const char *oldCountString = NULL;
		unsigned __int64	unOldChunksUploaded;
		
		pUploadListItem = *( m_vecUploadItems->getElement(existingListIndex) ) ;
        if( alreadyInList ) 
		{
            // grab last status and count strings so we can check for changes
            // before updating UI
			
			unOldChunksUploaded = pUploadListItem->m_unChunksUploaded;
		}
		
       
        // determine the upload's current status
        unsigned long elapsedSeconds = currentSeconds - lastChunkTimes[i];
        int numChunksSent = lastChunksSent[i] + 1;
		
        if( numChunksSent == chunksInFile[i] ) 
		{
            // all chunks uploaded: done
			pUploadListItem->m_unItemState = E_UL_ITEM_COMPLETE;
            pUploadListItem->m_strStatus = m_ExtStr.LoadString( IDS_ULOAD_STATUS_DONE_ENG + g_unStringLanguageIdOffset );
		}
        else if( elapsedSeconds >= 15 * 60 ) 
		{
            // no progress for 15 minutes: failed            
			pUploadListItem->m_unItemState = E_UL_ITEM_FAILED;
			pUploadListItem->m_strStatus = m_ExtStr.LoadString( IDS_ULOAD_STATUS_FAILED_ENG + g_unStringLanguageIdOffset );
		}
        else if( elapsedSeconds >= 60 ) 
		{
            // no progress for 1 minute: stalled            
			pUploadListItem->m_unItemState = E_UL_ITEM_STALLED;
			pUploadListItem->m_strStatus = m_ExtStr.LoadString( IDS_ULOAD_STATUS_STALLED_ENG + g_unStringLanguageIdOffset );
		}
        else 
		{
            // actively sending: status shows time remaining
            
            if( numChunksSent == 1 ) 
			{
                // don't try to compute time remaining, since we don't
                // have enough information from only one chunk
				pUploadListItem->m_unItemState = E_UL_ITEM_STARTINGUP;
				pUploadListItem->m_strStatus = m_ExtStr.LoadString( IDS_ULOAD_LIST_STARTINGUP_ENG + g_unStringLanguageIdOffset );
			}
            else 
			{
				CString	strTemp;
                double secondsPerChunk =
                    (double)( currentSeconds - firstChunkTimes[i] ) /
                    (double)( numChunksSent - 1 );
                
				
                double timeRemainingSeconds =
                    secondsPerChunk * ( chunksInFile[i] - numChunksSent );
                
                char *timeRemainingString =
                    formatTimeIntervalWithUnits( timeRemainingSeconds );									
                
				strTemp = m_ExtStr.LoadString( IDS_ULOAD_TIMEREMAINING_ENG + g_unStringLanguageIdOffset );
				pUploadListItem->m_unItemState = E_UL_ITEM_ACTIVE;
                pUploadListItem->m_strStatus.Format( strTemp, timeRemainingString );				
                delete [] timeRemainingString;
			}
		}

		// update the info columns only if they have changed; this minimizes
        // that annoying screen flashing and probably saves a good bit of
        // CPU once you've got a couple hundred uploads posted
		
        // format a new count string
		if( (int) pUploadListItem->m_unChunksUploaded < numChunksSent )
		{
			int percentDone = numChunksSent * 100 / chunksInFile[i];
			char *countString = autoSprintf( "%d%%  %d/%d",
				percentDone,
				numChunksSent,
				chunksInFile[i] );
			
			
			pUploadListItem->m_strChunkUpload = countString;
			pUploadListItem->m_unChunksUploaded = numChunksSent;
			
			delete [] countString;
		}        		
		
		m_UploadList.UpdateInfo( pUploadListItem );			
				
        delete [] hostAddresses[i];
        delete [] filePaths[i];
	}
	
	delete [] uploadIDs;
	delete [] hostAddresses;
	delete [] filePaths;
	delete [] chunksInFile;
	delete [] lastChunksSent;
	delete [] firstChunkTimes;
	delete [] lastChunkTimes;		

	str = m_ExtStr.LoadString( IDS_UPLOADS_UPLOADS_SHOWING_LABEL_ENG + g_unStringLanguageIdOffset );
	str2.Format( "%d", m_UploadList.GetItemCount() );
	str += str2;
	m_oUploadsCntLabel.SetText( str );


	m_UploadList.UnlockList();
	m_UploadList.Invalidate();

	if( m_vecUploadItems->size() > 100 )
	{
		// clean up automatically after a while so we don't 
		// crash the program with the huge list.. 
		PostMessage(UPLOAD_REMOVE_ALL_EVENT,0,0); 
	}

	str = m_ExtStr.LoadString( IDS_UPLOADS_TOTAL_CHUNKS_UPLOADED_LABEL_ENG + g_unStringLanguageIdOffset );
	char *szTemp = formatDataSizeWithUnits( g_nBytesSent );
	str2.Format( "%d -- %s", g_ulChunksSent, szTemp );
	delete [] szTemp;
	str += str2;
	m_oChunksCntLabel.SetText( str );

	SetStrings();

	return (LRESULT) TRUE;
}



//*******************************************************************
//  FUNCTION:   -	OnUploadCancel()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
LRESULT CMuteUploadsDlg::OnUploadCancel( WPARAM wParam, LPARAM lParam )
{
	CMuteUploadListItem						*pListItem;
	int										item;		
	int										nUploadID;
	int										numListItem;
	CString									str,str2;	

	if( !m_UploadList.LockList(100000) )
	{
		return (LRESULT) FALSE;
	}
	
	if( NULL == mUploadListIndexVector )
	{	
		m_UploadList.UnlockList();
		return (LRESULT) FALSE;
	}

	// skip 2 update cycles after a deletion of items.. This is arbitrary...
	m_nDontUpdateCtr = 2; 

	muteUploadStatusLock();
	muteStopUploadStatusUpdates(); // sets a flag to prevent updating the lists
								  // reset later in update cycle..
	Sleep(500); // allow time for underlying thread to block on semaphore

	item = m_UploadList.GetItemCount() - 1;
	while( item >= 0 )
	{
		if( m_UploadList.GetItemState( item, LVIS_SELECTED ) )
		{	
			pListItem = (CMuteUploadListItem *) m_UploadList.GetItemData(item);
			
			numListItem = mUploadListIndexVector->size() - 1;
			while( numListItem > - 1 )
			{
				nUploadID = (int) *(mUploadListIndexVector->getElement( numListItem ));				
				if( nUploadID == pListItem->m_nMUTEUploadIndex )
				{
					
					muteRemoveUploadStatus( nUploadID );						
					mUploadListIndexVector->deleteElement( numListItem );					
					m_UploadList.DeleteItem(item);
					// get rid of our stored item in the vector...
					delete pListItem;
					m_vecUploadItems->deleteElement( numListItem );
					break;
				}				
				numListItem--;
			}
		}
		item--;
	}
		
	m_UploadList.Invalidate();
	muteUploadStatusUnlock();
	m_UploadList.UnlockList();

	str = m_ExtStr.LoadString( IDS_UPLOADS_UPLOADS_SHOWING_LABEL_ENG + g_unStringLanguageIdOffset );
	str2.Format( "%d", m_UploadList.GetItemCount() );
	str += str2;
	m_oUploadsCntLabel.SetText( str );
	
	SetStrings();
	return (LRESULT) true;    	
}
//*******************************************************************
//  FUNCTION:   -	OnUploadRemoveAll()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
LRESULT CMuteUploadsDlg::OnUploadRemoveAll( WPARAM wParam, LPARAM lParam )
{
	CMuteUploadListItem *item;
	CString				str,str2;
    
	if( NULL == mUploadListIndexVector )
	{
		return (LRESULT) FALSE;
	}

	while( mUploadListIndexVector->size() > 0 )
	{
		muteRemoveUploadStatus( *(mUploadListIndexVector->getElement( 0 )) );
		mUploadListIndexVector->deleteElement( 0 );
		item = *( m_vecUploadItems->getElement( 0 ) );
		delete item;
		m_vecUploadItems->deleteElement(0);
	}
	
    m_UploadList.DeleteAllItems();	
	
	str = m_ExtStr.LoadString( IDS_UPLOADS_UPLOADS_SHOWING_LABEL_ENG + g_unStringLanguageIdOffset );
	str2.Format( "%d", m_UploadList.GetItemCount() );
	str += str2;
	m_oUploadsCntLabel.SetText( str );
	
	SetStrings();
	return (LRESULT) FALSE;
}

//*******************************************************************
//  FUNCTION:   -	OnRemoveAllUploadsButton()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteUploadsDlg::OnRemoveAllUploadsButton() 
{
	PostMessage( UPLOAD_REMOVE_ALL_EVENT, 0, 0 );	
}

//*******************************************************************
//  FUNCTION:   -	OnCancelButton()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteUploadsDlg::OnCancelButton()
{	
	OnUploadCancel(NULL,NULL);
}


//*******************************************************************
//  FUNCTION:   -	SetStrings()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteUploadsDlg::SetStrings()
{
	CString str;
	CString	str2;

	m_UploadList.SetStrings();
    
    str.Format( "%s (%d)",
                    m_ExtStr.LoadString( IDS_TAB_TITLE_ULOAD_TEXT_ENG + g_unStringLanguageIdOffset),
				    m_UploadList.GetItemCount()  );
	GetDlgItem( IDC_UPLOADS_STATIC )->SetWindowText( str );
    

	str = m_ExtStr.LoadString( IDS_ULOAD_BTN_REMOVALL_ENG + g_unStringLanguageIdOffset );
	m_btnRemoveAllUpload.SetWindowText( str );

	str = m_ExtStr.LoadString( IDS_ULOAD_BTN_CANCELCLEAR_ENG + g_unStringLanguageIdOffset );
	m_btnClearSelectedUpload.SetWindowText( str );	

	str = m_ExtStr.LoadString( IDS_UPLOADS_UPLOADS_SHOWING_LABEL_ENG + g_unStringLanguageIdOffset );
	str2.Format( "%d", m_UploadList.GetItemCount() );
	str += str2;
	m_oUploadsCntLabel.SetText( str );

	str = m_ExtStr.LoadString( IDS_UPLOADS_TOTAL_CHUNKS_UPLOADED_LABEL_ENG + g_unStringLanguageIdOffset );
	char *szTemp = formatDataSizeWithUnits( g_nBytesSent );
	str2.Format( "%d -- %s", g_ulChunksSent, szTemp );
	delete [] szTemp;
	str += str2;
	m_oChunksCntLabel.SetText( str );		
}

/*
//*******************************************************************
//  FUNCTION:    -	OnShowWindow()
//  RETURNS:     -	
//  PARAMETERS:  -	
//  COMMENTS:	 -	
//*******************************************************************
*/
void CMuteUploadsDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CResizableDialog::OnShowWindow(bShow, nStatus);		
	SetStrings();
}
