// MuteDownloadsDlg.cpp : implementation file
//

/* 02-14-2005 JROC -->

    -- Fixed code that adds downloads from the Search window that were already listed in the
    download queue.  After further testing of version 0.0.3, I found that this code was broken
    and duplicate items would show up in the download list.
    -- Increased semaphore timeout values in Downloads Dialog code.
    -- On the Downloads screen, when files are selected, the DELETE key performs the cancel/clear 
    function.
*/
#include "stdafx.h"
#include "MFC_MUTE_GUI.h"
#include "MUTE/otherApps/fileSharing/fileShare.h"
#include "MuteDownloadsDlg.h"
#include "WinFileSysUtils.h"
#include "ExternString.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CMuteDownloadsDlg dialog
CMuteDownloadsDlg::CMuteDownloadsDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CMuteDownloadsDlg::IDD, pParent)
{
	mDownloadItems = new SimpleVector<CMuteDownloadListItem *>();
	
	m_pDLQueue = NULL;	

	//{{AFX_DATA_INIT(CMuteDownloadsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CMuteDownloadsDlg::~CMuteDownloadsDlg()
{	
}


void CMuteDownloadsDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMuteDownloadsDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_DOWNLOAD_LIST, m_DownloadList);
	DDX_Control(pDX, IDC_EXPLORE_SHARED_FOLDER, m_btnExploreSharedFiles);
	DDX_Control(pDX, IDC_EXPLORE_INCOMING_FOLDER, m_btnExploreIncomingFiles);	
	DDX_Control(pDX, IDC_PURGE_INCOMING_FOLDER, m_btnPurgeIncomingFolder);
	DDX_Control(pDX, IDC_CLEAR_COMPLETED_BUTTON, m_btnClearCompletedButton);
	DDX_Control(pDX, IDC_CLEAR_FAILED_BUTTON, m_btnClearFailedButton);	
	DDX_Control(pDX, IDC_CANCEL_BUTTON, m_btnCancelButton); // need this
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMuteDownloadsDlg, CResizableDialog)
	//{{AFX_MSG_MAP(CMuteDownloadsDlg)
	ON_MESSAGE( DOWNLOAD_DIALOG_PREPARE_FOR_CLOSE, OnPrepareForClose )
	ON_MESSAGE( DOWNLOAD_CLEAR_COMPLETE_EVENT, OnClearComplete )
	ON_MESSAGE( DOWNLOAD_CLEAR_STALLED_EVENT, OnClearStalled )	
	ON_MESSAGE( DOWNLOAD_CANCEL_EVENT, OnDownloadCancel )
	ON_MESSAGE( DOWNLOAD_RESULT_SET_TO_QUEUED, OnSetDownloadToQueued )
	ON_MESSAGE( DOWNLOAD_QUEUED_ITEM_IS_QUEUED, OnSetItemStatusTextToQueued )
	ON_MESSAGE( DOWNLOAD_QUEUED_ITEM_IS_SEARCHING, OnSetItemStatusTextToSearching )
	ON_MESSAGE( DOWNLOAD_RESULT_REMOVE_FROM_QUEUE, OnRemoveDownloadFromQueue )	
	ON_MESSAGE( DOWNLOAD_SEARCH_QUEUE_ITEM_NEXT_EVENT, OnSearchQueueItemNext )
	ON_BN_CLICKED(IDC_CANCEL_BUTTON, OnCancelButton)
	ON_BN_CLICKED(IDC_CLEAR_FAILED_BUTTON, OnClearFailedButton)
	ON_BN_CLICKED(IDC_CLEAR_COMPLETED_BUTTON, OnClearCompletedButton)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_EXPLORE_SHARED_FOLDER, OnExploreSharedFolder)
	ON_BN_CLICKED(IDC_PURGE_INCOMING_FOLDER, OnPurgeIncomingFolder)
	ON_BN_CLICKED(IDC_EXPLORE_INCOMING_FOLDER, OnExploreIncomingFolder)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMuteDownloadsDlg message handlers

