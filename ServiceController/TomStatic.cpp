// TomStatic.cpp : Implementation file for the class of static text object
//	controls for use in populating a user interface window.
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
#include "TomStatic.h"

/////////////////////////////////////////////////////////////////////////////
// TomStatic

// This constructor requires that the STATIC_BITMAP bit NOT be set in the StaticStyle parameter.
TomStatic::TomStatic( char *pStaticText, int StaticWidth, int StaticHeight, int FontHeight, int FontWidth, int FontWeight,
				COLORREF TextColor, COLORREF BackgroundColor, COLORREF ActivatedBkgdColor, DWORD StaticStyle, UINT nID )
{
	m_StaticText = pStaticText;
	m_StaticWidth = StaticWidth;
	m_StaticHeight = StaticHeight;
	m_FontHeight = FontHeight;
	m_FontWidth = FontWidth;
	m_FontWeight = FontWeight * 100;			// FontWeight: 1 through 9
	m_TextColor = TextColor;
	m_OriginalIdleBkgColor = BackgroundColor;
	m_IdleBkgColor = BackgroundColor;
	m_ActivatedBkgdColor = ActivatedBkgdColor;
	m_StaticStyle = StaticStyle;
	m_nObjectID = nID;
}


// This constructor requires that the STATIC_BITMAP bit be set in the StaticStyle parameter.
TomStatic::TomStatic( UINT StaticBitmapID, int StaticBitmapWidth, int StaticBitmapHeight, DWORD StaticStyle, UINT nID )
{
	m_BitmapID = StaticBitmapID;
	m_StaticWidth = StaticBitmapWidth;
	m_StaticHeight = StaticBitmapHeight;
	m_StaticStyle = StaticStyle;
	m_nObjectID = nID;
	m_StaticText = 0;
}


TomStatic::~TomStatic()
{
}


BEGIN_MESSAGE_MAP(TomStatic, CWnd)
	//{{AFX_MSG_MAP(TomStatic)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// Caution:		Since this function creates the object, it must be called before any
//				functions such as Invalidate(), etc., that manipulate an active window.
BOOL TomStatic::SetPosition( int x, int y, CWnd* pParentWnd )
{
	BOOL			bResult;
	CRect			StaticRect;
	DWORD			WindowsStaticStyle;

	WindowsStaticStyle = WS_CHILD | WS_DISABLED | WS_VISIBLE;
	StaticRect.SetRect( x, y, x + m_StaticWidth, y + m_StaticHeight );
	bResult = Create( NULL, m_StaticText, WindowsStaticStyle, StaticRect, pParentWnd, m_nObjectID );
	
	return bResult;
}


void TomStatic::ChangeStatus( DWORD ClearStatus, DWORD SetStatus )
{
	m_StaticStyle &= ~ClearStatus;
	m_StaticStyle |= SetStatus;
}


void TomStatic::HasBeenCompleted( BOOL bHasBeenCompleted )
{
	if ( ( m_StaticStyle & STATIC_BITMAP ) == 0 )
		{
		if ( bHasBeenCompleted )
			m_IdleBkgColor = m_ActivatedBkgdColor;
		else
			m_IdleBkgColor = m_OriginalIdleBkgColor;
		}
}


