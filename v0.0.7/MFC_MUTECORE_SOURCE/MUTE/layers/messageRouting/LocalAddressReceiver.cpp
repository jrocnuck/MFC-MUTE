/*
 * Modification History
 *
 * 2003-July-20    Jason Rohrer
 * Created.
 *
 * 2003-August-25   Jason Rohrer
 * Added support for registering message handler functions.
 *
 * 2003-October-7   Jason Rohrer
 * Added missing call to unlock.
 * Added separate locks for handlers and receive addresses to fix deadlocks.
 * Fixed bug in address removal function.
 * Removed print statements.
 *
 * 2003-December-5   Jason Rohrer
 * Added support for message utility.
 *
 * 2004-January-11   Jason Rohrer
 * Fixed locking bugs in destructor.
 */



#include "LocalAddressReceiver.h"

#include "minorGems/util/stringUtils.h"


LocalAddressReceiver::LocalAddressReceiver()
    : mHandlerLock( new MutexLock() ),
      mReceiveAddressLock( new MutexLock() ),
      mAddressVector( new SimpleVector<char *>() ),
      mNextFreeHandlerID( 0 ),
      mMessageHandlerVector( new SimpleVector<MessageHandlerWrapper *>() ),
      mMessageQueueVector( new SimpleVector< SimpleVector<char *>* >() ),
      mFromAddressQueueVector( new SimpleVector< SimpleVector<char *>* >() ) {

    }



LocalAddressReceiver::~LocalAddressReceiver() {
    mReceiveAddressLock->lock();

    int numAddresses = mAddressVector->size();

    int i;
    for( i=0; i<numAddresses; i++ ) {
        delete [] *( mAddressVector->getElement( i ) );

        SimpleVector<char *> *currentMessageQueue
            = *( mMessageQueueVector->getElement( i ) );
        SimpleVector<char *> *currentFromAddressQueue
            = *( mFromAddressQueueVector->getElement( i ) );

        int numInQueue = currentMessageQueue->size();
        for( int j=0; j<numInQueue; j++ ) {
            delete [] *( currentMessageQueue->getElement( j ) );
            delete [] *( currentFromAddressQueue->getElement( j ) );
            }
        delete currentMessageQueue;
        delete currentFromAddressQueue;
        }
    delete mAddressVector;
    delete mMessageQueueVector;
    delete mFromAddressQueueVector;

    mReceiveAddressLock->unlock();


    mHandlerLock->lock();
    
    int numHandlers = mMessageHandlerVector->size();

    for( i=0; i<numHandlers; i++ ) 
	{
        MessageHandlerWrapper *wrapper = *( mMessageHandlerVector->getElement( i ) );
        delete wrapper;
    }
    delete mMessageHandlerVector;
    
    mHandlerLock->unlock();

    delete mHandlerLock;
    delete mReceiveAddressLock;
    }



int LocalAddressReceiver::findAddressIndex( char *inAddress ) {
    int numAddresses = mAddressVector->size();

    for( int i=0; i<numAddresses; i++ ) {
        char *otherAddress = *( mAddressVector->getElement( i ) );

        if( strcmp( otherAddress, inAddress ) == 0 ) {
            return i;
            }
        }

    // not found
    return -1;
    }
    


void LocalAddressReceiver::addReceiveAddress( char *inAddress ) {
    mReceiveAddressLock->lock();

    // make sure address not already added
    int index = findAddressIndex( inAddress );
    if( index != -1 ) {        
        mReceiveAddressLock->unlock();
        return;
        }
    
    // add address and a message queue
    mAddressVector->push_back( stringDuplicate( inAddress ) );
    mMessageQueueVector->push_back( new SimpleVector<char *>() );
    mFromAddressQueueVector->push_back( new SimpleVector<char *>() );
    
    mReceiveAddressLock->unlock();
    }



void LocalAddressReceiver::removeReceiveAddress( char *inAddress ) {
    mReceiveAddressLock->lock();

    int index = findAddressIndex( inAddress );

    if( index != -1 ) {
        delete [] *( mAddressVector->getElement( index ) );
        mAddressVector->deleteElement( index );
        
        SimpleVector<char *> *currentMessageQueue
            = *( mMessageQueueVector->getElement( index ) );
        SimpleVector<char *> *currentFromAddressQueue
            = *( mFromAddressQueueVector->getElement( index ) );

        int numInQueue = currentMessageQueue->size();
        for( int j=0; j<numInQueue; j++ ) {
            delete [] *( currentMessageQueue->getElement( j ) );
            delete [] *( currentFromAddressQueue->getElement( j ) );
            }
        delete currentMessageQueue;
        delete currentFromAddressQueue;

        mMessageQueueVector->deleteElement( index );
        mFromAddressQueueVector->deleteElement( index );
        }

    mReceiveAddressLock->unlock();
    }



