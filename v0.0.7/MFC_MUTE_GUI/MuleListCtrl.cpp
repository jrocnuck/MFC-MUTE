//////////////////////////////////
// MuleListCtrl.cpp
// : implementation file
//

#include "stdafx.h"
//#include "emule.h"
#include "memdc.h"
#include "MuleListCtrl.h"


#ifdef _DEBUG1
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif




//////////////////////////////////
// CMuleListCtrl

IMPLEMENT_DYNAMIC(CMuleListCtrl, CListCtrl)
CMuleListCtrl::CMuleListCtrl(PFNLVCOMPARE pfnCompare, DWORD dwParamSort) 
{
	DWORD dwComCtrlMjr = 4;
	DWORD dwComCtrlMin = 0;

	m_SortProc = pfnCompare;
	m_dwParamSort = dwParamSort;

	m_bCustomDraw = false;
	m_iCurrentSortItem = -1;
	m_iColumnsTracked = 0;
	m_aColumns = NULL;
	m_iRedrawCount = 0;

	//just in case
    m_crWindow = 0;
    m_crWindowText = 0;
    m_crHighlight = 0;
    m_crFocusLine = 0;
    m_crNoHighlight = 0;
    m_crNoFocusLine = 0;
	m_bGeneralPurposeFind = false;
    m_bFindMatchCase = false;
    m_iFindDirection = 1;
    m_iFindColumn = 0;

	m_iDfltImageListColorFlags = ILC_COLOR;
	m_ullComCtrlVer = MAKEDLLVERULL(4,0,0,0);
	AtlGetCommCtrlVersion(&dwComCtrlMjr, &dwComCtrlMin);
	m_ullComCtrlVer = MAKEDLLVERULL(dwComCtrlMjr,dwComCtrlMin,0,0);
	//m_sizSmallSystemIcon.cx = GetSystemMetrics(SM_CXSMICON);
	//m_sizSmallSystemIcon.cy = GetSystemMetrics(SM_CYSMICON);

	m_iDfltImageListColorFlags = GetAppImageListColorFlag();

	// don't use 32bit color resources if not supported by commctl
	if (m_iDfltImageListColorFlags == ILC_COLOR32 && m_ullComCtrlVer < MAKEDLLVERULL(6,0,0,0))
		m_iDfltImageListColorFlags = ILC_COLOR16;
	// don't use >8bit color resources with OSs with restricted memory for GDI resources
	if (afxData.bWin95)
		m_iDfltImageListColorFlags = ILC_COLOR8;


}

CMuleListCtrl::~CMuleListCtrl() 
{
	if(m_aColumns != NULL)
		delete[] m_aColumns;
}

int CMuleListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) 
{
	return 0;
}

void CMuleListCtrl::SetName(LPCTSTR lpszName) 
{
	m_Name = lpszName;
}

void CMuleListCtrl::PreSubclassWindow() 
{
	SetColors();
	CListCtrl::PreSubclassWindow();
	ModifyStyle(LVS_SINGLESEL|LVS_LIST|LVS_ICON|LVS_SMALLICON,LVS_REPORT|LVS_SINGLESEL|LVS_REPORT);
	SetExtendedStyle(LVS_EX_HEADERDRAGDROP);
}

// JROC ADDED..
int CMuleListCtrl::GetItemUnderMouse(CListCtrl* ctrl)
{
	CPoint pt;
	::GetCursorPos(&pt);
	ctrl->ScreenToClient(&pt);
	LVHITTESTINFO hit, subhit;
	hit.pt = pt;
	subhit.pt = pt;
	ctrl->SubItemHitTest(&subhit);
	int sel = ctrl->HitTest(&hit);
	if (sel != LB_ERR && (hit.flags & LVHT_ONITEM))
	{
		// just for grabbing first column
		/*if (subhit.iSubItem == 0) */
			return sel;
	}
	return LB_ERR;
}

int CMuleListCtrl::IndexToOrder(CHeaderCtrl* pHeader, int iIndex) 
{
	int iCount = pHeader->GetItemCount();
	int *piArray = new int[iCount];
	Header_GetOrderArray( pHeader->m_hWnd, iCount, piArray);
	for(int i=0; i < iCount; i++ )
	{
		if(piArray[i] == iIndex) 
		{
			delete[] piArray;
			return i;
		}
	}

	delete[] piArray;
	return -1;
}

void CMuleListCtrl::HideColumn(int iColumn) 
{
	int i;
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	if(iColumn < 1 || iColumn >= iCount || m_aColumns[iColumn].bHidden)
		return;

	//stop it from redrawing
	SetRedraw(FALSE);

	//shrink width to 0
	HDITEM item;
	item.mask = HDI_WIDTH;
	pHeaderCtrl->GetItem(iColumn, &item);
	m_aColumns[iColumn].iWidth = item.cxy;
	item.cxy = 0;
	pHeaderCtrl->SetItem(iColumn, &item);

	//move to front of list
	INT *piArray = new INT[m_iColumnsTracked];
	pHeaderCtrl->GetOrderArray(piArray, m_iColumnsTracked);

	int iFrom = m_aColumns[iColumn].iLocation;
	for( i = 0; i < m_iColumnsTracked; i++)
		if(m_aColumns[i].iLocation > m_aColumns[iColumn].iLocation && m_aColumns[i].bHidden)
			iFrom++;

	for( i = iFrom; i > 0; i--)
	{
		piArray[i] = piArray[i - 1];
	}
	piArray[0] = iColumn;
	pHeaderCtrl->SetOrderArray(m_iColumnsTracked, piArray);
	delete[] piArray;

	//update entry
	m_aColumns[iColumn].bHidden = true;

	//redraw
	SetRedraw(TRUE);
	Invalidate(FALSE);
}