BOOL CMuteDownloadsDlg::PreTranslateMessage(MSG* pMsg) 
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if( (VK_ESCAPE == pMsg->wParam) || (VK_RETURN == pMsg->wParam) )
		{
			// Don't close the window if the user hits the ESC key.
			return FALSE;
		}

		if( VK_DELETE == pMsg->wParam )
		{
			PostMessage( DOWNLOAD_CANCEL_EVENT, NULL, NULL );
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
BOOL CMuteDownloadsDlg::OnInitDialog() 
{
	CString	strTemp;
	CResizableDialog::OnInitDialog();
	ShowSizeGrip(FALSE);
	
	m_DownloadList.SetParent((CWnd *) this );
    m_DownloadList.Init();
	
    AddAnchor( m_DownloadList, TOP_LEFT, BOTTOM_RIGHT );
	AddAnchor( m_btnClearCompletedButton, BOTTOM_CENTER );
	AddAnchor( m_btnClearFailedButton, BOTTOM_CENTER );
	AddAnchor( m_btnCancelButton, BOTTOM_CENTER );
	AddAnchor( m_btnExploreSharedFiles, BOTTOM_CENTER );
	AddAnchor( m_btnExploreIncomingFiles, BOTTOM_CENTER );
	AddAnchor( m_btnPurgeIncomingFolder, BOTTOM_CENTER );

    ((CStatic*)GetDlgItem(IDC_DOWNLOADS_ICO))->SetIcon((HICON)::LoadImage(AfxGetInstanceHandle(),
        MAKEINTRESOURCE(IDI_DOWNLOAD5_ICON), IMAGE_ICON, 16, 16, 0));

    
	m_pDLQueue = new CDownloadQueue( (void *) this );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//*******************************************************************
//  FUNCTION:   -	addDownload()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteDownloadsDlg::addDownload( const char *inFromAddress, const char *inLocalFilePath, const char *inRemoteFilePath,
		const char *inFileHash, bool bIsAResumedFile , unsigned int unNumResumes )
{
    CMuteDownloadListItem *addedItem = new CMuteDownloadListItem( ((CDialog * )&m_DownloadList), inFromAddress, inLocalFilePath, inRemoteFilePath,
                      inFileHash, bIsAResumedFile, unNumResumes );
	
	addedItem->m_strHostVirtualAddress = inFromAddress;

    mDownloadItems->push_back( addedItem );
	m_DownloadList.AddDownloadResult( *( mDownloadItems->getElement( mDownloadItems->size() - 1 ) ) );
	m_pDLQueue->SetItemDLoadListItemPtrFromHash( (void *) addedItem, inFileHash );

	SetStrings();
}

//*******************************************************************
//  FUNCTION:   -	addDownloadFromSearch()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteDownloadsDlg::addDownloadFromSearch( const char *inFromAddress, const char *inFilePath, const char *inFileHash ) 
{
	bool					bIsAlreadyDownloading = false;
	int						i, indexFound;
	int						nItemID, nListItemIDFound;
	CString					strInFilePath, strInFileHash;
	CMuteDownloadListItem	*item;
	CMuteDownloadListItem	*itemFound = NULL;
	// per Nate's request, experimenting with limiting concurrent downloads
	// from the same source VIP
	bool					bShouldWeQueue = false; // set to true if we don't want to start active
	unsigned int			unVIPCnt = 0;	

	// don't add a download that is already in the list and is active!
	if( !m_DownloadList.LockList(60000) )
	{
		return;
	}

	strInFilePath = inFilePath;
	i = strInFilePath.ReverseFind('/');
	if( i > -1 )
	{
		strInFilePath.Delete( 0, i+1 );
	}
    
    strInFileHash = inFileHash;

	unVIPCnt = 0;
	for( i = 0; i < mDownloadItems->size(); i++ )
	{
		item = *( mDownloadItems->getElement( i ) );

        // this piece of code is just to increase generate the count of active
        // downloads from the VIP we're trying to download from now
		if( item->m_strHostVirtualAddress == inFromAddress )
		{			
			switch( item->m_unItemState )
			{
			case E_ITEM_QUEUED_SEARCHING:
			case E_ITEM_ATTEMPTING_RESUME:
			case E_ITEM_RESUMING:
			case E_ITEM_FETCHING_INFO:					
			case E_ITEM_ACTIVE:			
				unVIPCnt++;					
			}
		}

        // need to check the local file name or the HASH code
		if( (strInFilePath == item->m_strLocalFileName) || (strInFileHash == item->m_strHash) )
		{
			indexFound = i;
			bIsAlreadyDownloading = true;
			//break; no longer stop going through the list because we want
			// to control the #of active downloads from the same VIP
			nItemID = m_DownloadList.GetItemCount() - 1;
			while( nItemID >= 0 )
			{
				itemFound = (CMuteDownloadListItem *) m_DownloadList.GetItemData(nItemID);
				if( itemFound == item  )
				{
					nListItemIDFound = nItemID;
					break;
				}
                nItemID--;
			}
		}
	}

	m_DownloadList.UnlockList();

	if( unVIPCnt >= 4 )
	{
		bShouldWeQueue = true;
	}	

	if( bIsAlreadyDownloading )
	{
		// 01-23-2005 -- if already is downloading, but item is queued, restart the download!
		// just remove from the Queue, then continue on in this code which will re-add it!		
		switch( itemFound->m_unItemState )
		{
		case E_ITEM_QUEUED_SEARCHING:
		case E_ITEM_ATTEMPTING_RESUME:
		case E_ITEM_RESUMING:
		case E_ITEM_FETCHING_INFO:		
		case E_ITEM_CANCELLING:		
		case E_ITEM_ACTIVE:
		case E_ITEM_CORRUPT:		
		case E_ITEM_COMPLETE:
			// if the item is not queued don't do anything
			return;
			break;
		default:
			/*
			the following cases will allow us to re-add
			the item from the search dialog:

			case E_ITEM_NOT_FOUND:
			case E_ITEM_FAILED_TO_WRITE_LOCAL_FILE:
			case E_ITEM_QUEUED:
			case E_ITEM_FAILED:
			case E_ITEM_CANCELED:
			*/
			// if the item was queued, remove it, then we'll
			// add it back below.
			// only remove and re-add as active if not downloading too many from same VIP
			if( !bShouldWeQueue )
			{
				m_pDLQueue->RemoveQueueItem( strInFilePath );
				// need to delete the item from the list.. 
				m_DownloadList.DeleteItem(nListItemIDFound);
				// explicitly delete because deleteElement doesn't delete object that is pushed back onto vector
				delete itemFound;
				mDownloadItems->deleteElement( indexFound );
			}
			else
			{
				// it's already queued, so just do nothing for now
				return;
			}
		}
	}

	if( !bShouldWeQueue )
	{
		// add to DownloadQueue as an active download.. 
		// when coming from Search window.. assume local file name and remote file name are the same.. 
		m_pDLQueue->AddQueueItem( false /*false because active */, (LPCSTR) strInFilePath, (LPCSTR) strInFilePath, inFilePath, inFileHash );
		addDownload( inFromAddress, inFilePath, inFilePath, inFileHash, false , 0 );
	}
	else
	{
		item = (CMuteDownloadListItem * )addDownloadFromQueue( (LPCSTR) strInFilePath, inFileHash );
		if( NULL != item )
		{
			m_pDLQueue->AddQueueItem( true /*true because queued */, (LPCSTR) strInFilePath, (LPCSTR) strInFilePath, inFilePath, inFileHash );
			m_pDLQueue->SetItemDLoadListItemPtrFromLocalFilename( item, strInFilePath );
		}
	}	

	SetStrings();
}

//*******************************************************************
//  FUNCTION:   -	addDownloadFromQueue()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void * CMuteDownloadsDlg::addDownloadFromQueue( const char *inLocalFilePath, const char *inFileHash )
{
	CMuteDownloadListItem  *addedItem;	
	bool					bIsAlreadyDownloading = false;
	int						i;
	CString					strInFilePath;
	CMuteDownloadListItem	*item;

	strInFilePath = inLocalFilePath;
	i = strInFilePath.ReverseFind('/');
	if( i > -1 )
	{
		strInFilePath.Delete( 0, i+1 );
	}

	// don't add a download that is already in the list and is active!
	if( !m_DownloadList.LockList(60000) )
	{
		return (void *) NULL;
	}

	for( i = 0; i < mDownloadItems->size(); i++ )
	{
		item = *( mDownloadItems->getElement( i ) );

		if( strInFilePath == item->m_strLocalFileName )
		{
			bIsAlreadyDownloading = true;
			break;
		}
	}			

	if( bIsAlreadyDownloading )
	{
		m_DownloadList.UnlockList();
		return (void *) NULL;		
	}
	
	addedItem = new CMuteDownloadListItem( ((CDialog * )&m_DownloadList), inLocalFilePath, inFileHash );
    mDownloadItems->push_back( addedItem );
	m_DownloadList.AddDownloadResult( *( mDownloadItems->getElement( mDownloadItems->size() - 1 ) ) );
	m_DownloadList.UnlockList();

	SetStrings();

	return( (void *) addedItem );
}

//*******************************************************************
//  FUNCTION:   -	ResumeFromQueue()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	called from the MuteDownloadQueue class when a download
//					is resumed.. 
//*******************************************************************
bool CMuteDownloadsDlg::ResumeFromQueue( CString& strLocalFilename, CString& strRemoteFilename, CString& strHash, CString& strVIP )
{
	CMuteDownloadListItem	*item;
	int						i;
	unsigned int			unVIPCnt;

	// don't add a download that is already in the list and is active!
	if( !m_DownloadList.LockList(60000) )
	{
		return false;
	}

	unVIPCnt = 0;
	for( i = 0; i < mDownloadItems->size(); i++ )
	{
		item = *( mDownloadItems->getElement( i ) );

		if( item->m_strHostVirtualAddress == strVIP )
		{			
			switch( item->m_unItemState )
			{
			case E_ITEM_QUEUED_SEARCHING:
			case E_ITEM_ATTEMPTING_RESUME:
			case E_ITEM_RESUMING:
			case E_ITEM_FETCHING_INFO:			
			case E_ITEM_ACTIVE:
				unVIPCnt++;					
			}
		}		
	}	

	// not allowing more than 4 concurrent downloads from same VIP
	if( unVIPCnt >= 4 )
	{
		m_DownloadList.UnlockList();
		return false;		
	}

	item = (CMuteDownloadListItem * )GetPtrToListItemFromHash( (LPCSTR) strHash );

	if( NULL == item )
	{
		item = (CMuteDownloadListItem * )GetPtrToListItemFromLocalFileName( (LPCSTR) strLocalFilename );
		if( NULL != item )
		{
			item->m_strHash = strHash; // have to update hash, because we started with local file name only
		}
	}
	if( NULL == item )
	{
		m_DownloadList.UnlockList();
		return false;
	}

	if( CMuteDownloadListCtrl::m_bAppIsClosing || item->isCanceled() )
	{
		m_DownloadList.UnlockList();
		return false;		
	}	

	item->RestartDownload( strVIP, strRemoteFilename );
	m_DownloadList.UpdateInfo( item );
	//m_DownloadList.Invalidate();//08-14-2005
	m_DownloadList.UnlockList();
	
	return true;
}


//*******************************************************************
//  FUNCTION:   -	OnClearStalled()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
LRESULT CMuteDownloadsDlg::OnClearStalled( WPARAM wParam, LPARAM lParam )
{	
	CMuteDownloadListItem	*pListItem;
    int						i;
	int						nItemID;
	CString					strFailed, strNotFound, strCorrupted;
	
	if( !m_DownloadList.LockList(10000) )
	{
		return (LRESULT) FALSE;
	}
	
	nItemID = m_DownloadList.GetItemCount() - 1;
	
	strFailed = m_ExtStr.LoadString( IDS_DLOAD_FAILED_ENG + g_unStringLanguageIdOffset );
	strNotFound = m_ExtStr.LoadString( IDS_DLOAD_NOT_FOUND_ENG + g_unStringLanguageIdOffset );
	strCorrupted = m_ExtStr.LoadString( IDS_DLOAD_FILE_CORRUPTED_ENG + g_unStringLanguageIdOffset );

	while( nItemID >= 0 )
	{
		pListItem = (CMuteDownloadListItem *) m_DownloadList.GetItemData(nItemID);
			
		if( (E_ITEM_FAILED == pListItem->m_unItemState) ||
			(E_ITEM_CORRUPT == pListItem->m_unItemState) ||
			(E_ITEM_NOT_FOUND == pListItem->m_unItemState)
		  )		
		{
	
			pListItem->OnCancelClear();
		
			i = mDownloadItems->size() - 1;
			while( i >= 0 )			
			{		
				CMuteDownloadListItem *item = *( mDownloadItems->getElement( i ) );
				
				if( pListItem == item )
				{
					Sleep(5);
					if( item->isCleared() ) 
					{
						// destroy it
						
						// this will remove it from its sizer and from this panel,
						// though if it is cleared, it isn't visible anymore anyway
						//                item->Destroy();
						m_DownloadList.DeleteItem(nItemID);
						// explicitly delete because deleteElement doesn't delete object that is pushed back onto vector
						delete pListItem;
						mDownloadItems->deleteElement( i );
						break; // next in list... 
					}
					else
					{
						pListItem->m_unItemState = E_ITEM_CANCELLING;
						pListItem->m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_CANCELLING_ENG + g_unStringLanguageIdOffset );
						m_DownloadList.SetItemText(nItemID,1, pListItem->m_strStatusText );
					}
				}
				i--;
			}
			
		}
		
		nItemID--;
	}	
	
	m_DownloadList.UnlockList();
	m_DownloadList.Invalidate();

	SetStrings();
	return (LRESULT) FALSE;
}


