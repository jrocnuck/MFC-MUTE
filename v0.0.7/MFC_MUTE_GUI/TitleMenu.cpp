// TitleMenu.cpp: implementation of the CTitleMenu class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Utilities.h"
#include "CxImage/xImage.h"
#include "TitleMenu.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define MIIM_STRING      0x00000040
#define MIIM_BITMAP      0x00000080
#define MIIM_FTYPE       0x00000100
#define HBMMENU_CALLBACK ((HBITMAP) -1)

#define MIM_STYLE			0x00000010
#define MNS_CHECKORBMP      0x04000000

#ifndef UNICODE
typedef struct tagMENUITEMINFOAEX
{
    UINT     cbSize;
    UINT     fMask;
    UINT     fType;         // used if MIIM_TYPE (4.0) or MIIM_FTYPE (>4.0)
    UINT     fState;        // used if MIIM_STATE
    UINT     wID;           // used if MIIM_ID
    HMENU    hSubMenu;      // used if MIIM_SUBMENU
    HBITMAP  hbmpChecked;   // used if MIIM_CHECKMARKS
    HBITMAP  hbmpUnchecked; // used if MIIM_CHECKMARKS
    ULONG_PTR dwItemData;   // used if MIIM_DATA
    LPSTR    dwTypeData;    // used if MIIM_TYPE (4.0) or MIIM_STRING (>4.0)
    UINT     cch;           // used if MIIM_TYPE (4.0) or MIIM_STRING (>4.0)
    HBITMAP  hbmpItem;      // used if MIIM_BITMAP
}   MENUITEMINFOEX, FAR *LPMENUITEMINFOEX;
#else
typedef struct tagMENUITEMINFOWEX
{
    UINT     cbSize;
    UINT     fMask;
    UINT     fType;         // used if MIIM_TYPE (4.0) or MIIM_FTYPE (>4.0)
    UINT     fState;        // used if MIIM_STATE
    UINT     wID;           // used if MIIM_ID
    HMENU    hSubMenu;      // used if MIIM_SUBMENU
    HBITMAP  hbmpChecked;   // used if MIIM_CHECKMARKS
    HBITMAP  hbmpUnchecked; // used if MIIM_CHECKMARKS
    ULONG_PTR dwItemData;   // used if MIIM_DATA
    LPWSTR   dwTypeData;    // used if MIIM_TYPE (4.0) or MIIM_STRING (>4.0)
    UINT     cch;           // used if MIIM_TYPE (4.0) or MIIM_STRING (>4.0)
    HBITMAP  hbmpItem;      // used if MIIM_BITMAP
}   MENUITEMINFOEX, FAR *LPMENUITEMINFOEX;
#endif // UNICODE

TSetMenuInfo	CTitleMenu::SetMenuInfo;
TGetMenuInfo	CTitleMenu::GetMenuInfo;
HMODULE			CTitleMenu::m_hUSER32_DLL;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CTitleMenu::CTitleMenu()
{
	DWORD dwComCtrlMjr = 4;
	DWORD dwComCtrlMin = 0;


	Init();

	HFONT hfont = CreatePopupMenuTitleFont();
	ASSERT(hfont);
	m_Font.Attach(hfont);
	clLeft=::GetSysColor(COLOR_ACTIVECAPTION);
	clRight=::GetSysColor(27);  //COLOR_GRADIENTACTIVECAPTION
	clText=::GetSysColor(COLOR_CAPTIONTEXT);
	
	hinst_msimg32 = LoadLibrary( "msimg32.dll" );
	m_bCanDoGradientFill = FALSE;
	
	if(hinst_msimg32)
	{
		m_bCanDoGradientFill = TRUE;		
		dllfunc_GradientFill = ((LPFNDLLFUNC1) GetProcAddress( hinst_msimg32, "GradientFill" ));
	}
	bDrawEdge=false;
	flag_edge=BDR_SUNKENINNER;
	m_bIconMenu = false;
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
	m_mapIconPos.InitHashTable(29);

}

