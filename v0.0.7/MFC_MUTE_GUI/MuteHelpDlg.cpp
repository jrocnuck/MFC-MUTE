// MuteHelpDlg.cpp : implementation file
//

#include "stdafx.h"
#include "mfc_mute_gui.h"
#include "MuteHelpDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMuteHelpDlg dialog
CMuteHelpDlg::CMuteHelpDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CMuteHelpDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMuteHelpDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMuteHelpDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMuteHelpDlg)
	DDX_Control(pDX, IDC_BUTTON_NEXT, m_btnNext);
	DDX_Control(pDX, IDC_BUTTON_BACK, m_btnBack);
	DDX_Control(pDX, IDC_EXPLORER1, m_browser);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMuteHelpDlg, CResizableDialog)
	//{{AFX_MSG_MAP(CMuteHelpDlg)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_BACK, OnButtonBack)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, OnButtonNext)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMuteHelpDlg message handlers
BOOL CMuteHelpDlg::PreTranslateMessage(MSG* pMsg) 
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( (pMsg->wParam == VK_ESCAPE) || (pMsg->wParam == VK_RETURN) )
		{
			// Don't close the window if the user hits the ESC key.			
			return FALSE;
		}
	}
	
	return CResizableDialog::PreTranslateMessage(pMsg);
}


BOOL CMuteHelpDlg::OnInitDialog() 
{
	CResizableDialog::OnInitDialog();
	
    ShowSizeGrip(FALSE);
	m_browser.SetResizable(true);	

	CRect rect;
    GetClientRect(rect);
    m_browser.MoveWindow(rect.left,rect.top,rect.Width(),rect.bottom-45,true);   
    m_browser.Navigate("http://www.planetpeer.de/wiki/index.php/MuteDevFaq?PHPSESSID=7ed52bcc8d54e9acd83c41c8b315b8d2", NULL, NULL, NULL, NULL);
	index++;

    HICON hIconBack = AfxGetApp()->LoadIcon(IDI_BACK_ICON);
	m_btnBack.SetIcon(hIconBack);

	HICON hIconNext = AfxGetApp()->LoadIcon(IDI_NEXT_ICON);
	m_btnNext.SetIcon(hIconNext);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMuteHelpDlg::OnSize(UINT nType, int cx, int cy) 
{
	CResizableDialog::OnSize(nType, cx, cy);

	CRect rect;
    GetClientRect(rect);

    if( m_browser )
	{
		m_browser.MoveWindow(rect.left,rect.top,rect.Width(),rect.bottom-45,true);
	}
	
	if( m_btnBack )
	{
		m_btnBack.MoveWindow(rect.left,rect.bottom-44,rect.left+(rect.Width()/2),rect.bottom-44,true);
	}
	
    if( m_btnNext )
	{
		m_btnNext.MoveWindow(rect.Width()/2,rect.bottom-44,((rect.right)/2),rect.bottom-44,true);
	}
}


//*******************************************************************
//  FUNCTION:   -	QueueSearchResultHandler()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteHelpDlg::OnButtonBack() 
{	
	if(m_browser)	
	{
		m_browser.GoBack();	
	}
}

//*******************************************************************
//  FUNCTION:   -	QueueSearchResultHandler()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CMuteHelpDlg::OnButtonNext() 
{
	if( m_browser )
	{
		m_browser.GoForward();
	}
}


BEGIN_EVENTSINK_MAP(CMuteHelpDlg, CResizableDialog)
    //{{AFX_EVENTSINK_MAP(CMuteHelpDlg)
	ON_EVENT(CMuteHelpDlg, IDC_EXPLORER1, 105 /* CommandStateChange */, OnCommandStateChangeExplorer1, VTS_I4 VTS_BOOL)
    //}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


void CMuteHelpDlg::OnCommandStateChangeExplorer1(long Command, BOOL Enable) 
{
	if (Command==CSC_NAVIGATEFORWARD) m_btnNext.EnableWindow(Enable);
    if (Command==CSC_NAVIGATEBACK)    m_btnBack.EnableWindow(Enable);	
}
