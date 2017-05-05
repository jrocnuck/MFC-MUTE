//{{AFX_INCLUDES()
#include "webbrowser2.h"
//}}AFX_INCLUDES
#if !defined(AFX_MUTEHELPDLG_H__C8BBA102_CC15_404B_A55C_C821E0895A6E__INCLUDED_)
#define AFX_MUTEHELPDLG_H__C8BBA102_CC15_404B_A55C_C821E0895A6E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MuteHelpDlg.h : header file
//
#include "ResizableLib\ResizableDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CMuteHelpDlg dialog

class CMuteHelpDlg : public CResizableDialog
{
// Construction
public:
	CMuteHelpDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMuteHelpDlg)
	enum { IDD = IDD_MUTE_WEB_DLG };
	CButton	m_btnNext;
	CButton	m_btnBack;
	CWebBrowser2	m_browser;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMuteHelpDlg)
	protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int index;
	int maxindex;
    CBitmap m_bitmapBack;
    CBitmap m_bitmapNext;

	// Generated message map functions
	//{{AFX_MSG(CMuteHelpDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnButtonBack();
	afx_msg void OnButtonNext();	
	afx_msg void OnCommandStateChangeExplorer1(long Command, BOOL Enable);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MUTEHELPDLG_H__C8BBA102_CC15_404B_A55C_C821E0895A6E__INCLUDED_)