//*******************************************************************
//  FUNCTION:   -	OnClearComplete()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
LRESULT CMuteDownloadsDlg::OnClearComplete( WPARAM wParam, LPARAM lParam )
{		
	CMuteDownloadListItem	*pListItem;
	int						nItemID;
	int						i;
	CString					strOneHundredPercentOf;

	if( !m_DownloadList.LockList(10000) )
	{
		return (LRESULT) FALSE;
	}
	
	nItemID = m_DownloadList.GetItemCount() - 1;	
	strOneHundredPercentOf = m_ExtStr.LoadString( IDS_DLOAD_ONE_HUNDRED_PERCENT_OF_ENG + g_unStringLanguageIdOffset );		
	while( nItemID >= 0 )
	{	
		pListItem = (CMuteDownloadListItem *) m_DownloadList.GetItemData(nItemID);

		if( pListItem->m_strStatusText.Find(strOneHundredPercentOf) > -1 )
		{
			pListItem->m_unItemState = E_ITEM_COMPLETE;
		}
		
		if( E_ITEM_COMPLETE == pListItem->m_unItemState )		
		{
			pListItem->OnCancelClear();
			
			i = mDownloadItems->size() - 1;
			while( i >= 0 )
			{
			
				CMuteDownloadListItem *item = *( mDownloadItems->getElement( i ) );
				
				if( pListItem == item )
				{
					Sleep(5);
					if( item->isCleared() ) 
					{
						// destroy it
						
						// this will remove it from its sizer and from this panel,
						// though if it is cleared, it isn't visible anymore anyway
						//                item->Destroy();
						m_DownloadList.DeleteItem(nItemID);
						// explicitly delete because deleteElement doesn't delete object that is pushed back onto vector
						delete pListItem;
						mDownloadItems->deleteElement( i );
						break; // next in list... 											
					}
					else
					{
						pListItem->m_unItemState = E_ITEM_CANCELLING;
						pListItem->m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_CANCELLING_ENG + g_unStringLanguageIdOffset );
						m_DownloadList.SetItemText(nItemID,1, pListItem->m_strStatusText );
					}
				}
				i--;
			}			
		}
		
		nItemID--;
	}	
	
	m_DownloadList.UnlockList();
	m_DownloadList.Invalidate();

	SetStrings();

	return (LRESULT) FALSE;
}

