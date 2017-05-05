/*
 * Modification History
 *
 * 2003-June-22    Jason Rohrer
 * Copied from konspire2b project and modified.
 *
 * 2003-July-22    Jason Rohrer
 * Added vector of corresponding output streams (one per channel).
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
 * 2003-October-12   Jason Rohrer
 * Added support for ALL broadcasts.
 *
 * 2003-November-24   Jason Rohrer
 * Added support for flags.
 * Added support for clearing routing information.
 *
 * 2003-December-5   Jason Rohrer
 * Added support for message utility.
 *
 * 2003-December-26   Jason Rohrer
 * Added support for majority routing.
 */



#ifndef OUTBOUND_CHANNEL_MANAGER_INCLUDED
#define OUTBOUND_CHANNEL_MANAGER_INCLUDED



#include "minorGems/network/p2pParts/OutboundChannel.h"

#include "minorGems/util/SimpleVector.h"
#include "minorGems/system/MutexLock.h"
#include "minorGems/util/random/RandomSource.h"
#include "minorGems/io/OutputStream.h"



/**
 * A routing table entry used internally by OutboundChannelManager.
 *
 * @author Jason Rohrer.
 */
class RoutingTableEntry {
    public:

        // The from address for this table entry
        char *mFromAddress;

        // Channels on which messages from mFromAddress have been
        // received.
        // May contain duplicates or pointers to channels that no
        // longer exist.
        SimpleVector<OutboundChannel *> *mReceivingChannels;
    };



/**
 * Manages routing messages through a collection of outbound channels.
 *
 * @author Jason Rohrer
 */
class OutboundChannelManager {



    public:


        
        /**
         * Constructs a manager.
         *
         * @param inRandSource the random number source.
         *   Must be destroyed by caller after this class is destroyed.
         */
        OutboundChannelManager( RandomSource *inRandSource );


        
        
        ~OutboundChannelManager();
        
        

        /**
         * Adds a channel to this manager if no channel exists yet
         * to the remote node attached to inChannel.
         *
         * Thread safe.
         *
         * @param inNodeUniqueName the unique name of the remote node.
         *   Must be destroyed by caller.
         * @param inChannel the channel to add.
         *   Will be destroyed by this class when necessary, unless
         *   adding the channel fails, in which case inChannel must
         *   be destroyed by caller.
         * @param inOutputStream the output stream associated with the channel.
         *   Will be destroyed by this class when necessary, unless
         *   adding the channel fails, in which case inChannel must
         *   be destroyed by caller.
         * @param inFirstMessage the first message to send on the
         *   channel if its creation is accepted, or NULL to specify
         *   no special first message.
         *   Message must be a complete text message that can be pushed
         *   directly onto the channel.
         *   This feature is intended primarily to send Accepted status
         *   to the remote host before messages are routed through this
         *   channel.
         *   Defaults to NULL.
         *   Must be destroyed by caller if non-NULL.
         *
         * @return true if added the channel succeeds, or false if
         *   inChannel is connected to a node for which we already
         *   have an active channel.
         */
        char channelCreated( char *inNodeUniqueName,
                             OutboundChannel * inChannel,
                             OutputStream *inOutputStream,
                             char *inFirstMessage = NULL );



        /**
         * Informs this manager that a channel has broken.
         *
         * Thread safe.
         *
         * @param inChannel the broken channel.
         *   Will be destroyed by this class before this call returns.
         */
        void channelBroken( OutboundChannel * inChannel );




        /**
         * Adds routing information to this manager about a received message.
         *
         * This information is used to build a routing table (for
         * back-routing purposes).  This function should be called
         * for each message that is received by this node, regardless
         * of whether the message is consumed locally or routed onward.
         *
         * @param inFromAddress the virtual address that the message is from.
         *   Must be destroyed by caller.
         * @param inReceivingChannel the channel that the message
         *   was received on.
         *   Will be destroyed by this class, assuming that inReceivingChannel
         *   is managed by this class.
         */
        void addRoutingInformation( char *inFromAddress,
                                    OutboundChannel *inReceivingChannel );


        
        /**
         * Clears the routing information for a particular address.
         *
         * Future messages destined for the address will be sent to all
         * neighbors until new routing information is collected for the
         * address.
         *
         * @param inAddress the address to clear routing information for.
         *   Must be destroyed by caller.
         */         
        void clearRoutingInformation( char *inAddress );


        
        /**
         * Sends routes a message through managed channels.
         *
         * Thread safe.
         *
         * @param inUniqueID the unique ID of this message.
         *   Must be destroyed by caller.
         * @param inFromAddress the virtual address that the message is from.
         *   Must be destroyed by caller.
         * @param inToAddress the virtual address that the message is to, or
         *   "ALL" to route a broadcast.
         *   Must be destroyed by caller.
         * @param inFlags the string of flags to attach to this message.
         *   Must be destroyed by caller.
         * @param inUtilityCounter the utility counter for this message.
         * @param inMessage the message to send.
         *   Must be destroyed by caller.
         * @param inReceivingChannel the channel that this message was
         *   received on, or NULL if this is a locally generated message.
         *   Defaults to NULL.
         */
        void routeMessage( char *inUniqueID,
                           char *inFromAddress,
                           char *inToAddress,
                           char *inFlags,
                           int inUtilityCounter,
                           char *inMessage,
                           OutboundChannel *inReceivingChannel = NULL );

        

        /**
         * Gets the hosts that currently are part of active
         * connections in this channel manager.
         *
         * @param outSentMessageCounts pointer to where an array
         *   of sent message counts, one for each channel, should be
         *   returned, or NULL to not return sent counts.
         *   Defaults to NULL.
         *   Returned array must be destroyed by caller.
         * @param outQueuedMessageCounts pointer to where an array
         *   of queued message counts, one for each channel, should be
         *   returned, or NULL to not return queued counts.
         *   Defaults to NULL.
         *   Returned array must be destroyed by caller.
         * @param outDroppedMessageCounts pointer to where an array
         *   of dropped message counts, one for each channel, should be
         *   returned, or NULL to not return dropped counts.
         *   Defaults to NULL.
         *   Returned array must be destroyed by caller.
         *         
         * @return the connected hosts as a vector.
         *   Must be destroyed by caller.
         */
        SimpleVector<HostAddress *> *getConnectedHosts(
            int **outSentMessageCounts = NULL,
            int **outQueuedMessageCounts = NULL,
            int **outDroppedMessageCounts = NULL );



        /**
         * Gets the number of active connections.
         *
         * Thread safe.
         *
         * @return the number of connections.
         */
        int getConnectionCount();
        

        
        // implements the Thread interface
        void run();


        
    protected:
        RandomSource *mRandSource;

        SimpleVector<char *> *mNodeUniqueNames;
        SimpleVector<OutboundChannel *> *mChannelVector;
        SimpleVector<OutputStream *> *mStreamVector;

        MutexLock *mLock;
        

        int mTableSizeLimit;
        int mTableEntrySizeLimit;
        double mUniformProbability;
		int mOutboundTimeout;

        char mUseMajorityRouting;
        
        
        SimpleVector<RoutingTableEntry *> *mRoutingTable;

        FILE *mHistoryOutputFile;        

		double mMaxDroppedMessageFraction;
    };



#endif
