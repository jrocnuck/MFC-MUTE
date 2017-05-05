// MuteOptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MFC_MUTE_GUI.h"
#include "MuteOptionsDlg.h"
#include ".\muteoptionsdlg.h"
#include "MuteMainTabCtrl.h"



// CMuteOptionsDlg dialog

IMPLEMENT_DYNAMIC(CMuteOptionsDlg, CDialog)
CMuteOptionsDlg::CMuteOptionsDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CMuteOptionsDlg::IDD, pParent)
{
}

CMuteOptionsDlg::~CMuteOptionsDlg()
{
}

void CMuteOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResizableDialog)
	DDX_Control(pDX, IDC_OPTIONS_TREE, m_optionsTreeCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMuteOptionsDlg, CResizableDialog)
//{{AFX_MSG_MAP(CMuteOptionsDlg)
	ON_NOTIFY(TVN_SELCHANGED, IDC_OPTIONS_TREE, OnTvnSelchangedOptionsTree)	
	ON_WM_SIZE()	
	ON_WM_CLOSE()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// CMuteOptionsDlg message handlers
BOOL CMuteOptionsDlg::PreTranslateMessage(MSG* pMsg) 
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


BOOL CMuteOptionsDlg::OnInitDialog()
{	
	CResizableDialog::OnInitDialog();	
	ShowSizeGrip(FALSE);

	AddAnchor(IDC_OPTIONS_TREE,TOP_LEFT,BOTTOM_LEFT);

    // Create and populate Image list
	m_imglist.Create(16, 16, ILC_MASK | ILC_COLOR32, 0, 3);	
    
	m_imglist.Add(AfxGetApp()->LoadIcon(IDI_PREFERENCES_2_ICON));
    m_imglist.Add(AfxGetApp()->LoadIcon(IDI_MUTE_HELP_ICON));
    m_imglist.Add(AfxGetApp()->LoadIcon(IDI_MUTE_ICON));

	m_optionsTreeCtrl.SetImageList(&m_imglist, TVSIL_NORMAL);
	
	SetStrings();

   // Create all options CDialogs
    m_MuteSettingsDlg.Create(IDD_MUTE_SETTINGS_DLG,FromHandle(this->m_hWnd));
    m_MuteAboutDlg.Create(IDD_MUTE_ABOUT_DLG,FromHandle(this->m_hWnd));
    m_MuteHelpDlg.Create(IDD_MUTE_WEB_DLG,FromHandle(this->m_hWnd));


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMuteOptionsDlg::OnTvnSelchangedOptionsTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

    if (bRefreshDlg==FALSE) return;	
	
	HTREEITEM hItem = m_optionsTreeCtrl.GetSelectedItem();
	CString strTemp;
    CString strItemText = m_optionsTreeCtrl.GetItemText(hItem);

	m_MuteSettingsDlg.ShowWindow(SW_HIDE);
	m_MuteHelpDlg.ShowWindow(SW_HIDE);
	m_MuteAboutDlg.ShowWindow(SW_HIDE);

	strTemp = m_ExtStr.LoadString(IDS_TAB_TITLE_SETTINGS_TEXT_ENG + g_unStringLanguageIdOffset );
	int nLen = strTemp.GetLength();
	if (strItemText ==_T(strTemp.GetBuffer(nLen)))
	{		
        m_MuteSettingsDlg.ShowWindow(SW_SHOW);		
	}

	strTemp = m_ExtStr.LoadString(IDS_MUTE_HELP_ENG + g_unStringLanguageIdOffset );
	nLen = strTemp.GetLength();

	if (strItemText ==_T(strTemp.GetBuffer(nLen)))
	{		
        m_MuteHelpDlg.ShowWindow(SW_SHOW);
	}
	
	strTemp = m_ExtStr.LoadString(IDS_MUTE_ABOUT_ENG+ g_unStringLanguageIdOffset );
	nLen = strTemp.GetLength();
    if (strItemText ==_T(strTemp.GetBuffer(nLen)))
	{		
	    m_MuteAboutDlg.ShowWindow(SW_SHOW);
	}
	
	*pResult = 0;
}

