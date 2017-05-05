/*
 * Modification History
 *
 * 2003-October-10   Jason Rohrer
 * Created.
 *
 * 2003-October-12   Jason Rohrer
 * Fixed bugs and leaks.
 * Added recursive list printing function.
 *
 * 2003-November-18   Jason Rohrer
 * Added runtime setting for message history log.
 */



#include "MessageIDTracker.h"

#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SettingsManager.h"

#include <time.h>



// Incoming IDs that have counters far beyond our largest counter
// are seen as "too new" (probably an attack?) and not counted as fresh
unsigned int MESSAGE_COUNTER_TOO_NEW_INCREMENT = 1000000;

// maximum unsigned int
unsigned int MAX_MESSAGE_COUNTER = 4294967295U;



unsigned int getCounterDistance( unsigned int inCounterA,
                                 unsigned int inCounterB,
                                 char *outABelowB ) {

    unsigned int firstDistance;
    char firstABelowB;
    
    if( inCounterA < inCounterB ) {
        firstDistance = inCounterB - inCounterA;
        firstABelowB = true;
        }
    else {
        firstDistance = inCounterA - inCounterB;
        firstABelowB = false;
        }

    unsigned int secondDistance;
    char secondABelowB;
    
    if( inCounterA < inCounterB ) {
        secondDistance =
            MAX_MESSAGE_COUNTER - inCounterB + inCounterA;
        secondABelowB = false;
        }
    else {
        secondDistance =
            MAX_MESSAGE_COUNTER - inCounterA + inCounterB;
        secondABelowB = true;
        }

    // return smaller distance
    if( firstDistance < secondDistance ) {
        *outABelowB = firstABelowB;
        return firstDistance;
        }
    else {
        *outABelowB = secondABelowB;
        return secondDistance;
        }    
    }
                                   




MessageIDListItem::MessageIDListItem( MessageIDListItem *inNextItem,
                                      char *inUniqueName,
                                      unsigned int inCounter )
    : mNextItem( inNextItem ),
      mUniqueName( stringDuplicate( inUniqueName ) ),
      mCounter( inCounter ) {
    
    }

        
        
MessageIDListItem::~MessageIDListItem() {
    delete [] mUniqueName;
    }



void MessageIDListItem::deleteAll() {
    if( mNextItem != NULL ) {
        mNextItem->deleteAll();
        delete mNextItem;

        mNextItem = NULL;
        }
    }



char MessageIDListItem::insertSorted( MessageIDListItem *inItem ) {

    if( mNextItem != NULL ) {

        char belowNext;
        getCounterDistance( inItem->mCounter, mNextItem->mCounter,
                            &belowNext);

        if( belowNext ) {
            // inItem should go between us and our current next item
            
            // its next item is already NULL
            inItem->mNextItem = mNextItem;
            
            mNextItem = inItem;
            
            // inItem is not the last on the list
            return false;
            }
        else {
            // should go somewhere after our next item
            
            // recurse down list
            return mNextItem->insertSorted( inItem );
            }
        }
    else {
        // we're the last on the list

        mNextItem = inItem;

        // inItem is now last item on list
        return true;
        }
    }



char MessageIDListItem::nameExists( char *inUniqueName ) {
    if( strcmp( inUniqueName, mUniqueName ) == 0 ) {
        return true;
        }
    else {
        if( mNextItem != NULL ) {
            return mNextItem->nameExists( inUniqueName );
            }
        else {
            // end of list, not found
            return false;
            }
        }
    }



void MessageIDListItem::printListCounters( FILE *inFile ) {
    fprintf( inFile, "%u ", mCounter );
    if( mNextItem != NULL ) {
        mNextItem->printListCounters( inFile );
        }
    else {
        fprintf( inFile, "\n" );
        }
    }



MessageIDTracker::MessageIDTracker( int inCacheSize )
    : mMaxCacheSize( inCacheSize ),
      mCurrentCacheSize( 0 ),
      mLock( new MutexLock() ),
      mListHead( NULL ),
      mFreshestCounter( 0 ),
      mTotalMessageCount( 0 ),
      mHistoryOutputFile( NULL ) {

    char found;
    int logMessageHistory =
        SettingsManager::getIntSetting( "logMessageHistory",
                                        &found );

    if( logMessageHistory == 1 ) {
        mHistoryOutputFile = fopen( "messageHistory.log", "w" );
        }
    }



MessageIDTracker::~MessageIDTracker() {
    if( mListHead != NULL ) {
        mListHead->deleteAll();
        delete mListHead;
        mListHead = NULL;
        }

    if( mHistoryOutputFile != NULL ) {
        fclose( mHistoryOutputFile );
        }
    
    delete mLock;
    }

        

