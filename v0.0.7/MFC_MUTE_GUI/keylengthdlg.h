#if !defined(AFX_KEYLENGTHDLG_H__45D555DC_401C_4F93_B4EA_77BF0354DE9C__INCLUDED_)
#define AFX_KEYLENGTHDLG_H__45D555DC_401C_4F93_B4EA_77BF0354DE9C__INCLUDED_

#include "ExternString.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// KeyLengthDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CKeyLengthDlg dialog

class CKeyLengthDlg : public CDialog
{
// Construction
public:
	CExternStr m_ExtStr;
	CKeyLengthDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CKeyLengthDlg)
	enum { IDD = IDD_KEY_LENGTH_DLG };
	CListBox	m_ctrlKeyLengthLB;
	int			m_nKeyLength;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeyLengthDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CKeyLengthDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnSelchangeKeyLengthList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KEYLENGTHDLG_H__45D555DC_401C_4F93_B4EA_77BF0354DE9C__INCLUDED_)
