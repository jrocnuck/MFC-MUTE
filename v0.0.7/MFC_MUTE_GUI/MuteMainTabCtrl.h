#if !defined(AFX_MYTABCTRL_H__F3E8650F_019C_479F_9E0F_60FE1181F49F__INCLUDED_)
#define AFX_MYTABCTRL_H__F3E8650F_019C_479F_9E0F_60FE1181F49F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MyTabCtrl.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CMuteMainDlgTabCtrl window

class CMuteMainDlgTabCtrl : public CTabCtrl
{
// Construction
public:
	CMuteMainDlgTabCtrl();
	virtual ~CMuteMainDlgTabCtrl();

// Attributes
public:
	CDialog		*m_tabPages[6];
	int			m_tabCurrent;
	int			m_nNumberOfPages;
    CImageList	m_pImgLst;          //*** MCoder 05.09.2005 - als public deklariert

private:
	HICON		m_IconHandles[6];

// Operations
public:
	void	Init();
	void	SetRectangle();	
	void	GotoPageIndex( const unsigned int nPageIndex );
	inline unsigned int	GetIconWidth() const 
	{
		return m_unIconWidth;
	}

protected:
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMuteMainDlgTabCtrl)
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

	bool	m_bFirstTime;
	unsigned int m_unIconWidth;
	unsigned int m_unIconHeight;

	// Generated message map functions
protected:
	//{{AFX_MSG(CMuteMainDlgTabCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);		
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	

	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYTABCTRL_H__F3E8650F_019C_479F_9E0F_60FE1181F49F__INCLUDED_)
