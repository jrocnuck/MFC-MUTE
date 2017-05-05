/*
 * Modification History
 *
 * 2003-June-22   Jason Rohrer
 * Created.
 *
 * 2003-July-20   Jason Rohrer
 * Added per-address access to received messages.
 *
 * 2003-August-8   Jason Rohrer
 * Added functions for getting and setting target connection count.
 *
 * 2003-August-14   Jason Rohrer
 * Made unique name function publicly available.
 * Added a function for getting connected host list.
 *
 * 2003-August-24   Jason Rohrer
 * Added functions for seeding the random number generator.
 *
 * 2003-August-25   Jason Rohrer
 * Added support for registering message handler functions.
 *
 * 2003-October-9   Jason Rohrer
 * Added support for message limiters.
 *
 * 2003-October-12   Jason Rohrer
 * Switched to a floating point limit.
 * Added use of message ID tracker.
 * Added comment about ALL support broadcast.
 *
 * 2003-October-28   Jason Rohrer
 * Fixed typo in example comment.
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
 *
 * 2004-March-19   Jason Rohrer
 * Added support for DROP tails.
 *
 * 2004-March-23   Jason Rohrer
 * Added a maximum connection count.
 *
 * 2004-December-24   Jason Rohrer
 * Added a function for getting the connection count.
 *
 * 2005-April-15   Jason Rohrer
 * Changed to use drop trees instead of drop chains.
 */



/**
 * The API for the message routing layer of MUTE.
 *
 * All calls are thread-safe.
 *
 * All function parameters must be destroyed by caller.
 * All string parameters must be \0-terminated.
 *
 * @author Jason Rohrer.
 */



#ifndef MUTE_MESSAGE_ROUTER_API
#define MUTE_MESSAGE_ROUTER_API



#include <stdio.h>



/**
 * Seeds the secure random number generator used by this node.
 *
 * Random generator must be seeded before this MUTE node is started.
 *
 * Note:
 * This routing layer uses the static, global random number generator
 * from MUTE/common/CryptoUtils.h.
 * If the generator is seeded directly by code in a higher layer (for
 * example, in the point-to-point layer of an application), it should not
 * be seeded again before starting this layer.
 *
 * @param inSeed the string to seed the generator with.
 *
 * Example: <PRE>
 * muteSeedRandomGenerator( "asd848jkkbm47809sd9fjb772379vmbk932489dsfj" );
 * </PRE>
 */
void muteSeedRandomGenerator( char *inSeedString );



/**
 * Gets the state of the random number generator.
 *
 * The output of this function can be used to securely re-seed the random
 * number generator at the next startup.  Thus, an application that uses
 * this routing API could ask the user for random data once at the first
 * system startup and then automatically re-seed the random generator for
 * each subsequent startup.
 *
 * @return a string representing the state of the random number generator.
 *
 * Example: <PRE>
 * char *randState = muteGetRandomGeneratorState();
 * // process sate here (save to disk, etc.)
 * ...
 * // now destroy the returned string
 * delete [] randState;
 * </PRE>
 */
char *muteGetRandomGeneratorState();



/**
 * Starts this MUTE node.
 *
 * @param inPort the port to listen for connections on.
 */
void muteStart( unsigned int inPort );



/**
 * Stops this MUTE node.
 */
void muteStop();



/**
 * Adds a host to the pool of possible connections that this node might make.
 *
 * @param inAddress the internet address (IP or DNS) of the host.
 * @param inPort the port number that the host is listening on.
 *
 * Example: <PRE>
 * muteAddHost( "128.244.23.10", 8910 );
 * </PRE>
 */
void muteAddHost( char *inAddress, unsigned int inPort );



/**
 * Sets the number of connections that this node should try to maintain.
 *
 * @param inTarget the target number of connections.
 *
 * Example: <PRE>
 * muteSetTargetNumberOfConnections( 5 );
 * </PRE>
 */
void muteSetTargetNumberOfConnections( int inTarget );



/**
 * Gets the number of connections that this node is trying to maintain.
 *
 * @return the target number of connections.
 *
 * Example: <PRE>
 * int target = muteGetTargetNumberOfConnections();
 * </PRE>
 */
int muteGetTargetNumberOfConnections();



/**
 * Sets the maximum number of connections that this node will allow.
 *
 * @param inMax the maximum number of connections.
 *
 * Example: <PRE>
 * muteSetMaxNumberOfConnections( 5 );
 * </PRE>
 */
void muteSetMaxNumberOfConnections( int inMax );



