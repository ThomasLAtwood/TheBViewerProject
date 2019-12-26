// TomComboBox.cpp : Implementation file for the TomComboBox class of enhanced
//	text display and editing controls for use in a user interface window.
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
#include "ReportStatus.h"
#include "TomComboBox.h"

#define _CRTDBG_MAP_ALLOC 


// TomComboBox
TomComboBox::TomComboBox( char *pEditText, int EditWidth, int EditHeight, int FontHeight, int FontWidth, int FontWeight, int FontType,
				COLORREF TextColor, COLORREF BackgroundColor, COLORREF ActivatedBkgdColor, COLORREF VisitedBkgdColor,
				DWORD EditStyle, unsigned long ValidationType, UINT nID ) : CComboBox()
{
	m_pGroup = 0;
	m_EditText = (const char*)pEditText;
	m_EditWidth = EditWidth;
	m_EditHeight = EditHeight;
	m_FontHeight = FontHeight;
	m_FontWidth = FontWidth;
	m_FontWeight = FontWeight * 100;			// FontWeight: 1 through 9
	m_TextColor = TextColor;
	m_OriginalIdleBkgColor = BackgroundColor;
	m_IdleBkgColor = m_OriginalIdleBkgColor;
	m_ActivatedBkgdColor = ActivatedBkgdColor;
	m_VisitedBkgdColor = VisitedBkgdColor;
	m_SpecialBkgColor = COLORREF( RGB( 255, 0, 0 ) );
	m_EditStyle = EditStyle;
	m_nObjectID = nID;
	m_bHasReceivedInput = FALSE;
	m_FontType = FontType;
	m_ValidationType = ValidationType;
	m_BkgdBrush.CreateSolidBrush( BackgroundColor );
	m_ActivatedBkgdBrush.CreateSolidBrush( ActivatedBkgdColor );
	m_VisitedBkgdBrush.CreateSolidBrush( VisitedBkgdColor );
	m_pCurrentBkgdBrush = &m_BkgdBrush;
}


TomComboBox::~TomComboBox()
{
	m_EditText.Empty();
}


BEGIN_MESSAGE_MAP(TomComboBox, CWnd)
	//{{AFX_MSG_MAP(TomComboBox)
	ON_WM_CTLCOLOR()
	ON_WM_GETDLGCODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// Caution:		Since this function creates the object, it must be called before any
//				functions such as Invalidate(), etc., that manipulate an active window.
BOOL TomComboBox::SetPosition( int x, int y, CWnd *pParentWnd )
{
	BOOL			bResult;
	CRect			EditRect;
	DWORD			WindowsEditStyle;

	WindowsEditStyle = WS_CHILD | WS_VISIBLE;
	if ( m_EditStyle & EDIT_VSCROLL )
		WindowsEditStyle |= WS_VSCROLL;
	WindowsEditStyle |= CBS_DROPDOWNLIST | CBS_SORT;
	EditRect.SetRect( x, y, x + m_EditWidth, y + m_EditHeight );
	bResult = Create( WindowsEditStyle, EditRect, pParentWnd, m_nObjectID );
	CreateSpecifiedFont();
	SetFont( &m_TextFont, FALSE );
	
	return bResult;
}


void TomComboBox::SetDecimalRange( double MinValue, double MaxValue, int DecimalDigitsDisplayed )
{
	if ( m_ValidationType & EDIT_VALIDATION_DECIMAL_RANGE )
		{
		m_MinimumDecimalValue = MinValue;
		m_MaximumDecimalValue = MaxValue;
		}
	m_DecimalDigitsDisplayed = DecimalDigitsDisplayed;
}


void TomComboBox::ChangeStatus( DWORD ClearStatus, DWORD SetStatus )
{
	m_EditStyle &= ~ClearStatus;
	m_EditStyle |= SetStatus;

	if ( ClearStatus & CONTROL_VISIBLE )
		ModifyStyle( WS_VISIBLE, 0, 0 );
	else if ( SetStatus & CONTROL_VISIBLE )
		ModifyStyle( 0, WS_VISIBLE, 0 );

}


void TomComboBox::HasBeenCompleted( BOOL bHasBeenCompleted )
{
	if ( bHasBeenCompleted )
		{
		m_IdleBkgColor = m_ActivatedBkgdColor;
		m_pCurrentBkgdBrush = &m_ActivatedBkgdBrush;
		}
	else
		{
		m_IdleBkgColor = m_OriginalIdleBkgColor;
		m_pCurrentBkgdBrush = &m_BkgdBrush;
		}
}


BOOL TomComboBox::CreateSpecifiedFont() 
{
	BOOL			Ok;
	char			FaceName[ 20 ];
	BYTE			PitchAndFamily;
	
	if ( m_FontType == FIXED_PITCH_FONT )
		{
		strcpy( FaceName, "Courier" );
		PitchAndFamily = FIXED_PITCH | FF_MODERN;
		}
	else
		{
		strcpy( FaceName, "Arial" );
		PitchAndFamily = DEFAULT_PITCH | FF_SWISS;
		}

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
			PitchAndFamily,				// nPitchAndFamily - fixed or variable pitch
			FaceName					// lpszFacename
			) != 0 );

	return Ok;
}


HBRUSH TomComboBox::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
{
	HBRUSH			hBrush;

	if ( nCtlColor == CTLCOLOR_EDIT || nCtlColor == CTLCOLOR_LISTBOX )
		{
		pDC -> SetBkColor( m_IdleBkgColor );
		pDC -> SetTextColor( m_TextColor );
		pDC -> SetBkMode( OPAQUE );
		hBrush = HBRUSH( *m_pCurrentBkgdBrush );
		}
	else
		hBrush = HBRUSH( m_BkgdBrush );

	return hBrush;
}


UINT TomComboBox::OnGetDlgCode()
{
	return DLGC_WANTALLKEYS;
}


