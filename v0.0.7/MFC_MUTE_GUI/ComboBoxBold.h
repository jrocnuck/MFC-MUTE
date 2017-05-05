#if !defined(AFX_COMBOBOXBOLD_H__D3478A87_E84A_4351_A136_432E41DB0929__INCLUDED_)
#define AFX_COMBOBOXBOLD_H__D3478A87_E84A_4351_A136_432E41DB0929__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ComboBoxBold.h : header file
//

#include <afxtempl.h>


struct ITEMDATA
{
	BOOL bold;
	HICON icon;
};
/////////////////////////////////////////////////////////////////////////////
// CComboBoxBold window

class CComboBoxBold : public CComboBox
{
// Construction
public:
	CComboBoxBold();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CComboBoxBold)
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetIcon(int iItem, CString strFilePath);
	void SetIcon(int iItem, int iconId);
	void SetItemBold(int iItem,bool bold);
	virtual ~CComboBoxBold();

	// Generated message map functions
protected:
	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
	//{{AFX_MSG(CComboBoxBold)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	CMap<int,int,ITEMDATA,ITEMDATA> myMap;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMBOBOXBOLD_H__D3478A87_E84A_4351_A136_432E41DB0929__INCLUDED_)