void CMuleListCtrl::ShowColumn(int iColumn) 
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	if(iColumn < 1 || iColumn >= iCount || !m_aColumns[iColumn].bHidden)
		return;

	//stop it from redrawing
	SetRedraw(FALSE);

	//restore position in list
	INT *piArray = new INT[m_iColumnsTracked];
	pHeaderCtrl->GetOrderArray(piArray, m_iColumnsTracked);
	int iCurrent = IndexToOrder(pHeaderCtrl, iColumn);

	for(; iCurrent < IndexToOrder(pHeaderCtrl, 0) && iCurrent < m_iColumnsTracked - 1; iCurrent++ )
	{
		piArray[iCurrent] = piArray[iCurrent + 1];
	}

	for(; m_aColumns[iColumn].iLocation > m_aColumns[pHeaderCtrl->OrderToIndex(iCurrent + 1)].iLocation &&
	      iCurrent < m_iColumnsTracked - 1; iCurrent++)
	{
		piArray[iCurrent] = piArray[iCurrent + 1];
	}
	piArray[iCurrent] = iColumn;
	pHeaderCtrl->SetOrderArray(m_iColumnsTracked, piArray);
	delete[] piArray;

	//and THEN restore original width
	HDITEM item;
	item.mask = HDI_WIDTH;
	item.cxy = m_aColumns[iColumn].iWidth;
	pHeaderCtrl->SetItem(iColumn, &item);

	//update entry
	m_aColumns[iColumn].bHidden = false;

	//redraw
	SetRedraw(TRUE);
	Invalidate(FALSE);
}


void CMuleListCtrl::SetColors() 
{
	m_crWindow      = ::GetSysColor(COLOR_WINDOW);
	m_crWindowText  = ::GetSysColor(COLOR_WINDOWTEXT);

	COLORREF crHighlight = ::GetSysColor(COLOR_HIGHLIGHT);
	m_crFocusLine   = crHighlight;
	m_crNoHighlight = MLC_RGBBLEND(crHighlight, m_crWindow, 8);
	m_crNoFocusLine = MLC_RGBBLEND(crHighlight, m_crWindow, 2);
	m_crHighlight   = MLC_RGBBLEND(crHighlight, m_crWindow, 4);
}

void CMuleListCtrl::SetSortArrow(int iColumn, ArrowType atType) 
{
	HDITEM headerItem;
	headerItem.mask = HDI_FORMAT;
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();

	if(iColumn != m_iCurrentSortItem) 
	{
		pHeaderCtrl->GetItem(m_iCurrentSortItem, &headerItem);
		headerItem.fmt &= ~(HDF_IMAGE | HDF_BITMAP_ON_RIGHT);
		pHeaderCtrl->SetItem(m_iCurrentSortItem, &headerItem);
		m_iCurrentSortItem = iColumn;
		m_imlHeaderCtrl.DeleteImageList();
	}

	//place new arrow unless we were given an invalid column
	if( iColumn >= 0 && pHeaderCtrl->GetItem(iColumn, &headerItem) ) 
	{
		m_atSortArrow = atType;

		HINSTANCE hInstRes = AfxFindResourceHandle(MAKEINTRESOURCE(m_atSortArrow), RT_BITMAP);
		if (hInstRes != NULL)
		{
			HBITMAP hbmSortStates = (HBITMAP)::LoadImage(hInstRes, MAKEINTRESOURCE(m_atSortArrow), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS);
			if (hbmSortStates != NULL){
				CBitmap bmSortStates;
				bmSortStates.Attach(hbmSortStates);

				CImageList imlSortStates;
				if (imlSortStates.Create(14, 14, m_iDfltImageListColorFlags | ILC_MASK, 1, 0))
				{
					VERIFY( imlSortStates.Add(&bmSortStates, RGB(255, 0, 255)) != -1 );

					// To avoid drawing problems (which occure only with an image list *with* a mask) while
					// resizing list view columns which have the header control bitmap right aligned, set
					// the background color of the image list.
					if (m_ullComCtrlVer < MAKEDLLVERULL(6,0,0,0))
					{
						imlSortStates.SetBkColor(GetSysColor(COLOR_BTNFACE));
					}
			
					// When setting the image list for the header control for the first time we'll get
					// the image list of the listview control!! So, better store the header control imagelist separate.
					(void)pHeaderCtrl->SetImageList(&imlSortStates);
					m_imlHeaderCtrl.DeleteImageList();
					m_imlHeaderCtrl.Attach(imlSortStates.Detach());

					// Use smaller bitmap margins -- this saves some pixels which may be required for 
					// rather small column titles.
					if (m_ullComCtrlVer >= MAKEDLLVERULL(5,8,0,0))
					{
					    int iBmpMargin = pHeaderCtrl->SendMessage(HDM_GETBITMAPMARGIN);
					    int iNewBmpMargin = GetSystemMetrics(SM_CXEDGE) + GetSystemMetrics(SM_CXEDGE)/2;
					    if (iNewBmpMargin < iBmpMargin)
						    pHeaderCtrl->SendMessage(HDM_SETBITMAPMARGIN, iNewBmpMargin);
					}
				}
			}
		}

		headerItem.mask |= HDI_IMAGE;
		headerItem.fmt |= HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
		headerItem.iImage = 0;
		pHeaderCtrl->SetItem(iColumn, &headerItem);
	}
}

