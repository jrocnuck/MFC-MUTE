//jroc remove later
__int64 g_nBytesSentToAll = 0;
/*
 * Modification History
 *
 * 2003-June-22    Jason Rohrer
 * Copied from konspire2b project and modified.
 *
 * 2003-July-22    Jason Rohrer
 * Added vector of corresponding output streams (one per channel).
 *
 * 2003-July-27    Jason Rohrer
 * Fixed a bug in deleting channel stream when channel breaks.
 * Fixed a memory leak.
 *
 * 2003-August-7    Jason Rohrer
 * Fixed a bug in deleting channels.
 * Fixed a memory leak.
 *
 * 2003-August-12    Jason Rohrer
 * Added support for specifying a first message to send upon channel creation.
 *
 * 2003-August-13    Jason Rohrer
 * Added use of unique names to prevent multiple connectsions to same node.
 *
 * 2003-August-14    Jason Rohrer
 * Added a separate log file for routing history.
 * Added function for registering backrouting information.
 *
 * 2003-August-28    Jason Rohrer
 * Fixed a bug when there are no outbound channels.
 *
 * 2003-October-12   Jason Rohrer
 * Added support for ALL broadcasts.
 *
 * 2003-October-17   Jason Rohrer
 * Fixed to prevent uniform pick routing from returning message to sender.
 *
 * 2003-October-21   Jason Rohrer
 * Replaced uniform pick routing with periodic broadcast routing.
 *
 * 2003-November-18   Jason Rohrer
 * Added runtime setting for routing history log.
 *
 * 2003-November-20   Jason Rohrer
 * Added runtime setting for broadcast probability.
 *
 * 2003-November-24   Jason Rohrer
 * Added support for flags.
 * Added support for clearing routing information.
 * Fixed a thread safety bug in addRoutingInformation().
 * Added logging of flags and route info clearings in routing history.
 *
 * 2003-December-5   Jason Rohrer
 * Added support for message utility.
 *
 * 2003-December-25   Jason Rohrer
 * Changed to avoid potentially blocking DNS lookup with mutex locked.
 *
 * 2003-December-26   Jason Rohrer
 * Added support for majority routing.
 *
 * 2003-December-30   Jason Rohrer
 * Fixed rare infinite loop in routing choice algorithm.
 *
 * 2004-January-11   Jason Rohrer
 * Made include paths explicit to help certain compilers.
 *
 * 2004-January-27   Jason Rohrer
 * Removed utility counter chaff code.
 *
 * 2004-February-4   Jason Rohrer
 * Added support for processing FORWARD-flagged messages.
 *
 * 2004-March-1   Jason Rohrer
 * Changed to avoid FORWARDing messages back through the receiving channel.
 *
 * 2004-March-9   Jason Rohrer
 * Added support for new FORWARD scheme.
 *
 * 2004-March-19   Jason Rohrer
 * Added support for DROP tails.
 *
 * 2004-March-22   Jason Rohrer
 * Fixed a bug in drop chain routing.
 *
 * 2005-April-15   Jason Rohrer
 * Changed to use drop trees instead of drop chains.
 */


#include "MUTE/layers/messageRouting/OutboundChannelManager.h"
#include "minorGems/util/SettingsManager.h"
#include "minorGems/util/log/AppLog.h"

#include "MUTE/layers/messageRouting/messageRouter.h"



