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


// SearchListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "MUTEConnectionListCtrl.h"
#include <winsock2.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// CMUTEConnectionListCtrl

IMPLEMENT_DYNAMIC(CMUTEConnectionListCtrl, CMuleListCtrl)
CMUTEConnectionListCtrl::CMUTEConnectionListCtrl() 
{
	m_iColumns = 0;
}

//*******************************************************************
//  FUNCTION:   -	Init()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMUTEConnectionListCtrl::Init()
{
	CString strTemp;
	int		nColIndex = 0;	
	
	strTemp = m_ExtStr.LoadString( IDS_CONNECTIONS_COLHEADING_ADDRESS_ENG + g_unStringLanguageIdOffset );
	InsertColumn( nColIndex++, strTemp, LVCFMT_LEFT, 200 );
	
	strTemp = m_ExtStr.LoadString( IDS_CONNECTIONS_COLHEADING_PORT_ENG + g_unStringLanguageIdOffset );
	InsertColumn( nColIndex++, strTemp,	LVCFMT_LEFT, 75 );
	
	strTemp = m_ExtStr.LoadString( IDS_CONNECTIONS_COLHEADING_SENT_ENG + g_unStringLanguageIdOffset );
	InsertColumn( nColIndex++, strTemp,	LVCFMT_LEFT, 75 );
	
	strTemp = m_ExtStr.LoadString( IDS_CONNECTIONS_COLHEADING_QUEUED_ENG + g_unStringLanguageIdOffset );
    InsertColumn( nColIndex++, strTemp,	LVCFMT_LEFT, 75 );
	
	strTemp = m_ExtStr.LoadString( IDS_CONNECTIONS_COLHEADING_DROPPED_ENG + g_unStringLanguageIdOffset );
    InsertColumn( nColIndex++, strTemp, LVCFMT_LEFT, 75 );

	m_iColumns = nColIndex;
	m_vecOldSortAscending.resize(m_iColumns);

}

CMUTEConnectionListCtrl::~CMUTEConnectionListCtrl()
{
}


BEGIN_MESSAGE_MAP(CMUTEConnectionListCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
END_MESSAGE_MAP()

// CMUTEConnectionListCtrl message handlers

//*******************************************************************
//  FUNCTION:   -	AddConnectionItem()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMUTEConnectionListCtrl::AddConnectionItem( CMUTEConnectionListItem *pListItem )
{
	
	UINT32 itemnr = GetItemCount();
	CString	strTemp;
		
	itemnr = InsertItem(LVIF_TEXT|LVIF_PARAM,itemnr,(LPCSTR)pListItem->m_strAddress,0,0,0,(LPARAM)pListItem);
	strTemp.Format( "%d", pListItem->m_unPort );
	SetItemText(itemnr,1, strTemp);
	strTemp.Format( "%d", pListItem->m_unSent );
	SetItemText(itemnr,2, strTemp);
	strTemp.Format( "%d", pListItem->m_unQueued );
	SetItemText(itemnr,3, strTemp);
	strTemp.Format( "%d", pListItem->m_unDropped );
	SetItemText(itemnr,4, strTemp);
}



//*******************************************************************
//  FUNCTION:   -	OnColumnClick
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMUTEConnectionListCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult)
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
int CMUTEConnectionListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	ULONG	ulAddress1, ulAddress2;
	CMUTEConnectionListItem *pListItem1  = (CMUTEConnectionListItem *)lParam1;
	CMUTEConnectionListItem *pListItem2 = (CMUTEConnectionListItem *)lParam2;
	
	ulAddress1 = inet_addr( pListItem1->m_strAddress );
	ulAddress2 = inet_addr( pListItem2->m_strAddress );
	switch(lParamSort)
	{
	case 0:  // asc
		return ulAddress1 > ulAddress2;
	case 10: // desc
		return ulAddress2 > ulAddress1;
	case 1:	 // asc
		return pListItem1->m_unPort > pListItem2->m_unPort;
	case 11: // desc
		return pListItem2->m_unPort > pListItem1->m_unPort;
	case 2: // asc
		return pListItem1->m_unSent > pListItem2->m_unSent;
	case 12:// desc
		return pListItem2->m_unSent > pListItem1->m_unSent;
	case 3: // asc
		return pListItem1->m_unQueued > pListItem2->m_unQueued;
	case 13:// desc
		return pListItem2->m_unQueued > pListItem1->m_unQueued;
	case 4:// asc
		return pListItem1->m_unDropped > pListItem2->m_unDropped;
	case 14: // desc
		return pListItem2->m_unDropped > pListItem1->m_unDropped;	
	default:
		return 0;
	}
	return 0;
}

void CMUTEConnectionListCtrl::OnLvnDeleteallitems(NMHDR *pNMHDR, LRESULT *pResult)
{
	// To suppress subsequent LVN_DELETEITEM notification messages, return TRUE.
	*pResult = TRUE;
}

//*******************************************************************
//  FUNCTION:   -	SetStrings()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMUTEConnectionListCtrl::SetStrings()
{
	CString strTemp;
	int		nColIndex = 0;	

	LVCOLUMN pColumn;
	pColumn.mask=LVCF_TEXT;

	strTemp = m_ExtStr.LoadString( IDS_CONNECTIONS_COLHEADING_ADDRESS_ENG + g_unStringLanguageIdOffset );
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(nColIndex++,&pColumn);
	strTemp.ReleaseBuffer();
	
	strTemp = m_ExtStr.LoadString( IDS_CONNECTIONS_COLHEADING_PORT_ENG + g_unStringLanguageIdOffset );
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(nColIndex++,&pColumn);
	strTemp.ReleaseBuffer();
	
	strTemp = m_ExtStr.LoadString( IDS_CONNECTIONS_COLHEADING_SENT_ENG + g_unStringLanguageIdOffset );
	pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(nColIndex++,&pColumn);
	strTemp.ReleaseBuffer();
	
	strTemp = m_ExtStr.LoadString( IDS_CONNECTIONS_COLHEADING_QUEUED_ENG + g_unStringLanguageIdOffset );
    pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(nColIndex++,&pColumn);
	strTemp.ReleaseBuffer();
	
	strTemp = m_ExtStr.LoadString( IDS_CONNECTIONS_COLHEADING_DROPPED_ENG + g_unStringLanguageIdOffset );
    pColumn.pszText = strTemp.GetBuffer(MAX_PATH);
    SetColumn(nColIndex++,&pColumn);
	strTemp.ReleaseBuffer();

	m_iColumns = nColIndex;
	m_vecOldSortAscending.resize(m_iColumns);
}