int CMuleListCtrl::MoveItem(int iOldIndex, int iNewIndex) 
{ //move item in list, returns index of new item
	if(iNewIndex > iOldIndex)
		iNewIndex--;

	//save substrings
	CString *strs=NULL;
	DWORD Style = GetStyle();
	if((Style & LVS_OWNERDATA) == 0) 
	{
		strs = new CString[m_iColumnsTracked];
		for(int i = 0; i < m_iColumnsTracked; i++)
			strs[i] = GetItemText(iOldIndex, i);
	}

	//copy item
	LVITEM lvi;
	TCHAR szText[256];
	lvi.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM | LVIF_INDENT | LVIF_IMAGE;
	lvi.stateMask = (UINT)-1;
	lvi.iItem = iOldIndex;
	lvi.iSubItem = 0;
	lvi.pszText = szText;
	lvi.cchTextMax = sizeof(szText) / sizeof(szText[0]);
	lvi.iIndent = 0;
	if(GetItem(&lvi) == 0)
		return -1;
	lvi.iItem = iNewIndex;

	//do the move
	SetRedraw(FALSE);
	//SetItemData(iOldIndex, 0);  //should do this to be safe?
	DeleteItem(iOldIndex);
	iNewIndex = InsertItem(&lvi);
	SetRedraw(TRUE);

	//restore substrings
	if((Style & LVS_OWNERDATA) == 0) 
	{
		for(int i = 0; i < m_iColumnsTracked; i++) 
		{
			//SetItemText(iNewIndex, i, strs[i]);
			LVITEM lvi;
			lvi.iSubItem = i;
			lvi.pszText = (LPTSTR)((LPCTSTR)strs[i]);
			DefWindowProc(LVM_SETITEMTEXT, iNewIndex, (LPARAM)&lvi);
		}
		delete[] strs;
	}

	return iNewIndex;
}

int CMuleListCtrl::UpdateLocation(int iItem) 
{
	int iItemCount = GetItemCount();
	if(iItem >= iItemCount || iItem < 0)
		return iItem;

	BOOL notLast = iItem + 1 < iItemCount;
	BOOL notFirst = iItem > 0;

	DWORD_PTR dwpItemData = GetItemData(iItem);
	if(dwpItemData == NULL)
		return iItem;

	if(notFirst) 
	{
		int iNewIndex = iItem - 1;
		POSITION pos = m_Params.FindIndex(iNewIndex);
		int iResult = m_SortProc(dwpItemData, GetParamAt(pos, iNewIndex), m_dwParamSort);
		if(iResult < 0) 
		{
			POSITION posPrev = pos;
			int iDist = iNewIndex / 2;
			while(iDist > 1) 
			{
				for(int i = 0; i < iDist; i++)
					m_Params.GetPrev(posPrev);

				if(m_SortProc(dwpItemData, GetParamAt(posPrev, iNewIndex - iDist), m_dwParamSort) < 0) 
				{
					iNewIndex = iNewIndex - iDist;
					pos = posPrev;
				}
				else
				{
					posPrev = pos;
				}
				iDist /= 2;
			}
			while(--iNewIndex >= 0) 
			{
				m_Params.GetPrev(pos);
				if(m_SortProc(dwpItemData, GetParamAt(pos, iNewIndex), m_dwParamSort) >= 0)
					break;
			}
			MoveItem(iItem, iNewIndex + 1);
			return iNewIndex + 1;
		}
	}

	if(notLast) 
	{
		int iNewIndex = iItem + 1;
		POSITION pos = m_Params.FindIndex(iNewIndex);
		int iResult = m_SortProc(dwpItemData, GetParamAt(pos, iNewIndex), m_dwParamSort);
		if(iResult > 0) 
		{
			POSITION posNext = pos;
			int iDist = (GetItemCount() - iNewIndex) / 2;
			while(iDist > 1) 
			{
				for(int i = 0; i < iDist; i++)
					m_Params.GetNext(posNext);

				if(m_SortProc(dwpItemData, GetParamAt(posNext, iNewIndex + iDist), m_dwParamSort) > 0) 
				{
					iNewIndex = iNewIndex + iDist;
					pos = posNext;
				} 
				else 
				{
					posNext = pos;
				}
				iDist /= 2;
			}
			while(++iNewIndex < iItemCount) 
			{
				m_Params.GetNext(pos);
				if(m_SortProc(dwpItemData, GetParamAt(pos, iNewIndex), m_dwParamSort) <= 0)
					break;
			}
			MoveItem(iItem, iNewIndex);
			return iNewIndex;
		}
	}

	return iItem;
}

DWORD_PTR CMuleListCtrl::GetItemData(int iItem) 
{
	POSITION pos = m_Params.FindIndex(iItem);
	LPARAM lParam = GetParamAt(pos, iItem);

	MLC_ASSERT(lParam == CListCtrl::GetItemData(iItem));
	return lParam;
}

