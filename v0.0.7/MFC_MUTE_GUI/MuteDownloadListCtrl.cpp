// this file was derived from SearchListCtrl.cpp of the eMule project, therefore
// that header is left here.
//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.



#include "stdafx.h"
#include "MuteDownloadListCtrl.h"
#include "ColorNames.h"

#include "MUTE/otherApps/fileSharing/userInterface/common/formatUtils.h"

#include "minorGems/util/SettingsManager.h"
#include "ExternString.h"



// includes and definitions copied from wxWindows sample calendar app



#include "minorGems/util/stringUtils.h"
#include "minorGems/io/file/Path.h"
#include "minorGems/system/Time.h"

#include "MUTE/otherApps/fileSharing/fileShare.h"

#include "MuteDownloadsDlg.h"

#include <time.h>



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern int muteShareChunkSize; // from fileshare.cpp

// CMuteDownloadListCtrl
bool CMuteDownloadListCtrl::m_bAppIsClosing = false;

IMPLEMENT_DYNAMIC(CMuteDownloadListCtrl, CMuleListCtrl)
CMuteDownloadListCtrl::CMuteDownloadListCtrl() 
{
	m_iColumns = 0;
	m_bSetImageList = false;
	//SetGeneralPurposeFind(true);
	m_hSystemImageList = NULL;
	m_bStopUpdating = false;
}

BOOL CMuteDownloadListCtrl::PreTranslateMessage(MSG* pMsg) 
{                     
	// relay mouse events to tooltip control
	if (pMsg->message== WM_LBUTTONDOWN || pMsg->message== WM_LBUTTONUP || pMsg->message== WM_MOUSEMOVE)
	{
		if( !m_bAppIsClosing )
		{
			UpdateToolTips();
			m_ToolTip.RelayEvent(pMsg);
		}
	} 

  return CMuleListCtrl::PreTranslateMessage(pMsg);
}

//*******************************************************************
//  FUNCTION:   -	PumpMessages()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	This function can be called to make sure WINDOWS messages
//					to the control will not hang the system.. i.e.
//					they will be processed...
//*******************************************************************
void CMuteDownloadListCtrl::PumpMessages()
{
	MSG		msg;

	while( ::PeekMessage( &msg, GetSafeHwnd(), 0, 0, PM_REMOVE) )
	{
		::DispatchMessage( &msg );
	}
}
//*******************************************************************
//  FUNCTION:   -	PrepareForListDestruction()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	call before wiping out list (at close with downloads present)
//*******************************************************************
void CMuteDownloadListCtrl::PrepareForListDestruction()
{
	// don't worry about the semaphore
	m_bStopUpdating = true;
	CMuteDownloadListCtrl::m_bAppIsClosing = true;
}


//*******************************************************************
//  FUNCTION:   -	Init()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteDownloadListCtrl::Init()
{
	int		i;
	CString	strTemp;

	SetExtendedStyle(LVS_EX_FULLROWSELECT);
	ModifyStyle(LVS_SINGLESEL,0); // IF YOU WANT IT TO BE SINGLE SELECT REM THIS LINE OUT...

	CreateMenu();

	m_ToolTip.Create(this);
	m_ToolTip.ModifyStyle(0, TTS_NOPREFIX);
	m_ToolTip.SetDelayTime(TTDT_AUTOPOP, 7000);
	m_ToolTip.SetDelayTime(TTDT_INITIAL, 2000);
	m_ToolTip.SendMessage(TTM_SETMAXTIPWIDTH, 0, SHRT_MAX); // recognize \n chars!
    m_ToolTip.Activate(TRUE);
	m_ToolTip.SetTipTextColor( colBlue );  
	
	i=0;
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILENAME_ENG + g_unStringLanguageIdOffset );
	InsertColumn(i++,strTemp,LVCFMT_LEFT,200);
	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_STATUS_ENG + g_unStringLanguageIdOffset );
	InsertColumn(i++,strTemp,LVCFMT_LEFT,170);

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_NUMRESUMES_ENG + g_unStringLanguageIdOffset );
	InsertColumn(i++,strTemp,LVCFMT_LEFT,80);

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILETYPE_ENG + g_unStringLanguageIdOffset );
	InsertColumn(i++,strTemp,LVCFMT_LEFT,100);

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_STARTTIME_ENG + g_unStringLanguageIdOffset );
	InsertColumn(i++,strTemp,LVCFMT_LEFT,130);

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_LASTUPDATETIME_ENG + g_unStringLanguageIdOffset );
	InsertColumn(i++,strTemp,LVCFMT_LEFT,130);	

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_ROUTEQUALITY_ENG + g_unStringLanguageIdOffset );
	InsertColumn(i++,strTemp,LVCFMT_LEFT,100);	

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_HOST_VIP_ENG + g_unStringLanguageIdOffset );
	InsertColumn(i++,strTemp,LVCFMT_LEFT,250);

	m_iColumns = i;
	m_vecOldSortAscending.resize(m_iColumns);
}

CMuteDownloadListCtrl::~CMuteDownloadListCtrl()
{
	m_RightClkMenu.DestroyMenu();
}


BEGIN_MESSAGE_MAP(CMuteDownloadListCtrl, CMuleListCtrl)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT,0,0xFFFF,OnToolTipNotify)
	ON_NOTIFY_REFLECT ( NM_CUSTOMDRAW, OnCustomdraw )
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_WM_CONTEXTMENU()	
#if defined (_MSC_VER) && (_MSC_VER==1310)
// using visual studio 2003
	ON_MESSAGE( DOWNLOAD_PROGRESS_EVENT, (LRESULT(AFX_MSG_CALL CWnd::*)(WPARAM, LPARAM))OnDownloadProgress )
	ON_MESSAGE( DOWNLOAD_RESULT_EVENT, (LRESULT(AFX_MSG_CALL CWnd::*)(WPARAM, LPARAM))OnDownloadResult )
    ON_MESSAGE( DOWNLOAD_CHUNK_RETRY_EVENT, (LRESULT(AFX_MSG_CALL CWnd::*)(WPARAM, LPARAM))OnChunkRetry )
    ON_MESSAGE( DOWNLOAD_CHUNK_RECEIVED_EVENT, (LRESULT(AFX_MSG_CALL CWnd::*)(WPARAM, LPARAM))OnChunkReceived )
	ON_COMMAND( MP_CANCEL, (AFX_PMSG)MenuCancelClick )
	ON_COMMAND( MP_CLEAR, (AFX_PMSG)MenuCancelClick )
	ON_COMMAND( MP_CLEAR_COMPLETE, (AFX_PMSG)MenuClearComplete )	
	ON_COMMAND( MP_CLEAR_STALLED, (AFX_PMSG)MenuClearStalled )
	ON_COMMAND( MP_SEARCH_QUEUE_ITEM_NEXT, (AFX_PMSG)MenuSearchQueueItemNext )
#else
// using visual studio 6.0
	ON_MESSAGE( DOWNLOAD_PROGRESS_EVENT, OnDownloadProgress )
	ON_MESSAGE( DOWNLOAD_RESULT_EVENT, OnDownloadResult )
    ON_MESSAGE( DOWNLOAD_CHUNK_RETRY_EVENT, OnChunkRetry )
    ON_MESSAGE( DOWNLOAD_CHUNK_RECEIVED_EVENT, OnChunkReceived )	
	ON_COMMAND( MP_CANCEL, MenuCancelClick )
	ON_COMMAND( MP_CLEAR, MenuCancelClick )
	ON_COMMAND( MP_CLEAR_COMPLETE, MenuClearComplete )	
	ON_COMMAND( MP_CLEAR_STALLED, MenuClearStalled )
	ON_COMMAND( MP_SEARCH_QUEUE_ITEM_NEXT, MenuSearchQueueItemNext )
#endif
END_MESSAGE_MAP()

// CMuteDownloadListCtrl message handlers

/*
//**************************************************************************************
//  FUNCTION:	-	MenuCancelClick()
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		If a right click menu is popped up over an item and the user clicks
//					the cancel item, this is called for the list.  The list then grabs the
//					pointer to the download item and passes it to the parent dialog because
//					the parent dialog maintains the list of all downloads.
//**************************************************************************************
*/
void CMuteDownloadListCtrl::MenuCancelClick(  WPARAM wParam, LPARAM lParam )
{	
	if( ::IsWindow( GetParent()->GetSafeHwnd() ) )
	{
		GetParent()->PostMessage( DOWNLOAD_CANCEL_EVENT, NULL, NULL );		
	}		
}

//**************************************************************************************
//  FUNCTION:	-	MenuClearComplete()
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		Walks list to see if there are any items that are complete...
//					if so, basically calls cancel/clear on the items.
//**************************************************************************************
void CMuteDownloadListCtrl::MenuClearComplete(  WPARAM wParam, LPARAM lParam )
{

	if( ::IsWindow( GetParent()->GetSafeHwnd() ) )
	{
		GetParent()->PostMessage( DOWNLOAD_CLEAR_COMPLETE_EVENT, NULL, NULL );		
	}		
}


