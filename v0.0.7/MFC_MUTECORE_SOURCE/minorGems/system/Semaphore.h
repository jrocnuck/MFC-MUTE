/*
 * Modification History
 *
 * 2001-January-11		Jason Rohrer
 * Created.
 *
 * 2001-January-11		Jason Rohrer
 * Added a willBlock() function. 
 *
 * 2001-February-24		Jason Rohrer
 * Fixed incorrect delete usage.
 *
 * 2002-February-11		Jason Rohrer
 * Fixed a mistake in the signal() comment.
 *
 * 2003-August-26   Jason Rohrer
 * Added support for timeouts on wait.
 *
 * 2003-December-28   Jason Rohrer
 * Fixed a bug in semaphore value when we timeout on wait.
 *
 * 2004-January-9   Jason Rohrer
 * Fixed a preprocessor error.
 */

#include "minorGems/common.h" 



#ifndef SEMAPHORE_CLASS_INCLUDED
#define SEMAPHORE_CLASS_INCLUDED

#include "MutexLock.h"
#include "BinarySemaphore.h"


/**
 * General semaphore with an unbounded value.
 *
 * This class uses BinarySemaphores to implement general semaphores,
 * so it relies on platform-specific BinarySemaphore implementations, 
 * but this class itself is platform-independent.
 *
 * @author Jason Rohrer
 */
class Semaphore : public BinarySemaphore {

	public:
		
		/**
		 * Constructs a semaphore.
		 *
		 * @param inStartingValue the starting value for this semaphore.
		 *   Defaults to 0 if unspecified.
		 */
		Semaphore(){};
		
		~Semaphore(){};

		
		inline int wait( int inTimeoutInMilliseconds = -1 )
		{
			return BinarySemaphore::wait( inTimeoutInMilliseconds );
		}
		
		
		/**
		 * Signals the semaphore, allowing a waiting thread to return from
		 * its call to wait(). (The semaphore is set to 1 by this call if
		 * no thread is waiting on the semaphore currently.)
		 */
		inline void signal()
		{
			BinarySemaphore::signal();
		}

	};

#endif