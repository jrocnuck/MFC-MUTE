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
#include "MuteSharedFilesListCtrl.h"
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


// CMuteSharedFilesListCtrl

BEGIN_MESSAGE_MAP(CMuteSharedFilesListCtrl, CMuleListCtrl)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT,0,0xFFFF,OnToolTipNotify)
	ON_NOTIFY_REFLECT ( NM_CUSTOMDRAW, OnCustomdraw )
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
END_MESSAGE_MAP()



IMPLEMENT_DYNAMIC(CMuteSharedFilesListCtrl, CMuleListCtrl)
CMuteSharedFilesListCtrl::CMuteSharedFilesListCtrl() 
{
	m_iColumns = 0;
	m_bSetImageList = false;
	//SetGeneralPurposeFind(true);
	m_hSystemImageList = NULL;
	m_bStopUpdating = false;
}

BOOL CMuteSharedFilesListCtrl::PreTranslateMessage(MSG* pMsg) 
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
void CMuteSharedFilesListCtrl::Init()
{
	int		i;
	CString	strTemp;

	SetExtendedStyle(LVS_EX_FULLROWSELECT);
	ModifyStyle(LVS_SINGLESEL,0); // IF YOU WANT IT TO BE SINGLE SELECT REM THIS LINE OUT...

	m_ToolTip.Create(this);
	m_ToolTip.ModifyStyle(0, TTS_NOPREFIX);
	m_ToolTip.SetDelayTime(TTDT_AUTOPOP, 7000);
	m_ToolTip.SetDelayTime(TTDT_INITIAL, 2000);
	m_ToolTip.SendMessage(TTM_SETMAXTIPWIDTH, 0, SHRT_MAX); // recognize \n chars!
    m_ToolTip.Activate(TRUE);
	m_ToolTip.SetTipTextColor( colBlue );  

	i = 0;
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILENAME_ENG + g_unStringLanguageIdOffset );
	InsertColumn(i++,strTemp,LVCFMT_LEFT,300);	
	
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILETYPE_ENG + g_unStringLanguageIdOffset );
	InsertColumn(i++,strTemp,LVCFMT_LEFT,100);

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_HASH_ENG + g_unStringLanguageIdOffset );	
	InsertColumn(i++,strTemp,LVCFMT_LEFT,300);	

	m_iColumns = i;
	m_vecOldSortAscending.resize(m_iColumns);
}




CMuteSharedFilesListCtrl::~CMuteSharedFilesListCtrl()
{
}

// CMuteSharedFilesListCtrl message handlers

//*******************************************************************
//  FUNCTION:   -	AddUploadItem()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteSharedFilesListCtrl::AddSharedFileItem( CMuteSharedFileListItem *pListItem )
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

	UINT32 itemnr = GetItemCount();	

	szSharePath = muteShareGetSharingPath();
	if( NULL != szSharePath )
	{				
		pListItem->m_strFileName.Replace( szSharePath,"..." );
		delete [] szSharePath;
	}
	
	itemnr = InsertItem(LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE,itemnr,(LPCSTR)pListItem->m_strFileName,0,0,iImage,(LPARAM)pListItem);
	i = 1;

	SetItemText(itemnr,i++, pListItem->m_szFileType );
	SetItemText(itemnr,i++, pListItem->m_strHash );
}

