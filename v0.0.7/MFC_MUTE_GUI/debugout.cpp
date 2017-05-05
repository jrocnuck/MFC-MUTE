#include "stdafx.h"

// This function can be used in places where including lots
// of windows header files cause headaches.
// This just calls the MSVC++ function OutputDebugString
// to put text in the debug window while running
// in debugger mode.
void JROCDebugString( char * pszString )
{
#ifdef _DEBUG
	OutputDebugString( pszString );
#endif
}