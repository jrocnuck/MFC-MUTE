/*
 * Modification History
 *
 * 2003-July-20    Jason Rohrer
 * Created.
 *
 * 2003-July-27    Jason Rohrer
 * Added socket destruction.
 * Added host list parsing.
 *
 * 2003-August-7   Jason Rohrer
 * Improved handling of remote address.
 * Added a duplicate message detector.
 *
 * 2003-August-11   Jason Rohrer
 * Added use of connection maintainer, generic type used to avoid include loop.
 * Moved readStreamUpToTag into common file.
 *
 * 2003-October-9   Jason Rohrer
 * Added support for message limiters.
 *
 * 2003-October-12   Jason Rohrer
 * Added use of message ID tracker.
 *
 * 2003-December-5   Jason Rohrer
 * Added support for message utility.
 *
 * 2004-January-11   Jason Rohrer
 * Made include paths explicit to help certain compilers.
 *
 * 2004-February-4   Jason Rohrer
 * Added support for new utility counter modification algorithm.
 * Added support for processing FORWARD-flagged messages.
 *
 * 2004-February-13   Jason Rohrer
 * Added Mycroftxxx's patch to close connections that are dropping messages.
 *
 * 2004-March-15   Jason Rohrer
 * Moved forward hash code into messageRouter.cpp.
 */



#ifndef CHANNEL_RECEIVING_THREAD_INCLUDED
#define CHANNEL_RECEIVING_THREAD_INCLUDED


#include "MUTE/layers/messageRouting/MessageIDTracker.h"

#include "minorGems/network/p2pParts/OutboundChannel.h"
#include "minorGems/network/p2pParts/HostCatcher.h"
#include "minorGems/network/p2pParts/MessagePerSecondLimiter.h"


#include "MUTE/layers/messageRouting/LocalAddressReceiver.h"
#include "MUTE/layers/messageRouting/OutboundChannelManager.h"

// avoid include loop by refering to ConnectionMaintainer as a (void *) type
// #include "MUTE/layers/messageRouting/ConnectionMaintainer.h"

#include "minorGems/io/InputStream.h"
#include "minorGems/system/Thread.h"
#include "minorGems/system/MutexLock.h"
#include "minorGems/network/Socket.h"

#include "minorGems/util/random/RandomSource.h"



/**
 * A thread that receives inbound messages on a channel and processes them.
 *
 * @author Jason Rohrer.
 */
class ChannelReceivingThread : public Thread {



    public:
        
        /**
         * Constructs and starts this thread.
         *
         * @param inInputStream the input stream to read messages from.
         *   Will be destroyed when this class is destroyed.
         * @param inOutboundChannel the outbound channel associated with this
         *   thread's channel.
         *   Must be destroyed by caller after this class is destroyed.
         * @param inSocket the socket associated with this channel.
         *   Will be destroyed by this class when the connection breaks
         *   and after this class triggers the destruction of
         *   inOutboundChannel.
         * @param inRemoteAddress the address string for the remote host.
         *   Note that the port number is unknown when this thread is
         *   constructed.
         *   Must be destroyed by caller.
         * @param inRemotePort the port number for the remote host, if known,
         *   or -1 if unknown.
         * @param inMessageReceiver a receiver for locally addressed messages.
         *   Must be destroyed by caller after this class is destroyed.
         * @param inOutboundChannelManager manager of outbound channels.
         *   Must be destroyed by caller after this class is destroyed.
         * @param inConnectionMaintainer the connection maintainer.
         *   Must be cast to (void *) type to avoid an include file loop.
         *   Must be destroyed by caller after this class is destroyed.
         * @param inHostCatcher the host catcher.
         *   Must be destroyed by caller after this class is destroyed.
         * @param inMessageIDTracker the ID tracker.
         *   Must be destroyed by caller after this class is destroyed.
         * @param inLimiter the limiter for inbound messages.
         *   Must be destroyed by caller after this class is destroyed.
         * @param inRandSource the random generator.
         *   Will be destroyed when this class is destroyed.
         */
        ChannelReceivingThread(
            InputStream *inInputStream,
            OutboundChannel *inOutboundChannel,
            Socket *inSocket,
            char *inRemoteAddress,
            int inRemotePort,
            LocalAddressReceiver *inMessageReceiver,
            OutboundChannelManager *inOutboundChannelManager,
            void *inConnectionMaintainer,
            HostCatcher *inHostCatcher,
            MessageIDTracker *inMessageIDTracker,
            MessagePerSecondLimiter *inLimiter,
            RandomSource *inRandSource );


        
        /**
         * Stops and destroys this thread.
         *
         * May block indefinitely while waiting for thread to terminate,
         * since thread may be blocked receiving a message.
         */
        ~ChannelReceivingThread();

        

        /**
         * Gets whether this thread is finished.
         *
         * Thread only finished when the connection breaks.
         */
        char isFinished();



        // implements the Thread interface
        void run();


        
    protected:
        MutexLock *mLock;
        InputStream *mInputStream;
        OutboundChannel *mOutboundChannel;
        Socket *mSocket;
        char *mRemoteAddress;
        int mRemotePort;
        LocalAddressReceiver *mReceiver;
        OutboundChannelManager *mOutboundChannelManager;
        // avoid include file loop
        void *mConnectionMaintainer;
        HostCatcher *mHostCatcher;
        MessageIDTracker *mMessageIDTracker;
        MessagePerSecondLimiter *mLimiter;
        
        char mStopSignal;
        char mFinished;

        char *mLoggerName;

        int mMaxUtilityCounter;
        int mUtilityAlpha;
        int mUtilityBeta;
        int mUtilityGamma;

        RandomSource *mRandSource;

        double mMaxDroppedMessageFraction;


        
        /**
         * Processes the flags on a message to generate new flags and
         * dictate message actions.
         *
         * @param inOldFlags the existing flags on the message.
         *   Must be destroyed by caller.
         * @param outIgnoreUC pointer to location where boolean value
         *   will be returned indicating whether the message's UC should
         *   be ignored.
         * @param outDropMessage pointer to location where boolean value
         *   will be returned indicating whether the message should be dropped.
         * @param inLoggerName the name to use when logging messages.
         *   Must be destroyed by caller.
         * @return the new message flags.
         *   Must be destroyed by caller.
         */
        char *processFlags( char *inOldFlags,
                            char *outIgnoreUC, char *outDropMessage,
                            char *inLoggerName );

        
        
    };




#endif
