// JROC .. 5/20/2004
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


// MuteUploadListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "MuteUploadListCtrl.h"
#include "ColorNames.h"

#include "MUTE/otherApps/fileSharing/fileShare.h"

#include "minorGems/util/stringUtils.h"
#include <time.h>
#include <stdio.h>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// CMuteUploadListCtrl

BEGIN_MESSAGE_MAP(CMuteUploadListCtrl, CMuleListCtrl)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT,0,0xFFFF,OnToolTipNotify)
	ON_NOTIFY_REFLECT ( NM_CUSTOMDRAW, OnCustomdraw )
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_WM_CONTEXTMENU()
#if defined (_MSC_VER) && (_MSC_VER == 1310)
	// for compiling on Visual Studio 2003
	ON_COMMAND( MP_CANCEL, (AFX_PMSG)MenuCancelClick )
	ON_COMMAND( MP_UP_REMOVE_ALL, (AFX_PMSG)MenuRemoveAll )	
#else
	ON_COMMAND( MP_CANCEL, MenuCancelClick )
	ON_COMMAND( MP_UP_REMOVE_ALL, MenuRemoveAll )	
#endif
END_MESSAGE_MAP()



IMPLEMENT_DYNAMIC(CMuteUploadListCtrl, CMuleListCtrl)
CMuteUploadListCtrl::CMuteUploadListCtrl() 
{
	m_iColumns = 0;
	m_bSetImageList = false;
	//SetGeneralPurposeFind(true);
	m_hSystemImageList = NULL;
	m_bStopUpdating = false;
}

BOOL CMuteUploadListCtrl::PreTranslateMessage(MSG* pMsg) 
{                     
	// relay mouse events to tooltip control
	if (pMsg->message== WM_LBUTTONDOWN || pMsg->message== WM_LBUTTONUP || pMsg->message== WM_MOUSEMOVE)
	{
		try
		{
			UpdateToolTips();
			m_ToolTip.RelayEvent(pMsg);
		}
		catch(...)
		{
		}		
	} 

  return CMuleListCtrl::PreTranslateMessage(pMsg);
}

//*******************************************************************
//  FUNCTION:   -	Init()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteUploadListCtrl::Init()
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

	i = 0;
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILENAME_ENG + g_unStringLanguageIdOffset );
	InsertColumn(i++,strTemp,LVCFMT_LEFT,200);	
	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_STATUS_ENG + g_unStringLanguageIdOffset );
	InsertColumn(i++,strTemp,LVCFMT_LEFT,150);

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_CHUNKSUPLOADED_ENG + g_unStringLanguageIdOffset );
	InsertColumn(i++,strTemp,LVCFMT_LEFT,125);

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILETYPE_ENG + g_unStringLanguageIdOffset );
	InsertColumn(i++,strTemp,LVCFMT_LEFT,100);
	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_DESTINATION_VIP_ENG + g_unStringLanguageIdOffset );
	InsertColumn(i++,strTemp,LVCFMT_LEFT,200);	

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_STARTTIME_ENG + g_unStringLanguageIdOffset );
	InsertColumn(i++,strTemp,LVCFMT_LEFT,130);	

	m_iColumns = i;
	m_vecOldSortAscending.resize(m_iColumns);
}


/*
//**************************************************************************************
//  FUNCTION:	-	MenuCancelClick()
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		
//**************************************************************************************
*/
void CMuteUploadListCtrl::MenuCancelClick(  WPARAM wParam, LPARAM lParam )
{
	if( ::IsWindow( GetParent()->GetSafeHwnd() ) )
	{
		GetParent()->PostMessage( UPLOAD_CANCEL_EVENT, NULL, NULL );			
	}
}

//**************************************************************************************
//  FUNCTION:	-	MenuRemoveAll()
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		Kills all the uploads
//**************************************************************************************
void CMuteUploadListCtrl::MenuRemoveAll(  WPARAM wParam, LPARAM lParam )
{		

	if( !LockList(10000) )
	{
		return;
	}

	if( ::IsWindow( GetParent()->GetSafeHwnd() ) )
	{
		GetParent()->SendMessage( UPLOAD_REMOVE_ALL_EVENT, NULL, NULL );
	}	
	
	UnlockList();
	Invalidate();
}


