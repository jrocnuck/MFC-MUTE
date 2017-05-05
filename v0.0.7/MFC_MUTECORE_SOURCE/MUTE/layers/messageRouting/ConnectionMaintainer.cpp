/*
 * Modification History
 *
 * 2003-July-27   Jason Rohrer
 * Created.  Modified from konspire2b source.
 * Changed to send host list as first thing on all new connections.
 * Fixed stop signal bug.
 * Added support for exchanging port numbers with remote hosts.
 * Fixed bug in host list termination.
 *
 * 2003-August-7   Jason Rohrer
 * Fixed bug in successful connection detection.
 * Improved handling of remote address.
 * Added a duplicate message detector.
 *
 * 2003-August-11   Jason Rohrer
 * Changed so that only connectionFormed and connectionBroken modify current
 * connection count.
 * Added use of connection maintainer in ChannelReceivingThread.
 * Fixed behavior when more connections are not allowed.
 * Fixed a deletion bug.
 * Changed to linger for 2 seconds before closing a connection.
 *
 * 2003-August-12   Jason Rohrer
 * Added connection status (accept/reject) after host list.
 * Changed to send/receive local ports before creating threads for a channel.
 *
 * 2003-August-13   Jason Rohrer
 * Added use of unique node name to prevent self-connection.
 *
 * 2003-August-14   Jason Rohrer
 * Added removal of catcher hosts that send us malformatted information.
 *
 * 2003-August-24   Jason Rohrer
 * Added use of secure streams.
 * Fixed a potential deadlock in the destructor.
 *
 * 2003-October-9   Jason Rohrer
 * Added support for message limiters.
 *
 * 2003-October-12   Jason Rohrer
 * Added use of message ID tracker.
 * Fixed deletion bugs.
 *
 * 2003-October-17   Jason Rohrer
 * Changed to use read timeout for secure stream setup.
 *
 * 2003-December-5   Jason Rohrer
 * Added timestamp counter exchange in connection protocol.
 *
 * 2004-January-1   Jason Rohrer
 * Added support for fetching seed hosts from web caches.
 *
 * 2004-January-11   Jason Rohrer
 * Made include paths explicit to help certain compilers.
 *
 * 2004-January-27   Jason Rohrer
 * Added support for logging connection contents.
 *
 * 2004-February-4   Jason Rohrer
 * Added support passing RandomSource into ChannelReceivingThread constructor.
 *
 * 2004-February-20   Jason Rohrer
 * Added function for getting info about current connection attempt.
 * Fixed a memory bug.
 *
 * 2004-February-29   Jason Rohrer
 * Added tag to dummy IDs passed to tracker so that they log properly.
 *
 * 2004-March-23   Jason Rohrer
 * Added a maximum connection count.
 *
 * 2004-March-28   Jason Rohrer
 * Changed to post address to web caches on a fixed time interval.

 * 2004-August-01  JROC
 * Improved delay problem in closing connection manager when quiting program
 * Increased the amount of hosts to iterate through before "resting"
 *
 * 2004-December-12   Jason Rohrer
 * Added setting for outbound channel send queue sizes.
 */


#include "MUTE/layers/messageRouting/ConnectionMaintainer.h"
#include "MUTE/layers/messageRouting/messageRouter.h"
#include "MUTE/common/SimpleWebHostCache.h"


#include "MUTE/layers/secureStream/SecureStreamFactory.h"
#include "minorGems/io/InputStream.h"
#include "minorGems/io/OutputStream.h"
#include "minorGems/network/LoggingSocketStream.h"


#include "minorGems/util/SettingsManager.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SimpleVector.h"
#include "minorGems/util/StringBufferOutputStream.h"

#include "minorGems/system/Time.h"

#include "minorGems/network/p2pParts/protocolUtils.h"

#include "minorGems/util/random/StdRandomSource.h"

#include <string.h>
#include <stdio.h>



