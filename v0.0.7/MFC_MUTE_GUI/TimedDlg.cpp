// TimedDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TimedDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CTimedDlg dialog

CTimedDlg::CTimedDlg( CWnd* pParent /*=NULL*/ )
//	: CDialog(CTimedDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTimedDlg)
	//}}AFX_DATA_INIT

	m_pParentWnd = pParent;

	CDialog::Create(CTimedDlg::IDD, pParent);
	m_bReadyToClose = FALSE;
}

//*******************************************************************
//  FUNCTION:   -	DoDataExchange
//  RETURNS:    -	
//  PARAMETERS: -	CDataExchange* pDX -- pointer to windows Data Exchange object
//  COMMENTS:	-	Called by the framework.  This function ties the GUI
//					resources to the actual objects in code.
//*******************************************************************
void CTimedDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTimedDlg)
	DDX_Control(pDX, IDC_STATIC_TIMEDLG_TEXT, m_objTimedDlgText);
	DDX_Control(pDX, IDC_PROGRESS1, m_objProgress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTimedDlg, CDialog)
	//{{AFX_MSG_MAP(CTimedDlg)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTimedDlg message handlers

void CTimedDlg::OnClose() 
{
	int i;

	if ( m_bReadyToClose )
	{
		
		// since we are cross threaded, we have to allow all MESSAGES in the message pump to get processed before
		// the Timed Dlg WINDOW is deleted.  The Timed DIALOG window has to allow the animated GIF thread to go away
		// before deleting it from this thread.
		for( i = 0; i < 20; i++ )
		{
			Sleep(1);
			PumpMessages();
		}

		DestroyWindow();
	}

	return;
}

//*******************************************************************
//  FUNCTION:   -	
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
BOOL CTimedDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
//	m_dlgText.SetFontSize( 16 );
//	m_dlgText.SetFontBold(TRUE);

	m_objProgress.SetRange(0,5);
	m_objProgress.SetPos(0);
	//m_ActivityGIF.Load(MAKEINTRESOURCE(IDR_SLIDERBAR_GIF), "GIF" );
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//*******************************************************************
//  FUNCTION:   -	
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
BOOL CTimedDlg::PreTranslateMessage(MSG* pMsg) 
{
	// Check for the ESC key.
	if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE )
	{
		// Filter out the ESC key.
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

//*******************************************************************
//  FUNCTION:   -	
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CTimedDlg::CloseDlgWindow()
{
	CWnd *pWnd = NULL;
	
	if ( m_bReadyToClose == FALSE )
	{
		// hide the window before enabling the parent, etc.
		if (m_hWnd != NULL)
		{
			SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW|
				SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
		}

		m_bReadyToClose = TRUE;

		OnClose();
	}
	
	PumpMessages();


	return;
}


//*******************************************************************
//  FUNCTION:   -	PumpMessages()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CTimedDlg::PumpMessages()
{
	MSG	 msg;

	while( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
	{
		::DispatchMessage( &msg );
	}
}


//*******************************************************************
//  FUNCTION:   -	DisplayWindow()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CTimedDlg::DisplayWindow()
{
	CWnd *pWnd = NULL;
	RECT rectMainWnd;
	RECT rectDlg;
	int nMainWidth;
	int nDlgWidth;
	int nDlgLeft;

	int nMainHeight;
	int nDlgHeight;
	int nDlgTop;
	POINT dlgPoint;

	// Calculate the center of the desktop.
	GetClientRect( &rectDlg );
	GetDesktopWindow()->GetWindowRect( &rectMainWnd );
	GetWindowRect( &rectDlg );
	
	nMainWidth = rectMainWnd.right + rectMainWnd.left;
	nDlgWidth = rectDlg.right + rectDlg.left;
	nDlgLeft = (nMainWidth / 2) - (nDlgWidth / 2);
	
	nMainHeight = rectMainWnd.bottom + rectMainWnd.top;
	nDlgHeight = rectDlg.bottom + rectDlg.top;
	nDlgTop = (nMainHeight / 2) - (nDlgHeight / 2);
	
	dlgPoint.x = nDlgLeft;
	dlgPoint.y = nDlgTop;
	ClientToScreen( &dlgPoint );

	SetWindowPos( NULL, dlgPoint.x, dlgPoint.y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW );
	PumpMessages();
	return;
}

void CTimedDlg::HideWindow()
{
	ShowWindow( SW_HIDE );
	return;
}