BOOL TomStatic::IsCompleted()
{
	if ( ( m_StaticStyle & STATIC_BITMAP ) == 0 )
		return ( m_IdleBkgColor == m_ActivatedBkgdColor );
	else
		return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// TomStatic message handlers

bool TomStatic::CreateSpecifiedFont() 
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

void TomStatic::OnPaint() 
{
	CPaintDC		dc(this);
	CRect			ClientRect;
	CRect			TextRect;
	CSize			TextSize;
	char			*pText;
	char			*pLineTerminator;
	int				nChars;
	CBrush			BkgdBrush;
	COLORREF		PrevBkgdColor;
	COLORREF		PrevTextColor;
	CFont			*PrevFont;
	CSize			FullTextExtent;
	CSize			ConsiseTextExtent;
	CSize			ShortTextExtent;
	char			*pTextString;
	unsigned int	TextFormat;
	int				nStartPos;
	int				nTextLines = 0;
	int				nTextLine;
	char			TextLines[ 10 ][ 200 ];

	if ( m_StaticStyle & STATIC_VISIBLE )
		{
		if ( m_StaticStyle & STATIC_BITMAP )
			{
			CBitmap			ClientAreaBitmap;
			CBitmap			*pOldBitmap;
			CDC				MemoryDC;

			ClientAreaBitmap.LoadBitmap( m_BitmapID );					// Load the bitmap resource.
			MemoryDC.CreateCompatibleDC( &dc );
			pOldBitmap = MemoryDC.SelectObject( &ClientAreaBitmap );	// Select the bitmap into the memory DC.
			// Paint the bitmap by copying it from the memory DC to the display DC.
			dc.BitBlt( 0, 0, m_StaticWidth, m_StaticHeight, &MemoryDC, 0, 0, SRCCOPY );
			MemoryDC.SelectObject( pOldBitmap );
			}
		else
			{
			if ( m_StaticStyle & STATIC_MULTILINE )
				{
				pText = m_StaticText;
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
				strncat( TextLines[ 0 ], m_StaticText, 99 );
				nTextLines = 1;
				}
			CreateSpecifiedFont();
			PrevFont = dc.SelectObject( &m_TextFont );
			TextSize = dc.GetOutputTextExtent( TextLines[ 0 ], (int)strlen( TextLines[ 0 ] ) );

			GetClientRect( ClientRect );
			TextRect.SetRect( 0, 0, ClientRect.right, ClientRect.bottom );

			BkgdBrush.CreateSolidBrush( m_IdleBkgColor );
			PrevBkgdColor = dc.SetBkColor( m_IdleBkgColor );
			PrevTextColor = dc.SetTextColor( m_TextColor );
			
			if ( m_StaticStyle & STATIC_BACKGROUND_TRANSPARENT )
				dc.SetBkMode( TRANSPARENT );
			else
				dc.FillRect( &TextRect, &BkgdBrush );		

			pTextString = m_StaticText;

			TextFormat = 0;
			if ( m_StaticStyle & STATIC_TEXT_TOP_JUSTIFIED )
				{
				nStartPos = TextRect.top;
				if ( ( m_StaticStyle & STATIC_MULTILINE ) == 0 )
					TextFormat |= DT_TOP;
				}
			else if ( m_StaticStyle & STATIC_TEXT_BOTTOM_JUSTIFIED )
				{
				nStartPos = TextRect.bottom - nTextLines* TextSize.cy - 2;
				if ( ( m_StaticStyle & STATIC_MULTILINE ) == 0 )
					TextFormat |= DT_VCENTER;
				}
			else if ( m_StaticStyle & STATIC_TEXT_VERTICALLY_CENTERED )
				{
				nStartPos = ( TextRect.Height() - nTextLines * TextSize.cy ) / 2 - 1;
				if ( ( m_StaticStyle & STATIC_MULTILINE ) == 0 )
					{
					TextFormat |= DT_TOP;
					}
				}
			else
				{
				nStartPos = ( TextRect.Height() - ( nTextLines + 1 ) * TextSize.cy ) / 2;
				TextFormat |= DT_TOP;
				}

			if ( m_StaticStyle & STATIC_TEXT_RIGHT_JUSTIFIED )
				TextFormat |= DT_RIGHT;
			else if ( m_StaticStyle & STATIC_TEXT_LEFT_JUSTIFIED )
				TextFormat |= DT_LEFT;
			else if ( m_StaticStyle & STATIC_TEXT_HORIZONTALLY_CENTERED )
				TextFormat |= DT_CENTER;
			else
				TextFormat |= DT_LEFT;

			if ( ( m_StaticStyle & STATIC_CLIP ) == 0 )
				TextFormat |= DT_NOCLIP;
			if ( m_StaticStyle & STATIC_MULTILINE )
				TextFormat |= DT_WORDBREAK;

			for ( nTextLine = 0; nTextLine < nTextLines; nTextLine++ )
				{
				TextRect.SetRect( 0, 0, ClientRect.right, ClientRect.bottom );
				if ( nStartPos <= 0 )
					nStartPos = 1;
				TextRect.top = nStartPos + TextSize.cy * nTextLine;
				TextRect.bottom = nStartPos + TextSize.cy * ( nTextLine + 1 );
				pText = TextLines[ nTextLine ];
				dc.DrawText( pText, (int)strlen( pText ), TextRect, TextFormat );
				}

			dc.SelectObject( PrevFont );
			m_TextFont.DeleteObject();
			dc.SetTextColor( PrevTextColor );
			dc.SetBkColor( PrevBkgdColor );
			}
		}

	return;
}


