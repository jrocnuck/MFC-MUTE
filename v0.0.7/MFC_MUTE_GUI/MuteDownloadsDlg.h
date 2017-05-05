/* 02-14-2005 JROC -->

    -- Fixed code that adds downloads from the Search window that were already listed in the
    download queue.  After further testing of version 0.0.3, I found that this code was broken
    and duplicate items would show up in the download list.
    -- Increased semaphore timeout values in Downloads Dialog code.
    -- On the Downloads screen, when files are selected, the DELETE key performs the cancel/clear 
    function.
*/
#if !defined(AFX_MUTEDOWNLOADSDLG_H__CA7C7F8A_20B6_4D99_8CDF_E7C0A5177003__INCLUDED_)
#define AFX_MUTEDOWNLOADSDLG_H__CA7C7F8A_20B6_4D99_8CDF_E7C0A5177003__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MuteDownloadsDlg.h : header file
//

#include "minorGems/system/Thread.h"
#include "minorGems/system/MutexLock.h"
#include "minorGems/io/file/File.h"

#include <stdio.h>


#include "minorGems/util/SimpleVector.h"

#include "MuteDownloadQueue.h"
#include "MuteDownloadListCtrl.h"
#include "ResizableLib\ResizableDialog.h"
#include "BtnST.h"



// BECAREFUL NOT TO OVERLAP MESSAGE ID's IN THE LIST CONTROLS
enum DOWNLOADGUI_MESSAGE_IDS
{
	DOWNLOAD_DIALOG_PREPARE_FOR_CLOSE  = (WM_USER+300),
	DOWNLOAD_QUEUED_ITEM_IS_QUEUED,
	DOWNLOAD_QUEUED_ITEM_IS_SEARCHING
};


/////////////////////////////////////////////////////////////////////////////
// CMuteDownloadsDlg dialog

class CMuteDownloadsDlg : public CResizableDialog
{

// Construction
public:
	void SetStrings();
	CMuteDownloadsDlg(CWnd* pParent = NULL);   // standard constructor
	
	~CMuteDownloadsDlg();
	/**
	* Adds a download to this panel.
	*
	* @param inFromAddress the virtual address of the host to download
	*   from.  Must be destroyed by caller.
	* @param inFilePath the file path to download.
	*   Must be destroyed by caller.
	* @param inFileHash the SHA1 hash of the file.
	*   Must be destroyed by caller.
	*/
	
	void * GetPtrToListItemFromHash( const char *inFileHash );
	void * GetPtrToListItemFromLocalFileName( const char *inLocalFilename );

	// call this after download is queued... 
	void addDownload( const char *inFromAddress, const char *inLocalFilePath, const char *inRemoteFilePath,
		const char *inFileHash, bool bIsAResumedFile = false, unsigned int unNumResumes = 0 );

	// call this when coming from Search Dialog window..
	// this function will add to download queue and add the download to the downloads screen...
	void addDownloadFromSearch( const char *inFromAddress, const char *inFilePath, const char *inFileHash );
	
	// call this when coming from Search Dialog window..
	// this function will add to download queue and add the download to the downloads screen...
	void * addDownloadFromQueue( const char *inLocalFilePath, const char *inFileHash);

	// called from the MuteDownloadQueue class when a download is resumed.. 
	bool ResumeFromQueue( CString& strLocalFilename, CString& strRemoteFilename, CString& strHash, CString& strVIP );

    /**
    * Gets whether any downloads in this panel are still active.
    *
    * If downloads are active, they should be canceled by the user
    * before this panel is destroyed (for example, before app shutdown).
    *
    * @return true if downloads are still active.
    */         
    char areDownloadsActive();

// Dialog Data
	//{{AFX_DATA(CMuteDownloadsDlg)
	enum { IDD = IDD_MUTE_DOWNLOADS_DLG };	
	CMuteDownloadListCtrl		m_DownloadList;
	CButtonST					m_btnExploreSharedFiles;
	CButtonST					m_btnExploreIncomingFiles;
	CButtonST					m_btnPurgeIncomingFolder;
	CButtonST					m_btnClearCompletedButton;
	CButtonST					m_btnClearFailedButton;
	CButtonST					m_btnCancelButton;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMuteDownloadsDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMuteDownloadsDlg)
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnPrepareForClose( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnClearComplete( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnClearStalled( WPARAM wParam, LPARAM lParam );	
	afx_msg LRESULT OnDownloadCancel( WPARAM wParam, LPARAM lParam );	
	afx_msg LRESULT OnSetDownloadToQueued( WPARAM wParam, LPARAM lParam );	
	afx_msg LRESULT OnSetItemStatusTextToQueued( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSetItemStatusTextToSearching( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnRemoveDownloadFromQueue( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnClearResumedDownload( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSearchQueueItemNext(  WPARAM wParam, LPARAM lParam );
	afx_msg void OnCancelButton();
	afx_msg void OnClearFailedButton();
	afx_msg void OnClearCompletedButton();
	afx_msg void OnClose();
	afx_msg void OnExploreSharedFolder();
	afx_msg void OnPurgeIncomingFolder();
	afx_msg void OnExploreIncomingFolder();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	SimpleVector<CMuteDownloadListItem *>	*mDownloadItems;
	CDownloadQueue							*m_pDLQueue;
	CExternStr								m_ExtStr;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MUTEDOWNLOADSDLG_H__CA7C7F8A_20B6_4D99_8CDF_E7C0A5177003__INCLUDED_)
