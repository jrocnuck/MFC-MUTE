/*
 * Modification History
 *
 * 2003-July-20    Jason Rohrer
 * Created.
 *
 * 2003-July-27    Jason Rohrer
 * Added socket destruction.
 * Added host list parsing.
 * Added support for exchanging port numbers with remote hosts.
 *
 * 2003-August-7   Jason Rohrer
 * Improved handling of remote address.
 * Fixed a bug in using string after deletion.
 * Fixed a memory leak.
 * Added a duplicate message detector.
 *
 * 2003-August-11   Jason Rohrer
 * Added use of connection maintainer, generic type used to avoid include loop.
 * Added log message when tag search fails for incoming message.
 * Changed to break connection if host list receipt fails.
 * Fixed bugs in bad received message logging.
 *
 * 2003-August-12   Jason Rohrer
 * Added connection status (accept/reject) after host list.
 * Moved readStreamUpToTag into common file.
 * Moved remote port reading code into ConnectionMaintainer.
 *
 * 2003-August-14   Jason Rohrer
 * Added function for registering backrouting information in
 * OutboundChannelManager.
 *
 * 2003-September-9   Jason Rohrer
 * Removed print statement.
 *
 * 2003-October-9   Jason Rohrer
 * Added support for message limiters.
 *
 * 2003-October-12   Jason Rohrer
 * Added use of message ID tracker.
 *
 * 2003-October-17   Jason Rohrer
 * Fixed to support broadcasts.
 * Fixed to include channel argument when routing.
 *
 * 2003-November-20   Jason Rohrer
 * Added a runtime-toggled connection log.
 *
 * 2003-November-24   Jason Rohrer
 * Added support for flags.
 *
 * 2003-December-5   Jason Rohrer
 * Added support for message utility.
 *
 * 2003-December-25   Jason Rohrer
 * Improved position of message payload logging.
 *
 * 2003-December-26   Jason Rohrer
 * Fixed utility counter default.  Added better logging about utility.
 *
 * 2004-January-11   Jason Rohrer
 * Made include paths explicit to help certain compilers.
 *
 * 2004-January-27   Jason Rohrer
 * Changed to fix negative utility counters before routing messages.
 *
 * 2004-February-4   Jason Rohrer
 * Added support for new utility counter modification algorithm.
 * Added support for processing FORWARD-flagged messages.
 *
 * 2004-February-13   Jason Rohrer
 * Added Mycroftxxx's patch to close connections that are dropping messages.
 *
 * 2004-February-26   Jason Rohrer
 * Fixed spelling error in variable name, thanks to Eric Nuckols.
 *
 * 2004-March-1   Jason Rohrer
 * Added several improvements to the handling of FORWARD-flagged messages.
 *
 * 2004-March-9   Jason Rohrer
 * Added support for new FORWARD scheme.
 *
 * 2004-March-15   Jason Rohrer
 * Moved forward hash code into messageRouter.cpp.
 *
 * 2004-March-19   Jason Rohrer
 * Added support for DROP tails.
 *
 * 2004-March-21   Jason Rohrer
 * Added code to switch into DROP_CHAIN mode if UC too high.
 * 2005-April-15   Jason Rohrer
 * Changed to use drop trees instead of drop chains.
 */



#include "MUTE/layers/messageRouting/ChannelReceivingThread.h"
// safe to include in the .cpp file (to avoid an include file loop)
#include "MUTE/layers/messageRouting/ConnectionMaintainer.h"

#include "MUTE/layers/messageRouting/messageRouter.h"


#include "minorGems/util/log/AppLog.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SettingsManager.h"

#include <stdio.h>

#include "minorGems/network/p2pParts/protocolUtils.h"

#include "minorGems/crypto/hashes/sha1.h"



