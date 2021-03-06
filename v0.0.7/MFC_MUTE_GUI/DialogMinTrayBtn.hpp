// ------------------------------------------------------------
//  CDialogMinTrayBtn template class
//  MFC CDialog with minimize to systemtray button (0.04)
//  Supports WinXP styles (thanks to David Yuheng Zhao for CVisualStylesXP - yuheng_zhao@yahoo.com)
// ------------------------------------------------------------
//  DialogMinTrayBtn.hpp
//  zegzav - 2002,2003 - eMule project (http://www.emule-project.net)
// ------------------------------------------------------------

#include "AfxBeginMsgMapTemplate.h"
#include "DialogMinTrayBtn.h"
#include "VisualStylesXP.h"


// ------------------------------
//  constants
// ------------------------------

#define CAPTION_BUTTONSPACE      (2)
#define CAPTION_MINHEIGHT        (8)

#define TIMERMINTRAYBTN_ID       0x76617a67
#define TIMERMINTRAYBTN_PERIOD   200    // ms

#define WP_TRAYBUTTON WP_MINBUTTON

BEGIN_TM_PART_STATES(TRAYBUTTON)
    TM_STATE(1, TRAYBS, NORMAL)
    TM_STATE(2, TRAYBS, HOT)
    TM_STATE(3, TRAYBS, PUSHED)
    TM_STATE(4, TRAYBS, DISABLED)
	// Inactive
    TM_STATE(5, TRAYBS, INORMAL)	
    TM_STATE(6, TRAYBS, IHOT)
    TM_STATE(7, TRAYBS, IPUSHED)
    TM_STATE(8, TRAYBS, IDISABLED)
END_TM_PART_STATES()

#define BMP_TRAYBTN_WIDTH		(21)
#define BMP_TRAYBTN_HEIGHT		(21)
#define BMP_TRAYBTN_BLUE		"TRAYBUTTON_LUNA_BLUE_BMP"
#define BMP_TRAYBTN_METALLIC	"TRAYBUTTON_LUNA_METALLIC_BMP"
#define BMP_TRAYBTN_HOMESTEAD	"TRAYBUTTON_LUNA_HOMESTEAD_BMP"
#define BMP_TRAYBTN_TRANSCOLOR	(RGB(255,0,255))

template <class BASE> const CHAR *CDialogMinTrayBtn<BASE>::m_pszMinTrayBtnBmpName[] = { BMP_TRAYBTN_BLUE, BMP_TRAYBTN_METALLIC, BMP_TRAYBTN_HOMESTEAD };

#define VISUALSTYLESXP_DEFAULTFILE		L"LUNA.MSSTYLES"
#define VISUALSTYLESXP_BLUE				0
#define VISUALSTYLESXP_METALLIC			1
#define VISUALSTYLESXP_HOMESTEAD		2
#define VISUALSTYLESXP_NAMEBLUE			L"NORMALCOLOR"
#define VISUALSTYLESXP_NAMEMETALLIC		L"METALLIC"
#define VISUALSTYLESXP_NAMEHOMESTEAD	L"HOMESTEAD"

// _WIN32_WINNT >= 0x0501 (XP only)
#define _WM_THEMECHANGED                0x031A	
#define _ON_WM_THEMECHANGED()														\
	{	_WM_THEMECHANGED, 0, 0, 0, AfxSig_lwl,										\
		(AFX_PMSG)(AFX_PMSGW)														\
		(static_cast< LRESULT (AFX_MSG_CALL CWnd::*)(void) > (_OnThemeChanged))		\
	},

// _WIN32_WINDOWS >= 0x0410 (95 not supported)
//template <class BASE> BOOL (WINAPI *CDialogMinTrayBtn<BASE>::_TransparentBlt)(HDC, int, int, int, int, HDC, int, int, int, int, UINT)= NULL;


// ------------------------------
//  contructor/init
// ------------------------------

template <class BASE> CDialogMinTrayBtn<BASE>::CDialogMinTrayBtn() :
    m_MinTrayBtnPos(0,0), m_MinTrayBtnSize(0,0), m_bMinTrayBtnEnabled(TRUE), m_bMinTrayBtnVisible(TRUE), 
    m_bMinTrayBtnUp(TRUE), m_bMinTrayBtnCapture(FALSE), m_bMinTrayBtnActive(FALSE), m_bMinTrayBtnHitTest(FALSE)
{
    MinTrayBtnInit();
}

