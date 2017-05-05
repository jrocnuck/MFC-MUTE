extern void JROCDebugString( char * pszString );
/*
 * Modification History
 *
 * 2003-June-27   Jason Rohrer
 * Created.
 *
 * 2003-August-7   Jason Rohrer
 * Added a duplicate message detector.
 *
 * 2003-August-8   Jason Rohrer
 * Switched log to file output.
 * Added functions for getting and setting target connection count.
 *
 * 2003-August-11   Jason Rohrer
 * Added stopping log messages.
 *
 * 2003-August-12   Jason Rohrer
 * Changed to tell duplicate detector about our locally generated messages.
 *
 * 2003-August-13   Jason Rohrer
 * Added use of unique node name to prevent self and multiple connections.
 *
 * 2003-August-14   Jason Rohrer
 * Made unique name function publicly available.
 * Added a function for getting connected host list.
 *
 * 2003-August-24   Jason Rohrer
 * Added generation of node RSA keys.
 * Added functions for seeding the random number generator.
 *
 * 2003-August-25   Jason Rohrer
 * Added support for registering message handler functions.
 * Replaced local info hash with secure random string for unique names.
 *
 * 2003-October-9   Jason Rohrer
 * Added support for message limiters.
 *
 * 2003-October-12   Jason Rohrer
 * Switched to a floating point limit.
 * Added use of message ID tracker.
 *
 * 2003-November-2   Jason Rohrer
 * Switched inbound limit setting function to a floating point limit.
 *
 * 2003-November-24   Jason Rohrer
 * Added support for flags.
 *
 * 2003-December-5   Jason Rohrer
 * Added support for message utility.
 *
 * 2004-January-11   Jason Rohrer
 * Made include paths explicit to help certain compilers.
 *
 * 2004-February-4   Jason Rohrer
 * Added utility functions for modifying flag strings.
 *
 * 2004-February-20   Jason Rohrer
 * Added function for getting info about current connection attempt.
 *
 * 2004-March-1   Jason Rohrer
 * Added function for generating message IDs.
 *
 * 2004-March-9   Jason Rohrer
 * Added support for new FORWARD scheme.
 *
 * 2004-March-15   Jason Rohrer
 * Added forward hash code that was originally in ChannelReceivingThread.cpp.
 * Added code to ensure that forward hash seed is forwardable.
 *
 * 2004-March-19   Jason Rohrer
 * Added support for DROP tails.
 *
 * 2004-March-23   Jason Rohrer
 * Added a maximum connection count.
 *
 * 2004-December-12   Jason Rohrer
 * Added a setting for the log rollover time.
 *
 * 2004-December-24   Jason Rohrer
 * Added a function for getting the connection count.
 *
 * 2005-April-15   Jason Rohrer
 * Changed to use drop trees instead of drop chains.
 */



#include "MUTE/layers/messageRouting/messageRouter.h"


#include "MUTE/layers/messageRouting/ConnectionMaintainer.h"
#include "MUTE/layers/messageRouting/OutboundChannelManager.h"
#include "MUTE/layers/messageRouting/LocalAddressReceiver.h"
#include "MUTE/layers/messageRouting/ServerThread.h"
#include "MUTE/layers/messageRouting/MessageIDTracker.h"

#include "MUTE/common/CryptoUtils.h"


#include "minorGems/network/p2pParts/HostCatcher.h"
#include "minorGems/network/p2pParts/MessagePerSecondLimiter.h"

#include "minorGems/network/HostAddress.h"
#include "minorGems/util/random/StdRandomSource.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SettingsManager.h"
#include "minorGems/util/SimpleVector.h"
#include "minorGems/crypto/hashes/sha1.h"

#include "minorGems/util/log/AppLog.h"
#include "minorGems/util/log/FileLog.h"


#include <stdio.h>

#include "debugout.h"

// static data elements
OutboundChannelManager *muteOutboundChannelManager = NULL;
LocalAddressReceiver *muteLocalAddressReceiver = NULL;
ConnectionMaintainer *muteConnectionMaintainer = NULL; 
HostCatcher *muteHostCatcher = NULL;
MessageIDTracker *muteMessageIDTracker = NULL;
MessagePerSecondLimiter *muteOutboundMessagePerSecondLimiter = NULL;
MessagePerSecondLimiter *muteInboundMessagePerSecondLimiter = NULL;

