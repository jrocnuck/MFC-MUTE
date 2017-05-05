/*
 * Modification History
 *
 * 2001-January-10		Jason Rohrer
 * Created.
 *
 * 2002-October-13		Jason Rohrer
 * Added support for timeout on connect.
 */

#include "minorGems/common.h"


#ifndef SOCKET_CLIENT_CLASS_INCLUDED
#define SOCKET_CLIENT_CLASS_INCLUDED

#include "Socket.h"
#include "HostAddress.h"

#include <stdlib.h>

/**
 * Class that can make connections to socket servers.
 *
 * Note:  Implementation for the functions defined here is provided
 *   separately for each platform (in the mac/ linux/ and win32/ 
 *   subdirectories).
 *
 * @author Jason Rohrer
 */ 
class SocketClient {

	public:
		
		/**
		 * Connects to a server and returns a socket.
		 *
		 * @param inAddress the host to connect to.  Must be destroyed
		 *   by caller.
         * @param inTimeoutInMilliseconds the timeout value for the connect
         *   in milliseconds, or -1 for no timeout.  Defaults to -1.
         * @param outTimedOut pointer to where the timeout flag should
         *   be returned, or NULL for no timeout.  If timeout used, will
         *   be set to true if timeout happened, or false if it did not.
         *   Defaults to NULL.
		 *
		 * @return a socket for the connection, or NULL if an error occurs.
		 */
		static Socket *connectToServer( HostAddress *inAddress,
                                        long inTimeoutInMilliseconds = -1,
                                        char *outTimedOut = NULL ); 
	
	};		
	
#endif
