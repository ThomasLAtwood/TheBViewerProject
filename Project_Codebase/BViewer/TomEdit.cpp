// TomEdit.cpp : Implementation file for the TomEdit class of enhanced
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
#include "TomEdit.h"

#define _CRTDBG_MAP_ALLOC 


extern BOOL						bMakeDumbButtons;


// TomEdit
TomEdit::TomEdit( char *pEditText, int EditWidth, int EditHeight, int FontHeight, int FontWidth, int FontWeight, int FontType,
				COLORREF TextColor, COLORREF BackgroundColor, COLORREF ActivatedBkgdColor, COLORREF VisitedBkgdColor,
				DWORD EditStyle, unsigned long ValidationType, UINT nID ) : CEdit()
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


TomEdit::~TomEdit()
{
	m_EditText.Empty();
}


BEGIN_MESSAGE_MAP(TomEdit, CWnd)
	//{{AFX_MSG_MAP(TomEdit)
	ON_WM_SETFOCUS()
	ON_WM_CHAR()
	ON_WM_KILLFOCUS()
	ON_WM_GETDLGCODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// Caution:		Since this function creates the object, it must be called before any
//				functions such as Invalidate(), etc., that manipulate an active window.
BOOL TomEdit::SetPosition( int x, int y, CWnd *pParentWnd )
{
	BOOL			bResult;
	CRect			EditRect;
	DWORD			WindowsEditStyle;

	WindowsEditStyle = ES_LEFT | ES_WANTRETURN | WS_CHILD | WS_VISIBLE;
	if ( m_EditStyle & CONTROL_MULTILINE )
		WindowsEditStyle |= ES_MULTILINE;
	if ( m_EditStyle & EDIT_VSCROLL )
		WindowsEditStyle |= ES_AUTOVSCROLL | WS_VSCROLL;
	if ( m_EditStyle & EDIT_BORDER )
		WindowsEditStyle |= WS_BORDER;
	if ( m_EditStyle & EDIT_READONLY )
		WindowsEditStyle |= ES_READONLY;
	EditRect.SetRect( x, y, x + m_EditWidth, y + m_EditHeight );
	bResult = Create( WindowsEditStyle, EditRect, pParentWnd, m_nObjectID );
	CreateSpecifiedFont();
	SetFont( &m_TextFont, FALSE );
	
	return bResult;
}


void TomEdit::Reinitialize()
{
	m_IdleBkgColor = m_OriginalIdleBkgColor;
	SetWindowText( "" );
}


void TomEdit::SetDecimalRange( double MinValue, double MaxValue, int DecimalDigitsDisplayed )
{
	if ( m_ValidationType & EDIT_VALIDATION_DECIMAL_RANGE )
		{
		m_MinimumDecimalValue = MinValue;
		m_MaximumDecimalValue = MaxValue;
		}
	m_DecimalDigitsDisplayed = DecimalDigitsDisplayed;
}


void TomEdit::ChangeStatus( DWORD ClearStatus, DWORD SetStatus )
{
	m_EditStyle &= ~ClearStatus;
	m_EditStyle |= SetStatus;

	if ( ClearStatus & CONTROL_VISIBLE )
		ModifyStyle( WS_VISIBLE, 0, 0 );
	else if ( SetStatus & CONTROL_VISIBLE )
		ModifyStyle( 0, WS_VISIBLE, 0 );

}


void TomEdit::HasBeenCompleted( BOOL bHasBeenCompleted )
{
	if ( bHasBeenCompleted )
		{
		if ( bMakeDumbButtons )
			{
			m_IdleBkgColor = m_OriginalIdleBkgColor;
			m_pCurrentBkgdBrush = &m_BkgdBrush;
			}
		else
			{
			m_IdleBkgColor = m_ActivatedBkgdColor;
			m_pCurrentBkgdBrush = &m_ActivatedBkgdBrush;
			}
		}
	else
		{
		m_IdleBkgColor = m_OriginalIdleBkgColor;
		m_pCurrentBkgdBrush = &m_BkgdBrush;
		}
}


BOOL TomEdit::CreateSpecifiedFont() 
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


void TomEdit::OnSetFocus( CWnd *pOldWnd )
{
	NMHDR			NotifyMessageHeader;

	CEdit::OnSetFocus( pOldWnd );
	m_bHasReceivedInput = TRUE;

	m_IdleBkgColor = m_ActivatedBkgdColor;
	m_pCurrentBkgdBrush = &m_ActivatedBkgdBrush;

	NotifyMessageHeader.code = WM_SETFOCUS;
	NotifyMessageHeader.hwndFrom = GetSafeHwnd();
	NotifyMessageHeader.idFrom = GetDlgCtrlID();
	GetParent() -> SendMessage( WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&NotifyMessageHeader );
	Invalidate( TRUE );
}