char *muteNodeUniqueName = NULL;
unsigned int mutePort = 0;
ServerThread *muteServerThread = NULL;
StdRandomSource *muteRandomSource = NULL;

char *muteForwardHashSeed = NULL;
double muteContinueForwardProbability = 0.5;
int muteNumNeighborsToSendDropTailsTo = 0;
int muteDropTailTreeStartingTTL = 2;


void muteSeedRandomGenerator( char *inSeedString ) {
    CryptoUtils::seedRandomGenerator( inSeedString );
    }



char *muteGetRandomGeneratorState() {
    return CryptoUtils::getRandomGeneratorState();
    }



void muteStart( unsigned int inPort ) {
    char logRolloverTimeFound;
    int logRolloverTime =
        SettingsManager::getIntSetting( "logRolloverInSeconds",
                                        &logRolloverTimeFound );

    if( !logRolloverTimeFound || logRolloverTime <= 0 ) {
        // defaults to one hour
        logRolloverTime = 3600;
        }
    AppLog::setLog( new FileLog( "MUTE.log",
                                 (unsigned long)logRolloverTime ) );

    AppLog::setLoggingLevel( Log::TRACE_LEVEL );
    
    char *loggerName = "messageRouter";


    // load RSA keys
    char *nodePublicKey =
        SettingsManager::getStringSetting( "nodePublicKey" );
    char *nodePrivateKey =
        SettingsManager::getStringSetting( "nodePrivateKey" );

    if( nodePublicKey == NULL || strcmp( nodePublicKey, "" ) == 0 ||
        nodePrivateKey == NULL || strcmp( nodePrivateKey, "" ) == 0 ) {

        if( nodePublicKey != NULL ) {
            delete [] nodePublicKey;
            }
        if( nodePrivateKey != NULL ) {
            delete [] nodePrivateKey;
            }

        char keyLengthFound;
        int keyLength = SettingsManager::getIntSetting( "nodeKeySize",
                                                        &keyLengthFound );

        if( !keyLengthFound ) {
            // defaults to 1024
            keyLength = 1024;
            }

        printf( "Generating %d-bit RSA key pair for node connections...\n",
                keyLength );
        
        CryptoUtils::generateRSAKey( keyLength, &nodePublicKey,
                                     &nodePrivateKey );

        printf( "...done\n" );

        SettingsManager::setSetting( "nodePublicKey", nodePublicKey );
        SettingsManager::setSetting( "nodePrivateKey", nodePrivateKey );
        }

    delete [] nodePublicKey;
    delete [] nodePrivateKey;
    
    muteRandomSource = new StdRandomSource();

    muteNodeUniqueName = muteGetUniqueName();


    // read our setting for the probability of continuing to forward a message
    char found;
    float continueForwardProbabilitySetting =
        SettingsManager::getFloatSetting( "continueForwardProbability",
                                          &found );  

    if( found ) {
        muteContinueForwardProbability = continueForwardProbabilitySetting;
        }
    else {
        // default to 50%
        muteContinueForwardProbability = 0.5;
        }

    // make sure our forward probability is positive
    if( muteContinueForwardProbability <= 0 ) {
        // default to 50% if not
        muteContinueForwardProbability = 0.5;
        }


    // we need to decide how many neighbors we will send DROP_CHAIN
    // messages on to.  We will use the following probability distribution,
    // where p(n) is the probability that we send DROP_CHAINs to n neighbors:
    //
    // p(0) = 3/4
    // p(i) = 1 / ( 2^(i+2) )
    //
    // Thus,
    // p(1) = 1/8
    // p(2) = 1/16
    // p(3) = 1/32
    // p(4) = 1/64
    //     ...
    //
    // We can achieve this distribution by first flipping an unfair coin
    // to decide whether we send to 0 or not (3/4 chance of sending to 0, and
    // 1/4 chance of sending to more).  If we decide to send to more than zero,
    // we can flip a series of fair coins until we flip "false" to decide
    // how many neighbors we send to.  Thus, for our series of fair flips, we
    // have:
    // false                => send to 1
    // true false           => send to 2
    // true true false      => send to 3
    // true true true false => send to 3
    // Note that the chance of choosing 2 is half the chance of choosing 1,
    // and the chance of choosing i+1 is half the chance of choosing i, for
    // all i >= 1, which is what we want.

    // a test program that demonstrates this technique can be found in
    // test/dropTreeProbabilityTest.cpp
    
    // with this formulation, our expected number of neighbors is 1/2
    double expectedDropRate = 0.5;

    
    // first, flip an unfair coin
    if( muteRandomSource->getRandomFloat() <= 0.75f ) {
        muteNumNeighborsToSendDropTailsTo = 0;
        }
    else {
        // flip a series of fair coins until we flip "false"

        // increment our neighbor count each time we flip true
        muteNumNeighborsToSendDropTailsTo = 1;

        while( muteRandomSource->getRandomFloat() <= 0.5f ) {
            muteNumNeighborsToSendDropTailsTo++;
            }
        }

    char *logMessage =
        autoSprintf( "Will send DROP_CHAIN messages on to %d neighbors.",
                     muteNumNeighborsToSendDropTailsTo );
    AppLog::info( loggerName, logMessage );
    delete [] logMessage;
    // pick a TTL that matches the expected length of our drop chain
    muteDropTailTreeStartingTTL = (int)( 1.0 / expectedDropRate );

    
    
    // keep picking hash seeds until we find one that we would
    // forward according to our forward probability
    muteForwardHashSeed = CryptoUtils::getRandomHexString( 20 );

    // re-hash our seed to see if it produces a new hash that would
    // be forwarded
    char *newHashSeed = muteComputeNewForwardHash( muteForwardHashSeed );

    // while new hash would not be forwarded
    while( newHashSeed == NULL ) {
        char *logMessage = autoSprintf( "Picked %s as forward hash seed, "
                                        "but it does not re-hash to a "
                                        "forwardable hash, so picking again.",
                                        muteForwardHashSeed );
        AppLog::info( loggerName, logMessage );
        delete [] logMessage;

        
        delete [] muteForwardHashSeed;
        muteForwardHashSeed = CryptoUtils::getRandomHexString( 20 );

        newHashSeed = muteComputeNewForwardHash( muteForwardHashSeed );
        }

    // we have found a new hash seed that would be forwarded

    // use this new hash seed as our hash seed
    delete [] muteForwardHashSeed;
    muteForwardHashSeed = newHashSeed;

    logMessage = autoSprintf( "Chose a forward hash seed: %s",
                              muteForwardHashSeed );

    AppLog::info( loggerName, logMessage );
    delete [] logMessage;
    
        
    muteOutboundChannelManager =
        new OutboundChannelManager( muteRandomSource );

    muteLocalAddressReceiver = new LocalAddressReceiver();

    muteHostCatcher = new HostCatcher( 50 );

    muteMessageIDTracker = new MessageIDTracker( 100 );


    double outboundLimit =
        SettingsManager::getFloatSetting( "outboundMessageLimit",
                                          &found );
    if( !found ) {
        outboundLimit = -1;
        }
    muteOutboundMessagePerSecondLimiter =
        new MessagePerSecondLimiter( outboundLimit );

    
    double inboundLimit =
        SettingsManager::getFloatSetting( "inboundMessageLimit",
                                          &found );
    if( !found ) {
        inboundLimit = -1;
        }
    muteInboundMessagePerSecondLimiter =
        new MessagePerSecondLimiter( inboundLimit );

    
    

    // load any seed hosts

    SimpleVector<char *> *seedHostVector =
        SettingsManager::getSetting( "seedHosts" );

    int vectorSize = seedHostVector->size(); 
    
    if( vectorSize % 2 != 0 ) {
        printf( "seedHosts settings file not properly formatted\n" );
        
        for( int i=0; i<vectorSize; i++ ) {
            delete [] ( *( seedHostVector->getElement( i ) ) );
            }
        }
    else {
    
        int numSeedHosts = vectorSize / 2;
        for( int i=0; i<numSeedHosts; i++ ) {
        
            char *addressString = *( seedHostVector->getElement( 2 * i ) );
            char *portString = *( seedHostVector->getElement( 2 * i + 1 ) );
        
            int port;
            int numRead = sscanf( portString, "%d", &port ); 
        
            if( numRead == 1 ) {
                HostAddress *host =
                    new HostAddress( stringDuplicate( addressString ),
                                     port );

                muteHostCatcher->addHost( host );
                
                delete host;
                }
            else {
                printf( "seedHosts settings file not properly formatted\n" );
                }

            delete [] addressString;
            delete [] portString;
            }
        }
    
    delete seedHostVector;


    mutePort = inPort;
    
    muteConnectionMaintainer = new ConnectionMaintainer(
        mutePort,
        muteNodeUniqueName,
        muteLocalAddressReceiver,
        muteOutboundChannelManager,
        muteHostCatcher,
        muteMessageIDTracker,
        muteOutboundMessagePerSecondLimiter,
        muteInboundMessagePerSecondLimiter );
    
    muteServerThread = new ServerThread( mutePort, muteConnectionMaintainer );	
    }