/*
//**************************************************************************************
//  FUNCTION:	-	Creates the right click menu, so that users can get rid of uploads
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		
//**************************************************************************************
*/
void CMuteUploadListCtrl::CreateMenu()
{	
	CString strTemp;

	m_RightClkMenu.CreatePopupMenu();

	strTemp = m_ExtStr.LoadString( IDS_ULOAD_RT_CLK_MENU_CAPTION_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AddMenuTitle(strTemp, true);

	strTemp = m_ExtStr.LoadString( IDS_ULOAD_RT_CLK_MENU_CANCEL_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AppendMenu(MF_STRING,MP_CANCEL, strTemp, MAKEINTRESOURCE(IDI_CANCEL_ICON) );

	strTemp = m_ExtStr.LoadString( IDS_ULOAD_RT_CLK_MENU_REMOVEALL_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AppendMenu(MF_STRING,MP_UP_REMOVE_ALL, strTemp, MAKEINTRESOURCE(IDI_CLEAR_RESULTS_ICON) );	
}

CMuteUploadListCtrl::~CMuteUploadListCtrl()
{
	m_RightClkMenu.DestroyMenu();
}

// CMuteUploadListCtrl message handlers
/*
//**************************************************************************************
//  FUNCTION:	-	OnContextMenu
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		
//**************************************************************************************
*/
void CMuteUploadListCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (GetSelectionMark() != (-1))
	{
		m_RightClkMenu.EnableMenuItem(MP_CANCEL,MF_ENABLED);
		m_RightClkMenu.EnableMenuItem(MP_UP_REMOVE_ALL,MF_ENABLED);
		m_RightClkMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);		
	}
	else
	{
		if( GetItemCount() > 0 )
		{
			m_RightClkMenu.EnableMenuItem(MP_CANCEL,MF_ENABLED);
			m_RightClkMenu.EnableMenuItem(MP_UP_REMOVE_ALL,MF_ENABLED);
			m_RightClkMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
		}
	}
}



//*******************************************************************
//  FUNCTION:   -	AddUploadItem()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteUploadListCtrl::AddUploadItem( CMuteUploadListItem *pListItem )
{
	
	int i;
	char * szSharePath = NULL;

	// stuff for image lists
	int iImage = GetFileTypeSystemImageIdx(pListItem->m_strFileName, -1, pListItem->m_strFileExt, pListItem->m_szFileType );
	if (!m_bSetImageList && m_hSystemImageList != NULL)
	{
		ASSERT( GetStyle() & LVS_SHAREIMAGELISTS );
		ApplyImageList(m_hSystemImageList);
		m_bSetImageList = true;
	}

	
	pListItem->m_oStartTime = CTime::GetCurrentTime();
	pListItem->m_strStartTime = pListItem->m_oStartTime .Format("%X %x");

	UINT32 itemnr = GetItemCount();	

	szSharePath = muteShareGetSharingPath();
	if( NULL != szSharePath )
	{				
		pListItem->m_strFileName.Replace( szSharePath,"..." );
		delete [] szSharePath;
	}
		
	
	itemnr = InsertItem(LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE,itemnr,(LPCSTR)pListItem->m_strFileName,0,0,iImage,(LPARAM)pListItem);
	i = 1;
	pListItem->m_unItemState = E_UL_ITEM_ACTIVE;
	SetItemText(itemnr,i++, pListItem->m_strStatus );
	SetItemText(itemnr,i++, pListItem->m_strChunkUpload );
	SetItemText(itemnr,i++, pListItem->m_szFileType );	
	SetItemText(itemnr,i++, pListItem->m_strHostVirtualAddress );	
	SetItemText(itemnr,i++, pListItem->m_strStartTime );
}

//*******************************************************************
//  FUNCTION:   -	UpdateInfo()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteUploadListCtrl::UpdateInfo(CMuteUploadListItem * pListItem )
{
	int			i;	
	int			nListCount = GetItemCount();
	int			iCol;
	LVITEM		lvi;	
	
	for( i = 0; i < nListCount; i++ )
	{		
		if( (CMuteUploadListItem *) GetItemData( i ) == pListItem )
		{	
			int nCurrentSortItem;
			iCol = 0;

			// update image
			int iImage = GetFileTypeSystemImageIdx(pListItem->m_strFileName, -1, pListItem->m_strFileExt, pListItem->m_szFileType );
			lvi.iItem = i; // row
			lvi.iSubItem = iCol; // column
			GetItem(&lvi);
			lvi.iImage = iImage;
			SetItem(&lvi);
			//update text

			SetItemText( i,iCol++, pListItem->m_strFileName );
			SetItemText( i,iCol++, pListItem->m_strStatus );
			SetItemText( i,iCol++, pListItem->m_strChunkUpload );
			SetItemText( i,iCol++, pListItem->m_szFileType );
			SetItemText( i,iCol++, pListItem->m_strHostVirtualAddress );					
			SetItemText( i,iCol++, pListItem->m_strStartTime );

			nCurrentSortItem = GetCurrentSortItem();
			if( -1 != nCurrentSortItem )
			{
				SortItems(SortProc, nCurrentSortItem + (!m_vecOldSortAscending[nCurrentSortItem] ? 0:10));			
			}
		}
	}
}


//*******************************************************************
//  FUNCTION:   -	UpdateToolTips()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteUploadListCtrl::UpdateToolTips(void)
{
	CString str;
	CString strTemp;

	CMuteUploadListItem *pListItem;

	if( !m_semListOperations.Lock( 100 ) )
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

	int sel = GetItemUnderMouse(this);
	
	if( (sel >= 0) && (sel < 65535) )
	{
		// build info text and display it
		pListItem = (CMuteUploadListItem *) GetItemData(sel);
	}
	else
	{
		m_ToolTip.DelTool( this, 0 );
		m_semListOperations.Unlock();
		return;		
	}
	
	if( NULL != pListItem)
	{		
		if( !pListItem->m_strFileName.IsEmpty() )
		{
			strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILENAME_ENG + g_unStringLanguageIdOffset );
			str = strTemp;
			str += ": ";
			str += pListItem->m_strFileName;			
			
			str += "\n";
			strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILETYPE_ENG + g_unStringLanguageIdOffset );
			str += strTemp;
			str += ": ";			
			str += pListItem->m_szFileType;


			str += "\n";
			strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_DESTINATION_VIP_ENG + g_unStringLanguageIdOffset );
			str += strTemp;
			str += ": ";
			str += pListItem->m_strHostVirtualAddress;


			str += "\n";
			strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_CHUNKSUPLOADED_ENG + g_unStringLanguageIdOffset );
			str += strTemp;
			str += ": ";
			str += pListItem->m_strChunkUpload;

			
			str += "\n";
			strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_STATUS_ENG + g_unStringLanguageIdOffset );
			str += strTemp;
			str += ": ";
			str += pListItem->m_strStatus;

			str += "\n";
			strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_STARTTIME_ENG + g_unStringLanguageIdOffset );
			str += strTemp;
			str += ": ";			
			str += pListItem->m_strStartTime;
		}		
	}
	else
	{
		m_semListOperations.Unlock();
		return;
	}
	
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
BOOL CMuteUploadListCtrl::OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult)
{
	CString strTemp;
	
	TOOLTIPTEXT *pText = (TOOLTIPTEXT *)pNMH;
	CString		str;

	if( m_bStopUpdating )
	{
		return FALSE;
	}

	int sel = GetItemUnderMouse(this);
	
	if (sel < 0 || sel == 65535)
	{
		return FALSE;
	}

	// build info text and display it
	CMuteUploadListItem *pListItem = (CMuteUploadListItem*) GetItemData(sel);
	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILENAME_ENG + g_unStringLanguageIdOffset );
	str = strTemp;
	str += ": ";
	str += pListItem->m_strFileName;			
	
	str += "\n";
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILETYPE_ENG + g_unStringLanguageIdOffset );
	str += strTemp;
	str += ": ";			
	str += pListItem->m_szFileType;
	
	
	str += "\n";
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_DESTINATION_VIP_ENG + g_unStringLanguageIdOffset );
	str += strTemp;
	str += ": ";
	str += pListItem->m_strHostVirtualAddress;
	
	
	str += "\n";
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_CHUNKSUPLOADED_ENG + g_unStringLanguageIdOffset );
	str += strTemp;
	str += ": ";
	str += pListItem->m_strChunkUpload;
	
	
	str += "\n";
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_STATUS_ENG + g_unStringLanguageIdOffset );
	str += strTemp;
	str += ": ";
	str += pListItem->m_strStatus;
	
	str += "\n";
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_STARTTIME_ENG + g_unStringLanguageIdOffset );
	str += strTemp;
	str += ": ";			
	str += pListItem->m_strStartTime;
	
	m_strToolTip.ReleaseBuffer(); // release old used buffer
	m_strToolTip = str;
	pText->lpszText = m_strToolTip.GetBuffer(1);
	pText->hinst = NULL; // we are not using a resource
	PostMessage(WM_ACTIVATE);
	return TRUE;
}

