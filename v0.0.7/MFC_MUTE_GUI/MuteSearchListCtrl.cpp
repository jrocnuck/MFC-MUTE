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
#include "MuteSearchListCtrl.h"
#include "MuteSearchDlg.h"
#include "ColorNames.h"
#include "ExternString.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// CMuteSearchListCtrl

IMPLEMENT_DYNAMIC(CMuteSearchListCtrl, CMuleListCtrl)
CMuteSearchListCtrl::CMuteSearchListCtrl() 
{
	m_iColumns = 0;
	m_bSetImageList = false;
	//SetGeneralPurposeFind(true);
	m_hSystemImageList = NULL;
}

//*******************************************************************
//  FUNCTION:   -	PreTranslateMessage()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
BOOL CMuteSearchListCtrl::PreTranslateMessage(MSG* pMsg) 
{                     
	// relay mouse events to tooltip control
	if (pMsg->message== WM_LBUTTONDOWN || pMsg->message== WM_LBUTTONUP || pMsg->message== WM_MOUSEMOVE)
	{
		UpdateToolTips();
		m_ToolTip.RelayEvent(pMsg);
	} 

  return CMuleListCtrl::PreTranslateMessage(pMsg);
}

//*******************************************************************
//  FUNCTION:   -	Init()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteSearchListCtrl::Init()
{
	int		i;
	CString	str;

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
	str = m_ExtStr.LoadString( IDS_COL_HEADING_FILENAME_ENG + g_unStringLanguageIdOffset );
	InsertColumn(i++,str,LVCFMT_LEFT,300);

	str = m_ExtStr.LoadString( IDS_COL_HEADING_FILETYPE_ENG + g_unStringLanguageIdOffset );
	InsertColumn(i++,str,LVCFMT_LEFT,100);

	str = m_ExtStr.LoadString( IDS_COL_HEADING_SIZE_ENG + g_unStringLanguageIdOffset );
	InsertColumn(i++,str,LVCFMT_LEFT,75);

	str = m_ExtStr.LoadString( IDS_COL_HEADING_HASH_ENG + g_unStringLanguageIdOffset );	
	InsertColumn(i++,str,LVCFMT_LEFT,200);

	str = m_ExtStr.LoadString( IDS_COL_HEADING_HOST_VIP_ENG + g_unStringLanguageIdOffset );	
	InsertColumn(i++,str,LVCFMT_LEFT,200);
	m_iColumns = i;
	m_vecOldSortAscending.resize(m_iColumns);
}

CMuteSearchListCtrl::~CMuteSearchListCtrl()
{
	m_RightClkMenu.DestroyMenu();
}


BEGIN_MESSAGE_MAP(CMuteSearchListCtrl, CMuleListCtrl)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT,0,0xFFFF,OnToolTipNotify)
	ON_NOTIFY_REFLECT ( NM_CUSTOMDRAW, OnCustomdraw )
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclkSearchlist)	
#if defined (_MSC_VER) && (_MSC_VER == 1310)
	// compiling under Visual Studio 2003
	ON_COMMAND( MUTE_SEARCH_MENU_DOWNLOAD, (AFX_PMSG)SearchDownloadClick )
#else
	ON_COMMAND( MUTE_SEARCH_MENU_DOWNLOAD, SearchDownloadClick )
#endif
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

// CMuteSearchListCtrl message handlers

/*
//**************************************************************************************
//  FUNCTION:	-	SearchDownloadClick()
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		If a right click menu is popped up over an item and the user clicks
//					the download item, this is called for the list.
//**************************************************************************************
*/
void CMuteSearchListCtrl::SearchDownloadClick(  WPARAM wParam, LPARAM lParam )
{
	if( !m_semListOperations.Lock(1500) )
	{
		return;
	}

	if (GetSelectionMark() != (-1))
	{
		m_semListOperations.Unlock();
		((CMuteSearchDlg *)GetParent())->OnDownloadButton();	
	}
	else
	{
		m_semListOperations.Unlock();
	}
}

