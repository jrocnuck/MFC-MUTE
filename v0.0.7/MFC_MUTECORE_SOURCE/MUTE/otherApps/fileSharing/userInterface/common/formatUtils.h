/*
 * Modification History
 *
 * 2003-November-6   Jason Rohrer
 * Created.
 *
 * 2004-February-6   Jason Rohrer
 * Added format function for time.
 */



#ifndef FORMAT_UTILS_INCLUDED
#define FORMAT_UTILS_INCLUDED



/**
 * Formats a data size as a string with units.
 *
 * For example, the size 1024 would be formatted as "1.00 KiB".
 *
 * @param inSizeInBytes the size to format in bytes.
 *
 * @return a formatted string with scale-appropriate units (B, KiB, MiB, etc).
 *   Must be destroyed by caller.
 */
char *formatDataSizeWithUnits( unsigned long inSizeInBytes );
char *formatDataSizeWithUnits( __int64 nSizeInBytes64 );


/**
 * Formats a time interval with units, to the closest scale-appropriate unit.
 *
 * For example, the time 605.0 seconds would be formatted as "10 min".
 *
 * @param inTimeInSeconds the time interval to format in seconds.
 *
 * @return a formatted string with scale-appropriate units (ms, sec, min,
 *   hours, etc).
 *   Must be destroyed by caller.
 */
char *formatTimeIntervalWithUnits( double inTimeInSeconds );



#endif