//*******************************************************************
//  FUNCTION:   -	OnSetDownloadToQueued()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
LRESULT CMuteDownloadsDlg::OnSetDownloadToQueued( WPARAM wParam, LPARAM lParam )
{
	CMuteDownloadListItem	*pListItem;
	CMuteDownloadListItem	*pListItem2;
	int						nItemID;

	pListItem = (CMuteDownloadListItem *) lParam;
	
	if( !m_DownloadList.LockList(60000) )
	{
		return (LRESULT) FALSE;
	}		
	
	nItemID = m_DownloadList.GetItemCount() - 1;
	
	// iterate through list to make sure we didn't cancel the download while 
	// a thread was returning results (aka.. pListItem is no longer valid )
	while( nItemID >= 0 )
	{	
		pListItem2 = (CMuteDownloadListItem *) m_DownloadList.GetItemData(nItemID);	

		if( pListItem2 == pListItem )
		{
			if( NULL != m_pDLQueue )
			{
				m_pDLQueue->SetItemToQueued( true, (LPCSTR) pListItem->m_strHash );
			}

			break;
		}

		nItemID--;
	}
	
	m_DownloadList.UnlockList();
	//m_DownloadList.Invalidate();//08-14-2005
	return (LRESULT) FALSE;
}

//*******************************************************************
//  FUNCTION:   -	OnSetItemStatusTextToQueued()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
LRESULT CMuteDownloadsDlg::OnSetItemStatusTextToQueued( WPARAM wParam, LPARAM lParam )
{
	CMuteDownloadListItem	*pListItem = (CMuteDownloadListItem	*) lParam;
	CMuteDownloadListItem	*pListItem2;
	int						nItemID;
	
	if( !m_DownloadList.LockList(60000) )
	{
		return (LRESULT) FALSE;
	}
			
	nItemID = m_DownloadList.GetItemCount() - 1;
	
	// iterate through list to make sure we didn't cancel the download while 
	// a thread was returning results (aka.. pListItem is no longer valid )
	while( nItemID >= 0 )
	{	
		pListItem2 = (CMuteDownloadListItem *) m_DownloadList.GetItemData(nItemID);	

		if( pListItem2 == pListItem )
		{				
			pListItem->m_unItemState = E_ITEM_QUEUED;
			pListItem->m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_QUEUED_ENG + g_unStringLanguageIdOffset );			
			m_DownloadList.UpdateInfo( pListItem );
			break;
		}
		
		nItemID--;
	}
		
	m_DownloadList.UnlockList();	

	return (LRESULT) FALSE;
}

