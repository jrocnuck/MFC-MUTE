#include "stdafx.h"
#include "MFC_MUTE_GUI.h"
#include "StrParse.h"
#include "MuteDownloadListCtrl.h"
#include "MuteDownloadsDlg.h"

#include "MuteDownloadQueueItem.h"
#include "MuteDownloadQueue.h"
#include <fstream>
/*
** 01-18-2005-- JROC -->
	Changed the delays for automated searching.
	-- after a search is put out, we only wait 30 seconds for results
	-- added a BinarySemaphore to do waiting in threads rather than using
	   silly polling code.
    -- changed delay between automated searches to 60 seconds
    
** 01-22-2005-- JROC --> 
	Added StreamQueueToFile() calls to both of the remove and add
	queue item functions, so the queue file maintains better persistence.
	--Really only applies if user forces app to shutdown from task
	manager, or has a crash (but we all know that never happens)
    
** 02-14-2005-- JROC -->
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

unsigned int g_unNumSecsForSearchTimeout = 20;
//*******************************************************************
//  FUNCTION:   -	QueueSearchResultHandler()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
char QueueSearchResultHandler( char *inFromAddress, char *inFilePath,
                          unsigned long inFileSize,
                          char *inFileHash,
                          void *inExtraParam )
{    
    CDownloadQueue *pDLQueue = (CDownloadQueue *) inExtraParam;	
    
	char keepGoing = true;
	
	if( (time(NULL) - pDLQueue->m_unStartTime) > g_unNumSecsForSearchTimeout ) 
	{
        keepGoing = false;
		pDLQueue->ProcessSearchResults();
		return keepGoing;
    }
    
    if( inFromAddress != NULL ) 
	{
        // got results, no timeout
    	CDownloadQueueItem  item;
		item.SetHash( inFileHash );
		item.AddRemoteInfo( inFilePath, inFromAddress );   
		pDLQueue->m_vecSearchResults.push_back( item );        
		if( g_unNumSecsForSearchTimeout >= 14 )
		{
			g_unNumSecsForSearchTimeout -= 14; // cut search time down after we find at least one result!
		}		
    }
    	
    if( pDLQueue->IsQueueDisabled() ) 
	{
        keepGoing = false;		
		pDLQueue->ProcessSearchResults();
    }  

    // only post event if we are not canceled
    return keepGoing;
}
//*******************************************************************
//  FUNCTION:   -	CDownloadQueueThread()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
CDownloadQueueThread::CDownloadQueueThread( void *pDLQueue )
{
	m_pDLQueue = pDLQueue;
	m_bStopped = false;
	m_nThreadLoopDelay = 1000;	// 1,000 milliseconds (1 second)
	start();
}

//*******************************************************************
//  FUNCTION:   -	~CDownloadQueueThread()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
CDownloadQueueThread::~CDownloadQueueThread()
{		

    m_bStopped = true;
	m_nThreadLoopDelay = 5;
	m_semThreadDelay.signal();
    join();
}


//*******************************************************************
//  FUNCTION:   -	CDownloadQueueThread::run()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CDownloadQueueThread::run() 
{
    CDownloadQueue		*pDLQueue = (CDownloadQueue *)m_pDLQueue;
    CDownloadQueueItem	objDLQueueItem;
	CMuteDownloadsDlg	*pDlg = (CMuteDownloadsDlg *) pDLQueue->m_pDownloadDlg;	

    while( !m_bStopped ) 
	{		
		if( !pDLQueue->SearchIsActive() && !pDLQueue->AreResultsReady() )
		{			
			if( !m_bStopped )
			{
				if( pDLQueue->GetNextQueueItem( objDLQueueItem ) )		
				{					
					CString strSearch = "hash_";
					CString strHash = objDLQueueItem.GetHash();
					CString strLocalFileName = objDLQueueItem.GetLocalFileName();					
					
					// hard code length of hash value.. 
					if( 40 == strHash.GetLength() )
					{
						// search based on hash if the hash is available
						strSearch += strHash;
						objDLQueueItem.RemoveAllRemoteInfo();
						g_unNumSecsForSearchTimeout = 20; // reset search timeout
						pDLQueue->StartSearch( strSearch );
						if( pDLQueue->SearchIsActive() )
						{
							pDlg->PostMessage( DOWNLOAD_QUEUED_ITEM_IS_SEARCHING, 0, (LPARAM) objDLQueueItem.GetDownloadListItemPtr() );
						}
					}
					else if( !strLocalFileName.IsEmpty() )
					{
						// search based on local filename if the hash is not available
						strSearch = strLocalFileName;
						objDLQueueItem.RemoveAllRemoteInfo();
						g_unNumSecsForSearchTimeout = 20; // reset search timeout
						pDLQueue->StartSearch( strSearch );
						if( pDLQueue->SearchIsActive() )
						{
							pDlg->PostMessage( DOWNLOAD_QUEUED_ITEM_IS_SEARCHING, 0, (LPARAM) objDLQueueItem.GetDownloadListItemPtr() );
						}
					}					
				}
			}
			else
			{
				break;
			}
		}
		else if( pDLQueue->AreResultsReady() )
		{			
			_QueueItemRemoteFileInfo remoteInfo;
			pDLQueue->StopSearch();
			
			// snag VIPs for search results
			// if results >= 1, add the download back to the downloads window..			
			for( unsigned int i = 0; i < (unsigned int) pDLQueue->m_vecSearchResults.size(); i++ )
			{					
				for( unsigned int j = 0; j < pDLQueue->m_vecSearchResults[i].GetRemoteInfoCount(); j++ )
				{
					if( pDLQueue->m_vecSearchResults[i].GetRemoteInfo( j, remoteInfo ) )
					{
						objDLQueueItem.AddRemoteInfo( remoteInfo );
					}
				}
			}

			if( objDLQueueItem.GetRemoteInfoCount() > 0 )
			{				
				CString	strHash;				
				// for now get a random index of the returned sources..
				// in future we may spawn off separate "part" files for each of the sources
				// for multi source swarming.. 
				unsigned int	unIndex = objDLQueueItem.GetRandomRemoteInfoIndex();
				
				// in case we started out the queue without the hash... 
				// we need to update the hash...				
				strHash = objDLQueueItem.GetHash();
				if( strHash.GetLength() < 40 )
				{
					objDLQueueItem.SetHash( pDLQueue->m_vecSearchResults[unIndex].GetHash() );
					// also update the DLQueues DLQueueitem in it's vector of items..
					pDLQueue->SetHashFromLocalFileName( (LPCSTR) objDLQueueItem.GetLocalFileName(), (LPCSTR) objDLQueueItem.GetHash() );
				}

				if( pDlg->ResumeFromQueue( objDLQueueItem.GetLocalFileName(), objDLQueueItem.GetRemoteFileName(unIndex), objDLQueueItem.GetHash(), objDLQueueItem.GetVIP(unIndex) ) )
				{
					pDLQueue->SetItemToQueued( false, (LPCSTR) objDLQueueItem.GetHash() );
				}
				else
				{
					// didn't find any results.. so put item text back to queued...					
					pDlg->PostMessage( DOWNLOAD_QUEUED_ITEM_IS_QUEUED, 0, (LPARAM) objDLQueueItem.GetDownloadListItemPtr() );					
				}
			}
			else
			{
				// didn't find any results.. so put item text back to queued...				
				pDlg->PostMessage(DOWNLOAD_QUEUED_ITEM_IS_QUEUED, 0, (LPARAM) objDLQueueItem.GetDownloadListItemPtr() );				
			}			
			// clear results ready!
			pDLQueue->ClearResultsReadyFlag();			
		}
	
		if( !m_bStopped )
		{
			m_semThreadDelay.wait( m_nThreadLoopDelay );
		}
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//*******************************************************************
//  FUNCTION:   -	CDownloadQueue()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
CDownloadQueue::CDownloadQueue( void *pDownloadDialog )
{		
	EnableQueue(); // call this first things first... 
	m_pDownloadDlg = pDownloadDialog; //(CMuteDownloadsDlg *)

	m_vecQueueItems.resize(0);
	// first things first.. load from old file if possible.. 
	ConstructQueueFromFile();
	m_bResultsAreReady = false;
	m_bSearchIsActive = false;	
	m_pSearchThread = NULL;
	m_pQueueThread = new CDownloadQueueThread( (void *) this );	
}


//*******************************************************************
//  FUNCTION:   -	~CDownloadQueue()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
CDownloadQueue::~CDownloadQueue()
{
	DisableQueue();
	if( NULL != m_pSearchThread )
	{
		delete m_pSearchThread;
		m_pSearchThread = NULL;
	}
	delete m_pQueueThread; // stop the thread before saving queue data to file.. 	
	StreamQueueToFile();	
}

//*******************************************************************
//  FUNCTION:   -	DisableQueue()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CDownloadQueue::DisableQueue()
{
	m_bQueueIsDisabled = true;
	StopSearch();
}

//*******************************************************************
//  FUNCTION:   -	EnableQueue()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CDownloadQueue::EnableQueue()
{
	if( m_semQueueOperations.Lock( 60000 ) )
	{
		m_bQueueIsDisabled = false;
		m_semQueueOperations.Unlock();
	}
}

//*******************************************************************
//  FUNCTION:   -	StartSearch()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CDownloadQueue::StartSearch( CString& strSearchText )
{
	if( m_semQueueOperations.Lock( 120000 ) )
	{
		if( m_bQueueIsDisabled )
		{
			m_semQueueOperations.Unlock();
			return;
		}

		if( NULL != m_pSearchThread )
		{
			delete m_pSearchThread;
			m_pSearchThread = NULL;
		}

		m_vecSearchResults.resize(0);
		m_bResultsAreReady = false;
		m_bSearchIsActive = true;
		m_unStartTime = time(NULL);		
		m_pSearchThread = new QueueSearchThread( (LPCSTR) strSearchText, (void *)this );
		m_semQueueOperations.Unlock();
	}
}

//*******************************************************************
//  FUNCTION:   -	StopSearch()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CDownloadQueue::StopSearch()
{
	if( NULL != m_pSearchThread )
	{
		delete m_pSearchThread;
		m_pSearchThread = NULL;
	}

	if( m_semQueueOperations.Lock( 60000 ) )
	{		
		m_bSearchIsActive = false;
		m_semQueueOperations.Unlock();
	}
}


//*******************************************************************
//  FUNCTION:   -	SearchIsActive()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
bool CDownloadQueue::SearchIsActive()
{
	bool bIsActive = false;
	if( m_semQueueOperations.Lock( 60000 ) )
	{
		bIsActive = m_bSearchIsActive;
		m_semQueueOperations.Unlock();
	}
	return bIsActive;
}

//*******************************************************************
//  FUNCTION:   -	ProcessSearchResults()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CDownloadQueue::ProcessSearchResults()
{
	if( m_semQueueOperations.Lock( 60000 ) )
	{
		m_bResultsAreReady = true;
		m_semQueueOperations.Unlock();
	}
}
	

//*******************************************************************
//  FUNCTION:   -	AreResultsReady()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
bool CDownloadQueue::AreResultsReady()
{
	bool bRetVal = false;
	if( m_semQueueOperations.Lock( 60000 ) )
	{
		if( !m_bQueueIsDisabled )
		{
			bRetVal = m_bResultsAreReady;
		}
		m_semQueueOperations.Unlock();
	}

	return bRetVal;
}

//*******************************************************************
//  FUNCTION:   -	ConstructQueueFromFile()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CDownloadQueue::ConstructQueueFromFile()
{
	ifstream	inFile;
	CString		strQueueFile;

	strQueueFile = ( (CMUTEMFC2App *) AfxGetApp() )->m_strStartingPath;
	if( strQueueFile.IsEmpty() )
	{
		return;
	}

	m_bFirstStart = true;

	strQueueFile += "\\settings\\Downloadqueue.ini";

	inFile.open( (LPCSTR) strQueueFile );

	if( inFile.is_open() )
	{
		string			strInputLine;
		CString			strTemp;
		CString			strLocalFile, strHash;
		CStringParser	*oParser = new CStringParser();

		// file format:
		// FIELDS ARE SEPARATED BY AN ASTERIK '*' .... THERE ARE 2 FIELDS...
		// LOCAL FILE NAME * FILE HASH
		while( !inFile.eof() )
		{
			getline( inFile, strInputLine, '\n' );						

			oParser->vGo( strInputLine.c_str(), '*', FALSE );
			if( oParser->nGetNumFields() > 0 )
			{
				strLocalFile = oParser->m_strOutput[0].c_str();
				strLocalFile.TrimLeft();
				strLocalFile.TrimRight();
			}
			
			if( oParser->nGetNumFields() > 1 )
			{
				strHash = oParser->m_strOutput[1].c_str();
				strHash.TrimLeft();
				strHash.TrimRight();
			}

			// at the least, we need the local file name
			if( !( strLocalFile.IsEmpty() || ("" == strLocalFile) ) )
			{		
				if( strHash.GetLength() < 40 )
				{
					strHash.Empty();
				}

				CMuteDownloadsDlg	*pDlg = (CMuteDownloadsDlg *) m_pDownloadDlg;
				void				*pDLListItem = NULL;
								
				// get entry onto download list, even though it will be queued.. 
				pDLListItem = pDlg->addDownloadFromQueue( (LPCSTR) strLocalFile, (LPCSTR) strHash );
													
				if( NULL != pDLListItem )
				{
					AddQueueItem( true, NULL, (LPCSTR) strLocalFile, NULL, (LPCSTR) strHash );				
					SetItemDLoadListItemPtrFromLocalFilename( pDLListItem, (LPCSTR) strLocalFile );
				}				
			}

			strLocalFile.Empty();
			strHash.Empty();
		}
		
		delete oParser;

		inFile.close();
	}

	// set to this value, so the first time through on start up
	// we will start searching the queue for the first file in the queue.. 
	m_unCurrentItemIndex = m_vecQueueItems.size();
	m_bFirstStart = false;
}


//*******************************************************************
//  FUNCTION:   -	StreamQueueToFile()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CDownloadQueue::StreamQueueToFile()
{
	ofstream	outFile;
	CString		strQueueFile;



	strQueueFile = ( (CMUTEMFC2App *) AfxGetApp() )->m_strStartingPath;
	if( strQueueFile.IsEmpty() )
	{
		return;
	}

	// 01-22-2005 -- need to lock the semaphore here
	// because we don't want other operations interfering!
	if( !m_semQueueOperations.Lock( 120000 ) )
	{
		return;
	}

	strQueueFile += "\\settings\\Downloadqueue.ini";

	outFile.open( (LPCSTR) strQueueFile );

	if( outFile.is_open() )
	{
		CString	strLocalFileName;
		CString	strHash;
		int		i;

		// file format:
		// FIELDS ARE SEPARATED BY AN ASTERIK '*' .... THERE ARE 2 FIELDS...
		// LOCAL FILE NAME * FILE HASH
		for( i = 0; i < (int) m_vecQueueItems.size(); i++ )
		{
			if( i > 0 )
			{
				// prevents putting newline character at end of file.
				outFile << endl;
			}
			strLocalFileName = m_vecQueueItems[i].GetLocalFileName();
			strHash = m_vecQueueItems[i].GetHash();
			if( "" == strLocalFileName || strLocalFileName.IsEmpty() )
			{
				outFile << " * ";
			}
			else
			{
				outFile << (LPCSTR) strLocalFileName << " * ";
			}
			
			if( !("" == strHash || strHash.IsEmpty()) )
			{
				outFile << (LPCSTR) strHash; 
			}
		}		

		outFile.close();
	}

	// 01-22-2005 
	// unlock the semaphore
	m_semQueueOperations.Unlock();
}


//*******************************************************************
//  FUNCTION:   -	AddQueueItem()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	return true if item gets added, but is queued...
//*******************************************************************
bool CDownloadQueue::AddQueueItem( bool bQueued, const char *inFromAddress, const char *inLocalFilename, const char *inRemoteFilePath, const char *inFileHash )
{
	bool bRetVal = false;
	
	CDownloadQueueItem	queueItem;
	CString				strLocalFile;
	CString				strHash;

	if( !m_semQueueOperations.Lock( 120000 ) )
	{
		return bRetVal;
	}
	
	if( m_bQueueIsDisabled )
	{
		m_semQueueOperations.Unlock();
		return bRetVal;
	}

	if( NULL != inLocalFilename )
	{
		strLocalFile = inLocalFilename;
	}
	else
	{
		strLocalFile.Empty();
	}
	
	if( NULL != inFileHash )
	{
		strHash = inFileHash;
	}
	else
	{
		strHash.Empty();
	}

	// without a local file name and hash, we can't do anything... 
	if( !( strLocalFile.IsEmpty() && strHash.IsEmpty() ) )
	{
		queueItem.SetLocalFileName( strLocalFile );
		queueItem.SetHash( strHash );
		queueItem.AddRemoteInfo( inLocalFilename, inFromAddress );
		queueItem.SetQueued( bQueued );
		m_vecQueueItems.push_back( queueItem );
	}

	// unlock the semaphore
	m_semQueueOperations.Unlock();

	// 01-22-2005 -- want to keep our queue file consistent with whats in the
	// program memory!
	if( !m_bFirstStart ) // 02/14/2005 -- can't write to the file will constructing from the file
	{
		StreamQueueToFile();
	}
	return bRetVal;
}


//*******************************************************************
//  FUNCTION:   -	RemoveQueueItem()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CDownloadQueue::RemoveQueueItem( const char *inLocalFilename )
{	
	vector<CDownloadQueueItem>::iterator	iterQueueItem;		
	CString				strLocalFileName;

	if( !m_semQueueOperations.Lock( 120000 ) )
	{
		return;
	}

	if( m_bQueueIsDisabled )
	{
		m_semQueueOperations.Unlock();
		return;
	}

	strLocalFileName = inLocalFilename;

	for ( iterQueueItem = m_vecQueueItems.begin(); iterQueueItem < m_vecQueueItems.end(); ++iterQueueItem )
	{
		if ( strLocalFileName == (*iterQueueItem).GetLocalFileName() )
		{
			// Remove the VIP from the list.
			m_vecQueueItems.erase( iterQueueItem );
			break;
		}
	}

	// unlock the semaphore
	m_semQueueOperations.Unlock();

	// todo ... 
	// 01-22-2005 ... Stream QUEUE file to disk so if we crash or reboot,
	// the queue will have been updated!
	StreamQueueToFile();
	return;
}


//*******************************************************************
//  FUNCTION:   -	IsItemActive()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	01-23-2005 JROC -- 
//*******************************************************************
bool CDownloadQueue::IsItemActive( const char *inFileHash )
{
	vector<CDownloadQueueItem>::iterator	iterQueueItem;		
	CString				strHash;
	bool				bItemIsActive = false;

	if( !m_semQueueOperations.Lock( 120000 ) )
	{
		return false;
	}

	if( m_bQueueIsDisabled )
	{
		m_semQueueOperations.Unlock();
		return false;
	}

	strHash = inFileHash;
	for ( iterQueueItem = m_vecQueueItems.begin(); iterQueueItem < m_vecQueueItems.end(); ++iterQueueItem )
	{
		if ( strHash == (*iterQueueItem).GetHash() )
		{
			// Remove the VIP from the list.
			m_vecQueueItems.erase( iterQueueItem );
			bItemIsActive = (*iterQueueItem).IsActive();
			break;
		}
	}

	// unlock the semaphore
	m_semQueueOperations.Unlock();

	return bItemIsActive;
}


//*******************************************************************
//  FUNCTION:   -	GetNextQueueItem()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	return true if item is valid and is queued
//*******************************************************************
bool CDownloadQueue::GetNextQueueItem( CDownloadQueueItem& objDLQueueItem )
{
	bool bRetVal = false;
	if( !m_semQueueOperations.Lock( 15000 ) )
	{
		return bRetVal;
	}

	if( m_bQueueIsDisabled )
	{
		m_semQueueOperations.Unlock();
		return bRetVal;
	}

	if( m_vecQueueItems.size() > 0 )
	{
		m_unCurrentItemIndex++;
		if( m_unCurrentItemIndex >= m_vecQueueItems.size() )
		{
			m_unCurrentItemIndex = 0;
		}

		objDLQueueItem = m_vecQueueItems[ m_unCurrentItemIndex ];
		if( objDLQueueItem.IsQueued() )
		{
			bRetVal = true;
		}
	}

	m_semQueueOperations.Unlock();
	return bRetVal;
}

//*******************************************************************
//  FUNCTION:   -	SearchQueueItemNext()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	return true if item is valid and is queued
//*******************************************************************
void CDownloadQueue::SearchQueueItemNext( const char *inLocalFilename ) // 01-23-2005 JROC 
{	
	int i;

	if( !m_semQueueOperations.Lock( 15000 ) )
	{
		return;
	}

	if( m_bQueueIsDisabled )
	{
		m_semQueueOperations.Unlock();
		return;
	}

	for( i = 0; i < (int) m_vecQueueItems.size(); i++ )
	{
		if( m_vecQueueItems[i].GetLocalFileName() == inLocalFilename )
		{			
			m_unCurrentItemIndex = i;
			// after finding it, we have to back off by 1 because
			// the GetNextQueueItem() will increment to the index
			// of the actual file being passed to this function
			if( 0 == m_unCurrentItemIndex )
			{
				m_unCurrentItemIndex = m_vecQueueItems.size() - 1;
			}
			else
			{
				m_unCurrentItemIndex--;
			}
			break;
		}
	}
		
	m_semQueueOperations.Unlock();
	return;
}

//*******************************************************************
//  FUNCTION:   -	SetItemToQueued()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
bool CDownloadQueue::SetItemToQueued( bool bIsQueued, const char *inFileHash )
{
	bool									bFound = false;
	vector<CDownloadQueueItem>::iterator	iterQueueItem;		
	CString									strHash;

	if( !m_semQueueOperations.Lock( 60000 ) )
	{
		return bFound;
	}
	
	if( m_bQueueIsDisabled )
	{
		m_semQueueOperations.Unlock();
		return bFound;
	}

	strHash = inFileHash;

	for ( iterQueueItem = m_vecQueueItems.begin(); iterQueueItem < m_vecQueueItems.end(); ++iterQueueItem )
	{
		if ( strHash == (*iterQueueItem).GetHash() )
		{
			// Remove the VIP from the list.
			(*iterQueueItem).SetQueued( bIsQueued );			
			bFound = true;
			break;
		}
	}

	// unlock the semaphore
	m_semQueueOperations.Unlock();
	return bFound;
}

//*******************************************************************
//  FUNCTION:   -	SetItemDLoadListItemPtrFromHash()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CDownloadQueue::SetItemDLoadListItemPtrFromHash( void *pDownloadItem, const char *inFileHash )
{	
	vector<CDownloadQueueItem>::iterator	iterQueueItem;		
	CString									strHash;

	if( !m_semQueueOperations.Lock( 60000 ) )
	{
		return;
	}
	
	if( m_bQueueIsDisabled )
	{
		m_semQueueOperations.Unlock();
		return;
	}

	strHash = inFileHash;

	for ( iterQueueItem = m_vecQueueItems.begin(); iterQueueItem < m_vecQueueItems.end(); ++iterQueueItem )
	{
		if ( strHash == (*iterQueueItem).GetHash() )
		{
			// Remove the VIP from the list.
			(*iterQueueItem).SetDownloadListItemPtr( pDownloadItem );					
			break;
		}
	}

	// unlock the semaphore
	m_semQueueOperations.Unlock();	
}

//*******************************************************************
//  FUNCTION:   -	SetItemDLoadListItemPtrFromLocalFilename()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CDownloadQueue::SetItemDLoadListItemPtrFromLocalFilename( void *pDownloadItem, const char *inLocalFilename )
{
	vector<CDownloadQueueItem>::iterator	iterQueueItem;		
	CString									strLocalFilename;

	if( !m_semQueueOperations.Lock( 60000 ) )
	{
		return;
	}
	
	if( m_bQueueIsDisabled )
	{
		m_semQueueOperations.Unlock();
		return;
	}

	strLocalFilename = inLocalFilename;

	for ( iterQueueItem = m_vecQueueItems.begin(); iterQueueItem < m_vecQueueItems.end(); ++iterQueueItem )
	{
		if ( strLocalFilename == (*iterQueueItem).GetLocalFileName() )
		{
			// Remove the VIP from the list.
			(*iterQueueItem).SetDownloadListItemPtr( pDownloadItem );					
			break;
		}
	}

	// unlock the semaphore
	m_semQueueOperations.Unlock();
}

//*******************************************************************
//  FUNCTION:   -	SetHashFromLocalFileName()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	find item with incoming filename and set hash .. 
//*******************************************************************
void CDownloadQueue::SetHashFromLocalFileName( const char *inLocalFilename, const char *inHash )
{
	vector<CDownloadQueueItem>::iterator	iterQueueItem;		
	CString									strLocalFilename;

	if( !m_semQueueOperations.Lock( 60000 ) )
	{
		return;
	}
	
	if( m_bQueueIsDisabled )
	{
		m_semQueueOperations.Unlock();
		return;
	}

	strLocalFilename = inLocalFilename;

	for ( iterQueueItem = m_vecQueueItems.begin(); iterQueueItem < m_vecQueueItems.end(); ++iterQueueItem )
	{
		if ( strLocalFilename == (*iterQueueItem).GetLocalFileName() )
		{
			// Remove the VIP from the list.
			(*iterQueueItem).SetHash( inHash );					
			break;
		}
	}

	// unlock the semaphore
	m_semQueueOperations.Unlock();
}
