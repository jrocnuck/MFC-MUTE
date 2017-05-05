/*
 * Modification History
 *
 * 2002-April-4    Jason Rohrer
 * Created.
 * Changed to reflect the fact that the base class
 * destructor is called *after* the derived class destructor.
 *
 * 2002-August-5   Jason Rohrer
 * Fixed member initialization order to match declaration order.
 *
 * 2003-September-5   Jason Rohrer
 * Moved into minorGems.
 */



#include "StopSignalThread.h"



StopSignalThread::StopSignalThread()
    : mStopLock( new MutexLock() ), mStopped( false ) {

    }



StopSignalThread::~StopSignalThread() {

    delete mStopLock;
    }



char StopSignalThread::isStopped() {
    mStopLock->lock();
    char stoped = mStopped;
    mStopLock->unlock();

    return stoped;
    }



void StopSignalThread::stop() {
    mStopLock->lock();
    mStopped = true;
    mStopLock->unlock();
    }

