/////////////////////////////////////////////////////

#include "stdafx.h"
#include "Resource.h"
#include "MFC_MUTE_GUI.h"
#include "MuteMainTabCtrl.h"
#include "MuteSearchDlg.h"
#include "MuteDownloadsDlg.h"
#include "MuteUploadsDlg.h"
#include "MuteConnectionsDlg.h"
#include "MuteSharedFilesDlg.h"
#include "MuteOptionsDlg.h"      
#include "MSDNIcons.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMuteMainDlgTabCtrl

//**************************************************************************************
//  FUNCTION:	-	CMuteMainDlgTabCtrl()
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		
//**************************************************************************************
CMuteMainDlgTabCtrl::CMuteMainDlgTabCtrl()
{
	unsigned int	unMaxX = 0;
	unsigned int	unMaxY = 0;
	unsigned int	unCnt;	
	vector<CString>	vecStrIconFileNames;
	LPICONRESOURCE	lpi;
	TCHAR szDirectory[MAX_PATH] = "";
  
	::ZeroMemory( &m_tabPages, sizeof(m_tabPages) );	
	m_nNumberOfPages = 6;
	
	unCnt = 0;
	m_tabPages[unCnt++] = new CMuteSearchDlg;
	m_tabPages[unCnt++] = new CMuteDownloadsDlg;
	m_tabPages[unCnt++] = new CMuteUploadsDlg;
	m_tabPages[unCnt++] = new CMuteConnectionsDlg;
	m_tabPages[unCnt++] = new CMuteSharedFilesDlg;
	m_tabPages[unCnt++] = new CMuteOptionsDlg;

	((CMuteSearchDlg *)m_tabPages[0] )->SetPtrToDownloadWindow( (CDialog *) m_tabPages[1] );
	
	
	// version 0.0.6 -- can now use larger icons statically (as resources)
	// just change the height and width here to match the size of the icons
	// defaults:
	unMaxX = 16;
	unMaxY = 16;
	m_unIconWidth = unMaxX;
	m_unIconHeight = unMaxY;

	::ZeroMemory( &m_IconHandles, m_nNumberOfPages * sizeof(HICON) );
	vecStrIconFileNames.resize(m_nNumberOfPages);

	::GetCurrentDirectory(sizeof(szDirectory) - 1, szDirectory);

	unCnt = 0;
	vecStrIconFileNames[unCnt] = szDirectory;
	vecStrIconFileNames[unCnt] += "\\icons\\searchTab.ico";
	m_IconHandles[unCnt] = (HICON)LoadImage( AfxGetInstanceHandle(), vecStrIconFileNames[unCnt], IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	
	unCnt++;
	vecStrIconFileNames[unCnt] = szDirectory;
	vecStrIconFileNames[unCnt] += "\\icons\\downloadsTab.ico";
	m_IconHandles[unCnt] = (HICON)LoadImage( AfxGetInstanceHandle(), vecStrIconFileNames[unCnt], IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	
	unCnt++;
	vecStrIconFileNames[unCnt] = szDirectory;
	vecStrIconFileNames[unCnt] += "\\icons\\uploadsTab.ico";
	m_IconHandles[unCnt] = (HICON)LoadImage( AfxGetInstanceHandle(), vecStrIconFileNames[unCnt], IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	
	unCnt++;
	vecStrIconFileNames[unCnt] = szDirectory;
	vecStrIconFileNames[unCnt] += "\\icons\\connectionsTab.ico";
	m_IconHandles[unCnt] = (HICON)LoadImage( AfxGetInstanceHandle(), vecStrIconFileNames[unCnt], IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	
	unCnt++;
	vecStrIconFileNames[unCnt] = szDirectory;
	vecStrIconFileNames[unCnt] += "\\icons\\sharedfilesTab.ico";
	m_IconHandles[unCnt] = (HICON)LoadImage( AfxGetInstanceHandle(), vecStrIconFileNames[unCnt], IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	
	unCnt++;
	vecStrIconFileNames[unCnt] = szDirectory;
	vecStrIconFileNames[unCnt] += "\\icons\\settingsTab.ico";
	m_IconHandles[unCnt] = (HICON)LoadImage( AfxGetInstanceHandle(), vecStrIconFileNames[unCnt], IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	
	// determine the max x and y icons.. 
	for( unCnt = 0; unCnt < (unsigned int) m_nNumberOfPages; unCnt++ )
	{
		lpi = NULL;
		lpi = ReadIconFromICOFile( vecStrIconFileNames[unCnt] );
		if( NULL != lpi )
		{
			if( lpi->nNumImages > 0 )
			{
				if( lpi->IconImages[0].Width > unMaxX )
				{
					unMaxX = lpi->IconImages[0].Width;
				}

				if( lpi->IconImages[0].Height > unMaxY )
				{
					unMaxY = lpi->IconImages[0].Height;
				}									

				// Free all the bits
				for( UINT i=0; i< lpi->nNumImages; i++ )
				{
					if( lpi->IconImages[i].lpBits != NULL )
					{
						free( lpi->IconImages[i].lpBits );
					}
				}			
			}
			free( lpi );
		}	
	}

	m_unIconWidth = unMaxX;
	m_unIconHeight = unMaxY;

	m_pImgLst.Create(m_unIconWidth,m_unIconHeight,ILC_COLOR32|ILC_MASK,0,10);
	m_pImgLst.SetBkColor(CLR_NONE);
	
	unCnt = 0;
	if( NULL != m_IconHandles[unCnt] )
	{
		m_pImgLst.Add(m_IconHandles[unCnt]);
	}
	else
	{
		m_pImgLst.Add(AfxGetApp()->LoadIcon(IDI_SEARCH_ICON));
	}

	unCnt++;
	if( NULL != m_IconHandles[unCnt] )
	{
		m_pImgLst.Add(m_IconHandles[unCnt]);
	}
	else
	{
		m_pImgLst.Add(AfxGetApp()->LoadIcon(IDI_DOWNLOAD2_ICON));
	}

	unCnt++;
	if( NULL != m_IconHandles[unCnt] )
	{
		m_pImgLst.Add(m_IconHandles[unCnt]);
	}
	else
	{
		m_pImgLst.Add(AfxGetApp()->LoadIcon(IDI_UPLOAD2_ICON));
	}

	unCnt++;
	if( NULL != m_IconHandles[unCnt] )
	{
		m_pImgLst.Add(m_IconHandles[unCnt]);
	}
	else
	{
		m_pImgLst.Add(AfxGetApp()->LoadIcon(IDI_CONNECTIONS_ICON));
	}
	
	unCnt++;
	if( NULL != m_IconHandles[unCnt] )
	{
		m_pImgLst.Add(m_IconHandles[unCnt]);
	}
	else
	{
		m_pImgLst.Add(AfxGetApp()->LoadIcon(IDI_SHARED_FILES_ICON));
	}

	unCnt++;
	if( NULL != m_IconHandles[unCnt] )
	{
		m_pImgLst.Add(m_IconHandles[unCnt]);
	}
	else
	{
		m_pImgLst.Add(AfxGetApp()->LoadIcon(IDI_PREFERENCES_ICON));
	}	

	m_bFirstTime = true;
}

//**************************************************************************************
//  FUNCTION:	-	
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		
//**************************************************************************************
CMuteMainDlgTabCtrl::~CMuteMainDlgTabCtrl()
{
	for(int nCount=0; nCount < m_nNumberOfPages; nCount++)
	{
		delete m_tabPages[nCount];
	}
}

//**************************************************************************************
//  FUNCTION:	-	Init()
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		
//**************************************************************************************
void CMuteMainDlgTabCtrl::Init()
{
	int i;
	m_tabCurrent = 0;
	
	i = 0;
	m_tabPages[i++]->Create( CMuteSearchDlg::IDD, this );
	m_tabPages[i++]->Create( CMuteDownloadsDlg::IDD, this );
	m_tabPages[i++]->Create( CMuteUploadsDlg::IDD, this );
	m_tabPages[i++]->Create( CMuteConnectionsDlg::IDD, this );	
	m_tabPages[i++]->Create( CMuteSharedFilesDlg::IDD, this );
	m_tabPages[i++]->Create( CMuteOptionsDlg::IDD, this );        


	for( i = 0; i < m_nNumberOfPages; i++ )
	{
		m_tabPages[i]->ShowWindow(SW_SHOW);
	}
	
	m_tabPages[m_tabCurrent]->SetFocus();

	if( m_bFirstTime )
	{
		SendMessage( TCM_SETITEMSIZE,0,MAKELPARAM (m_unIconWidth + 3, m_unIconHeight + 3) );

	   m_bFirstTime = false;
	}

	SetRectangle();	
}

//**************************************************************************************
//  FUNCTION:	-	GotoPageIndex()
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		Equivalent of clicking on a tab..to show the current tabs' page
//**************************************************************************************
void CMuteMainDlgTabCtrl::GotoPageIndex( const unsigned int nPageIndex )
{
	if( (int) nPageIndex > (m_nNumberOfPages - 1) )
	{
		return;
	}
	
	m_tabPages[m_tabCurrent]->ShowWindow(SW_HIDE);
	m_tabCurrent = nPageIndex;
	m_tabPages[m_tabCurrent]->ShowWindow(SW_SHOW);
	m_tabPages[m_tabCurrent]->SetFocus();
	SetCurSel(m_tabCurrent);
}

//**************************************************************************************
//  FUNCTION:	-	SetRectangle()
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		
//**************************************************************************************
void CMuteMainDlgTabCtrl::SetRectangle()
{
	CRect tabRect, itemRect;
	int nX, nY, nXc, nYc;

	GetClientRect(&tabRect);
	GetItemRect(0, &itemRect);

	nX=itemRect.left;
	nY=itemRect.bottom+1;
	nXc=tabRect.right-itemRect.left-1;
	nYc=tabRect.bottom-nY-3;

	m_tabPages[0]->SetWindowPos(&wndTop, nX, nY, nXc, nYc, SWP_SHOWWINDOW);
	
	for(int nCount=1; nCount < m_nNumberOfPages; nCount++)
	{
		m_tabPages[nCount]->SetWindowPos(&wndTop, nX, nY, nXc, nYc, SWP_HIDEWINDOW);
	}
}

BEGIN_MESSAGE_MAP(CMuteMainDlgTabCtrl, CTabCtrl)
	//{{AFX_MSG_MAP(CMuteMainDlgTabCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMuteMainDlgTabCtrl message handlers

void CMuteMainDlgTabCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CTabCtrl::OnLButtonDown(nFlags, point);

	if( m_tabCurrent != GetCurFocus() )
	{
		m_tabPages[m_tabCurrent]->ShowWindow(SW_HIDE);
		m_tabCurrent = GetCurFocus();
		m_tabPages[m_tabCurrent]->ShowWindow(SW_SHOW);
		m_tabPages[m_tabCurrent]->SetFocus();
	}
}

//**************************************************************************************
//  FUNCTION:	-	DrawItem()
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		
//**************************************************************************************
void CMuteMainDlgTabCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CRect rect = lpDrawItemStruct->rcItem;
	IMAGEINFO info;
	int nTabIndex = lpDrawItemStruct->itemID;
	if (nTabIndex < 0) return;
	BOOL bSelected = (nTabIndex == GetCurSel());
	
	char label[64];
	TC_ITEM tci;
	tci.mask = TCIF_TEXT|TCIF_IMAGE;
	tci.pszText = label;     
	tci.cchTextMax = 63;    	
	if (!GetItem(nTabIndex, &tci )) return;

	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	if (!pDC) return;
	int nSavedDC = pDC->SaveDC();

	rect.top += ::GetSystemMetrics(SM_CYEDGE);
	
	if (bSelected)
	{
		rect.bottom -= 1;
	}
	else
	{
		rect.bottom += 2;
	}

	pDC->FillSolidRect(rect, ::GetSysColor(COLOR_3DFACE));


	pDC->SetBkMode(TRANSPARENT);

	// Draw image
	CImageList* pImageList = &m_pImgLst;
	if (pImageList && tci.iImage >= 0) 
	{	
		rect.left += pDC->GetTextExtent(_T(" ")).cx;		// Margin

		// Get height of image so we
		pImageList->GetImageInfo(nTabIndex, &info);
		CRect ImageRect(info.rcImage);
		int nYpos = rect.top;

		pImageList->Draw(pDC, nTabIndex, CPoint(rect.left, nYpos), ILD_TRANSPARENT);
		rect.left += ImageRect.Width();
	}

	if (bSelected) {
		rect.top -= ::GetSystemMetrics(SM_CYEDGE);
		pDC->DrawText(label, rect, DT_SINGLELINE|DT_VCENTER|DT_CENTER|DT_NOPREFIX);
		rect.top += ::GetSystemMetrics(SM_CYEDGE);
	} else {
		pDC->DrawText(label, rect, DT_SINGLELINE|DT_BOTTOM|DT_CENTER|DT_NOPREFIX);
	}

	if (nSavedDC)
		pDC->RestoreDC(nSavedDC);
}

//**************************************************************************************
//  FUNCTION:	-	
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		
//**************************************************************************************
void CMuteMainDlgTabCtrl::PreSubclassWindow()
{
	CTabCtrl::PreSubclassWindow();
	//ModifyStyle(0, TCS_OWNERDRAWFIXED); //*** MCoder 05.09.2005 - auskommentiert
}

//**************************************************************************************
//  FUNCTION:	-	
//  RETURNS:	-	
//  PARAMETERS:	-	
//  COMMENTS:		
//**************************************************************************************
int CMuteMainDlgTabCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTabCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	//ModifyStyle(0, TCS_OWNERDRAWFIXED); //*** MCoder 05.09.2005 - auskommentiert
	return 0;
}


//*******************************************************************
//  FUNCTION:   -	
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteMainDlgTabCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CTabCtrl::OnSize(nType, cx, cy);

	CRect tabRect, itemRect;
	int nTabIndex = GetCurSel();
	int nX, nY, nXc, nYc;

	GetClientRect(&tabRect);
	GetItemRect(0, &itemRect);

	nX=itemRect.left;
	nY=itemRect.bottom+1;
	nXc=tabRect.right-itemRect.left-1;
	nYc=tabRect.bottom-nY-3;

	for(int nCount=0; nCount < m_nNumberOfPages; nCount++)
	{
		if( nTabIndex == nCount )
		{
			m_tabPages[nCount]->SetWindowPos(&wndTop, nX, nY, nXc, nYc, SWP_SHOWWINDOW);
		}
		else
		{
			m_tabPages[nCount]->SetWindowPos(&wndTop, nX, nY, nXc, nYc, SWP_HIDEWINDOW);
		}
	}	
	SetCurSel( nTabIndex );
}