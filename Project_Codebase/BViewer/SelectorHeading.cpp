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
//	*[1] 03/15/2023 by Tom Atwood
//		Fixed code security issues.
//
#include "stdafx.h"
#include "BViewer.h"
#include "StudySelector.h"
#include "SelectorHeading.h"


// CSelectorHeading
CSelectorHeading::CSelectorHeading()
{
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
	CRect			ButtonRect;
	CBrush			BkgdBrush;
	HDITEM			hdi;
	TCHAR			lpBuffer[ 256 ];
	COLORREF		SavedTextColor;
	COLORREF		SavedBackgroundColor;

	pDC = CDC::FromHandle( pDrawItemStruct -> hDC );
	hdi.mask = HDI_TEXT;
	hdi.pszText = lpBuffer;
	hdi.cchTextMax = 256;
	GetItem( pDrawItemStruct -> itemID, &hdi );
	// Draw the button frame.
	::DrawFrameControl( pDC -> m_hDC, &pDrawItemStruct -> rcItem, DFC_BUTTON, DFCS_BUTTONPUSH );
	
	ButtonRect.CopyRect( &pDrawItemStruct -> rcItem ); 
	BkgdBrush.CreateSolidBrush( COLOR_PATIENT );
	pDC -> FillRect( ButtonRect, &BkgdBrush );
	// Draw the item's text using the text color white:
	SavedTextColor = pDC -> SetTextColor( RGB( 255, 255, 255 ) );
	SavedBackgroundColor = pDC -> SetBkColor( COLOR_PATIENT );
	::DrawText( pDC -> m_hDC, lpBuffer, (int)strlen( lpBuffer ), &pDrawItemStruct -> rcItem, DT_SINGLELINE | DT_VCENTER | DT_LEFT );
	pDC -> SetBkColor( SavedBackgroundColor );
	pDC -> SetTextColor( SavedTextColor );				// *[1] Added color restore.
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




