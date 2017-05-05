/*
 * Modification History
 *
 * 2003-November-6   Jason Rohrer
 * Created.
 *
 * 2004-February-6   Jason Rohrer
 * Added format function for time.
 * Fixed unit string for single unit quantities.
 */



#include "formatUtils.h"

#include "minorGems/util/stringUtils.h"



char *formatDataSizeWithUnits( unsigned long inSizeInBytes ) {
    char *sizeString;
    
    if( inSizeInBytes >= 1073741824 ) {
        // GiB range
        float sizeGiB = inSizeInBytes / (float) 1073741824.0;

        sizeString = autoSprintf( "%.2f GiB", sizeGiB ); 
        }
    else if( inSizeInBytes >= 1048576 ) {
        // MiB range
        float sizeMiB =(float) (inSizeInBytes / 1048576.0);

        sizeString = autoSprintf( "%.2f MiB", sizeMiB ); 
        }
    else if( inSizeInBytes >= 1024 ) {
        // KiB range
        float sizeKiB =(float) (inSizeInBytes / 1024.0);

        sizeString = autoSprintf( "%.2f KiB", sizeKiB ); 
        }
    else {
        // Bytes
        sizeString = autoSprintf( "%lu B", inSizeInBytes );
        }
    
    return sizeString;
    }

char *formatDataSizeWithUnits( __int64 nSizeInBytes64 )
{
	char *sizeString;	 
    
	if( nSizeInBytes64 >= 1099511627776 )
	{
		//TiB range
		double sizeTiB = nSizeInBytes64 / (double) 1073741824.0;

        sizeString =	autoSprintf( "%.2f TiB", sizeTiB ); 
	}
    if( nSizeInBytes64 >= 1073741824 ) {
        // GiB range
        double sizeGiB = nSizeInBytes64 / (double) 1073741824.0;

        sizeString = autoSprintf( "%.2f GiB", sizeGiB ); 
        }
    else if( nSizeInBytes64 >= 1048576 ) {
        // MiB range
        double sizeMiB =(double) (nSizeInBytes64 / 1048576.0);

        sizeString = autoSprintf( "%.2f MiB", sizeMiB ); 
        }
    else if( nSizeInBytes64 >= 1024 ) {
        // KiB range
        double sizeKiB =(double) (nSizeInBytes64 / 1024.0);

        sizeString = autoSprintf( "%.2f KiB", sizeKiB ); 
        }
    else {
        // Bytes
        sizeString = autoSprintf( "%lu B", nSizeInBytes64 );
        }
    
    return sizeString;
}


char *formatTimeIntervalWithUnits( double inTimeInSeconds ) {
    char *timeString;
    
    if( inTimeInSeconds >= 3600 ) {
        // hour range
        double floatHours = inTimeInSeconds / 3600;

        // round to nearest hour
        int hours = (int)floatHours;
        if( floatHours - hours >= 0.5 ) {
            hours++;
            }
        
        char *unitString;
        if( hours == 1 ) {
            unitString = "hour"; // todo.. need to get translations for this for GUI!
            }
        else {
            unitString = "hours"; // todo.. need to get translations for this for GUI!
            }
        timeString = autoSprintf( "%d %s", hours, unitString );
        }
    else if( inTimeInSeconds >= 60 ) {
        // minute range
        double floatMinutes = inTimeInSeconds / 60;

        // round to nearest minute
        int minutes = (int)floatMinutes;
        if( floatMinutes - minutes >= 0.5 ) {
            minutes++;
            }

        char *unitString;
        if( minutes == 1 ) {
            unitString = "minute";// todo.. need to get translations for this for GUI!
            }
        else {
            unitString = "minutes";// todo.. need to get translations for this for GUI!
            }
        timeString = autoSprintf( "%d %s", minutes, unitString );
        }
    else if( inTimeInSeconds >= 1 ) {
        // second range

        // round to nearest second
        int seconds = (int)inTimeInSeconds;
        if( inTimeInSeconds - seconds >= 0.5 ) {
            seconds++;
            }

        char *unitString;
        if( seconds == 1 ) {
            unitString = "second";// todo.. need to get translations for this for GUI!
            }
        else {
            unitString = "seconds";// todo.. need to get translations for this for GUI!
            }
        timeString = autoSprintf( "%d %s", seconds, unitString );
        }
    else {
        // milliseconds range
        double floatMilliseconds = inTimeInSeconds * 1000;

        // round to nearest millisecond
        int milliseconds = (int)floatMilliseconds;
        if( floatMilliseconds - milliseconds >= 0.5 ) {
            milliseconds++;
            }
        
        char *unitString;
        if( milliseconds == 1 ) {
            unitString = "millisecond"; // todo.. need to get translations for this for GUI!
            }
        else {
            unitString = "milliseconds"; // todo.. need to get translations for this for GUI!
            }
        timeString = autoSprintf( "%d %s", milliseconds, unitString );
        }
    
    return timeString;

    }
