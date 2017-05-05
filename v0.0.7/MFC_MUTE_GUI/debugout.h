#ifndef _JROC_DEBUG_OUTPUT_
#define	_JROC_DEBUG_OUTPUT_

#ifndef TRACE
	#define TRACE( _x )	 JROCDebugString( (char *) _x );
#endif
void JROCDebugString( char * pszString );


#endif // _JROC_DEBUG_OUTPUT_