/*
//**************************************************************************************
//  FUNCTION:	-	Creates the right click menu, so that users can download.
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		
//**************************************************************************************
*/
void CMuteSearchListCtrl::CreateMenu()
{
	CString	strTemp;
	m_RightClkMenu.CreatePopupMenu();
	strTemp = m_ExtStr.LoadString( IDS_SEARCH_RT_CLK_MENU_CAPTION_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AddMenuTitle( strTemp, true );

	strTemp = m_ExtStr.LoadString( IDS_SEARCH_RT_CLK_MENU_DLOAD_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AppendMenu(MF_STRING,MUTE_SEARCH_MENU_DOWNLOAD, strTemp, MAKEINTRESOURCE(IDI_DOWNLOAD3_ICON) );	
}

/*
//**************************************************************************************
//  FUNCTION:	-	OnContextMenu
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		When a user right clicks on the search list, this call back comes up
//					and if an item is selected, the menu will give the option to download.
//**************************************************************************************
*/
void CMuteSearchListCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if( !m_semListOperations.Lock(1500) )
	{
		return;
	}

	if (GetSelectionMark() != (-1))
	{
		m_RightClkMenu.EnableMenuItem(MUTE_SEARCH_MENU_DOWNLOAD,MF_ENABLED);
		m_RightClkMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
	}
	
	m_semListOperations.Unlock();
}


//*******************************************************************
//  FUNCTION:   -	AddSearchResult()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteSearchListCtrl::AddSearchResult( CMuteSearchListItem *pListItem )
{
	CString		str;
	int			i;
	int			nCurrentSortItem;

	if( !m_semListOperations.Lock(1500) )
	{
		return;
	}
	
	// stuff for image lists
	int iImage = GetFileTypeSystemImageIdx(pListItem->m_strFileName, -1, pListItem->m_strFileExt, pListItem->m_strFileType );
	pListItem->m_strFileExt.MakeLower( );
	if (!m_bSetImageList && m_hSystemImageList != NULL)
	{
		ASSERT( GetStyle() & LVS_SHAREIMAGELISTS );
		ApplyImageList(m_hSystemImageList);
		m_bSetImageList = true;
	}

	
	UINT32 itemnr = GetItemCount();
		
	itemnr = InsertItem(LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE,itemnr,(LPCSTR)pListItem->m_strFileName,0,0,iImage,(LPARAM)pListItem);
	
	i = 1;
	SetItemText(itemnr,i++, pListItem->m_strFileType);
	SetItemText(itemnr,i++, pListItem->m_strFileSize);
	SetItemText(itemnr,i++, pListItem->m_strHash );
	SetItemText(itemnr,i++, pListItem->m_strHostVirtualAddress );
	
	nCurrentSortItem = GetCurrentSortItem();
	if( -1 != nCurrentSortItem )
	{
		SortItems(SortProc, nCurrentSortItem + (!m_vecOldSortAscending[nCurrentSortItem] ? 0:10));			
	}
	m_semListOperations.Unlock();
}


//*******************************************************************
//  FUNCTION:   -	UpdateToolTips()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteSearchListCtrl::UpdateToolTips(void)
{
	CString str;
	CString	strTemp;

	int sel = GetItemUnderMouse(this);
	
	if( GetItemCount() <= 0 )
	{
		m_ToolTip.DelTool( this, 0 );
	}

	if (sel < 0 || sel == 65535)
	{
		m_ToolTip.DelTool( this, 0 );
		return;
	}

	// build info text and display it
	CMuteSearchListItem *pListItem = (CMuteSearchListItem *) GetItemData(sel);
	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILENAME_ENG + g_unStringLanguageIdOffset );
	str = strTemp + ": ";
	str += pListItem->m_strFileName;
	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILETYPE_ENG + g_unStringLanguageIdOffset );
	str += "\n";
	str += strTemp;
	str += ": ";
	str += pListItem->m_strFileType;

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_SIZE_ENG + g_unStringLanguageIdOffset );
	str += "\n";
	str += strTemp;
	str += ": ";
	str += pListItem->m_strFileSize;

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_HASH_ENG + g_unStringLanguageIdOffset );	
	str += "\n";
	str += strTemp;
	str += ": ";
	str += pListItem->m_strHash;
	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_HOST_VIP_ENG + g_unStringLanguageIdOffset );	
	str += "\n";
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
}

//*******************************************************************
//  FUNCTION:   -	OnToolTipNotify()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
BOOL CMuteSearchListCtrl::OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult)
{
	CString		strTemp;
	TOOLTIPTEXT *pText = (TOOLTIPTEXT *)pNMH;
	CString		str;

	
	int sel = GetItemUnderMouse(this);
	
	if (sel < 0 || sel == 65535)
	{
			return FALSE;
	}

	// build info text and display it
	CMuteSearchListItem *pListItem = (CMuteSearchListItem *) GetItemData(sel);
	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILENAME_ENG + g_unStringLanguageIdOffset );
	str = strTemp + ": ";
	str += pListItem->m_strFileName;
	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILETYPE_ENG + g_unStringLanguageIdOffset );
	str += "\n";
	str += strTemp;
	str += ": ";
	str += pListItem->m_strFileType;

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_SIZE_ENG + g_unStringLanguageIdOffset );
	str += "\n";
	str += strTemp;
	str += ": ";
	str += pListItem->m_strFileSize;

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_HASH_ENG + g_unStringLanguageIdOffset );	
	str += "\n";
	str += strTemp;
	str += ": ";
	str += pListItem->m_strHash;
	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_HOST_VIP_ENG + g_unStringLanguageIdOffset );	
	str += "\n";
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