void muteStop() {

    char *loggerName = "messageRouter";
    
    if( muteServerThread != NULL ) {
        AppLog::info( loggerName, "Destroying ServerThread." );
		TRACE( "\tmuteStop:Destroying ServerThread.\n" );
        delete muteServerThread;
        muteServerThread = NULL;
        }
    if( muteConnectionMaintainer != NULL ) {
        AppLog::info( loggerName, "Destroying Connectionmaintainer." );
		TRACE("\tmuteStop: Destroying Connectionmaintainer.\n" );
        delete muteConnectionMaintainer;
        muteConnectionMaintainer = NULL;
        }
    if( muteHostCatcher != NULL ) {
        AppLog::info( loggerName, "Destroying HostCatcher." );
		TRACE("\tmuteStop: Destroying HostCatcher.\n" );
        delete muteHostCatcher;
        muteHostCatcher = NULL;
        }
    if( muteMessageIDTracker != NULL ) {
        AppLog::info( loggerName, "Destroying MessageIDTracker." );
		TRACE("\tmuteStop: Destroying MessageIDTracker.\n" );
        delete muteMessageIDTracker;
        muteMessageIDTracker = NULL;
        }
    if( muteOutboundMessagePerSecondLimiter != NULL ) {
        AppLog::info( loggerName,
                      "Destroying outbound MessagePerSecondLimiter." );
		TRACE("\tmuteStop: Destroying outbound MessagePerSecondLimiter.\n" );
        delete muteOutboundMessagePerSecondLimiter;
        muteOutboundMessagePerSecondLimiter = NULL;
        }
    if( muteInboundMessagePerSecondLimiter != NULL ) {
        AppLog::info( loggerName,
                      "Destroying inbound MessagePerSecondLimiter." );
		TRACE("\tmuteStop: Destroying inbound MessagePerSecondLimiter.\n" );
        delete muteInboundMessagePerSecondLimiter;
        muteInboundMessagePerSecondLimiter = NULL;
        }
    if( muteLocalAddressReceiver != NULL ) {
        AppLog::info( loggerName, "Destroying LocalAddressReceiver." );
		TRACE("\tmuteStop: Destroying LocalAddressReceiver.\n" );
        delete muteLocalAddressReceiver;
        muteLocalAddressReceiver = NULL;
        }
    if( muteOutboundChannelManager != NULL ) {
        AppLog::info( loggerName, "Destroying OutboundChannelManager." );
		TRACE("\tmuteStop: Destroying OutboundChannelManager.\n" );
        delete muteOutboundChannelManager;
        muteOutboundChannelManager = NULL;
        }
    if( muteNodeUniqueName != NULL ) {
        AppLog::info( loggerName, "Destroying node's unique name." );
		TRACE("\tmuteStop: Destroying node's unique name.\n" );
        delete [] muteNodeUniqueName;
        muteNodeUniqueName = NULL;
        }
    if( muteForwardHashSeed != NULL ) {
        AppLog::info( loggerName, "Destroying node's forward hash seed." );
		TRACE("\tmuteStop: Destroying node's forward hash seed.\n" );
        delete [] muteForwardHashSeed;
        muteForwardHashSeed = NULL;
        }
    if( muteRandomSource != NULL ) {
        AppLog::info( loggerName, "Destroying RandomSource." );
		TRACE("\tmuteStop: Destroying RandomSource.\n" );
        delete muteRandomSource;
        muteRandomSource = NULL;
        }
    }