char LocalAddressReceiver::messageReceived( char *inFromAddress,
                                            char *inToAddress,
                                            char *inBody,
                                            int *outUtilityGenerated ) {
//    mReceiveAddressLock->lock();
    
    int index = findAddressIndex( inToAddress ); 
    if( index != -1 ) {
        // received locally

        // track the utility that our handlers generate for this message
        int utility = 0;
                    
        mHandlerLock->lock();
        int numHandlers = mMessageHandlerVector->size();
        
        if( numHandlers <= 0 ) {
            mHandlerLock->unlock();
            // no handlers, so queue messages
/*            JROC.. I don't understand why we create a couple template vectors, load them up then
			exit this code.. seems wrong to me!

            SimpleVector<char *> *currentMessageQueue
                = *( mMessageQueueVector->getElement( index ) );
            SimpleVector<char *> *currentFromAddressQueue
                = *( mFromAddressQueueVector->getElement( index) );

            currentMessageQueue->push_back( stringDuplicate( inBody ) );
            currentFromAddressQueue->push_back(
                stringDuplicate( inFromAddress ) );
*/
//          mReceiveAddressLock->unlock();
            }
        else {
            // pass message to each handler
            
            // unlock so that handlers can modify receive addresses
//          mReceiveAddressLock->unlock();

            
            for( int i=0; i<numHandlers; i++ ) {
                MessageHandlerWrapper *wrapper =
                    *( mMessageHandlerVector->getElement( i ) );

				// jroc
				/*
                utility +=
                    wrapper->mHandlerFunction(
                        inFromAddress, inToAddress, inBody,
                        wrapper->mExtraHandlerArgument );
						*/
				utility = wrapper->mHandlerFunction( inFromAddress, inToAddress, inBody, wrapper->mExtraHandlerArgument );
				
				// kick out if utility less than 0 // jroc
				if( utility < 0 )
					break;
                }
            mHandlerLock->unlock();
            }

		// jroc
		if( utility < 0 )
			utility = 0;
        // message was received locally
        *outUtilityGenerated = utility;
        return true;
        }
    else {
//        mReceiveAddressLock->unlock();
        *outUtilityGenerated = 0;
        return false;
        }
    }



int LocalAddressReceiver::addMessageHandler(
    int (*inHandlerFunction)( char *, char *, char *, void * ),
    void *inExtraHandlerArgument ) {

	// jroc char szJROCDebug[1024];

    mHandlerLock->lock();

    int id = mNextFreeHandlerID;
    mNextFreeHandlerID++;
		
    MessageHandlerWrapper *wrapper = new MessageHandlerWrapper();
    wrapper->mHandlerID = id;
    wrapper->mHandlerFunction = inHandlerFunction;
    wrapper->mExtraHandlerArgument = inExtraHandlerArgument;

    mMessageHandlerVector->push_back( wrapper );    

    mHandlerLock->unlock();

    return id;
    }



void LocalAddressReceiver::removeMessageHandler( int inHandlerID ) {

	// jroc char szJROCDebug[1024];

    mHandlerLock->lock();

    int numHandlers = mMessageHandlerVector->size();

    char found = false;
    for( int i=0; i<numHandlers && !found; i++ ) 
	{
        MessageHandlerWrapper *wrapper = *( mMessageHandlerVector->getElement( i ) );

        if( wrapper->mHandlerID == inHandlerID ) 
		{			
            mMessageHandlerVector->deleteElement( i );
            delete wrapper;
            found = true;
		}
	}
    
    mHandlerLock->unlock();    
    }


// jroc... don't think MUTE GUI is using this function
unsigned int LocalAddressReceiver::getWaitingMessageCount( char *inAddress ) {
    mReceiveAddressLock->lock();
    
    int index = findAddressIndex( inAddress ); 

    unsigned int returnValue;
    if( index != -1 ) {
        SimpleVector<char *> *currentMessageQueue
            = *( mMessageQueueVector->getElement( index ) );
        returnValue = currentMessageQueue->size();
        }
    else {
        returnValue = 0;
        }
    
    mReceiveAddressLock->unlock();

    return returnValue;    
    }


// jroc... don't think this function is being used by the MUTE GUI... 
unsigned int LocalAddressReceiver::getReceivedMessages(
    char *inAddress,
    unsigned int inNumMessages,
    char ***outMessages,
    char ***outFromAddresses ) {

    mReceiveAddressLock->lock();
    
    int index = findAddressIndex( inAddress ); 

    int returnValue;
    char **messages;
    char **fromAddresses;
    
    if( index != -1 ) {
        SimpleVector<char *> *currentMessageQueue
            = *( mMessageQueueVector->getElement( index ) );
        SimpleVector<char *> *currentFromAddressQueue
            = *( mFromAddressQueueVector->getElement( index) );

        int numToGet = inNumMessages;
        int numAvailable = currentMessageQueue->size();

        if( numToGet > numAvailable ) {
            numToGet = numAvailable;
            }

        messages = new char*[numToGet];
        fromAddresses = new char*[numToGet];

        for( int i=0; i<numToGet; i++ ) {
            messages[i] = *( currentMessageQueue->getElement(0) );
            fromAddresses[i] = *( currentFromAddressQueue->getElement(0) );

            currentMessageQueue->deleteElement( 0 );
            currentFromAddressQueue->deleteElement( 0 );
            }

        returnValue = numToGet;
        }
    else {
        returnValue = 0;

        messages = new char*[0];
        fromAddresses = new char*[0];
        }

    *outMessages = messages;
    *outFromAddresses = fromAddresses;

    mReceiveAddressLock->unlock();

    return returnValue;    
    }