//*******************************************************************
//  FUNCTION:   -	OnCustomdraw()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteUploadListCtrl::OnCustomdraw(NMHDR* pNMHDR, LRESULT* pResult )
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );

    *pResult = CDRF_DODEFAULT;

	CString strTemp;	

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
		COLORREF	crText = ::GetSysColor(COLOR_WINDOWTEXT);
		UINT32		red = GetRValue(crText);
		UINT32		green = GetGValue(crText);
		UINT32		blue = GetGValue(crText);
		
		CMuteUploadListItem *pListItem = (CMuteUploadListItem *) pLVCD->nmcd.lItemlParam;
		
		// and color code them or something cool...
		if( NULL != pListItem )
		{										
			if( E_UL_ITEM_COMPLETE == pListItem->m_unItemState )
			{
				// if done, make line blue
				blue = 255;
			}
			else if( (E_UL_ITEM_FAILED == pListItem->m_unItemState ) || (E_UL_ITEM_STALLED == pListItem->m_unItemState) )
			{
				// if there was a problem, make row red... 
				red = 255;
			}
			else
			{
				// if currently downloading, make green 
				green = 128;
			}
		}
	
		pLVCD->clrText = RGB(red, green ,blue);
		// have to do this because because this function makes the 
		// MuleListCtrl see this list as Custom Drawn...
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
void CMuteUploadListCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	int	sortItem;
	
	// Item is column clicked
	sortItem = pNMListView->iSubItem;

	// Sort table
	SetSortArrow(sortItem, m_vecOldSortAscending[sortItem]);
	SortItems(SortProc, sortItem + (m_vecOldSortAscending[sortItem] ? 0:10));
	m_vecOldSortAscending[sortItem] = !m_vecOldSortAscending[sortItem]; // toggle
	m_semListOperations.Unlock();
	
	*pResult = 0;
}

