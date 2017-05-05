#include "stdafx.h"
#include <math.h>
#include "MuteDownloadQueueItem.h"


//*******************************************************************
//  FUNCTION:   -	CDownloadQueueItem()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
CDownloadQueueItem::CDownloadQueueItem() 
{
	SetQueued( true );
	m_pDownloadListItemPtr = NULL;
}


//*******************************************************************
//  FUNCTION:   -	~CDownloadQueueItem()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
CDownloadQueueItem::~CDownloadQueueItem()
{	
}

//*******************************************************************
//  FUNCTION:   -	Equals Overload
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
CDownloadQueueItem &CDownloadQueueItem::operator=(const CDownloadQueueItem &copy)
{
	m_strLocalFileName		=	copy.m_strLocalFileName;
	m_strHash				=	copy.m_strHash;
	m_bActive				=	copy.m_bActive;
	m_bQueued				=	copy.m_bQueued;
	m_pDownloadListItemPtr	=	copy.m_pDownloadListItemPtr;

	m_vecRemoteFileInfo.resize( copy.m_vecRemoteFileInfo.size() );

	for( int i = 0; i < (int) copy.m_vecRemoteFileInfo.size(); i++ )
	{
		m_vecRemoteFileInfo[i].strVIP = copy.m_vecRemoteFileInfo[i].strVIP;
		m_vecRemoteFileInfo[i].strRemoteFileName = copy.m_vecRemoteFileInfo[i].strRemoteFileName;
	}

	return *this;
}

//*******************************************************************
//  FUNCTION:   -	SetDownloadListItemPtr()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CDownloadQueueItem::SetDownloadListItemPtr( void *pListItem )
{
	m_pDownloadListItemPtr = pListItem;
}

//*******************************************************************
//  FUNCTION:   -	IsActive()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
bool CDownloadQueueItem::IsActive() const
{ 
	return(m_bActive); 
}

//*******************************************************************
//  FUNCTION:   -	
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
bool CDownloadQueueItem::IsQueued() const
{ 
	return(m_bQueued); 
}

//*******************************************************************
//  FUNCTION:   -	SetLocalFileName()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CDownloadQueueItem::SetLocalFileName( CString& strFileName )
{
	m_strLocalFileName = strFileName;
}

//*******************************************************************
//  FUNCTION:   -	GetLocalFileName()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
CString CDownloadQueueItem::GetLocalFileName() const
{
	return( m_strLocalFileName );
}


//*******************************************************************
//  FUNCTION:   -	GetRemoteFileName()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
CString CDownloadQueueItem::GetRemoteFileName( unsigned int unIndex ) const
{
	CString strRemoteFileName;
	strRemoteFileName.Empty();
	if( m_vecRemoteFileInfo.size() > unIndex )
	{
		strRemoteFileName = m_vecRemoteFileInfo[ unIndex ].strRemoteFileName;		
	}
	return strRemoteFileName;
}

//*******************************************************************
//  FUNCTION:   -	GetRandomRemoteInfoIndex()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
unsigned int CDownloadQueueItem::GetRandomRemoteInfoIndex()
{
	unsigned int	unIndex;	
	double			dRandom;
	
	// if there is only one entry for remote info.. quickly return 0 as the valid
	// index to the remote file info.. 
	if( 1 == m_vecRemoteFileInfo.size() )
	{
		return 0;
	}
	srand(GetTickCount()); // shake up the random number generator
	dRandom = floor( (double)(m_vecRemoteFileInfo.size() * rand()) / (double) RAND_MAX );
	unIndex = (unsigned int) dRandom;	
    
    if( unIndex >= m_vecRemoteFileInfo.size() )
    {
        //should never get here, but who knows these days... :)
        unIndex = m_vecRemoteFileInfo.size() - 1;
    }
    
	return( unIndex );
}

//*******************************************************************
//  FUNCTION:   -	SetHash()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CDownloadQueueItem::SetHash( const char * szHash )
{
	m_strHash = szHash;
}

//*******************************************************************
//  FUNCTION:   -	GetHash()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
CString CDownloadQueueItem::GetHash() const
{
	return( m_strHash );
}
 
//*******************************************************************
//  FUNCTION:   -	AddRemoteInfo()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	adds a new source VIP and remote filename to download from
//*******************************************************************
void CDownloadQueueItem::AddRemoteInfo( const char * szRemoteFileName, const char * szVIP )
{
	int i;
	_QueueItemRemoteFileInfo remoteInfo;

	remoteInfo.strRemoteFileName	= szRemoteFileName;
	remoteInfo.strVIP				= szVIP;

	for( i = 0; i < (int)m_vecRemoteFileInfo.size(); i++ )
	{
		if( remoteInfo.strVIP == m_vecRemoteFileInfo[i].strVIP )
		{
			if( remoteInfo.strRemoteFileName == m_vecRemoteFileInfo[i].strRemoteFileName )
			{
				return; // already in the queued item's vector of remote info.. 
			}
		}
	}

	// wasn't already in the vector, so add it... 
	m_vecRemoteFileInfo.push_back( remoteInfo );	
}

