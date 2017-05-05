/* 02-14-2005 JROC -->
    -- In the Shared Files refresh code, added a 1 millisecond sleep call to help prevent there
    thread from starving other threads of CPU cycles.
*/    

#if !defined(AFX_MUTESHAREDFILESDLG1_H__E5FD26E7_8146_4C22_99C8_65545553615C__INCLUDED_)
#define AFX_MUTESHAREDFILESDLG1_H__E5FD26E7_8146_4C22_99C8_65545553615C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MuteSharedFilesDlg.h : header file
//

#include "minorGems/system/Thread.h"
#include "minorGems/system/MutexLock.h"
#include "minorGems/util/SimpleVector.h"

#include "MuteSharedFilesListCtrl.h"
#include "ResizableLib\ResizableDialog.h"
#include "ExternString.h"
#include "BtnST.h"

enum
{
	SHARED_FILES_UPDATE_EVENT = WM_USER + 5,
};


/////////////////////////////////////////////////////////////////////////////
// CMuteSharedFilesDlg dialog

class CMuteSharedFilesDlg : public CResizableDialog
{
// Construction
public:
	void SetStrings();
	CMuteSharedFilesDlg(CWnd* pParent = NULL);   // standard constructor
	~CMuteSharedFilesDlg();
// Dialog Data
	//{{AFX_DATA(CMuteSharedFilesDlg)
	enum { IDD = IDD_MUTE_SHARED_FILES_DLG };
		// NOTE: the ClassWizard will add data members here
	CMuteSharedFilesListCtrl		m_SharedFilesList;
	CButtonST						m_btnRefresh;
	CButtonST						m_btnExportlist;
	//}}AFX_DATA
private:			
	vector<CMuteSharedFileListItem *>		m_vecSharedFileItems;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMuteSharedFilesDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CExternStr	m_ExtStr;	

	static bool m_bAppIsClosing;
	// Generated message map functions
	//{{AFX_MSG(CMuteSharedFilesDlg)	
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg LRESULT OnSharedFileUpdate(WPARAM, LPARAM);	
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:	
	afx_msg void OnBnClickedExportSearchResultsButton();
	afx_msg void OnBnClickedRefresh();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MUTESHAREDFILESDLG1_H__E5FD26E7_8146_4C22_99C8_65545553615C__INCLUDED_)
