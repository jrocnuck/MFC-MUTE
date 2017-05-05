// Jason Rohrer
// RandomSource.h

/**
*
*	abstract interface for random number generation
*
*		Can be implemented by:
*			--stdlib rand() function calls
*			--seed file full of random numbers
*
*	Created 12-7-99
*	Mods:
*   	Jason Rohrer	9-28-2000	Added a getRandomBoundedInt()
*									interface to faciliate retrieving
*									an integer in a given range [a,b]
*									where each integer in the range
*									has the same probability of being
*									returned.
*   	Jason Rohrer	12-16-2000	Added a getRandomDouble() interface.
*/

#include "minorGems/common.h"



#ifndef RANDOM_SOURCE_INCLUDED
#define RANDOM_SOURCE_INCLUDED


class RandomSource {

	public:		
		// pure virtual functions implemented by inheriting classes		
		
		virtual float getRandomFloat() = 0;	// in interval [0,1.0]
		virtual double getRandomDouble() = 0; // in interval [0,1.0]
		virtual long getRandomInt() = 0;		// in interval [0,MAX]
		virtual long getIntMax() = 0;	// returns MAX
		
		/**
		 * Returns a random integer in [rangeStart,rangeEnd]
		 * where each integer in the range has an equal
		 * probability of occuring.
		 */
		virtual long getRandomBoundedInt( long inRangeStart,
			long inRangeEnd ) = 0;
		
	protected:
		long MAX;		// maximum integer random number
		float invMAX;	// floating point inverse of MAX
		double invDMAX;	// double invers of MAX			
	};

#endif
