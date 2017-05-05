/*
 * Modification History
 *
 * 2001-January-9		Jason Rohrer
 * Created.
 *
 * 2001-January-10		Jason Rohrer
 * Fixed a bug in the receive code (in use of timed_read).
 *
 * 2001-January-15		Jason Rohrer
 * Added a safeguard against receives that exceed the 
 * network buffer size.
 *
 * 2001-January-28		Jason Rohrer
 * Changed to comply with new initSocketFramework Socket interface. 
 * Added a close() call to the destructor.  
 *
 * 2001-January-29		Jason Rohrer
 * Fixed compile bug by including unistd.h for close call.
 *
 * 2001-November-13		Jason Rohrer
 * Changed to use ::send function instead of __send function.
 * Changed timeout parameter to signed, since -1 is a possible argument. 
 *
 * 2001-December-12		Jason Rohrer
 * Changed the include order to make BSD compatible.
 *
 * 2002-August-2   Jason Rohrer
 * Changed to ignore SIGPIPE.
 * Added functon for getting remote host address.
 * Added fix for linux-BSD differences.
 *
 * 2002-November-15   Jason Rohrer
 * Fixed a security hole when getting the remote host address.
 *
 * 2003-February-3   Jason Rohrer
 * Added a function for getting the local host address from a socket.
 *
 * 2003-February-21   Jason Rohrer
 * Fixed a BSD compatibility bug.
 *
 * 2004-January-4   Jason Rohrer
 * Added use of network function locks.
 *
 * 2004-March-23   Jason Rohrer
 * Removed timeout error message.
 */



#include "minorGems/network/Socket.h"
#include "minorGems/network/NetworkFunctionLocks.h"

#include <sys/time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#include <unistd.h>
#include <signal.h>



// BSD does not define socklen_t
#ifdef BSD
typedef int socklen_t;
#endif



// prototypes
int timed_read( int inSock, unsigned char *inBuf, 
	int inLen, long inMilliseconds );



/**
 * Linux-specific implementation of the Socket class member functions.
 *
 * May also be compatible with other POSIX-like systems.
 *
 * To compile:
 * no special options needed
 */



char Socket::sInitialized = false;



int Socket::initSocketFramework() {
    // ignore SIGPIPE, which occurs on send whenever the receiver
    // has closed the socket
    signal(SIGPIPE, SIG_IGN);

    sInitialized = true;
	return 0;
	}



Socket::~Socket() {
	int *socketIDptr = (int *)( mNativeObjectPointer );
	int socketID = socketIDptr[0];
	
	shutdown( socketID, SHUT_RDWR );
	
	close( socketID );
	
	delete [] socketIDptr;
	}



int Socket::send( unsigned char *inBuffer, int inNumBytes ) {
	
	int *socketIDptr = (int *)( mNativeObjectPointer );
	int socketID = socketIDptr[0];
		
	return ::send( socketID, inBuffer, inNumBytes, 0 );
	}
		
		
		
int Socket::receive( unsigned char *inBuffer, int inNumBytes,
	long inTimeout ) {
	
	int *socketIDptr = (int *)( mNativeObjectPointer );
	int socketID = socketIDptr[0];
	
	if( inTimeout == -1 ) {
		return recv( socketID, inBuffer, inNumBytes, MSG_WAITALL );
		}
	else {		
		return timed_read( socketID, inBuffer, inNumBytes, inTimeout );
		}
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
	socklen_t len = sizeof( struct sockaddr_in );

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
	
	//ret = fcntl( inSock, F_SETFL, O_NONBLOCK );
	
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
	
	ret = recv( inSock, inBuf, inLen, MSG_WAITALL );
	
	//fcntl( inSock, F_SETFL, 0 );
	
	return ret;
	}