//**************************************************************************************
//  FUNCTION:	-	MenuClearStalled()
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		Walks list to see if there are any items that are stalled...
//					if so, basically calls cancel/clear on the items.
//**************************************************************************************
void CMuteDownloadListCtrl::MenuClearStalled(  WPARAM wParam, LPARAM lParam )
{
	if( ::IsWindow( GetParent()->GetSafeHwnd() ) )
	{				
		GetParent()->PostMessage( DOWNLOAD_CLEAR_STALLED_EVENT, NULL, NULL );		
	}	
}

//**************************************************************************************
//  FUNCTION:	-	MenuSearchQueueItemNext()
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		Used to individually pull search items that are queued to the 
//					next position that will be searched next automatically by the queue
//**************************************************************************************
void CMuteDownloadListCtrl::MenuSearchQueueItemNext(  WPARAM wParam, LPARAM lParam )
{
	if( ::IsWindow( GetParent()->GetSafeHwnd() ) )
	{				
		GetParent()->PostMessage( DOWNLOAD_SEARCH_QUEUE_ITEM_NEXT_EVENT, NULL, NULL );		
	}	
}

/*
//**************************************************************************************
//  FUNCTION:	-	AddDownloadResult()
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		
//**************************************************************************************
*/
void CMuteDownloadListCtrl::AddDownloadResult( CMuteDownloadListItem *pListItem )
{
	CString	str;	
	int i;

	pListItem->m_oStartTime					= CTime::GetCurrentTime();
	pListItem->m_oLastUpdateTime			= pListItem->m_oStartTime;
	pListItem->m_strStartTime				= pListItem->m_oStartTime.Format("%X %x");
	pListItem->m_strResumesText.Format( "%d", pListItem->m_unNumResumes );
	pListItem->m_strLastStatusUpdateTime	= pListItem->m_strStartTime;

	// correct race conditions temporarily
	if( pListItem->m_currentQuality > 20 )
	{
		pListItem->m_currentQuality = 20;
	}
	else if( pListItem->m_currentQuality < 0 )
	{
		pListItem->m_currentQuality = 0;
	}

	// we're out of 20, so 100%/20 = multiplier of 5.
	pListItem->m_strRouteQuality.Format( "%3.0f%%", (float) pListItem->m_currentQuality * 5.0 );

	// stuff for image lists
	int iImage = GetFileTypeSystemImageIdx(pListItem->m_strLocalFileName, -1, pListItem->m_strFileExt, pListItem->m_strFileType );
	pListItem->m_strFileExt.MakeLower();
	if (!m_bSetImageList && m_hSystemImageList != NULL)
	{
		ASSERT( GetStyle() & LVS_SHAREIMAGELISTS );
		ApplyImageList(m_hSystemImageList);
		m_bSetImageList = true;
	}

	
	UINT32 itemnr = GetItemCount();
	
	itemnr = InsertItem(LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE,itemnr,(LPCSTR)pListItem->m_strLocalFileName,0,0,iImage,(LPARAM)pListItem);
	i = 1;
	SetItemText(itemnr,i++, pListItem->m_strStatusText );
	SetItemText(itemnr,i++, pListItem->m_strResumesText );
	SetItemText(itemnr,i++, pListItem->m_strFileType );
	SetItemText(itemnr,i++, pListItem->m_strStartTime );
	SetItemText(itemnr,i++, pListItem->m_strLastStatusUpdateTime );	
	SetItemText(itemnr,i++, pListItem->m_strRouteQuality );
	SetItemText(itemnr,i++, pListItem->m_strHostVirtualAddress );	
}

//*******************************************************************
//  FUNCTION:   -	UpdateToolTips()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteDownloadListCtrl::UpdateToolTips(void)
{
	CString str;
	CString	strTemp;

	int sel = GetItemUnderMouse(this);

	
	if( !m_semListOperations.Lock(100) )
	{
		return;
	}

	if( m_bStopUpdating )
	{
		m_semListOperations.Unlock();
		return;
	}

	if( GetItemCount() <= 0 )
	{
		m_ToolTip.DelTool( this, 0 );
		m_semListOperations.Unlock();
		return;
	}
	
	if (sel < 0 || sel == 65535)
	{
		m_ToolTip.DelTool( this, 0 );
		m_semListOperations.Unlock();
		return;
	}

	// build info text and display it
	CMuteDownloadListItem *pListItem = (CMuteDownloadListItem *) GetItemData(sel);
	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILENAME_ENG + g_unStringLanguageIdOffset );
	str = strTemp;
	str += ": ";
	str += pListItem->m_strLocalFileName;

	str += "\n";	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILETYPE_ENG + g_unStringLanguageIdOffset );
	str += strTemp;
	str += ": ";
	str += pListItem->m_strFileType;

	str += "\n";	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_STARTTIME_ENG + g_unStringLanguageIdOffset );
	str += strTemp;
	str += ": ";
	str += pListItem->m_strStartTime;

	str += "\n";
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_LASTUPDATETIME_ENG + g_unStringLanguageIdOffset );
	str += strTemp;
	str += ": ";
	str += pListItem->m_strLastStatusUpdateTime;

	str += "\n";	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_STATUS_ENG + g_unStringLanguageIdOffset );
	str += strTemp;
	str += ": ";
	str += pListItem->m_strStatusText;
	
	str += "\n";	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_NUMRESUMES_ENG + g_unStringLanguageIdOffset );
	str += strTemp;
	str += ": ";
	str += pListItem->m_strResumesText;
	
	str += "\n";	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_ROUTEQUALITY_ENG + g_unStringLanguageIdOffset );
	str += strTemp;
	str += ": ";
	str += pListItem->m_strRouteQuality;
	
	str += "\n";	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_HOST_VIP_ENG + g_unStringLanguageIdOffset );
	str += strTemp;
	str += ": ";
	str += pListItem->m_strHostVirtualAddress;

	if( m_ToolTip.GetToolCount() < 1 )
	{
		m_ToolTip.AddTool(this, str );
	}
	else
	{
		m_ToolTip.UpdateTipText( (LPCSTR) str, this, 0 );
	}
	
	m_semListOperations.Unlock();
}

//*******************************************************************
//  FUNCTION:   -	OnToolTipNotify()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
BOOL CMuteDownloadListCtrl::OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult)
{
	TOOLTIPTEXT *pText = (TOOLTIPTEXT *)pNMH;
	CString		str;
	CString		strTemp;
	
	int sel = GetItemUnderMouse(this);
	
	if (sel < 0 || sel == 65535)
	{
		return FALSE;
	}

	// build info text and display it
	CMuteDownloadListItem *pListItem = (CMuteDownloadListItem *) GetItemData(sel);
	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILENAME_ENG + g_unStringLanguageIdOffset );
	str = strTemp;
	str += ": ";
	str += pListItem->m_strLocalFileName;

	str += "\n";	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILETYPE_ENG + g_unStringLanguageIdOffset );
	str += strTemp;
	str += ": ";
	str += pListItem->m_strFileType;

	str += "\n";
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_STARTTIME_ENG + g_unStringLanguageIdOffset );
	str += strTemp;
	str += ": ";
	str += pListItem->m_strStartTime;

	str += "\n";
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_LASTUPDATETIME_ENG + g_unStringLanguageIdOffset );
	str += strTemp;
	str += ": ";
	str += pListItem->m_strLastStatusUpdateTime;

	str += "\n";	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_STATUS_ENG + g_unStringLanguageIdOffset );
	str += strTemp;
	str += ": ";
	str += pListItem->m_strStatusText;
	
	str += "\n";	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_NUMRESUMES_ENG + g_unStringLanguageIdOffset );
	str += strTemp;
	str += ": ";
	str += pListItem->m_strResumesText;
	
	str += "\n";	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_ROUTEQUALITY_ENG + g_unStringLanguageIdOffset );
	str += strTemp;
	str += ": ";
	str += pListItem->m_strRouteQuality;
	
	str += "\n";	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_HOST_VIP_ENG + g_unStringLanguageIdOffset );
	str += strTemp;
	str += ": ";
	str += pListItem->m_strHostVirtualAddress;	

			
	m_strToolTip.ReleaseBuffer(); // release old used buffer
	m_strToolTip = str;
	pText->lpszText = m_strToolTip.GetBuffer(1);
	pText->hinst = NULL; // we are not using a resource
	PostMessage(WM_ACTIVATE);
	return TRUE;
}

