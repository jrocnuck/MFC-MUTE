/*
 * Modification History
 *
 * 2003-September-8   Jason Rohrer
 * Created.
 *
 * 2003-September-9   Jason Rohrer
 * Added missing chat functions.  Improved text layout.
 * Added node management functions.
 *
 * 2003-October-9   Jason Rohrer
 * Added support for message limiters.
 */



#include "MUTE/layers/messageRouting/messageRouter.h"

#include "MUTE/layers/pointToPoint/pointToPointCommunicator.h"

#include "MUTE/layers/fileTransfer/fileTransmitter.h"

#include "MUTE/userInterface/fileTransferWebInterface/fileTransferWebServer.h"



#include "minorGems/util/SettingsManager.h"
#include "minorGems/util/SimpleVector.h"
#include "minorGems/util/stringUtils.h"

#include "minorGems/util/printUtils.h"
#include "minorGems/system/Time.h"
#include "minorGems/system/Thread.h"
#include "minorGems/system/MutexLock.h"
#include "minorGems/system/Semaphore.h"

#include <stdio.h>



char *readUserString( char *inPrompt ) {

    int numRead = 0;

    char *buffer = new char[1000];
    
    while( numRead != 1 ) {
        printf( inPrompt );
        fflush( stdout );
        
        numRead = scanf( "%999s", buffer );
        }
    return buffer;
    }



char *readUserLine( char *inPrompt ) {
    printf( inPrompt );
    fflush( stdout );
    
    SimpleVector<char> *readChars = new SimpleVector<char>();


    
    int readInt = getc( stdin );

    // first, skip starting newlines
    while( readInt == '\n' || readInt == '\r' ) {
        readInt = getc( stdin );
        }
    
    while( readInt != '\n' && readInt != '\r' && readInt != EOF ) {
        readChars->push_back( (char)readInt );

        readInt = getc( stdin );
        }

    char *readLine = readChars->getElementString();

    delete readChars;

    return readLine;
    }



int readUserInt( char *inPrompt ) {

    int readInt;
    int numRead = 0;
    
    while( numRead != 1 ) {
        printf( inPrompt );
        fflush( stdout );
        
        numRead = scanf( "%d", &readInt );
        }
    return readInt;
    }



MutexLock *printMessageLock;
MutexLock *recentMessageLock;
Semaphore *messageReadySemaphore;
SimpleVector<char *> *recentContacts;
SimpleVector<char *> *recentMessages;


// the most recent contact that we've received a message from
char *mostRecentContact = NULL;



// callback for incoming chat messages
char chatHandler( char inContactKnown,
                  char *inFromContact, char *inMessage, void *inExtraParam ) {

    // reject chat from unknown contacts
    if( ! inContactKnown ) {
        return false;
        }
    
    char returnValue = false;
    
    SimpleVector<char *> *tokens = tokenizeString( inMessage );

    // second token is message type

    int numTokens = tokens->size();
    if( numTokens > 2 ) {
        char *messageType = *( tokens->getElement( 1 ) );

        if( strcmp( messageType, "Chat" ) == 0 ) {

            char *pointerToBody = strstr( inMessage, "Body:" ); 

            recentMessageLock->lock();

            if( pointerToBody != NULL ) {
                // skip the body tag
                pointerToBody = &( pointerToBody[ strlen( "Body:" ) ] );
            
                recentMessages->push_back(
                    stringDuplicate( pointerToBody ) );
                }
            else {
                // body not found
                recentMessages->push_back(
                    stringDuplicate( "[missing body]" ) );
                }
            recentContacts->push_back( stringDuplicate( inFromContact ) );

            returnValue = true;
            recentMessageLock->unlock();
            messageReadySemaphore->signal();
            }
        }

    for( int i=0; i<numTokens; i++ ) {
        delete [] *( tokens->getElement( i ) );
        }
    delete tokens;

    return returnValue;
    }



class MessagePrintingThread : public Thread {

    public:


        
        MessagePrintingThread()
            : mStopLock( new MutexLock() ), mStopped( false ) {

            }


        
        ~MessagePrintingThread() {
            delete mStopLock;
            }

        
        
        void stop() {
            mStopLock->lock();
            mStopped = true;
            mStopLock->unlock();
            }
        
        // implements the Thread interface
        void run();

    protected:

        char isStopped() {
            mStopLock->lock();
            char stopped = mStopped;
            mStopLock->unlock();
            return stopped;
            }
        
        MutexLock *mStopLock;
        char mStopped;

    };