CTitleMenu::~CTitleMenu()
{
	m_Font.DeleteObject();
	FreeLibrary( hinst_msimg32 );
}



void CTitleMenu::Init()
{
	m_hUSER32_DLL = 0;
	SetMenuInfo = NULL;
	GetMenuInfo = NULL;
}



/////////////////////////////////////////////////////////////////////////////
// CTitleMenu message handlers


HFONT CTitleMenu::CreatePopupMenuTitleFont()
{
	// start by getting the stock menu font
	HFONT hfont = (HFONT)GetStockObject(ANSI_VAR_FONT);
	if (hfont) 
	{ 
	    LOGFONT lf; //get the complete LOGFONT describing this font
	    if (GetObject(hfont, sizeof(LOGFONT), &lf)) 
		{
			lf.lfWeight = FW_BOLD;	// set the weight to bold
			// recreate this font with just the weight changed
			return ::CreateFontIndirect(&lf);
		}
	}
	return NULL;
}


//
// This function adds a title entry to a popup menu
//
//void CTitleMenu::AddMenuTitle(LPCTSTR lpszTitle)
void CTitleMenu::AddMenuTitle(LPCTSTR lpszTitle, bool bIsIconMenu)
{
	WORD	dwVersion = DetectWinVersion();

/*	// insert an empty owner-draw item at top to serve as the title
	// note: item is not selectable (disabled) but not grayed
	m_strTitle=CString(lpszTitle);
	InsertMenu(0, MF_BYPOSITION | MF_OWNERDRAW | MF_STRING | MF_DISABLED, 0);
*/
		// insert an empty owner-draw item at top to serve as the title
	// note: item is not selectable (disabled) but not grayed
	if (lpszTitle != NULL)
	{
		m_strTitle = lpszTitle;
		m_strTitle.Replace(_T("&"), _T(""));
		InsertMenu(0, MF_BYPOSITION | MF_OWNERDRAW | MF_STRING | MF_DISABLED, MP_TITLE);
	}
	
	if( bIsIconMenu && (_WINVER_XP_ == dwVersion || _WINVER_2K_ == dwVersion) )
	{
		m_bIconMenu = true;
		m_ImageList.DeleteImageList();
		m_ImageList.Create(ICONSIZE, ICONSIZE, m_iDfltImageListColorFlags|ILC_MASK, 0, 1);
		m_ImageList.SetBkColor(CLR_NONE);
		if (LoadAPI())
		{
			MENUINFO mi;
			mi.fMask = MIM_STYLE;
			mi.cbSize = sizeof(mi);
			GetMenuInfo(m_hMenu, &mi);
			mi.dwStyle |= MNS_CHECKORBMP;
			SetMenuInfo(m_hMenu, &mi);
		}
	}
}


void CTitleMenu::MeasureItem(LPMEASUREITEMSTRUCT mi)
{
	if (mi->itemID == MP_TITLE)
	{
		// get the screen dc to use for retrieving size information
		CDC dc;
		dc.Attach(::GetDC(NULL));
		// select the title font
		//HFONT hfontOld = (HFONT)SelectObject(dc.m_hDC, (HFONT)theApp.m_fontDefaultBold);
		HFONT hfontOld = (HFONT)SelectObject(dc.m_hDC, (HFONT)m_Font);
		// compute the size of the title
		CSize size = dc.GetTextExtent(m_strTitle);
		// deselect the title font
		::SelectObject(dc.m_hDC, hfontOld);
		// add in the left margin for the menu item
		size.cx += GetSystemMetrics(SM_CXMENUCHECK)+8;


		//Return the width and height
		//+ include space for border
		const int nBorderSize = 2;
		mi->itemWidth = size.cx + nBorderSize;
		mi->itemHeight = size.cy + nBorderSize;
		
		// cleanup
		::ReleaseDC(NULL, dc.Detach());
	}
	else
	{
		CMenu::MeasureItem(mi);
		if (m_bIconMenu)
		{
			mi->itemHeight = max(mi->itemHeight, 16);
			mi->itemWidth += 18;
		}
	}
}

