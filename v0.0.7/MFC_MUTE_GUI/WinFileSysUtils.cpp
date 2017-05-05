#include "StdAfx.h"
#include "WinFileSysUtils.h"


///////////////////////////////////////////////////////////////////////////////////////
// be careful when using this function on certain directories... your choice of the
// two boolean flags could cause problems.. :)
BOOL EmptyDirectory( LPCTSTR szPath, BOOL bDeleteDesktopIni, BOOL bWipeIndexDat )
{
	WIN32_FIND_DATA wfd;
	HANDLE hFind;
	CString sFullPath;
	CString sFindFilter;
	DWORD dwAttributes = 0;

	sFindFilter = szPath;
	sFindFilter += _T("\\*.*");
	
	hFind = FindFirstFile(sFindFilter, &wfd);
	
	if( INVALID_HANDLE_VALUE == hFind )
	{
		return FALSE;
	}

	do
	{
		if( _tcscmp(wfd.cFileName, _T(".")) == 0 || 
			_tcscmp(wfd.cFileName, _T("..")) == 0 ||
			(bDeleteDesktopIni == FALSE && _tcsicmp(wfd.cFileName, _T("desktop.ini")) == 0) )
		{
			continue;
		}

		sFullPath = szPath;
		sFullPath += _T('\\');
		sFullPath += wfd.cFileName;

		//remove readonly attributes
		dwAttributes = GetFileAttributes(sFullPath);
		if( dwAttributes & FILE_ATTRIBUTE_READONLY )
		{
			dwAttributes &= ~FILE_ATTRIBUTE_READONLY;
			SetFileAttributes(sFullPath, dwAttributes);
		}

		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			EmptyDirectory(sFullPath, bDeleteDesktopIni, bWipeIndexDat);
			RemoveDirectory(sFullPath);
		}
		else
		{
			if( bWipeIndexDat && _tcsicmp(wfd.cFileName, _T("index.dat")) == 0 )
			{
				WipeFile(szPath, wfd.cFileName);
			}
			DeleteFile(sFullPath);
		}
	}
	while( FindNextFile( hFind, &wfd ) );

	FindClose( hFind );

	return TRUE;
}

//*******************************************************************
//  FUNCTION:   -	WipeFile
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	Deletes a file
//*******************************************************************
BOOL WipeFile( LPCTSTR szDir, LPCTSTR szFile )
{
	CString sPath;
	HANDLE	hFile;
	DWORD	dwSize;
	DWORD	dwWrite;
	char	sZero[SWEEP_BUFFER_SIZE];

	memset(sZero, 0, SWEEP_BUFFER_SIZE);

	sPath = szDir;
	sPath += _T('\\');
	sPath += szFile;

	hFile = CreateFile(sPath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if( INVALID_HANDLE_VALUE == hFile )
	{
		return FALSE;
	}

	dwSize = GetFileSize( hFile, NULL );

	//skip file header (actually, I don't know the file format of index.dat)
	dwSize -= 64;
	SetFilePointer( hFile, 64, NULL, FILE_BEGIN );

	while( dwSize > 0 )
	{
		if( dwSize > SWEEP_BUFFER_SIZE )
		{
			WriteFile( hFile, sZero, SWEEP_BUFFER_SIZE, &dwWrite, NULL );
			dwSize -= SWEEP_BUFFER_SIZE;
		}
		else
		{
			WriteFile( hFile, sZero, dwSize, &dwWrite, NULL );
			break;
		}
	}

	CloseHandle( hFile );

	return TRUE;
}