//lower level than everything else so poorly overriden functions don't break us
BOOL CMuleListCtrl::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	int i;
	POSITION pos;
	//lets look for the important messages that are essential to handle
	switch(message) 
	{
	case WM_NOTIFY:
		if(wParam == 0) 
		{
			if(((NMHDR*)lParam)->code == NM_RCLICK) 
			{
				//handle right click on headers and show column menu

				POINT point;
				GetCursorPos (&point);

				/*CTitleMenu tmColumnMenu;
				tmColumnMenu.CreatePopupMenu();
				if(m_Name.GetLength() != 0)
					tmColumnMenu.AddMenuTitle(m_Name);

				CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
				int iCount = pHeaderCtrl->GetItemCount();
				for(int iCurrent = 1; iCurrent < iCount; iCurrent++) {
					HDITEM item;
					char text[255];
					item.pszText = text;
					item.mask = HDI_TEXT;
					item.cchTextMax = 255;
					pHeaderCtrl->GetItem(iCurrent, &item);

					tmColumnMenu.AppendMenu(MF_STRING | m_aColumns[iCurrent].bHidden ? 0 : MF_CHECKED,
						MLC_IDC_MENU + iCurrent, item.pszText);
				}
				tmColumnMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
				VERIFY( tmColumnMenu.DestroyMenu() );
				*/
				return *pResult = TRUE;

			} 
			else if(((NMHDR*)lParam)->code == HDN_BEGINTRACKA || ((NMHDR*)lParam)->code == HDN_BEGINTRACKW) 
			{
				//stop them from changing the size of anything "before" first column

				HD_NOTIFY *pHDN = (HD_NOTIFY*)lParam;
				if(m_aColumns[pHDN->iItem].bHidden)
					return *pResult = TRUE;

			} 
			else if(((NMHDR*)lParam)->code == HDN_ENDDRAG) 
			{
				//stop them from moving first column

				NMHEADER *pHeader = (NMHEADER*)lParam;
				if(pHeader->iItem != 0 && pHeader->pitem->iOrder != 0) 
				{

					int iNewLoc = pHeader->pitem->iOrder - GetHiddenColumnCount();
					if(iNewLoc > 0) 
					{

						if(m_aColumns[pHeader->iItem].iLocation != iNewLoc) 
						{

							if(m_aColumns[pHeader->iItem].iLocation > iNewLoc) 
							{
								int iMax = m_aColumns[pHeader->iItem].iLocation;
								int iMin = iNewLoc;
								for(int i = 0; i < m_iColumnsTracked; i++) 
								{
									if(m_aColumns[i].iLocation >= iMin && m_aColumns[i].iLocation < iMax)
										m_aColumns[i].iLocation++;
								}
							}
							else if(m_aColumns[pHeader->iItem].iLocation < iNewLoc) 
							{
								int iMin = m_aColumns[pHeader->iItem].iLocation;
								int iMax = iNewLoc;
								for(int i = 0; i < m_iColumnsTracked; i++) 
								{
									if(m_aColumns[i].iLocation > iMin && m_aColumns[i].iLocation <= iMax)
										m_aColumns[i].iLocation--;
								}
							}

							m_aColumns[pHeader->iItem].iLocation = iNewLoc;

							Invalidate(FALSE);
							break;
						}
					}
				}

				return *pResult = 1;
			} 
			else if(((NMHDR*)lParam)->code == HDN_DIVIDERDBLCLICKA || ((NMHDR*)lParam)->code == HDN_DIVIDERDBLCLICKW) 
			{
				if (GetStyle() & LVS_OWNERDRAWFIXED) 
				{
					NMHEADER *pHeader = (NMHEADER*)lParam;
					// As long as we do not handle the HDN_DIVIDERDBLCLICK according the actual
					// listview item contents it's better to resize to the header width instead of
					// resizing to zero width. The complete solution for this would require a lot
					// of rewriting in the owner drawn listview controls...
					SetColumnWidth(pHeader->iItem, LVSCW_AUTOSIZE_USEHEADER);
					return *pResult = 1;
				}
			}
		}
		break;

	case WM_COMMAND:
		//deal with menu clicks

		if(wParam == MLC_IDC_UPDATE) 
		{
			UpdateLocation(lParam);
			return *pResult = 1;

		}
		else if(wParam >= MLC_IDC_MENU) 
		{

			CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
			int iCount = pHeaderCtrl->GetItemCount();

			int iToggle = wParam - MLC_IDC_MENU;
			if(iToggle >= iCount)
				break;

			if(m_aColumns[iToggle].bHidden)
				ShowColumn(iToggle);
			else
				HideColumn(iToggle);

			return *pResult = 1;
		}
		break;

	case LVM_DELETECOLUMN:
		//book keeping!
		if(m_aColumns != NULL) {
			for(i = 0; i < m_iColumnsTracked; i++)
				if(m_aColumns[i].bHidden)
					ShowColumn(i);

			delete[] m_aColumns;
			m_aColumns = NULL; // 'new' may throw an exception
		}
		m_aColumns = new MULE_COLUMN[--m_iColumnsTracked];
		for( i = 0; i < m_iColumnsTracked; i++) {
			m_aColumns[i].iLocation = i;
			m_aColumns[i].bHidden = false;
		}
		break;

	//case LVM_INSERTCOLUMN:
	case LVM_INSERTCOLUMNA:
	case LVM_INSERTCOLUMNW:
		//book keeping!

		if(m_aColumns != NULL) 
		{
			for( i = 0; i < m_iColumnsTracked; i++)
				if(m_aColumns[i].bHidden)
					ShowColumn(i);

			delete[] m_aColumns;
			m_aColumns = NULL; // 'new' may throw an exception
		}
		m_aColumns = new MULE_COLUMN[++m_iColumnsTracked];
		for( i = 0; i < m_iColumnsTracked; i++) 
		{
			m_aColumns[i].iLocation = i;
			m_aColumns[i].bHidden = false;
		}
		break;

	case LVM_SETITEM:
		//book keeping
		{
			POSITION pos = m_Params.FindIndex(((LPLVITEM)lParam)->iItem);
			if(pos) 
			{
				m_Params.SetAt(pos, MLC_MAGIC);
				PostMessage(LVM_UPDATE, ((LPLVITEM)lParam)->iItem);
			}
		}
		break;
	case LVN_KEYDOWN:
		{
		break;
		}
	case LVM_SETITEMTEXT:
		//need to check for movement

		*pResult = DefWindowProc(message, wParam, lParam);
		if(*pResult)
			PostMessage(WM_COMMAND, MLC_IDC_UPDATE, wParam);
		return *pResult;

	case LVM_SORTITEMS:
		//book keeping...

		m_dwParamSort = (LPARAM)wParam;
		m_SortProc = (PFNLVCOMPARE)lParam;
		for( pos = m_Params.GetHeadPosition(); pos != NULL; m_Params.GetNext(pos))
			m_Params.SetAt(pos, MLC_MAGIC);
		break;
	case LVM_DELETEALLITEMS:
		//book keeping...

		if(!CListCtrl::OnWndMsg(message, wParam, lParam, pResult) && DefWindowProc(message, wParam, lParam)) 
			m_Params.RemoveAll();
		return *pResult = TRUE;

	case LVM_DELETEITEM:
		//book keeping.....

		MLC_ASSERT(m_Params.GetAt(m_Params.FindIndex(wParam)) == CListCtrl::GetItemData(wParam));
		if(!CListCtrl::OnWndMsg(message, wParam, lParam, pResult) && DefWindowProc(message, wParam, lParam))
				m_Params.RemoveAt(m_Params.FindIndex(wParam));
		return *pResult = TRUE;

	//case LVM_INSERTITEM:
	case LVM_INSERTITEMA:
	case LVM_INSERTITEMW:
		//try to fix position of inserted items
		{
			LPLVITEM pItem = (LPLVITEM)lParam;
			int iItem = pItem->iItem;
			int iItemCount = GetItemCount();
			BOOL notLast = iItem < iItemCount;
			BOOL notFirst = iItem > 0;

			if(notFirst) 
			{
				int iNewIndex = iItem - 1;
				pos = m_Params.FindIndex(iNewIndex);
				int iResult = m_SortProc(pItem->lParam, GetParamAt(pos, iNewIndex), m_dwParamSort);
				if(iResult < 0) {
					POSITION posPrev = pos;
					int iDist = iNewIndex / 2;
					while(iDist > 1) 
					{
						for(int i = 0; i < iDist; i++)
							m_Params.GetPrev(posPrev);

						if(m_SortProc(pItem->lParam, GetParamAt(posPrev, iNewIndex - iDist), m_dwParamSort) < 0) 
						{
							iNewIndex = iNewIndex - iDist;
							pos = posPrev;
						}
						else 
						{
							posPrev = pos;
						}
						iDist /= 2;
					}
					
					while(--iNewIndex >= 0) 
					{
						m_Params.GetPrev(pos);
						if(m_SortProc(pItem->lParam, GetParamAt(pos, iNewIndex), m_dwParamSort) >= 0)
							break;
					}
					pItem->iItem = iNewIndex + 1;
					notLast = false;
				}
			}

			if(notLast) 
			{
				int iNewIndex = iItem;
				pos = m_Params.FindIndex(iNewIndex);
				int iResult = m_SortProc(pItem->lParam, GetParamAt(pos, iNewIndex), m_dwParamSort);
				if(iResult > 0) 
				{
					POSITION posNext = pos;
					int iDist = (GetItemCount() - iNewIndex) / 2;
					while(iDist > 1) 
					{
						for(int i = 0; i < iDist; i++)
							m_Params.GetNext(posNext);

						if(m_SortProc(pItem->lParam, GetParamAt(posNext, iNewIndex + iDist), m_dwParamSort) > 0) 
						{
							iNewIndex = iNewIndex + iDist;
							pos = posNext;
						}
						else 
						{
							posNext = pos;
						}
						iDist /= 2;
					}
					while(++iNewIndex < iItemCount) 
					{
						m_Params.GetNext(pos);
						if(m_SortProc(pItem->lParam, GetParamAt(pos, iNewIndex), m_dwParamSort) <= 0)
							break;
					}
					pItem->iItem = iNewIndex;
				}
			}

			if(pItem->iItem == 0) 
			{
				m_Params.AddHead(pItem->lParam);
				return FALSE;
			}

			LRESULT lResult = DefWindowProc(message, wParam, lParam);
			if(lResult != -1) 
			{
				if(lResult >= GetItemCount())
					m_Params.AddTail(pItem->lParam);
				else if(lResult == 0)
					m_Params.AddHead(pItem->lParam);
				else
					m_Params.InsertAfter(m_Params.FindIndex(lResult - 1), pItem->lParam);
			}
			return *pResult = lResult;
		}
		break;

	case LVM_UPDATE:
		//better fix for old problem... normally Update(int) causes entire list to redraw

		if(wParam == (WPARAM)UpdateLocation(wParam)) 
		{ //no need to invalidate rect if item moved
			RECT rcItem;
			BOOL bResult = GetItemRect(wParam, &rcItem, LVIR_BOUNDS);
			if(bResult)
				InvalidateRect(&rcItem, FALSE);
			return *pResult = bResult;
		}
		return *pResult = TRUE;
	}

	return CListCtrl::OnWndMsg(message, wParam, lParam, pResult);
}


