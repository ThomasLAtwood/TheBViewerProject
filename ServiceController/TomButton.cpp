// TomButton.cpp : Implementation file for the TomButton class of
//	enhanced button controls for use in a user interface window.
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
#include "TomButton.h"

/////////////////////////////////////////////////////////////////////////////
// TomButton

TomButton::TomButton( char *pButtonText, int ButtonWidth, int ButtonHeight, int FontHeight, int FontWidth, int FontWeight,
						COLORREF TextColor, COLORREF BackgroundColor, COLORREF ActivatedBkgdColor, COLORREF VisitedBkgdColor,
						DWORD ButtonStyle, UINT nID )
{
	m_ButtonText = pButtonText;
	m_ButtonWidth = ButtonWidth;
	m_ButtonHeight = ButtonHeight;
	m_FontHeight = FontHeight;
	m_FontWidth = FontWidth;
	m_FontWeight = FontWeight * 100;			// FontWeight: 1 through 9
	m_TextColor = TextColor;
	m_OriginalIdleBkgColor = BackgroundColor;
	m_IdleBkgColor = BackgroundColor;
	m_ActivatedBkgdColor = ActivatedBkgdColor;
	m_VisitedBkgdColor = VisitedBkgdColor;
	m_ButtonStyle = ButtonStyle;
	m_nObjectID = nID;
	m_pGroup = 0;
	RecomputePressedColor();
	m_ToggleState = BUTTON_OFF;
	m_SemanticState = BUTTON_UNTOUCHED;
	m_SpecialBkgColor		= COLORREF( RGB( 255, 0, 0 ) );
	m_DisabledBkgndColor	= GetSysColor( COLOR_BTNFACE );
	m_Light					= GetSysColor( COLOR_3DLIGHT );
	m_Highlight				= GetSysColor( COLOR_BTNHIGHLIGHT );
	m_Shadow				= GetSysColor( COLOR_BTNSHADOW );
	m_DarkShadow			= GetSysColor( COLOR_3DDKSHADOW );
	m_EngageSpecialState = false;
}


TomButton::~TomButton()
{
}


// Caution:		Since this function creates the object, it must be called before any
//				functions such as Invalidate(), etc., that manipulate an active window.
BOOL TomButton::SetPosition( int x, int y, CWnd* pParentWnd )
{
	BOOL			bResult;
	CRect			ButtonRect;
	DWORD			WindowsButtonStyle;

	WindowsButtonStyle = WS_CHILD | WS_VISIBLE;
	if ( m_ButtonStyle & BUTTON_PUSHBUTTON )
		WindowsButtonStyle |= BS_PUSHBUTTON | BS_OWNERDRAW;
	else if ( m_ButtonStyle & BUTTON_CHECKBOX )
		WindowsButtonStyle |= BS_PUSHBUTTON | BS_OWNERDRAW;
	ButtonRect.SetRect( x, y, x + m_ButtonWidth, y + m_ButtonHeight );
	bResult = Create( m_ButtonText, WindowsButtonStyle, ButtonRect, pParentWnd, m_nObjectID );
	
	return bResult;
}


void TomButton::Reinitialize()
{
	m_IdleBkgColor = m_OriginalIdleBkgColor;
	m_ToggleState = BUTTON_OFF;
}


void TomButton::ChangeStatus( DWORD ClearStatus, DWORD SetStatus )
{
	m_ButtonStyle &= ~ClearStatus;
	m_ButtonStyle |= SetStatus;
}



BEGIN_MESSAGE_MAP(TomButton, CButton)
	//{{AFX_MSG_MAP(TomButton)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


