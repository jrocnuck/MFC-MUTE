#include "MUTE/common/CryptoUtils.h"
#include "ExternString.h"

#if !defined(AFX_GENERATEKEYPAIRDLG_H__19133D8A_BC09_4DB7_9E43_5133350D76EE__INCLUDED_)
#define AFX_GENERATEKEYPAIRDLG_H__19133D8A_BC09_4DB7_9E43_5133350D76EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GenerateKeyPairDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGenerateKeyPairDlg dialog

class CGenerateKeyPairDlg : public CDialog
{
// Construction
public:
	CExternStr m_ExtStr;
	CGenerateKeyPairDlg(CWnd* pParent = NULL);   // standard constructor
	~CGenerateKeyPairDlg()
	{
		if( NULL != m_NodePublicKey )
		{
			delete [] m_NodePublicKey;
		}
		if( NULL != m_NodePrivateKey )
		{
			delete [] m_NodePrivateKey;
		}
	}

// Dialog Data
	//{{AFX_DATA(CGenerateKeyPairDlg)
	enum { IDD = IDD_GENERATE_KEY_PAIR_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	int		m_keyLength;
	char *	m_NodePublicKey;
    char *	m_NodePrivateKey;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGenerateKeyPairDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGenerateKeyPairDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GENERATEKEYPAIRDLG_H__19133D8A_BC09_4DB7_9E43_5133350D76EE__INCLUDED_)