void CMuleListCtrl::OnKeyDown(UINT nChar,UINT nRepCnt,UINT nFlags)
{
	
	if ( nChar=='A' && ::GetAsyncKeyState(VK_CONTROL)<0 ) 
	{
		// Ctrl+A: Select all items
		LV_ITEM theItem;
		theItem.mask= LVIF_STATE;
		theItem.iItem= -1;
		theItem.iSubItem= 0;
		theItem.state= LVIS_SELECTED;
		theItem.stateMask= 2;
		SetItemState(-1, &theItem);
	}
	else if (nChar==VK_DELETE)
	{
		PostMessage(WM_COMMAND, MPG_DELETE, 0);
	}
	else if (m_bGeneralPurposeFind)
	{
		if (nChar == 'F' && (GetKeyState(VK_CONTROL) & 0x8000))
		{
			// Ctrl+F: Search item
			OnFindStart();
		}
		else if (nChar == VK_F3)
		{
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				// Shift+F3: Search previous
				OnFindPrev();
			}
			else
			{
				// F3: Search next
				OnFindNext();
			}
		}
	}

	CListCtrl::OnKeyDown(nChar,nRepCnt,nFlags);
}

BOOL CMuleListCtrl::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if(message != WM_DRAWITEM) 
	{
		//catch the prepaint and copy struct
		if(message == WM_NOTIFY && ((NMHDR*)lParam)->code == NM_CUSTOMDRAW &&
		  ((LPNMLVCUSTOMDRAW)lParam)->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) 
		{

			m_bCustomDraw = CListCtrl::OnChildNotify(message, wParam, lParam, pResult);
			if(m_bCustomDraw)
			{
				memcpy(&m_lvcd, (void*)lParam, sizeof(NMLVCUSTOMDRAW));
			}

			return m_bCustomDraw;
		}

		return CListCtrl::OnChildNotify(message, wParam, lParam, pResult);
	}

	ASSERT(pResult == NULL); // no return value expected
	UNUSED(pResult);         // unused in release builds

	DrawItem((LPDRAWITEMSTRUCT)lParam);
	return TRUE;
}

