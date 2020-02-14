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


#define _CRTDBG_MAP_ALLOC 

// TomStatic
IMPLEMENT_DYNAMIC( TomStatic, TomControl )


TomStatic::TomStatic( char *pStaticText, int StaticWidth, int StaticHeight, int FontHeight, int FontWidth, int FontWeight,
				COLORREF TextColor, COLORREF BackgroundColor, COLORREF ActivatedBkgdColor, DWORD StaticStyle, UINT nID, char *pControlTipText )
			: TomControl( pStaticText, StaticWidth, StaticHeight, FontHeight, FontWidth, FontWeight,
								TextColor, BackgroundColor, ActivatedBkgdColor, StaticStyle, nID, pControlTipText )
{
}

TomStatic::~TomStatic()
{
}


BEGIN_MESSAGE_MAP( TomStatic, TomControl )
	//{{AFX_MSG_MAP( TomStatic )
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void TomStatic::OnPaint() 
{
	CPaintDC		dc(this);
	CRect			ClientRect;
	CRect			TextRect;
	CRect			PlotRect;
	CSize			TextSize;
	char			*pText;
	char			*pLineTerminator;
	int				nChars;
	int				nHistogramBar;
	double			HistogramBarHeight;
	double			HistogramBinWidth;
	double			HistogramBinLeftEdge;
	CBrush			BkgdBrush;
	CBrush			PlotBkgdBrush;
	CBrush			PlotBrush;
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
	char			TextLines[ 20 ][ 200 ];

	if ( m_ControlStyle & CONTROL_VISIBLE )
		{
		if ( m_ControlStyle & CONTROL_BITMAP )
			{
			CBitmap			ClientAreaBitmap;
			CBitmap			*pOldBitmap;
			CDC				MemoryDC;

			ClientAreaBitmap.LoadBitmap( m_BitmapID );					// Load the bitmap resource.
			MemoryDC.CreateCompatibleDC( &dc );
			pOldBitmap = MemoryDC.SelectObject( &ClientAreaBitmap );	// Select the bitmap into the memory DC.
			// Paint the bitmap by copying it from the memory DC to the display DC.
			dc.BitBlt( 0, 0, m_ControlWidth, m_ControlHeight, &MemoryDC, 0, 0, SRCCOPY );
			MemoryDC.SelectObject( pOldBitmap );
			}
		else
			{
			if ( m_ControlStyle & CONTROL_HISTOGRAM )
				{
				if ( m_pHistogramData != 0 && m_pHistogramData -> AverageViewableBinValue != 0 )
					{
					GetClientRect( ClientRect );
					PlotBkgdBrush.CreateSolidBrush( m_IdleBkgColor );
					dc.FillRect( &ClientRect, &PlotBkgdBrush );		
					PlotBrush.CreateSolidBrush( m_TextColor );
					PlotRect.bottom = ClientRect.bottom;
					HistogramBinWidth = (double)ClientRect.Width() / 128.0;
					HistogramBinLeftEdge = (double)ClientRect.left;
					for ( nHistogramBar = 0; nHistogramBar < 128; nHistogramBar++ )
						{
						HistogramBarHeight = (double)m_pHistogramData -> ViewableHistogramArray[ nHistogramBar ] / ( 4.0 * m_pHistogramData -> AverageViewableBinValue );
						PlotRect.top = PlotRect.bottom - (long)( (double)ClientRect.Height() * HistogramBarHeight );
						if ( PlotRect.top < ClientRect.top )
							PlotRect.top = ClientRect.top;
						PlotRect.left = (long)HistogramBinLeftEdge;
						PlotRect.right = (long)( HistogramBinLeftEdge + HistogramBinWidth );
						dc.FillRect( &PlotRect, &PlotBrush );
						HistogramBinLeftEdge += HistogramBinWidth;
						}
					}
				}
			else
				{
				if ( m_ControlStyle & CONTROL_MULTILINE )
					{
					pText = m_ControlText;
					do
						{
						pLineTerminator = strchr( pText, '\n' );
						if ( pLineTerminator != 0 )
							nChars = (int)( (DWORD_PTR)pLineTerminator - (DWORD_PTR)pText );
						else
							nChars = (int)strlen( pText );
						strcpy( TextLines[ nTextLines ], "" );
						if ( nChars > 0 )
							strncat( TextLines[ nTextLines ], pText, nChars );
						nTextLines++;
						if ( pLineTerminator != 0 )
							pText = pLineTerminator + 1;
						}
					while ( pLineTerminator != 0 && nTextLines < 20 );
					}
				else
					{
					strcpy( TextLines[ 0 ], "" );
					strncat( TextLines[ 0 ], m_ControlText, 99 );
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
			
				if ( m_ControlStyle & CONTROL_BACKGROUND_TRANSPARENT )
					dc.SetBkMode( TRANSPARENT );
				else
					dc.FillRect( &TextRect, &BkgdBrush );		

				pTextString = m_ControlText;
				TextFormat = 0;
				if ( m_ControlStyle & CONTROL_TEXT_TOP_JUSTIFIED )
					{
					nStartPos = TextRect.top;
					if ( ( m_ControlStyle & CONTROL_MULTILINE ) == 0 )
						TextFormat |= DT_TOP;
					}
				else if ( m_ControlStyle & CONTROL_TEXT_BOTTOM_JUSTIFIED )
					{
					nStartPos = TextRect.bottom - nTextLines * TextSize.cy - 2;
					if ( ( m_ControlStyle & CONTROL_MULTILINE ) == 0 )
						TextFormat |= DT_VCENTER;
					}
				else if ( m_ControlStyle & CONTROL_TEXT_VERTICALLY_CENTERED )
					{
					nStartPos = ( TextRect.Height() - nTextLines * TextSize.cy ) / 2 - 1;
					if ( ( m_ControlStyle & CONTROL_MULTILINE ) == 0 )
						TextFormat |= DT_TOP;
					}
				else
					{
					nStartPos = ( TextRect.Height() - ( nTextLines + 1 ) * TextSize.cy ) / 2;
					TextFormat |= DT_TOP;
					}

				if ( m_ControlStyle & CONTROL_TEXT_RIGHT_JUSTIFIED )
					TextFormat |= DT_RIGHT;
				else if ( m_ControlStyle & CONTROL_TEXT_LEFT_JUSTIFIED )
					TextFormat |= DT_LEFT;
				else if ( m_ControlStyle & CONTROL_TEXT_HORIZONTALLY_CENTERED )
					TextFormat |= DT_CENTER;
				else
					TextFormat |= DT_LEFT;

				if ( ( m_ControlStyle & CONTROL_CLIP ) == 0 )
					TextFormat |= DT_NOCLIP;
				if ( m_ControlStyle & CONTROL_MULTILINE )
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
		}

	return;
}