/*
void CTitleMenu::MeasureItem(LPMEASUREITEMSTRUCT mi)
{
	// get the screen dc to use for retrieving size information
	CDC dc;
	dc.Attach(::GetDC(NULL));
	// select the title font
	HFONT hfontOld = (HFONT)SelectObject(dc.m_hDC, (HFONT)m_Font);
	// compute the size of the title
	CSize size = dc.GetTextExtent(m_strTitle);
	// deselect the title font
	::SelectObject(dc.m_hDC, hfontOld);
	// add in the left margin for the menu item
	size.cx += GetSystemMetrics(SM_CXMENUCHECK)+8;


	//Return the width and height
	//+ include space for border
	const int nBorderSize = 2;
	mi->itemWidth = size.cx + nBorderSize;
	mi->itemHeight = size.cy + nBorderSize;
	
	// cleanup
	::ReleaseDC(NULL, dc.Detach());
}
*/

void CTitleMenu::DrawItem(LPDRAWITEMSTRUCT di)
{
	if (di->itemID == MP_TITLE)
	{
		COLORREF crOldBk = ::SetBkColor(di->hDC, clLeft);
		
		if(m_bCanDoGradientFill&&(clLeft!=clRight))
		{
 			TRIVERTEX rcVertex[2];
			di->rcItem.right--; // exclude this point, like FillRect does 
			di->rcItem.bottom--;
			rcVertex[0].x=di->rcItem.left;
			rcVertex[0].y=di->rcItem.top;
			rcVertex[0].Red=GetRValue(clLeft)<<8;	// color values from 0x0000 to 0xff00 !!!!
			rcVertex[0].Green=GetGValue(clLeft)<<8;
			rcVertex[0].Blue=GetBValue(clLeft)<<8;
			rcVertex[0].Alpha=0x0000;
			rcVertex[1].x=di->rcItem.right; 
			rcVertex[1].y=di->rcItem.bottom;
			rcVertex[1].Red=GetRValue(clRight)<<8;
			rcVertex[1].Green=GetGValue(clRight)<<8;
			rcVertex[1].Blue=GetBValue(clRight)<<8;
			rcVertex[1].Alpha=0;
			GRADIENT_RECT rect;
			rect.UpperLeft=0;
			rect.LowerRight=1;
			
			// fill the area 
			GradientFill( di->hDC,rcVertex,2,&rect,1,GRADIENT_FILL_RECT_H);
		}
		else
		{
			::ExtTextOut(di->hDC, 0, 0, ETO_OPAQUE, &di->rcItem, NULL, 0, NULL);
		}

		if(bDrawEdge)
		{
			::DrawEdge(di->hDC, &di->rcItem, flag_edge, BF_RECT);
		}
	 

		int modeOld = ::SetBkMode(di->hDC, TRANSPARENT);
		COLORREF crOld = ::SetTextColor(di->hDC, clText);
		// select font into the dc
	//	HFONT hfontOld = (HFONT)SelectObject(di->hDC, (HFONT)theApp.m_fontDefaultBold);
		HFONT hfontOld = (HFONT)SelectObject(di->hDC, (HFONT)m_Font);

		// add the menu margin offset
		di->rcItem.left += GetSystemMetrics(SM_CXMENUCHECK)+8;

		// draw the text left aligned and vertically centered
		::DrawText(di->hDC, m_strTitle, -1, &di->rcItem, DT_SINGLELINE|DT_VCENTER|DT_LEFT);

		//Restore font and colors...
		::SelectObject(di->hDC, hfontOld);
		::SetBkMode(di->hDC, modeOld);
		::SetBkColor(di->hDC, crOldBk);
		::SetTextColor(di->hDC, crOld);
	}
	else
	{
		CDC* dc = CDC::FromHandle(di->hDC);
		int posY = di->rcItem.top + ((di->rcItem.bottom-di->rcItem.top)-ICONSIZE) / 2;
		int nIconPos;
		if (!m_mapIconPos.Lookup(di->itemID, nIconPos))
			return;

		if ((di->itemState & ODS_GRAYED) != 0)	
		{
			DrawMonoIcon(nIconPos, CPoint(di->rcItem.left,posY), dc);
			return;
		}

		// Draw the bitmap on the menu.
		m_ImageList.Draw(dc, nIconPos, CPoint(di->rcItem.left,posY), ILD_TRANSPARENT);
	}
}
/*
void CTitleMenu::DrawItem(LPDRAWITEMSTRUCT di)
{
	COLORREF crOldBk = ::SetBkColor(di->hDC, clLeft);
	
	if(m_bCanDoGradientFill&&(clLeft!=clRight))
	{
 		TRIVERTEX rcVertex[2];
		di->rcItem.right--; // exclude this point, like FillRect does 
		di->rcItem.bottom--;
		rcVertex[0].x=di->rcItem.left;
		rcVertex[0].y=di->rcItem.top;
		rcVertex[0].Red=GetRValue(clLeft)<<8;	// color values from 0x0000 to 0xff00 !!!!
		rcVertex[0].Green=GetGValue(clLeft)<<8;
		rcVertex[0].Blue=GetBValue(clLeft)<<8;
		rcVertex[0].Alpha=0x0000;
		rcVertex[1].x=di->rcItem.right; 
		rcVertex[1].y=di->rcItem.bottom;
		rcVertex[1].Red=GetRValue(clRight)<<8;
		rcVertex[1].Green=GetGValue(clRight)<<8;
		rcVertex[1].Blue=GetBValue(clRight)<<8;
		rcVertex[1].Alpha=0;
		GRADIENT_RECT rect;
		rect.UpperLeft=0;
		rect.LowerRight=1;
		
		// fill the area 
		GradientFill( di->hDC,rcVertex,2,&rect,1,GRADIENT_FILL_RECT_H);
	}
	else
	{
		::ExtTextOut(di->hDC, 0, 0, ETO_OPAQUE, &di->rcItem, NULL, 0, NULL);
	}
	if(bDrawEdge)
		::DrawEdge(di->hDC, &di->rcItem, flag_edge, BF_RECT);
 

	int modeOld = ::SetBkMode(di->hDC, TRANSPARENT);
	COLORREF crOld = ::SetTextColor(di->hDC, clText);
	// select font into the dc
	HFONT hfontOld = (HFONT)SelectObject(di->hDC, (HFONT)m_Font);

	// add the menu margin offset
	di->rcItem.left += GetSystemMetrics(SM_CXMENUCHECK)+8;

	// draw the text left aligned and vertically centered
	::DrawText(di->hDC, m_strTitle, -1, &di->rcItem, DT_SINGLELINE|DT_VCENTER|DT_LEFT);

	//Restore font and colors...
	::SelectObject(di->hDC, hfontOld);
	::SetBkMode(di->hDC, modeOld);
	::SetBkColor(di->hDC, crOldBk);
	::SetTextColor(di->hDC, crOld);
}
*/

