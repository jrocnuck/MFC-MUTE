#if !defined(AFX_TIMEDDLG_H__3C63C5BA_970A_4995_A778_4FC81356B55C__INCLUDED_)
#define AFX_TIMEDDLG_H__3C63C5BA_970A_4995_A778_4FC81356B55C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TimedDlg.h : header file
//

#include "stdafx.h"
#include "Label.h"
#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// CTimedDlg dialog

class CTimedDlg : public CDialog
{
// Construction
public:
	CTimedDlg( CWnd* pParent = NULL );   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTimedDlg)
	enum { IDD = IDD_TIMED_DLG };
	CLabel	m_objTimedDlgText;
	CProgressCtrl	m_objProgress;	
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTimedDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	static void PumpMessages();
	void CloseDlgWindow();
	void DisplayWindow();
	void HideWindow();	

protected:
	BOOL m_bReadyToClose;	

	CWnd *m_pParentWnd;

	// Generated message map functions
	//{{AFX_MSG(CTimedDlg)
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TIMEDDLG_H__3C63C5BA_970A_4995_A778_4FC81356B55C__INCLUDED_)
