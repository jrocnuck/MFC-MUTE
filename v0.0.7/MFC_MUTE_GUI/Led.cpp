///////////////////////////////////////////////////////////////////////////////
// Led.cpp : implementation file
// Visual Source Safe: $Revision: 1 $
//
// Led static control. Will display a LED in 4 different colors and two shapes.
///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1998-1999 Monte Variakojis ( monte@apollocom.com )
// All rights reserved - not to be sold.
///////////////////////////////////////////////////////////////////////////////


#include "StdAfx.h"
#include "Led.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLed
#define TIMER_ID_PING		1		// Timer Ping ID

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CLed::CLed( )
{
	m_nLedColor = LED_COLOR_RED;
	m_nLedMode = LED_OFF;
	m_nLedShape = LED_ROUND;
}

CLed::CLed( UINT unIDBMP )
{
	LoadBitmapResource( unIDBMP );
	m_nLedColor = LED_COLOR_RED;
	m_nLedMode = LED_OFF;
	m_nLedShape = LED_ROUND;
}

CLed::~CLed()
{
	VERIFY(m_LedBitmap.DeleteObject());
}


BEGIN_MESSAGE_MAP(CLed, CStatic)
	//{{AFX_MSG_MAP(CLed)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLed message handlers

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void CLed::OnPaint() 
{
	if( !m_Semaphore.Lock(1500) )
	{
		return;
	}

	CPaintDC dc(this); // device context for painting
	DrawLed(&dc,m_nLedColor,m_nLedMode,m_nLedShape);
	
	// Do not call CStatic::OnPaint() for painting messages
	m_Semaphore.Unlock();
}



//**************************************************************************************
//  FUNCTION:   -	LoadBitmapResource()
//  RETURNS:    -
//  PARAMETERS: -	UINT unIDBMP
//  COMMENTS:   -	This loads the BITMAP resource.  This allows us to not HARD CODE the
//					resource ID.
//**************************************************************************************
void CLed::LoadBitmapResource( UINT unIDBMP )
{
	if( 0 != unIDBMP )
	{
		m_LedBitmap.LoadBitmap(unIDBMP);
	}
}


///////////////////////////////////////////////////////////////////////////////
// Name:		SetLed
// Description:	This method will draw the LED to the specified DC.
//
// Entry:
//				CDC *pDC - DC to draw to
//
//				int iLedColor - Where color is defined by:
//			 		LED_COLOR_RED
//					LED_COLOR_GREEN
//					LED_COLOR_YELLOW
//					LED_COLOR_BLUE
//
//				int iMode - where mode is defined by:
//					LED_ON
//					LED_OFF
//					LED_DISABLED
//
//				int iShape - where shape is defined by:
//					LED_ROUND
//					LED_SQUARE
//					LED_RECTANGLE
///////////////////////////////////////////////////////////////////////////////
void CLed::DrawLed(CDC *pDC,int nLEDColor, int nMode, int nShape)
{
	COLORREF		OldBkColor;
	CDC				srcDC;
	CDC				dcMask;
	CDC				TempDC;
	CBitmap			*pBitmap;
	CBitmap			*pOldBitmap;
	CBitmap			*pOldMaskbitmap;
	CBitmap			*pOldBitmapTemp;	
	CBitmap			bitmapTemp;
	CBitmap			TransBitmap;
	CRect			rect;
	int				nWidth = LED_SIZE;
	int				nHeight = LED_SIZE;
	int				nTmpWidth;
	int				nTmpHeight;
	

	GetClientRect(&rect);

	
	if( nShape == LED_RECTANGLE )
	{
		nWidth = LED_SIZE * 2;
	}

	//
	// Center led within an oversized window
	//
	if( (rect.Width() >= nWidth) && (rect.Height() >= nHeight) )
	{
		nTmpWidth = rect.Width();
		nTmpHeight = rect.Height();
		rect.left += (nTmpWidth - nWidth)/2;
		rect.right -= (nTmpWidth - nWidth)/2;
		rect.top += (nTmpHeight - nHeight)/2;
		rect.bottom -= (nTmpHeight - nHeight)/2;
	}

	//
	// Prepare temporary DCs and bitmaps
	//
	TempDC.CreateCompatibleDC(pDC);
	srcDC.CreateCompatibleDC(pDC);
	dcMask.CreateCompatibleDC(pDC);
	TransBitmap.CreateBitmap(nWidth,nHeight,1,1,NULL);	
	pBitmap = &m_LedBitmap;
	

	pOldBitmap = srcDC.SelectObject(pBitmap);
	pOldMaskbitmap = dcMask.SelectObject(&TransBitmap);	
	bitmapTemp.CreateCompatibleBitmap(pDC,nWidth,nHeight);

	//
	// Work with tempDC and bitmapTemp to reduce flickering
	//
	pOldBitmapTemp = TempDC.SelectObject(&bitmapTemp);
	TempDC.BitBlt(0, 0, nWidth, nHeight, pDC, rect.left, rect.top, SRCCOPY); 

	//
	// Create mask
	//
	OldBkColor = srcDC.SetBkColor(RGB(255,0,255));
	dcMask.BitBlt(0, 0, nWidth, nHeight,&srcDC, nMode*nWidth, nLEDColor+nShape, SRCCOPY);	
	srcDC.SetBkColor(OldBkColor);

	//
	// Using the IDB_LEDS bitmap, index into the bitmap for the appropriate
	// LED. By using the mask color (RGB(255,0,255)) a mask has been created
	// so the bitmap will appear transparent.
	//
	TempDC.BitBlt(0, 0, nWidth, nHeight, &srcDC, nMode*nWidth, nLEDColor+nShape, SRCINVERT); 
	TempDC.BitBlt(0, 0, nWidth, nHeight,&dcMask, 0, 0, SRCAND); 
	TempDC.BitBlt(0, 0, nWidth, nHeight, &srcDC, nMode*nWidth, nLEDColor+nShape, SRCINVERT); 

	//
	// Since the actual minipulation is done to tempDC so there is minimal
	// flicker, it is now time to draw the result to the screen.
	//
	pDC->BitBlt(rect.left, rect.top, nWidth, nHeight, &TempDC, 0, 0, SRCCOPY); 
	
	//
	// House cleaning
	//
	srcDC.SelectObject(pOldBitmap);
	dcMask.SelectObject(pOldMaskbitmap);
	TempDC.SelectObject(pOldBitmapTemp);

	VERIFY(TransBitmap.DeleteObject());
	VERIFY(bitmapTemp.DeleteObject());

	VERIFY(srcDC.DeleteDC());
	VERIFY(dcMask.DeleteDC());
	VERIFY(TempDC.DeleteDC());	
}