void muteAddHost( char *inAddress, unsigned int inPort ) {
    HostAddress *address = new HostAddress( stringDuplicate( inAddress ),
                                            inPort );
    muteHostCatcher->addHost( address );

    delete address;
    }



void muteSetTargetNumberOfConnections( int inTarget ) {
    muteConnectionMaintainer->setTargetConnectionCount( inTarget );
    }



int muteGetTargetNumberOfConnections() {
    return muteConnectionMaintainer->getTargetConnectionCount();
    }



void muteSetMaxNumberOfConnections( int inMax ) {
    muteConnectionMaintainer->setMaxConnectionCount( inMax );
    }



int muteGetMaxNumberOfConnections() {
    return muteConnectionMaintainer->getMaxConnectionCount();
    }



int muteGetConnectedHostList( char ***outHostAddresses,
                              int **outHostPorts,
                              int **outSentMessageCounts,
                              int **outQueuedMessageCounts,
                              int **outDroppedMessageCounts,
                              unsigned long **outStartTimes ) {

    SimpleVector<HostAddress *> *addressList =
        muteOutboundChannelManager->getConnectedHosts(
            outSentMessageCounts, outQueuedMessageCounts,
            outDroppedMessageCounts );

    int count = addressList->size();

    char **addresses = new char*[ count ];
    int *ports = new int[ count ];
	unsigned long *times = new unsigned long[ count ];

    for( int i=0; i<count; i++ ) {
        HostAddress *host = *( addressList->getElement( i ) );

        addresses[i] = stringDuplicate( host->mAddressString );
        ports[i] = host->mPort;
		times[i] = host->mStartTime;
        delete host;
        }

    *outHostAddresses = addresses;
    *outHostPorts = ports;
	*outStartTimes = times;

    delete addressList;
    
    return count;
    }



