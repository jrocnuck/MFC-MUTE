// GenerateKeyPairDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MFC_MUTE_GUI.h"
#include "GenerateKeyPairDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGenerateKeyPairDlg dialog


CGenerateKeyPairDlg::CGenerateKeyPairDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGenerateKeyPairDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGenerateKeyPairDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_NodePublicKey = NULL;
    m_NodePrivateKey = NULL;
}


void CGenerateKeyPairDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGenerateKeyPairDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGenerateKeyPairDlg, CDialog)
	//{{AFX_MSG_MAP(CGenerateKeyPairDlg)
	ON_WM_SHOWWINDOW()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGenerateKeyPairDlg message handlers

BOOL CGenerateKeyPairDlg::OnInitDialog() 
{
	CString	strTemp;
	CDialog::OnInitDialog();
	
	strTemp = m_ExtStr.LoadString( IDS_GEN_KEY_PAIR_WINDOW_TITLE_ENG + g_unStringLanguageIdOffset );
	SetWindowText( strTemp );

	strTemp = m_ExtStr.LoadString( IDS_GEN_KEY_PAIR_WINDOW_MAIN_TEXT_ENG + g_unStringLanguageIdOffset );
	GetDlgItem( IDC_STATIC_HOW_LONG )->SetWindowText( strTemp );

	SetTimer(666,2000,NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGenerateKeyPairDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
}

void CGenerateKeyPairDlg::OnTimer(UINT nIDEvent) 
{
	if( 666 == nIDEvent )
	{
		KillTimer(666);
		DWORD dwStart;
		int		nSeconds;
		CString	str;
		CString	strTemp;
		
		dwStart = GetTickCount();
		CryptoUtils::generateRSAKey( m_keyLength, &m_NodePublicKey, &m_NodePrivateKey );
		
		nSeconds = abs(GetTickCount() - dwStart);
		strTemp = m_ExtStr.LoadString( IDS_KEY_GENERATION_TIME_TEXT_ENG + g_unStringLanguageIdOffset );
		str.Format( strTemp, nSeconds / 1000 );
		GetDlgItem(IDC_STATIC_HOW_LONG)->SetWindowText( str );
		GetDlgItem(IDOK)->EnableWindow(TRUE);
	}
	CDialog::OnTimer(nIDEvent);
}


