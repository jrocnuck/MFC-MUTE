/*
 * Modification History
 *
 * 2003-October-10   Jason Rohrer
 * Created.
 *
 * 2003-October-12   Jason Rohrer
 * Added recursive list printing function.
 */



#ifndef MESSAGE_ID_TRACKER_INCLUDED_H
#define MESSAGE_ID_TRACKER_INCLUDED_H



#include "minorGems/system/MutexLock.h"
#include <stdio.h>



/**
 * Internal list data structure.
 *
 * @author Jason Rohrer
 */
class MessageIDListItem {


        
    public:


        
        /**
         * Constructs a list item.
         *
         * @param inNextItem the next item in the list, or NULL
         *   if this item is the last in the list.
         *   Must be destroyed by caller after this class is destroyed.
         * @param inUniqueName the unique name in this message ID.
         *   Must be destroyed by caller.
         * @param inCounter the counter in this message ID.
         */
        MessageIDListItem( MessageIDListItem *inNextItem,
                           char *inUniqueName,
                           unsigned int inCounter );

        
        
        ~MessageIDListItem();



        MessageIDListItem *mNextItem;
        char *mUniqueName;
        unsigned int mCounter;


        
        /**
         * Deletes all items in the list that follow this item.
         * Does not delete this item.
         */
        void deleteAll();


        
        /**
         * Inserts an item into the list somewhere after this item
         * in sorted order (smallest Counter values first).
         *
         * @param inItem the item to insert.
         *   Must have its mNextItem set to NULL.
         *   Must be destroyed by caller.
         * @return true if the item's Counter is the largest
         *   counter in the list (inserted at end of list), or false otherwise.
         */
        char insertSorted( MessageIDListItem *inItem );


        
        /**
         * Checks whether a unique name already exists in the list at
         * and beyond this item.
         *
         * @param inUniqueName the name to look for.
         *   Must be destroyed by caller.
         *
         * @return true if the name is found, or false otherwise.
         */
        char nameExists( char *inUniqueName );


        
        /**
         * Prints all counters on this list, starting with this item's counter,
         * in sorted order.
         *
         * @param inFile the file handle to print to.
         *   Must be destroyed by caller.
         */
        void printListCounters( FILE *inFile );

        
        
    };



/**
 * Class that maintains a collection of recent message IDs and filters
 * out expired or duplicate IDs.
 *
 * Message IDs handled by this class must be in the following format:
 * uniqueName_counter
 *
 * The unique name must be a contiguous (no whitespace) string that does not
 * contain "_".
 * The counter must be a 32-bit, unsigned integer as an ASCII (base 10)
 * string.
 *
 * For example:
 * DABF3C4B9AF2F80A7D957F5A3AD41174FC0ED152_6738
 *
 * @author Jason Rohrer
 */
class MessageIDTracker {


        
    public:

        
        /**
         * Constructs a tracker.
         *
         * @param inCacheSize the number of message IDs to
         *   maintain in the cache.
         *   Defaults to 100
         */        
        MessageIDTracker( int inCacheSize = 100 );

        ~MessageIDTracker();

        

        /**
         * Checks if a message ID is fresh.
         *
         * An ID is fresh if:
         * 1. We have not seen its unique name in the past.
         * 2. Its counter has not expired.
         * 3. Its counter is not too new (1,000,000 or more beyond
         *    the latest counter that we have seen)
         *
         * @param inMessageID the ID for the message.
         *   Must be destroyed by caller.
         *
         * @return true if the message ID is fresh, or
         *   false if the message ID has expired.
         */
        char checkIfIDFresh( char *inMessageID );
        

        /**
         * Gets a fresh counter for an outbound message.
         *
         * Must be combined with a unique name to be a valid message ID.
         *
         * @return a fresh counter as a 32-bit, unsigned integer.
         */
        unsigned int getFreshCounter();
        
        
    protected:

        int mMaxCacheSize;
        int mCurrentCacheSize;
        MutexLock *mLock;

        MessageIDListItem *mListHead;

        unsigned int mFreshestCounter;
        

        int mTotalMessageCount;
        FILE *mHistoryOutputFile;
        
    };



#endif