OutboundChannelManager::OutboundChannelManager( RandomSource *inRandSource )
    : mRandSource( inRandSource ),
      mNodeUniqueNames( new SimpleVector<char *>() ),
      mChannelVector( new SimpleVector<OutboundChannel *>() ),
      mStreamVector( new SimpleVector<OutputStream *>() ),
      mLock( new MutexLock() ),
      mTableSizeLimit( 900 ),
      mTableEntrySizeLimit( 50 ),
      mUniformProbability( 0.0 ),
      mUseMajorityRouting( false ),
      mRoutingTable( new SimpleVector<RoutingTableEntry *>() ),
      mHistoryOutputFile( NULL ) {

    char found;
    int logRoutingHistory =
        SettingsManager::getIntSetting( "logRoutingHistory",
                                        &found );
    if( found && logRoutingHistory == 1 ) {
        mHistoryOutputFile = fopen( "routingHistory.log", "w" );
        }

    
    float broadcastProbability =
        SettingsManager::getFloatSetting( "broadcastProbability",
                                          &found );
    if(  found ) {
        mUniformProbability = broadcastProbability;
        }


    int useMajorityRoutingFlag =
        SettingsManager::getIntSetting( "useMajorityRouting",
                                        &found );
    if( found && useMajorityRoutingFlag == 1 ) {
        mUseMajorityRouting = true;
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

    }



OutboundChannelManager::~OutboundChannelManager() {
    mLock->lock();

    int i;
    for( i=0; i<mChannelVector->size(); i++ ) {
        delete [] *( mNodeUniqueNames->getElement( i ) );
        
        OutboundChannel *chan = *( mChannelVector->getElement( i ) );
        delete chan;
        OutputStream *stream = *( mStreamVector->getElement( i ) );
        delete stream;
        }
    delete mNodeUniqueNames;
    delete mChannelVector;
    delete mStreamVector;

    for( i=0; i<mRoutingTable->size(); i++ ) {
        RoutingTableEntry *entry = *( mRoutingTable->getElement( i ) );
        delete [] entry->mFromAddress;
        delete entry->mReceivingChannels;
        delete entry;
        }
    delete mRoutingTable;
    
    mLock->unlock();
    
    delete mLock;
    }



char OutboundChannelManager::channelCreated( char *inNodeUniqueName,
                                             OutboundChannel * inChannel,
                                             OutputStream *inOutputStream,
                                             char *inFirstMessage ) {
    mLock->lock();

    // make sure a connection does not already exist to the remote
    // host attached to inChannel
    HostAddress *proposedHost = inChannel->getHost();
    
    char exists = false;
    int numChan = mChannelVector->size();
    for( int i=0; i<numChan && !exists; i++ ) {
        OutboundChannel *chan = *( mChannelVector->getElement( i ) );

        HostAddress *existingHost = chan->getHost();
        char *existingName = *( mNodeUniqueNames->getElement( i ) );

        // check for either address or unique name match
        // if( existingHost->equals( proposedHost ) ||
        //    strcmp( existingName, inNodeUniqueName ) == 0 ) {

        // only check unique name to avoid blocking on DNS lookup
        if( strcmp( existingName, inNodeUniqueName ) == 0 ) {
            exists = true;
            }
        delete existingHost;
        }

    delete proposedHost;

    if( !exists ) {
        if( inFirstMessage != NULL ) {
            inChannel->sendMessage( inFirstMessage );
            }
        mNodeUniqueNames->push_back( stringDuplicate( inNodeUniqueName ) );
        mChannelVector->push_back( inChannel );
        mStreamVector->push_back( inOutputStream );
        }
    
    mLock->unlock();

    // return false if a connection already exists to the remote host
    return !exists;
    }



void OutboundChannelManager::channelBroken( OutboundChannel * inChannel ) {
    mLock->lock();

    char foundChannel = false;
    int foundIndex = -1;

    char *foundUniqueName = NULL;
    
    OutputStream *foundStream = NULL;
    
    int numChannels = mChannelVector->size(); 
    for( int i=0; i<numChannels && !foundChannel; i++ ) {

        OutboundChannel *chan = *( mChannelVector->getElement( i ) );

        if( chan == inChannel ) {
            foundChannel = true;
            foundIndex = i;
            foundStream = *( mStreamVector->getElement( i ) );
            foundUniqueName = *( mNodeUniqueNames->getElement( i ) );

            // remove elements from all vectors
            mChannelVector->deleteElement( i );            
            mStreamVector->deleteElement( i );
            mNodeUniqueNames->deleteElement( i );
            }
        
        }

    // unlock before deleting, since the delete could block for a long
    // time while we wait for the channel's thread to finish it's
    // last write and eventually return
    // We only want to block the thread that's calling channelBroken
    // in this case, and we don't want to block other OutboundChannelManager
    // function calls.
    mLock->unlock();

    
    // make sure we only delete the channel once
    // we delete the first time it is found/removed from mChannelVector
    // even though more channelBroken calls may occur later
    if( foundChannel ) {
        delete inChannel;
        
        // delete the corresponding stream and name.
        delete foundStream;
        delete [] foundUniqueName;
        }
    
    }



void OutboundChannelManager::addRoutingInformation(
    char *inFromAddress,
    OutboundChannel *inReceivingChannel ) {

    mLock->lock();

    // add a new routing table entry
    RoutingTableEntry *entry = NULL;
        
    int tableSize = mRoutingTable->size();

    for( int i=0; i<tableSize && entry == NULL; i++ ) {

        RoutingTableEntry *currentEntry =
            *( mRoutingTable->getElement( i ) );

        if( strcmp( currentEntry->mFromAddress, inFromAddress ) == 0 ) {
            entry = currentEntry;

            // move to end of queue
            mRoutingTable->deleteElement( i );
            mRoutingTable->push_back( entry );
            }
        }

    if( entry == NULL ) {
        // add new entry to table
        entry = new RoutingTableEntry();
        entry->mFromAddress = stringDuplicate( inFromAddress );
        entry->mReceivingChannels = new SimpleVector<OutboundChannel *>();

        mRoutingTable->push_back( entry );

        // remove extra entries
        while( mRoutingTable->size() > mTableSizeLimit ) {
            RoutingTableEntry *oldestEntry =
                *( mRoutingTable->getElement( 0 ) );
                
            delete [] oldestEntry->mFromAddress;
            delete oldestEntry->mReceivingChannels;

            mRoutingTable->deleteElement( 0 );
            }
        }

    // add the receiving channel to this table entry
    entry->mReceivingChannels->push_back( inReceivingChannel );

    if( mHistoryOutputFile != NULL ) {
        HostAddress *address = inReceivingChannel->getHost();
        fprintf( mHistoryOutputFile, "Received from %s on %s:%d\n",
                 inFromAddress,
                 address->mAddressString, address->mPort );
        delete address;
        }

    mLock->unlock();
    }



void OutboundChannelManager::clearRoutingInformation( char *inAddress ) {
    mLock->lock();
    
    char found = false;

    int tableSize = mRoutingTable->size();

    for( int i=0; i<tableSize && !found; i++ ) {

        RoutingTableEntry *currentEntry =
            *( mRoutingTable->getElement( i ) );

        if( strcmp( currentEntry->mFromAddress, inAddress ) == 0 ) {
            
            // remove from queue
            mRoutingTable->deleteElement( i );

            delete [] currentEntry->mFromAddress;
            delete currentEntry->mReceivingChannels;
            delete currentEntry;

            found = true;
            }
        }

    if( found ) {
        if( mHistoryOutputFile != NULL ) {
            fprintf( mHistoryOutputFile, "Cleared route info for %s\n",
                     inAddress );
            }
        }
    
    mLock->unlock();
    }



void OutboundChannelManager::routeMessage(
    char *inUniqueID,
    char *inFromAddress,
    char *inToAddress,
    char *inFlags,
    int inUtilityCounter,
    char *inMessage,
    OutboundChannel *inReceivingChannel ) {

    mLock->lock();

    if( mChannelVector->size() == 0 ) {
        // no outbound channels to route message through
        mLock->unlock();
        return;
        }

    // Don't add chaff, since it is not compatible with lower maxMessageUtility
    // settings (for example, maxMessageUtility of 10).
    // Should think more about
    // add chaff to utility to thwart analysis
    // int utilityChaff = mRandSource->getRandomBoundedInt( -10, 10 );

    //int utilityCounter = inUtilityCounter + utilityChaff;
    int utilityCounter = inUtilityCounter;
    
    char *fullMessage = autoSprintf( "UniqueID: %s\n"
                                     "From: %s\n"
                                     "To:   %s\n"
                                     "Flags: %s\n"
                                     "UtilityCounter: %d\n"
                                     "Length: %d\n"
                                     "Body:%s",
                                     inUniqueID,
                                     inFromAddress,
                                     inToAddress,
                                     inFlags,
                                     utilityCounter,
                                     strlen( inMessage ),
                                     inMessage );

    // Nate's special "no spew" code....
    // We don't spew large file chunks or search results everwhere!
    // Yea, it's stupid but it's all in this one file and
    // if you don't like it you can spend the next 6 months and
    // fix it yourself!
    // We let all the other little messages pass but that should be
    // fixed when everything else is
    // The next thing to do is go through all the calling code
    // and have it tell us what type of message is being sent
    // then we don't have to do this

    char headers[1240];
    char noSpew = false;

    // capture just the headers we need and some junk
    // the problem here is the filepath could be 1024 characters
    // so we need to search through a little more than that
    sprintf(headers, "%.1224s", inMessage);
    if( strstr( headers, "ChunkData:" ) != NULL) 
	{
		noSpew = true; // true if file chunk
		//printf("NOSPEW FILE %s\n", headers);
	}
    else if( strstr( headers, "Results:" ) != NULL)
	{
		noSpew = true; // true if search result
		//printf("NOSPEW RES %s\n", headers);
	}

    // That wasn't so bad now was it?
    if( !noSpew && strcmp( inToAddress, "ALL" ) == 0 ) {
        // broadcast to all

////////
//////// Handle DROP_ CHAIN messages ////////
////////

        if( strstr( inFlags, "DROP_CHAIN" ) != NULL ) {
            // send the unmodified drop chain message to a fixed-sized
            // subset of our neighbors, not including the receiving channel
            int numToSendDropChainTo = muteGetNumNeighborsToSendDropTailsTo();
            
            // make a DROP_TTL message, with the starting TTL value,
            // and send the message to the rest of our neighbors.

            char *dropTTLFlag =
                autoSprintf( "DROP_TTL_%d",
                             muteGetDropTailTreeStartingTTL() );
            char *tempFlags = muteRemoveFlag( inFlags, "DROP_CHAIN" );
            char *newFlags = muteAddFlag( tempFlags, dropTTLFlag );
            delete [] tempFlags;
            delete [] dropTTLFlag;

            char *newMessage = autoSprintf( "UniqueID: %s\n"
                                            "From: %s\n"
                                            "To:   %s\n"
                                            "Flags: %s\n"
                                            "UtilityCounter: %d\n"
                                            "Length: %d\n"
                                            "Body:%s",
                                            inUniqueID,
                                            inFromAddress,
                                            inToAddress,
                                            newFlags,
                                            utilityCounter,
                                            strlen( inMessage ),
                                            inMessage );
		
            // count how many neighbors we have sent the DROP_CHAIN
            // version of our message to so far
            int numSentDropChainTo = 0;
            
            int numChannels = mChannelVector->size();
            for( int i=0; i<numChannels; i++ ) {
                OutboundChannel *channel =
                    *( mChannelVector->getElement( i ) );

                // if inReceivingChannel is NULL, this formula will
                // always be true
                if( channel != inReceivingChannel ) {
                    if( numSentDropChainTo < numToSendDropChainTo ) {
                        // send DROP_CHAIN message to an additional neighbor
                        // neighbor that we fine
                        channel->sendMessage( fullMessage );

						// JROC -- TODO -- REMOVE
						g_nBytesSentToAll += strlen( fullMessage );						
						// JROC -- TODO -- REMOVE

                        numSentDropChainTo ++;

                        if( mHistoryOutputFile != NULL ) {
                            HostAddress *address = channel->getHost();
                            fprintf(
                                mHistoryOutputFile,
                                "%s, From %s to %s, Flags: %s, "
                                "drop tree pick: %s:%d\n",
                                inUniqueID, inFromAddress,
                                inToAddress,
                                inFlags,
                                address->mAddressString,
                                address->mPort );
                            delete address;
                            }
                        }
                    else {
                        // Already sent the DROP_CHAIN message to enough
                        // of our neighbors.
                        // Send the DROP_TTL message to the remaining
                        // neighbors
                        // NOTE:  if numToSendDropChainTo is 0, then
                        // we end up sending a DROP_TTL message to all
                        // neighbors.  We must do this to preserve anonymity
                        // (so attackers cannot tell whether or not we
                        // are sending on DROP_CHAIN messages or not).
                        channel->sendMessage( newMessage );
						// JROC -- TODO -- REMOVE						
						g_nBytesSentToAll += strlen( newMessage );						
						// JROC -- TODO -- REMOVE
                        }
                    }
                }

            if( mHistoryOutputFile != NULL ) {
                fprintf( mHistoryOutputFile,
                         "%s, From %s to %s, Flags: %s, "
                         "broadcast to other neighbors\n",
                         inUniqueID, inFromAddress, inToAddress,
                         newFlags );
                }
            
            delete [] newFlags;
            delete [] newMessage;
            }
////////
//////// Handle send to "ALL" messages ////////
////////
        else {
            // no DROP_CHAIN, send same message to all neighbors
            // (even if DROP_TTL, since the TTL was decremented
            //  before routeMessage was called)
            
            int numChannels = mChannelVector->size();
            for( int i=0; i<numChannels; i++ ) {
                OutboundChannel *channel =
                    *( mChannelVector->getElement( i ) );

                // if inReceivingChannel is NULL, this formula will
                // always be true
                if( channel != inReceivingChannel ) {
                    channel->sendMessage( fullMessage );
						
					// JROC -- TODO -- REMOVE					
					g_nBytesSentToAll += strlen( fullMessage );					
					// JROC -- TODO -- REMOVE
                    }
                }
            
            if( mHistoryOutputFile != NULL ) {
                fprintf( mHistoryOutputFile,
                         "%s, From %s to %s, Flags: %s, "
                         "broadcast to all\n",
                         inUniqueID, inFromAddress, inToAddress,
                         inFlags );
                }
            }
        }
////////
//////// Handle regular routing of messages ////////
////////
    else {
        // route
////////
//////// Find a routing table entry, if any ////////
////////
        
        // we need to look up the to-address to decide how to route it
        RoutingTableEntry *entry = NULL;
        
        int tableSize = mRoutingTable->size();

        for( int i=0; i<tableSize && entry == NULL; i++ ) {

            RoutingTableEntry *currentEntry =
                *( mRoutingTable->getElement( i ) );
        
            if( strcmp( currentEntry->mFromAddress, inToAddress ) == 0 ) {
                entry = currentEntry;
                }
            }

    
        // if we have an entry and it has some routing information in it
        if( entry != NULL && entry->mReceivingChannels->size() != 0 ) {

            // walk through the table entries and get rid of inactives
            int i, channelIndex;
            int numChannelsInEntry =
                   entry->mReceivingChannels->size();

            // do this backwards, delete as you go
            for( i= numChannelsInEntry -1; i >= 0; i-- ) {
                 OutboundChannel *channel =
                     *( entry->mReceivingChannels->getElement( i ) );

                 // make sure channel is still live
                 channelIndex =
                     mChannelVector->getElementIndex( channel );

                 if( channelIndex == -1 ) {
                     entry->mReceivingChannels->deleteElement( i );
                     }
                 }

            numChannelsInEntry =
                entry->mReceivingChannels->size();

			//if( noSpew )
			//{ 
			//	printf("\nNOSPEW FOUND TABLE ENTRY!! mReceivingChannels->size= %d\n", entry->mReceivingChannels->size());
			//}

            // pick a channel, possibly at random until we find a good one

            // make sure we don't loop too long making random choices
            // in certain situations, we can actually loop forever
            int loopCount = 0;
            int maxLoopCount = 20;
            
            int index; 
            char sent = false;
            while( !sent ) {
////////
//////// Send messages that can't be routed any other way ////////
////////

                // with small probability (or by default if we've emptied the
                // queue or looped too many times), broadcast to all
                // this will make our routing algorithm robust in the long
                // term (we can discover new, faster routes as they become
                // available)
                if( numChannelsInEntry == 0 ||
                    mRandSource->getRandomDouble() < mUniformProbability ||
                    loopCount >= maxLoopCount ) {

                  // Nate's no spew code...
                  if( !noSpew ) {
                    int numChannels = mChannelVector->size();
                    for( int i=0; i<numChannels; i++ ) {
                        OutboundChannel *channel =
                            *( mChannelVector->getElement( i ) );

                        // make sure we don't route message back to
                        // the node that sent it to us
                        // if inReceivingChannel is NULL, this formula will
                        // always be true
                        if( channel != inReceivingChannel ) {
                            channel->sendMessage( fullMessage );
                            }
                        }

                    if( mHistoryOutputFile != NULL ) {
                        fprintf( mHistoryOutputFile,
                                 "%s, From %s to %s, Flags: %s, "
                                 "chose broadcast to all\n",
                                 inUniqueID, inFromAddress, inToAddress,
                                 inFlags );
                        }
                      } // if !noSpew (packet will be dropped)
                    sent = true;
                    }
                else {

////////
//////// Use majority routing method (if enabled by setting) ////////
////////

                    // Nate's no spew code...
                    // we always use the best route for large packets
                    if( noSpew || mUseMajorityRouting ) {
                        int numChannels = mChannelVector->size();

                        int i;
                        
                        // count how many messages from this destination
                        // node have come through each channel
                        int *channelRouteCounts = new int[ numChannels ];
                        for( i=0; i<numChannels; i++ ) {
                            channelRouteCounts[i] = 0;
                            }

                        // walk through the table entries

                        for( i=0; i<numChannelsInEntry; i++ ) {
                            OutboundChannel *channel =
                               *( entry->mReceivingChannels->getElement( i ) );

                            // make sure channel is still live
                            int channelIndex =
                                mChannelVector->getElementIndex( channel );

                            if( channelIndex != -1 ) {
                                channelRouteCounts[ channelIndex ] ++;
                                }
                                }
                        // now find the majority, the highest amount of packets
                        // from this address came through this channel
                        int majorityIndex = -1;
                        int majorityRouteCount = 0;
                        for( i=0; i<numChannels; i++ ) {
                            if( channelRouteCounts[i] > majorityRouteCount ) {
                                majorityRouteCount = channelRouteCounts[i];
                                majorityIndex = i;
                                }
                            }

                        if( majorityIndex != -1 ) {
                            // found a majority
                            OutboundChannel *channel =
                              *( mChannelVector->getElement( majorityIndex ) );

                            if( channel != inReceivingChannel ) {
                                channel->sendMessage( fullMessage );

//								if( noSpew )
//								{
//									printf("NOSPEW PACKET SENT!! majorityIndex= %d\n", majorityIndex);
//								}

                                if( mHistoryOutputFile != NULL ) {
                                    HostAddress *address = channel->getHost();
                                    fprintf(
                                        mHistoryOutputFile,
                                        "%s, From %s to %s, Flags: %s, "
                                        "majority backroute pick: %s:%d\n",
                                        inUniqueID, inFromAddress,
                                        inToAddress,
                                        inFlags,
                                        address->mAddressString,
                                        address->mPort );
                                    delete address;
                                    }
                                }
                            else {
                                AppLog::error(
                                    "OutboundChannelManager",
                                    "Majority backroute pick is the same"
                                    " as the receiving channel.  "
                                    "Dropping message." );
                                }

                            // flag as sent even if we didn't actually
                            // send it because channel == inReceivingChannel
                            // (flagging as sent essentially drops the message
                            //  here)
                            sent = true;
                            }
                        
                        // drop large messages here, others can go through
                        // the loop again for random choices etc...
                        if( noSpew ) sent = true;
                        delete [] channelRouteCounts;
                        }
////////
//////// Use probabalistic routing method (if majority routing is disabled) ////////
////////
                    else {
                        // use probabalistic routing choice
                        index = mRandSource->getRandomBoundedInt(
                            0, numChannelsInEntry - 1 );

                        OutboundChannel *channel =
                           *( entry->mReceivingChannels->getElement( index ) );

                            // make sure we don't route message back to
                            // the node that sent it to us
                            // if inReceivingChannel is NULL, this formula will
                            // always be true
                            if( channel != inReceivingChannel ) {
                                channel->sendMessage( fullMessage );

                                if( mHistoryOutputFile != NULL ) {
                                    HostAddress *address = channel->getHost();
                                    fprintf(
                                        mHistoryOutputFile,
                                        "%s, From %s to %s, Flags: %s, "
                                        "probability backroute pick: %s:%d\n",
                                        inUniqueID, inFromAddress,
                                        inToAddress,
                                        inFlags,
                                        address->mAddressString,
                                        address->mPort );
                                    delete address;
                                    }
                            
                                sent = true;
                                }
                            // else pick another
                            }
                            }

                loopCount++;
                }
            }
////////
//////// Handle routing if no routing table entries are found ////////
////////
        else {

          // Nate's no spew code...
          if( !noSpew ) {

            // default to broadcast
            int numChannels = mChannelVector->size();
            for( int i=0; i<numChannels; i++ ) {
                OutboundChannel *channel =
                    *( mChannelVector->getElement( i ) );

                // make sure we don't route message back to
                // the node that sent it to us
                // if inReceivingChannel is NULL, this formula will
                // always be true
                if( channel != inReceivingChannel ) {
                    channel->sendMessage( fullMessage );
                    }
                }

            if( mHistoryOutputFile != NULL ) {
                fprintf( mHistoryOutputFile,
                         "%s, From %s to %s, Flags: %s, no route info, "
                         "broadcast to all\n",
                         inUniqueID, inFromAddress, inToAddress,
                         inFlags );
                }
             } // if !noSpew
        
            }
        }

    delete [] fullMessage;

    if( mHistoryOutputFile != NULL ) {
        fflush( mHistoryOutputFile );
        }
    
    mLock->unlock();
    }



SimpleVector<HostAddress *> *OutboundChannelManager::getConnectedHosts(
    int **outSentMessageCounts,
    int **outQueuedMessageCounts,
    int **outDroppedMessageCounts ) {

    mLock->lock();

    
    int numChan = mChannelVector->size();

    SimpleVector<HostAddress *> *returnVector =
        new SimpleVector<HostAddress *>();
    
    int *sentCounts = new int[ numChan ];
    int *queueCounts = new int[ numChan ];
    int *droppedCounts = new int[ numChan ];
    
    // extract a copy of the host from each channel
    for( int i=0; i<numChan; i++ ) {
        OutboundChannel *chan = *( mChannelVector->getElement( i ) );

        returnVector->push_back( chan->getHost() );

        sentCounts[i] = chan->getSentMessageCount();
        queueCounts[i] = chan->getQueuedMessageCount();
        droppedCounts[i] = chan->getDroppedMessageCount();
        }

    
    mLock->unlock();

    if( outSentMessageCounts != NULL ) {
        *outSentMessageCounts = sentCounts;
        }
    else {
        delete [] sentCounts;
        }
    
    if( outQueuedMessageCounts != NULL ) {
        *outQueuedMessageCounts = queueCounts;
        }
    else {
        delete [] queueCounts;
        }

    if( outDroppedMessageCounts != NULL ) {
        *outDroppedMessageCounts = droppedCounts;
        }
    else {
        delete [] droppedCounts;
        }
    
    return returnVector;
    }



int OutboundChannelManager::getConnectionCount() {
    mLock->lock();

    int count = mChannelVector->size();
    
    mLock->unlock();

    return count;
    }