template <class BASE> CDialogMinTrayBtn<BASE>::CDialogMinTrayBtn(LPCTSTR lpszTemplateName, CWnd* pParentWnd) : BASE(lpszTemplateName, pParentWnd),
    m_MinTrayBtnPos(0,0), m_MinTrayBtnSize(0,0), m_bMinTrayBtnEnabled(TRUE), m_bMinTrayBtnVisible(TRUE), 
    m_bMinTrayBtnUp(TRUE), m_bMinTrayBtnCapture(FALSE), m_bMinTrayBtnActive(FALSE), m_bMinTrayBtnHitTest(FALSE)
{
    MinTrayBtnInit();
}

template <class BASE> CDialogMinTrayBtn<BASE>::CDialogMinTrayBtn(UINT nIDTemplate, CWnd* pParentWnd) : BASE(nIDTemplate, pParentWnd),
    m_MinTrayBtnPos(0,0), m_MinTrayBtnSize(0,0), m_bMinTrayBtnEnabled(TRUE), m_bMinTrayBtnVisible(TRUE), 
    m_bMinTrayBtnUp(TRUE), m_bMinTrayBtnCapture(FALSE), m_bMinTrayBtnActive(FALSE), m_bMinTrayBtnHitTest(FALSE)
{
    MinTrayBtnInit();
}

template <class BASE> void CDialogMinTrayBtn<BASE>::MinTrayBtnInit()
{
    m_nMinTrayBtnTimerId= 0;
	BOOL bBmpResult = MinTrayBtnInitBitmap();
	// - Never use the 'TransparentBlt' function under Win9x (read SDK)
	// - Load the 'MSIMG32.DLL' only, if it's really needed.
	if (!afxData.bWin95 && bBmpResult && !_TransparentBlt)
	{
		HMODULE hMsImg32= LoadLibrary("MSIMG32.DLL");
		if (hMsImg32)
		{
			(FARPROC &)_TransparentBlt= GetProcAddress(hMsImg32, "TransparentBlt");
			if (!_TransparentBlt)
				FreeLibrary(hMsImg32);
		}
	}
}


BEGIN_MESSAGE_MAP_TEMPLATE(template <class BASE>, CDialogMinTrayBtn<BASE>, CDialogMinTrayBtn, BASE)
    ON_WM_NCPAINT()
    ON_WM_NCACTIVATE()
    ON_WM_NCHITTEST()
    ON_WM_NCLBUTTONDOWN()
    ON_WM_NCRBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_TIMER()
	_ON_WM_THEMECHANGED()
END_MESSAGE_MAP()

// ------------------------------
//  messages
// ------------------------------

template <class BASE> BOOL CDialogMinTrayBtn<BASE>::OnInitDialog()
{
     BOOL bReturn= BASE::OnInitDialog();
     m_nMinTrayBtnTimerId= SetTimer(TIMERMINTRAYBTN_ID, TIMERMINTRAYBTN_PERIOD, NULL);
     return bReturn;
}

template <class BASE> void CDialogMinTrayBtn<BASE>::OnNcPaint() 
{
    // TODO: Add your message handler code here

    BASE::OnNcPaint();
    MinTrayBtnUpdatePosAndSize();
    MinTrayBtnDraw();
}

template <class BASE> BOOL CDialogMinTrayBtn<BASE>::OnNcActivate(BOOL bActive)
{
    MinTrayBtnUpdatePosAndSize();
    BOOL bResult= BASE::OnNcActivate(bActive);
    m_bMinTrayBtnActive= bActive;
    MinTrayBtnDraw();
    return bResult;
}

template <class BASE> UINT CDialogMinTrayBtn<BASE>::OnNcHitTest(CPoint point)
{
    BOOL bPreviousHitTest= m_bMinTrayBtnHitTest;
    m_bMinTrayBtnHitTest= MinTrayBtnHitTest(point);
    if ((!IsWindowsClassicStyle()) && (m_bMinTrayBtnHitTest != bPreviousHitTest))
        MinTrayBtnDraw(); // Windows XP Style (hot button)
    if (m_bMinTrayBtnHitTest)
    {
       return HTMINTRAYBUTTON;
    }
    return BASE::OnNcHitTest(point);
}