//*******************************************************************
//  FUNCTION:   -	OnCustomdraw
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteSearchListCtrl::OnCustomdraw(NMHDR* pNMHDR, LRESULT* pResult )
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );

    *pResult = CDRF_DODEFAULT;

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
		
		CMuteSearchListItem *pListItem = (CMuteSearchListItem *) pLVCD->nmcd.lItemlParam;
		// todo.. obviously come up with lots of extensions that are known
		// and color code them or something cool...
		if( NULL == pListItem )
		{
			return;
		}

		if( !pListItem->m_strFileExt.IsEmpty() )
		{	
			if( ( 0 == pListItem->m_strFileExt.CompareNoCase("mp3") ) || 
				( 0 == pListItem->m_strFileExt.CompareNoCase("wav") ) ||
				( 0 == pListItem->m_strFileExt.CompareNoCase("ogg") ) ||
				( 0 == pListItem->m_strFileExt.CompareNoCase("wma") )
			  )
			{
				red = 255;
			}
			else if( ( 0 == pListItem->m_strFileExt.CompareNoCase("avi") ) ||
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("mpg") ) ||
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("mpeg") ) ||
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("mov") ) ||
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("asf") ) ||
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("wmv") ) ||
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("asf") ) ||
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("gif") ) ||
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("jpg") ) ||
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("bmp") ) ||
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("png") ) ||
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("tif") )
				   )
			{
				green = 128;
			}
			else if( ( 0 == pListItem->m_strFileExt.CompareNoCase("pdf") ) ||
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("xml") ) ||	 
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("csv") ) ||	 
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("htm") ) ||	 
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("html") ) ||	 
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("chm") ) ||
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("doc") ) ||
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("xls") ) ||
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("ppt") ) ||
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("mdb") ) ||
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("txt") )
				   )
			{
				blue = 255;
			}
			else if( ( 0 == pListItem->m_strFileExt.CompareNoCase("zip") ) ||
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("rar") ) ||
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("gz" ) ) ||
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("tar") ) ||
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("ace") ) ||
					 ( 0 == pListItem->m_strFileExt.CompareNoCase("arj") )
				   )
			{
				red = 45;
				blue = 150;
				green = 100;
			}
		}

		
		if( pListItem->m_bIsDownloading )
		{
			// highlight the background of an item when it is downloaded.
			pLVCD->clrTextBk = colLemonChiffon; // comes from colornames.h
		}
		else
		{
			// if not downloading make background color normal.
			pLVCD->clrTextBk = ::GetSysColor(COLOR_WINDOW);
		}	
		
		
		pLVCD->clrText = RGB(red, green ,blue);
        *pResult = CDRF_DODEFAULT;
     }
}

//*******************************************************************
//  FUNCTION:   -	DrawItem
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
/*
void CMuteSearchListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!lpDrawItemStruct->itemData)
		return;
}
*/


//*******************************************************************
//  FUNCTION:   -	OnColumnClick
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteSearchListCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	int	sortItem;
	
	// Item is column clicked
	sortItem = pNMListView->iSubItem;

	// Sort table
	if( m_semListOperations.Lock(1500) )
	{
		SetSortArrow(sortItem, m_vecOldSortAscending[sortItem]);
		SortItems(SortProc, sortItem + (m_vecOldSortAscending[sortItem] ? 0:10));
		m_vecOldSortAscending[sortItem] = !m_vecOldSortAscending[sortItem]; // toggle
		m_semListOperations.Unlock();
	}
	*pResult = 0;
}

//*******************************************************************
//  FUNCTION:   -	OnNMDblclkSearchlist()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteSearchListCtrl::OnNMDblclkSearchlist(NMHDR *pNMHDR, LRESULT *pResult) 
{
	int iSel = GetSelectionMark();
	if (iSel != -1)
	{
		//CMuteSearchListItem *pListItem = (CMuteSearchListItem *)this->GetItemData(iSel);
		((CMuteSearchDlg *)GetParent())->OnDownloadButton();
	}
	
	*pResult = 0;
}


