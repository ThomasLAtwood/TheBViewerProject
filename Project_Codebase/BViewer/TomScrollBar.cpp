// TomScrollBar.cpp : implementation file
//


#include "stdafx.h"
#include "TomScrollBar.h"

#define _CRTDBG_MAP_ALLOC 

/////////////////////////////////////////////////////////////////////////////
// TomScrollBar

TomScrollBar::TomScrollBar( char *pButtonText, int ScrollBarWidth, int ScrollBarHeight, int MinScrollPosition, int MaxScrollPosition,
						int PagingSize, int FontHeight, int FontWidth, int FontWeight,
						COLORREF TextColor, COLORREF BackgroundColor, COLORREF ActivatedBkgdColor, COLORREF VisitedBkgdColor,
						DWORD ScrollBarStyle, UINT nID )
{
	m_ButtonText = pButtonText;
	m_ScrollBarWidth = ScrollBarWidth;
	m_ScrollBarHeight = ScrollBarHeight;
	m_MinScrollPosition = MinScrollPosition;
	m_MaxScrollPosition = MaxScrollPosition;
	m_PagingSize = PagingSize;
	m_FontHeight = FontHeight;
	m_FontWidth = FontWidth;
	m_FontWeight = FontWeight * 100;			// FontWeight: 1 through 9
	m_TextColor = TextColor;
	m_OriginalIdleBkgColor = BackgroundColor;
	m_IdleBkgColor = BackgroundColor;
	m_ActivatedBkgdColor = ActivatedBkgdColor;
	m_VisitedBkgdColor = VisitedBkgdColor;
	m_ScrollBarStyle = ScrollBarStyle;
	m_nObjectID = nID;

	RecomputePressedColor();

	m_SpecialBkgColor		= COLORREF( RGB( 255, 0, 0 ) );

	m_DisabledBkgndColor	= GetSysColor( COLOR_BTNFACE );
	m_Light					= GetSysColor( COLOR_3DLIGHT );
	m_Highlight				= GetSysColor( COLOR_BTNHIGHLIGHT );
	m_Shadow				= GetSysColor( COLOR_BTNSHADOW );
	m_DarkShadow			= GetSysColor( COLOR_3DDKSHADOW );

	m_EngageSpecialState = false;
	m_BkgdBrush.CreateSolidBrush( m_IdleBkgColor );
	}


TomScrollBar::~TomScrollBar()
{
//	m_ButtonText.Empty();
}


// Caution:		Since this function creates the object, it must be called before any
//				functions such as Invalidate(), etc., that manipulate an active window.
BOOL TomScrollBar::SetPosition( int x, int y, CWnd* pParentWnd )
{
	BOOL			bResult;
	CRect			ScrollBarRect;
	DWORD			WindowsButtonStyle;
	SCROLLINFO		ScrollInfo;
	SCROLLBARINFO	ScrollBarInfo;

	WindowsButtonStyle = WS_CHILD | WS_VISIBLE;
	if ( m_ScrollBarStyle & SCROLLBAR_HORIZONTAL )
		WindowsButtonStyle |= SBS_HORZ;
	else if ( m_ScrollBarStyle & SCROLLBAR_VERTICAL )
		WindowsButtonStyle |= SBS_VERT;
	ScrollBarRect.SetRect( x, y, x + m_ScrollBarWidth, y + m_ScrollBarHeight );
	bResult = Create( WindowsButtonStyle, ScrollBarRect, pParentWnd, m_nObjectID );
	if ( bResult != 0 )
		{
		// Set the ScrollBar information.
		ScrollInfo.cbSize = sizeof(SCROLLINFO);
		GetScrollInfo( &ScrollInfo, SIF_ALL );
		ScrollInfo.nMax = m_MaxScrollPosition;
		ScrollInfo.nMin = m_MinScrollPosition;
		ScrollInfo.fMask = SIF_ALL;
		ScrollInfo.nPage = m_PagingSize;
		ScrollInfo.nPos = 0;
		ScrollInfo.nTrackPos = 0;
		SetScrollInfo( &ScrollInfo, FALSE );
		GetScrollInfo( &ScrollInfo, SIF_ALL );
//		SetScrollRange( m_MinScrollPosition, m_MaxScrollPosition );
		ScrollBarInfo.cbSize = sizeof(SCROLLBARINFO);
//		GetScrollBarInfo( &ScrollBarInfo );
//		::EnableScrollBar( GetSafeHwnd(), SB_CTL, ESB_ENABLE_BOTH );
//		::GetScrollBarInfo( GetSafeHwnd(), OBJID_CLIENT, &ScrollBarInfo );
		}
	
	return bResult;
}