template <class BASE> void CDialogMinTrayBtn<BASE>::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
    if ((GetStyle() & WS_DISABLED) || (!MinTrayBtnIsEnabled()) || (!MinTrayBtnIsVisible()) || (!MinTrayBtnHitTest(point)))
    {
        BASE::OnNcLButtonDown(nHitTest, point);
        return;
    }

    SetCapture();
    m_bMinTrayBtnCapture= TRUE;
    MinTrayBtnSetDown();
}

template <class BASE> void CDialogMinTrayBtn<BASE>::OnNcRButtonDown(UINT nHitTest, CPoint point) 
{
    if ((GetStyle() & WS_DISABLED) || (!MinTrayBtnIsVisible()) || (!MinTrayBtnHitTest(point)))
        BASE::OnNcRButtonDown(nHitTest, point);
}

template <class BASE> void CDialogMinTrayBtn<BASE>::OnMouseMove(UINT nFlags, CPoint point) 
{
    if ((GetStyle() & WS_DISABLED) || (!m_bMinTrayBtnCapture))
    { 
        BASE::OnMouseMove(nFlags, point);
        return;
    }

    ClientToScreen(&point);
    m_bMinTrayBtnHitTest= MinTrayBtnHitTest(point);
    if (m_bMinTrayBtnHitTest)
    {
        if (m_bMinTrayBtnUp)
            MinTrayBtnSetDown();
    }
    else
    {
        if (!m_bMinTrayBtnUp)
            MinTrayBtnSetUp();
    }
}

template <class BASE> void CDialogMinTrayBtn<BASE>::OnLButtonUp(UINT nFlags, CPoint point) 
{
    if ((GetStyle() & WS_DISABLED) || (!m_bMinTrayBtnCapture))
    {
        BASE::OnLButtonUp(nFlags, point);
        return;
    }

    ReleaseCapture();
    m_bMinTrayBtnCapture= FALSE;
    MinTrayBtnSetUp();

    ClientToScreen(&point);
    if (MinTrayBtnHitTest(point))
       SendMessage(WM_SYSCOMMAND, SC_MINIMIZETRAY, MAKELONG(point.x, point.y)); 
}

template <class BASE> void CDialogMinTrayBtn<BASE>::OnTimer(UINT_PTR nIDEvent)
{
    if ((!IsWindowsClassicStyle()) && (nIDEvent == m_nMinTrayBtnTimerId))
    {
        // Visual XP Style (hot button)
        CPoint point;
        GetCursorPos(&point);
        BOOL bHitTest= MinTrayBtnHitTest(point);
        if (m_bMinTrayBtnHitTest != bHitTest)
        {
            m_bMinTrayBtnHitTest= bHitTest;
            MinTrayBtnDraw();
        }
    }
}

template <class BASE> LRESULT CDialogMinTrayBtn<BASE>::_OnThemeChanged()
{
	// BASE::OnThemeChanged();
	MinTrayBtnInitBitmap();
	return 0;
}


// ------------------------------
//  methods
// ------------------------------

template <class BASE> void CDialogMinTrayBtn<BASE>::MinTrayBtnUpdatePosAndSize()
{
    DWORD dwStyle= GetStyle();
    DWORD dwExStyle= GetExStyle();

    INT caption= ((dwExStyle & WS_EX_TOOLWINDOW) == 0) ? GetSystemMetrics(SM_CYCAPTION) - 1 : GetSystemMetrics(SM_CYSMCAPTION) - 1;
    if (caption < CAPTION_MINHEIGHT)
       caption= CAPTION_MINHEIGHT;

    CSize borderfixed(-GetSystemMetrics(SM_CXFIXEDFRAME), GetSystemMetrics(SM_CYFIXEDFRAME));
    CSize bordersize(-GetSystemMetrics(SM_CXSIZEFRAME), GetSystemMetrics(SM_CYSIZEFRAME));

    CRect window;
    GetWindowRect(&window);

    CSize button;
    button.cy= caption - (CAPTION_BUTTONSPACE * 2);
    button.cx= button.cy;
    if (IsWindowsClassicStyle())
        button.cx+= 2;

    m_MinTrayBtnSize= button;

    m_MinTrayBtnPos.x= window.Width() - ((CAPTION_BUTTONSPACE + button.cx) * 2);
    m_MinTrayBtnPos.y= CAPTION_BUTTONSPACE;

    if ((dwStyle & WS_THICKFRAME) != 0)
    {
        // resizable window
        m_MinTrayBtnPos+= bordersize;
    }
    else
    {
        // fixed window
        m_MinTrayBtnPos+= borderfixed;
    }

    if ( ((dwExStyle & WS_EX_TOOLWINDOW) == 0) && (((dwStyle & WS_MINIMIZEBOX) != 0) || ((dwStyle & WS_MAXIMIZEBOX) != 0)) )
    {
        if (IsWindowsClassicStyle())
            m_MinTrayBtnPos.x-= (button.cx * 2) + CAPTION_BUTTONSPACE;
        else
            m_MinTrayBtnPos.x-= (button.cx + CAPTION_BUTTONSPACE) * 2;
    }
       
}