/*
//**************************************************************************************
//  FUNCTION:	-	Creates the right click menu, so that users can clear/cancel downloads.
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		
//**************************************************************************************
*/
void CMuteDownloadListCtrl::CreateMenu()
{
	CString	strTemp;

	m_RightClkMenu.CreatePopupMenu();
	strTemp = m_ExtStr.LoadString( IDS_DLOAD_RT_CLK_MENU_CAPTION_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AddMenuTitle( strTemp, true );

	strTemp = m_ExtStr.LoadString( IDS_DLOAD_RT_CLK_MENU_CANCEL_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AppendMenu(MF_STRING,MP_CANCEL, strTemp, MAKEINTRESOURCE(IDI_CANCEL_ICON) );

	strTemp = m_ExtStr.LoadString( IDS_DLOAD_RT_CLK_MENU_CLEAR_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AppendMenu(MF_STRING,MP_CLEAR, strTemp, MAKEINTRESOURCE(IDI_CLEAR_ICON) );

	strTemp = m_ExtStr.LoadString( IDS_DLOAD_RT_CLK_MENU_CLEARCOMPLETE_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AppendMenu(MF_STRING,MP_CLEAR_COMPLETE, strTemp, MAKEINTRESOURCE(IDI_CLEAR_COMPLETE_ICON) );

	strTemp = m_ExtStr.LoadString( IDS_DLOAD_RT_CLK_MENU_REMOVESTALLED_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AppendMenu(MF_STRING,MP_CLEAR_STALLED, strTemp, MAKEINTRESOURCE(IDI_REMOVE_STALLED_ICON) );

	//01-23-2005 JROC allow user to push items to next in line in queue
	strTemp = m_ExtStr.LoadString( IDS_DLOAD_RT_CLK_MENU_SEARCH_QUEUE_ITEM_NEXT_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AppendMenu(MF_STRING,MP_SEARCH_QUEUE_ITEM_NEXT, strTemp, MAKEINTRESOURCE(IDI_PRIORITY_ICON) );
}

/*
//**************************************************************************************
//  FUNCTION:	-	OnContextMenu
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		When a user right clicks on the download list, this call back comes up
//					and if an item is selected, the menu will give the option to cancel
//					or clear depending on the state of the item.
//**************************************************************************************
*/
void CMuteDownloadListCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (GetSelectionMark() != (-1))
	{
		CMuteDownloadListItem* pListItem = (CMuteDownloadListItem*)this->GetItemData(GetSelectionMark());
		if( !pListItem->m_strLocalFileName.IsEmpty() )
		{			
			CString	/*strComplete,*/ strOneHundredPercentOf;
			
			strOneHundredPercentOf = m_ExtStr.LoadString( IDS_DLOAD_ONE_HUNDRED_PERCENT_OF_ENG + g_unStringLanguageIdOffset );
	
			if( pListItem->m_strStatusText.Find(strOneHundredPercentOf) > -1 )
			{
				pListItem->m_unItemState = E_ITEM_COMPLETE;
			}

			// if shows completed, don't allow cancel/clear canceled
			if( E_ITEM_COMPLETE == pListItem->m_unItemState )
			{
				m_RightClkMenu.EnableMenuItem(MP_CANCEL,MF_GRAYED);
				m_RightClkMenu.EnableMenuItem(MP_CLEAR,MF_GRAYED);							
			}
			else if( !pListItem->isCanceled() )
			{
				// if not canceled, and not Complete....
				// show cancel, but not clear.
				m_RightClkMenu.EnableMenuItem(MP_CANCEL,MF_ENABLED);
				m_RightClkMenu.EnableMenuItem(MP_CLEAR,MF_GRAYED);				
			}
			else
			{
				// if canceled and not complete.... 
				m_RightClkMenu.EnableMenuItem(MP_CANCEL,MF_GRAYED);
				m_RightClkMenu.EnableMenuItem(MP_CLEAR,MF_ENABLED);			
			}	

			//01-23-2005 allow user to push items to next in line in queue
			if( E_ITEM_QUEUED == pListItem->m_unItemState )
			{
				if( 1 == GetSelectedCount() )
				{
					m_RightClkMenu.EnableMenuItem( MP_SEARCH_QUEUE_ITEM_NEXT, MF_ENABLED );
				}
				else
				{
					m_RightClkMenu.EnableMenuItem( MP_SEARCH_QUEUE_ITEM_NEXT, MF_GRAYED );
				}
			}
			else
			{
				m_RightClkMenu.EnableMenuItem( MP_SEARCH_QUEUE_ITEM_NEXT, MF_GRAYED );
			}

			// always allow clear completed and clear stalled...
			m_RightClkMenu.EnableMenuItem(MP_CLEAR_COMPLETE,MF_ENABLED);
			m_RightClkMenu.EnableMenuItem(MP_CLEAR_STALLED,MF_ENABLED);
			m_RightClkMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);	
		}
	}
	else
	{
		m_RightClkMenu.EnableMenuItem(MP_CANCEL,MF_GRAYED);
		m_RightClkMenu.EnableMenuItem(MP_CLEAR,MF_GRAYED);		
		m_RightClkMenu.EnableMenuItem(MP_CLEAR_COMPLETE,MF_ENABLED);
		m_RightClkMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
	}
}

//*******************************************************************
//  FUNCTION:   -	OnDownloadProgress()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteDownloadListCtrl::OnDownloadProgress( WPARAM wParam, LPARAM lParam )
{
	LVFINDINFO				find;
	CMuteDownloadListItem	*pListItem = ( CMuteDownloadListItem *) lParam;

	find.flags = LVFI_PARAM;
	find.lParam = lParam;
	int result = FindItem(&find);

	if (result != (-1) )
	{
		// get the actual update data for the download
		pListItem->OnDownloadProgress();
		// update the info in the list.
		UpdateInfo( pListItem );
	}	
}



//*******************************************************************
//  FUNCTION:   -	OnChunkRetry()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteDownloadListCtrl::OnChunkRetry( WPARAM wParam, LPARAM lParam ) 
{
	LVFINDINFO				find;
	CMuteDownloadListItem	*pListItem = ( CMuteDownloadListItem *) lParam;
	
	find.flags = LVFI_PARAM;
	find.lParam = lParam;
	int result = FindItem(&find);

	if (result != (-1) )
	{
		pListItem->OnChunkRetry();
		UpdateInfo( pListItem );
	}
}



//*******************************************************************
//  FUNCTION:   -	OnChunkReceived()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteDownloadListCtrl::OnChunkReceived( WPARAM wParam, LPARAM lParam )
{
	LVFINDINFO				find;
	CMuteDownloadListItem	*pListItem = ( CMuteDownloadListItem *) lParam;
	
	find.flags = LVFI_PARAM;
	find.lParam = lParam;
	int result = FindItem(&find);

	if (result != (-1) )
	{
		pListItem->OnChunkReceived();
		UpdateInfo( pListItem );
	}	
}



//*******************************************************************
//  FUNCTION:   -	OnDownloadResult()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteDownloadListCtrl::OnDownloadResult( WPARAM wParam, LPARAM lParam )
{
	LVFINDINFO				find;
	CMuteDownloadListItem	*pListItem = ( CMuteDownloadListItem *) lParam;
	
	find.flags = LVFI_PARAM;
	find.lParam = lParam;
	int result = FindItem(&find);

	if (result != (-1) )
	{
		pListItem->OnDownloadResult();
		UpdateInfo( pListItem );
	}	
}

//*******************************************************************
//  FUNCTION:   -	UpdateInfo
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteDownloadListCtrl::UpdateInfo(CMuteDownloadListItem * pListItem )
{
	int		i;
	int		nListCount = GetItemCount();
	int		item;

	pListItem->m_oLastUpdateTime = CTime::GetCurrentTime();
	pListItem->m_strLastStatusUpdateTime = pListItem->m_oLastUpdateTime.Format("%X %x");
	pListItem->m_strResumesText.Format( "%d", pListItem->m_unNumResumes );

	// correct race conditions temporarily
	if( pListItem->m_currentQuality > 20 )
	{
		pListItem->m_currentQuality = 20;
	}
	else if( pListItem->m_currentQuality < 0 )
	{
		pListItem->m_currentQuality = 0;
	}
	// we're out of 20, so 100%/20 = multiplier of 5.
	pListItem->m_strRouteQuality.Format( "%3.0f%%", (float) pListItem->m_currentQuality * 5.0 );
	
	
	for( i = 0; i < nListCount; i++ )
	{		
		if( (CMuteDownloadListItem *) GetItemData( i ) == pListItem )
		{
			int nCurrentSortItem;
			item = 0;
			
			SetItemText( i,item++, pListItem->m_strLocalFileName );
			SetItemText( i,item++, pListItem->m_strStatusText );
			SetItemText( i,item++, pListItem->m_strResumesText );	
			SetItemText( i,item++, pListItem->m_strFileType );
			SetItemText( i,item++, pListItem->m_strStartTime );
			SetItemText( i,item++, pListItem->m_strLastStatusUpdateTime );			
			SetItemText( i,item++, pListItem->m_strRouteQuality );
			SetItemText( i,item++, pListItem->m_strHostVirtualAddress );
			
			nCurrentSortItem = GetCurrentSortItem();
			if( -1 != nCurrentSortItem )
			{
				SortItems(SortProc, nCurrentSortItem + (!m_vecOldSortAscending[nCurrentSortItem] ? 0:10));			
			}
		}
	}
}

//*******************************************************************
//  FUNCTION:   -	OnCustomdraw
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteDownloadListCtrl::OnCustomdraw(NMHDR* pNMHDR, LRESULT* pResult )
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );

    *pResult = CDRF_DODEFAULT;

	if( m_bAppIsClosing )
	{
		return;
	}

	if( m_bStopUpdating )
	{
		return;
	}

    if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
        *pResult = CDRF_NOTIFYITEMDRAW;
    }
    else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		COLORREF crText = ::GetSysColor(COLOR_WINDOWTEXT);
		UINT32 red = GetRValue(crText);
		UINT32 green = GetGValue(crText);
		UINT32 blue = GetGValue(crText);
		
		CMuteDownloadListItem *pListItem = (CMuteDownloadListItem *) pLVCD->nmcd.lItemlParam;
		
		if( NULL != pListItem ) 
		{				
			CString	strOneHundredPercent;

			strOneHundredPercent = m_ExtStr.LoadString( IDS_DLOAD_ONE_HUNDRED_PERCENT_ENG + g_unStringLanguageIdOffset );			
			
			if( pListItem->m_strStatusText.Find( strOneHundredPercent ) > -1 )
			{
				pListItem->m_unItemState = E_ITEM_COMPLETE;
			}
			
			if( (E_ITEM_CANCELED == pListItem->m_unItemState ) ||
				(E_ITEM_CANCELLING == pListItem->m_unItemState ) ||
				(E_ITEM_FAILED == pListItem->m_unItemState ) ||
				(E_ITEM_FAILED_TO_WRITE_LOCAL_FILE == pListItem->m_unItemState ) ||
				(E_ITEM_CORRUPT == pListItem->m_unItemState ) ||
				(E_ITEM_NOT_FOUND == pListItem->m_unItemState ) ||
				pListItem->isCanceled()
			  )		
			{
				red = 255;
			}			
			else if( E_ITEM_COMPLETE == pListItem->m_unItemState )				
			{
				blue = 255;
			}
			else
			{
				green = 128;			
			}
		}
		
		pLVCD->clrText = RGB(red, green ,blue);
		pLVCD->clrTextBk = ::GetSysColor(COLOR_WINDOW);
        *pResult = CDRF_DODEFAULT;
     }
}