//*******************************************************************
//  FUNCTION:   -	OnSetItemStatusTextToSearching()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
LRESULT CMuteDownloadsDlg::OnSetItemStatusTextToSearching( WPARAM wParam, LPARAM lParam )
{
	CMuteDownloadListItem	*pListItem = (CMuteDownloadListItem	*) lParam;
	CMuteDownloadListItem	*pListItem2;
	int						nItemID;	
	
	if( !m_DownloadList.LockList(60000) )
	{
		return (LRESULT) FALSE;
	}

	nItemID = m_DownloadList.GetItemCount() - 1;
	
	// iterate through list to make sure we didn't cancel the download while 
	// a thread was returning results (aka.. pListItem is no longer valid )
	while( nItemID >= 0 )
	{		
		pListItem2 = (CMuteDownloadListItem *) m_DownloadList.GetItemData(nItemID);	
		if( pListItem2 == pListItem )
		{				
			pListItem->m_unItemState = E_ITEM_QUEUED_SEARCHING;
			pListItem->m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_QUEUED_SEARCHING_ENG + g_unStringLanguageIdOffset );			
			m_DownloadList.UpdateInfo( pListItem );
			break;
		}

		nItemID--;
	}
	
	m_DownloadList.UnlockList();	
	SetStrings();
	return (LRESULT) FALSE;
}

//*******************************************************************
//  FUNCTION:   -	OnRemoveDownloadFromQueue()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	this will occur after a hard failure/file complete situation
//*******************************************************************
LRESULT CMuteDownloadsDlg::OnRemoveDownloadFromQueue( WPARAM wParam, LPARAM lParam )
{
	CMuteDownloadListItem	*pListItem;
	
	if( !m_DownloadList.LockList(60000) )
	{
		return (LRESULT) FALSE;
	}
	
	pListItem = (CMuteDownloadListItem *) lParam;
	
	if( NULL != m_pDLQueue )
	{
		m_pDLQueue->RemoveQueueItem( (LPCSTR) pListItem->m_strLocalFileName );
	}
	
	m_DownloadList.UnlockList();
	return (LRESULT) FALSE;
}


//*******************************************************************
//  FUNCTION:   -	OnClearResumedDownload()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************

LRESULT CMuteDownloadsDlg::OnClearResumedDownload( WPARAM wParam, LPARAM lParam )
{	
	CMuteDownloadListItem	*pListItem;
	int						nItemID;
	int						i;
	bool					bFound = false;
	
	if( !m_DownloadList.LockList(60000) )
	{
		return (LRESULT) FALSE;
	}
	
	pListItem = (CMuteDownloadListItem *) lParam;

	nItemID = m_DownloadList.GetItemCount() - 1;
	while( (nItemID >= 0) && !bFound )
	{
		if( (CMuteDownloadListItem *) m_DownloadList.GetItemData(nItemID) == pListItem  )
		{			
			pListItem->OnCancelClear();
			
			i = mDownloadItems->size() - 1;
			while( (i >= 0) && !bFound )
			{		
				CMuteDownloadListItem *item = *( mDownloadItems->getElement( i ) );
				
				if( pListItem == item )
				{
					Sleep(5);
					if( item->isCleared() ) 
					{
						// destroy it
						
						// this will remove it from its sizer and from this panel,
						// though if it is cleared, it isn't visible anymore anyway
						//                item->Destroy();
						m_DownloadList.DeleteItem(nItemID);
						// explicitly delete because deleteElement doesn't delete object that is pushed back onto vector
						delete pListItem;
						mDownloadItems->deleteElement( i );
						bFound = true;
						break; // next in list... 											
					}
					else
					{						
						pListItem->m_unItemState = E_ITEM_ATTEMPTING_RESUME;
						pListItem->m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_ATTEMPTING_RESUME_ENG + g_unStringLanguageIdOffset );
						m_DownloadList.SetItemText(nItemID,1, pListItem->m_strStatusText );
					}
				}
				i--;
			}			
		}
		
		nItemID--;
	}	
	
	m_DownloadList.UnlockList();
	m_DownloadList.Invalidate();

	SetStrings();

	return (LRESULT) FALSE;
}