//*******************************************************************
//  FUNCTION:   -	SortProc   (static function)
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
int CMuteUploadListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CMuteUploadListItem *pListItem1  = (CMuteUploadListItem *)lParam1;
	CMuteUploadListItem *pListItem2 = (CMuteUploadListItem *)lParam2;

	CString strType1, strType2;
	
	strType1 = pListItem1->m_szFileType;
	strType2 = pListItem2->m_szFileType;

	switch(lParamSort)
	{	
	case 0:  // File name asc
		return pListItem1->m_strFileName.CompareNoCase(pListItem2->m_strFileName);
	case 10: // File name desc
		return pListItem2->m_strFileName.CompareNoCase(pListItem1->m_strFileName);
	case 1: // status asc
		return pListItem1->m_strStatus.CompareNoCase(pListItem2->m_strStatus);
	case 11:// status desc
		return pListItem2->m_strStatus.CompareNoCase(pListItem1->m_strStatus);
	case 2:	 // chunks sent asc
		return ( pListItem1->m_unChunksUploaded > pListItem2->m_unChunksUploaded );
	case 12: // chunks sent desc
		return ( pListItem2->m_unChunksUploaded > pListItem1->m_unChunksUploaded );
	case 3:  // File type asc
		return strType1.CompareNoCase(strType2);
	case 13: // File type desc
		return strType2.CompareNoCase(strType1);
	case 4:  // asc
		return pListItem1->m_strHostVirtualAddress.CompareNoCase(pListItem2->m_strHostVirtualAddress);
	case 14: // desc
		return pListItem2->m_strHostVirtualAddress.CompareNoCase(pListItem1->m_strHostVirtualAddress);	
	case 5: // StartTime asc
		return pListItem1->m_oStartTime > pListItem2->m_oStartTime;
	case 15:// StartTime desc
		return pListItem2->m_oStartTime > pListItem1->m_oStartTime;
	default:
		return 0;
	}
	return 0;
}




//*******************************************************************
//  FUNCTION:   -	OnLvnDeleteallitems()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteUploadListCtrl::OnLvnDeleteallitems(NMHDR *pNMHDR, LRESULT *pResult)
{
	// To suppress subsequent LVN_DELETEITEM notification messages, return TRUE.
	*pResult = TRUE;
}


