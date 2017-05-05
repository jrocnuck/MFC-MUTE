/*
 * Modification History
 *
 * 2001-January-28		Jason Rohrer
 * Created.  
 *
 * 2001-February-4		Jason Rohrer
 * Fixed receive so that it waits for all requested bytes to arrive.
 *
 * 2001-March-4		Jason Rohrer
 * Replaced include of <winbase.h> and <windef.h> with <windows.h> 
 * to fix compile bugs encountered with newer windows compilers.
 *
 * 2001-May-12   Jason Rohrer
 * Fixed a bug in socket receive error checking. 
 *
 * 2001-November-13		Jason Rohrer
 * Changed timeout parameter to signed, since -1 is a possible argument. 
 *
 * 2002-April-15    Jason Rohrer
 * Removed call to WSAGetLastError, since it seems to pick up errors from
 * non-socket calls.  For example, if sys/stat.h stat() is called on a file
 * that does not exist, WSAGetLastError returns 2, which is not even a
 * winsock error code.  We should probably report this bug, huh?
 *
 * 2002-August-2    Jason Rohrer
 * Added functon for getting remote host address, but no implementation.
 *
 * 2002-August-5    Jason Rohrer
 * Added implementation of getRemoteHostAddress().
 *
 * 2002-September-8    Jason Rohrer
 * Fixed a major looping bug with broken sockets.
 *
 * 2002-November-15   Jason Rohrer
 * Fixed a security hole when getting the remote host address.
 *
 * 2003-February-4   Jason Rohrer
 * Added a function for getting the local host address from a socket.
 * Still need to test the win32 version of this.
 *
 * 2003-February-5   Jason Rohrer
 * Fixed a bug in call to gethostname.  Removed unused variable.
 *
 * 2004-January-4   Jason Rohrer
 * Added use of network function locks.
 *
 * 2004-January-11   Jason Rohrer
 * Fixed a bug in handling of timeout return value.
 *
 * 2004-March-23   Jason Rohrer
 * Removed timeout error message.
 */



#include "minorGems/network/Socket.h"
#include "minorGems/network/NetworkFunctionLocks.h"

#include <winsock.h>
#include <windows.h>

#include <stdio.h>
#include <time.h>
#include <string.h>


// declaration of global byte counts
__int64 g_nMFCMuteBytesOut = 0;
__int64 g_nMFCMuteBytesIn = 0;

// prototypes
int timed_read( int inSock, unsigned char *inBuf, 
	int inLen, long inMilliseconds );
	

/**
 * Windows-specific implementation of the Socket class member functions.
 *
 */



// Win32 does not define socklen_t
typedef int socklen_t;



char Socket::sInitialized = false;



int Socket::initSocketFramework() {
	WORD wVersionRequested;
	WSADATA wsaData;
	
	int err; 
	wVersionRequested = MAKEWORD( 1, 0 ); 
	err = WSAStartup( wVersionRequested, &wsaData );
	
	if ( err != 0 ) {
    	// no usable DLL found  
    	printf( "WinSock DLL version 1.0 or higher not found.\n" );  
    	
		return -1;
		}
    
    sInitialized = true;
	return 0;
	}
		



Socket::~Socket() {
	int *socketIDptr = (int *)( mNativeObjectPointer );
	int socketID = socketIDptr[0];

	// 2 specifies shutting down both sends and receives
//	shutdown( socketID, 2 ); // jroc... 
	
	closesocket( socketID );
	
	delete [] socketIDptr;
	}

// jroc
void Socket::PrepareForDelete()
{
	int *socketIDptr = (int *)( mNativeObjectPointer );
	int socketID = socketIDptr[0];
	DWORD dwSetToNonBlocking = TRUE;
	ioctlsocket( socketID, FIONBIO, &dwSetToNonBlocking );
	closesocket( socketID );
}

int Socket::send( unsigned char *inBuffer, int inNumBytes ) {
	
	int *socketIDptr = (int *)( mNativeObjectPointer );
	int socketID = socketIDptr[0];
	
	// jroc -- global sent byte count
	g_nMFCMuteBytesOut += (__int64) inNumBytes;
	return ::send( socketID, (char*)inBuffer, inNumBytes, 0 );
	}
		
		
		