//////////////////////////////////
// CMuleListCtrl message map

BEGIN_MESSAGE_MAP(CMuleListCtrl, CListCtrl)
	ON_WM_DRAWITEM()
	ON_WM_KEYDOWN()
	ON_WM_ERASEBKGND()
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

//////////////////////////////////
// CMuleListCtrl message handlers

void CMuleListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	//set up our flicker free drawing
	CRect rcItem(lpDrawItemStruct->rcItem);
	CDC *oDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	COLORREF crOldDCBkColor;
	//COLORREF crOldDCBkColor = oDC->SetBkColor(m_crWindow);
	CMemDC pDC(oDC, &rcItem);	
	CFont *pOldFont = pDC->SelectObject(GetFont());
	COLORREF crOldTextColor;
	if(m_bCustomDraw)
	{
		// JROC added this fix to the MULE list to also do backgrounds....
		crOldDCBkColor = pDC->SetBkColor(m_lvcd.clrTextBk);
		crOldTextColor = pDC->SetTextColor(m_lvcd.clrText);
	}
	else
	{
		// JROC added this fix to the MULE list to also do backgrounds...
		crOldDCBkColor = pDC->SetBkColor(m_crWindow);
		crOldTextColor = pDC->SetTextColor(m_crWindowText);
	}

	int iOffset = pDC->GetTextExtent(_T(" "), 1 ).cx*2;
	int iItem = lpDrawItemStruct->itemID;
	CImageList* pImageList;
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();

	//gets the item image and state info
	LV_ITEM lvi;
	lvi.mask = LVIF_IMAGE | LVIF_STATE;
	lvi.iItem = iItem;
	lvi.iSubItem = 0;
	lvi.stateMask = LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED;
	GetItem(&lvi);

	//see if the item be highlighted
	BOOL bHighlight = ((lvi.state & LVIS_DROPHILITED) || (lvi.state & LVIS_SELECTED));
	BOOL bCtrlFocused = ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS));

	//get rectangles for drawing
	CRect rcBounds, rcLabel, rcIcon;
	GetItemRect(iItem, rcBounds, LVIR_BOUNDS);
	GetItemRect(iItem, rcLabel, LVIR_LABEL);
	GetItemRect(iItem, rcIcon, LVIR_ICON);
	CRect rcCol(rcBounds);

	//the label!
	CString sLabel = GetItemText(iItem, 0);
	//labels are offset by a certain amount
	//this offset is related to the width of a space character
	CRect rcHighlight;
	CRect rcWnd;

	//should I check (GetExtendedStyle() & LVS_EX_FULLROWSELECT) ?
	rcHighlight.top    = rcBounds.top;
	rcHighlight.bottom = rcBounds.bottom;
	rcHighlight.left   = rcBounds.left  + 1;
	rcHighlight.right  = rcBounds.right - 1;

	COLORREF crOldBckColor;
	//draw the background color
	if(bHighlight) 
	{
		if(bCtrlFocused) 
		{
			pDC->FillRect(rcHighlight, &CBrush(m_crHighlight));
			crOldBckColor = pDC->SetBkColor(m_crHighlight);
		} 
		else 
		{
			pDC->FillRect(rcHighlight, &CBrush(m_crNoHighlight));
			crOldBckColor = pDC->SetBkColor(m_crNoHighlight);
		}
	} 
	else 
	{		
		// JROC added this fix to the MULE list to also do backgrounds....
		if(!m_bCustomDraw)
		{
			pDC->FillRect(rcHighlight, &CBrush(m_crWindow));
			crOldBckColor = pDC->SetBkColor(m_crWindow);
		}
		else
		{
			pDC->FillRect(rcHighlight, &CBrush(m_lvcd.clrTextBk));
			crOldBckColor = pDC->SetBkColor(m_lvcd.clrTextBk);
		}		
	}

	//update column
	rcCol.right = rcCol.left + GetColumnWidth(0);

	//draw state icon
	if(lvi.state & LVIS_STATEIMAGEMASK) 
	{
		int nImage = ((lvi.state & LVIS_STATEIMAGEMASK)>>12) - 1;
		pImageList = GetImageList(LVSIL_STATE);
		if(pImageList) 
		{
			COLORREF crOld = pImageList->SetBkColor(CLR_NONE);
			pImageList->Draw(pDC, nImage, rcCol.TopLeft(), ILD_NORMAL);
			pImageList->SetBkColor(crOld);
		}
	}

	//draw the item's icon
	pImageList = GetImageList(LVSIL_SMALL);
	if(pImageList) 
	{
		COLORREF crOld = pImageList->SetBkColor(CLR_NONE);
		pImageList->Draw(pDC, lvi.iImage, rcIcon.TopLeft(), ILD_NORMAL);
		pImageList->SetBkColor(crOld);
	}

	//draw item label (column 0)
	rcLabel.left += iOffset / 2;
	rcLabel.right -= iOffset;
	pDC->DrawText(sLabel, -1, rcLabel, MLC_DT_TEXT | DT_LEFT | DT_NOCLIP);

	//draw labels for remaining columns
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH;
	rcBounds.right = rcHighlight.right > rcBounds.right ? rcHighlight.right : rcBounds.right;

	int iCount = pHeaderCtrl->GetItemCount();
	for(int iCurrent = 1; iCurrent < iCount; iCurrent++) 
	{
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		//don't draw column 0 again
		if(iColumn == 0)
			continue;

		GetColumn(iColumn, &lvc);
		//don't draw anything with 0 width
		if(lvc.cx == 0)
			continue;

		rcCol.left = rcCol.right;
		rcCol.right += lvc.cx;

		sLabel = GetItemText(iItem, iColumn);
		if (sLabel.GetLength() == 0)
			continue;

		//get the text justification
		UINT nJustify = DT_LEFT;
		switch(lvc.fmt & LVCFMT_JUSTIFYMASK) 
		{
			case LVCFMT_RIGHT:
				nJustify = DT_RIGHT;
				break;
			case LVCFMT_CENTER:
				nJustify = DT_CENTER;
				break;
			default:
				break;
		}

		rcLabel = rcCol;
		rcLabel.left += iOffset;
		rcLabel.right -= iOffset;

		pDC->DrawText(sLabel, -1, rcLabel, MLC_DT_TEXT | nJustify);
	}

	//draw focus rectangle if item has focus
	if((lvi.state & LVIS_FOCUSED) && (bCtrlFocused || (lvi.state & LVIS_SELECTED))) 
	{
		if(!bCtrlFocused || !(lvi.state & LVIS_SELECTED))
		{
			pDC->FrameRect(rcHighlight, &CBrush(m_crNoFocusLine));
		}
		else
		{
			pDC->FrameRect(rcHighlight, &CBrush(m_crFocusLine));
		}
	}

	pDC->Flush();
	//restore old font
	pDC->SelectObject(pOldFont);
	pDC->SetTextColor(crOldTextColor);
	pDC->SetBkColor(crOldBckColor);
	pDC->SetBkColor(crOldDCBkColor);
}