//*******************************************************************
//  FUNCTION:   -	AddRemoteInfo()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	adds a new source VIP and remote filename to download from
//*******************************************************************
void CDownloadQueueItem::AddRemoteInfo( _QueueItemRemoteFileInfo remoteInfo )
{
	int i;

	for( i = 0; i < (int)m_vecRemoteFileInfo.size(); i++ )
	{
		if( remoteInfo.strVIP == m_vecRemoteFileInfo[i].strVIP )
		{
			if( remoteInfo.strRemoteFileName == m_vecRemoteFileInfo[i].strRemoteFileName )
			{
				return; // already in the queued item's vector of remote info.. 
			}
		}
	}

	// wasn't already in the vector, so add it... 
	m_vecRemoteFileInfo.push_back( remoteInfo );	
}

//*******************************************************************
//  FUNCTION:   -	GetRemoteInfo
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-
//*******************************************************************
bool CDownloadQueueItem::GetRemoteInfo( unsigned int unIndex, _QueueItemRemoteFileInfo &remoteInfo )
{
	if( m_vecRemoteFileInfo.size() > unIndex )
	{
		remoteInfo.strRemoteFileName = m_vecRemoteFileInfo[ unIndex ].strRemoteFileName;
		remoteInfo.strVIP = m_vecRemoteFileInfo[ unIndex ].strVIP;
		return( true );
	}		
	return( false );
}
 
//*******************************************************************
//  FUNCTION:   -	RemoveRemoteInfoByVIP
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-
//*******************************************************************
void CDownloadQueueItem::RemoveRemoteInfoByVIP( CString strVIP )
{
	vector<_QueueItemRemoteFileInfo>::iterator	iterRemoteInfo;	

	for ( iterRemoteInfo = m_vecRemoteFileInfo.begin(); iterRemoteInfo < m_vecRemoteFileInfo.end(); ++iterRemoteInfo )
	{
		if ( strVIP == (*iterRemoteInfo).strVIP )
		{
			// Remove each of this VIP from the list.
			m_vecRemoteFileInfo.erase( iterRemoteInfo );
			iterRemoteInfo--; // account for removal from list... 
		}
	}
}

//*******************************************************************
//  FUNCTION:   -	RemoveRemoteInfoByFileName
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-
//*******************************************************************
void CDownloadQueueItem::RemoveRemoteInfoByFileName( CString strRemoteFileName )
{
	vector<_QueueItemRemoteFileInfo>::iterator	iterRemoteInfo;		

	for ( iterRemoteInfo = m_vecRemoteFileInfo.begin(); iterRemoteInfo < m_vecRemoteFileInfo.end(); ++iterRemoteInfo )
	{
		if ( strRemoteFileName == (*iterRemoteInfo).strRemoteFileName )
		{
			// Remove each of this VIP from the list.
			m_vecRemoteFileInfo.erase( iterRemoteInfo );
			iterRemoteInfo--; // account for removal from list... 
		}
	}
}

//*******************************************************************
//  FUNCTION:   -	RemoveAllRemoteInfo()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	 
//*******************************************************************
void CDownloadQueueItem::RemoveAllRemoteInfo()
{
	m_vecRemoteFileInfo.resize(0);
}


//*******************************************************************
//  FUNCTION:   -	GetVIP
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
CString CDownloadQueueItem::GetVIP( unsigned int unIndex )
{
	CString strVIP;
	strVIP.Empty();
	if( m_vecRemoteFileInfo.size() > unIndex )
	{
		strVIP = m_vecRemoteFileInfo[ unIndex ].strVIP;
	}
	return strVIP;
}

// if we are not actively downloading, set this to true
//*******************************************************************
//  FUNCTION:   -	SetQueued()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CDownloadQueueItem::SetQueued( bool bIsQueued )
{	
	if( bIsQueued )
	{
		m_bQueued = true;
		m_bActive = false;
	}
	else
	{
		m_bQueued = false;
		m_bActive = true;
	}
}

// if we are actively downloading, set this to true
//*******************************************************************
//  FUNCTION:   -	
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CDownloadQueueItem::SetActive( bool bIsActive )
{
	SetQueued( !bIsActive );
}