//*******************************************************************
//  FUNCTION:   -	UpdateInfo()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteSharedFilesListCtrl::UpdateInfo(CMuteSharedFileListItem * pListItem )
{
	int			i;	
	int			nListCount = GetItemCount();
	int			iCol;
	LVITEM		lvi;	
	
	for( i = 0; i < nListCount; i++ )
	{		
		if( (CMuteSharedFileListItem *) GetItemData( i ) == pListItem )
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

			SetItemText( i,iCol++, pListItem->m_strFileExt );
			SetItemText( i,iCol++, pListItem->m_strHash );
			
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
void CMuteSharedFilesListCtrl::UpdateToolTips(void)
{
	CString str;
	CString strTemp;

	CMuteSharedFileListItem *pListItem;

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
		pListItem = (CMuteSharedFileListItem *) GetItemData(sel);
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
			strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_HASH_ENG + g_unStringLanguageIdOffset );
			str += strTemp;
			str += ": ";			
			str += pListItem->m_strHash;
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
BOOL CMuteSharedFilesListCtrl::OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult)
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
	CMuteSharedFileListItem *pListItem = (CMuteSharedFileListItem*) GetItemData(sel);
	
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
	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_HASH_ENG + g_unStringLanguageIdOffset );
	str += strTemp;
	str += ": ";			
	str += pListItem->m_strHash;
	str += "\n";

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
void CMuteSharedFilesListCtrl::OnCustomdraw(NMHDR* pNMHDR, LRESULT* pResult )
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
		
		CMuteSharedFileListItem *pListItem = (CMuteSharedFileListItem *) pLVCD->nmcd.lItemlParam;
		
		if( NULL == pListItem )
		{
			return;
		}
		// and color code them or something cool...
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
void CMuteSharedFilesListCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult)
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
int CMuteSharedFilesListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CMuteSharedFileListItem *pListItem1  = (CMuteSharedFileListItem *)lParam1;
	CMuteSharedFileListItem *pListItem2 = (CMuteSharedFileListItem *)lParam2;

	CString strType1, strType2;
	
	strType1 = pListItem1->m_szFileType;
	strType2 = pListItem2->m_szFileType;

	switch(lParamSort)
	{	
	case 0:  // File name asc
		return pListItem1->m_strFileName.CompareNoCase(pListItem2->m_strFileName);
	case 10: // File name desc
		return pListItem2->m_strFileName.CompareNoCase(pListItem1->m_strFileName);
	case 1:  // File type asc
		return strType1.CompareNoCase(strType2);
	case 11: // File type desc
		return strType2.CompareNoCase(strType1);
	case 2:  // asc
		return pListItem1->m_strHash.CompareNoCase(pListItem2->m_strHash);
	case 12: // desc
		return pListItem2->m_strHash.CompareNoCase(pListItem1->m_strHash);	
	default:
		return 0;
	}
	return 0;
}

//*******************************************************************
//  FUNCTION:   -	GetFileTypeSystemImageIdx()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
int CMuteSharedFilesListCtrl::GetFileTypeSystemImageIdx(LPCTSTR pszFilePath, int iLength, /*OUT*/ CString &strExtension, /*OUT*/ char * szFileType )
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
void CMuteSharedFilesListCtrl::SetStrings()
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

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_FILETYPE_ENG + g_unStringLanguageIdOffset );
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(i++,&pColumn);
	strTemp.ReleaseBuffer();

	strTemp = m_ExtStr.LoadString( IDS_COL_HEADING_HASH_ENG + g_unStringLanguageIdOffset );
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(i++,&pColumn);
	strTemp.ReleaseBuffer();
}
/*
//**************************************************************************************
//  FUNCTION:	-	Creates the right click menu, so that users can 
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		
//**************************************************************************************
*/
/*void CMuteSharedFilesListCtrl::CreateMenu()
{
	CString	strTemp;
	m_RightClkMenu.CreatePopupMenu();
	strTemp = m_ExtStr.LoadString( IDS_SHAREDFILES_RT_CLK_MENU_CAPTION_ENG + g_unStringLanguageIdOffset );
	m_RightClkMenu.AddMenuTitle( strTemp, true );

	//strTemp = m_ExtStr.LoadString( IDS_SEARCH_RT_CLK_MENU_DLOAD_ENG + g_unStringLanguageIdOffset );
	//m_RightClkMenu.AppendMenu(MF_STRING,MUTE_SEARCH_MENU_DOWNLOAD, strTemp, MAKEINTRESOURCE(IDI_DOWNLOAD3_ICON) );	
}
*/