BOOL CMuleListCtrl::OnEraseBkgnd(CDC* pDC) 
{
	int itemCount = GetItemCount();
	if (!itemCount)
		return CListCtrl::OnEraseBkgnd(pDC);

	RECT clientRect;
	RECT itemRect;
	int topIndex = GetTopIndex();
	int maxItems = GetCountPerPage();
	int drawnItems = itemCount < maxItems ? itemCount : maxItems;

	//draw top portion
	GetClientRect(&clientRect);
	GetItemRect(topIndex, &itemRect, LVIR_BOUNDS);
	clientRect.bottom = itemRect.top;
	pDC->FillSolidRect(&clientRect,GetBkColor());

	//draw bottom portion if we have to
	if(topIndex + maxItems >= itemCount) 
	{
		GetClientRect(&clientRect);
		GetItemRect(topIndex + drawnItems - 1, &itemRect, LVIR_BOUNDS);
		clientRect.top = itemRect.bottom;
		pDC->FillSolidRect(&clientRect, GetBkColor());
	}

	//draw right half if we need to
	if (itemRect.right < clientRect.right) 
	{
		GetClientRect(&clientRect);
		clientRect.left = itemRect.right;
		pDC->FillSolidRect(&clientRect, GetBkColor());
	}

	return TRUE;
}

void CMuleListCtrl::OnSysColorChange() 
{
	//adjust colors
	CListCtrl::OnSysColorChange();
	SetColors();
	
	//redraw the up/down sort arrow (if it's there)
	if(m_iCurrentSortItem >= 0)
		SetSortArrow(m_iCurrentSortItem, (ArrowType)m_atSortArrow);
}

void CMuleListCtrl::ApplyImageList(HIMAGELIST himl)
{
	SendMessage(LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)himl);
	if (m_imlHeaderCtrl.m_hImageList != NULL){
		// Must *again* set the image list for the header control, because LVM_SETIMAGELIST
		// always resets any already specified header control image lists!
		GetHeaderCtrl()->SetImageList(&m_imlHeaderCtrl);
	}
}