void TomEdit::OnChar( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	BOOL			bValidInput = TRUE;

	if ( nChar == 0x0D || nChar == 0x09 )
		{
		if ( m_pGroup != 0 )
			m_pGroup -> RespondToSelection( (void*)this );

		m_IdleBkgColor = m_ActivatedBkgdColor;
		m_pCurrentBkgdBrush = &m_ActivatedBkgdBrush;

		OnKillFocus( GetParent() );
		}
	else if ( nChar != 0x08 )
		{
		if ( m_ValidationType == EDIT_VALIDATION_NONE )
			bValidInput = TRUE;
		else
			{
			if ( m_ValidationType & EDIT_VALIDATION_NUMERIC )
				{
				if ( isprint( nChar ) && !isdigit( nChar ) )
					bValidInput = FALSE;
				}
			if ( m_ValidationType & EDIT_VALIDATION_DECIMAL )
				{
				if ( !( isdigit( nChar ) || nChar == '.' || nChar == '-' || nChar == '+' ) )
					bValidInput = FALSE;
				}
			if ( m_ValidationType & EDIT_VALIDATION_NUMERIC_RANGE )
				{
				}
			if ( m_ValidationType & EDIT_VALIDATION_SSN )
				{
				if ( isprint( nChar ) && !isdigit( nChar ) )
					bValidInput = FALSE;
				}
			}
		if ( bValidInput )
			{
			m_IdleBkgColor = m_ActivatedBkgdColor;
			m_pCurrentBkgdBrush = &m_ActivatedBkgdBrush;

			CEdit::OnChar( nChar, nRepCnt, nFlags );
			}
		else
			m_IdleBkgColor = m_SpecialBkgColor;
		}
	else
		CEdit::OnChar( nChar, nRepCnt, nFlags );

	Invalidate( FALSE );
}


void TomEdit::OnKillFocus( CWnd *pNewWnd )
{
	NMHDR				NotifyMessageHeader;
	BOOL				bValidInput = TRUE;
	char				NumberConvertedToText[ _CVTBUFSIZE ];
	int					nChars;
	int					nChar;
	BOOL				bNonNumericCharEncountered;
	double				DecimalValueEntered;
	char				NumberFormat[ 32 ];

	if ( m_ValidationType == EDIT_VALIDATION_NONE )
		bValidInput = TRUE;
	else
		{
		if ( m_ValidationType & EDIT_VALIDATION_NUMERIC )
			{
			GetWindowText( NumberConvertedToText, _CVTBUFSIZE );
			TrimBlanks( NumberConvertedToText );
			bNonNumericCharEncountered = FALSE;
			nChars = (int)strlen( NumberConvertedToText );
			for ( nChar = 0; nChar < nChars; nChar++ )
				if ( !isdigit( NumberConvertedToText[ nChar ] ) )
					bNonNumericCharEncountered = TRUE;
			if ( bNonNumericCharEncountered )
				{
				m_IdleBkgColor = m_SpecialBkgColor;
				SetWindowText( "Error" );
				bValidInput = FALSE;
				}
			}
		if ( m_ValidationType & EDIT_VALIDATION_DECIMAL )
			{
			GetWindowText( NumberConvertedToText, _CVTBUFSIZE );
			TrimBlanks( NumberConvertedToText );
			bNonNumericCharEncountered = FALSE;
			nChars = (int)strlen( NumberConvertedToText );
			for ( nChar = 0; nChar < nChars; nChar++ )
			if ( !( isdigit( NumberConvertedToText[ nChar ] ) || NumberConvertedToText[ nChar ] == '.' ||
							NumberConvertedToText[ nChar ] == '-' || NumberConvertedToText[ nChar ] == '+' ) )
				bNonNumericCharEncountered = TRUE;
			if ( bNonNumericCharEncountered )
				{
				m_IdleBkgColor = m_SpecialBkgColor;
				SetWindowText( "Error" );
				bValidInput = FALSE;
				}
			}
		if ( m_ValidationType & EDIT_VALIDATION_DECIMAL_RANGE )
			{
			GetWindowText( NumberConvertedToText, _CVTBUFSIZE );
			TrimBlanks( NumberConvertedToText );
			DecimalValueEntered = atof( NumberConvertedToText );
			if ( DecimalValueEntered < m_MinimumDecimalValue )
				DecimalValueEntered = m_MinimumDecimalValue;
			if ( DecimalValueEntered > m_MaximumDecimalValue )
				DecimalValueEntered = m_MaximumDecimalValue;
			if ( m_DecimalDigitsDisplayed == 0 )
				strcpy( NumberFormat, "%8.0f" );
			else if ( m_DecimalDigitsDisplayed == 1 )
				strcpy( NumberFormat, "%7.1f" );
			else if ( m_DecimalDigitsDisplayed == 2 )
				strcpy( NumberFormat, "%6.2f" );
			else if ( m_DecimalDigitsDisplayed == 3 )
				strcpy( NumberFormat, "%5.3f" );
			else
				strcpy( NumberFormat, "%16.8f" );
			sprintf( NumberConvertedToText, NumberFormat, DecimalValueEntered );
			TrimBlanks( NumberConvertedToText );
			SetWindowText( NumberConvertedToText );
			}
		if ( m_ValidationType & EDIT_VALIDATION_SSN )
			{
			GetWindowText( NumberConvertedToText, _CVTBUFSIZE );
			TrimBlanks( NumberConvertedToText );
			bNonNumericCharEncountered = FALSE;
			nChars = (int)strlen( NumberConvertedToText );
			for ( nChar = 0; nChar < nChars; nChar++ )
				if ( !isdigit( NumberConvertedToText[ nChar ] ) )
					bNonNumericCharEncountered = TRUE;
			if ( bNonNumericCharEncountered )
				{
				m_IdleBkgColor = m_SpecialBkgColor;
				SetWindowText( "Error" );
				bValidInput = FALSE;
				}
			}
		}
	CEdit::OnKillFocus( pNewWnd );

	NotifyMessageHeader.code = WM_KILLFOCUS;
	NotifyMessageHeader.hwndFrom = GetSafeHwnd();
	NotifyMessageHeader.idFrom = GetDlgCtrlID();
	GetParent() -> SendMessage( WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&NotifyMessageHeader );
}


UINT TomEdit::OnGetDlgCode()
{
	return DLGC_WANTALLKEYS;
}