ChannelReceivingThread::ChannelReceivingThread(
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
    RandomSource *inRandSource )
    : mLock( new MutexLock() ),
      mInputStream( inInputStream ),
      mOutboundChannel( inOutboundChannel ),
      mSocket( inSocket ),
      mRemoteAddress( stringDuplicate( inRemoteAddress ) ),
      mRemotePort( inRemotePort ),
      mReceiver( inMessageReceiver ),
      mOutboundChannelManager( inOutboundChannelManager ),
      mConnectionMaintainer( inConnectionMaintainer ),
      mHostCatcher( inHostCatcher ),
      mMessageIDTracker( inMessageIDTracker ),
      mLimiter( inLimiter ),
      mStopSignal( false ),
      mFinished( false ),
      mLoggerName( NULL ),
      mRandSource( inRandSource ) {


    mLoggerName = autoSprintf( "ChannelReceivingThread %s:%d\n",
                               mRemoteAddress, mRemotePort );

    char found;
    int maxUtilityCounterSetting =
        SettingsManager::getIntSetting( "maxMessageUtility", &found );  

    if( found ) {
        mMaxUtilityCounter = maxUtilityCounterSetting;
        }
    else {
        // default to 100
        mMaxUtilityCounter = 100;
        }

    
    int utilityAlphaSetting =
        SettingsManager::getIntSetting( "utilityAlpha", &found );  

    if( found ) {
        mUtilityAlpha = utilityAlphaSetting;
        }
    else {
        // default to 1
        mUtilityAlpha = 1;
        }


    int utilityBetaSetting =
        SettingsManager::getIntSetting( "utilityBeta", &found );  

    if( found ) {
        mUtilityBeta = utilityBetaSetting;
        }
    else {
        // default to 1
        mUtilityBeta = 1;
        }


    int utilityGammaSetting =
        SettingsManager::getIntSetting( "utilityGamma", &found );  

    if( found ) {
        mUtilityGamma = utilityGammaSetting;
        }
    else {
        // default to 0
        mUtilityGamma = 0;
        }
    

    float maxDroppedMessageFractionSetting =
        SettingsManager::getFloatSetting( "maxDroppedMessageFraction",
                                        &found );  

    if( found ) {
        mMaxDroppedMessageFraction = maxDroppedMessageFractionSetting;
        }
    else {
        // default to 10%
        mMaxDroppedMessageFraction = 0.1;
        }
    
    
    this->start();
    }



ChannelReceivingThread::~ChannelReceivingThread() {
    mLock->lock();
    mStopSignal = true;
	//jroc
	if( !( (NULL == (unsigned int) mSocket) || (0xcccccccc == (unsigned int) mSocket) ) )
	{
		mSocket->PrepareForDelete();
	}
    mLock->unlock();
    
    this->join();

    delete mInputStream;
    delete mLock;
    delete [] mRemoteAddress;
    delete [] mLoggerName;

    delete mRandSource;
    }



char ChannelReceivingThread::isFinished() {
    mLock->lock();

    char returnValue = mFinished;

    mLock->unlock();

    return returnValue;
    }



