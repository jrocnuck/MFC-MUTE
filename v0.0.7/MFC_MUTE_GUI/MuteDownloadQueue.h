/*  02-14-2005 -- JROC -->
    -1--> Fixed a bug when loading in the queued downloads from Downloadqueue.ini.
          In version 0.0.3 not all of the queued downloads in Downloadqueue.ini
          would properly be added on restart.
    -2--> Fixed a bug where a queued download was automatically searched for and found, but
          in the process of attempting to resume the download, there were already 4 downloads
          active from the remote source.  Since only 4 active downloads are allowed from 1 VIP
          the queued item must be placed back into the queue.  The BUG was that the item was
          placed back into the queue, but the status text of the item was not properly updated.
          This gave the (invalid) appearance of many queued items searching at once. 
    -3--> Increased semaphore timeout values in Download Queue code. 
*/    

#include "stdafx.h"
#include <vector>
#include "MUTE/otherApps/fileSharing/fileShare.h"
#include "minorGems/system/Thread.h"
#include "minorGems/system/BinarySemaphore.h"
#include "MuteDownloadQueueItem.h"

using namespace std;

#ifndef _MUTE_DOWNLOAD_QUEUE_
#define _MUTE_DOWNLOAD_QUEUE_

// callback for incoming search results
char QueueSearchResultHandler( char *inFromAddress, char *inFilePath,
                          unsigned long inFileSize,
                          char *inFileHash,
                          void *inExtraParam );

/**
 * Thread that runs the MUTE search.
 */
class QueueSearchThread : public Thread 
{

    public:
        /**
         * Constructs and starts a search thread
         * @param inSearchString must be destroyed by caller.
         * @param inExtraHandlerParam parameter to pass to results handler.
         */
        QueueSearchThread( const char *inSearchString, void *inExtraHandlerParam )
            : mSearchString(NULL),mExtraHandlerParam( inExtraHandlerParam ) 
		{
			
			if( strlen( inSearchString ) > 0 )
			{
				mSearchString = stringDuplicate( inSearchString );
			}
            start();
        }

        /**
         * Joins and destroys this search thread.
         */
        ~QueueSearchThread() 
		{
            join();

            if( NULL != mSearchString )
			{
				delete [] mSearchString;
			}
        }

        // implements Thread interface
        void run() {
            muteShareSearch( mSearchString, QueueSearchResultHandler,
                             mExtraHandlerParam,
                             1000 );
            }
        
    private:
        char *mSearchString;
        void *mExtraHandlerParam;
        
};

/////////////////////////////////////////////////////////////////////////////
// CLASS: CDownloadQueue
class CDownloadQueueThread : public Thread
{
public:
	CDownloadQueueThread( void *pDLQueue );
	~CDownloadQueueThread();

		// implements Thread interface
	void run();

private:
	void			*m_pDLQueue;    
	bool			m_bStopped;
	int				m_nThreadLoopDelay;	
	BinarySemaphore	m_semThreadDelay;
};

/////////////////////////////////////////////////////////////////////////////
// CLASS: CDownloadQueue
class CDownloadQueue
{
public:
	// constructors/destructors
	CDownloadQueue( void *pDownloadDialog );
	~CDownloadQueue();

	// public member variables
	CSemaphore						m_semQueueOperations;
	unsigned int					m_unStartTime;
	QueueSearchThread				*m_pSearchThread;
	vector< CDownloadQueueItem >	m_vecSearchResults;
	void							*m_pDownloadDlg;	

	// public member functions
	bool GetNextQueueItem( CDownloadQueueItem& objDLQueueItem );
	void SearchQueueItemNext( const char *inLocalFilename ); // 01-23-2005 JROC 
	// if adding the item, it has to be queued, return true
	bool AddQueueItem( bool bQueued, const char *inFromAddress, const char *inLocalFilename, const char *inRemoteFilePath, const char *inFileHash );
	void RemoveQueueItem( const char *inLocalFilename ); // 02-18-2005 JROC ..we don't always have hash, but we always have local file name!
	bool IsItemActive( const char *inFileHash ); // 01-23-2005 JROC 	
	bool SetItemToQueued( bool bIsQueued, const char *inFileHash );
	void SetItemDLoadListItemPtrFromHash( void *pDownloadItem, const char *inFileHash );
	void SetItemDLoadListItemPtrFromLocalFilename( void *pDownloadItem, const char *inLocalFilename );
	void SetHashFromLocalFileName( const char *inLocalFilename, const char *inHash );
	bool SearchIsActive();
	void StartSearch( CString& strSearchText );
	void StopSearch();
	void DisableQueue();
	void EnableQueue();
	inline bool IsQueueDisabled()
	{
		return m_bQueueIsDisabled;
	}
	void ProcessSearchResults();
	bool AreResultsReady();
	inline void ClearResultsReadyFlag() 
	{
		m_bResultsAreReady = false;
	}
	

protected:

private:
	// private member variables
	vector< CDownloadQueueItem >	m_vecQueueItems;
	unsigned int					m_unCurrentItemIndex; // index of item that is currently being monitored..
	CDownloadQueueThread			*m_pQueueThread;
	bool							m_bSearchIsActive;
	bool							m_bQueueIsDisabled;
	bool							m_bResultsAreReady;
	bool							m_bFirstStart;
	// private member functions
	void							ConstructQueueFromFile();
	void							StreamQueueToFile();	
};	

#endif //_MUTE_DOWNLOAD_QUEUE_