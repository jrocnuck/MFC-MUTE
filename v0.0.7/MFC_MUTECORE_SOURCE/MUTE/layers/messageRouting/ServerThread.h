/*
 * Modification History
 *
 * 2003-July-22    Jason Rohrer
 * Created.
 *
 * 2004-January-11   Jason Rohrer
 * Made include paths explicit to help certain compilers.
 */



#ifndef SERVER_THREAD_INCLUDED
#define SERVER_THREAD_INCLUDED



#include "minorGems/network/p2pParts/OutboundChannel.h"
#include "MUTE/layers/messageRouting/ConnectionMaintainer.h"

#include "minorGems/system/Thread.h"
#include "minorGems/system/MutexLock.h"



/**
 * A thread that receives inbound connections.
 *
 * @author Jason Rohrer
 */
class ServerThread : public Thread {


        
    public:


        
        /**
         * Constructs and starts a server thread.
         *
         * @param inPort the port to listen on.
         * @param inConnectionMaintainer the connection maintainer.
         *   Must be destroyed by caller after this class is destroyed.
         */
        ServerThread( int inPort,
                      ConnectionMaintainer *inConnectionMaintainer );



        /**
         * Stops and destroys this thread.
         *
         * Blocks until thread terminates.
         */
        ~ServerThread();


        
        // implements Thread.run();
        void run();

        
        
    protected:
        int mPort;

        ConnectionMaintainer *mConnectionMaintainer;
        
        MutexLock *mLock;

        char mStopSignal;

        char *mLoggerName;


        
        /**
         * Gets whether stop signal set.
         *
         * Thread-safe.
         *
         * @return true if stop signal set.
         */
        char isStopped();

        
        
    };



#endif
