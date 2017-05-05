#if !defined(AFX_MUTEOPTIONSDLG_H__4CCC811C_5408_4ADE_A9BC_29E6B9BA087D__INCLUDED_)
#define AFX_MUTEOPTIONSDLG_H__4CCC811C_5408_4ADE_A9BC_29E6B9BA087D__INCLUDED_


#pragma once

#if defined (_MSC_VER) && (_MSC_VER == 1310)
// NEED THESE FOR VISUAL STUDIO .NET 2003
#include "c:\program files\microsoft visual studio .net 2003\vc7\atlmfc\include\afxcmn.h"
#include "c:\program files\microsoft visual studio .net 2003\vc7\atlmfc\include\afxwin.h"
#endif

#include "Label.h"
#include "ResizableLib\ResizableDialog.h"
#include "MuteSettingsDlg.h"
#include "MuteHelpDlg.h"
#include "MuteAboutDlg.h"
#include "ExternString.h"

// CMuteOptionsDlg dialog

class CMuteOptionsDlg : public CResizableDialog
{
	DECLARE_DYNAMIC(CMuteOptionsDlg)

public:
	CMuteOptionsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMuteOptionsDlg();

// Dialog Data
	// Dialog Data
	//{{AFX_DATA(CMuteSearchDlg)
	enum { IDD = IDD_MUTE_OPTIONS_DLG };
	CTreeCtrl m_optionsTreeCtrl;	
	//}}AFX_DATA




protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CMuteSettingsDlg m_MuteSettingsDlg;
	CMuteAboutDlg m_MuteAboutDlg;
	CMuteHelpDlg m_MuteHelpDlg;
	CExternStr m_ExtStr;

	// Generated message map functions
	//{{AFX_MSG(CMuteOptionsDlg)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnTvnSelchangedOptionsTree(NMHDR *pNMHDR, LRESULT *pResult);	
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	BOOL bRefreshDlg;
	void SetStrings();
	virtual BOOL OnInitDialog();	
	CImageList m_imglist;
	
};





//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // #if !defined(AFX_MUTEOPTIONSDLG_H__4CCC811C_5408_4ADE_A9BC_29E6B9BA087D__INCLUDED_)