/**
 * Gets the maximum number of connections that this node will allow.
 *
 * @return the maximum number of connections.
 *
 * Example: <PRE>
 * int max = muteGetMaxNumberOfConnections();
 * </PRE>
 */
int muteGetMaxNumberOfConnections();



/**
 * Gets a list of hosts that the local node is directly connected to.
 *
 * @param outHostAddresses pointer to location where an array of host address
 *   strings should be returned.  The array and the strings it contains
 *   must be destroyed by caller.
 * @param outHostPorts pointer to location where an array of host ports should
 *   be returned.  The array must be destroyed by caller.
 * @param outSentMessageCounts pointer to location where an array of sent
 *   outbound message counts should be returned.
 *   The array must be destroyed by caller.
 * @param outQueuedMessageCounts pointer to location where an array of queued
 *   outbound message counts should be returned.
 *   The array must be destroyed by caller.
 * @param outDroppedMessageCounts pointer to location where an array of dropped
 *   outbound message counts should be returned.
 *   The array must be destroyed by caller.
 *
 * @return the number of hosts returned (in other words, the length of
 *   each returned array).
 *
 * Example: <PRE>
 * char **addresses;
 * int *ports;
 * int *sentCounts;
 * int *queueCounts;
 * int *dropCounts;
 * int numHosts = muteGetConnectedHostList( &addresses, &ports, &sentCounts,
 *                                          &queueCounts, &dropCounts );
 * // process addresses here
 * ...
 * // now destroyed returned arrays
 * for( int i=0; i<numHosts; i++ ) {
 *     delete [] addresses[i];
 *     }
 * delete [] addresses;
 * delete [] ports;
 * delete [] dropCounts;
 * delete [] sentCounts;
 * delete [] queueCounts;
 * </PRE>
 */
int muteGetConnectedHostList( char ***outHostAddresses,
                              int **outHostPorts,
                              int **outSentMessageCounts,
                              int **outQueuedMessageCounts,
                              int **outDroppedMessageCounts,
                              unsigned long **outStartTimes );



/**
 * Gets the number of active connections.
 *
 * @return the number of connections.
 */
int muteGetConnectionCount();



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
char muteGetCurrentConnectionAttempt( char **outHostAddress, int *outPort );



/**
 * Sets a limit on the number of outbound messages transmitted per second.
 *
 * @param inLimitPerSecond the maximum number of messages per second, or
 *   -1 to set no limit.
 *   Must be positive or -1 (cannot be 0 or less than -1). 
 */
void muteSetOutboundMessagePerSecondLimit( double inLimitPerSecond );



/**
 * Gets the limit on the number of outbound messages transmitted per second.
 *
 * @return the maximum number of messages per second, or -1 if no limit
 *   is set.
 */
double muteGetOutboundMessagePerSecondLimit();



/**
 * Sets a limit on the number of inbound messages received per second.
 *
 * @param inLimitPerSecond the maximum number of messages per second, or
 *   -1 to set no limit.
 *   Must be positive or -1 (cannot be 0 or less than -1). 
 */
void muteSetInboundMessagePerSecondLimit( double inLimitPerSecond );



/**
 * Gets the limit on the number of inbound messages received per second.
 *
 * @return the maximum number of messages per second, or -1 if no limit
 *   is set.
 */
double muteGetInboundMessagePerSecondLimit();



/**
 * Gets a secure, guarnteed unique name string.
 *
 * Useful as a virtual address.
 *
 * These names are generated in a way that makes them difficult to map
 * back to the node that generated them.  It should also be difficult
 * to prove that a particular node generated a given address.  In other
 * words, these unique names are cryptographically secure.
 *
 * @return a secure unique name.
 *
 * Example: <PRE>
 * char *name = muteGetUniqueName();
 * // use the name here
 * ...
 * // now destroy the name
 * delete [] name;
 * </PRE>
 */ 
char *muteGetUniqueName();



/**
 * Gets a fresh message ID.
 *
 * @return a fresh message ID.
 *
 * Example: <PRE>
 * char *id = muteGetFreshMessageID();
 * // use the ID here
 * ...
 * // now destroy the ID
 * delete [] id;
 */
char *muteGetFreshMessageID();



/**
 * Adds an address that this node will receive on.
 *
 * Note that the address "ALL" must be added in order to receive broadcasts.
 *
 * @param inAddress a virtual address that this node will receive messages on.
 *
 * Example: <PRE>
 * muteAddReceiveAddress( "a482fe12" );
 * </PRE>
 */
