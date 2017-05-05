#if !defined(AFX_MUTESETTINGSDLG_H__3E640ADD_B3CF_42B7_A191_AC59934E8548__INCLUDED_)
#define AFX_MUTESETTINGSDLG_H__3E640ADD_B3CF_42B7_A191_AC59934E8548__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MuteSettingsDlg.h : header file
//

#include "ResizableLib\ResizableDialog.h"
#include "NumberEdit.h"
#include "IconStatic.h"
#include "Label.h"
#include "ComboBoxBold.h"
#include "ExternString.h"

/////////////////////////////////////////////////////////////////////////////
// CMuteSettingsDlg dialog

class CMuteSettingsDlg : public CResizableDialog
{
// Construction
public:
	CMuteSettingsDlg(CWnd* pParent = NULL);   // standard constructor
	
	static	CString		m_static_strDir;
// Dialog Data
	//{{AFX_DATA(CMuteSettingsDlg)
	enum { IDD = IDD_MUTE_SETTINGS_DLG };
	CComboBoxBold	m_ctrlCBLanguage;
	CComboBox	m_ctrlComboDeleteCanceledDLs;
	CString		m_strShareDirectory;
	CString		m_strIncomingDirectory;
	CString		m_strHashDirectory;
	CString		m_strInboundLimit;
	CNumberEdit	m_oInboundLimit;
	CString		m_strOutboundLimit;
	CNumberEdit	m_oOutboundLimit;
	CString		m_strInitialDownloadTimeout;
	CNumberEdit	m_oInitialDownloadTimeout;
	CString		m_strMaintainConnections;
	CNumberEdit	m_oMaintainConnections;
	CString		m_strAllowString;
	CNumberEdit	m_oAllowConnections;
	CNumberEdit	m_oMaxUploads;
	CNumberEdit m_oMaxUploadsPerVIP;
	CString		m_strMaxUploads;
	CString		m_strMaxUploadsPerVIP;
	int			m_nDeleteFromIncoming;
	CIconStatic m_oMsgLimitsGroup;
	CIconStatic m_oDownloadsGroup;
	CIconStatic m_oConnectionsGroup;
	CIconStatic m_oUploadsGroup;	
	CIconStatic m_oLanguageGroup;
	CString		m_strExtLanguageName;
	CString		m_strLanguageName;
	CIconStatic m_oPortGroup;
	CString		m_strPort;
	CNumberEdit	m_oPort;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMuteSettingsDlg)
	public:
	protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CExternStr m_ExtStr;
	CString m_strLangGroupIconPath;

	static int CALLBACK BrowseCallbackProc( HWND hwnd,  UINT uMsg, LPARAM lParam, LPARAM lpData );
	// Generated message map functions
	//{{AFX_MSG(CMuteSettingsDlg)
	afx_msg void OnBrowseSharedFolder();
	afx_msg void OnBrowseIncomingFolder();
	afx_msg void OnBrowseHashFolder();
	virtual BOOL OnInitDialog();
	afx_msg void OnSaveSettingsButton();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


//public:

		
	
	CString GetLanguageFilePath(CString m_strLanguageName);	
	void InitCurrentLanguage();
	void SetStrings();
	void ListExternLanguages();
	bool bExternSel;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MUTESETTINGSDLG_H__3E640ADD_B3CF_42B7_A191_AC59934E8548__INCLUDED_)