ConnectionMaintainer::ConnectionMaintainer(
    unsigned int inLocalPort,
    char *inNodeUniqueName,
    LocalAddressReceiver *inMessageReceiver,
    OutboundChannelManager *inOutboundChannelManager,
    HostCatcher *inHostCatcher,
    MessageIDTracker *inMessageIDTracker,
    MessagePerSecondLimiter *inOutboundMessagePerSecondLimiter,
    MessagePerSecondLimiter *inInboundMessagePerSecondLimiter )
    : mLock( new MutexLock() ),
      mStopSignal( false ),
      mLocalPort( inLocalPort ),
      mNodeUniqueName( stringDuplicate( inNodeUniqueName ) ),
      mLocalReceiver( inMessageReceiver ),
      mOutboundChannelManager( inOutboundChannelManager ),
      mHostCatcher( inHostCatcher ),
      mMessageIDTracker( inMessageIDTracker ),
      mOutboundMessagePerSecondLimiter( inOutboundMessagePerSecondLimiter ),
      mInboundMessagePerSecondLimiter( inInboundMessagePerSecondLimiter ),
      mThreadManager( new ChannelReceivingThreadManager() ),
      mCurrentConnectionCount( 0 ),
      mCurrentAttemptInfoLock( new MutexLock() ),
      mCurrentAttemptAddress( NULL ),
      mCurrentAttemptPort( -1 ),
      // 0-time (very long ago) to ensure that we post once at startup
      mTimeOfLastAddressPostInSeconds( 0 ),
      mSleepSemaphore( new BinarySemaphore() ) {

    char found;
    int target = SettingsManager::getIntSetting( "targetConnectionCount",
                                                 &found );

    if( found ) {
        mTargetConnectionCount = target;
        }
    else {
        mTargetConnectionCount = 4;
        }


    int max = SettingsManager::getIntSetting( "maxConnectionCount",
                                              &found );

    if( found ) {
        mMaxConnectionCount = max;
        }
    else {
        mMaxConnectionCount = 6;
        }

    if( mMaxConnectionCount < mTargetConnectionCount ) {
        mMaxConnectionCount = mTargetConnectionCount;
        }


    int postInterval =
        SettingsManager::getIntSetting( "webHostCachePostIntervalSeconds",
                                        &found );

    if( found && postInterval >= 0 ) {
        mTimeToWaitBetweenAddressPostsInSeconds = postInterval;
        }
    else {
        // one hour, plus one minute
        mTimeToWaitBetweenAddressPostsInSeconds = 3660;
        }
    
    this->start();
    }



ConnectionMaintainer::~ConnectionMaintainer() {

    mLock->lock();
    mStopSignal = true;
    mLock->unlock();

    // signal to wake the thread up if it is sleeping
    mSleepSemaphore->signal();
    
    join();



    // leave unlocked as we delete the manager so that channel threads
    // can report back to us when their connections break
    // (this avoids deadlock)
    delete mThreadManager;
    
    delete mLock;
    delete mSleepSemaphore;
    
    delete [] mNodeUniqueName;


    // destroy any current attempt info
    setNewCurrentAttempt( NULL, -1 );

    delete mCurrentAttemptInfoLock;
    }



void ConnectionMaintainer::setTargetConnectionCount(
    int inTargetConnectionCount ) {
    mLock->lock();

    mTargetConnectionCount = inTargetConnectionCount;

    if( mMaxConnectionCount < mTargetConnectionCount ) {
        mMaxConnectionCount = mTargetConnectionCount;

        SettingsManager::setSetting( "maxConnectionCount",
                                     mMaxConnectionCount );
        }
    
    SettingsManager::setSetting( "targetConnectionCount",
                                 mTargetConnectionCount );
    
    mLock->unlock();    
    }



int ConnectionMaintainer::getTargetConnectionCount() {
    mLock->lock();

    int target = mTargetConnectionCount;

    mLock->unlock();

    return target;
    }



