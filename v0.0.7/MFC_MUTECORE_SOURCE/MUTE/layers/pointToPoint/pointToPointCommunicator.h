/*
 * Modification History
 *
 * 2003-August-25   Jason Rohrer
 * Created.
 *
 * 2003-August-29   Jason Rohrer
 * Added some support for contact info requests.
 * Added support for processing messages from unknown contacts.
 *
 * 2003-August-31   Jason Rohrer
 * Reversed order that handlers are invoked.
 * Added support for unknown contacts.
 * Added more return values.
 * Improved API layout and comments.
 *
 * 2003-September-8   Jason Rohrer
 * Added support for setting timeouts in API calls.
 *
 * 2003-October-2   Jason Rohrer
 * Removed support for looking up contact info by virtual address.
 */



/**
 * The API for the point-to-point layer of MUTE.
 *
 * All calls are thread-safe.
 *
 * All function parameters must be destroyed by caller.
 * All string parameters must be \0-terminated.
 *
 * Before calling this API, application should call
 * muteSeedRandomGenerator and muteStart
 * from the messageRouting layer.
 *
 * After using this API, application should call
 * muteStop
 * from the messageRouting layer.
 * Application may also optionally call
 * muteGetRandomGeneratorState
 * from the messageRouting layer to save the state of the random generator.
 *
 * @author Jason Rohrer.
 */



#ifndef MUTE_POINT_TO_POINT_API
#define MUTE_POINT_TO_POINT_API



/**
 * Starts the point-to-point layer of this node.
 *
 * Must be called before using the point-to-point API.
 *
 * Should be called after calling muteSeedRandomGenerator and muteStart
 * from the messageRouting layer.
 */
void mutePointToPointStart();



/**
 * Stops the point-to-point layer of this node.
 *
 * Must be called after using the point-to-point API.
 *
 * Should be called before calling muteStop
 * from the messageRouting layer.
 */
void mutePointToPointStop();



/**
 * Generates new local contact information for this node.
 * Contact information is stored in the settings directory and in the main
 * directory in a file called "<inContactName>.mc".  This .mc file can be
 * given to other contacts to establish secure communications with them.
 *
 * Any existing local contact information is overwritten.
 *
 * @param inContactName the human-readible contact name to use.
 * @param inKeyLength the RSA key length in bits to generate.
 *
 * Example: <PRE>
 * muteGenerateLocalContactInformation( "afx", 1024 );
 * </PRE>
 */ 
void muteGenerateLocalContactInformation( char *inContactName,
                                          int inKeyLength );



/**
 * Gets a list of contacts available to this node.  Contact names
 * returned by this function are used as contact handles throughout this
 * API.
 *
 * @param outNumContacts pointer to where the number of contacts should
 *   be returned.
 *
 * @return an array of contact names.
 *   Names and array must be destroyed by caller.
 *
 * Example: <PRE>
 * int numContacts;
 * char **contacts = muteGetContactList( &numContacts );
 * // process contact list here
 * ...
 * // destroy contact list
 * for( int i=0; i<numContacts; i++ ) {
 *     delete [] contacts[i];
 *     }
 * delete [] contacts;
 * </PRE>
 */
char **muteGetContactList( int *outNumContacts );



/**
 * Sends our local contact information to a contact.
 *
 * Useful when trying to establish communications with a contact.
 * If we are not yet known to the contact, our contact info will be added
 * to the contact's list of unknown contacts waiting for approval.
 *
 * @param inContactName the contact to send our information to.
 *
 * @return true if the contact was found and our information was sent,
 *   or false otherwise.
 *
 * Example: <PRE>
 * mutePushLocalContactInfoToContact( "cydonia" );
 * </PRE>
 */
char mutePushLocalContactInfoToContact( char *inContactName );



/**
 * Gets a list of unsolicited contacts pending approval.  Contact names
 * returned by this function are used as contact handles throughout this
 * API.
 *
 * @param outNumContacts pointer to where the number of contacts should
 *   be returned.
 *
 * @return an array of contact names.
 *   Names and array must be destroyed by caller.
 *
 * Example: <PRE>
 * int numContacts;
 * char **contacts = muteGetUnknownContactList( &numContacts );
 * // process contact list here
 * ...
 * // destroy contact list
 * for( int i=0; i<numContacts; i++ ) {
 *     delete [] contacts[i];
 *     }
 * delete [] contacts;
 * </PRE>
 */
char **muteGetUnknownContactList( int *outNumContacts );



/**
 * Approves an unknown contact, making them a full-fledged contact.
 *
 * @param inContactName the name of the unknown contact to approve.
 *   Should be a name returned by muteGetUnknownContactList.
 *
 * @return true if the unknown contact was found and approved,
 *    or false otherwise.
 *
 * Example: <PRE>
 * muteApproveUnknownContact( "cydonia" );
 * </PRE>
 */
char muteApproveUnknownContact( char *inContactName );



/**
 * Sets whether unknown contacts are automatically approved when unsolicited
 * contact information is received.
 *
 * @param inAutoApproveOn set to true to enable auto-approve, or false
 *   to disable it.
 *
 * Example: <PRE>
 * muteSetUnknownContactAutoApprove( true );
 * </PRE>
 */
void muteSetUnknownContactAutoApprove( char inAutoApproveOn );