//*******************************************************************
//  FUNCTION:   -	OnColumnClick()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteDownloadListCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	int	sortItem;
	
	// Item is column clicked
	sortItem = pNMListView->iSubItem;

	// Sort table
	SetSortArrow(sortItem, m_vecOldSortAscending[sortItem]);
	SortItems(SortProc, sortItem + (m_vecOldSortAscending[sortItem] ? 0:10));
	m_vecOldSortAscending[sortItem] = !m_vecOldSortAscending[sortItem]; // toggle
	
	*pResult = 0;
}

//*******************************************************************
//  FUNCTION:   -	SortProc()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
int CMuteDownloadListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CMuteDownloadListItem *pListItem1  = (CMuteDownloadListItem *)lParam1;
	CMuteDownloadListItem *pListItem2 = (CMuteDownloadListItem *)lParam2;

	if( m_bAppIsClosing )
	{
		return false;
	}

	switch(lParamSort)
	{
	case 0:  // filename asc
		return pListItem1->m_strLocalFileName.CompareNoCase(pListItem2->m_strLocalFileName);
	case 10: // filename desc
		return pListItem2->m_strLocalFileName.CompareNoCase(pListItem1->m_strLocalFileName);	
	case 1: // Status text asc
		return (pListItem1->m_unItemState > pListItem2->m_unItemState);
	case 11:
		return (pListItem2->m_unItemState > pListItem1->m_unItemState);	
	case 2:// resumes asc
		return (pListItem1->m_unNumResumes > pListItem2->m_unNumResumes);		
	case 12://resumes desc
		return (pListItem2->m_unNumResumes > pListItem1->m_unNumResumes);
	case 3:  // file type asc
		return pListItem1->m_strFileType.CompareNoCase(pListItem2->m_strFileType);
	case 13: // file type desc
		return pListItem2->m_strFileType.CompareNoCase(pListItem1->m_strFileType);	
	case 4: // start time asc
		return (pListItem1->m_oStartTime > pListItem2->m_oStartTime);
	case 14:// start time desc
		return (pListItem2->m_oStartTime > pListItem1->m_oStartTime);
	case 5: // last update time asc
		return (pListItem1->m_oLastUpdateTime > pListItem2->m_oLastUpdateTime);
	case 15:// last update time desc
		return (pListItem2->m_oLastUpdateTime > pListItem1->m_oLastUpdateTime);	
	case 6: // route quality asc
		return (pListItem1->m_currentQuality > pListItem2->m_currentQuality);
	case 16: // route quality desc
		return (pListItem2->m_currentQuality > pListItem1->m_currentQuality);
	case 7: // virtual address asc
		return pListItem1->m_strHostVirtualAddress.CompareNoCase(pListItem2->m_strHostVirtualAddress);
	case 17:// virtual address desc
		return pListItem2->m_strHostVirtualAddress.CompareNoCase(pListItem1->m_strHostVirtualAddress);
	default:
		return 0;
	}
	return 0;
}




//*******************************************************************
//  FUNCTION:   -	
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteDownloadListCtrl::OnLvnDeleteallitems(NMHDR *pNMHDR, LRESULT *pResult)
{
	// To suppress subsequent LVN_DELETEITEM notification messages, return TRUE.
	*pResult = TRUE;
}


//*******************************************************************
//  FUNCTION:   -	GetFileTypeSystemImageIdx()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	Snags the file type and icon image for the file type
//*******************************************************************
int CMuteDownloadListCtrl::GetFileTypeSystemImageIdx(LPCTSTR pszFilePath, int iLength, /*OUT*/ CString &strExtension, /*out*/ CString &strFileType )
{
	//TODO: This has to be MBCS aware..
	DWORD dwFileAttributes;
	LPCTSTR pszCacheExt = NULL;
	if (iLength == -1)
		iLength = _tcslen(pszFilePath);
	if (iLength > 0 && (pszFilePath[iLength - 1] == _T('\\') || pszFilePath[iLength - 1] == _T('/'))){
		// it's a directory
		pszCacheExt = _T("\\");
		dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	}
	else{
		dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
		// search last '.' character *after* the last '\\' character
		for (int i = iLength - 1; i >= 0; i--)
		{
			if (pszFilePath[i] == _T('\\') || pszFilePath[i] == _T('/'))
				break;
			if (pszFilePath[i] == _T('.')) {
				// point to 1st character of extension (skip the '.')
				pszCacheExt = &pszFilePath[i+1];
				break;
			}
		}
		if (pszCacheExt == NULL)
			pszCacheExt = _T("");	// empty extension
	}

	strExtension = pszCacheExt;
	// Search extension in "ext->idx" cache.
	LPVOID vData;
	if (!m_aExtToSysImgIdx.Lookup(pszCacheExt, vData))
	{
		// Get index for the system's small icon image list
		SHFILEINFO sfi;
		DWORD dwResult = SHGetFileInfo(pszFilePath, dwFileAttributes, &sfi, sizeof(sfi),
									   SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_TYPENAME);
		if (dwResult == 0)
			return 0;
		ASSERT( m_hSystemImageList == NULL || m_hSystemImageList == (HIMAGELIST)dwResult );
		m_hSystemImageList = (HIMAGELIST)dwResult;

		// Store icon index in local cache
		m_aExtToSysImgIdx.SetAt(pszCacheExt, (LPVOID)sfi.iIcon);
		strFileType = sfi.szTypeName;
		return sfi.iIcon;
	}

	SHFILEINFO sfi2;
	DWORD dwResult2 = SHGetFileInfo(pszFilePath, dwFileAttributes, &sfi2, sizeof(sfi2),
									   SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_TYPENAME);
	if(	0 != dwResult2 )
	{
		strFileType = sfi2.szTypeName;
	}
	// Return already cached value
	// Elandal: Assumes sizeof(void*) == sizeof(int)
	return (int)vData;
}

// callback for incoming files
char fileHandler( unsigned char *inChunk,
                  int inChunkLengthInBytes,
                  void *inExtraParam ) 
{

    // extra param is CMuteDownloadListItem
    CMuteDownloadListItem *item = (CMuteDownloadListItem *)inExtraParam;
	
	// jroc.. override on shutdown so we don't try to access
	// a pointer that doesn't exist any longer
	if( CMuteDownloadListCtrl::m_bAppIsClosing )
	{
		return false;
	}

    char keepGoing = !( item->isCanceled() );
    
    if( keepGoing ) 
	{
        // only send chunk event if we haven't been canceled

        if( inChunk != NULL ) 
		{
            // we have been passed chunk data
            item->processChunk( inChunk, inChunkLengthInBytes, inExtraParam );
        }
        else 
		{
            // else chunk timed out, and API will retry

            // tell item about the retry
            //jroc -- let's slow down the update rate on this
			item->m_unRetryUpdateCtr++;
			if( item->m_unRetryUpdateCtr > 5 )
			{	
				item->processChunk( NULL, -1, inExtraParam );
				item->m_unRetryUpdateCtr = 0;
            }
		}
	}
	
	return keepGoing;
}


