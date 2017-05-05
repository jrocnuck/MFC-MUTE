#include "stdafx.h"
#include <vector>

using namespace std;


#ifndef _MUTE_DOWNLOAD_QUEUE_ITEM_
#define _MUTE_DOWNLOAD_QUEUE_ITEM_

typedef struct 
{
	CString strRemoteFileName;
	CString	strVIP;

} _QueueItemRemoteFileInfo;

/////////////////////////////////////////////////////////////////////////////
// CDownloadQueueItem
/////////////////////////////////////////////////////////////////////////////
class CDownloadQueueItem
{
public:
	CDownloadQueueItem();
	~CDownloadQueueItem();

	// OVERLOADED OPERATORS
	CDownloadQueueItem &operator=(const CDownloadQueueItem &copy);

	bool IsActive() const;
	bool IsQueued() const;
	void SetLocalFileName( CString& strFileName );
	CString GetLocalFileName() const;	
	CString GetRemoteFileName( unsigned int unIndex ) const;
	void SetHash( const char * szHash );
	CString GetHash() const;
	void SetDownloadListItemPtr( void *pListItem );
	inline void * GetDownloadListItemPtr() { return m_pDownloadListItemPtr; }

	CString GetVIP( unsigned int unIndex );
	// adds a new source VIP to download from
	// potential useful in future for swarming.. 
	inline unsigned int GetRemoteInfoCount() { return (unsigned int) m_vecRemoteFileInfo.size(); }
	void AddRemoteInfo( const char * szRemoteFileName, const char * szVIP );
	void AddRemoteInfo( _QueueItemRemoteFileInfo remoteInfo );
	bool GetRemoteInfo( unsigned int unIndex, _QueueItemRemoteFileInfo &remoteInfo );

	// call this when a download fails/times out/etc. etc.. 
	void RemoveRemoteInfoByVIP( CString strVIP );
	void RemoveRemoteInfoByFileName( CString strRemoteFilename );
	void RemoveAllRemoteInfo();
	unsigned int GetRandomRemoteInfoIndex();


	// if we are not actively downloading, set this to true
	void SetQueued( bool bIsQueued );

	// if we are actively downloading, set this to true
	void SetActive( bool bIsActive );


protected:

private:
	CString									m_strLocalFileName;
	vector<_QueueItemRemoteFileInfo>		m_vecRemoteFileInfo;	
	CString									m_strHash;
	bool									m_bActive; // true if downloading
	bool									m_bQueued; // true if not downloading, but queued
	bool									m_bSourceIsValid; // set to true if recent search results show 1 or more sources...	
	void									*m_pDownloadListItemPtr;
};

#endif // _MUTE_DOWNLOAD_QUEUE_ITEM_