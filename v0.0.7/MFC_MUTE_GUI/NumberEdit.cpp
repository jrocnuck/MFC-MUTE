// JROC
// This is a quick little MFC control that derives from
// the CEdit control to make sure the value is a number
// without having leading chars..etc.

#include "stdafx.h"
#include "NumberEdit.h"

//*******************************************************************
//  FUNCTION:   -	OnChar()
//  RETURNS:    -	
//  PARAMETERS: -	
//  COMMENTS:	-	
//*******************************************************************
void CNumberEdit::OnChar( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	int nStartSel, nEndSel, nCount;
	GetSel( nStartSel, nEndSel );

	// Get the number of highlighted characters .
	nCount = nEndSel - nStartSel;

	// If character passes for the position, continue
	if ( IsValidChar( nChar, nStartSel, nCount ) )
	{
		CEdit::OnChar( nChar, nRepCnt, nFlags );
	}

	return;
}

//*******************************************************************
//  FUNCTION:   -	IsValidChar()
//  RETURNS:    -	true - if valid, false otherwise.
//  PARAMETERS: -	
//  COMMENTS:	-	Allows the minus sign (only at the
//					beginning of the string) and all digits (0-9).
//*******************************************************************
BOOL CNumberEdit::IsValidChar( UINT nChar, int nCharPosition, int nCount )
{
	BOOL bAllowChar = TRUE;
	CString strTmp;
	int nTextLimit = (int)GetLimitText();

	GetWindowText( strTmp );

	if ( nChar == VK_BACK )  // Always allow backspace.
	{
		return TRUE;
	}

	if ( nTextLimit > 0 )  // Zero means no limit.
	{
		// If no text is highlighted then check the text limit.
		if ( nCount < 1 )
		{
			// Don't allow any characters after the limit.
			if ( nCharPosition >= nTextLimit )
				return FALSE;
	
			// If the position is less than the limit, we still need to check
			// the size of the string so that we don't exceed the limit.
			if ( strTmp.GetLength() == nTextLimit )
				return FALSE;
		}
	}

	// if the character is not allowed, return without doing anything
	// check isprint to ensure that unprinted characters are passed to cedit (ctrl-v,etc.)
	if( !isdigit( nChar ) && isprint( nChar ) )
	{
		bAllowChar = FALSE;

		switch( nChar )
		{
			case '-':				
				if ( nCharPosition == 0 )
				{
					// Allow the '-' even if other characters are highlighted.
					//allow '-' only as first character

					// none selected
					if ( nCount == 0 )
					{											
						// if it's already in the box, don't allow it again!
						if( -1 == strTmp.Find( '-', 0 ) )
						{
							if( m_bAllowNegative )
							{
								bAllowChar = TRUE;
							}
							else
							{
								bAllowChar = FALSE;
							}
						}						
					}
					else
					{
						// multiple characters selected
						if( m_bAllowNegative )
						{
							bAllowChar = TRUE;
						}
						else
						{
							bAllowChar = FALSE;
						}
					}

				}
				break;
			case '.':				
				// if it's already in the box, don't allow it again!
				if( -1 == strTmp.Find( '.', 0 ) )
				{
					if( m_bAllowDecimal )
					{
						bAllowChar = TRUE;
					}
					else
					{
						bAllowChar = FALSE;
					}					
				}
				break;
			default:
				break;
		} 
	}

	return bAllowChar;
}


BEGIN_MESSAGE_MAP(CNumberEdit, CEdit)
	//{{AFX_MSG_MAP(CNumberEdit)
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()	
