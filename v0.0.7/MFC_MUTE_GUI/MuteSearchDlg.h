#if !defined(AFX_MUTESEARCHDLG_H__3CCC811C_5408_4ADE_A9BC_29E6B9BA087D__INCLUDED_)
#define AFX_MUTESEARCHDLG_H__3CCC811C_5408_4ADE_A9BC_29E6B9BA087D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MuteSearchListCtrl.h"

#include "minorGems/system/MutexLock.h"
#include "minorGems/system/Thread.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SimpleVector.h"

#include "ResizableLib\ResizableDialog.h"
#include "Led.h"
#include "Label.h"
#include "StrParse.h"
#include "NumberEdit.h"
#include "BtnST.h"

#include "SearchThread.h"
#include "ExternString.h"


enum
{
	SEARCH_TIMEOUT_EVENT = WM_USER + 4,
	SEARCH_RESULTS_EVENT,
	SEARCH_STOP_EVENT
};

enum
{
	SEARCH_BOOLEAN_AND = 0,
	SEARCH_BOOLEAN_OR,
	SEARCH_BOOLEAN_ENTIRE_PHRASE
};

enum
{
	SEARCH_FILESIZE_ATLEAST = 0,
	SEARCH_FILESIZE_ATMOST
};


enum {
    MUTE_SEARCH_DO_DOWNLOAD_EVENT = WM_USER+400,
};



// MuteSearchDlg.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CMuteSearchDlg dialog

class CMuteSearchDlg : public CResizableDialog
{
// Construction
public:
	void SetStrings();
	CMuteSearchDlg(CWnd* pParent = NULL);   // standard constructor
	~CMuteSearchDlg();

// Dialog Data
	//{{AFX_DATA(CMuteSearchDlg)
	enum { IDD = IDD_MUTE_SEARCH_DLG };
	CComboBox	m_comboSearchBoolean;
	CComboBox	m_comboFileSize;
	CLed				m_oSearchingLED;
	CStatic				m_staticSearchingText;
	CLabel				m_oSearchForLabel;
	CLabel				m_oExcludeLabel;
	CEdit				m_SearchEdit;	
	CMuteSearchListCtrl	m_SearchList;
	CString				m_strSearchEdit; // this is the word(s) to search for text
	CString				m_strSearchExcludeEdit; // filter out search results with these words in them
	CNumberEdit			m_oFileSizeEditCtrl;
	CString				m_strFileSizeEdit; // text of file size filter in kilo bytes
	int					m_nSearchBoolean; // this is the index of the combo box for 
	int					m_nFileSizeBoolean; // this is the index of the combo File Size Boolean
	CButtonST			m_btnPurgeHashFolder;
	CButtonST			m_btnExportResultsButton;
	CButtonST			m_btnClearResultsButton;
	CButtonST			m_btnDownloadButton;
	//}}AFX_DATA
	vector<CMuteSearchListItem>	m_vecSearchItems;

	/**
    * Cancels any active search.
    *
    * Must be called before shutting down MUTE filesharing layer.
    */
    void cancelActiveSearch();

//	char mPrintSearchSyncTrace;

	MutexLock *mSearchLock;
	char mSearchActive;
	char mSearchCanceled;
	
	MutexLock *mResultLock;
	SimpleVector<char *> *mResultsFromAddresses;
	SimpleVector<char *> *mResultsFilePaths;
	SimpleVector<char *> *mResultsFileHashes;
    SimpleVector<unsigned long> *mResultsFileSizes;
	inline SetPtrToDownloadWindow( CDialog *pDlg ) { m_pDownloadDialog = pDlg; }
	bool DoWeKeepSearchResult( CMuteSearchListItem& item );

	afx_msg void OnDownloadButton();
	afx_msg void OnClearResults();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMuteSearchDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:
	vector<SearchThread *>	m_vecSearchThread;
	CDialog					*m_pDownloadDialog;
	CStringParser			m_oSearchParser;
	CStringParser			m_oSearchExcludeParser;
	CExternStr              m_ExtStr;
	
	// Generated message map functions
	//{{AFX_MSG(CMuteSearchDlg)
	afx_msg void OnClose();
	afx_msg void OnSearchButton();
	afx_msg void OnStopButton();
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnSearchTimeout(WPARAM, LPARAM); 
	afx_msg LRESULT OnSearchResults(WPARAM, LPARAM); 	
	afx_msg LRESULT DoStopSearch(WPARAM, LPARAM); 
	afx_msg LRESULT DoDownload(WPARAM, LPARAM);
	afx_msg void OnExportSearchResultsButton();
	afx_msg void OnPurgeHashFolder();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MUTESEARCHDLG_H__3CCC811C_5408_4ADE_A9BC_29E6B9BA087D__INCLUDED_)
