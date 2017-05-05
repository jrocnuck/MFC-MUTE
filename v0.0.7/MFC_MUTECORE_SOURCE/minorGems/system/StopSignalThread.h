/*
 * Modification History
 *
 * 2002-April-4    Jason Rohrer
 * Created.
 * Changed to reflect the fact that the base class
 * destructor is called *after* the derived class destructor.
 *
 * 2003-September-5   Jason Rohrer
 * Moved into minorGems.
 *
 * 2004-November-19   Jason Rohrer
 * Changed to virtual inheritance from Thread class.
 */



#ifndef STOP_SIGNAL_THREAD_INCLUDED
#define STOP_SIGNAL_THREAD_INCLUDED



#include "minorGems/system/Thread.h"
#include "minorGems/system/MutexLock.h"



/**
 * Abstract subclass of thread that has a stop signal.
 *
 * Note that subclasses MUST check the isStopped() function
 * periodically in their run() function for this class to work
 * properly.
 *
 * @author Jason Rohrer
 */
class StopSignalThread : public virtual Thread {



    public:


        
        /**
         * Only destroys this thread.
         * Does not stop or join.
         */
        virtual ~StopSignalThread();



    protected:


        
        StopSignalThread();

        

        /**
         * Signals this thread to stop.
         *
         * Thread safe.
         *
         * Thread must be joined after this call returns.
         */
        void stop();

        
        
        /**
         * Gets whether this thread has been signaled to stop.
         *
         * Thread safe.
         *
         * @return true if this thread should stop.
         */
        char isStopped();


        
    private:

        MutexLock *mStopLock;
        char mStopped;

        
        
    };



#endif