//*******************************************************************
//  FUNCTION:   -	OnSearchQueueItemNext()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	Moves an item to be the next item that will be
//					searched within the download queue
//*******************************************************************
LRESULT CMuteDownloadsDlg::OnSearchQueueItemNext( WPARAM wParam, LPARAM lParam )
{
	CMuteDownloadListItem	*pListItem;
	int						nItemID = -1;	
	
	if( !m_DownloadList.LockList(10000) )
	{
		return (LRESULT) FALSE;
	}
		
	if( 1 == m_DownloadList.GetSelectedCount() )
	{
		nItemID = m_DownloadList.GetNextItem(nItemID, LVNI_SELECTED);

		if( -1 != nItemID  )
		{
			pListItem = (CMuteDownloadListItem *) m_DownloadList.GetItemData(nItemID);

			if( NULL != m_pDLQueue )
			{
				m_pDLQueue->SearchQueueItemNext( (LPCSTR) pListItem->m_strLocalFileName );
			}					
		}
	}	
	
	m_DownloadList.UnlockList();
	//m_DownloadList.Invalidate(); //08-14-2005
	return (LRESULT) FALSE;
}


//*******************************************************************
//  FUNCTION:   -	OnDownloadCancel()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
LRESULT CMuteDownloadsDlg::OnDownloadCancel( WPARAM wParam, LPARAM lParam )
{	
	CMuteDownloadListItem	*pListItem;
	int						nItemID;
	int						i;
	
	if( !m_DownloadList.LockList(10000) )
	{
		return (LRESULT) FALSE;
	}
	
	nItemID = m_DownloadList.GetItemCount() - 1;
	while( nItemID >= 0 )
	{
		if( m_DownloadList.GetItemState( nItemID, LVIS_SELECTED ) )
		{
			pListItem = (CMuteDownloadListItem *) m_DownloadList.GetItemData(nItemID);

			pListItem->OnCancelClear();
			
			i = mDownloadItems->size() - 1;
			while( i >= 0 )
			{		
				CMuteDownloadListItem *item = *( mDownloadItems->getElement( i ) );
				
				if( pListItem == item )
				{
					Sleep(5);
					if( item->isCleared() ) 
					{
						// destroy it
						
						if( NULL != m_pDLQueue )
						{
							m_pDLQueue->RemoveQueueItem( (LPCSTR)pListItem->m_strLocalFileName );
						}
						// this will remove it from its sizer and from this panel,
						// though if it is cleared, it isn't visible anymore anyway
						//                item->Destroy();
						m_DownloadList.DeleteItem(nItemID);
						// explicitly delete because deleteElement doesn't delete object that is pushed back onto vector
						delete pListItem;
						mDownloadItems->deleteElement( i );
						break; // next in list... 											
					}
					else
					{	
						pListItem->m_unItemState = E_ITEM_CANCELLING;
						pListItem->m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_CANCELLING_ENG + g_unStringLanguageIdOffset );
						m_DownloadList.SetItemText(nItemID,1, pListItem->m_strStatusText );
					}
				}
				i--;
			}
			
		}
		
		nItemID--;
	}	
	
	m_DownloadList.UnlockList();
	//m_DownloadList.Invalidate();//08-14-2005
	SetStrings();
	return (LRESULT) FALSE;
}


//*******************************************************************
//  FUNCTION:   -	areDownloadsActive()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
char CMuteDownloadsDlg::areDownloadsActive() {

    try
	{
		int numItems = mDownloadItems->size();
    
		for( int i=0; i<numItems; i++ ) {
        
			CMuteDownloadListItem  *item = *( mDownloadItems->getElement( i ) );

			if(	item->isActive() ) {
	            return true;
		        }
	        }
	}
	catch(...)
	{
	}

    // no actives found
    return false;
    }


//*******************************************************************
//  FUNCTION:   -	OnCancelButton()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteDownloadsDlg::OnCancelButton() 
{	
	PostMessage( DOWNLOAD_CANCEL_EVENT, NULL, NULL );
}


//*******************************************************************
//  FUNCTION:   -	OnClose()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteDownloadsDlg::OnClose() 
{		
	int				nItems;
	int				i;
	vector<int>		vec_nClearedRetries;
	CWaitCursor		wait;	

	delete		m_pDLQueue;
	m_pDLQueue = NULL;

	// no need in waiting if no downloads exist!
	if( mDownloadItems->size() == 0 )
	{
		delete mDownloadItems;
		return;
	}

	OnPrepareForClose( NULL, NULL );// cancels any current

	if( NULL != mDownloadItems )
	{
		vec_nClearedRetries.resize( mDownloadItems->size() );
		for( i = 0; i < (int) vec_nClearedRetries.size(); i++ )
		{
			vec_nClearedRetries[i] = 0;
		}

		while( mDownloadItems->size() > 0 )
		{
			nItems = mDownloadItems->size();
			for( i = (nItems - 1); i >= 0; i-- )
			{
				CMuteDownloadListItem *item = *( mDownloadItems->getElement( i ) );
				
				item->OnCancelClear(); // clears any canceled
				m_DownloadList.PumpMessages();
				wait.Restore();
				Sleep(1);
				if( item->isCleared() || ( vec_nClearedRetries[i] > 40 ) )
				{
					delete item;
					mDownloadItems->deleteElement(i);					
				}
				else
				{
					vec_nClearedRetries[i]++;
				}
			}			
		}
		delete mDownloadItems;
		mDownloadItems = NULL;		
	}		

	//force a sleep here to make sure other threads get a chance.	
	//	Sleep(5000); // 11-16-2004 JROC... DON'T THINK WE NEED THIS NOW THAT SEMAPHORES WORK CORRECTLY
	
	m_DownloadList.DeleteAllItems();
}