int Socket::receive( unsigned char *inBuffer, int inNumBytes,
	long inTimeout ) {
	
	int *socketIDptr = (int *)( mNativeObjectPointer );
	int socketID = socketIDptr[0];
	
	int numReceived = 0;
	
	char error = false;
	char errorReturnValue = -1;

    
	// for win32, we can't specify MSG_WAITALL
	// so we have too loop until the entire message is received,
	// as long as there is no error.
	while( numReceived < inNumBytes && !error ) {
		   
		// the number of bytes left to receive
		int numRemaining = inNumBytes - numReceived;
		
		// pointer to the spot in the buffer where the
		// remaining bytes should be stored
		unsigned char *remainingBuffer = &( inBuffer[ numReceived ] );
		
		int numReceivedIn;
		
		if( inTimeout == -1 ) {		
			numReceivedIn = 
				recv( socketID, (char*)remainingBuffer, numRemaining, 0 );
			}
		else {		
			numReceivedIn = 
				timed_read( socketID, remainingBuffer,
                            numRemaining, inTimeout );	
			}
			
			
		if( numReceivedIn > 0 ) {
			numReceived += numReceivedIn;
			}
        else {
            error = true;

            if( numReceivedIn == 0 ) {
                // the socket was gracefully closed
                errorReturnValue = -1;
                }
            else if( numReceivedIn == SOCKET_ERROR ) {
                // socket error
                errorReturnValue = -1;
                }
            else if( numReceivedIn == -2 ) {
                // timeout
                errorReturnValue = -2;
                }
            else {
                printf( "Unexpected return value from socket receive: %d.\n",
                        numReceivedIn );
                errorReturnValue = -1;
                }
            
            }
			
		}
		
	// jroc -- global byte received count!
	g_nMFCMuteBytesIn += numReceived;
	return numReceived;
	}



HostAddress *Socket::getRemoteHostAddress() {

    int *socketIDptr = (int *)( mNativeObjectPointer );
	int socketID = socketIDptr[0];

    // adapted from Unix Socket FAQ
    
    socklen_t len;
    struct sockaddr_in sin;
    
    len = sizeof sin;
    int error = getpeername( socketID, (struct sockaddr *) &sin, &len );

    if( error ) {
        return NULL;
        }

    // this is potentially insecure, since a fake DNS name might be returned
    // we should use the IP address only
    //
    // struct hostent *host = gethostbyaddr( (char *) &sin.sin_addr,
    //                                       sizeof sin.sin_addr,
    //                                       AF_INET );


    NetworkFunctionLocks::mInet_ntoaLock.lock();
    // returned string is statically allocated, copy it
    char *ipAddress = stringDuplicate( inet_ntoa( sin.sin_addr ) );
    NetworkFunctionLocks::mInet_ntoaLock.unlock();
    
    return new HostAddress( ipAddress, 0 );    
    }



HostAddress *Socket::getLocalHostAddress() {
    int *socketIDptr = (int *)( mNativeObjectPointer );
	int socketID = socketIDptr[0];

    // adapted from GTK-gnutalla code, and elsewhere

    struct sockaddr_in addr;
	int len = sizeof( struct sockaddr_in );

    int result = getsockname( socketID, (struct sockaddr*)( &addr ), &len );

    if( result == -1 ) {
        return NULL;
        }
    else {

        char *stringAddress = inet_ntoa( addr.sin_addr );

        return new HostAddress( stringDuplicate( stringAddress ),
                                0 );
        }
    
    }



/* timed_read adapted from gnut, by Josh Pieper */
/* Josh Pieper, (c) 2000 */
/* This file is distributed under the GPL, see file COPYING for details */

// exactly like the real read, except that it returns -2
// if no data was read before the timeout occurred...
int timed_read( int inSock, unsigned char *inBuf, 
	int inLen, long inMilliseconds ) {
	fd_set fsr;
	struct timeval tv;
	int ret;
	
	FD_ZERO( &fsr );
	FD_SET( inSock, &fsr );
 
	tv.tv_sec = inMilliseconds / 1000;
	int remainder = inMilliseconds % 1000;
	tv.tv_usec = remainder * 1000;
	
	ret = select( inSock + 1, &fsr, NULL, NULL, &tv );
	
	if( ret==0 ) {
		// printf( "Timed out waiting for data on socket receive.\n" );
		return -2;
		}
	
	if( ret<0 ) {
		printf( "Selecting socket during receive failed.\n" );
		return ret;
		}
	
	ret = recv( inSock, (char*)inBuf, inLen, 0 );
	
	return ret;
	}
