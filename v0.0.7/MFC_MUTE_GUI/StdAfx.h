// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__E1802571_C87E_4646_987E_9B83DD5A1A17__INCLUDED_)
#define AFX_STDAFX_H__E1802571_C87E_4646_987E_9B83DD5A1A17__INCLUDED_

#define  WINVER  0x0400

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxtempl.h>
#include <atlbase.h>
#include <afximpl.h> // add to include directories..."???\Microsoft Visual Studio\VC98\MFC\SRC" where ??? is your install dir
					// with .NET add $(VCInstallDir)atlmfc\src\mfc to your VC++ include directories
#include <afxmt.h>

#define ARRSIZE(x)	(sizeof(x)/sizeof(x[0]))
#define YieldProcessor() __asm { rep nop } // added 08-14-2005

// we will have string resources and the equivalent ID in English
// will be offset by 1000 for the Italian equivalent, and 2000, for German..
// ETC ETC..
enum eLanguageEnums
{
	MUTE_ENGLISH	        = 0,
	MUTE_ITALIAN	        = 1000,
	MUTE_GERMAN		        = 2000,
	MUTE_FRENCH		        = 3000,
	MUTE_SPANISH			= 4000,
	MUTE_DANISH				= 5000,
	MUTE_LITHUANIAN			= 6000,
	MUTE_TURKISH			= 7000,
	MUTE_OTHER_LANGUAGE     = -1,
};

extern unsigned int	g_unStringLanguageIdOffset;
extern CStringArray strArray;
extern __int64 g_nMFCMuteBytesOut;
extern __int64 g_nMFCMuteBytesIn;
extern __int64 g_nBytesSentToAll;
extern __int64 g_nBytesDownloaded;
extern __int64 g_nBytesSearchResultsRcvd;
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__E1802571_C87E_4646_987E_9B83DD5A1A17__INCLUDED_)