void ChannelReceivingThread::run() {
    AppLog::info( mLoggerName, "Starting up." );

    char correctMessageReceived = true;
    
    // first, receive the incoming host list
    // read up to the first occurrence of "EndHostList", allowing 10,000
    // characters
    char *readListBuffer = readStreamUpToTag( mInputStream,
                                              "EndHostList", 10000 );
    if( readListBuffer != NULL ) {

        AppLog::trace( mLoggerName, readListBuffer );
        
        // tokenize it
        SimpleVector<char *> *tokenVector =
            tokenizeString( readListBuffer ); 

        delete [] readListBuffer;
        
        
        int numTokens = tokenVector->size();
        
        // count should be second token, so skip first two

        int i;
        for( i=2; i<numTokens-1; i+=2 ) {

            char *addressString = *( tokenVector->getElement( i ) );
            char *portString = *( tokenVector->getElement( i + 1 ) );

            int port;

            int numRead = sscanf( portString, "%d", &port );

            if( numRead == 1 ) {
                HostAddress *address = new HostAddress(
                    stringDuplicate( addressString ), port );
                mHostCatcher->addHost( address );
                delete address;
                }
            }

        for( i=0; i<numTokens; i++ ) {
            delete [] *( tokenVector->getElement(i) );
            }

        delete tokenVector;
        }
    else {
        AppLog::error( mLoggerName, "Failed to receive host list." );
        correctMessageReceived = false;
        }


    // check for an accepted connection status
    
    char connectionAccepted = false;
    
    if( correctMessageReceived ) {
        correctMessageReceived = false;
        
        // next, receive the connection status
        char *readStatusBuffer =
            readStreamUpToTag( mInputStream, "EndConnectionStatus", 1000 );

        if( readStatusBuffer != NULL ) {
            AppLog::trace( mLoggerName, readStatusBuffer );
            
            // tokenize it
            SimpleVector<char *> *tokenVector =
                tokenizeString( readStatusBuffer ); 

            delete [] readStatusBuffer;
        
        
            int numTokens = tokenVector->size();

            if( numTokens > 2 ) {
                // second token is status
                char *status = *( tokenVector->getElement( 1 ) );

                if( strcmp( status, "Accepted" ) == 0 ) {
                    connectionAccepted = true;
                    correctMessageReceived = true;
                    // we are actually connected now

                    // log it, if needed
                    char found;
                    int logConnectionsFlag =
                        SettingsManager::getIntSetting( "logConnections",
                                                        &found );

                    if( found && logConnectionsFlag == 1 ) {
                        FILE *connectionLogFILE =
                            fopen( "connectionHistory.log", "a" );

                        if( connectionLogFILE != NULL ) {

                            fprintf( connectionLogFILE, "%s : %d\n",
                                     mRemoteAddress,
                                     mRemotePort );

                            fclose( connectionLogFILE );
                            }
                        }
                    
                    }
                else if( strcmp( status, "Rejected" ) == 0 ) {
                    connectionAccepted = false;
                    correctMessageReceived = true;

                    AppLog::info( mLoggerName,
                                  "Connection rejected by remote host." );
                    }                    
                }

            for( int i=0; i<numTokens; i++ ) {
                delete [] *( tokenVector->getElement( i ) );
                }
            delete tokenVector;            
            }
        
        if( !correctMessageReceived ) {
            AppLog::error( mLoggerName,
                           "Failed to receive connection status." );
            }
        }


    
    
    // now read a series of MUTE messages 
    
    mLock->lock();
    char stopped = mStopSignal;
    mLock->unlock();

    while( !stopped && correctMessageReceived  && connectionAccepted ) {

        // read a message
        correctMessageReceived = false;
        


        // read up to the first occurrence of "Body:"
        char *readCharBuffer = readStreamUpToTag( mInputStream,
                                                  "Body:", 5000 );
                
        if( readCharBuffer != NULL ) {
            // buffer ends with "Body:" plus terminating character

            AppLog::detail( mLoggerName, "Got header" );
            AppLog::trace( mLoggerName, readCharBuffer );

            // tokenize it
            SimpleVector<char *> *tokenVector =
                tokenizeString( readCharBuffer ); 

            char *messageID = NULL;
            char *fromAddress = NULL;
            char *toAddress = NULL;
            char *flags = NULL;
            int utilityCounter = 0;
            int payloadLength = -1;
            char *dataPayload = NULL;

            int numTokens = tokenVector->size();

            int i;
            for( i=0; i<numTokens-1; i++ ) {
                char *currentToken = *( tokenVector->getElement( i ) );
                char *nextToken = *( tokenVector->getElement( i + 1 ) );

                if( strcmp( currentToken, "UniqueID:" ) == 0 ) {
                    messageID = stringDuplicate( nextToken );
                    }
                else if( strcmp( currentToken, "From:" ) == 0 ) {
                    fromAddress = stringDuplicate( nextToken );
                    }
                else if ( strcmp( currentToken, "To:" ) == 0 ) {
                    toAddress = stringDuplicate( nextToken );
                    }
                else if ( strcmp( currentToken, "Flags:" ) == 0 ) {
                    flags = stringDuplicate( nextToken );
                    }
                else if ( strcmp( currentToken, "UtilityCounter:" ) == 0 ) {
                    sscanf( nextToken, "%d", &utilityCounter );
                    }
                else if ( strcmp( currentToken, "Length:" ) == 0 ) {
                    sscanf( nextToken, "%d", &payloadLength );
                    }
                }

            // destroy vector
            for( i=0; i<numTokens; i++ ) {
                delete [] *( tokenVector->getElement( i ) );
                }
            delete tokenVector;

            
            if( messageID != NULL &&
                fromAddress != NULL &&
                toAddress != NULL &&
                flags != NULL &&
                payloadLength > 0 ) {

                // all header items read correctly

                // make sure payload not too big
                if( payloadLength <= 32768 ) {

                    dataPayload = new char[ payloadLength + 1 ];

                    int numRead =
                        mInputStream->read( (unsigned char *)dataPayload,
                                            payloadLength );

                    // terminate payload string
                    if( numRead > 0 && numRead <= payloadLength ) {
                        dataPayload[ numRead ] = '\0';
                        }
                    else {
                        dataPayload[ payloadLength ] = '\0';
                        }
                    
                    if( numRead == payloadLength ) {                        
                        // we've already traced the header
                        AppLog::trace( mLoggerName, dataPayload );
                        
                        correctMessageReceived = true;
                        // message formatted correctly,
                        // so process it

                        // obey the limit
                        // we will block here if message rate is too high
                        mLimiter->messageTransmitted();


                       
                        
                        // process the message
                        //printf( "Registering received ID %s\n", messageID );
                        char fresh =
                            mMessageIDTracker->checkIfIDFresh(
                                messageID );

                        if( fresh ) {
                            AppLog::detail( mLoggerName,
                                            "Message fresh." );
                            }
                        else {
                            AppLog::detail( mLoggerName,
                                            "Message stale." );
                            }

                        char ignoreUC;
                        char dropMessage;
                        
                        char *newFlags = processFlags( flags,
                                                       &ignoreUC,
                                                       &dropMessage,
                                                       mLoggerName );
                        
                        delete [] flags;
                        flags = newFlags;

                        if( dropMessage ) {
                            AppLog::detail(
                                mLoggerName,
                                "Dropping message based on flags." );
                            }
                        

                        // ignore messages that are not fresh
                        // or that should be dropped according to flags.
                        if( fresh && !dropMessage ) {
                            
                                
                            if( strstr( flags, "FRESH_ROUTE" ) != NULL ) {
                                // clear the routing information in both
                                // directions
                                mOutboundChannelManager->
                                    clearRoutingInformation( fromAddress );
                                mOutboundChannelManager->
                                    clearRoutingInformation( toAddress );
                                }
                            
                            // add new back-routing information 
                            mOutboundChannelManager->addRoutingInformation(
                                fromAddress,
                                mOutboundChannel );

                            int generatedUtility = 0;
                            char receivedLocally;

                            if( strstr( flags, "DROP_TTL" ) != NULL ) {
                                // don't locally-process DROP_TTL messages
                                receivedLocally = false;
                                }
                            else {
                                receivedLocally =
                                    mReceiver->messageReceived(
                                        fromAddress,
                                        toAddress,
                                        dataPayload,
                                        &generatedUtility );
                                }
                            
                            if( receivedLocally ) {
                                AppLog::detail(
                                    mLoggerName,
                                    "Message consumed locally." );
                                }

                            
                            // check this even if ignoring UCs
                            if( utilityCounter < 0 ) {
                                // correct negative utility counters
                                utilityCounter = 0;
                                }

                            // we might generate utility for any message
                            // type, either routed or broadcast (ALL),
                            // that we do not consume locally

                            if( !ignoreUC ) {
                                // add in our weighted, generated utility
                                utilityCounter +=
                                    mUtilityAlpha * generatedUtility;
                                }

                            
                                // if we didn't consume message locally
                                // or if message is a broadcast
                            if( ! receivedLocally ||
                                strcmp( toAddress, "ALL" ) == 0 ) {


                                if( strcmp( toAddress, "ALL" ) == 0 ) {

                                    if( !ignoreUC ) {
                                        // for ALL messages, also add in
                                        // the weighted (beta) branching
                                        // factor at this node and the
                                        // (gamma) constant factor

                                        int numNeighbors =
                                            mOutboundChannelManager->
                                            getConnectionCount();
                                        
                                        // we won't send to the neighbor that
                                        // sent the message to us
                                        int branchFactor = numNeighbors - 1;
                                        
                                        utilityCounter +=
                                            mUtilityBeta * branchFactor +
                                            mUtilityGamma;
                                        }
                                    }

                                if( !ignoreUC &&
                                    utilityCounter > mMaxUtilityCounter ) {

                                    // we have pushed the UC over the top

                                    // switch the message into DROP_CHAIN
                                    // mode to start the drop tail
                                    // the OutboundChannelManager will handle
                                    // the chaining and the side DROP_TTL
                                    // trees when it sees the DROP_CHAIN flag
                                    
                                    char *tempFlags =
                                        muteAddFlag( flags,
                                                     "DROP_CHAIN" );
                                    delete [] flags;
                                    flags = tempFlags;

                                    AppLog::detail(
                                        mLoggerName,
                                        "Message switched into DROP_CHAIN "
                                        " mode because its"
                                        " utility is too high.");
                                    // if we switch a message into DROP_CHAIN
                                    // mode, we should pass it to the
                                    // OutboundChannelManager even if
                                    // the number of neighbors that we send
                                    // DROP_CHAIN messages to is 0.
                                    // In these cases, we will be sending
                                    // DROP_TTL messages to all neighbors.
                                    // We must do this because we have
                                    // already processed the message above
                                    // before we decide to change it into
                                    // DROP_CHAIN mode.  If we simply
                                    // drop it at this point, our neighbors
                                    // could tell that we didn't send out
                                    // any DROP_TTL message (and therefore
                                    // that we weren't passing the DROP_CHAIN
                                    // message on to any neighbors).  They
                                    // could be sure the results they see
                                    // are coming from us.
                                    }
                                
                                if( !dropMessage ) {
                                // route it
                                // this manager will pay attention
                                // to flags when routing.
                                mOutboundChannelManager->routeMessage(
                                    messageID,
                                    fromAddress,
                                    toAddress,
                                    flags,
                                    utilityCounter,
                                    dataPayload,
                                    mOutboundChannel );
                                    
                                AppLog::detail(
                                    mLoggerName,
                                    "Message routed onward." );
                                    }
                                }
                            }                        
                        }
                    else {
                        AppLog::error( mLoggerName,
                                       "Failed to read message data payload");
                        }
                    }
                else {
                    AppLog::error( mLoggerName,
                                   "Failed to read message payload too long");
                    }
                }
            else {
                AppLog::error( mLoggerName,
                               "Payload length negative");
                }
                


            // clean up, whether correct message was received or not
            if( messageID != NULL ) {
                delete [] messageID;
                }
            if( fromAddress != NULL ) {
                delete [] fromAddress;
                }
            if( toAddress != NULL ) {
                delete [] toAddress;
                }
            if( flags != NULL ) {
                delete [] flags;
                }
            if( dataPayload != NULL ) {
                delete [] dataPayload;
                }

            delete [] readCharBuffer;
            }
        

        if( correctMessageReceived ) {
            AppLog::detail( mLoggerName, "Correct message received" );
            }
        else {
            AppLog::error( mLoggerName, "Bad message received" );
            }
        
        mLock->lock();
        stopped = mStopSignal;		
        mLock->unlock();


        
        // Check for excessive dropped messsages.  It would normally make
        // more sense to do this in OutboundChannelManager or OutboundChannel,
        // but this thread has all the info and access to the right classes to
        // close both sides cleanly, and they don't.  It's easier to
        // just do it here than to change everything around.  The only
        // downside is that it means that a channel with too many drops won't
        // notice it and terminate until the next message gets received.
        // Good enough.
        int sentCount = mOutboundChannel->getSentMessageCount();
        int droppedCount = mOutboundChannel->getDroppedMessageCount();
        
        if( droppedCount > sentCount * mMaxDroppedMessageFraction ) {
            AppLog::error( mLoggerName,
                           "Too many dropped messages, "
                           "forcing connection to break." );
            stopped = true;
            }        
        }

    // however we got here, we should remove the host from the catcher
    // (rejected our connection, broke the connection, sent a bad message,
    //  dropped too many messages)
    HostAddress *remoteHost =
        new HostAddress( stringDuplicate( mRemoteAddress ),
                         mRemotePort );
    mHostCatcher->noteHostBad( remoteHost );
    delete remoteHost;

    // let channel manager know that channel has been broken
    mOutboundChannelManager->channelBroken( mOutboundChannel );
 
	// cast to true type
    ConnectionMaintainer *maintainer =
        (ConnectionMaintainer *)mConnectionMaintainer;
    maintainer->connectionBroken();
    
    
    mLock->lock();
    mFinished = true;
	// JROC.. MOVED HERE TO ATTEMPT TO HELP CLEAN UP BLOCKING ON BAD SOCKET READ/SENDS..
	// REALLY SPEEDS THINGS UP... BUT STILL NEEDS SOME WORK...
	// destroy the socket

    delete mSocket;
	mSocket = NULL;
    mLock->unlock();
    }