void muteAddReceiveAddress( char *inAddress );



/**
 * Removes a receive address, causing this node to stop receiving on the
 * address.
 *
 * @param inAddress a virtual address to stop receiving on.
 *
 * Example: <PRE>
 * muteAddReceiveAddress( "a482fe12" );
 * </PRE>
 */
void muteRemoveReceiveAddress( char *inAddress );




/**
 * Sends a message through the network using virtual node address
 * for routing.
 *
 * @param inFromAddress the virtual address of the sender.
 * @param inToAddress the virtual address of the receiver, or "ALL"
 *   to send a broadcast.
 * @param inMessage the message body.
 * @param inFlags the string of flags to attach to this message, or
 *   NULL to specify no flags (in which case, the NONE flag is attached).
 *   Flags should be separated by the "|" character.
 *   Defaults to NULL.
 *
 * Example A: <PRE>
 * muteSendMessage( "a482fe12", "38bd98ef", "test message" );
 * </PRE>
 *
 * Example B: <PRE>
 * muteSendMessage( "a482fe12", "ALL", "test message" );
 * </PRE>
 *
 * Example C: <PRE>
 * muteSendMessage( "a482fe12", "38bd98ef", "test message", "FRESH_ROUTE" );
 * </PRE>
 */
void muteSendMessage( char *inFromAddress,
                      char *inToAddress,
                      char *inMessage,
                      char *inFlags = NULL );



/**
 * Registers a handler function to process all locally-received messages.
 *
 * All (char *) parameters passed in to handler will be destroyed by caller and
 * therefore should not be destroyed by the handler.
 *
 * The extra (void *) argument can be used to encapsulate any additional
 * state that should be associated with a particular handler.  For example,
 * it could be a pointer to a C++ object that is "unwrapped" by casting
 * inside the handler function.
 *
 * @param inHandlerFunction a pointer to the handler function.
 *   This function must return the utility generated by the handler and
 *   take the following arguments:
 *   (char *inFromAddress, char *inToAddress, char *inBody,
 *    void *inExtraArgument ).
 * @param inExtraHandlerArgument pointer to an extra argument to be passed
 *   in to the handler function each time it is called.
 *   Must be destroyed by caller after the handler is removed.
 *
 * @return an ID for this handler that can be used to remove it later.
 *
 * Example: <PRE>
 * // define a handler function
 * int myHandler( char *inFromAddress, char *inToAddress, char *inBody,
 *                void *inExtraArgument ) {
 *     // handle message here
 *     ...
 *     return 10;   // this handler generates constant utility
 *     }
 *
 * // elsewhere in code, register the handler function
 * // extra argument NULL since myHandler does not use it
 * int handlerID = muteAddMessageHandler( myHandler, (void *)NULL );
 * </PRE>
 *
 * Example 2: <PRE>
 * // define a handler class
 * class MyBodyHandler {
 *     public:
 *         int handleBody( char *inBody ) {
 *             // handle body of message here
 *             ...
 *             return 0;  // this handler generates no utility
 *             }
 *     };
 *
 * // define a handler function
 * int myHandler( char *inFromAddress, char *inToAddress, char *inBody,
 *                 void *inExtraArgument ) {
 *     // unwrap a MyBodyHandler class object from inExtraArgument
 *     MyBodyHandler *handler = (MyBodyHandler *)inExtraArgument;
 *     return handler->handleBody( inBody );
 *     }
 *
 * // elsewhere in code, register the handler function
 * // extra argument is a newly-constructed MyBodyHandler object
 * MyBodyHandler *extraArgument = new MyBodyHandler();
 * int handlerID = muteAddMessageHandler( myHandler, (void *)extraArgument );
 *
 * // later, after removing the handler with a call to muteRemoveMessageHandler
 * delete extraArgument;
 * </PRE>
 */
int muteAddMessageHandler(
    int (*inHandlerFunction)( char *, char *, char *, void * ),
    void *inExtraHandlerArgument );



/**
 * Removes a message handler so that it will no longer process
 * locally-received messages.
 *
 * @param inHandlerID the ID of the handler as returned by
 *   a call to muteAddMessageHandler.
 */
void muteRemoveMessageHandler( int inHandlerID );



/**
 * Gets the number of received messages that are waiting.
 *
 * Note that this function is only useful if no message handlers are
 * registered.  If handlers are registered, they will process all messages
 * and this function will always return 0.
 *
 * @param inAddress the virtual address to check for waiting messages.
 *
 * @return the number of received messages.
 *
 * Example: <PRE>
 * int count = muteGetWaitingMessageCount( "a482fe12" );
 * </PRE>
 */