bool TomButton::CreateSpecifiedFont() 
{
	bool			Ok;

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


void TomButton::RecomputePressedColor() 
{
	unsigned long		RedAdjust;
	unsigned long		GreenAdjust;
	unsigned long		BlueAdjust;

	RedAdjust	= ( ( m_IdleBkgColor & 0x000000ff ) + ( ( 0x000000ff - ( m_IdleBkgColor & 0x000000ff ) ) >> 1 ) ) & 0x000000ff;
	GreenAdjust	= ( ( m_IdleBkgColor & 0x0000ff00 ) + ( ( 0x0000ff00 - ( m_IdleBkgColor & 0x0000ff00 ) ) >> 1 ) ) & 0x0000ff00;
	BlueAdjust	= ( ( m_IdleBkgColor & 0x00ff0000 ) + ( ( 0x00ff0000 - ( m_IdleBkgColor & 0x00ff0000 ) ) >> 1 ) ) & 0x00ff0000;
	m_PressedBkgColor = RedAdjust | GreenAdjust | BlueAdjust;
}


/////////////////////////////////////////////////////////////////////////////
// TomButton message handlers

void TomButton::OnLButtonDown( UINT nFlags, CPoint point ) 
{
	if ( ( m_ButtonStyle & BUTTON_FROZEN ) == 0 )
		{
		m_IdleBkgColor = m_ActivatedBkgdColor;
		CButton::OnLButtonDown( nFlags, point );
		}
	else
		{
		m_IdleBkgColor = m_SpecialBkgColor;
		Invalidate( TRUE );
		}

}


void TomButton::SetCheckBoxColor() 
{
	if ( m_ToggleState == BUTTON_OFF )
		m_IdleBkgColor = m_VisitedBkgdColor;
	else
		m_IdleBkgColor = m_ActivatedBkgdColor;
	RecomputePressedColor();
}


void TomButton::OnLButtonUp( UINT nFlags, CPoint point ) 
{
	if ( ( m_ButtonStyle & BUTTON_FROZEN ) == 0 )
		{
		if ( m_ToggleState == BUTTON_OFF )
			m_ToggleState = BUTTON_ON;
		else
			m_ToggleState = BUTTON_OFF;

		if ( m_ButtonStyle & BUTTON_CHECKBOX )
			SetCheckBoxColor();
		else if ( m_ButtonStyle & BUTTON_PUSHBUTTON )
			m_IdleBkgColor = m_VisitedBkgdColor;
		RecomputePressedColor();
		CButton::OnLButtonUp( nFlags, point );
		}
	else
		{
		m_IdleBkgColor = m_OriginalIdleBkgColor;
		Invalidate( TRUE );
		}
}


// This function can be used to reload a historical state for this button.
void TomButton::HasBeenPressed( BOOL bHasBeenPressed )
{
	if ( m_ButtonStyle & BUTTON_PUSHBUTTON )
		{
		if ( bHasBeenPressed )
			m_IdleBkgColor = m_VisitedBkgdColor;
		else
			m_IdleBkgColor = m_OriginalIdleBkgColor;
		RecomputePressedColor();
		}
}


void TomButton::TurnOnSpecialState() 
{
	m_EngageSpecialState = true;
}


void TomButton::TurnOffSpecialState() 
{
	m_EngageSpecialState = false;
}


void TomButton::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct ) 
{
	CDC				*pDC;
	CRect			FocusRect;
	CRect			TextRect;
	CRect			OffsetTextRect;
	CRect			ButtonRect;
	UINT			ButtonState;
	CFont			*PrevFont;
	char			*pTextString;
	COLORREF		Color;
	unsigned		nWidth;
	int				nCheckWidth;
	
	if ( m_ButtonStyle & BUTTON_VISIBLE )
		{
		pDC = CDC::FromHandle( lpDrawItemStruct -> hDC );
		ButtonState = lpDrawItemStruct -> itemState;
		
		CreateSpecifiedFont();
		PrevFont = pDC -> SelectObject( &m_TextFont );

		FocusRect.CopyRect( &lpDrawItemStruct -> rcItem ); 
		ButtonRect.CopyRect( &lpDrawItemStruct -> rcItem ); 
		
		TextRect = ButtonRect;
		TextRect.OffsetRect( -1, -1 );

		OffsetTextRect = TextRect;
		OffsetTextRect.OffsetRect( 1, 1 );
		
		nWidth = ( FocusRect.bottom - FocusRect.top ) / 20;
		if ( nWidth == 0 )
			nWidth = 1;

		// Set the focus rectangle just outside the border decoration.
		FocusRect.left += nWidth;
		FocusRect.top += nWidth;
		FocusRect.right -= nWidth;
		FocusRect.bottom -= nWidth;
		
		pTextString = m_ButtonText;

		if ( ButtonState & ODS_DISABLED )
			DrawFilledRect( pDC, ButtonRect, m_DisabledBkgndColor );
		else
			{
			if ( m_EngageSpecialState )
				DrawFilledRect( pDC, ButtonRect, m_SpecialBkgColor );
			else if ( ButtonState & ODS_SELECTED )
				DrawFilledRect( pDC, ButtonRect, m_PressedBkgColor );
			else
				{
				// Check semantic considerations first:  they override default behavior.
				if ( m_SemanticState == BUTTON_COMPLETED )
					DrawFilledRect( pDC, ButtonRect, m_ActivatedBkgdColor );
				else if ( m_SemanticState == BUTTON_TOUCHED )
					DrawFilledRect( pDC, ButtonRect, m_VisitedBkgdColor );
				else
					DrawFilledRect( pDC, ButtonRect, m_IdleBkgColor );
				}
			}
		
		if ( ButtonState & ODS_SELECTED )
			DrawFrame( pDC, ButtonRect, BUTTON_IN );
		else
			{
			if ( ( ButtonState & ODS_DEFAULT ) || ( ButtonState & ODS_FOCUS ) )
				DrawFrame( pDC, ButtonRect, BUTTON_OUT | BUTTON_BLACK_BORDER );			
			else
				DrawFrame( pDC, ButtonRect, BUTTON_OUT | BUTTON_BLACK_BORDER );
			}
		
		if ( m_ButtonStyle & ( BUTTON_PUSHBUTTON | BUTTON_CHECKBOX ) )
			{
			if (ButtonState & ODS_DISABLED)
				{
				DrawButtonText( pDC, OffsetTextRect, pTextString, COLORREF( RGB( 255, 255, 255 ) ) );
				DrawButtonText( pDC, TextRect, pTextString, COLORREF( RGB( 128, 128, 128 ) ) );
				}
			else
				{
				if ( ButtonState & ODS_SELECTED )
					DrawButtonText( pDC, OffsetTextRect, pTextString, m_TextColor );
				else
					DrawButtonText( pDC, TextRect, pTextString, m_TextColor );
				}
			}
		if ( m_ButtonStyle & BUTTON_CHECKBOX )
			{
			CRect				CheckMarkRect;

			Color = RGB( 0, 0, 255 );		// Blue.
			if ( m_ToggleState == BUTTON_ON )
				{
				nWidth = ( FocusRect.bottom - FocusRect.top ) / 10;
				if ( nWidth <= 1 )
					nWidth = 2;
				CheckMarkRect = TextRect;
				CheckMarkRect.left += nWidth;
				CheckMarkRect.top += nWidth;
				CheckMarkRect.right -= nWidth;
				CheckMarkRect.bottom -= nWidth;
				nCheckWidth = CheckMarkRect.bottom - CheckMarkRect.top;
				if ( CheckMarkRect.right - CheckMarkRect.left < nCheckWidth )
					nCheckWidth = CheckMarkRect.right - CheckMarkRect.left;
				DrawLine( pDC, CheckMarkRect.left, ( CheckMarkRect.top + CheckMarkRect.bottom ) / 2,
								CheckMarkRect.left + nCheckWidth / 3, CheckMarkRect.bottom, nWidth, Color );
				DrawLine( pDC, CheckMarkRect.left + nCheckWidth / 3, CheckMarkRect.bottom, 
								CheckMarkRect.left + nCheckWidth, CheckMarkRect.top, nWidth, Color );
				}
			}
		if ( ButtonState & ODS_FOCUS )
			DrawFocusRect( lpDrawItemStruct -> hDC, (LPRECT)&FocusRect );
		pDC -> SelectObject( PrevFont );
		m_TextFont.DeleteObject();
		}
}