//*******************************************************************
//  FUNCTION:   -	OnPrepareForClose()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
LRESULT CMuteDownloadsDlg::OnPrepareForClose( WPARAM wParam, LPARAM lParam )
{
	int		i;

	m_DownloadList.LockList(100000);
	Sleep(100);

	//attempt to cancel current downloads
	if( NULL != mDownloadItems )
	{
		int	nItems = mDownloadItems->size();

		for( i = 0; i < nItems; i++ )
		{
			CMuteDownloadListItem *item = *( mDownloadItems->getElement( i ) );
			item->OnCancelClear();
			m_DownloadList.PumpMessages();
			Sleep(1);
		}
	}

	m_DownloadList.UnlockList();
	m_DownloadList.PrepareForListDestruction();
	return (LRESULT) TRUE;
}

//*******************************************************************
//  FUNCTION:   -	OnClearFailedButton()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteDownloadsDlg::OnClearFailedButton() 
{
	PostMessage( DOWNLOAD_CLEAR_STALLED_EVENT, NULL, NULL );
}

//*******************************************************************
//  FUNCTION:   -	OnClearCompletedButton()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	This function will clear any download that has the status of "Completed"
//*******************************************************************
void CMuteDownloadsDlg::OnClearCompletedButton() 
{
	PostMessage( DOWNLOAD_CLEAR_COMPLETE_EVENT, NULL, NULL );	
}



//*******************************************************************
//  FUNCTION:   -	OnExploreSharedFolder() 
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	This function will open an explorer window in the users
//					share directory.
//*******************************************************************
void CMuteDownloadsDlg::OnExploreSharedFolder() 
{
	char * szSharePath = NULL;

	szSharePath = muteShareGetSharingPath();
	if( NULL != szSharePath )
	{
		// check to see if path exists
		if( PathIsDirectory( szSharePath ) )
		{
			ShellExecute( GetSafeHwnd(), "explore", szSharePath, NULL, szSharePath, SW_SHOW );
		}
		else
		{
			CString strTemp, strTemp2;
			
			strTemp = m_ExtStr.LoadString( IDS_SHARING_FOLDER_DOES_NOT_EXIST_ENG + g_unStringLanguageIdOffset );
			strTemp2 = m_ExtStr.LoadString( IDS_MUTE_ERROR_CAPTION_ENG + g_unStringLanguageIdOffset );
			MessageBox( strTemp, strTemp2, MB_ICONERROR );			
		}

		delete [] szSharePath;
	}	
}

//*******************************************************************
//  FUNCTION:   -	OnPurgeIncomingFolder
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	This function will purge the MUTE incoming files
//					directory only if no downloads are present!
//*******************************************************************
void CMuteDownloadsDlg::OnPurgeIncomingFolder()
{
	char		*szIncomingPath = NULL;	
	int			nListCount;

	if( !m_DownloadList.LockList(10000) )
	{
		return;
	}
	
	nListCount = m_DownloadList.GetItemCount();
	m_DownloadList.UnlockList();

	if( nListCount == 0 )
	{	
		szIncomingPath = muteShareGetIncomingFilesPath();
		
		if( NULL != szIncomingPath )
		{	
			CString	strPurgeCaption;
			CString	strPurgeQuestion;

			strPurgeCaption = m_ExtStr.LoadString( IDS_PURGE_INCOMING_CAPTION_ENG + g_unStringLanguageIdOffset );
			strPurgeQuestion = m_ExtStr.LoadString( IDS_PURGE_INCOMING_QUESTION_ENG + g_unStringLanguageIdOffset );
			if( IDYES == MessageBox( strPurgeQuestion, strPurgeCaption, MB_ICONQUESTION | MB_YESNO ) )
			{
				EmptyDirectory( szIncomingPath, TRUE, TRUE );
			}
			delete [] szIncomingPath;
		}	
	}
	else
	{
		CString	strPurgeCaptionError;
		CString	strPurgeErrorMsg;

		strPurgeCaptionError = m_ExtStr.LoadString( IDS_PURGE_INCOMING_CAPTION_ERROR_ENG + g_unStringLanguageIdOffset );
		strPurgeErrorMsg = m_ExtStr.LoadString( IDS_PURGE_INCOMING_ERROR_MSG_ENG + g_unStringLanguageIdOffset );
		MessageBox( strPurgeErrorMsg, strPurgeCaptionError, MB_ICONERROR );
	}
}

//*******************************************************************
//  FUNCTION:   -	OnExploreIncomingFolder() 
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	This function will open an explorer window in the users
//					incoming directory.
//*******************************************************************
void CMuteDownloadsDlg::OnExploreIncomingFolder() 
{
	char * szPath = NULL;

	szPath = muteShareGetIncomingFilesPath();
	if( NULL != szPath )
	{
		// check to see if path exists
		if( PathIsDirectory( szPath ) )
		{
			ShellExecute( GetSafeHwnd(), "explore", szPath, NULL, szPath, SW_SHOW );
		}
		else
		{
			CString strTemp, strTemp2;
			
			strTemp = m_ExtStr.LoadString( IDS_INCOMING_FOLDER_DOES_NOT_EXIST_ENG + g_unStringLanguageIdOffset );
			strTemp2 = m_ExtStr.LoadString( IDS_MUTE_ERROR_CAPTION_ENG + g_unStringLanguageIdOffset );
			MessageBox( strTemp, strTemp2, MB_ICONERROR );
		}

		delete [] szPath;
	}		
}