int muteGetConnectionCount() {
    return muteOutboundChannelManager->getConnectionCount();
    }



char muteGetCurrentConnectionAttempt( char **outHostAddress, int *outPort ) {
    return
        muteConnectionMaintainer->getCurrentConnectionAttempt( outHostAddress,
                                                               outPort );
    }



void muteSetOutboundMessagePerSecondLimit( double inLimitPerSecond ) {
    SettingsManager::setSetting( "outboundMessageLimit",
                                 (float)inLimitPerSecond );
    muteOutboundMessagePerSecondLimiter->setLimit( inLimitPerSecond );
    }



double muteGetOutboundMessagePerSecondLimit() {
    return muteOutboundMessagePerSecondLimiter->getLimit();
    }



void muteSetInboundMessagePerSecondLimit( double inLimitPerSecond ) {
    SettingsManager::setSetting( "inboundMessageLimit",
                                 (float)inLimitPerSecond );
    muteInboundMessagePerSecondLimiter->setLimit( inLimitPerSecond );
    }



double muteGetInboundMessagePerSecondLimit() {
    return muteInboundMessagePerSecondLimiter->getLimit();
    }



void muteAddReceiveAddress( char *inAddress ) {
    muteLocalAddressReceiver->addReceiveAddress( inAddress );
    }



void muteRemoveReceiveAddress( char *inAddress ) {
    muteLocalAddressReceiver->removeReceiveAddress( inAddress );
    }



void muteSendMessage( char *inFromAddress,
                      char *inToAddress,
                      char *inMessage,
                      char *inFlags ) {

    // generate an ID for this outbound message
    char *messageID = muteGetFreshMessageID();

    //printf( "Registering outbound ID %s\n", messageID );
    // tell the ID tracker about our message so that we will drop
    // it if it gets routed back through us
    muteMessageIDTracker->checkIfIDFresh( messageID );
    
    char *flags;
    if( inFlags != NULL ) {
        flags = inFlags;
        }
    else {
        flags = "NONE";
        }

    
    if( strstr( flags, "FRESH_ROUTE" ) ) {
        // clear the routing information in the outbound direction
        muteOutboundChannelManager->
            clearRoutingInformation( inToAddress );

        // we don't need to clear routes for inFromAddress, since it is
        // our address and we shouldn't have any routing information for it
        }

    
    // send the message
    // a fresh message has 0 utility so far, but the channel manager
    // will add chaff to protect our identity
    muteOutboundChannelManager->routeMessage( messageID,
                                              inFromAddress,
                                              inToAddress,
                                              flags,
                                              0,  // utility
                                              inMessage );

    delete [] messageID;
    }



int muteAddMessageHandler(
    int (*inHandlerFunction)( char *, char *, char *, void * ),
    void *inExtraHandlerArgument ) {
    return muteLocalAddressReceiver->addMessageHandler(
        inHandlerFunction, inExtraHandlerArgument );
    }