void TomButton::DrawFrame( CDC *pDC, CRect ButtonRect, int ButtonState )
{
	COLORREF				Color;
	
	if ( ButtonState & BUTTON_BLACK_BORDER )
		{
		Color = RGB(0, 0, 0);		// Black.
		DrawLine( pDC, ButtonRect.left, ButtonRect.top, ButtonRect.right, ButtonRect.top, 1, Color );		// Across top
		DrawLine( pDC, ButtonRect.left, ButtonRect.top, ButtonRect.left,  ButtonRect.bottom, 1, Color );	// Down left
		DrawLine( pDC, ButtonRect.left, ButtonRect.bottom - 1, ButtonRect.right,
														ButtonRect.bottom - 1, 1, Color );					// Across bottom
		DrawLine( pDC, ButtonRect.right - 1, ButtonRect.top, ButtonRect.right - 1,
														ButtonRect.bottom, 1, Color );						// Down right
		ButtonRect.InflateRect( -1, -1 );
		}
	
	if ( ButtonState & BUTTON_OUT )
		{
		Color = m_Highlight;
		DrawLine( pDC, ButtonRect.left, ButtonRect.top, ButtonRect.right, ButtonRect.top, 1, Color );		// Across top
		DrawLine( pDC, ButtonRect.left, ButtonRect.top, ButtonRect.left, ButtonRect.bottom, 1, Color );		// Down left

		Color = m_DarkShadow;
		DrawLine( pDC, ButtonRect.left, ButtonRect.bottom - 1, ButtonRect.right,
														ButtonRect.bottom - 1, 1, Color );					// Across bottom
		DrawLine( pDC, ButtonRect.right - 1, ButtonRect.top, ButtonRect.right - 1,
														ButtonRect.bottom, 1, Color );						// Down right
		ButtonRect.InflateRect( -1, -1 );

		Color = m_Light;
		DrawLine( pDC, ButtonRect.left, ButtonRect.top, ButtonRect.right, ButtonRect.top, 1, Color );		// Across top
		DrawLine( pDC, ButtonRect.left, ButtonRect.top, ButtonRect.left,  ButtonRect.bottom, 1, Color );	// Down left
		
		Color = m_Shadow;
		DrawLine( pDC, ButtonRect.left, ButtonRect.bottom - 1, ButtonRect.right,
														ButtonRect.bottom - 1, 1, Color );					// Across bottom
		DrawLine( pDC, ButtonRect.right - 1, ButtonRect.top, ButtonRect.right - 1,
														ButtonRect.bottom, 1, Color );						// Down right
		}
	
	if ( ButtonState & BUTTON_IN )
		{
		Color = m_DarkShadow;
		DrawLine( pDC, ButtonRect.left, ButtonRect.top, ButtonRect.right, ButtonRect.top, 1, Color );		// Across top
		DrawLine( pDC, ButtonRect.left, ButtonRect.top, ButtonRect.left, ButtonRect.bottom, 1, Color );		// Down left
		DrawLine( pDC, ButtonRect.left, ButtonRect.bottom - 1, ButtonRect.right,
														ButtonRect.bottom - 1, 1, Color );					// Across bottom
		DrawLine( pDC, ButtonRect.right - 1, ButtonRect.top, ButtonRect.right - 1,
														ButtonRect.bottom, 1, Color );						// Down right
		ButtonRect.InflateRect( -1, -1 );
		
		Color = m_Shadow;
		DrawLine( pDC, ButtonRect.left, ButtonRect.top, ButtonRect.right, ButtonRect.top, 1, Color );		// Across top
		DrawLine( pDC, ButtonRect.left, ButtonRect.top, ButtonRect.left, ButtonRect.bottom, 1, Color );		// Down left
		DrawLine( pDC, ButtonRect.left, ButtonRect.bottom - 1, ButtonRect.right,
														ButtonRect.bottom - 1, 1, Color );					// Across bottom
		DrawLine( pDC, ButtonRect.right - 1, ButtonRect.top, ButtonRect.right - 1,
														ButtonRect.bottom, 1, Color );						// Down right
		}
}