//*******************************************************************
//  FUNCTION:   -	GetPtrToListItemFromHash() 
//  RETURNS:    -	
//  PARAMETERS: -	const char *inFileHash
//  COMMENTS:	-	This function will return the pointer to the list item
//					associated with a given Hash...
//*******************************************************************
void * CMuteDownloadsDlg::GetPtrToListItemFromHash( const char *inFileHash )
{
	void					*pListItemPtr = NULL;
	CMuteDownloadListItem	*pListItem;
	bool					bFound = false;
	CString					strHash = inFileHash;
	int						nItemID;
    
	nItemID = m_DownloadList.GetItemCount() - 1;
	while( nItemID >= 0 )
	{
		pListItem = (CMuteDownloadListItem *) m_DownloadList.GetItemData(nItemID);
		if( strHash == pListItem->m_strHash )
		{
			pListItemPtr = (void *) pListItem;
			break;
		}
		nItemID--;
	}
	
	return pListItemPtr;
}

//*******************************************************************
//  FUNCTION:   -	GetPtrToListItemFromLocalFileName() 
//  RETURNS:    -	
//  PARAMETERS: -	const char *inLocalFilename
//  COMMENTS:	-	This function will return the pointer to the list item
//					associated with a given local filename..
//*******************************************************************
void * CMuteDownloadsDlg::GetPtrToListItemFromLocalFileName( const char *inLocalFilename )
{
	void					*pListItemPtr = NULL;
	CMuteDownloadListItem	*pListItem;
	bool					bFound = false;
	CString					strLocalFilename = inLocalFilename;
	int						nItemID;
    
	nItemID = m_DownloadList.GetItemCount() - 1;
	while( nItemID >= 0 )
	{
		pListItem = (CMuteDownloadListItem *) m_DownloadList.GetItemData(nItemID);
		if( strLocalFilename == pListItem->m_strLocalFileName )
		{
			pListItemPtr = (void *) pListItem;
			break;
		}
		nItemID--;
	}
	
	return pListItemPtr;
}

//*******************************************************************
//  FUNCTION:   -	SetStrings()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-					
//*******************************************************************
void CMuteDownloadsDlg::SetStrings()
{
	CString strTemp;
	CExternStr  m_ExtStr;

	m_DownloadList.SetStrings();	
    
   	strTemp.Format( "%s (%d)",
                    m_ExtStr.LoadString( IDS_TAB_TITLE_DLOAD_TEXT_ENG + g_unStringLanguageIdOffset),
                    m_DownloadList.GetItemCount() );
	GetDlgItem( IDC_DOWNLOADS_STATIC )->SetWindowText(strTemp);


	m_btnExploreSharedFiles.SetIcon( IDI_SHAREDFILES_ICON, NULL );
	m_btnExploreSharedFiles.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, 200);	
	m_btnExploreSharedFiles.SetTooltipText( IDS_TOOLTIP_EXPLORE_SHARED_FILES_ENG + g_unStringLanguageIdOffset );
	strTemp = m_ExtStr.LoadString( IDS_DLOAD_EXPLORE_SHARED_BUTTON_ENG + g_unStringLanguageIdOffset );
	m_btnExploreSharedFiles.SetWindowText( strTemp );

	m_btnExploreIncomingFiles.SetIcon( IDI_FOLDER_ICON, NULL );
	m_btnExploreIncomingFiles.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, 200);
	m_btnExploreIncomingFiles.SetTooltipText( IDS_TOOLTIP_EXPLORE_INCOMING_FILES_ENG + g_unStringLanguageIdOffset );
	strTemp = m_ExtStr.LoadString( IDS_DLOAD_EXPLORE_INCOMING_BUTTON_ENG + g_unStringLanguageIdOffset );
	m_btnExploreIncomingFiles.SetWindowText( strTemp );

	m_btnPurgeIncomingFolder.SetIcon( IDI_TRASH_ICON, NULL );
	m_btnPurgeIncomingFolder.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, 200);
	m_btnPurgeIncomingFolder.SetTooltipText( IDS_TOOLTIP_PURGE_INCOMING_FILES_ENG + g_unStringLanguageIdOffset );
	strTemp = m_ExtStr.LoadString( IDS_DLOAD_PURGE_INCOMING_BUTTON_ENG + g_unStringLanguageIdOffset );
	m_btnPurgeIncomingFolder.SetWindowText( strTemp );

	m_btnClearCompletedButton.SetIcon( IDI_CLEAR_RESULTS_ICON, NULL );
	m_btnClearCompletedButton.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, 200);
	strTemp = m_ExtStr.LoadString( IDS_DLOAD_CLEAR_COMPLETE_BUTTON_ENG + g_unStringLanguageIdOffset );
	m_btnClearCompletedButton.SetWindowText( strTemp );
	
	m_btnClearFailedButton.SetIcon( IDI_CLEAR2_ICON, NULL );
	m_btnClearFailedButton.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, 200);
	strTemp = m_ExtStr.LoadString( IDS_DLOAD_CLEAR_FAILED_BUTTON_ENG + g_unStringLanguageIdOffset );
	m_btnClearFailedButton.SetWindowText( strTemp );

	m_btnCancelButton.SetIcon( IDI_CANCEL_SELECTED_ICON, NULL );
	m_btnCancelButton.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, 200);
	strTemp = m_ExtStr.LoadString( IDS_DLOAD_CANCEL_BUTTON_ENG + g_unStringLanguageIdOffset );
	m_btnCancelButton.SetWindowText( strTemp );	
}



/*
//*******************************************************************
//  FUNCTION:    -	OnShowWindow()
//  RETURNS:     -	
//  PARAMETERS:  -	
//  COMMENTS:	 -	
//*******************************************************************
*/
void CMuteDownloadsDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CResizableDialog::OnShowWindow(bShow, nStatus);
	SetStrings();
}