DownloadThread::DownloadThread( const char *inFromAddress, const char *inFilePath,
                                const char *inFileHash,
								int inStartChunk,
                                SHA_CTX inShaContext,
                                unsigned long *inDownloadSizePointer,
                                void *inCMuteDownloadListItem )
    : mFromAddress( stringDuplicate( inFromAddress ) ),
      mFilePath( stringDuplicate( inFilePath ) ),
      mFileHash( stringDuplicate( inFileHash ) ),
	  mStartChunk( inStartChunk ),
      mShaContext( inShaContext ),
      mDownloadSizePointer( inDownloadSizePointer ),
      mDownloadItem( inCMuteDownloadListItem ),
      mStopLock( new MutexLock() ),
      mStopped( false ) {

    start();
    }



DownloadThread::~DownloadThread() {
    // tell thread to stop (so it won't send its last event after the Yield)
    // jroc removed 10-05-2004 mStopLock->lock();	
    mStopped = true;
    //mStopLock->unlock();    

	join();

    delete [] mFromAddress;
    delete [] mFilePath;
    delete [] mFileHash;
    
    delete mStopLock;
}




void DownloadThread::run() 
{
    // get the timeout setting
	CMuteDownloadListItem * pDLItem		= (CMuteDownloadListItem *) mDownloadItem;
	FILE				  * pf			= pDLItem->GetFilePtr();
	File				  * pFileObj	= pDLItem->GetFileObjPtr();
    char					found;
    int						downloadTimeoutMilliseconds;
	
	downloadTimeoutMilliseconds = SettingsManager::getIntSetting( "downloadTimeoutMilliseconds", &found );
   
	if( !found || downloadTimeoutMilliseconds < 0 ) 
	{
        // default to 60 seconds
        downloadTimeoutMilliseconds = 60000;
    }

	// jroc.. move the file hashing on resume here since it will be in a thread separate
	// from the GUI, it may help the GUI to not appear as "frozen"
	if( pDLItem->IsFileBeingResumed() )
	{
		// time to get the SHAContext calculated here!
		unsigned long	lengthNow = pFileObj->getLength();
		unsigned long	ulTempChunkSize = 50000;
		unsigned long	ulBytesRead = 0;
		int				chunksInFile = lengthNow / muteShareChunkSize;     
		//unsigned char	*buffer = new unsigned char[ muteShareChunkSize + 10 ];
		unsigned char	*buffer = new unsigned char[ ulTempChunkSize ];

		unsigned long		numRead;

		if( lengthNow % muteShareChunkSize != 0 ) 
		{
			// extra partial chunk, who knows, go back one
			chunksInFile--;
			if (chunksInFile < 0) 
			{
				chunksInFile = 0;
			}
		}

		// hash the incoming file data
		ulBytesRead = 0;
		while( ulBytesRead < lengthNow )
		{
			if( (lengthNow - ulBytesRead) > ulTempChunkSize )
			{
				numRead = (unsigned long) fread( buffer, 1, ulTempChunkSize, pf );
			}
			else
			{
				numRead = (unsigned long) fread( buffer, 1, (lengthNow - ulBytesRead), pf );
			}
			ulBytesRead += numRead;
			if( numRead > 0 ) 
			{
				// add data to the hash
				SHA1_Update( &mShaContext, buffer, numRead );
			}
			else
			{
				break; // read all the bytes in!
			}
			// added 08-14-2005 trying to avoid slowing other threads
			// when we have big file resumes and costly hashing computations
			YieldProcessor();
		}				

		delete [] buffer;

		pDLItem->SetBytesDLSoFar( chunksInFile * muteShareChunkSize );		

		// we should now be positioned at the correct
		// spot to start writing again		
		mStartChunk = chunksInFile;		
	}



	// start the file transfer
	int result = muteShareGetFile(
			mFromAddress,
			mFilePath,
			mFileHash,			
			mStartChunk,
			mShaContext,
			fileHandler,
			mDownloadItem,  // pass download item as handler arg
			mDownloadSizePointer,
			downloadTimeoutMilliseconds );
    
	mStopLock->lock();
    char stopped = mStopped;
	mStopLock->unlock();

	if( !stopped ) 
	{
		// don't post event if stopped (avoid deadlock)
        // pass result back to parent item
        CMuteDownloadListItem *item = (CMuteDownloadListItem *)mDownloadItem;
        item->processDownloadResult( result );
    }
}

/////////////////////////////////////////////////////////////////////////////////
///////  constructor used when we restart MUTE and items are queued ... 
CMuteDownloadListItem::CMuteDownloadListItem( CDialog *inParent, const char *inLocalFilePath, const char *inFileHash ) 
	: mParentWindow( inParent ),
      mDownloadStatusLock( new MutexLock() ),
	  mDownloadThread( NULL ),
	  mDownloadFILE( NULL ),
	  mDownloadFile( NULL ),
      mDownloadActive( false ),
      mDownloadSizeInBytes( 0 ),
      mDownloadedSoFarInBytes( 0 ),
      mCurrentBlockStartTimeSeconds( 0 ),
      mCurrentBlockStartTimeMilliseconds( 0 ),
      mCurrentBlockStartSize( 0 ),
      mCurrentRate( 0 ),
      mCanceled( false ),
      mCleared( false ),
	  m_unRetryUpdateCtr( 0 ),
	  m_unNumResumes( 0 )
{
	m_unInitialByteCount = 0;
	m_strLocalFileName = inLocalFilePath;
	m_strHash = inFileHash;
	m_bIsBeingResumed = true;

	m_unItemState = E_ITEM_QUEUED;
	m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_QUEUED_ENG + g_unStringLanguageIdOffset );
	m_strResumesText = "0";
	m_strHostVirtualAddress = "-";	
}


// the parameter bIsAResumedFile is used to change the initial status text
// from "Fetching file info.." into "Resuming download..."
CMuteDownloadListItem::CMuteDownloadListItem( CDialog *inParent, const char *inFromAddress, const char *inLocalFilePath,const char *inRemoteFilePath,
                     const char *inFileHash, bool bIsAResumedFile, unsigned unNumResumes )
	: mParentWindow( inParent ),
      mDownloadStatusLock( new MutexLock() ),
      mDownloadActive( true ),
      mDownloadSizeInBytes( 0 ),
      mDownloadedSoFarInBytes( 0 ),
      mCurrentBlockStartTimeSeconds( 0 ),
      mCurrentBlockStartTimeMilliseconds( 0 ),
      mCurrentBlockStartSize( 0 ),
      mCurrentRate( 0 ),
      mCanceled( false ),
      mCleared( false ),
	  m_unRetryUpdateCtr( 0 ),
	  m_unNumResumes( unNumResumes )
{
	m_unInitialByteCount = 0;
	mDownloadThread = NULL; // JROC... 
	mDownloadFILE =	NULL;
	// this is used when we delete item so we don't delete the partial download
	// don't confuse this with the paramater passed to this function
	m_bIsBeingResumed = false;
	m_strHash = inFileHash;

    Time::getCurrentTime( &mDownloadStartTimeSeconds,
                          &mDownloadStartTimeMilliseconds );

    mCurrentBlockStartTimeSeconds = mDownloadStartTimeSeconds;
    mCurrentBlockStartTimeMilliseconds = mDownloadStartTimeMilliseconds;  

	m_currentQuality = 10;

    // save the download to a file
    char *lastSlash = new char[ strlen(inLocalFilePath) + 1];
	char *pOriginalLastSlash = lastSlash;
	strcpy( lastSlash, inLocalFilePath );
    char *nextSlash = new char[ strlen(inLocalFilePath) + 1];
	char *pOriginalNextSlash = nextSlash;
	strcpy( nextSlash, inLocalFilePath );
    while( nextSlash != NULL ) 
	{
        // skip slash
        if( nextSlash[0] == '/' ) 
		{
            lastSlash = &( nextSlash[1] );
		}
        
        nextSlash = strstr( lastSlash, "/" );
    }
    
	delete [] pOriginalNextSlash;
    char *localFileName = lastSlash;
	m_strLocalFileName = localFileName;	
    
	char *shareIncomingPathString = muteShareGetIncomingFilesPath();
    File *incomingDirectory = new File( NULL, shareIncomingPathString );    
	delete [] shareIncomingPathString;
    
    if( incomingDirectory != NULL && ! incomingDirectory->exists() ) 
	{
        incomingDirectory->makeDirectory();
    }

    char fileOpenedForWriting = false;
    
    if( incomingDirectory != NULL ) 
	{
        mDownloadFile = incomingDirectory->getChildFile( localFileName );
    
        if( mDownloadFile != NULL ) 
		{
            char *fullFileName = mDownloadFile->getFullFileName();
                                    
			int startChunk = 0; // chunks start at number 0
            SHA_CTX shaContext;
            SHA1_Init( &shaContext ); // start the sha1 variable here

			// does it exist?
			if( mDownloadFile->exists()) 
			{ 
				// if download was resumed from search window, account for it!
				if( 0 == m_unNumResumes )
				{
					m_unNumResumes++;
					m_bIsBeingResumed = true;
				}

                mDownloadFILE = fopen( fullFileName, "r+b" ); // binary file for updating r&w

				if( NULL == mDownloadFILE )
				{	
					m_unNumResumes = 0;
					m_bIsBeingResumed = false;
					mDownloadFILE = fopen( fullFileName, "wb" );
				}     

			} // if (the file exists)
			else
			{
				m_unNumResumes = 0;
				m_bIsBeingResumed = false;
				mDownloadFILE = fopen( fullFileName, "wb" );
			}

            delete [] fullFileName;

			if( m_bIsBeingResumed )
			{		
				m_unItemState = E_ITEM_RESUMING;
				m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_RESUMING_ENG + g_unStringLanguageIdOffset );
			}
			else
			{
				m_unItemState = E_ITEM_FETCHING_INFO;
				m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_FETCHINGINFO_ENG + g_unStringLanguageIdOffset );
			}
			
            // make sure that we could open the file for writing
            if( mDownloadFILE != NULL ) 
			{                
                mDownloadThread = new DownloadThread( inFromAddress,
                                                      inRemoteFilePath,
                                                      inFileHash,
													  startChunk,
                                                      shaContext,
                                                      &mDownloadSizeInBytes,
                                                      (void *)this );
                fileOpenedForWriting = true;
            }
            else 
			{
                delete mDownloadFile;
				mDownloadFile = NULL;
            }
		}
		
		delete incomingDirectory;
	}

    
    if( !fileOpenedForWriting ) 
	{
        mDownloadActive = false;
		CMuteDownloadsDlg *pDownloadsDlg = (CMuteDownloadsDlg *) mParentWindow->GetParent();

		m_unItemState = E_ITEM_FAILED_TO_WRITE_LOCAL_FILE;
		m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_FAILED_TO_WRITE_LOCAL_FILE_ENG + g_unStringLanguageIdOffset );        
		pDownloadsDlg->PostMessage( DOWNLOAD_RESULT_REMOVE_FROM_QUEUE, 0, (LPARAM) this ); // remove from DL queue
    }

	delete [] pOriginalLastSlash;
}