/*
//*******************************************************************
//  FUNCTION:    -	
//  RETURNS:     -	
//  PARAMETERS:  -	
//  COMMENTS:	 -	
//*******************************************************************
*/
void CMuteOptionsDlg::OnSize(UINT nType, int cx, int cy)
{
	CResizableDialog::OnSize(nType, cx, cy);

	// Make all options Dlg the same size
 

     CRect rectDlg;
	 GetClientRect(rectDlg);

	 HWND hwndTree=::GetDlgItem(this->m_hWnd,IDC_OPTIONS_TREE);
	 CWnd wndTree;
	 wndTree.FromHandle(hwndTree);
	 CRect rectTree;
	 ::GetWindowRect(hwndTree,rectTree);
	 ScreenToClient(rectTree);

	 rectDlg.left=rectDlg.left + rectTree.right+15;
	 rectDlg.top=rectTree.top;
    
	if(m_MuteSettingsDlg) m_MuteSettingsDlg.MoveWindow(rectDlg,0);
	if(m_MuteHelpDlg) m_MuteHelpDlg.MoveWindow(rectDlg,0);
	if(m_MuteAboutDlg) m_MuteAboutDlg.MoveWindow(rectDlg,0);
	Invalidate();
}


/*
//*******************************************************************
//  FUNCTION:    -	SetStrings()
//  RETURNS:     -	
//  PARAMETERS:  -	
//  COMMENTS:	 -	
//*******************************************************************
*/
void CMuteOptionsDlg::SetStrings()
{
	CString strTemp;

	bRefreshDlg=FALSE;
		
	m_optionsTreeCtrl.DeleteAllItems();

	strTemp = m_ExtStr.LoadString(IDS_TAB_TITLE_SETTINGS_TEXT_ENG + g_unStringLanguageIdOffset );
	int nLen = strTemp.GetLength(); 
	HTREEITEM hSettings =m_optionsTreeCtrl.InsertItem(_T(strTemp.GetBuffer(nLen)),0,0,TVI_ROOT,TVI_LAST);

	strTemp = m_ExtStr.LoadString(IDS_MUTE_HELP_ENG+ g_unStringLanguageIdOffset );
	nLen = strTemp.GetLength(); 
	HTREEITEM hHelp =m_optionsTreeCtrl.InsertItem(_T(strTemp.GetBuffer(nLen)),1,1,TVI_ROOT,TVI_LAST);  

	strTemp = m_ExtStr.LoadString(IDS_MUTE_ABOUT_ENG+ g_unStringLanguageIdOffset );
	nLen = strTemp.GetLength(); 
	HTREEITEM hAbout = m_optionsTreeCtrl.InsertItem(_T(strTemp.GetBuffer(nLen)),2,2,TVI_ROOT,TVI_LAST);

	if(m_MuteSettingsDlg) m_MuteSettingsDlg.ShowWindow(SW_SHOW);

	bRefreshDlg=TRUE;	
}

void CMuteOptionsDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{	
	CResizableDialog::OnShowWindow(bShow, nStatus);	

	if( FALSE == bShow )
	{
		// this will stop the animation when another tab is clicked..
		// no need in wasting clock cycles for the anim.. 
		m_MuteAboutDlg.ShowWindow(SW_HIDE);
	}
	else
	{
		// this will reshow the About Dialog if it was open when the 
		// options screen was closed last.. 
		HTREEITEM	hItem = m_optionsTreeCtrl.GetSelectedItem();
		CString		strTemp;
		CString		strItemText = m_optionsTreeCtrl.GetItemText(hItem);
		int			nLen;

		strTemp = m_ExtStr.LoadString(IDS_MUTE_ABOUT_ENG + g_unStringLanguageIdOffset );
		nLen = strTemp.GetLength();
		if (strItemText ==_T(strTemp.GetBuffer(nLen)))
		{		
			m_MuteAboutDlg.ShowWindow(SW_SHOW);
		}		

		strTemp = m_ExtStr.LoadString(IDS_TAB_TITLE_SETTINGS_TEXT_ENG + g_unStringLanguageIdOffset );
		nLen = strTemp.GetLength();
		if (strItemText ==_T(strTemp.GetBuffer(nLen)))
		{		
			m_MuteSettingsDlg.ShowWindow(SW_SHOW);		
		}

		strTemp = m_ExtStr.LoadString(IDS_MUTE_HELP_ENG + g_unStringLanguageIdOffset );
		nLen = strTemp.GetLength();

		if (strItemText ==_T(strTemp.GetBuffer(nLen)))
		{		
			m_MuteHelpDlg.ShowWindow(SW_SHOW);
		}		
	}	
}

void CMuteOptionsDlg::OnClose() 
{
	CResizableDialog::OnClose();	
}
