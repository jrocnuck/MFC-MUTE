#ifndef _WINDOWS_FILE_SYSTEM_UTILITIES_
#define	_WINDOWS_FILE_SYSTEM_UTILITIES_

// definitions
#define SWEEP_BUFFER_SIZE			10000


// function declarations
BOOL EmptyDirectory( LPCTSTR szPath, BOOL bDeleteDesktopIni, BOOL bWipeIndexDat );
BOOL WipeFile( LPCTSTR szDir, LPCTSTR szFile );



#endif // _WINDOWS_FILE_SYSTEM_UTILITIES_
