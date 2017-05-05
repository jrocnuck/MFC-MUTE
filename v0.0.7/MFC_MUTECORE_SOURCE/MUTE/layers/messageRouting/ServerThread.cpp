/*
 * Modification History
 *
 * 2003-July-22    Jason Rohrer
 * Created.
 *
 * 2003-July-27    Jason Rohrer
 * Added a channel thread manager.
 * Changed to pass socket into channel thread for destruction.
 * Moved channel setup into ConnectionMaintainer.
 *
 * 2004-January-11   Jason Rohrer
 * Made include paths explicit to help certain compilers.
 * Fixed bug in handling connection after passing to ConnectionMaintainer.
 *
 * 2004-February-26   Jason Rohrer
 * Changed to not give up when accepting a connection fails.
 */



#include "MUTE/layers/messageRouting/ServerThread.h"
#include "MUTE/layers/messageRouting/ChannelReceivingThread.h"

#include "minorGems/util/log/AppLog.h"
#include "minorGems/network/Socket.h"
#include "minorGems/network/SocketServer.h"
#include "minorGems/network/SocketStream.h"


ServerThread::ServerThread( int inPort,
                            ConnectionMaintainer *inConnectionMaintainer )
    : mPort( inPort ),
      mConnectionMaintainer( inConnectionMaintainer ),
      mLock( new MutexLock() ),
      mStopSignal( false ),
      mLoggerName( stringDuplicate( "ServerThread" ) ) {

    
    
    this->start();
    }



ServerThread::~ServerThread() {
    AppLog::info( mLoggerName, "Stopping." );

    mLock->lock();
    mStopSignal = true;
    mLock->unlock();
    
    this->join();

    delete mLock;
    
    delete [] mLoggerName;
    }



char ServerThread::isStopped() {
    mLock->lock();
    char stopped = mStopSignal;
    mLock->unlock();

    return stopped;
    }



void ServerThread::run() {
    
    SocketServer *server = new SocketServer( mPort, 100 );
	
	// jroc... we don't want to go crazy because of a bad socket
	// binding or something similar... otherwise, we just fill
	// the log file with bad reports of not accepting connections.
	if( server->HasCatastrophicFailure() )
	{
		delete server;
		mStopSignal = true;
		return;
	}
    
    char timedOut = true;

    // 5 seconds
    long timeout = 5000;
    
    Socket *received = NULL;

    
    AppLog::detail( mLoggerName, "Waiting for connection." );
    while( timedOut && !isStopped() ) {
        received = server->acceptConnection( timeout, &timedOut );
        }

    
    while( !isStopped() ) {

        if( received != NULL ) {
            AppLog::info( mLoggerName, "Connection received." );

            mConnectionMaintainer->connectionFormed( received );
            received = NULL;
            }
        else {
            AppLog::error( mLoggerName,
                           "Failed to accept a connection." );
            }

        
        AppLog::detail( mLoggerName, "Waiting for connection." );
        timedOut = true;
        while( timedOut && !isStopped() ) {
            received = server->acceptConnection( timeout, &timedOut );
            }
        }

    if( isStopped() ) {
        AppLog::info( mLoggerName, "Got stop signal." );
        }

    if( received != NULL ) {
        delete received;
        }
    
    delete server;

    AppLog::detail( mLoggerName, "run function returning." );
    }
    