template <class BASE> void CDialogMinTrayBtn<BASE>::MinTrayBtnShow()
{
    if (MinTrayBtnIsVisible())
       return;

    m_bMinTrayBtnVisible= TRUE;
    if (IsWindowVisible())
    {
        RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
    }
}

template <class BASE> void CDialogMinTrayBtn<BASE>::MinTrayBtnHide()
{
    if (!MinTrayBtnIsVisible())
       return;

    m_bMinTrayBtnVisible= FALSE;
    if (IsWindowVisible())
    {
        RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
    }
}

template <class BASE> void CDialogMinTrayBtn<BASE>::MinTrayBtnEnable()
{
    if (MinTrayBtnIsEnabled())
       return;

    m_bMinTrayBtnEnabled= TRUE;
    MinTrayBtnSetUp();
}

template <class BASE> void CDialogMinTrayBtn<BASE>::MinTrayBtnDisable()
{
    if (!MinTrayBtnIsEnabled())
       return;

    m_bMinTrayBtnEnabled= FALSE;
    if (m_bMinTrayBtnCapture)
    {
       ReleaseCapture();
       m_bMinTrayBtnCapture= FALSE;
    }
    MinTrayBtnSetUp();
}

template <class BASE> void CDialogMinTrayBtn<BASE>::MinTrayBtnDraw()
{
    if (!MinTrayBtnIsVisible())
       return;

    CDC *pDC= GetWindowDC();
    if (!pDC)
       return; // panic!

    if (IsWindowsClassicStyle())
    {
        CBrush black(GetSysColor(COLOR_BTNTEXT));
        CBrush gray(GetSysColor(COLOR_GRAYTEXT));
        CBrush gray2(GetSysColor(COLOR_BTNHILIGHT));

        // button
        if (m_bMinTrayBtnUp)
           pDC->DrawFrameControl(MinTrayBtnGetRect(), DFC_BUTTON, DFCS_BUTTONPUSH);
        else
           pDC->DrawFrameControl(MinTrayBtnGetRect(), DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED);

        // dot
        CRect btn= MinTrayBtnGetRect();
        btn.DeflateRect(2,2);
        UINT caption= MinTrayBtnGetSize().cy + (CAPTION_BUTTONSPACE * 2);
        UINT pixratio= (caption >= 14) ? ((caption >= 20) ? 2 + ((caption - 20) / 8) : 2) : 1;
        UINT pixratio2= (caption >= 12) ? 1 + (caption - 12) / 8: 0;
        UINT dotwidth= (1 + pixratio * 3) >> 1;
        UINT dotheight= pixratio;
        CRect dot(CPoint(0,0), CPoint(dotwidth, dotheight));
        CSize spc((1 + pixratio2 * 3) >> 1, pixratio2);
        dot-= dot.Size();
        dot+= btn.BottomRight();
        dot-= spc;
        if (!m_bMinTrayBtnUp)
           dot+= CPoint(1,1);
        if (m_bMinTrayBtnEnabled)
        {
           pDC->FillRect(dot, &black);
        }
        else
        {
           pDC->FillRect(dot + CPoint(1,1), &gray2);
           pDC->FillRect(dot, &gray);
        }
    }
	else
	{
		// VisualStylesXP
		CRect btn= MinTrayBtnGetRect();
		int iState;
		if (!m_bMinTrayBtnEnabled)
			iState= TRAYBS_DISABLED;
		else if (GetStyle() & WS_DISABLED)
			iState= MINBS_NORMAL;
		else if (m_bMinTrayBtnHitTest)
			iState= (m_bMinTrayBtnCapture) ? MINBS_PUSHED : MINBS_HOT;
		else
			iState= MINBS_NORMAL;
		// inactive
		if (!m_bMinTrayBtnActive)
			iState+= 4; // inactive state TRAYBS_Ixxx

		if ((m_bmMinTrayBtnBitmap.m_hObject) && (_TransparentBlt))
		{
			// known theme (bitmap)
			CBitmap *pBmpOld;
			CDC dcMem;
			if ((dcMem.CreateCompatibleDC(pDC)) && ((pBmpOld= dcMem.SelectObject(&m_bmMinTrayBtnBitmap)) != NULL))
			{
				_TransparentBlt(pDC->m_hDC, btn.left, btn.top, btn.Width(), btn.Height(), dcMem.m_hDC, 0, BMP_TRAYBTN_HEIGHT * (iState - 1), BMP_TRAYBTN_WIDTH, BMP_TRAYBTN_HEIGHT, BMP_TRAYBTN_TRANSCOLOR);
				dcMem.SelectObject(pBmpOld);
			}
		}
		else
		{
			// unknown theme (ThemeData)
			HTHEME hTheme= g_xpStyle.OpenThemeData(m_hWnd, L"Window");
			if (hTheme)
			{
				btn.top+= btn.Height() / 8;
				g_xpStyle.DrawThemeBackground(hTheme, pDC->m_hDC, WP_TRAYBUTTON, iState, &btn, NULL);
				g_xpStyle.CloseThemeData(hTheme);
			}
		}
	}

    ReleaseDC(pDC);
}

