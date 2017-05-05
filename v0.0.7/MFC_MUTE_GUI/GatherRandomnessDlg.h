#if !defined(AFX_GATHERRANDOMNESSDLG_H__5B629DFB_8272_4DCD_BDEA_293E2654DECF__INCLUDED_)
#define AFX_GATHERRANDOMNESSDLG_H__5B629DFB_8272_4DCD_BDEA_293E2654DECF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GatherRandomnessDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGatherRandomnessDlg dialog

class CGatherRandomnessDlg : public CDialog
{
// Construction
public:
	CGatherRandomnessDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGatherRandomnessDlg)
	enum { IDD = IDD_RANDOM_SEED_DLG };
	CString	m_strRandomnessString;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGatherRandomnessDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGatherRandomnessDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GATHERRANDOMNESSDLG_H__5B629DFB_8272_4DCD_BDEA_293E2654DECF__INCLUDED_)