/*
//////////////////////////////////////////////////////////////////////////////
// CDlgListSearchListSearch

class CDlgListSearchListSearch : public CDialog
{
	DECLARE_DYNAMIC(CDlgListSearchListSearch)

public:
	CDlgListSearchListSearch(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_LISTVIEW_SEARCH };

	CMuleListCtrl* m_pListView;
	CString m_strFindText;
	int m_iSearchColumn;

protected:
	CComboBox m_ctlSearchCol;

	void UpdateControls();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnEnChangeSearchText();
	virtual BOOL OnInitDialog();
};

IMPLEMENT_DYNAMIC(CDlgListSearchListSearch, CDialog)

BEGIN_MESSAGE_MAP(CDlgListSearchListSearch, CDialog)
	ON_EN_CHANGE(IDC_LISTVIEW_SEARCH_TEXT, OnEnChangeSearchText)
END_MESSAGE_MAP()

CDlgListSearchListSearch::CDlgListSearchListSearch(CWnd* pParent )
	: CDialog(CDlgListSearchListSearch::IDD, pParent)
	, m_strFindText(_T(""))
{
	m_pListView = NULL;
	m_iSearchColumn = 0;
}

void CDlgListSearchListSearch::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTVIEW_SEARCH_COLUMN, m_ctlSearchCol);
	DDX_CBIndex(pDX, IDC_LISTVIEW_SEARCH_COLUMN, m_iSearchColumn);
	DDX_Text(pDX, IDC_LISTVIEW_SEARCH_TEXT, m_strFindText);
}

void CDlgListSearchListSearch::OnEnChangeSearchText()
{
	UpdateControls();
}

void CDlgListSearchListSearch::UpdateControls()
{
	GetDlgItem(IDOK)->EnableWindow(GetDlgItem(IDC_LISTVIEW_SEARCH_TEXT)->GetWindowTextLength() > 0);
}

BOOL CDlgListSearchListSearch::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDlgItemText(IDC_LISTVIEW_SEARCH_TEXT_LBL, GetResString(IDS_SEARCH_TEXT) + _T(':'));
	SetDlgItemText(IDC_LISTVIEW_SEARCH_COLUMN_LBL, GetResString(IDS_SEARCH_COLUMN) + _T(':'));
	
	SetDlgItemText(IDCANCEL, GetResString(IDS_CANCEL));	

	SetIcon(theApp.emuledlg->sourceTrayIcon,FALSE);
	SetWindowText(GetResString(IDS_SW_SEARCHBOX));

	if (m_pListView != NULL)
	{
		TCHAR szColTitle[256];
		LVCOLUMN lvc;
		lvc.mask = LVCF_TEXT;
		lvc.cchTextMax = sizeof(szColTitle)/sizeof(szColTitle[0]);
		lvc.pszText = szColTitle;
		int iCol = 0;
		while (m_pListView->GetColumn(iCol++, &lvc))
			m_ctlSearchCol.AddString(lvc.pszText);
		if ((UINT)m_iSearchColumn >= (UINT)m_ctlSearchCol.GetCount())
			m_iSearchColumn = 0;
	}
	else
	{
		m_ctlSearchCol.EnableWindow(FALSE);
		m_ctlSearchCol.ShowWindow(SW_HIDE);

		m_iSearchColumn = 0;
	}
	m_ctlSearchCol.SetCurSel(m_iSearchColumn);

	UpdateControls();
	return TRUE;
}
*/
void CMuleListCtrl::DoFind(int iStartItem, int iDirection /*1=down, 0 = up*/, BOOL bShowError)
{
	CWaitCursor curHourglass;

	if (iStartItem < 0) {
		MessageBeep((UINT)-1);
		return;
	}

	int iNumItems = iDirection ? GetItemCount() : 0;
	int iItem = iStartItem;
	while ( iDirection ? iItem < iNumItems : iItem >= 0 )
	{
		CString strItemText(GetItemText(iItem, m_iFindColumn));
		//TRACE("iItem=%d  szText=%s\n", iItem, strItemText);
		if (!strItemText.IsEmpty())
		{
			if ( m_bFindMatchCase
				   ? _tcsstr(strItemText, m_strFindText) != NULL
				   : stristr(strItemText, m_strFindText) != NULL )
			{
				// Deselect all listview entries
				SetItemState(-1, 0, LVIS_SELECTED);

				// Select the found listview entry
				SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				SetSelectionMark(iItem);
				EnsureVisible(iItem, FALSE/*bPartialOK*/);
				SetFocus();

				return;
			}
		}

		if (iDirection)
			iItem++;
		else
			iItem--;
	}

	if (bShowError)
		AfxMessageBox(_T(GetResString(IDS_SEARCH_NORESULT_ENG + g_unStringLanguageIdOffset)), MB_ICONINFORMATION);
	else
		MessageBeep((UINT)-1);
}

void CMuleListCtrl::OnFindStart()
{
/*	CDlgListSearchListSearch dlg;
	dlg.m_pListView = this;
	dlg.m_strFindText = m_strFindText;
	//dlg.m_bMatchCase = m_bFindMatchCase;
	//dlg.m_iSearchDirection = m_iFindDirection;
	dlg.m_iSearchColumn = m_iFindColumn;
	if (dlg.DoModal() != IDOK || dlg.m_strFindText.IsEmpty())
		return;
	m_strFindText = dlg.m_strFindText;
	//m_bFindMatchCase = dlg.m_bMatchCase;
	//m_iFindDirection = dlg.m_iSearchDirection;
	m_iFindColumn = dlg.m_iSearchColumn;

	DoFindNext(TRUE/*bShowError/);
*/
}

void CMuleListCtrl::OnFindNext()
{
	DoFindNext(FALSE/*bShowError*/);
}

void CMuleListCtrl::DoFindNext(BOOL bShowError)
{
	int iStartItem = GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED);
	if (iStartItem == -1)
		iStartItem = 0;
	else
		iStartItem = iStartItem + (m_iFindDirection ? 1 : -1);
	DoFind(iStartItem, m_iFindDirection, bShowError);
}

void CMuleListCtrl::OnFindPrev()
{
	int iStartItem = GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED);
	if (iStartItem == -1)
		iStartItem = 0;
	else
		iStartItem = iStartItem + (!m_iFindDirection ? 1 : -1);

	DoFind(iStartItem, !m_iFindDirection, FALSE/*bShowError*/);
}
