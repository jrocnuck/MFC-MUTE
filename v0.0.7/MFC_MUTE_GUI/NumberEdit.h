// JROC
// This is a quick little MFC control that derives from
// the CEdit control to make sure the value is a number
// without having leading chars..etc.

#ifndef _NUMBER_EDIT_H_
#define _NUMBER_EDIT_H_


//*******************************************************************
//	CLASS:		CNumberEdit
//  COMMENTS:	-	
//*******************************************************************
class CNumberEdit : public CEdit 
{
public:
	BOOL IsValidChar( UINT nChar, int nCharPosition, int nCount );

public:
	inline void	SetAllowNegative( bool bAllowNegative )
	{
		m_bAllowNegative = bAllowNegative;
	}

	inline void	SetAllowDecimal( bool bAllowDecimal )
	{
		m_bAllowDecimal = bAllowDecimal;
	}

	virtual afx_msg void OnChar( UINT nChar, UINT nRepCnt, UINT nFlags );

	DECLARE_MESSAGE_MAP()
private:
	bool	m_bAllowNegative;
	bool	m_bAllowDecimal;

};

#endif // _NUMBER_EDIT_H_