void muteRemoveMessageHandler( int inHandlerID ) {
    muteLocalAddressReceiver->removeMessageHandler( inHandlerID );
    }



unsigned int muteGetWaitingMessageCount( char *inAddress ) {
    return muteLocalAddressReceiver->getWaitingMessageCount( inAddress );
    }



unsigned int muteGetReceivedMessages( char *inAddress,
                                      unsigned int inNumMessages,
                                      char ***outMessages,
                                      char ***outFromAddresses ) {
    return muteLocalAddressReceiver->getReceivedMessages(
        inAddress, inNumMessages, outMessages, outFromAddresses );
    }



char *muteGetUniqueName() {
    return CryptoUtils::getRandomHexString( 20 );
    }



char *muteGetFreshMessageID() {
    // first, generate a unique name
    char *uniqueName = muteGetUniqueName();

    unsigned int counter = muteMessageIDTracker->getFreshCounter();

    char *messageID = autoSprintf( "%s_%u", uniqueName, counter );
    delete [] uniqueName;
    
    return messageID;
    }



char *muteAddFlag( char *inFlags, char *inFlagToAdd ) {

    if( strcmp( inFlags, "NONE" ) == 0 ) {
        // new string only contains inFlagToAdd
        return stringDuplicate( inFlagToAdd );
        }
    else {
        // inFlags already contains flags
        
        // add inFlagToAdd to the end
        return autoSprintf( "%s|%s", inFlags, inFlagToAdd );
        }
    }



char *muteRemoveFlag( char *inFlags, char *inFlagToRemove ) {

    if( strstr( inFlags, inFlagToRemove ) == NULL ) {
        // inFlagToRemove does not exist in inFlags

        // no change to inFlags
        return stringDuplicate( inFlags );
        }
    else if( strcmp( inFlags, inFlagToRemove ) == 0 ) {
        // inFlags contains only inFlagToRemove

        // return the NONE flag string
        return stringDuplicate( "NONE" );
        }
    else {
        // inFlags contains multiple flags, including inFlagToRemove
        
        int numOldFlags;
        char **oldFlags = split( inFlags, "|", &numOldFlags );

        SimpleVector<char*> *newFlagVector = new SimpleVector<char*>();

        int i;
        for( i=0; i<numOldFlags; i++ ) {
            if( strcmp( oldFlags[i], inFlagToRemove ) != 0 ) {
                // not inFlagToRemove, add it to vector
                newFlagVector->push_back( oldFlags[i] );
                }
            else {
                // matches inFlagToRemove, delete it
                delete [] oldFlags[i];
                }
            }
        delete [] oldFlags;

        char **newFlags = newFlagVector->getElementArray();

        int numNewFlags = newFlagVector->size();
        delete newFlagVector;

        char *newFlagString = join( newFlags, numNewFlags, "|" );

        for( i=0; i<numNewFlags; i++ ) {
            delete [] newFlags[i];
            }
        delete [] newFlags;

        
        return newFlagString;
        }
    }



char *muteGetForwardHashSeed() {
    return stringDuplicate( muteForwardHashSeed );
    }



char *muteComputeNewForwardHash( char *inOldHash ) {

    // re-hash the hash to produce a new hex-encoded hash
    char *newHash =
        computeSHA1Digest( inOldHash );

    // compute the raw hash too so that we can easily extract the last byte
    unsigned char *newRawHash =
        computeRawSHA1Digest( inOldHash );

    // look at the last byte of the new hash
    // as a random number
    // (hash has length 20)
    unsigned char randomValue = newRawHash[ 19 ];


    delete [] newRawHash;
    
    
    // convert random value to a floating point
    // value in the range 0..1
    double randomFractionValue =
        (double)randomValue / 255.0;

                            
    if( randomFractionValue <=
        muteContinueForwardProbability ) {

        // keep forwarding, replacing the old hash with the new one
        return newHash;
        }
    else {
        // stop forwarding
        delete [] newHash;

        return NULL;
        }    
    }



char muteShouldDropTailChainMessages() {
    if( muteNumNeighborsToSendDropTailsTo == 0 ) {
        return true;
        }
    else {
        return false;
        }
    }
int muteGetNumNeighborsToSendDropTailsTo() {
    return muteNumNeighborsToSendDropTailsTo;
    }



int muteGetDropTailTreeStartingTTL() {
    return muteDropTailTreeStartingTTL;
    }