unsigned int muteGetWaitingMessageCount( char *inAddress );



/**
 * Gets received messages.
 *
 *
 * Note that this function is only useful if no message handlers are
 * registered.  If handlers are registered, they will process all messages
 * and this function will always return 0 messages and empty arrays.
 *
 * @param inAddress the virtual address to get waiting messages for.
 * @param inNumMessages the number of messages to get.
 * @param outMessages pointer to location where an array of
 *   messages should be returned.
 *   Returned array and messages must be destroyed by caller.
 * @param outFromAddresses pointer to location where an array of
 *   from addresses, one per message should be returned.
 *   Returned array and addresses must be destroyed by caller.
 *
 * @return the number of messages being returned (may be less
 *   than inNumMessages if fewer messages are available).
 *
 * Example: <PRE>
 * char **messages;
 * char **fromAddresses;
 * int count = muteGetWaitingMessageCount( "a482fe12", 10,
 *                                         &messages, &fromAddresses );
 * // process messages here
 * // ...
 * // now destroy returned arrays
 * for( int i=0; i<count; i++ ) {
 *     delete [] messages[i];
 *     delete [] fromAddresses[i];
 *     }
 * delete [] messages;
 * delete [] fromAddresses; 
 * </PRE>
 */
unsigned int muteGetReceivedMessages( char *inAddress,
                                      unsigned int inNumMessages,
                                      char ***outMessages,
                                      char ***outFromAddresses );



/**
 * Adds a flag to a flag string.
 *
 * @param inFlags the flag string.
 * @param inFlagToAdd the flag to add.
 *
 * @return the new flag string.
 *
 * Example: <PRE>
 * char *flagString = muteAddFlag( "FRESH_ROUTE|FLAG_A", "FLAG_B" );
 *
 * // flagString now contains "FRESH_ROUTE|FLAG_A|FLAG_B"
 * delete [] flagString;
 * </PRE>
 */
char *muteAddFlag( char *inFlags, char *inFlagToAdd );



/**
 * Removes a flag from a flag string.
 *
 * @param inFlags the flag string.
 * @param inFlagToRemove the flag to remove.
 *
 * @return the new flag string.
 *
 * Example A: <PRE>
 * char *flagString = muteRemoveFlag( "FRESH_ROUTE|FLAG_A", "FLAG_A" );
 *
 * // flagString now contains "FRESH_ROUTE"
 * delete [] flagString;
 * </PRE>
 *
 * Example B: <PRE>
 * char *flagString = muteRemoveFlag( "FRESH_ROUTE", "FRESH_ROUTE" );
 *
 * // flagString now contains "NONE"
 * delete [] flagString;
 * </PRE>
 */
char *muteRemoveFlag( char *inFlags, char *inFlagToRemove );



/**
 * Gets the hash that this node should use for any FORWARD flags
 * on messages that it generates.
 *
 * @return the 20-byte hash seed as a hex-encoded string (40 hex characters).
 */
char *muteGetForwardHashSeed();



/**
 * Gets whether we should continue to forward a message based on the FORWARD
 * hash attached to the message.
 *
 * @param inOldHash the received message's forward hash.
 *
 * @return the newly computed hash, if we should keep forwarding the message,
 *   or NULL if we should stop forwarding the message.
 */
char *muteComputeNewForwardHash( char *inNewHash ); 



/**
 * Gets whether we should drop all tail chain messages.
 *
 * @return true to drop all messages, or false to pass all messages on.
 */
char muteShouldDropTailChainMessages();



/**
 * Gets the number of neighbors to send drop tail messages to.
 * A this function returns 0 when muteShouldDropTailChainMessages() returns
 * true.  However, if muteShouldDropTailChainMessages() returns true, this
 * function provides information about how many neighbors the messages should
 * be sent to.
 *
 * Version 0.4 MUTE behavior was to send messages on to only one neighbor
 * (in a "drop chain"), but this behavior is vulnerable to a 2-neighbor attack.
 * Now tail messages are sent on to a subset of our neighbors (in a "drop
 * tree"), though the DROP_CHAIN flag is being kept for
 * backwards-compatibility.
 *
 * @return the number of neighbors that we should send all DROP_CHAIN messages
 *   to.
 */
int muteGetNumNeighborsToSendDropTailsTo();
/* Gets the starting TTL for drop tail trees.
 *
 * @return the starting TTL.
 */
int muteGetDropTailTreeStartingTTL();



#endif
