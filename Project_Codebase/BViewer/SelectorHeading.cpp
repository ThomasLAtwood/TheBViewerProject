// SelectorHeading.cpp : Implementation file for the CSelectorHeading class of
//	CHeaderCtrl, which implements the column header of the CStudySelector list.
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
// UPDATE HISTORY:
//
//	*[2] 08/12/2025 by Tom Atwood
//		Added scaling of display to compensate for resolution differences.
//	*[1] 03/15/2023 by Tom Atwood
//		Fixed code security issues.
//
#include "stdafx.h"
#include "BViewer.h"
#include "StudySelector.h"
#include "SelectorHeading.h"


// CSelectorHeading
CSelectorHeading::CSelectorHeading( double ActiveDisplayScaleFactor )	// *[2]
{
	m_ActiveDisplayScaleFactor = ActiveDisplayScaleFactor;				// *[2]
}


CSelectorHeading::~CSelectorHeading()
{
}


void CSelectorHeading::SubclassHeaderCtrl( CHeaderCtrl *pHeaderCtrl )
{
	SubclassWindow( pHeaderCtrl -> GetSafeHwnd() );
}


BEGIN_MESSAGE_MAP( CSelectorHeading, CHeaderCtrl )
	//{{AFX_MSG_MAP(CSelectorHeading)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CSelectorHeading::DrawItem( LPDRAWITEMSTRUCT pDrawItemStruct )
{
	CDC				*pDC;
	int				nSavedDC;				// *[2]
	HDITEM			HeaderItem;				// *[2]
	CRect			ItemRect;				// *[2]
	CRect			ScaledCellRect;			// *[2]
	CBrush			BkgdBrush;
	int				nItem;					// *[2]
	TCHAR			ItemText[ 256 ];		// *[2]
	COLORREF		SavedTextColor;
	COLORREF		SavedBackgroundColor;
	CFont			*pSelectionListFont;	// *[2]
	HFONT			hPrevFont;				// *[2]
	LOGFONT			FontInfo;				// *[2]
	HFONT			hFont;					// *[2]

	pDC = CDC::FromHandle( pDrawItemStruct -> hDC );
	nSavedDC = pDC -> SaveDC();				// *[2]

	nItem = pDrawItemStruct -> itemID;		// *[2]
	HeaderItem.mask = HDI_TEXT;				// *[2]
	HeaderItem.pszText = ItemText;			// *[2]
	HeaderItem.cchTextMax = 255;			// *[2]
	GetItem( nItem, &HeaderItem );			// *[2]

	pSelectionListFont = GetFont();			// *[2]
	pSelectionListFont -> GetLogFont( &FontInfo );						// *[2]
	FontInfo.lfHeight = (int)( -12.0 * m_ActiveDisplayScaleFactor );	// *[2]
	hFont = CreateFontIndirect( &FontInfo );							// *[2]
	hPrevFont = (HFONT)pDC -> SelectObject( hFont );					// *[2]

	// Draw the cell rectangle border.									// *[2]
	ItemRect = pDrawItemStruct -> rcItem;								// *[2]
	ItemRect.CopyRect( &pDrawItemStruct -> rcItem );					// *[2]
	ScaledCellRect.left = (long)( (double)ItemRect.left * m_ActiveDisplayScaleFactor );			// *[2]
	ScaledCellRect.bottom = (long)( 30.0 * m_ActiveDisplayScaleFactor );						// *[2]
	ScaledCellRect.right = (long)( (double)ItemRect.right * m_ActiveDisplayScaleFactor );		// *[2]
	ScaledCellRect.top = (long)( (double)ItemRect.top * m_ActiveDisplayScaleFactor );			// *[2]
	::DrawFrameControl( pDC -> m_hDC, &ScaledCellRect, DFC_BUTTON, DFCS_BUTTONPUSH );			// *[2]

	BkgdBrush.CreateSolidBrush( COLOR_PATIENT );
	pDC -> FillRect( &ScaledCellRect, &BkgdBrush );						// *[2]

	// Draw the item's text using the text color white:
	SavedTextColor = pDC -> SetTextColor( COLOR_WHITE );				// *[2]
	SavedBackgroundColor = pDC -> SetBkColor( COLOR_PATIENT );
	::DrawText( pDC -> m_hDC, ItemText, (int)strlen( ItemText ), &pDrawItemStruct -> rcItem, DT_SINGLELINE | DT_VCENTER | DT_LEFT );	// *[2]

	pDC -> SetBkColor( SavedBackgroundColor );
	pDC -> SetTextColor( SavedTextColor );				// *[1] Added color restore.
	pDC -> SelectObject( hPrevFont );		// *[2]
	DeleteObject( hFont );					// *[2]
	pDC -> RestoreDC( nSavedDC );			// *[2]
}


BOOL CSelectorHeading::OnEraseBkgnd( CDC *pDC )
{
	CBrush		BackgroundBrush( COLOR_PATIENT );
	CRect		BackgroundRectangle;

	CBrush		*pOldBrush = pDC -> SelectObject( &BackgroundBrush );
	GetClientRect( BackgroundRectangle );
	pDC -> FillRect( BackgroundRectangle, &BackgroundBrush );
	pDC -> SelectObject( pOldBrush );

	return TRUE;
}




