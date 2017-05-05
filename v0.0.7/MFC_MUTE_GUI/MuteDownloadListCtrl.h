
#pragma once
#include "stdafx.h"
#include <vector>
#include "TitleMenu.h"
#include "MuleListCtrl.h"
#include "ExternString.h"

#include "minorGems/system/Thread.h"
#include "minorGems/system/MutexLock.h"
#include "minorGems/io/file/File.h"

#include "minorGems/crypto/hashes/sha1.h"

#include <stdio.h>


using namespace std;


enum MENU_ENUMS
{	
	MP_CLEAR = MP_CANCEL + 1,
	MP_CLEAR_COMPLETE,
	MP_CLEAR_STALLED,
	MP_SEARCH_QUEUE_ITEM_NEXT
};

enum E_DL_ITEM_STATE
{
	E_ITEM_QUEUED,
	E_ITEM_QUEUED_SEARCHING,
	E_ITEM_ATTEMPTING_RESUME,
	E_ITEM_RESUMING,
	E_ITEM_FETCHING_INFO,
	E_ITEM_CANCELED,
	E_ITEM_CANCELLING,
	E_ITEM_FAILED,
	E_ITEM_FAILED_TO_WRITE_LOCAL_FILE,
	E_ITEM_ACTIVE,
	E_ITEM_CORRUPT,
	E_ITEM_NOT_FOUND,
	E_ITEM_COMPLETE
};

// IDs for various controls and event generators
enum {
    DOWNLOAD_PROGRESS_EVENT = WM_USER+4,
    DOWNLOAD_RESULT_EVENT,
    DOWNLOAD_CHUNK_RETRY_EVENT,
    DOWNLOAD_CHUNK_RECEIVED_EVENT,
	DOWNLOAD_CANCEL_EVENT,
	DOWNLOAD_CLEAR_COMPLETE_EVENT,
	DOWNLOAD_CLEAR_STALLED_EVENT,
	DOWNLOAD_RESULT_SET_TO_QUEUED,
	DOWNLOAD_RESULT_REMOVE_FROM_QUEUE,
	DOWNLOAD_SEARCH_QUEUE_ITEM_NEXT_EVENT
    };


class DownloadThread : public Thread 
{

    public:
        /**
         * Constructs and starts a download thread.
         *
         * @param inFromAddress the virtual address of the host to download
         *   from.  Must be destroyed by caller.
         * @param inFilePath the file path to download.
         *   Must be destroyed by caller.
         * @param inFileHash the SHA1 hash of the file.
         *   Must be destroyed by caller.
         * @param inDownloadSizePointer pointer to where the download
         *   size should be set when it is known.
         * @param inDownloadItem the parent item panel, wrapped
         *   as a void * to avoid a class reference loop.
         */
        DownloadThread( const char *inFromAddress, const char *inFilePath,
                        const char *inFileHash,
						int inStartChunk,
                        SHA_CTX inShaContext,
                        unsigned long *inDownloadSizePointer,
                        void *inDownloadItem );

        // joins, and destroys this search thread
        ~DownloadThread();

        // implements Thread interface
        void run();

        
    private:
        char *mFromAddress;
        char *mFilePath;
        char *mFileHash;
		unsigned int mStartChunk;
        SHA_CTX mShaContext;
        unsigned long *mDownloadSizePointer;
        void *mDownloadItem;

        MutexLock *mStopLock;
        char mStopped;        
};



class CMuteDownloadListItem
{

    public:


        /**
         * Constructs an item.
         *
         * @param inParent the parent window.
         * @param inParentSizer the sizer that this item should add
         *   itself to.  This item will also remove itself once it has
         *   be canceled.
         * @param inFromAddress the virtual address of the host to download
         *   from.  Must be destroyed by caller.
         * @param inFilePath the file path to download.
         *   Must be destroyed by caller.
         * @param inFileHash the SHA1 hash of the file.
         *   Must be destroyed by caller.
         */
        /*CMuteDownloadListItem( wxWindow *inParent,
                      wxBoxSizer *inParentSizer,
                      char *inFromAddress, char *inFilePath,
                      char *inFileHash );
		*/
		
		CMuteDownloadListItem( CDialog *inParent, const char *inLocalFilePath, const char *inFileHash );

		CMuteDownloadListItem( CDialog *inParent, const char *inFromAddress, const char *inLocalFilePath, const char *inRemoteFilePath,
                      const char *inFileHash, bool bIsAResumedFile = false, unsigned int unNumResumes = 0 );

        
        
        ~CMuteDownloadListItem();
        

        void RestartDownload( const char *inFromAddress, const char *inRemoteFilePath );

        /**
         * Processes an incoming chunk of this download.
         *
         * @param inChunk the chunk data, or NULL to indicate that the
         *   current chunk has failed and will be retried.
         *   Must be destroyed by caller.
         * @param inChunkLengthInBytes the length of the chunk data in bytes,
         *   or -1 if the current chunk has failed and will be retried.
         */
        void processChunk( unsigned char *inChunk,
                           int inChunkLengthInBytes, void *inExtraParam );

        

        /**
         * Processes the final result of a download.
         *
         * @param inResult the return value of muteShareGetFile.
         */
        void processDownloadResult( int inResult );
        
        
        
        /**
         * Gets whether this download has been canceled by the user.
         *
         * @return true if canceled, or false otherwise.
         */
        char isCanceled();



        /**
         * Gets whether this item has been cleared from view and is ready for
         * explicit destruction.
         *
         * Uncleared windows will be destroyed automatically at program
         * termination by the standard wxWindows destruction process.
         *
         * @return true if this item has been cleared.
         */
        char isCleared();


        
        /**
         * Gets whether this download is still active (incoming).
         *
         * @return true if this download is still active.
         */
        char isActive();
        