//*******************************************************************
//  FUNCTION:   -	SortProc()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
int CMuteSearchListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CMuteSearchListItem *pListItem1  = (CMuteSearchListItem *)lParam1;
	CMuteSearchListItem *pListItem2 = (CMuteSearchListItem *)lParam2;

	switch(lParamSort)
	{
	case 0:  // filename asc
		return pListItem1->m_strFileName.CompareNoCase(pListItem2->m_strFileName);
	case 10: // filename desc
		return pListItem2->m_strFileName.CompareNoCase(pListItem1->m_strFileName);
	case 1:	 // file type asc
		return pListItem1->m_strFileType.CompareNoCase(pListItem2->m_strFileType);
	case 11: // file type desc
		return pListItem2->m_strFileType.CompareNoCase(pListItem1->m_strFileType);
	case 2:	 // file size asc
		return ( pListItem1->m_unFileSize > pListItem2->m_unFileSize );
	case 12: // file size desc
		return ( pListItem2->m_unFileSize > pListItem1->m_unFileSize );
	case 3: // hash asc
		return pListItem1->m_strHash.CompareNoCase(pListItem2->m_strHash);
	case 13:// hash desc
		return pListItem2->m_strHash.CompareNoCase(pListItem1->m_strHash);
	case 4: // virtual address asc
		return pListItem1->m_strHostVirtualAddress.CompareNoCase(pListItem2->m_strHostVirtualAddress);
	case 14:// virtual address desc
		return pListItem2->m_strHostVirtualAddress.CompareNoCase(pListItem1->m_strHostVirtualAddress);
	default:
		return 0;
	}
	return 0;
}


//*******************************************************************
//  FUNCTION:   -	OnLvnDeleteallitems
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteSearchListCtrl::OnLvnDeleteallitems(NMHDR *pNMHDR, LRESULT *pResult)
{
	// To suppress subsequent LVN_DELETEITEM notification messages, return TRUE.
	*pResult = TRUE;
}


//*******************************************************************
//  FUNCTION:   -	GetFileTypeSystemImageIdx()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	Retrieves small icon for a given file based on the
//					user's system setup.
//*******************************************************************
int CMuteSearchListCtrl::GetFileTypeSystemImageIdx(LPCTSTR pszFilePath, int iLength, /*OUT*/ CString &strExtension, /*OUT*/CString &strFileType )
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
									   SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_TYPENAME );
		if (dwResult == 0)
			return 0;
		
		ASSERT( m_hSystemImageList == NULL || m_hSystemImageList == (HIMAGELIST)dwResult );
		strFileType = sfi.szTypeName;
		m_hSystemImageList = (HIMAGELIST)dwResult;

		// Store icon index in local cache		
		m_aExtToSysImgIdx.SetAt(pszCacheExt, (LPVOID)sfi.iIcon);		
		// This works for search since we clear the complete list each time
		// we do a new search
		m_aExtToSysFileType.SetAt( pszCacheExt, (LPVOID) &strFileType );
		return sfi.iIcon;
	}

	// This works for search since we clear the complete list each time
	// we do a new search
	LPVOID vData2;
	CString * pStr;
	if( m_aExtToSysFileType.Lookup( pszCacheExt, vData2) )
	{
		pStr = (CString *) vData2;
		strFileType = (LPCSTR) (*pStr);		
	}
	
	// Return already cached value
	// Elandal: Assumes sizeof(void*) == sizeof(int)
	return (int)vData;
}


/*
//*******************************************************************
//  FUNCTION:    -	SetStrings()
//  RETURNS:     -	
//  PARAMETERS:  -	
//  COMMENTS:	 -	
//*******************************************************************
*/
void CMuteSearchListCtrl::SetStrings()
{
	CString strTemp;
	int	i = 0;

	CExternStr  m_ExtStr;

	LVCOLUMN pColumn;
	pColumn.mask=LVCF_TEXT;
	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILENAME_ENG + g_unStringLanguageIdOffset );
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(i++,&pColumn);
	strTemp.ReleaseBuffer();

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILETYPE_ENG + g_unStringLanguageIdOffset );
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(i++,&pColumn);
	strTemp.ReleaseBuffer();

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_SIZE_ENG + g_unStringLanguageIdOffset );
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(i++,&pColumn);
	strTemp.ReleaseBuffer();

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_HASH_ENG + g_unStringLanguageIdOffset );	
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(i++,&pColumn);
	strTemp.ReleaseBuffer();

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_HOST_VIP_ENG + g_unStringLanguageIdOffset );	
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(i++,&pColumn);
	strTemp.ReleaseBuffer();

	//m_iColumns = i;
	//m_vecOldSortAscending.resize(m_iColumns);


	////////////////
	//MENU

	m_RightClkMenu.DeleteMenu(0,MF_BYPOSITION);
	strTemp = m_ExtStr.LoadString( IDS_SEARCH_RT_CLK_MENU_CAPTION_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AddMenuTitle( strTemp );

	m_RightClkMenu.DeleteMenu(1,MF_BYPOSITION);
	strTemp = m_ExtStr.LoadString( IDS_SEARCH_RT_CLK_MENU_DLOAD_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AppendMenu(MF_STRING,MUTE_SEARCH_MENU_DOWNLOAD, strTemp );
	
	UpdateToolTips();
}