void ConnectionMaintainer::setMaxConnectionCount(
    int inMaxConnectionCount ) {
    mLock->lock();

    mMaxConnectionCount = inMaxConnectionCount;

    if( mMaxConnectionCount < mTargetConnectionCount ) {
        mMaxConnectionCount = mTargetConnectionCount;
        }
    
    SettingsManager::setSetting( "maxConnectionCount",
                                 mMaxConnectionCount );
    
    mLock->unlock();    
    }



int ConnectionMaintainer::getMaxConnectionCount() {
    mLock->lock();

    int max = mMaxConnectionCount;

    mLock->unlock();

    return max;
    }



char ConnectionMaintainer::connectionFormed( Socket *inNewConnection,
                                             HostAddress *inAddress ) {

    SocketStream *inputStream;
    SocketStream *outputStream;

    
    // check if we should be logging connection contents.
    char found;
    int logConnectionContentsSize =
        SettingsManager::getIntSetting( "logConnectionContents", &found );

    if( found && logConnectionContentsSize > 0 ) {
        // use logging socket streams
        
        // log first 10KiB of each connection

        // name log files using connection time
        unsigned long sec;
        unsigned long msec;
        Time::getCurrentTime( &sec, &msec );
        
        char *logPrefix = autoSprintf( "%d.%d", sec, msec );
        
        inputStream =
            new LoggingSocketStream( inNewConnection, true, false,
                                     logConnectionContentsSize,
                                     logPrefix );
        outputStream =
            new LoggingSocketStream( inNewConnection, false, true,
                                     logConnectionContentsSize,
                                     logPrefix );
        delete [] logPrefix;
        }
    else {
        // use normal socket streams, no logging 
    
        inputStream = new SocketStream( inNewConnection );
        outputStream = new SocketStream( inNewConnection );
        }

    
    // need to set the timeout directly in the input stream
    // timeouts not supported at the secure stream level

    // setting timeout here will cause the SecureStreamFactory to
    // fail if the remote host stalls in sending its key data
    inputStream->setReadTimeout( 5000 );
    

    // construct a secure stream
    char *nodePublicKey =
        SettingsManager::getStringSetting( "nodePublicKey" );
    char *nodePrivateKey =
        SettingsManager::getStringSetting( "nodePrivateKey" );
        
    InputStream *secureInputStream;
    OutputStream *secureOutputStream;

    char streamsFormed =
        SecureStreamFactory::establishStreams( inputStream,
                                               outputStream,
                                               nodePublicKey,
                                               nodePrivateKey,
                                               &secureInputStream,
                                               &secureOutputStream );
    delete [] nodePublicKey;
    delete [] nodePrivateKey;
    
    if( !streamsFormed ) {
        delete inputStream;
        delete outputStream;
        delete inNewConnection;

        
        if( inAddress != NULL ) {
            mHostCatcher->noteHostBad( inAddress );
            }
        
        return false;
        }

    
    // first, send our port number and other local info

    unsigned int counter = mMessageIDTracker->getFreshCounter();
    

    char *localInfoString = autoSprintf(
        "LocalUniqueName: %s\n"
        "LocalPort: %d\n"
        "TimestampCounter: %u\n"
        "EndLocalNodeInformation\n",
        mNodeUniqueName, mLocalPort, counter );

    secureOutputStream->writeString( localInfoString );

    delete [] localInfoString;

    
    // next, we need to read the remote host's local information
    // use a timeout so that we don't block forever if remote host is bad
    char readFailed = true;

    

    char *readRemotePortBuffer =
        readStreamUpToTag( secureInputStream,
                           "EndLocalNodeInformation", 1000 );

    int remotePort;
    char *remoteUniqueName = NULL;
    unsigned int remoteCounter;
    
    if( readRemotePortBuffer != NULL ) {
        // tokenize it
        SimpleVector<char *> *tokenVector =
            tokenizeString( readRemotePortBuffer ); 

        delete [] readRemotePortBuffer;
        
        
        int numTokens = tokenVector->size();

        
        if( numTokens > 6 ) {
            // second token is unique name
            remoteUniqueName =
                stringDuplicate( *( tokenVector->getElement( 1 ) ) );
            
            // fourth token is port

            char *portString = *( tokenVector->getElement( 3 ) );

            int numRead = sscanf( portString, "%d", &remotePort );

            if( numRead == 1 ) {
                
                // sixth token is counter
                char *counterString = *( tokenVector->getElement( 5 ) );

                numRead = sscanf( counterString, "%u", &remoteCounter );

                if( numRead == 1 ) {
                    readFailed = false;
                    }
                }
            }
        
        for( int i=0; i<numTokens; i++ ) {
            delete [] *( tokenVector->getElement( i ) );
            }
        delete tokenVector;
        }

    // reset timeout
    inputStream->setReadTimeout( -1 );
    
    if( readFailed ) {

        AppLog::error( "ConnectionMaintainer",
                       "Failed to receive port or counter from remote host." );
        
        // close connection immediately
        delete secureInputStream;
        delete secureOutputStream;
        delete inNewConnection;
        
        
        if( inAddress != NULL ) {
            mHostCatcher->noteHostBad( inAddress );
            }
        
        return false;
        }


    if( inAddress != NULL ) {
        // we already know the address of this host
        // (since we connected to them?)

        // make sure ports agree
        if( remotePort != inAddress->mPort ) {
            AppLog::error(
                "ConnectionMaintainer",
                "Reported port from remote host disagrees with"
                " port that we think it should report." );

            delete secureInputStream;
            delete secureOutputStream;
            delete inNewConnection;
            delete [] remoteUniqueName;

            mHostCatcher->noteHostBad( inAddress );
            return false;
            }
        }


    if( strcmp( remoteUniqueName, mNodeUniqueName ) == 0 ) {
        // same unique name as us--- self connection?

        AppLog::error(
            "ConnectionMaintainer",
            "Rejected connection from self (same unique node name as us)" );

        delete secureInputStream;
        delete secureOutputStream;
        delete inNewConnection;
        delete [] remoteUniqueName;

        if( inAddress != NULL ) {
            mHostCatcher->noteHostBad( inAddress );
            }
        
        return false;
        }

    
    // next, construct a host list
    // we will send this to the remote host whether or not we accept the
    // connection

    HostAddress *remoteHost = inNewConnection->getRemoteHostAddress();
    if( remoteHost == NULL ) {
        remoteHost = new HostAddress( stringDuplicate( "unknown" ), 0 );
        }

    // set with the port that the remote host sent us
    remoteHost->mPort = remotePort;
	remoteHost->mStartTime = time(NULL);

    mHostCatcher->addHost( remoteHost );
    
    
    StringBufferOutputStream *tempHostListStream =
        new StringBufferOutputStream();
    
    SimpleVector<HostAddress *> *hostList =
        mHostCatcher->getHostList( 50, remoteHost );
    int hostCount = hostList->size();
    char *countString = autoSprintf( "HostCount: %d\n", hostCount );
    
    tempHostListStream->writeString( countString );
    delete [] countString;
    
    for( int i=0; i<hostCount; i++ ) {
        HostAddress *currentHost = *( hostList->getElement( i ) );
        
        char *hostString = autoSprintf( "%s %d\n",
                                        currentHost->mAddressString,
                                        currentHost->mPort );
        tempHostListStream->writeString( hostString );
        delete [] hostString;
        
        delete currentHost;
        }
    delete hostList;

    tempHostListStream->writeString( "EndHostList\n" );
    
    char *hostListString = tempHostListStream->getString();
    delete tempHostListStream;


    
    mLock->lock();

    // allow connections if we are below our max number of connections,
    // even if we are above our target number of connections
    char allowed = false;
    if( mMaxConnectionCount > mCurrentConnectionCount ) {
        allowed = true;
        mCurrentConnectionCount ++;
        }

    mLock->unlock();

    

    if( allowed ) {
        
        char *remoteHostAddress =
            stringDuplicate( remoteHost->mAddressString );

        char queueSizeFound;
        int queueSize =
            SettingsManager::getIntSetting( "sendQueueSizePerConnection",
                                            &queueSizeFound );
        if( !queueSizeFound || queueSize <= 0 ) {
            // default to 300
            queueSize = 300;
            }
        
        OutboundChannel *outChannel =
            new OutboundChannel( secureOutputStream, remoteHost,
                                 mOutboundMessagePerSecondLimiter,
                                 queueSize );

        // send host list and connection status as the "first message"
        // through this channel
        char *acceptedMessage = "Connection: Accepted\nEndConnectionStatus\n";

//        char *firstMessage = concatonate( hostListString, acceptedMessage );
		//jroc fixed spelling error
		char *firstMessage = concatenate( hostListString, acceptedMessage );
        
        char creationAllowed =
            mOutboundChannelManager->channelCreated( remoteUniqueName,
                                                     outChannel,
                                                     secureOutputStream,
                                                     firstMessage );
        delete [] firstMessage;

        
        if( creationAllowed ) {

            // we have established a connection


            
            // update our ID tracker to include their counter

            // create a fresh name just to pass to ID tracker
            char *freshUniqueName = muteGetUniqueName();
            char *messageID = autoSprintf( "SynchWithRemoteCounter%s_%u",
                                           freshUniqueName,
                                           remoteCounter );
            
            mMessageIDTracker->checkIfIDFresh( messageID );

            delete [] freshUniqueName;
            delete [] messageID;
            

            
            // start a thread to receive on the new connection
            ChannelReceivingThread *inputThread =
                new ChannelReceivingThread( secureInputStream,
                                            outChannel,
                                            inNewConnection,
                                            remoteHostAddress,
                                            remotePort,
                                            mLocalReceiver,
                                            mOutboundChannelManager,
                                            this,
                                            mHostCatcher,
                                            mMessageIDTracker,
                                            mInboundMessagePerSecondLimiter,
                                            new StdRandomSource() );
            
            mThreadManager->addThread( inputThread );
            }
        else {
            // we don't want to use the outbound channel to reject,
            // since it will queue our message and possibly lose it when
            // we destroy the channel
            delete outChannel;
            
            // send host list plus reject message            
            secureOutputStream->writeString( hostListString );
            secureOutputStream->writeString(
                "Connection: Rejected\nEndConnectionStatus\n" );

            unsigned long seconds;
            unsigned long milliseconds;
            Time::getCurrentTime( &seconds, &milliseconds );
            
            // make sure all data goes through (wait at most 2 seconds)
            inNewConnection->sendFlushBeforeClose( 2000 );

            unsigned long msBeforeClose =
                Time::getMillisecondsSince( seconds, milliseconds );

            char *logMessage = autoSprintf(
                "Flushing socket after rejection tookk %d ms", msBeforeClose );
                
            AppLog::detail( "ConnectionMaintainer",
                            logMessage );
            delete [] logMessage;

            delete secureInputStream;
            delete secureOutputStream;
            delete inNewConnection;
            
            this->connectionBroken();
            }
        
        delete [] remoteHostAddress;
        }
    else {
        // at least send the host list and reject message
        secureOutputStream->writeString( hostListString );
        secureOutputStream->writeString(
            "Connection: Rejected\nEndConnectionStatus\n" );
        
        delete remoteHost;


        unsigned long seconds;
        unsigned long milliseconds;
        Time::getCurrentTime( &seconds, &milliseconds );
            
        // make sure all data goes through (wait at most 2 seconds)
        inNewConnection->sendFlushBeforeClose( 2000 );
        
        unsigned long msBeforeClose =
            Time::getMillisecondsSince( seconds, milliseconds );
        
        char *logMessage = autoSprintf(
            "Flushing socket after rejection took %d ms", msBeforeClose );
            
        AppLog::detail( "ConnectionMaintainer",
                        logMessage );
        delete [] logMessage;

        delete secureInputStream;
        delete secureOutputStream;
        delete inNewConnection;
        }

    delete [] hostListString;
    

    delete [] remoteUniqueName;

    
    // indicates only whether *we* allow it, ignoring case where
    // connection rejected by OutboundChannelManager
    return allowed;
    }