void TomButton::DrawFilledRect( CDC *pDC, CRect ButtonRect, COLORREF Color )
{
	CBrush				SolidBrush;
	
	SolidBrush.CreateSolidBrush( Color );
	pDC -> FillRect( ButtonRect, &SolidBrush );
}


void TomButton::DrawLine( CDC *pDC, long xStart, long yStart, long xEnd, long yEnd, int nWidth, COLORREF Color )
{
	CPen				DrawingPen;
	CPen				*PrevPen;
	
	DrawingPen.CreatePen( PS_SOLID, nWidth, Color );
	PrevPen = pDC -> SelectObject( &DrawingPen );

	pDC -> MoveTo( xStart, yStart );
	pDC -> LineTo( xEnd, yEnd );

	pDC -> SelectObject( PrevPen );
    DrawingPen.DeleteObject();	
}


void TomButton::DrawButtonText( CDC *pDC, CRect ButtonRect, char *pTextString, COLORREF TextColor )
{
	DWORD				WindowStyle = GetWindowLong( this -> m_hWnd, GWL_STYLE );
	int					nTextLines = 0;
	int					nTextLine;
	char				*pText;
	char				*pLineTerminator;
	int					nChars;
	char				TextLines[ 10 ][ 100 ];
	CSize				TextSize;
    COLORREF			PrevColor;
	int					nStartPos;
	CRect				TextRect;
	unsigned			TextFormat;

	nTextLines = 0;
	if ( m_ButtonStyle & BUTTON_MULTILINE )
		{
		pText = pTextString;
		do
			{
			pLineTerminator = strchr( pText, '\n' );
			if ( pLineTerminator != 0 )
				nChars = (int)( (DWORD_PTR)pLineTerminator - (DWORD_PTR)pText );
			else
				nChars = (int)strlen( pText );
			strcpy( TextLines[ nTextLines ], "" );
			strncat( TextLines[ nTextLines ], pText, nChars );
			nTextLines++;
			if ( pLineTerminator != 0 )
				pText = pLineTerminator + 1;
			}
		while ( pLineTerminator != 0 );
		}
	else
		{
		strcpy( TextLines[ 0 ], "" );
		strncat( TextLines[ 0 ], pTextString, 99 );
		nTextLines = 1;
		}
		
	TextSize = pDC -> GetOutputTextExtent( pTextString, (int)strlen( pTextString ) );
	PrevColor = pDC -> SetTextColor( TextColor );
	pDC -> SetBkMode( TRANSPARENT );
	nStartPos = ( ButtonRect.Height() - ( nTextLines + 1 ) * TextSize.cy ) / 2 - 1;
	TextFormat = 0;
	if ( m_ButtonStyle & BUTTON_MULTILINE )
		TextFormat |= DT_WORDBREAK;
	if ( ( m_ButtonStyle & BUTTON_CLIP ) == 0 )
			TextFormat |= DT_NOCLIP;

	if ( m_ButtonStyle & BUTTON_TEXT_TOP_JUSTIFIED )
		nStartPos = ButtonRect.top + 2;
	else if ( m_ButtonStyle & BUTTON_TEXT_BOTTOM_JUSTIFIED )
		nStartPos = ButtonRect.bottom - nTextLines* TextSize.cy - 2;
	else if ( m_ButtonStyle & BUTTON_TEXT_VERTICALLY_CENTERED )
		nStartPos = ( ButtonRect.Height() - nTextLines * TextSize.cy ) / 2 - 1;

	if ( m_ButtonStyle & BUTTON_TEXT_RIGHT_JUSTIFIED )
		TextFormat |= DT_RIGHT;
	else if ( m_ButtonStyle & BUTTON_TEXT_LEFT_JUSTIFIED )
		TextFormat |= DT_LEFT;
	else if ( m_ButtonStyle & BUTTON_TEXT_HORIZONTALLY_CENTERED )
		TextFormat |= DT_CENTER;
	else
		TextFormat |= DT_LEFT;

	for ( nTextLine = 0; nTextLine < nTextLines; nTextLine++ )
		{
		TextRect = ButtonRect;
		TextRect.DeflateRect( 3, 0, 3, 0 );
		TextRect.top = nStartPos + TextSize.cy * nTextLine;
		TextRect.bottom = nStartPos + TextSize.cy * ( nTextLine + 1 );
		pText = TextLines[ nTextLine ];
		pDC -> DrawText( pText, (int)strlen( pText ), TextRect, TextFormat );
		}
	pDC -> SetTextColor( PrevColor );
}