		void OnCancelClear();
		void OnDownloadProgress();
		void OnChunkRetry();
		void OnChunkReceived();
		void OnDownloadResult();
		inline void LockStatus()
		{
			mDownloadStatusLock->lock();
		}

		inline void UnlockStatus()
		{
			mDownloadStatusLock->unlock();
		}
		
        inline FILE * GetFilePtr()
		{
			return mDownloadFILE; // old fashoned file pointer
		}
        
		inline File * GetFileObjPtr() const
		{
			return mDownloadFile; // jason's class
		}

		inline bool IsFileBeingResumed()
		{
			return m_bIsBeingResumed;
		}

		inline SetBytesDLSoFar( int nBytesDLSoFar )
		{
			mDownloadedSoFarInBytes = nBytesDLSoFar;
			m_unInitialByteCount = (unsigned int) nBytesDLSoFar;
		}

    private:
                    
		CDialog *mParentWindow;

        MutexLock *mDownloadStatusLock;

        char mDownloadActive;
        unsigned long mDownloadSizeInBytes;
        int mDownloadedSoFarInBytes;
		int m_unInitialByteCount;

        // remember the start time so we can compute the ETA
        // using the average transfer rate instead of just the most
        // recent rate.
        unsigned long mDownloadStartTimeSeconds;
        unsigned long mDownloadStartTimeMilliseconds;

        
        // measure the rate with blocks of time that
        // are at least 1 second long
        unsigned long mCurrentBlockStartTimeSeconds;
        unsigned long mCurrentBlockStartTimeMilliseconds;
        
        long mCurrentBlockStartSize;
        
        float mCurrentRate;
        
        int mDownloadResult;		
        
        DownloadThread *mDownloadThread;

        char mCanceled;
        char mCleared;
        
        FILE *mDownloadFILE; // old fashoned file pointer
        File *mDownloadFile; // jason's class

		bool		m_bIsBeingResumed;
		CExternStr	m_ExtStr;

public:
	unsigned int		m_unItemState; // one of E_DL_ITEM_STATE
	CString				m_strLocalFileName;	
	CString				m_strFileSize;
	CString				m_strFileExt;
	CString				m_strFileType;
	CString				m_strHostVirtualAddress;
	CString				m_strHash;
	CString				m_strStatusText;	
	CString				m_strResumesText;
	unsigned int		m_unNumResumes;
	CTime				m_oStartTime;
	CString				m_strStartTime;
	CString				m_strRouteQuality;
	CTime				m_oLastUpdateTime;
	CString				m_strLastStatusUpdateTime;
	unsigned int		m_currentQuality;
	unsigned int		m_unRetryUpdateCtr;	
};


// CMuteDownloadListCtrl
class CMuteDownloadListCtrl : public CMuleListCtrl
{
	DECLARE_DYNAMIC(CMuteDownloadListCtrl)

public:
	void SetStrings();
	CMuteDownloadListCtrl();
	virtual ~CMuteDownloadListCtrl();
	void	Init();
	void	AddDownloadResult( CMuteDownloadListItem * pListItem );
	void	UpdateInfo(CMuteDownloadListItem * pListItem );
	void	PumpMessages();  // use for avoiding Dead Lock!! 
	vector<bool> m_vecOldSortAscending;

	void	PrepareForListDestruction();
	static bool m_bAppIsClosing;

	inline	bool LockList( int nTimeoutMS )
	{
		if( m_semListOperations.Lock( nTimeoutMS ) )
		{
			m_bStopUpdating = true;
			return( true );
		}
		else
		{
			return( false );
		}
	}

	inline	void UnlockList()
	{
		m_bStopUpdating = false;
		m_semListOperations.Unlock();
	}

protected:
	bool			m_bStopUpdating;
	int				m_iColumns;
	bool			m_bSetImageList;
	HIMAGELIST		m_hSystemImageList;
	CMapStringToPtr m_aExtToSysImgIdx;
	CSize			m_sizSmallSystemIcon;
	CSemaphore		m_semListOperations;
	CExternStr		m_ExtStr;

	void	CreateMenu();
	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
    // event handlers for download progress
        
        /**
         * Called for any kind of download progress to update
         * progress bar and ETA display.
         */	
	afx_msg void MenuCancelClick( WPARAM wParam, LPARAM lParam );
	afx_msg void MenuClearComplete( WPARAM wParam, LPARAM lParam );
	afx_msg void MenuClearStalled(  WPARAM wParam, LPARAM lParam );
	afx_msg void MenuSearchQueueItemNext( WPARAM wParam, LPARAM lParam );
	afx_msg void OnDownloadProgress( WPARAM wParam, LPARAM lParam );

        
        /**
         * Called when a chunk is retried.
         */ 
	afx_msg void OnChunkRetry(  WPARAM wParam, LPARAM lParam );

        
        /**
         * Called when a chunk is received.
         */ 
	afx_msg void OnChunkReceived(  WPARAM wParam, LPARAM lParam );

        
        /**
         * Called when a download finishes (completes, is canceled, or fails).
         */
	afx_msg void OnDownloadResult( WPARAM wParam, LPARAM lParam );
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnCustomdraw ( NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg	void OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnDeleteallitems(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void UpdateToolTips(void);
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()

private:
	CTitleMenu		m_RightClkMenu;
	CToolTipCtrl	m_ToolTip;
	CString			m_strToolTip;

	int		GetFileTypeSystemImageIdx( LPCTSTR pszFilePath, int iLength, CString &strExtension, CString &strFileType );	
};
