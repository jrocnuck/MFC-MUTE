// KeyLengthDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MFC_MUTE_GUI.h"
#include "KeyLengthDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKeyLengthDlg dialog


CKeyLengthDlg::CKeyLengthDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CKeyLengthDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CKeyLengthDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CKeyLengthDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CKeyLengthDlg)
	DDX_Control(pDX, IDC_KEY_LENGTH_LIST, m_ctrlKeyLengthLB);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CKeyLengthDlg, CDialog)
	//{{AFX_MSG_MAP(CKeyLengthDlg)
	ON_WM_CLOSE()
	ON_LBN_SELCHANGE(IDC_KEY_LENGTH_LIST, OnSelchangeKeyLengthList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeyLengthDlg message handlers

BOOL CKeyLengthDlg::OnInitDialog() 
{
	CString	strTemp1,strTemp2,strTemp3,strTemp4,strTemp5;

	CDialog::OnInitDialog();
	
	strTemp1 = m_ExtStr.LoadString( IDS_512BITS_ENG + g_unStringLanguageIdOffset );
	strTemp2 = m_ExtStr.LoadString( IDS_1024BITS_ENG + g_unStringLanguageIdOffset );
	strTemp3 = m_ExtStr.LoadString( IDS_1536BITS_ENG + g_unStringLanguageIdOffset );
	strTemp4 = m_ExtStr.LoadString( IDS_2048BITS_ENG + g_unStringLanguageIdOffset );
	strTemp5 = m_ExtStr.LoadString( IDS_4096BITS_ENG + g_unStringLanguageIdOffset );

	m_ctrlKeyLengthLB.AddString( strTemp1 );
	m_ctrlKeyLengthLB.AddString( strTemp2 );
	m_ctrlKeyLengthLB.AddString( strTemp3 );
	m_ctrlKeyLengthLB.AddString( strTemp4 );
	m_ctrlKeyLengthLB.AddString( strTemp5 );

	m_nKeyLength = 1;
	m_ctrlKeyLengthLB.SetCurSel(m_nKeyLength);
	
	strTemp1 = m_ExtStr.LoadString( IDS_KEY_GENERATION_SELECT_SIZE_ENG + g_unStringLanguageIdOffset );
	GetDlgItem(IDC_STATIC_KEY_GENERATION_SELECT_SIZE)->SetWindowText( strTemp1 );

	strTemp1 = m_ExtStr.LoadString( IDS_KEY_GENERATION_DLG_CAPTION_ENG + g_unStringLanguageIdOffset );
	SetWindowText( strTemp1 );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CKeyLengthDlg::OnClose() 
{
	CDialog::OnClose();
}

void CKeyLengthDlg::OnSelchangeKeyLengthList() 
{
	m_nKeyLength = m_ctrlKeyLengthLB.GetCurSel();	
}
