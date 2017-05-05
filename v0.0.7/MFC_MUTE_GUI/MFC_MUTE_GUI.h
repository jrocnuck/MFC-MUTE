// MUTEMFC2.h : main header file for the MUTEMFC2 application
//

#if !defined(AFX_MUTEMFC2_H__55F0FA24_684C_4772_9E83_6044A7E5DA3F__INCLUDED_)
#define AFX_MUTEMFC2_H__55F0FA24_684C_4772_9E83_6044A7E5DA3F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "StringResources.h" // for all the strings

/////////////////////////////////////////////////////////////////////////////
// CMUTEMFC2App:
// See MUTEMFC2.cpp for the implementation of this class
//

class CMUTEMFC2App : public CWinApp
{
public:
	CMUTEMFC2App();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMUTEMFC2App)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	CString		m_strStartingPath;
	//{{AFX_MSG(CMUTEMFC2App)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MUTEMFC2_H__55F0FA24_684C_4772_9E83_6044A7E5DA3F__INCLUDED_)
