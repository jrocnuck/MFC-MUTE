/*
 * Modification History
 *
 * 2003-July-27   Jason Rohrer
 * Created.  Modified from konspire2b source.
 * Added support for exchanging port numbers with remote hosts.
 *
 * 2003-August-7   Jason Rohrer
 * Fixed bug in successful connection detection.
 * Improved handling of remote address.
 * Added a duplicate message detector.
 *
 * 2003-August-13   Jason Rohrer
 * Added use of unique node name to prevent self-connection.
 *
 * 2003-August-14   Jason Rohrer
 * Added removal of catcher hosts that send us malformatted information.
 *
 * 2003-October-9   Jason Rohrer
 * Added support for message limiters.
 *
 * 2003-October-12   Jason Rohrer
 * Added use of message ID tracker.
 *
 * 2004-January-11   Jason Rohrer
 * Made include paths explicit to help certain compilers.
 *
 * 2004-February-20   Jason Rohrer
 * Added function for getting info about current connection attempt.
 *
 * 2004-March-23   Jason Rohrer
 * Added a maximum connection count.
 *
 * 2004-March-28   Jason Rohrer
 * Changed to post address to web caches on a fixed time interval.
 *
 * 2004-December-13    Jason Rohrer
 * Switched to use a binary semaphore for sleeping.
 */



#ifndef CONNECTION_MAINTAINER_INCLUDED
#define CONNECTION_MAINTAINER_INCLUDED



#include "MUTE/layers/messageRouting/ChannelReceivingThreadManager.h"

#include "MUTE/layers/messageRouting/ChannelReceivingThread.h"
#include "MUTE/layers/messageRouting/MessageIDTracker.h"

#include "MUTE/layers/messageRouting/LocalAddressReceiver.h"
#include "MUTE/layers/messageRouting/OutboundChannelManager.h"
#include "minorGems/network/p2pParts/OutboundChannel.h"
#include "minorGems/network/p2pParts/HostCatcher.h"
#include "minorGems/network/p2pParts/MessagePerSecondLimiter.h"

#include "minorGems/network/HostAddress.h"


#include "minorGems/system/MutexLock.h"
#include "minorGems/network/Socket.h"
#include "minorGems/network/SocketClient.h"
#include "minorGems/network/SocketStream.h"

#include "minorGems/util/log/AppLog.h"

#include "minorGems/system/Thread.h"
#include "minorGems/system/BinarySemaphore.h"



/**
 * Maintains a specified number of connections.
 *
 * @author Jason Rohrer
 */
class ConnectionMaintainer : public Thread {

        

    public:


        /**
         * Constructs and starts a maintainer.
         *
         * @param inLocalPort the port this node is listening on.
         * @param inNodeUniqueName the unique name of this node.
         *    Must be destroyed by caller.
         * @param inMessageReceiver a receiver for locally addressed messages.
         *   Must be destroyed by caller after this class is destroyed.
         * @param inOutboundChannelManager manager of outbound channels.
         *   Must be destroyed by caller after this class is destroyed.
         * @param inHostCatcher the host catcher to get hosts from.
         *   Must be destroyed by caller after this class is destroyed.
         * @param inMessageIDTracker the ID tracker.
         *   Must be destroyed by caller after this class is destroyed.
         * @param inOutboundMessagePerSecondLimiter a limiter for outbound
         *   messages.
         *   Must be destroyed by caller after this class is destroyed.
         * @param inInboundMessagePerSecondLimiter a limiter for inbound
         *   messages.
         *   Must be destroyed by caller after this class is destroyed.
         */
        ConnectionMaintainer(
            unsigned int inLocalPort,
            char *inNodeUniqueName,
            LocalAddressReceiver *inMessageReceiver,
            OutboundChannelManager *inOutboundChannelManager,
            HostCatcher *inHostCatcher,
            MessageIDTracker *inMessageIDTracker,
            MessagePerSecondLimiter *inOutboundMessagePerSecondLimiter,
            MessagePerSecondLimiter *inInboundMessagePerSecondLimiter );