///////////////////////////////////////////////////////////////////////////////
// Name:		SetLed
// Description:	This method will draw and set led parameters.
//
// Entry:		int iLedColor - Where color is defined by:
//			 		LED_COLOR_RED
//					LED_COLOR_GREEN
//					LED_COLOR_YELLOW
//					LED_COLOR_BLUE
//
//				int iMode - where mode is defined by:
//					LED_ON
//					LED_OFF
//					LED_DISABLED
//
//				int iShape - where shape is defined by:
//					LED_ROUND
//					LED_SQUARE
//					LED_RECTANGLE
///////////////////////////////////////////////////////////////////////////////
void CLed::SetLed(int nLedColor, int nMode, int nShape)
{
	CDC *pDC;

	m_nLedColor		= nLedColor;
	m_nLedMode		= nMode;
	m_nLedShape		= nShape;

	if( !m_Semaphore.Lock(1500) )
	{
		return;
	}

	pDC = GetDC();
	
	if( NULL != pDC )
	{
		DrawLed(pDC,m_nLedColor,m_nLedMode,m_nLedShape);
		ReleaseDC(pDC);
	}

	m_Semaphore.Unlock();
}

///////////////////////////////////////////////////////////////////////////////
// Name:		Ping
// Description:	This method will turn the led on for dwTimeout milliseconds and
//				then turn it off.
//
// Entry:		DWORD dwTimeout - Time out in  milliseconds
///////////////////////////////////////////////////////////////////////////////
void CLed::Ping(DWORD dwTimeout)
{
	// Return if pinging
	if(m_bPingEnabled == TRUE)
	{
		KillTimer(TIMER_ID_PING);
	}

	m_bPingEnabled = TRUE;
	SetLed(m_nLedColor,CLed::LED_ON,m_nLedShape);
	SetTimer(TIMER_ID_PING,dwTimeout,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//*******************************************************************
//  FUNCTION:   -	OnTimer()
//  RETURNS:    -	
//  PARAMETERS: -	UINT nIDEvent -- ID of the event that fired the timer
//  COMMENTS:	-	Called by the Windows frame work when the window
//					receives a WM_TIMER message.
//*******************************************************************
void CLed::OnTimer(UINT nIDEvent) 
{
	if(nIDEvent == TIMER_ID_PING)
	{
		SetLed(m_nLedColor,CLed::LED_OFF,m_nLedShape);
		KillTimer(nIDEvent);
		m_bPingEnabled = FALSE;
	}
	
	CStatic::OnTimer(nIDEvent);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL CLed::OnEraseBkgnd(CDC* pDC) 
{
	// No background rendering
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
