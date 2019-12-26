// TomEditDate.cpp : Implementation file for the TomEditDate class of enhanced
//	date display and editing controls for use in a user interface window.
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
#pragma once

#include "stdafx.h"
#include "TomEditDate.h"
#include "Customization.h"

#define _CRTDBG_MAP_ALLOC 


extern CCustomization				BViewerCustomization;


// TomEditDate
TomEditDate::TomEditDate( char *pEditText, int EditWidth, int EditHeight, int FontHeight, int FontWidth, int FontWeight,
				COLORREF TextColor, COLORREF BackgroundColor, COLORREF ActivatedBkgdColor, COLORREF VisitedBkgdColor,
				DWORD EditStyle, UINT nID ) : CDateTimeCtrl()
{
	m_EditText = (const char*)pEditText;
	m_EditWidth = EditWidth;
	m_EditHeight = EditHeight;
	m_FontHeight = FontHeight;
	m_FontWidth = FontWidth;
	m_FontWeight = FontWeight * 100;			// FontWeight: 1 through 9
	m_TextColor = TextColor;
	m_OriginalIdleBkgColor = BackgroundColor;
	m_IdleBkgColor = BackgroundColor;
	m_ActivatedBkgdColor = ActivatedBkgdColor;
	m_VisitedBkgdColor = VisitedBkgdColor;
	m_EditStyle = EditStyle;
	m_nObjectID = nID;
	m_bHasReceivedInput = FALSE;
}


TomEditDate::~TomEditDate()
{
	m_EditText.Empty();
	m_TextFont.DeleteObject();
}


BEGIN_MESSAGE_MAP(TomEditDate, CWnd)
	//{{AFX_MSG_MAP(TomEditDate)
	ON_WM_ERASEBKGND()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// Caution:		Since this function creates the object, it must be called before any
//				functions such as Invalidate(), etc., that manipulate an active window.
BOOL TomEditDate::SetPosition( int x, int y, CWnd *pParentWnd )
{
	BOOL			bResult;
	CRect			EditRect;
	DWORD			WindowsEditStyle;
	CString			DateFormatString;

	WindowsEditStyle = DTS_RIGHTALIGN | WS_BORDER | WS_CHILD | WS_VISIBLE;
	EditRect.SetRect( x, y, x + m_EditWidth, y + m_EditHeight );
	bResult = Create( WindowsEditStyle, EditRect, pParentWnd, m_nObjectID );
	CreateSpecifiedFont();
	SetFont( &m_TextFont, FALSE );
	SetMonthCalColor( MCSC_BACKGROUND,		m_VisitedBkgdColor	);	// MCSC_BACKGROUND  Set the background color displayed between months.
	SetMonthCalColor( MCSC_MONTHBK,			m_VisitedBkgdColor	);	// MCSC_MONTHBK  Set the background color displayed within a month.
	SetMonthCalColor( MCSC_TEXT,			m_TextColor			);	// MCSC_TEXT  Set the color used to display text within a month.
	SetMonthCalColor( MCSC_TITLEBK,			m_VisitedBkgdColor	);	// MCSC_TITLEBK  Set the background color displayed in the calendar's title.
	SetMonthCalColor( MCSC_TITLETEXT,		m_TextColor			);	// MCSC_TITLETEXT  Set the color used to display text within the calendar's title.
	SetMonthCalColor( MCSC_TRAILINGTEXT,	RGB( 0, 255, 255 )	);	// MCSC_TRAILINGTEXT  Set the color used to display header and trailing-day text.
	switch ( BViewerCustomization.m_CountryInfo.DateFormat )
		{
		case DATE_FORMAT_UNSPECIFIED:
			break;
		case DATE_FORMAT_YMD:
			DateFormatString = _T("yyyy/MM/dd");
			SetFormat( DateFormatString );
	 		break;
		case DATE_FORMAT_DMY:
			DateFormatString = _T("dd/MM/yyyy");
			SetFormat( DateFormatString );
	 		break;
			break;
		case DATE_FORMAT_MDY:
			break;
		}

	return bResult;
}


void TomEditDate::ChangeStatus( DWORD ClearStatus, DWORD SetStatus )
{
	m_EditStyle &= ~ClearStatus;
	m_EditStyle |= SetStatus;

	if ( ClearStatus & CONTROL_VISIBLE )
		ModifyStyle( WS_VISIBLE, 0, 0 );
	else if ( SetStatus & CONTROL_VISIBLE )
		ModifyStyle( 0, WS_VISIBLE, 0 );

}


void TomEditDate::HasBeenCompleted( BOOL bHasBeenCompleted )
{
	if ( bHasBeenCompleted )
		m_IdleBkgColor = m_ActivatedBkgdColor;
	else
		m_IdleBkgColor = m_OriginalIdleBkgColor;
}


BOOL TomEditDate::CreateSpecifiedFont() 
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


BOOL TomEditDate::OnEraseBkgnd( CDC *pDC )
{
	CBrush		BackgroundBrush( m_IdleBkgColor );
	CRect		BackgroundRectangle;

	UnrealizeObject( HBRUSH( BackgroundBrush ) );
	CBrush		*pOldBrush = pDC -> SelectObject( &BackgroundBrush );

	GetClientRect( BackgroundRectangle );
	pDC -> FillRect( BackgroundRectangle, &BackgroundBrush );
	pDC -> SelectObject( pOldBrush );

	return TRUE;
}


void TomEditDate::OnSetFocus( CWnd *pOldWnd )
{
	NMHDR			NotifyMessageHeader;

	CDateTimeCtrl::OnSetFocus( pOldWnd );
	m_bHasReceivedInput = TRUE;
	NotifyMessageHeader.code = WM_SETFOCUS;
	NotifyMessageHeader.hwndFrom = GetSafeHwnd();
	NotifyMessageHeader.idFrom = GetDlgCtrlID();
	GetParent() -> SendMessage( WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&NotifyMessageHeader );
}