////////////////////////////
void CMuteDownloadListItem::RestartDownload( const char *inFromAddress, const char *inRemoteFilePath )
{		
	mDownloadActive = true;
	mDownloadSizeInBytes = -1;
	mDownloadedSoFarInBytes = 0;
	mCurrentBlockStartTimeSeconds = 0;
    mCurrentBlockStartTimeMilliseconds = 0;
    mCurrentBlockStartSize = 0;
    mCurrentRate = 0;
    mCanceled = false;
    mCleared = false;
	m_unRetryUpdateCtr = 0;	
	m_bIsBeingResumed = true;
		
	char * pszLocalFileName;

	m_strHostVirtualAddress = inFromAddress;

	if( NULL != mDownloadFILE )
	{
		fclose( mDownloadFILE );
		mDownloadFILE =	NULL;
	}
	
    Time::getCurrentTime( &mDownloadStartTimeSeconds,
                          &mDownloadStartTimeMilliseconds );

    mCurrentBlockStartTimeSeconds = mDownloadStartTimeSeconds;
    mCurrentBlockStartTimeMilliseconds = mDownloadStartTimeMilliseconds;
    
	m_unItemState = E_ITEM_RESUMING;
	m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_RESUMING_ENG + g_unStringLanguageIdOffset );		
	m_currentQuality = 10;
    
	char *shareIncomingPathString = muteShareGetIncomingFilesPath();
    File *incomingDirectory = new File( NULL, shareIncomingPathString );    
	delete [] shareIncomingPathString;
    
    if( incomingDirectory != NULL && ! incomingDirectory->exists() ) 
	{
        incomingDirectory->makeDirectory();
    }

    char fileOpenedForWriting = false;
    
    if( incomingDirectory != NULL ) 
	{		
		if( NULL != mDownloadFile )
		{
			delete mDownloadFile;
			mDownloadFile = NULL;
		}

		pszLocalFileName = new char[ m_strLocalFileName.GetLength() + 1 ];
		sprintf( pszLocalFileName, "%s", (LPCSTR) m_strLocalFileName );
		mDownloadFile = incomingDirectory->getChildFile( pszLocalFileName );
		delete [] pszLocalFileName;
      
        if( mDownloadFile != NULL ) 
		{
            char *fullFileName = mDownloadFile->getFullFileName();
                                    
			int startChunk = 0; // chunks start at number 0
            SHA_CTX shaContext;
            SHA1_Init( &shaContext ); // start the sha1 variable here

			// does it exist?
			if( mDownloadFile->exists() ) 
			{ 
				m_unNumResumes++;
				mDownloadFILE = fopen( fullFileName, "r+b" ); // binary file for updating r&w

				if( NULL == mDownloadFILE )
				{	
					// in case user removed file or moved the file.. 
					m_unNumResumes = 0;
					m_bIsBeingResumed = false;
					mDownloadFILE = fopen( fullFileName, "wb" );
				}     

			} // if (the file exists)
			else
			{
				// in case user removed file or moved the file.. 
				m_unNumResumes = 0;
				m_bIsBeingResumed = false;
				mDownloadFILE = fopen( fullFileName, "wb" );
			}

            delete [] fullFileName;
			
            // make sure that we could open the file for writing
            if( mDownloadFILE != NULL ) 
			{                				
				if( mDownloadThread != NULL ) 
				{
					delete mDownloadThread;        
					mDownloadThread = NULL;	
				}
            
				mDownloadThread = new DownloadThread( inFromAddress,
                                                      inRemoteFilePath,
													  (LPCSTR) m_strHash,
													  startChunk,
                                                      shaContext,
                                                      &mDownloadSizeInBytes,
                                                      (void *)this );
                fileOpenedForWriting = true;				
				((CMuteDownloadListCtrl *) mParentWindow)->UpdateInfo(this);
            }
            else 
			{
                delete mDownloadFile;
				mDownloadFile = NULL;
            }
		}
		
		delete incomingDirectory;
	}

    
    if( !fileOpenedForWriting ) 
	{
        mDownloadActive = false;
		CMuteDownloadsDlg *pDownloadsDlg = (CMuteDownloadsDlg *) mParentWindow->GetParent();

		m_unItemState = E_ITEM_FAILED_TO_WRITE_LOCAL_FILE;
		m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_FAILED_TO_WRITE_LOCAL_FILE_ENG + g_unStringLanguageIdOffset );        
		pDownloadsDlg->PostMessage( DOWNLOAD_RESULT_REMOVE_FROM_QUEUE, 0, (LPARAM) this ); // remove from DL queue
    }
}


CMuteDownloadListItem::~CMuteDownloadListItem() {

	
	char	cValueFound;
	int		nDeleteCanceled;

    mDownloadStatusLock->lock();
    if( mDownloadActive ) {
        // cancel active download
        mCanceled = true;
        }
    mDownloadStatusLock->unlock();
    
    
    // yield to allow any last event from fileHandler to come through
	((CMuteDownloadListCtrl *) mParentWindow)->PumpMessages();
    	
	// wait for thread to finish and destroy it
    if( mDownloadThread != NULL ) {
		
        // make sure we unlock GUI mutex before joining thread to avoid
        // a deadlock (thread may be trying to post events)
        delete mDownloadThread;        
        
        mDownloadThread = NULL;	
        }
    
    // clean up a partial download if we were destroyed while
    // the download was still active
    // (only happens when app shuts down mid-download)
    mDownloadStatusLock->lock();
    if( mDownloadActive ) {
        // thread stopped before posting result

        // flush and close file
        fclose( mDownloadFILE );
		mDownloadFILE = NULL;

        // delete partial file
        if( mDownloadFile->exists() ) 
		{
			// JROC
			if( !m_bIsBeingResumed )
			{
				nDeleteCanceled = SettingsManager::getIntSetting( "deleteCanceled", &cValueFound );
				if( !cValueFound )
				{
					nDeleteCanceled = 1; // default is to delete files that are canceled!
				}
				
				if( 1 == nDeleteCanceled )
				{
					mDownloadFile->remove();
				}
			}
        }
    
		delete mDownloadFile;
		mDownloadFile = NULL;
    }
    mDownloadStatusLock->unlock();

    
    delete mDownloadStatusLock;
    }



