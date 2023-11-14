// PanelTabCtrl.cpp : Implementation file for the CPanelTabCtrl class of
//  CTabCtrl, which implements the tabs of the CControlPanel window.
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
//	*[1] 03/14/2023 by Tom Atwood
//		Fixed code security issues.
//
//
#include "stdafx.h"
#include "BViewer.h"
#include "PanelTabCtrl.h"


// CPanelTabCtrl
CPanelTabCtrl::CPanelTabCtrl()
{
	m_BkgdBrush.CreateSolidBrush( COLOR_REPORT_BKGD );
}

CPanelTabCtrl::~CPanelTabCtrl()
{
}


BEGIN_MESSAGE_MAP( CPanelTabCtrl, CTabCtrl )
	//{{AFX_MSG_MAP(CPanelTabCtrl)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CPanelTabCtrl::PreSubclassWindow()
{
	ModifyStyle( 0, TCS_OWNERDRAWFIXED, 0 );			// *[1] Removed unused return value assignment.
	CTabCtrl::PreSubclassWindow();
}


void CPanelTabCtrl::SubclassHeaderCtrl( CTabCtrl *pTabCtrl )
{
	SubclassWindow( pTabCtrl -> GetSafeHwnd() );
}


void CPanelTabCtrl::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	CDC*			pDC;
	CRect			rect = lpDrawItemStruct -> rcItem;
	int				nTabIndex;
// *[1]	BOOL			bSelected;
	int				nSavedDC;
	char			label[64];
	TC_ITEM			tci;
	COLORREF		TabColor;
	COLORREF		TextColor = 0;			// *[1] Initialized variable.

	pDC = CDC::FromHandle( lpDrawItemStruct -> hDC );

	tci.mask = TCIF_TEXT | TCIF_IMAGE;
	tci.pszText = label;
	tci.cchTextMax = 63;

	nTabIndex = lpDrawItemStruct -> itemID;
	if ( nTabIndex >= 0 && GetItem( nTabIndex, &tci ) != 0 && pDC != 0 )
		{
		TabColor = COLOR_PANEL_BKGD;
		
		if ( nTabIndex == 0 )
			{
			TabColor = COLOR_PATIENT;
			TextColor = COLOR_WHITE;
			}
		else if ( nTabIndex == 1 )
			{
			TabColor = COLOR_ANALYSIS_BKGD;
			TextColor = COLOR_WHITE;
			}
		else if ( nTabIndex == 2 )
			{
			TabColor = COLOR_REPORT_BKGD;
			TextColor = COLOR_WHITE;
			}
		else if ( nTabIndex == 3 )
			{
			TabColor = COLOR_LOG_BKGD;
			TextColor = COLOR_LOG_FONT;
			}
		else if ( nTabIndex == 4 )
			{
			TabColor = COLOR_CONFIG;
			TextColor = COLOR_BLACK;
			}
		else if ( nTabIndex == 5 )
			{
			TabColor = COLOR_REPORT;
			TextColor = COLOR_BLACK;
			}
// *[1]		bSelected = ( nTabIndex == GetCurSel() );
		nSavedDC = pDC -> SaveDC();

		// For some bizarre reason the rcItem you get extends above the actual
		// drawing area. We have to workaround this "feature".
		rect.top += ::GetSystemMetrics( SM_CYEDGE );
		pDC -> FillSolidRect( rect, TabColor );
		pDC -> SetTextColor( TextColor );
		pDC -> DrawText( label, rect, DT_SINGLELINE | DT_TOP | DT_CENTER );
		pDC -> RestoreDC( nSavedDC );
		}
}


BOOL CPanelTabCtrl::OnEraseBkgnd( CDC *pDC )
{
	CBrush		BackgroundBrush( COLOR_PANEL_BKGD );
	CRect		BackgroundRectangle;
	CBrush		*pOldBrush = pDC -> SelectObject( &BackgroundBrush );

	GetClientRect( BackgroundRectangle );
	pDC -> FillRect( BackgroundRectangle, &BackgroundBrush );
	pDC -> SelectObject( pOldBrush );

	return TRUE;
}