void MessagePrintingThread::run() {

    while( !isStopped() ) {

        // wait for a message to be ready
        messageReadySemaphore->wait();
        
        // semaphore also signaled when we need to stop
        if( !isStopped() ) {

            recentMessageLock->lock();
            int numMessages = recentMessages->size();

            // print one message, if there is one
            if( numMessages > 0 ) {
        
                char *message = *( recentMessages->getElement( 0 ) );
                char *from = *( recentContacts->getElement( 0 ) );

                if( mostRecentContact != NULL ) {
                    delete [] mostRecentContact;
                    }
                mostRecentContact = stringDuplicate( from );

                
                printMessageLock->lock();
                printf( "\n[ %s ]: %s\n", from, message );
                // print a fresh prompt
                printf( "> " );
                fflush( stdout );

                printMessageLock->unlock();

                delete [] from;
                delete [] message;

                recentMessages->deleteElement( 0 );
                recentContacts->deleteElement( 0 );
                }
            recentMessageLock->unlock();
            }
        }
    }



int main() {

    printf( "textMUTE\n"
            "a text-based user interface for MUTE chat "
            "and file transfer\n\n" );

    printf( "http://mute-net.sourceforge.net\n\n" );
    
    char *randomSeed = SettingsManager::getStringSetting( "randomSeed" );

    if( randomSeed == NULL ) {
        randomSeed = readUserString( "Enter some randomness: " );
        }
    else {
        printf( "Using randomness saved from last time\n" );
        }

    muteSeedRandomGenerator( randomSeed );
    delete [] randomSeed;


    char valueFound;

    int portNumber = SettingsManager::getIntSetting( "port", &valueFound );

    if( ! valueFound ) {

        portNumber = readUserInt( "Enter port number to listen on: " );

        SettingsManager::setSetting( "port", portNumber );
        }


    int webPortNumber =
        SettingsManager::getIntSetting( "webPort", &valueFound );

    if( ! valueFound ) {

        webPortNumber =
            readUserInt( "Enter port number for web connections: " );

        SettingsManager::setSetting( "webPort", webPortNumber );
        }


    char *localName = SettingsManager::getStringSetting( "localContactName" );

    if( localName == NULL ) {
        localName = readUserString( "Enter nickname: " );

        int keyLength =
            readUserInt( "Enter key size [512, 1024, 1536, 2048, 4096]: " );

        printf( "Generating %d-bit RSA key pair...\n", keyLength ); 
        muteGenerateLocalContactInformation( localName, keyLength );

        printf( "...done\n" );
        }
    else {
        printf( "Using keys for local nickname \"%s\"\n", localName ); 
        }

    delete [] localName;

    
    printf( "Listening for MUTE connections on port %d\n", portNumber );
    muteStart( portNumber );
        
    mutePointToPointStart();

    muteFileTransferStart();

    printf( "Listening for web connections on port %d\n", webPortNumber );
    muteFileTransferWebServerStart( webPortNumber );


    printf( "\nTo download files from contacts, point your browser at:\n"
            "    http://localhost:%d\n\n", webPortNumber );
    
    
    recentMessageLock = new MutexLock();
    printMessageLock = new MutexLock();
    messageReadySemaphore = new Semaphore();
    recentContacts = new SimpleVector<char *>();
    recentMessages = new SimpleVector<char *>();

    MessagePrintingThread *printerThread = new MessagePrintingThread();
    printerThread->start();
    
    // no extra parameter for handler (NULL)
    int handlerID = muteAddContactMessageHandler( chatHandler,
                                                  (void *)NULL );
    

    printf( "Enter ? to show a list of available commands\n" );
    
    char quitting = false;
    char *readCharBuffer = new char[2];
    // print the prompt 
    printf( "> " );
    fflush( stdout );

    while( !quitting ) {


        scanf( "%1s", readCharBuffer );

        switch( readCharBuffer[0] ) {
            case '?':
                printf( "Commands:\n" );
                
                printf( "  MUTE node management:\n" );
                printf( "    l -- set message rate limits\n" );
                printf( "    c -- print active connection list\n" );
                printf( "    h -- add host to pool of possible "
                        "connections\n" );

                printf( "  Contact management:\n" );
                printf( "    p -- print contact list (pings each contact)\n" );

                printf( "    i -- push our contact info to a contact\n" );
                printf( "    u -- print list of unknown contacts waiting "
                        "for approval\n" ); 
                printf( "    a -- approve an unknown contact\n" );
                printf( "    t -- toggle unknown contact auto-approve " );

                if( muteGetUnknownContactAutoApprove() ) {
                    printf( "[currently on]\n" );
                    }
                else {
                    printf( "[currently off]\n" );
                    }

                printf( "  Chat:\n" );
                printf( "    m -- send message to a contact\n" );
                                
                printf( "    r -- send message to most recent contact" );

                // stick most recent contact into help message, if one exists
                recentMessageLock->lock();
                if( mostRecentContact != NULL ) {
                    printf( " [%s]\n", mostRecentContact );
                    }
                else {
                    printf( "\n" );
                    }
                recentMessageLock->unlock();


                printf( "  General:\n" );
                printf( "    ? -- print this help message\n" );
                printf( "    q -- quit\n" );
                break;
            case 'l':
            case 'L': {
                int outboundLimit = muteGetOutboundMessagePerSecondLimit();
                int inboundLimit = muteGetInboundMessagePerSecondLimit();

                if( outboundLimit != -1 ) {
                    printf( "Outbound limit:  %d messages/second\n",
                            outboundLimit );
                    }
                else {
                    printf( "Outbound limit:  -1 (unlimited)\n" );
                    }
                if( inboundLimit != -1 ) {
                    printf( "Inbound limit:  %d messages/second\n",
                            inboundLimit );
                    }
                else {
                    printf( "Inbound limit:  -1 (unlimited)\n" );
                    }

                printf( "\n" );
                
                outboundLimit = readUserInt(
                    "Enter outbound limit in messages/second"
                    " (-1 = no limit): " );
                inboundLimit = readUserInt(
                    "Enter inbound limit in messages/second"
                    " (-1 = no limit): " );

                if( outboundLimit < 0 ) {
                    outboundLimit = -1;
                    }
                if( inboundLimit < 0 ) {
                    inboundLimit = -1;
                    }
                
                muteSetOutboundMessagePerSecondLimit( outboundLimit );
                muteSetInboundMessagePerSecondLimit( inboundLimit );
                
                break;
                }
            case 'c':
            case 'C': {
                char **addresses;
                int *ports;
                int *sentCounts;
                int *queuedCounts;
                int *droppedCounts;
                
                int connectionCount =
                    muteGetConnectedHostList( &addresses,
                                              &ports,
                                              &sentCounts,
                                              &queuedCounts,
                                              &droppedCounts );
                if( connectionCount > 0 ) {
                    printf( "Host:                     "
                            "Sent: "
                            "Queued: "
                            "Dropped:\n" );
                    for( int i=0; i<connectionCount; i++ ) {
                        printf( "%15s : %5d %7d %7d  %7d\n",
                                addresses[i], ports[i], sentCounts[i],
                                queuedCounts[i], droppedCounts[i] );
                        delete [] addresses[i];
                        }
                    }
                else {
                    printf( "No connections\n" );
                    }
                delete [] addresses;
                delete [] ports;
                delete [] sentCounts;
                delete [] queuedCounts;
                delete [] droppedCounts;
                break;
                }
            case 'h':
            case 'H': {
                char *address = readUserString( "Enter IP address: " );
                int port = readUserInt( "Enter port number: " );

                muteAddHost( address, port );

                printf( "Host added\n" );
                delete [] address;
                break;
                }
            case 'p':
            case 'P': {
                int contactCount;
                char **contactList = muteGetContactList( &contactCount );
                if( contactCount > 0 ) {
                    printf( "Contacts:\n" );
                    for( int i=0; i<contactCount; i++ ) {
                        // check if online
                        unsigned long seconds;
                        unsigned long ms;
                        Time::getCurrentTime( &seconds, &ms );
                        
                        char online = mutePingContact( contactList[i] );

                        unsigned long deltaMS
                            = Time::getMillisecondsSince( seconds, ms );
                        
                        if( online ) {
                            printf( "[online, ping: %d]  ", (int)deltaMS );
                            }
                        else {
                                printf( "[offline] " );
                            }
                        
                        printf( "%s\n", contactList[i] );
                        delete [] contactList[i];
                            }
                    }
                else {
                    printf(
                        "No contacts.\n"
                        "Add contact (.mc) files to "
                        "your \"contacts\" folder\n" );
                    }
                delete [] contactList;
                break;
                }
            case 'i':
            case 'I': {
                char *contactName = readUserString( "Enter contact name: " );

                char sent = mutePushLocalContactInfoToContact( contactName );

                if( ! sent ) {
                    printf( "Failed to sent our contact info to %s\n",
                            contactName );
                    }
                delete [] contactName;
                break;
                }
            case 'u':
            case 'U': {
                int contactCount;
                char **contactList =
                    muteGetUnknownContactList( &contactCount );
                if( contactCount > 0 ) {
                    printf( "Unknown contacts (waiting for approval):\n" );
                    for( int i=0; i<contactCount; i++ ) {
                        
                        printf( "%s\n", contactList[i] );
                        delete [] contactList[i];
                        }
                    }
                    else {
                        printf( "No unknown contacts.\n" );
                        }
                delete [] contactList;
                break;
                }
            case 'a':
            case 'A': {
                char *contactName = readUserString( "Enter contact name: " );

                char approved = muteApproveUnknownContact( contactName );

                if( approved ) {
                    printf( "%s added to contact list\n",
                            contactName );
                    }
                else {
                    printf( "Failed to approve %s\n",
                            contactName );
                    }
                delete [] contactName;
                break;
                }
            case 't':
            case 'T': {

                char oldValue = muteGetUnknownContactAutoApprove();

                muteSetUnknownContactAutoApprove( !oldValue );

                char newValue = muteGetUnknownContactAutoApprove();
                
                printf( "Auto-approve is " );

                if( newValue ) {
                    printf( "on\n" );
                    }
                else {
                    printf( "off\n" );
                    }
                break;
                }
            case 'r':
            case 'R':
            case 'm':
            case 'M': {
                char* name = NULL;

                if( readCharBuffer[0] == 'r' || readCharBuffer[0] == 'R' ) {
                    // use most recent contact
                    recentMessageLock->lock();
                    if( mostRecentContact != NULL ) {
                        name = stringDuplicate( mostRecentContact );
                        }
                    recentMessageLock->unlock();

                    if( name != NULL ) {
                        printf( "Send message to %s\n", name );
                        }
                    else {
                        printf( "[no recent contact]\n" );
                        }
                    }

                if( name == NULL ) {
                    name = readUserString( "Enter contact name: " );
                    }
                char *message = readUserLine( "Enter message: " );

                char *fullMessage = autoSprintf(
                    "MessageType: Chat\n"
                    "Body:%s",
                    message );
                
                char contactFound =
                    muteSendMessageToContact( name, fullMessage );

                if( ! contactFound ) {
                    printf( "Sending to %s failed\n", name );
                    }
                
                recentMessageLock->lock();
                if( mostRecentContact != NULL ) {
                    delete [] mostRecentContact;
                    }
                mostRecentContact = stringDuplicate( name );
                recentMessageLock->unlock();
                
                delete [] name;
                delete [] message;
                delete [] fullMessage;
                break;
                }
            case 'q':
            case 'Q':
                quitting = true;
                break;
            default:
                printf( "Unknown command: %c\n", readCharBuffer[0] );
                break;
            }

        // print the next prompt 
        printf( "> " );
        fflush( stdout );
        }
    delete [] readCharBuffer;

    threadPrintF( "Qutting...\n" );


    muteRemoveContactMessageHandler( handlerID );

    
    printerThread->stop();
    // signal the thread so it can get its stop signal
    messageReadySemaphore->signal();
    printerThread->join();
    delete printerThread;
    
    delete printMessageLock;
    
    recentMessageLock->lock();
    int numMessages = recentMessages->size();
    for( int i=0; i<numMessages; i++ ) {
        delete [] *( recentMessages->getElement( i ) );
        delete [] *( recentContacts->getElement( i ) );
        }
    delete recentContacts;
    delete recentMessages;

    if( mostRecentContact != NULL ) {
        delete [] mostRecentContact;
        }
    
    recentMessageLock->unlock();

    delete recentMessageLock;

    delete messageReadySemaphore;
    
    
    threadPrintF( "Stopping the web server\n" );
    muteFileTransferWebServerStop();

    
    threadPrintF( "Stopping file transfer layer\n" );
    muteFileTransferStop();


    threadPrintF( "Stopping point-to-point communications layer\n" );
    mutePointToPointStop();
    

    threadPrintF( "Saving randomness for use at next startup\n" );
    char *randState = muteGetRandomGeneratorState();

    SettingsManager::setSetting( "randomSeed", randState );
    delete [] randState;


    threadPrintF( "Stopping message routing layer\n" );
    muteStop();

    threadPrintF( "All layers are stopped, exiting.\n" );
    }