BOOL CTitleMenu::GradientFill(HDC hdc, PTRIVERTEX pVertex, DWORD dwNumVertex, PVOID pMesh, DWORD dwNumMesh, DWORD dwMode)
{
	ASSERT(m_bCanDoGradientFill); 
	return dllfunc_GradientFill(hdc,pVertex,dwNumVertex,pMesh,dwNumMesh,dwMode); 
}

BOOL CTitleMenu::AppendMenu(UINT nFlags, UINT_PTR nIDNewItem, LPCTSTR lpszNewItem, LPCTSTR lpszIconName)
{
	WORD	dwVersion = DetectWinVersion();
	BOOL	bResult = CMenu::AppendMenu(nFlags, nIDNewItem, lpszNewItem);

	if( !m_bIconMenu || 
		(nFlags & MF_SEPARATOR) != 0 || 
		!( (_WINVER_XP_ == dwVersion)|| 
		(_WINVER_2K_ == dwVersion) ) 
	  )
	{
		/* JROC -- just going to avoid ASSERTION
		if (lpszIconName != NULL)
			ASSERT( false );
		*/
		return bResult;
	}

	MENUITEMINFOEX info;
	ZeroMemory(&info, sizeof(info));
	info.fMask = MIIM_BITMAP;
	info.hbmpItem = HBMMENU_CALLBACK;
	info.cbSize = sizeof(info);
#if defined (_MSC_VER) && (_MSC_VER == 1310)
	VERIFY( SetMenuItemInfo( nIDNewItem, (MENUITEMINFO*)&info, FALSE ) ); 
#else
	VERIFY( SetMenuItemInfo( m_hMenu, nIDNewItem, FALSE, (MENUITEMINFO*)&info) ); 
#endif
	
	if (lpszIconName != NULL){
		int nPos = m_ImageList.Add(CTempIconLoader(lpszIconName));
		if (nPos == (-1))
		{
			ASSERT( false );
		}
		else
		{
			m_mapIconPos.SetAt(nIDNewItem, nPos);
		}
	}
	return bResult;
}