char *ChannelReceivingThread::processFlags( char *inOldFlags,
                                            char *outIgnoreUC,
                                            char *outDropMessage,
                                            char *inLoggerName ) {

    char *flags = stringDuplicate( inOldFlags );

    *outIgnoreUC = false;
    *outDropMessage = false;
    
    
    // process a FORWARD flag
    char *pointerToForwardFlag =
        strstr( flags, "FORWARD_" );
                        
    if( pointerToForwardFlag != NULL ) {
        // message has forward flag

        // extract the hash from the forward flag
        char *oldForwardFlag =
            stringDuplicate( pointerToForwardFlag );
        char *pointerToFlagSeparator =
            strstr( oldForwardFlag, "|" );

        if( pointerToFlagSeparator != NULL ) {
            // terminate string at separator
            pointerToFlagSeparator[0] = '\0';
            }

        // skip FORWARD_ to get to the hash
        char *pointerToHash =
            &( oldForwardFlag[
                strlen( "FORWARD_" ) ] );


        // re-hash the hash to produce a new hash
        char *newHash = muteComputeNewForwardHash( pointerToHash );

        if( newHash != NULL ) {
            // continue forwarding
            *outIgnoreUC = true;
                                
            char *newForwardFlag =
                autoSprintf( "FORWARD_%s", newHash );

            
            // replace old flag with new one
                                
            char *tempFlags =
                muteRemoveFlag( flags,
                                oldForwardFlag );
                                
            char *newFlags =
                muteAddFlag( tempFlags,
                             newForwardFlag );
            delete [] tempFlags;
            delete [] flags;
            delete [] newForwardFlag;
            
            flags = newFlags;

            
            delete [] newHash;
            }
        else {
            // we're breaking the forward tree
            *outIgnoreUC = false;
                                    
            // remove the forward flag
            char *newFlags =
                muteRemoveFlag( flags,
                                oldForwardFlag );
            delete [] flags;
            flags = newFlags;

            AppLog::detail(
                inLoggerName,
                "Breaking the FORWARD tree." );
            }

        delete [] oldForwardFlag;
        }


    
    // process a DROP_TTL flag
    char *pointerToDropTTLFlag =
        strstr( flags, "DROP_TTL_" );

    if( pointerToDropTTLFlag != NULL ) {
        // message has drop TTL flag

        // extract the hash from the dropTTL flag
        char *oldDropTTLFlag =
            stringDuplicate( pointerToDropTTLFlag );
        char *pointerToFlagSeparator =
            strstr( oldDropTTLFlag, "|" );

        if( pointerToFlagSeparator != NULL ) {
            // terminate string at separator
            pointerToFlagSeparator[0] = '\0';
            }

        // skip DROP_TTL_ to get to the TTL value
        char *pointerToTTLValue =
            &( oldDropTTLFlag[
                strlen( "DROP_TTL_" ) ] );


        int ttlValue;

        int numRead = sscanf( pointerToTTLValue, "%d", &ttlValue ); 

        if( numRead == 1 ) {

            ttlValue--;

            if( ttlValue <= 0 ) {
                *outDropMessage = true;

                AppLog::detail(
                    inLoggerName,
                    "Breaking the DROP_TTL tree, since the TTL reached 0." );
                }
            else {
                char *newDropTTLFlag = autoSprintf( "DROP_TTL_%d", ttlValue );

                // replace old flag with new one
                                
                char *tempFlags =
                    muteRemoveFlag( flags,
                                    oldDropTTLFlag );
                                
                char *newFlags =
                    muteAddFlag( tempFlags,
                                 newDropTTLFlag );
                delete [] tempFlags;
                delete [] flags;
                delete [] newDropTTLFlag;
            
                flags = newFlags;
                }
            
            // ignore the UC in drop mode
            *outIgnoreUC = true;
            }

        delete [] oldDropTTLFlag;
        }


    // process a DROP_CHAIN flag
    char *pointerToDropChainFlag =
        strstr( flags, "DROP_CHAIN" );

    if( pointerToDropChainFlag != NULL ) {
        // message has drop chain flag

        // ignore UCs on all DROP tail messages
        *outIgnoreUC = true;
        
        if( muteShouldDropTailChainMessages() ) {
            *outDropMessage = true;

            AppLog::detail(
                inLoggerName,
                "Breaking the DROP_CHAIN." );
            }
        }
    
    
    return flags;
    }



