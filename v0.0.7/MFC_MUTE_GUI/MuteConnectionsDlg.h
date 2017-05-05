#if !defined(AFX_MUTECONNECTIONSDLG_H__5F96E479_D8E1_46EA_9B0E_0758409ED91D__INCLUDED_)
#define AFX_MUTECONNECTIONSDLG_H__5F96E479_D8E1_46EA_9B0E_0758409ED91D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MuteConnectionsDlg.h : header file
//

#include "MUTEConnectionListCtrl.h"
#include "minorGems/system/Thread.h"
#include "minorGems/system/MutexLock.h"
#include "ResizableLib\ResizableDialog.h"
#include "ExternString.h"
#include "Label.h"
#include "BtnST.h"
#include "IconStatic.h"


enum
{
	 CONNECTIONS_UPDATE_EVENT = WM_USER + 4,
	 IN_OUT_STATS_UPDATE_EVENT = WM_USER + 21000
};


/////////////////////////////////////////////////////////////////////////////
// CMuteConnectionsDlg dialog

class CMuteConnectionsDlg : public CResizableDialog
{
// Construction
public:
	void SetStrings();
	CMuteConnectionsDlg(CWnd* pParent = NULL);   // standard constructor
	~CMuteConnectionsDlg(){}
	// Dialog Data
	//{{AFX_DATA(CMuteConnectionsDlg)
	enum { IDD = IDD_MUTE_CONNECTIONS_DLG };
	CStatic	m_staticStatusText;
	CLabel	m_oInOutStatsLabelLeft1;
	CLabel	m_oInOutStatsLabelLeft2;
	CLabel	m_oInOutStatsLabelLeft3;
	CLabel	m_oInOutStatsLabelLeft4;
	CLabel	m_oInOutStatsLabelLeft5;

	CLabel	m_oInOutStatsLabelRight1;
	CLabel	m_oInOutStatsLabelRight2;
	CLabel	m_oInOutStatsLabelRight3;
	CLabel	m_oInOutStatsLabelRight4;
	CLabel	m_oInOutStatsLabelRight5;

	CMUTEConnectionListCtrl	m_ConnectionList;
	CString	m_strAddressEdit;
	CString	m_strPortEdit;
	CIconStatic m_oAddHostGroup;
	CIconStatic m_oHostUpdateGroup;
	CIconStatic m_oMyInfoGroup;
	//}}AFX_DATA
	vector<CMUTEConnectionListItem>	m_vecListItems;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMuteConnectionsDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:    
	CExternStr			m_ExtStr;
	unsigned long		m_ulLastNumOfConnections;

	void OnConnectionsUpdate(); 
	void SaveConnections();

	// Generated message map functions
	//{{AFX_MSG(CMuteConnectionsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnAddHostButton();
	afx_msg void OnClose();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg LRESULT OnInOutStatsUpdate(WPARAM, LPARAM);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MUTECONNECTIONSDLG_H__5F96E479_D8E1_46EA_9B0E_0758409ED91D__INCLUDED_)
