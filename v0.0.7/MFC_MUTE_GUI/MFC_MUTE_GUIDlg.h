// MUTEMFC2Dlg.h : header file
//

#if !defined(AFX_MUTEMFC2DLG_H__37736F54_1FD7_498D_BFA6_1B322527C2F6__INCLUDED_)
#define AFX_MUTEMFC2DLG_H__37736F54_1FD7_498D_BFA6_1B322527C2F6__INCLUDED_

#include "MuteMainTabCtrl.h"
#include "TitleMenu.h"
#include "ResizableLib\ResizableDialog.h"
#include "DialogMinTrayBtn.h"
#include "Label.h"
#include "ExternString.h"
#include "IconStatic.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MUTE_MAIN_GUI_UPDATE_CONNECTION_STAT_MSG	(WM_USER+2000)

/////////////////////////////////////////////////////////////////////////////
// CMUTEMFC2Dlg dialog

class CMUTEMFC2Dlg : public CDialogMinTrayBtn<CResizableDialog>
{
// Construction
protected:
	typedef CDialogMinTrayBtn<CResizableDialog> CMUTEMFC2DialogBase;

public:
	void SetStrings();
	CMUTEMFC2Dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CMUTEMFC2Dlg)
	enum { IDD = IDD_MUTEMFC2_DIALOG };
		// NOTE: the ClassWizard will add data members here
	CMuteMainDlgTabCtrl		m_oTabCtrl;	
	CStatusBarCtrl  		m_Statusbar;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMUTEMFC2Dlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	CExternStr m_ExtStr;
	// Generated message map functions

	void RemoveTrayIcon();
	void CleanupAndExit();
	void SaveWindowPlacement();
	void LoadWindowPlacement();
	void ShowTransferRate();

	//{{AFX_MSG(CMUTEMFC2Dlg)
	afx_msg void TrayRestore();	
	afx_msg void TrayQuit();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);	
	afx_msg LRESULT AddTrayIcon(WPARAM, LPARAM lParam);
	afx_msg LRESULT UpdateConnectionStatus(WPARAM, LPARAM lParam);
	afx_msg LONG OnTrayMessage( WPARAM wparam, LPARAM lparam );
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg BOOL OnQueryEndSession();
    afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:	
	bool				m_bIsTrayIconActive;
	bool				m_bdoubleclicked;
	bool				m_bQuitingAppDialogIsShowing;
	CTitleMenu			m_TrayIconMenu;
	unsigned long		m_ulLastNumOfConnections;
	time_t				m_timeStart;
	CString				m_strBaseWindowText;
	HICON			connicons[3];
	HICON			transicons[4];
	HICON			imicons[3];
	HICON			usericon;

	//uint32		lastuprate;
	//uint32		lastdownrate;
public:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MUTEMFC2DLG_H__37736F54_1FD7_498D_BFA6_1B322527C2F6__INCLUDED_)