//*******************************************************************
//  FUNCTION:   -	GetFileTypeSystemImageIdx()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
int CMuteUploadListCtrl::GetFileTypeSystemImageIdx(LPCTSTR pszFilePath, int iLength, /*OUT*/ CString &strExtension, /*OUT*/ char * szFileType )
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

		if( NULL != sfi.szTypeName[0] )
		{
			strcpy( szFileType, sfi.szTypeName );
		}

		return sfi.iIcon;
	}

	SHFILEINFO sfi2;
	sfi2.szTypeName[0] = NULL;
	// 01-22-2005 JROC -- added SHGFI_USEFILEATTRIBUTES to this call, in case the path
	// isn't a complete valid path, that way the type info still works.
	DWORD dwResult2 = SHGetFileInfo(pszFilePath, dwFileAttributes, &sfi2, sizeof(sfi2), SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME);
	if(	0 != dwResult2 )
	{
		if( NULL != sfi2.szTypeName[0] )
		{
			strcpy( szFileType, sfi2.szTypeName );
		}		
	}

	// Return already cached value
	// Elandal: Assumes sizeof(void*) == sizeof(int)
	return (int)vData;
}

//*******************************************************************
//  FUNCTION:   -	SetStrings()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteUploadListCtrl::SetStrings()
{
	CString str;
	CString strTemp;
	int		i = 0;
	/////////////////

	LVCOLUMN pColumn;
	pColumn.mask=LVCF_TEXT;

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILENAME_ENG + g_unStringLanguageIdOffset );
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(i++,&pColumn);
	strTemp.ReleaseBuffer();

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_STATUS_ENG + g_unStringLanguageIdOffset );
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(i++,&pColumn);
	strTemp.ReleaseBuffer();

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_CHUNKSUPLOADED_ENG + g_unStringLanguageIdOffset );
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(i++,&pColumn);
	strTemp.ReleaseBuffer();

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILETYPE_ENG + g_unStringLanguageIdOffset );
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(i++,&pColumn);
	strTemp.ReleaseBuffer();
	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_DESTINATION_VIP_ENG + g_unStringLanguageIdOffset );
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(i++,&pColumn);
	strTemp.ReleaseBuffer();

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_STARTTIME_ENG + g_unStringLanguageIdOffset );
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(i++,&pColumn);
	strTemp.ReleaseBuffer();

	/////////////////////

	// MENU

	m_RightClkMenu.DeleteMenu(0,MF_BYPOSITION);
	strTemp = m_ExtStr.LoadString( IDS_ULOAD_RT_CLK_MENU_CAPTION_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AddMenuTitle(strTemp);

	m_RightClkMenu.DeleteMenu(MP_CANCEL,MF_BYCOMMAND);
	strTemp = m_ExtStr.LoadString( IDS_ULOAD_RT_CLK_MENU_CANCEL_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AppendMenu(MF_STRING,MP_CANCEL, strTemp );

	m_RightClkMenu.DeleteMenu(MP_UP_REMOVE_ALL,MF_BYCOMMAND);
	strTemp = m_ExtStr.LoadString( IDS_ULOAD_RT_CLK_MENU_REMOVEALL_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AppendMenu(MF_STRING,MP_UP_REMOVE_ALL, strTemp );

	if( !LockList(10000) )
	{
		return;
	}

	CMuteUploadListItem	*pListItem;
	int nItemID = GetItemCount() - 1;
	while( nItemID >= 0 )
	{
		pListItem = (CMuteUploadListItem *) GetItemData(nItemID);
		switch( pListItem->m_unItemState )
		{		
		case E_UL_ITEM_STARTINGUP:
			pListItem->m_strStatus = m_ExtStr.LoadString( IDS_ULOAD_LIST_STARTINGUP_ENG + g_unStringLanguageIdOffset );
			break;
		case E_UL_ITEM_ACTIVE:
			//do nothing for this case... the text will fix itself on next update
			break;
		case E_UL_ITEM_COMPLETE:
			pListItem->m_strStatus = m_ExtStr.LoadString( IDS_ULOAD_STATUS_DONE_ENG + g_unStringLanguageIdOffset );
			break;
		case E_UL_ITEM_STALLED:
			pListItem->m_strStatus = m_ExtStr.LoadString( IDS_ULOAD_STATUS_STALLED_ENG + g_unStringLanguageIdOffset );
			break;
		case E_UL_ITEM_FAILED:
			pListItem->m_strStatus = m_ExtStr.LoadString( IDS_ULOAD_STATUS_FAILED_ENG + g_unStringLanguageIdOffset );
			break;		
		}		

		SetItemText(nItemID,1, pListItem->m_strStatus );
		nItemID--;
	}

	UnlockList();
}


