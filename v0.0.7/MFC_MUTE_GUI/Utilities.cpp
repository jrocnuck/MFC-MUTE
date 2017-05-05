#include "StdAfx.h"
#include "Utilities.h"

CString GetResString(UINT uStringID) 
{
	CString resString;
	resString.LoadString(uStringID);
	return resString;
}

int GetAppImageListColorFlag()
{
	HDC hdcScreen = ::GetDC(NULL);
	int iColorBits = GetDeviceCaps(hdcScreen, BITSPIXEL) * GetDeviceCaps(hdcScreen, PLANES);
	::ReleaseDC(NULL, hdcScreen);
	int iIlcFlag;
	if (iColorBits >= 32)
		iIlcFlag = ILC_COLOR32;
	else if (iColorBits >= 24)
		iIlcFlag = ILC_COLOR24;
	else if (iColorBits >= 16)
		iIlcFlag = ILC_COLOR16;
	else if (iColorBits >= 8)
		iIlcFlag = ILC_COLOR8;
	else if (iColorBits >= 4)
		iIlcFlag = ILC_COLOR4;
	else
		iIlcFlag = ILC_COLOR;
	return iIlcFlag;
}


CHAR *stristr(const CHAR *str1, const CHAR *str2)
{
	const CHAR *cp = str1;
	const CHAR *s1;
	const CHAR *s2;

	if (!*str2)
		return (CHAR *)str1;

	while (*cp)
	{
		s1 = cp;
		s2 = str2;

		while (*s1 && *s2 && tolower(*s1) == tolower(*s2))
			s1++, s2++;

		if (!*s2)
			return (CHAR *)cp;

		cp++;
	}

	return NULL;
}

WORD DetectWinVersion()
{
	OSVERSIONINFOEX osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if(!GetVersionEx((OSVERSIONINFO*)&osvi))
	{
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if(!GetVersionEx((OSVERSIONINFO*)&osvi)) 
		return FALSE;
	}

	switch(osvi.dwPlatformId)
	{
		case VER_PLATFORM_WIN32_NT:
			if(osvi.dwMajorVersion <= 4)
				return _WINVER_NT4_;
			if(osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
				return _WINVER_2K_;
			if(osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
				return _WINVER_XP_;
			return _WINVER_XP_; // never return Win95 if we get the info about a NT system
      
		case VER_PLATFORM_WIN32_WINDOWS:
			if(osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
				return _WINVER_95_; 
			if(osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
				return _WINVER_98_; 
			if(osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
				return _WINVER_ME_; 
			break;
		
		default:
			break;
	}
	
	return _WINVER_95_;		// there should'nt be anything lower than this
}



CTempIconLoader::CTempIconLoader(LPCTSTR pszResourceID, int cx, int cy, UINT uFlags)
{
	//m_hIcon = theApp.LoadIcon(pszResourceID, cx, cy, uFlags);
	// todo.. in the future.. if we want to do "skins" we may want to have our own
	// LoadIcon function to pull the icons from files.. 
	m_hIcon = AfxGetApp()->LoadIcon(pszResourceID);
}

CTempIconLoader::~CTempIconLoader()
{
	if (m_hIcon)
		VERIFY( DestroyIcon(m_hIcon) );
}