void TomScrollBar::Reinitialize()
{
	m_IdleBkgColor = m_OriginalIdleBkgColor;
}


void TomScrollBar::ChangeStatus( DWORD ClearStatus, DWORD SetStatus )
{
	m_ScrollBarStyle &= ~ClearStatus;
	m_ScrollBarStyle |= SetStatus;
}



BEGIN_MESSAGE_MAP(TomScrollBar, CButton)
	//{{AFX_MSG_MAP(TomScrollBar)
	//}}AFX_MSG_MAP
//	ON_WM_HSCROLL()
ON_WM_CTLCOLOR()
ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


bool TomScrollBar::CreateSpecifiedFont() 
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

void TomScrollBar::RecomputePressedColor() 
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
// TomScrollBar message handlers


// This function can be used to reload a historical state for this button.
void TomScrollBar::HasBeenPressed( BOOL bHasBeenPressed )
{
	if ( m_ScrollBarStyle & BUTTON_PUSHBUTTON )
		{
		if ( bHasBeenPressed )
			m_IdleBkgColor = m_VisitedBkgdColor;
		else
			m_IdleBkgColor = m_OriginalIdleBkgColor;
		RecomputePressedColor();
		}
}


void TomScrollBar::TurnOnSpecialState() 
{
	m_EngageSpecialState = true;
}


void TomScrollBar::TurnOffSpecialState() 
{
	m_EngageSpecialState = false;
}


void TomScrollBar::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct ) 
{
	CDC				*pDC;
	CRect			FocusRect;
	CRect			TextRect;
	CRect			OffsetTextRect;
	CRect			ButtonRect;
	UINT			ButtonState;
	CFont			*PrevFont;
	char			*pTextString;
	unsigned		nWidth;
	
//	if ( lpDrawItemStruct -> CtlID == 1084 )
//		BOOL		bBreak = TRUE;
	if ( m_ScrollBarStyle & BUTTON_VISIBLE )
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
				DrawFilledRect( pDC, ButtonRect, m_IdleBkgColor );
				}
			}
		
		DrawFrame( pDC, ButtonRect );
		
		if ( m_ScrollBarStyle & ( BUTTON_PUSHBUTTON | BUTTON_CHECKBOX ) )
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

		if ( ButtonState & ODS_FOCUS )
			DrawFocusRect( lpDrawItemStruct -> hDC, (LPRECT)&FocusRect );
		pDC -> SelectObject( PrevFont );
		m_TextFont.DeleteObject();
		}
}