void CMuteDownloadListItem::processChunk( unsigned char *inChunk,
                                 int inChunkLengthInBytes, void * inExtraParam ) {

    if( inChunk != NULL ) {
        mDownloadStatusLock->lock();
        
        fwrite( inChunk, 1, inChunkLengthInBytes, mDownloadFILE );
        
        mDownloadedSoFarInBytes += inChunkLengthInBytes;
        
        mDownloadStatusLock->unlock();
        
        
        if( ::IsWindow( ((CMuteDownloadListCtrl *) mParentWindow)->GetSafeHwnd() ) )
		{
			((CMuteDownloadListCtrl *) mParentWindow)->PostMessage( DOWNLOAD_CHUNK_RECEIVED_EVENT, 0, (LPARAM) inExtraParam );
		}
    }
    else {

        if( ::IsWindow( ((CMuteDownloadListCtrl *) mParentWindow)->GetSafeHwnd() ) )
		{
			((CMuteDownloadListCtrl *) mParentWindow)->PostMessage( DOWNLOAD_CHUNK_RETRY_EVENT, 0, (LPARAM) inExtraParam );
		}
    }

	if( ::IsWindow( ((CMuteDownloadListCtrl *) mParentWindow)->GetSafeHwnd() ) )
	{

		((CMuteDownloadListCtrl *) mParentWindow)->PostMessage( DOWNLOAD_PROGRESS_EVENT, 0, (LPARAM) inExtraParam );
	}
}



void CMuteDownloadListItem::processDownloadResult( int inResult ) {
    mDownloadStatusLock->lock();    
    mDownloadResult = inResult;
    mDownloadStatusLock->unlock();    

	if( ::IsWindow( ((CMuteDownloadListCtrl *) mParentWindow)->GetSafeHwnd() ) )
	{		
		// send event so that GUI thread can update progress UI
		((CMuteDownloadListCtrl *) mParentWindow)->PostMessage( DOWNLOAD_RESULT_EVENT, 0, (LPARAM) this );
	}
	

}



char CMuteDownloadListItem::isCanceled() {
    mDownloadStatusLock->lock();
    char returnValue = mCanceled;
    mDownloadStatusLock->unlock();

    return returnValue;
    }



char CMuteDownloadListItem::isCleared() {
    mDownloadStatusLock->lock();
    char returnValue = mCleared;
    mDownloadStatusLock->unlock();

    return returnValue;
    }



char CMuteDownloadListItem::isActive() {
    mDownloadStatusLock->lock();
    char returnValue = mDownloadActive;
    mDownloadStatusLock->unlock();

    return returnValue;
    }

//*******************************************************************
//  FUNCTION:   -	OnCancelClear()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteDownloadListItem::OnCancelClear()   
{	
	mDownloadStatusLock->lock();

    if( mDownloadActive ) 
	{
        // cancel active download        
        mCanceled = true;        
		m_unItemState = E_ITEM_CANCELLING;
		m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_CANCELLING_ENG + g_unStringLanguageIdOffset );
    }
    else 
	{		
        // not active, so clear this item
        mCleared = true;        

		m_unItemState = E_ITEM_CANCELED;
		m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_CANCELLED_ENG + g_unStringLanguageIdOffset );
    }
	
	mDownloadStatusLock->unlock();
}

//*******************************************************************
//  FUNCTION:   -	OnDownloadProgress()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteDownloadListItem::OnDownloadProgress()
{
	CString	strTemp;
    mDownloadStatusLock->lock();
    
	if( -1 == mDownloadSizeInBytes )
	{
		if( m_unNumResumes > 0 )
		{
			m_unItemState = E_ITEM_RESUMING;
			m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_RESUMING_ENG + g_unStringLanguageIdOffset );
		}
		else
		{
			m_unItemState = E_ITEM_FETCHING_INFO;
			m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_FETCHINGINFO_ENG + g_unStringLanguageIdOffset );
		}

		mDownloadStatusLock->unlock();
		return;
	}
    
    float fractionDone =
        (float)mDownloadedSoFarInBytes / (float)mDownloadSizeInBytes;

    int percentDone = (int)( fractionDone * 100 );

    long timeDelta =
        Time::getMillisecondsSince( mCurrentBlockStartTimeSeconds,
                                    mCurrentBlockStartTimeMilliseconds ); 
    if( timeDelta >= 1000 ) {
        // 1 second has passed since last progress update

        int sizeDelta = mDownloadedSoFarInBytes - mCurrentBlockStartSize;

        if( sizeDelta > 0 ) {
            // we have received more data since the last progress update
            
            mCurrentRate = ( (float)sizeDelta / (float)timeDelta ) * 1000;
            
            mCurrentBlockStartSize = mDownloadedSoFarInBytes;
            
            Time::getCurrentTime( &mCurrentBlockStartTimeSeconds,
                                  &mCurrentBlockStartTimeMilliseconds );
            }
        }
        

    char *sizeString = formatDataSizeWithUnits( mDownloadSizeInBytes );
    
    char *rateString = formatDataSizeWithUnits( (unsigned long)mCurrentRate ); 

    
    // compute the average rate over the entire transfer so far
    long totalMillisecondsSoFar =
        Time::getMillisecondsSince( mDownloadStartTimeSeconds,
                                    mDownloadStartTimeMilliseconds );
    double averageBytePerSecondRate =
        ( (double)((unsigned int)mDownloadedSoFarInBytes - m_unInitialByteCount )/ (double)totalMillisecondsSoFar )
        * 1000;

    char *statusText = NULL;
    if( averageBytePerSecondRate > 0 ) { 
		
		CString	strTemp;
        // compute the ETA using the average rate
        long bytesRemaining = mDownloadSizeInBytes - mDownloadedSoFarInBytes;
        double secondsRemaining = bytesRemaining / averageBytePerSecondRate;

        
        char *timeRemainingString =
            formatTimeIntervalWithUnits( secondsRemaining );
		strTemp = m_ExtStr.LoadString( IDS_DLOAD_TIME_REMAINING_ENG + g_unStringLanguageIdOffset );
        statusText = autoSprintf( (LPCSTR) strTemp,
                                  percentDone,
                                  sizeString,
                                  rateString,
                                  timeRemainingString );
        delete [] timeRemainingString;

		m_unItemState = E_ITEM_ACTIVE;
		m_strStatusText = statusText;
		delete [] statusText;
        }

    delete [] sizeString;
    delete [] rateString;
        
    
    mDownloadStatusLock->unlock();
	
	// fixes a problem when 100% never quite goes to "complete" 
	strTemp = m_ExtStr.LoadString( IDS_DLOAD_ONE_HUNDRED_PERCENT_ENG + g_unStringLanguageIdOffset );
	if( m_strStatusText.Find( strTemp ) >= 0 )
	{		
		m_unItemState = E_ITEM_COMPLETE;
		m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_COMPLETE_ENG + g_unStringLanguageIdOffset );
	}
}

void CMuteDownloadListItem::OnChunkRetry() 
{
    mDownloadStatusLock->lock();
	m_currentQuality--;
    if( m_currentQuality < 0 ) {
        m_currentQuality = 0;
        }
	mDownloadStatusLock->unlock();
}

void CMuteDownloadListItem::OnChunkReceived()
{
	mDownloadStatusLock->lock();
    m_currentQuality++;
    if( m_currentQuality > 20 ) {
        m_currentQuality = 20;
        }
	mDownloadStatusLock->unlock();
}

