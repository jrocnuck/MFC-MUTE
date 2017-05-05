#if !defined(AFX_MUTEABOUTDLG_H__BA59E188_3B35_4514_8CE7_3DA80D38150D__INCLUDED_)
#define AFX_MUTEABOUTDLG_H__BA59E188_3B35_4514_8CE7_3DA80D38150D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MuteAboutDlg.h : header file
//
#include "ResizableLib\ResizableDialog.h"
#include "AboutCtrl.h"
#include "Label.h" 

/////////////////////////////////////////////////////////////////////////////
// CMuteAboutDlg dialog

class CMuteAboutDlg : public CResizableDialog
{
// Construction
public:
	void SetStrings();
	CMuteAboutDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMuteAboutDlg)
	enum { IDD = IDD_MUTE_ABOUT_DLG };
	CAboutCtrl	m_objCredits;
	CButton	m_oWebResourcesGroup;
	CLabel	m_oMuteHomePageLink;
	CLabel	m_oMuteSFMainHomePageLink;
	CLabel	m_oXybaronHomePageLink;
	CLabel	m_oPlanetPeerHomePageLink;
	CLabel	m_oMyMuteInfoHomePageLink;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMuteAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMuteAboutDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MUTEABOUTDLG_H__BA59E188_3B35_4514_8CE7_3DA80D38150D__INCLUDED_)
