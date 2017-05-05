/*
 * Modification History
 *
 * 2003-July-27    Jason Rohrer
 * Created.
 *
 * 2004-December-13    Jason Rohrer
 * Switched to use a binary semaphore for sleeping.
 */



#ifndef CHANNEL_RECEIVING_THREAD_MANAGER_INCLUDED
#define CHANNEL_RECEIVING_THREAD_MANAGER_INCLUDED



#include "ChannelReceivingThread.h"

#include "minorGems/util/SimpleVector.h"
#include "minorGems/system/Thread.h"
#include "minorGems/system/MutexLock.h"
#include "minorGems/system/BinarySemaphore.h"



/**
 * A thread that manages the destruction of ChannelReceivingThreads.
 *
 * @author Jason Rohrer.
 */
class ChannelReceivingThreadManager : public Thread {



    public:
        
        /**
         * Constructs and starts this manager.
         */
        ChannelReceivingThreadManager();


        
        /**
         * Stops and destroys this manager.
         */
        ~ChannelReceivingThreadManager();

        

        /**
         * Adds a thread to this manager.
         *
         * @param inThread the thread to add.
         *   Will be destroyed by this class.
         */
        void addThread( ChannelReceivingThread *inThread );

        

        // implements the Thread interface
        void run();


        
    protected:
        MutexLock *mLock;

        SimpleVector<ChannelReceivingThread *> *mThreadVector;
        
        char mStopSignal;

        BinarySemaphore *mSleepSemaphore;
        
        
    };




#endif