        /**
         * Stops and destroys this maintainer.
         */
        ~ConnectionMaintainer();



        /**
         * Sets the number of connections that this class will try
         * to maintain.
         *
         * @param inTargetConnectionCount the number of connections
         *   to maintain.
         */
        void setTargetConnectionCount( int inTargetConnectionCount );



        /**
         * Gets the number of connections that this class will try
         * to maintain.
         *
         * @return the number of connections being maintained.
         */
        int getTargetConnectionCount();



        /**
         * Sets the maximum number of connections that this class will allow.
         *
         * @param inMaxConnectionCount the maximum number of connections
         *   to maintain.
         */
        void setMaxConnectionCount( int inMaxConnectionCount );



        /**
         * Gets the maximum number of connections that this class will allow.
         *
         * @return the maximum number of connections allowed.
         */
        int getMaxConnectionCount();


        
        /**
         * Gets the host that this node is currently trying to connect to.
         *
         * @param outHostAddress pointer to location where the address of the
         *   host we're trying to connect to should be returned.
         *   NULL will be returned if we are not trying to connect.
         *   The returned string must be destroyed by caller if non-NULL.
         * @param outPort pointer to the location where the port of the remote
         *   host should be returned.
         *
         * @return true if we are trying to connect, or false otherwise.
         */
        char getCurrentConnectionAttempt( char **outHostAddress,
                                          int *outPort );

        

        /**
         * Notifies this manager that a permanent connection has been formed.
         *
         * Thread safe.
         *
         * @param inNewConnection the socket for the new connection.
         *   Will be destroyed by this class.
         * @param inAddress the address, if known, of the remote host
         *   (usually known only if we connected to them), or NULL.
         *   Defaults to NULL.
         *   Must be destroyed by caller if non-NULL.
         *
         * @return true if the connection is allowed, or false if
         *   no more connections are allowed (in which case, the connection
         *   will be broken immediately by this maintainer).
         */
        char connectionFormed( Socket *inNewConnection,
                               HostAddress *inAddress = NULL );


        
        /**
         * Notifies this manager that a permanent connection has been broken.
         *
         * Thread safe.
         */
        void connectionBroken();

        
        
        // implements the Thread interface
        void run();


        
    protected:

        MutexLock *mLock;
        char mStopSignal;

        unsigned int mLocalPort;
        char *mNodeUniqueName;
        
        LocalAddressReceiver *mLocalReceiver;
        OutboundChannelManager *mOutboundChannelManager;
        HostCatcher *mHostCatcher;
        MessageIDTracker *mMessageIDTracker;
        MessagePerSecondLimiter *mOutboundMessagePerSecondLimiter;
        MessagePerSecondLimiter *mInboundMessagePerSecondLimiter;

        
        ChannelReceivingThreadManager *mThreadManager;
        
        int mTargetConnectionCount;
        int mMaxConnectionCount;

        int mCurrentConnectionCount;
        
        
        MutexLock *mCurrentAttemptInfoLock;
        
        // NULL if not currently attempting to connect
        char *mCurrentAttemptAddress;
        int mCurrentAttemptPort;


        
        unsigned long mTimeToWaitBetweenAddressPostsInSeconds;
        unsigned long mTimeOfLastAddressPostInSeconds;
        
        BinarySemaphore *mSleepSemaphore;

        
        /**
         * Sets the current attempt info.
         *
         * Thread safe.
         *
         * @param inAddress the address of the attempt, or NULL to indicate
         *   that no attempt is currently being made.
         *   Must be destroyed by caller.
         * @param inPort the port of the attempt.  Ignored if inAddress is
         *   NULL.
         */
        void setNewCurrentAttempt( char *inAddress, int inPort );

        
        
    };



#endif