//*******************************************************************
//  FUNCTION:   -	OnDownloadResult()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteDownloadListItem::OnDownloadResult()
{
	CMuteDownloadsDlg	*pDownloadsDlg;
	char				cValueFound;
	int					nDeleteCanceled;
    
    mDownloadStatusLock->lock();
    mDownloadActive = false;
    
    int result = mDownloadResult;

    // flush and close file
    fclose( mDownloadFILE );
	mDownloadFILE = NULL;

    mDownloadStatusLock->unlock();
    
    char complete = false;
    
	m_bIsBeingResumed = false;
    switch( result ) 
	{
        case MUTE_SHARE_FILE_NOT_FOUND:            
			m_unItemState = E_ITEM_NOT_FOUND;
			m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_NOT_FOUND_ENG + g_unStringLanguageIdOffset );
			m_bIsBeingResumed = true; // setup for a resume
			break;
        case MUTE_SHARE_FILE_TRANSFER_FAILED:
            m_unItemState = E_ITEM_FAILED;
			m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_FAILED_ENG + g_unStringLanguageIdOffset );
			m_bIsBeingResumed = true; // setup for a resume
            break;            
        case MUTE_SHARE_FILE_TRANSFER_CANCELED:
			m_unItemState = E_ITEM_CANCELED;
            m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_CANCELLED_ENG + g_unStringLanguageIdOffset );
            break;            
        case MUTE_SHARE_FILE_TRANSFER_HASH_MISMATCH:
			m_unItemState = E_ITEM_CORRUPT;
			m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_FILE_CORRUPTED_ENG + g_unStringLanguageIdOffset );
            break;            
        case MUTE_SHARE_FILE_TRANSFER_COMPLETE:
			m_unItemState = E_ITEM_COMPLETE;
			m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_COMPLETE_ENG + g_unStringLanguageIdOffset );
            complete = true;
            break;            
    }

    
    mDownloadStatusLock->lock();
    if( !complete ) 
	{
        // delete partial file
        if( mDownloadFile->exists()  ) 
		{
			if( !m_bIsBeingResumed )
			{
				nDeleteCanceled = SettingsManager::getIntSetting( "deleteCanceled", &cValueFound );
				if( !cValueFound )
				{
					nDeleteCanceled = 1; // default is to delete files that are canceled!
				}
				
				if( 1 == nDeleteCanceled )
				{
					mDownloadFile->remove();
				}
				
				pDownloadsDlg = (CMuteDownloadsDlg *) mParentWindow->GetParent();
				pDownloadsDlg->PostMessage( DOWNLOAD_RESULT_REMOVE_FROM_QUEUE, 0, (LPARAM) this ); // remove from DL queue
			}
			else
			{
				// Re-Add this download to the downloads page, then delete this entry
				if( !(CMuteDownloadListCtrl::m_bAppIsClosing) && !mCanceled )
				{					
					pDownloadsDlg = (CMuteDownloadsDlg *) mParentWindow->GetParent();
					m_unItemState = E_ITEM_QUEUED;
					m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_QUEUED_ENG + g_unStringLanguageIdOffset );
					((CMuteDownloadListCtrl *) mParentWindow)->UpdateInfo(this);					
					pDownloadsDlg->PostMessage( DOWNLOAD_RESULT_SET_TO_QUEUED, 0, (LPARAM) this );						
				}
				else
				{
					nDeleteCanceled = SettingsManager::getIntSetting( "deleteCanceled", &cValueFound );
					if( !cValueFound )
					{
						nDeleteCanceled = 1; // default is to delete files that are canceled!
					}
					
					if( 1 == nDeleteCanceled )
					{
						mDownloadFile->remove();
					}
					pDownloadsDlg = (CMuteDownloadsDlg *) mParentWindow->GetParent();
					pDownloadsDlg->PostMessage( DOWNLOAD_RESULT_REMOVE_FROM_QUEUE, 0, (LPARAM) this ); // remove from DL queue
				}
			}
		}
    }
    else 
	{
		pDownloadsDlg = (CMuteDownloadsDlg *) mParentWindow->GetParent();
		pDownloadsDlg->PostMessage( DOWNLOAD_RESULT_REMOVE_FROM_QUEUE, 0, (LPARAM) this ); // remove from DL queue
        // move file into main directory
        char *partialFileName = mDownloadFile->getFileName();

        char *sharePathString = muteShareGetSharingPath();
        File *shareDirectory = new File( NULL, sharePathString );
        delete [] sharePathString;
        
        File *finalDownloadFile =
            shareDirectory->getChildFile( partialFileName );

        delete shareDirectory;
        delete [] partialFileName;

        
        if( finalDownloadFile != NULL ) {
            // move
            mDownloadFile->copy( finalDownloadFile );
            mDownloadFile->remove();

            delete finalDownloadFile;
            }
	}
        
    delete mDownloadFile;
	mDownloadFile = NULL;

	mDownloadStatusLock->unlock();
}

//*******************************************************************
//  FUNCTION:   -	SetStrings()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-					
//*******************************************************************

void CMuteDownloadListCtrl::SetStrings()
{
	CString		strTemp;
	CExternStr  m_ExtStr;

	int nCol=0;

	LVCOLUMN pColumn;
	pColumn.mask=LVCF_TEXT;

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILENAME_ENG + g_unStringLanguageIdOffset );
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(nCol++,&pColumn);
	strTemp.ReleaseBuffer();

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_STATUS_ENG + g_unStringLanguageIdOffset );
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(nCol++,&pColumn);
	strTemp.ReleaseBuffer();

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_NUMRESUMES_ENG + g_unStringLanguageIdOffset );
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(nCol++,&pColumn);
	strTemp.ReleaseBuffer();

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILETYPE_ENG + g_unStringLanguageIdOffset );
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(nCol++,&pColumn);
	strTemp.ReleaseBuffer();

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_STARTTIME_ENG + g_unStringLanguageIdOffset );
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(nCol++,&pColumn);
	strTemp.ReleaseBuffer();

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_LASTUPDATETIME_ENG + g_unStringLanguageIdOffset );
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(nCol++,&pColumn);
	strTemp.ReleaseBuffer();

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_ROUTEQUALITY_ENG + g_unStringLanguageIdOffset );
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(nCol++,&pColumn);
	strTemp.ReleaseBuffer();

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_HOST_VIP_ENG + g_unStringLanguageIdOffset );
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(nCol++,&pColumn);
	strTemp.ReleaseBuffer();

	m_iColumns = nCol;
	m_vecOldSortAscending.resize(m_iColumns);


	// MENU
	m_RightClkMenu.DeleteMenu(0,MF_BYPOSITION);
	strTemp = m_ExtStr.LoadString( IDS_DLOAD_RT_CLK_MENU_CAPTION_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AddMenuTitle( strTemp );

	m_RightClkMenu.DeleteMenu(MP_CANCEL,MF_BYCOMMAND);
	strTemp = m_ExtStr.LoadString( IDS_DLOAD_RT_CLK_MENU_CANCEL_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AppendMenu(MF_STRING,MP_CANCEL, strTemp );

	m_RightClkMenu.DeleteMenu(MP_CLEAR,MF_BYCOMMAND);
	strTemp = m_ExtStr.LoadString( IDS_DLOAD_RT_CLK_MENU_CLEAR_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AppendMenu(MF_STRING,MP_CLEAR, strTemp );

	m_RightClkMenu.DeleteMenu(MP_CLEAR_COMPLETE,MF_BYCOMMAND);
	strTemp = m_ExtStr.LoadString( IDS_DLOAD_RT_CLK_MENU_CLEARCOMPLETE_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AppendMenu(MF_STRING,MP_CLEAR_COMPLETE, strTemp );

	m_RightClkMenu.DeleteMenu(MP_CLEAR_STALLED,MF_BYCOMMAND);
	strTemp = m_ExtStr.LoadString( IDS_DLOAD_RT_CLK_MENU_REMOVESTALLED_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AppendMenu(MF_STRING,MP_CLEAR_STALLED, strTemp );

	//01-23-2005 JROC --  allow user to push items to next in line in queue
	m_RightClkMenu.DeleteMenu( MP_SEARCH_QUEUE_ITEM_NEXT, MF_BYCOMMAND );
	strTemp = m_ExtStr.LoadString( IDS_DLOAD_RT_CLK_MENU_SEARCH_QUEUE_ITEM_NEXT_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AppendMenu(MF_STRING,MP_SEARCH_QUEUE_ITEM_NEXT, strTemp );
	
	if( !LockList(10000) )
	{
		return;
	}

	CMuteDownloadListItem	*pListItem;
	int nItemID = GetItemCount() - 1;
	while( nItemID >= 0 )
	{
		pListItem = (CMuteDownloadListItem *) GetItemData(nItemID);
		switch( pListItem->m_unItemState )
		{
		case E_ITEM_QUEUED:
			pListItem->m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_QUEUED_ENG + g_unStringLanguageIdOffset );
			break;
		case E_ITEM_QUEUED_SEARCHING:
			pListItem->m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_QUEUED_SEARCHING_ENG + g_unStringLanguageIdOffset );
			break;
		case E_ITEM_ATTEMPTING_RESUME:
			pListItem->m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_ATTEMPTING_RESUME_ENG + g_unStringLanguageIdOffset );
			break;
		case E_ITEM_RESUMING:
			pListItem->m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_RESUMING_ENG + g_unStringLanguageIdOffset );
			break;
		case E_ITEM_FETCHING_INFO:
			pListItem->m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_FETCHINGINFO_ENG + g_unStringLanguageIdOffset );
			break;
		case E_ITEM_CANCELED:
			pListItem->m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_CANCELLED_ENG + g_unStringLanguageIdOffset );
			break;
		case E_ITEM_CANCELLING:
			pListItem->m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_CANCELLING_ENG + g_unStringLanguageIdOffset );
			break;
		case E_ITEM_FAILED:
			pListItem->m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_FAILED_ENG + g_unStringLanguageIdOffset );
			break;
		case E_ITEM_FAILED_TO_WRITE_LOCAL_FILE:
			pListItem->m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_FAILED_TO_WRITE_LOCAL_FILE_ENG + g_unStringLanguageIdOffset );
			break;
		case E_ITEM_ACTIVE:
			//do nothing for this case... the text will fix itself on next update
			break;
		case E_ITEM_CORRUPT:
			pListItem->m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_FILE_CORRUPTED_ENG  + g_unStringLanguageIdOffset );
			break;
		case E_ITEM_NOT_FOUND:
			pListItem->m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_NOT_FOUND_ENG + g_unStringLanguageIdOffset );
			break;
		case E_ITEM_COMPLETE:
			pListItem->m_strStatusText = m_ExtStr.LoadString( IDS_DLOAD_COMPLETE_ENG + g_unStringLanguageIdOffset );
			break;			
		}
		
		SetItemText(nItemID,1, pListItem->m_strStatusText );
		nItemID--;
	}

	UnlockList();
}