void TomScrollBar::DrawFrame( CDC *pDC, CRect ButtonRect )
{
	COLORREF				Color;
	
	// Draw black border.
	Color = RGB(0, 0, 0);		// Black.
	DrawLine( pDC, ButtonRect.left, ButtonRect.top, ButtonRect.right, ButtonRect.top, 1, Color );		// Across top
	DrawLine( pDC, ButtonRect.left, ButtonRect.top, ButtonRect.left,  ButtonRect.bottom, 1, Color );	// Down left
	DrawLine( pDC, ButtonRect.left, ButtonRect.bottom - 1, ButtonRect.right,
													ButtonRect.bottom - 1, 1, Color );					// Across bottom
	DrawLine( pDC, ButtonRect.right - 1, ButtonRect.top, ButtonRect.right - 1,
													ButtonRect.bottom, 1, Color );						// Down right
	ButtonRect.InflateRect( -1, -1 );

	Color = m_Highlight;
	DrawLine( pDC, ButtonRect.left, ButtonRect.top, ButtonRect.right, ButtonRect.top, 1, Color );		// Across top
	DrawLine( pDC, ButtonRect.left, ButtonRect.top, ButtonRect.left, ButtonRect.bottom, 1, Color );	// Down left

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


void TomScrollBar::DrawFilledRect( CDC *pDC, CRect ButtonRect, COLORREF Color )
{
	CBrush				SolidBrush;
	
	SolidBrush.CreateSolidBrush( Color );
	pDC -> FillRect( ButtonRect, &SolidBrush );
}


void TomScrollBar::DrawLine( CDC *pDC, long xStart, long yStart, long xEnd, long yEnd, int nWidth, COLORREF Color )
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


void TomScrollBar::DrawButtonText( CDC *pDC, CRect ButtonRect, char *pTextString, COLORREF TextColor )
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

//	TextSize = pDC -> GetOutputTextExtent( TextString );
	nTextLines = 0;
	if ( m_ScrollBarStyle & BUTTON_MULTILINE )
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

	if ( m_ScrollBarStyle & BUTTON_MULTILINE )
		TextFormat |= DT_WORDBREAK;
	if ( ( m_ScrollBarStyle & BUTTON_CLIP ) == 0 )
			TextFormat |= DT_NOCLIP;

	if ( m_ScrollBarStyle & BUTTON_TEXT_TOP_JUSTIFIED )
		nStartPos = ButtonRect.top + 2;
	else if ( m_ScrollBarStyle & BUTTON_TEXT_BOTTOM_JUSTIFIED )
		nStartPos = ButtonRect.bottom - nTextLines* TextSize.cy - 2;
	else if ( m_ScrollBarStyle & BUTTON_TEXT_VERTICALLY_CENTERED )
//		nStartPos = ButtonRect.Height() / 2 - ( nTextLines + 1 ) * TextSize.cy / 2;
		nStartPos = ( ButtonRect.Height() - nTextLines * TextSize.cy ) / 2 - 1;

	if ( m_ScrollBarStyle & BUTTON_TEXT_RIGHT_JUSTIFIED )
		TextFormat |= DT_RIGHT;
	else if ( m_ScrollBarStyle & BUTTON_TEXT_LEFT_JUSTIFIED )
		TextFormat |= DT_LEFT;
	else if ( m_ScrollBarStyle & BUTTON_TEXT_HORIZONTALLY_CENTERED )
		TextFormat |= DT_CENTER;
	else
		TextFormat |= DT_LEFT;

//	if ( TextFormat == 0 )
//		TextFormat = DT_CENTER | DT_VCENTER | DT_SINGLELINE;

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


int TomScrollBar::GetNewScrollPosition( int ScrollCode, int ScrollValue )
{
	SCROLLINFO			ScrollInfo;
	BOOL				bOkToUpdate;

	// Get the ScrollBar information.
	ScrollInfo.cbSize = sizeof(ScrollInfo);
	GetScrollInfo( &ScrollInfo, SIF_ALL );
	bOkToUpdate = TRUE;
	switch( ScrollCode )
		{
		case SB_LINELEFT:			// The scroll bar's left arrow was clicked.
			ScrollInfo.nPos -= 1;
			break;
		case SB_LINERIGHT:			// The scroll bar's right arrow was clicked.
			ScrollInfo.nPos += 1;
			break;
		case SB_PAGELEFT:			// The scroll bar's shaft left of the scroll box was clicked.
			ScrollInfo.nPos -= ScrollInfo.nPage;
			break;
		case SB_PAGERIGHT:			// The scroll bar's shaft right of the scroll box was clicked.
			ScrollInfo.nPos += ScrollInfo.nPage;
			break;
		case SB_LEFT:				// Scroll to the far left.
			ScrollInfo.nPos = ScrollInfo.nMin;
			break;
		case SB_RIGHT:				// Scroll to the far right.
			ScrollInfo.nPos = ScrollInfo.nMax;
			break;
		case SB_THUMBTRACK:			// The scroll bar was dragged.
			ScrollInfo.nPos = ScrollValue;
			break;
		case SB_THUMBPOSITION:		// Scroll to an absolute position.
			ScrollInfo.nPos = ScrollValue;
			break;
		case SB_ENDSCROLL:
			bOkToUpdate = FALSE;
			break;
		default:
			bOkToUpdate = FALSE;
			break;
		}
	// Set the position and then retrieve it after possible adjustments.
	if ( bOkToUpdate )
		{
		SetScrollInfo( &ScrollInfo, TRUE );
		GetScrollInfo( &ScrollInfo, SIF_ALL );
		}
	
	return ScrollInfo.nPos;
}


HBRUSH TomScrollBar::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    // Return handle to the parent (this) window's background CBrush object
    // for use in painting by the child controls.
	return HBRUSH( m_BkgdBrush );
}


BOOL TomScrollBar::OnEraseBkgnd(CDC* pDC)
{
	CBrush		BackgroundBrush( m_IdleBkgColor );
	CRect		BackgroundRectangle;

	CBrush		*pOldBrush = pDC -> SelectObject( &BackgroundBrush );
	GetClientRect( BackgroundRectangle );
	pDC -> FillRect( BackgroundRectangle, &BackgroundBrush );

	pDC -> SelectObject( pOldBrush );

	return TRUE;

//	return CScrollBar::OnEraseBkgnd(pDC);
}