template <class BASE> BOOL CDialogMinTrayBtn<BASE>::MinTrayBtnHitTest(CPoint point) const
{
    CRect rWnd;
    GetWindowRect(&rWnd);
    point.Offset(-rWnd.TopLeft());
    CRect rBtn= MinTrayBtnGetRect();
    rBtn.InflateRect(0, CAPTION_BUTTONSPACE);
    return (rBtn.PtInRect(point));
}

template <class BASE> void CDialogMinTrayBtn<BASE>::MinTrayBtnSetUp()
{
    m_bMinTrayBtnUp= TRUE;
    MinTrayBtnDraw();
}

template <class BASE> void CDialogMinTrayBtn<BASE>::MinTrayBtnSetDown()
{
    m_bMinTrayBtnUp= FALSE;
    MinTrayBtnDraw();
}

template <class BASE> BOOL CDialogMinTrayBtn<BASE>::IsWindowsClassicStyle() const
{
    return (!((g_xpStyle.IsThemeActive()) && (g_xpStyle.IsAppThemed())));
}

template <class BASE> void CDialogMinTrayBtn<BASE>::SetWindowText(LPCTSTR lpszString)
{
	BASE::SetWindowText(lpszString);
	MinTrayBtnDraw();
}

template <class BASE> INT CDialogMinTrayBtn<BASE>::GetVisualStylesXPColor() const
{
	if (IsWindowsClassicStyle())
		return -1;

	WCHAR szwThemeFile[MAX_PATH];
	WCHAR szwThemeColor[256];
	if (g_xpStyle.GetCurrentThemeName(szwThemeFile, MAX_PATH, szwThemeColor, 256, NULL, 0) != S_OK)
		return -1;
	WCHAR *p;
	if ((p= wcsrchr(szwThemeFile, '\\')) == NULL)
		return -1;
	p++;
	if (_wcsicmp(p, VISUALSTYLESXP_DEFAULTFILE) != 0)
		return -1;
	if (_wcsicmp(szwThemeColor, VISUALSTYLESXP_NAMEBLUE) == 0)
		return VISUALSTYLESXP_BLUE;
	if (_wcsicmp(szwThemeColor, VISUALSTYLESXP_NAMEMETALLIC) == 0)
		return VISUALSTYLESXP_METALLIC;
	if (_wcsicmp(szwThemeColor, VISUALSTYLESXP_NAMEHOMESTEAD) == 0)
		return VISUALSTYLESXP_HOMESTEAD;
	return -1;
}

template <class BASE> BOOL CDialogMinTrayBtn<BASE>::MinTrayBtnInitBitmap()
{
	INT nColor;
	m_bmMinTrayBtnBitmap.DeleteObject();
	if ((nColor= GetVisualStylesXPColor()) == -1)
		return FALSE;
	const CHAR *pszBmpName= m_pszMinTrayBtnBmpName[nColor];
	return m_bmMinTrayBtnBitmap.LoadBitmap(pszBmpName);
}
