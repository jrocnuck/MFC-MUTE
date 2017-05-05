// Jason Rohrer
// StdRandomSource.h

/**
*
*	Implementation of random number generation that uses stdlib calls
*
*
*	Created 12-7-99
*	Mods:
*   	Jason Rohrer	9-28-2000	Added a getRandomBoundedInt()
*									implementation
*   	Jason Rohrer	12-7-2000	Overloaded constructor to support
*									specifying a seed.
*   	Jason Rohrer	12-16-2000	Added a getRandomDouble() function.
*   	Jason Rohrer	12-17-2000	Fixed bug in initialization of invDMAX
*									in default constructor.
*   	Jason Rohrer	9-13-2001	Fixed a bug in getRandomBoundedInt()
*									as floats were being used, and they
*									don't provide enough resolution.
*   	Jason Rohrer	10-11-2002	Fixed some type casting warnings.
*/

#include "minorGems/common.h"



#ifndef STD_RANDOM_SOURCE_INCLUDED
#define STD_RANDOM_SOURCE_INCLUDED

#include <stdlib.h>
#include <time.h>
#include "RandomSource.h"

class StdRandomSource : public RandomSource {

	public:		
		
		StdRandomSource();		// needed to seed stdlib generator
		
		// specify the seed for the stdlib generator
		StdRandomSource( unsigned long inSeed );
			
			
		// implements these functions
		float getRandomFloat();	// in interval [0,1.0]
		double getRandomDouble(); // in interval [0,1.0]
		long getRandomInt();		// in interval [0,MAX]
		long getIntMax();	// returns MAX	
		
		/**
		 * Returns a random integer in [rangeStart,rangeEnd]
		 * where each integer in the range has an equal
		 * probability of occuring.
		 */
		long getRandomBoundedInt( long inRangeStart,
			long inRangeEnd );	
	
	private:
		double mInvMAXPlusOne; //  1 / ( MAX + 1 )
	};



inline StdRandomSource::StdRandomSource() {
	MAX = RAND_MAX;
	srand( (unsigned)time(NULL) );
	invMAX = (float)1.0 / ((float)MAX);
	invDMAX = 1.0 / ((double)MAX);
	mInvMAXPlusOne = 1.0 / ( ( (float)MAX ) + 1.0 );
	}



inline StdRandomSource::StdRandomSource( unsigned long inSeed ) {
	MAX = RAND_MAX;
	srand( inSeed );
	invMAX = (float)1.0 / ((float)MAX);
	invDMAX = 1.0 / ((double)MAX);
	mInvMAXPlusOne = 1.0 / ( ( (double)MAX ) + 1.0 );
	}
	


inline float StdRandomSource::getRandomFloat() {
	
	return (float)(rand()) * invMAX;
	}



inline double StdRandomSource::getRandomDouble() {
	
	return (double)(rand()) * invDMAX;
	}



inline long StdRandomSource::getRandomInt() {
	
	return rand();
	}

inline long StdRandomSource::getIntMax() {
	
	return MAX;
	}

inline long StdRandomSource::getRandomBoundedInt( long inRangeStart,
	long inRangeEnd ) {
	
	// float in range [0,1)
	double randFloat = (double)( rand() ) * mInvMAXPlusOne;

	long onePastRange = inRangeEnd + 1;

	long magnitude = (int)( randFloat * ( onePastRange - inRangeStart ) );
	
	return magnitude + inRangeStart;
	}

#endif
