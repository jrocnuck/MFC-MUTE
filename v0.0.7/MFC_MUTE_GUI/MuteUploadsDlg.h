/* 02-14-2005 JROC -->

    -- On the Uploads screen, when files are selected, the DELETE key performs 
    the cancel/clear function.
*/    

#if !defined(AFX_MUTEUPLOADSDLG1_H__E5FD26E7_8146_4C22_99C8_65545553615C__INCLUDED_)
#define AFX_MUTEUPLOADSDLG1_H__E5FD26E7_8146_4C22_99C8_65545553615C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "minorGems/system/Thread.h"
#include "minorGems/system/MutexLock.h"
#include "minorGems/util/SimpleVector.h"

#include "MuteUploadListCtrl.h"
#include "Label.h"
#include "ResizableLib\ResizableDialog.h"
#include "ExternString.h"
#include "BtnST.h"

enum
{
	UPLOADS_UPDATE_EVENT = WM_USER + 4,	
};


/////////////////////////////////////////////////////////////////////////////
// CMuteUploadsDlg dialog

class CMuteUploadsDlg : public CResizableDialog
{
// Construction
public:
	void SetStrings();
	CMuteUploadsDlg(CWnd* pParent = NULL);   // standard constructor
	~CMuteUploadsDlg();
// Dialog Data
	//{{AFX_DATA(CMuteUploadsDlg)
	enum { IDD = IDD_MUTE_UPLOADS_DLG };		
	CMuteUploadListCtrl		m_UploadList;
	CButtonST					m_btnRemoveAllUpload;
	CButtonST					m_btnClearSelectedUpload;	
	//}}AFX_DATA
private:
	CLabel								m_oUploadsCntLabel;
	CLabel								m_oChunksCntLabel;	
	unsigned long						m_ulChunksUploaded;
	int									m_nDontUpdateCtr;	
	SimpleVector<CMuteUploadListItem *> *m_vecUploadItems;
	SimpleVector<long>					*mUploadListIndexVector;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMuteUploadsDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	afx_msg void OnTimer( UINT_PTR nIDEvent );
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CExternStr		m_ExtStr;
	// Generated message map functions
	//{{AFX_MSG(CMuteUploadsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg LRESULT OnUploadsUpdate(WPARAM, LPARAM);
	afx_msg LRESULT OnUploadCancel(WPARAM, LPARAM);
	afx_msg LRESULT OnUploadRemoveAll( WPARAM wParam, LPARAM lParam );	
	afx_msg void OnRemoveAllUploadsButton();
	afx_msg void OnCancelButton();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MUTEUPLOADSDLG1_H__E5FD26E7_8146_4C22_99C8_65545553615C__INCLUDED_)