void CTitleMenu::DrawMonoIcon(int nIconPos, CPoint nDrawPos, CDC *dc){
	CWindowDC windowDC(0);
	CDC colorDC;
	colorDC.CreateCompatibleDC(0);
	CBitmap bmpColor;
	bmpColor.CreateCompatibleBitmap(&windowDC, ICONSIZE, ICONSIZE);
	CBitmap* bmpOldColor = colorDC.SelectObject(&bmpColor);
	colorDC.FillSolidRect(0, 0, ICONSIZE, ICONSIZE, dc->GetBkColor());
	CxImage imgBk, imgGray;
	imgBk.CreateFromHBITMAP((HBITMAP)bmpColor);
	m_ImageList.Draw(&colorDC, nIconPos, CPoint(0,0), ILD_TRANSPARENT);
	imgGray.CreateFromHBITMAP((HBITMAP)bmpColor);
	if (imgGray.IsValid() && imgBk.IsValid()){
		imgGray.GrayScale();
		imgBk.GrayScale();
		imgGray.SetTransIndex(imgGray.GetNearestIndex(imgBk.GetPixelColor(0,0)));
		imgGray.Draw((HDC)*dc,nDrawPos.x,nDrawPos.y);
	}
	colorDC.SelectObject(bmpOldColor);
	colorDC.DeleteDC();
	bmpColor.DeleteObject();
}

bool CTitleMenu::LoadAPI()
{
	if (m_hUSER32_DLL == 0)
		m_hUSER32_DLL = LoadLibrary(_T("User32.dll"));
    if (m_hUSER32_DLL == 0) {
        return false;
    }

	bool bSucceeded = true;
	bSucceeded = bSucceeded && (SetMenuInfo != NULL || (SetMenuInfo = (TSetMenuInfo) GetProcAddress(m_hUSER32_DLL,"SetMenuInfo")) != NULL);
	bSucceeded = bSucceeded && (GetMenuInfo != NULL || (GetMenuInfo = (TGetMenuInfo) GetProcAddress(m_hUSER32_DLL,"GetMenuInfo")) != NULL);

	if (!bSucceeded){
		FreeAPI();
		return false;
	}
	return true;
}

void CTitleMenu::FreeAPI()
{
	if (m_hUSER32_DLL != 0)
	{
		FreeLibrary(m_hUSER32_DLL);
		m_hUSER32_DLL = 0;
	}

	SetMenuInfo = NULL;
	GetMenuInfo = NULL;
}
