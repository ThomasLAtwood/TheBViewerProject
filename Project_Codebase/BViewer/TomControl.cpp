// TomControl.cpp : Implementation file for the TomControl base class of
//	all user interface control classes.
//
//	Written by Thomas L. Atwood
//	P.O. Box 1089
//	West Fork, Arkansas 72774
//	(479)445-4690
//	TomAtwood@Earthlink.net
//
//	Copyright © 2010 CDC
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//	
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.
//
#include "stdafx.h"
#include "TomControl.h"


#define _CRTDBG_MAP_ALLOC 

// TomControl
IMPLEMENT_DYNAMIC( TomControl, CWnd )

TomControl::TomControl( char *pControlText, int ControlWidth, int ControlHeight, int FontHeight, int FontWidth, int FontWeight,
				COLORREF TextColor, COLORREF BackgroundColor, COLORREF ActivatedBkgdColor, DWORD ControlStyle, UINT nID, char *pControlTipText )
{
	m_ControlText = pControlText;
	m_ControlWidth = ControlWidth;
	m_ControlHeight = ControlHeight;
	m_FontHeight = FontHeight;
	m_FontWidth = FontWidth;
	m_FontWeight = FontWeight * 100;			// FontWeight: 1 through 9
	m_TextColor = TextColor;
	m_OriginalIdleBkgColor = BackgroundColor;
	m_IdleBkgColor = m_OriginalIdleBkgColor;
	m_ActivatedBkgdColor = ActivatedBkgdColor;
	m_ControlStyle = ControlStyle;
	m_nObjectID = nID;
	m_pControlTipText = pControlTipText;
	m_ControlTipActivator = 0;
	m_bMouseIsOverMe = FALSE;
	m_bHasBeenCompleted = FALSE;
	m_pHistogramData = 0;
}

TomControl::~TomControl()
{
}


BEGIN_MESSAGE_MAP( TomControl, CWnd )
	//{{AFX_MSG_MAP( TomControl )
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL TomControl::IsStatic()
{
	BOOL				bIsStatic;
	CRuntimeClass		*pRuntimeClassInfo;

	pRuntimeClassInfo = GetRuntimeClass();
	bIsStatic = ( pRuntimeClassInfo != 0 && _stricmp( pRuntimeClassInfo -> m_lpszClassName, "TomStatic" ) == 0 );
	
	return bIsStatic;
}


// Caution:		Since this function creates the object, it must be called before any
//				functions such as Invalidate(), etc., that manipulate an active window.
BOOL TomControl::SetPosition( int x, int y, CWnd *pParentWnd )
{
	BOOL			bResult;
	CRect			ControlRect;
	DWORD			WindowsControlStyle;

	WindowsControlStyle = WS_CHILD | WS_VISIBLE;
	if ( IsStatic() && ( m_pControlTipText == 0 || strlen( m_pControlTipText ) == 0 ) )
		WindowsControlStyle |= WS_DISABLED;
	ControlRect.SetRect( x, y, x + m_ControlWidth, y + m_ControlHeight );
	bResult = Create( NULL, m_ControlText, WindowsControlStyle, ControlRect, pParentWnd, m_nObjectID );
	
	return bResult;
}


void TomControl::ChangeStatus( DWORD ClearStatus, DWORD SetStatus )
{
	m_ControlStyle &= ~ClearStatus;
	m_ControlStyle |= SetStatus;
	if ( m_ControlStyle & CONTROL_INVISIBLE )
		EnableWindow( FALSE );
	else if ( m_ControlStyle & CONTROL_VISIBLE )
		EnableWindow( TRUE );
}


void TomControl::HasBeenCompleted( BOOL bHasBeenCompleted )
{
	m_bHasBeenCompleted = bHasBeenCompleted;
	if ( bHasBeenCompleted )
		m_IdleBkgColor = m_ActivatedBkgdColor;
	else
		m_IdleBkgColor = m_OriginalIdleBkgColor;
}


BOOL TomControl::IsCompleted()
{
	if ( ( m_ControlStyle & CONTROL_BITMAP ) == 0 )
		return m_bHasBeenCompleted;
	else
		return TRUE;
}


BOOL TomControl::IsVisible()
{
	return ( ( m_ControlStyle & CONTROL_VISIBLE ) != 0 );
}


BOOL TomControl::CreateSpecifiedFont() 
{
	BOOL			Ok;

	Ok = ( m_TextFont.CreateFont(
			m_FontHeight,				// nHeight in device units.
			m_FontWidth,				// nWidth - use available aspect ratio
			0,							// nEscapement - make character lines horizontal
			0,							// nOrientation - individual chars are horizontal
			m_FontWeight,				// nWeight - character stroke thickness
			FALSE,						// bItalic - not italic
			FALSE,						// bUnderline - not underlined
			0,							// cStrikeOut - not a strikeout font
			ANSI_CHARSET,				// nCharSet - normal ansi characters
			OUT_DEFAULT_PRECIS,			// nOutPrecision - choose font type using default search
			CLIP_DEFAULT_PRECIS,		// nClipPrecision - use default clipping
			ANTIALIASED_QUALITY,		// nQuality - best possible appearance
			DEFAULT_PITCH | FF_SWISS,	// nPitchAndFamily - fixed or variable pitch
			"Arial"						// lpszFacename
			) != 0 );

	return Ok;
}


// When mouse messages are posted faster than a thread can process them, the system discards all
// but the most recent mouse message.
void TomControl::OnMouseMove( UINT nFlags, CPoint Point )
{
	CPoint			ParentWindowPoint;
	CWnd			*pParentWindow;

	if ( IsVisible() && IsWindowVisible() && IsWindowEnabled() && m_ControlTipActivator != 0 )
		{
		pParentWindow = GetParent();
		if ( pParentWindow != 0 )
			{
			ParentWindowPoint = Point;
			MapWindowPoints( pParentWindow, &ParentWindowPoint, 1 );
			if ( m_pControlTipText != 0 && m_ControlTipActivator != 0 && Point.x > ( 2 * m_ControlWidth / 3 ) && Point.y > ( m_ControlHeight / 2 ) )
				m_ControlTipActivator( pParentWindow, m_pControlTipText, ParentWindowPoint );
			else
				m_ControlTipActivator( pParentWindow, 0, ParentWindowPoint );
			}
		}

	CWnd::OnMouseMove( nFlags, Point );
}