void ConnectionMaintainer::connectionBroken() {
    mLock->lock();

    if( mCurrentConnectionCount <= 0 ) {
        AppLog::criticalError(
            "ConnectionMaintainer",
            "connectionBroken() called when connection count is 0." );
        }
    else {
        mCurrentConnectionCount --;
        }

    mLock->unlock();
    }


        
void ConnectionMaintainer::run() {

    char stopped;
    mLock->lock();
    stopped = mStopSignal;
    mLock->unlock();
    
    while( ! stopped ) {
        mLock->lock();
        
        char addingHost = false;

        // only add if below our target
        if( mCurrentConnectionCount < mTargetConnectionCount ) {
            addingHost = true;
            }

        mLock->unlock();


        if( addingHost ) {
            // we've decided to add a host

            AppLog::info( "ConnectionMaintainer",
                          "Trying to add a new connection." );

            
            char foundUnconnected = false;
            char foundAny = true;
            
            int tryCount = 0;

            char addingSucceeded = false;
            
            
            // try ten hosts from catcher, then give up and sleep
            while( foundAny && !foundUnconnected && tryCount < 100 ) {

                HostAddress *catcherHost = mHostCatcher->getHost();				
                
                if( catcherHost == NULL ) {
                    foundAny = false;
                    }
                else {                    
					catcherHost->mStartTime = time(NULL);
                    char match = false;

                    
                    // we need to delete this
                    SimpleVector<HostAddress *> *connectedList =
                        mOutboundChannelManager->getConnectedHosts();

                    
                    int numConnectedHosts = connectedList->size();
                
                    for( int i=0; i<numConnectedHosts; i++ ) {

                        HostAddress *connectedHost =
                            *( connectedList->getElement( i ) );

                        if( !match && connectedHost->equals( catcherHost ) ) {
                            match = true;
                            }

                        delete connectedHost;
                        }

                    delete connectedList;
                    
                    
                    if( !match ) {
                        foundUnconnected = true;

                        setNewCurrentAttempt( catcherHost->mAddressString,
                                              catcherHost->mPort );
                        
                        char *logBuffer =
                            autoSprintf( "Trying to connect to %s:%d",
                                         catcherHost->mAddressString,
                                         catcherHost->mPort );
                        AppLog::info( "ConnectionMaintainer",
                                      logBuffer );
                        
                        delete [] logBuffer;
                        
                        
                        char timedOut;
                        
                        Socket *sock =
                            SocketClient::connectToServer( catcherHost,
                                                           5000,
                                                           &timedOut );

                        if( timedOut ) {
                            AppLog::info( "ConnectionMaintainer",
                                          "Timed out connecting to host." );
                            }
                        
                        if( sock != NULL ) {

                            // may not be allowed if another thread
                            // has formed a connection since we last
                            // checked the connection count
                            char allowedByMaintainer =
                                connectionFormed( sock, catcherHost );
                            if( allowedByMaintainer ) {
                                addingSucceeded = true;
                                }
                            }
                        else {
                            char *logMessage =
                                autoSprintf( "Connection failed to %s:%d",
                                             catcherHost->mAddressString,
                                             catcherHost->mPort );
                        
                        
                            AppLog::error( "ConnectionMaintainer",
                                           logMessage );

                            mHostCatcher->noteHostBad( catcherHost );
                                

                            delete [] logMessage;
                            }
                    
                        setNewCurrentAttempt( NULL,
                                              -1 );
                        }
                

                    delete catcherHost;
                
                    tryCount++;
                    }
                
                }

            if( !addingSucceeded ) {
                AppLog::info( "ConnectionMaintainer",
                               "Adding a connection failed." );
                }


            // wait for more hosts to be caught
            if( !foundUnconnected ) {

                AppLog::info( "ConnectionMaintainer",
                              "Found no hosts to connect to." );

                AppLog::info( "ConnectionMaintainer",
                              "Trying to fetch more hosts from web cache." );

                char **seedAddresses;
                int *seedPorts;

                int numFound =
                    SimpleWebHostCache::getSeedNodes( &seedAddresses,
                                                      &seedPorts );

                if( numFound != -1 ) {
                    char *logMessage =
                        autoSprintf( "Got %d from web cache.",
                                     numFound );
                    AppLog::info( "ConnectionMaintainer",
                                  logMessage );

                    delete [] logMessage;
                    
                    for( int i=0; i<numFound; i++ ) {
                        HostAddress *address =
                            new HostAddress( seedAddresses[i], seedPorts[i] );

                        mHostCatcher->addHost( address );
                        delete address;
                        }
                    
                    delete [] seedAddresses;
                    delete [] seedPorts;
                    }
                else {
                    AppLog::warning( "ConnectionMaintainer",
                                     "Failed to get hosts from web cache." );
                    }

                
                mSleepSemaphore->wait( 5000 );
                }
            }
        else {
            mSleepSemaphore->wait( 5000 );
		}


        
        // check if we should post our address to web caches again
        unsigned long currentTimeInSeconds = time( NULL );

        unsigned long timeSinceLastPost =
            currentTimeInSeconds - mTimeOfLastAddressPostInSeconds;
        
        if( timeSinceLastPost > mTimeToWaitBetweenAddressPostsInSeconds ) {

            // time to post address again

            // only post if we're not behind a firewall
            char found;
            int behindFirewallFlag =
                SettingsManager::getIntSetting( "behindFirewall",
                                                &found );
            if( found && behindFirewallFlag != 1 ) {
                // not behind a firewall, so post our address
                // to the web caches
                    
                AppLog::info( "ConnectionMaintainer",
                              "Posting our address to web caches." );
                
                HostAddress *localAddress =
                    HostAddress::getNumericalLocalAddress();

                if( localAddress != NULL ) {
                    
                    SimpleWebHostCache::postLocalAddress(
                        localAddress->mAddressString,
                        mLocalPort );
                    
                    delete localAddress;
                    }
                }

            mTimeOfLastAddressPostInSeconds = currentTimeInSeconds;
            }
        
        

        mLock->lock();
        stopped = mStopSignal;
        mLock->unlock();
        }

    AppLog::info( "ConnectionMaintainer",
                  "Stopping" );
    }


char ConnectionMaintainer::getCurrentConnectionAttempt( char **outHostAddress,
                                                        int *outPort ) {
    char returnValue = false;

    
    mCurrentAttemptInfoLock->lock();

    if( mCurrentAttemptAddress != NULL ) {
        returnValue = true;
        *outHostAddress = stringDuplicate( mCurrentAttemptAddress );
        }
    else {
        *outHostAddress = NULL;
        }
    
    *outPort = mCurrentAttemptPort;

    mCurrentAttemptInfoLock->unlock();

    
    return returnValue;
    }



void ConnectionMaintainer::setNewCurrentAttempt( char *inAddress,
                                                 int inPort ) {

    mCurrentAttemptInfoLock->lock();

    if( mCurrentAttemptAddress != NULL ) {
        delete [] mCurrentAttemptAddress;
        mCurrentAttemptAddress = NULL;
        }

    if( inAddress != NULL ) {
        mCurrentAttemptAddress = stringDuplicate( inAddress );
        mCurrentAttemptPort = inPort;
        }

    mCurrentAttemptInfoLock->unlock();
    }