/**
 * Gets whether unknown contacts are being automatically approved.
 *
 * @return true if auto-approve is on, or false if it is off.
 *
 * Example: <PRE>
 * char autoApproveOn = muteGetUnknownContactAutoApprove();
 * </PRE>
 */
char muteGetUnknownContactAutoApprove();



/**
 * Sends a message to a contact.
 *
 * @param inContactName the name of a contact.
 * @param inMessage the message to send.
 *
 * @return true if the contact was found, or false otherwise.
 *
 * Example: <PRE>
 * char found = muteSendMessageToContact( "cydonia", "hello to cydonia" );
 * </PRE>
 */ 
char muteSendMessageToContact( char *inContactName, char *inMessage );



/**
 * Checks if a contact is online and accessible in the MUTE network.
 *
 * This function blocks until the ping operation is complete.
 *
 * @param inContactName the name of a contact.
 * @param inTimeoutInMilliseconds the time to wait for a response to our
 *   ping.  Defaults to 10 seconds (10,000 milliseconds).
 *
 * @return true if the contact is reachable, or false otherwise.
 *
 * Example 1: <PRE>
 * // use default timeout of 10 seconds
 * char online = mutePingContact( "cydonia" );
 * </PRE>
 *
 * Example 2: <PRE>
 * // give contact 20 seconds to respond
 * char online = mutePingContact( "cydonia", 20000 );
 * </PRE>
 */
char mutePingContact( char *inContactName,
                      int inTimeoutInMilliseconds = 10000 );



/**
 * Registers a handler function to process messages from contacts.
 *
 * When a message is received, it is passed to the handlers in the
 * order that they were registered, newest handlers first, until a
 * handler handles the message (by returning true).  Thus, each
 * message is handled by at most one handler.
 * 
 * All (char *) parameters passed in to handler will be destroyed by
 * the handler's caller and therefore should not be destroyed by the handler.
 *
 * The extra (void *) argument can be used to encapsulate any additional
 * state that should be associated with a particular handler.  For example,
 * it could be a pointer to a C++ object that is "unwrapped" by casting
 * inside the handler function.
 *
 * @param inHandlerFunction a pointer to the handler function.
 *   This function must return (char) and take the following arguments:
 *   (char inContactKnown, char *inFromContact, char *inMessage,
 *    void *inExtraArgument ).
 *   If the contact is not known (inContactKnown set to false), then
 *   inFromContact will be set to the virtual address of the message's sender.
 *   If the contact is known, then inFromContact will be set to the contact
 *   name.
 *   The handler function should return true if it handled the message or
 *   false otherwise.
 * @param inExtraHandlerArgument pointer to an extra argument to be passed
 *   in to the handler function each time it is called.
 *   Must be destroyed by caller after the handler is removed.
 *
 * @return an ID for this handler that can be used to remove it later.
 *
 * Example 1: <PRE>
 * // define a handler function
 * void myHandler( char inContactKnown, char *inFromContact, char *inMessage,
 *                 void *inExtraArgument ) {
 *     // handle message here
 *     ...
 *     }
 *
 * // elsewhere in code, register the handler function
 * // extra argument NULL since myHandler does not use it
 * int handlerID = muteAddContactMessageHandler( myHandler, (void *)NULL );
 * </PRE>
 *
 * Example 2: <PRE>
 * // define a handler class that only handles messages for one contact
 * class MySpecificContactHandler {
 *     public:
 *         MyBodyHandler( char *inContactName )
 *             : mContactName( inContactName ) {
 *             }
 *         char handleMessage( char *inContactName, char *inMessage ) {
 *             if( strcmp( inMessage, inContactName ) == 0 ) {
 *                 // our contact, handle message here
 *                 ...
 *                 return true;
 *                 }
 *             else {
 *                 // not handling for this contact
 *                 return false;
 *                 }
 *             }
 *     private:
 *         char *mContactName;
 *     };
 *
 * // define a handler function
 * char myHandler( char inContactKnown, char *inFromContact, char *inMessage,
 *                 void *inExtraArgument ) {
 *     // we only hanle messages from known contacts
 *     if( !inContactKnown ) {
 *         return false;
 *         }
 *     else {
 *         // unwrap a MySpecificContactHandler class object
 *         // from inExtraArgument
 *         MySpecificContactHandler *handler =
 *             (MySpecificContactHandler *)inExtraArgument;
 *         return handler->handleMessage( inFromContact, inMessage );
 *         }
 *     }
 *
 * // elsewhere in code, register the handler function
 * // extra argument is a newly-constructed MySpecificContactHandler
 * // object for contact cydonia
 * MySpecificContactHandler *extraArgument =
 *     new MySpecificContactHandler( "cydonia" );
 * int handlerID = muteAddContactMessageHandler( myHandler,
 *                                               (void *)extraArgument );
 *
 * // later, after removing the handler with a call to
 * // muteRemoveContactMessageHandler
 * delete extraArgument;
 * </PRE>
 */
int muteAddContactMessageHandler(
    char (*inHandlerFunction)( char, char *, char *, void * ),
    void *inExtraHandlerArgument );



/**
 * Removes a message handler so that it will no longer process
 * contact messages.
 *
 * @param inHandlerID the ID of the handler as returned by
 *   a call to muteAddMessageHandler.
 */
void muteRemoveContactMessageHandler( int inHandlerID );



#endif
