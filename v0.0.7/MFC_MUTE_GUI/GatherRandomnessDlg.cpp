// GatherRandomnessDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MFC_MUTE_GUI.h"
#include "GatherRandomnessDlg.h"
#include "ExternString.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGatherRandomnessDlg dialog


CGatherRandomnessDlg::CGatherRandomnessDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGatherRandomnessDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGatherRandomnessDlg)
	m_strRandomnessString = _T("");
	//}}AFX_DATA_INIT
}


void CGatherRandomnessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGatherRandomnessDlg)
	DDX_Text(pDX, IDC_EDIT_RANDOMNESS, m_strRandomnessString);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGatherRandomnessDlg, CDialog)
	//{{AFX_MSG_MAP(CGatherRandomnessDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGatherRandomnessDlg message handlers

BOOL CGatherRandomnessDlg::OnInitDialog() 
{
	CString	strTemp;
	CExternStr m_ExtStr;

	CDialog::OnInitDialog();
	
	strTemp = m_ExtStr.LoadString(IDS_GATHERING_RANDOMNESS_CAPTION_ENG + g_unStringLanguageIdOffset );
	SetWindowText( strTemp );
	
	strTemp = m_ExtStr.LoadString(IDS_GATHERING_RANDOMNESS_TEXT_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_STATIC_RANDOMNESS_TEXT )->SetWindowText( strTemp );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