char MessageIDTracker::checkIfIDFresh( char *inMessageID ) {
    mLock->lock();

    if( mHistoryOutputFile != NULL ) {
        fprintf( mHistoryOutputFile,
                 "%d %d %s",
                 mTotalMessageCount,
                 (int)( time( NULL ) ),
                 inMessageID );
        }

    char isFresh = false;


    // split into parts
    char *workingID = stringDuplicate( inMessageID );
    char *pointerToDelim = strstr( workingID, "_" );

    char *uniqueName;
    unsigned int counter;
    char idPartsFound = false;
    
    if( pointerToDelim != NULL ) {
        // terminate
        pointerToDelim[0] = '\0';

        uniqueName = workingID;

        char *counterString = &( pointerToDelim[1] );

        int numRead = sscanf( counterString, "%u", &counter );

        if( numRead == 1 ) {
            idPartsFound = true;
            }
        }

    if( idPartsFound ) {

        
        if( mListHead == NULL ) {
            // empty list, all are fresh
            isFresh = true;
            
            mListHead = new MessageIDListItem( NULL, uniqueName, counter );
            mCurrentCacheSize++;
            mFreshestCounter = counter;
            }
        else {
            // list not empty

            // check if name already exists
            if( ! mListHead->nameExists( uniqueName ) ) {
            
                if( mCurrentCacheSize < mMaxCacheSize ) {
                    // room in list, all are fresh
                    isFresh = true;
                    
                    if( counter <= mListHead->mCounter ) {
                        // new head
                        MessageIDListItem *newItem =
                            new MessageIDListItem( mListHead,
                                                   uniqueName,
                                                   counter );
                        
                        mListHead = newItem;
                    
                        mCurrentCacheSize++;
                        // no change to freshest counter
                        }
                    else {
                        // somewhere after head
                        MessageIDListItem *newItem =
                            new MessageIDListItem( NULL,
                                                   uniqueName,
                                                   counter );
                                        
                        char isNewTail =
                            mListHead->insertSorted( newItem );
                        
                        mCurrentCacheSize++;
                        
                        if( isNewTail ) {
                            // new item has freshest counter
                            mFreshestCounter = counter;
                            }
                        }                
                    }
                else {
                    // cache full
                    
                    // make sure ID not too new
                    char belowFreshest;
                    unsigned int distFromFreshest =
                        getCounterDistance( counter, mFreshestCounter,
                                            &belowFreshest );
                    if( belowFreshest ||
                        ( !belowFreshest &&
                          distFromFreshest <
                          MESSAGE_COUNTER_TOO_NEW_INCREMENT ) ) {

                        char belowHead;
                        getCounterDistance( counter, mListHead->mCounter,
                                            &belowHead );
                        
                        if( belowHead ) {
                            // don't even bother inserting
                            }
                        else {
                            
                            // insert and then delete the smallest (the head)
                            
                            MessageIDListItem *newItem =
                                new MessageIDListItem( NULL,
                                                       uniqueName,
                                                       counter );
                                        
                            char isNewTail =
                                mListHead->insertSorted( newItem );
                            
                            // delete head
                            MessageIDListItem *oldHead = mListHead;
                            mListHead = mListHead->mNextItem;
                            
                            delete oldHead;
                            
                            // no change in cache size
                            
                            if( isNewTail ) {
                                mFreshestCounter = counter;
                                }
                        
                            isFresh = true;
                            }  // end else not below head
                        }  // end if not too new
                    } // end else cache full
                } // end if name does not already exist in cache
            } // end else list not empty
        } // end if parts found
    
    delete [] workingID;
    

    
    if( mHistoryOutputFile != NULL ) {
        if( isFresh ) {
            fprintf( mHistoryOutputFile, " [fresh]\n" );
            }
        else {
            fprintf( mHistoryOutputFile, " [expired]\n" );
            }

        
        fprintf( mHistoryOutputFile, "Current sorted ID cache:\n" );
        if( mListHead != NULL ) {
            mListHead->printListCounters( mHistoryOutputFile );
            }
        else {
            fprintf( mHistoryOutputFile, "empty\n" );
            }
        
        fflush( mHistoryOutputFile );
        }
    
    
    mTotalMessageCount++;
    
    mLock->unlock();

    return isFresh;
    }
    



unsigned int MessageIDTracker::getFreshCounter() {
    mLock->lock();

    
    unsigned int newID;

    if( mFreshestCounter < MAX_MESSAGE_COUNTER ) {
        newID = mFreshestCounter + 1;
        }
    else {
        // wrap around
        newID = 0;
        }
    
    mLock->unlock();

    return newID;
    }        
