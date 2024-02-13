// ControlPanel.cpp : Implementation file for the CControlPanel class of
//	CPropertySheet, which implements the main set of tabbed windows for controlling the
//  BViewer application.
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
//
// UPDATE HISTORY:
//
//	*[1] 01/27/2024 by Tom Atwood
//		Eliminated an unused constructor.  Moved the AddPage calls to a separate
//		function for more better control over when they are called.
//
//		Attempted to solve the first time exception when a property page is opened
//		in Debug mode.  All attempts to create the pages prior to creating the
//		CControlPanel property sheet (or afterward) led to more serious results.
//		This exception is caused by a Microsoft problem, and Microsoft requires that
//		the system handles it.  The exception is raised when Windows tries to write
//		into a resource that is read-only.  There does not appear to be any fix for
//		this, so we must endeavor to persevere.
//

#include "stdafx.h"
#include "BViewer.h"
#include "BViewer.h"
#include "Module.h"
#include "ReportStatus.h"
#include "DiagnosticImage.h"
#include "Mouse.h"
#include "ImageView.h"
#include "MainFrm.h"

extern CBViewerApp			ThisBViewerApp;
extern CCustomization		*pBViewerCustomization;
extern BOOL					bTheLastKeyPressedWasESC;


// CControlPanel
CControlPanel::CControlPanel( LPCTSTR pszCaption, CWnd *pParentWnd, UINT iSelectPage )			// *[1] Simplified constructor.
									:CPropertySheet( pszCaption, pParentWnd, iSelectPage )
{
	m_bPropertyPagesCreated = FALSE;
	m_bControlPanelInitialized = FALSE;
}


// *[1] Created this separate function.  
void CControlPanel::AddControlPanelPages()
{
	AddPage( &m_SelectStudyPage );
	AddPage( &m_PerformAnalysisPage );
	AddPage( &m_ComposeReportPage );
	AddPage( &m_ViewLogPage );
	AddPage( &m_CustomizePage );
	AddPage( &m_UserManualPage );

	m_bPropertyPagesCreated = TRUE;
}


CControlPanel::~CControlPanel()
{
	KillTimer( 1 );
	m_CustomizePage.WriteBViewerConfiguration();
	DestroyWindow();
}


BEGIN_MESSAGE_MAP(CControlPanel, CPropertySheet)
	//{{AFX_MSG_MAP(CControlPanel)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CControlPanel::PreTranslateMessage( MSG *pMsg )
{
 	CMainFrame				*pMainFrame;
	BOOL					bMsgFound = FALSE;
	unsigned int			nChar;
	
	if ( m_bControlPanelInitialized )
		{
		if ( pMsg -> message == WM_KEYDOWN  )
			if ( pMsg -> wParam == VK_ESCAPE )
				{
				bTheLastKeyPressedWasESC = TRUE;
				return TRUE;    // DO NOT process further
				}
			else if ( bTheLastKeyPressedWasESC )
				{
				nChar = MapVirtualKey( pMsg -> wParam, MAPVK_VK_TO_CHAR );
				if ( nChar != 0 )
					{
					pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
					if ( pMainFrame != 0 )
						pMainFrame -> OnChar( nChar, 0, pMsg -> lParam );
					}
				return TRUE;    // DO NOT process further
				}
		}

	return CPropertySheet::PreTranslateMessage( pMsg );
}


void CControlPanel::OnSize( UINT nType, int cx, int cy )
{
	CTabCtrl		*pTabControl;

	CPropertySheet::OnSize( nType, cx, cy );

	pTabControl = GetTabControl();
	if ( pTabControl != 0 )
		pTabControl -> SetWindowPos( 0, 7, 7, cx - 14, cy - 14, 0 );
	if ( m_bPropertyPagesCreated )
		{
		if ( m_UserManualPage.GetSafeHwnd() != 0 )
			m_UserManualPage.SetWindowPos( 0, 10, 30, cx - 24, cy - 40, 0 );
		if ( m_PerformAnalysisPage.GetSafeHwnd() != 0 )
			m_PerformAnalysisPage.SetWindowPos( 0, 10, 30, cx - 24, cy - 40, 0 );
		if ( m_SelectStudyPage.GetSafeHwnd() != 0 )
			m_SelectStudyPage.SetWindowPos( 0, 10, 30, cx - 24, cy - 40, 0 );
		if ( m_ViewLogPage.GetSafeHwnd() != 0 )
			m_ViewLogPage.SetWindowPos( 0, 10, 30, cx - 24, cy - 40, 0 );
		if ( pBViewerCustomization != 0 && strlen( pBViewerCustomization -> m_ReaderInfo.LastName ) == 0 )
			SetActivePage( SETUP_PAGE );
		}
}


BOOL CControlPanel::OnInitDialog()
{
	BOOL					bResult = CPropertySheet::OnInitDialog();

	m_BkgdBrush.CreateSolidBrush( COLOR_REPORT_BKGD );
	SetIcon( ThisBViewerApp.m_hApplicationIcon, FALSE );
	GetTabControl() -> ModifyStyle( 0, TCS_OWNERDRAWFIXED, 0 );
	m_PanelTabControl.SubclassHeaderCtrl( GetTabControl() );
	m_bControlPanelInitialized = TRUE;

	return bResult;
}


HBRUSH CControlPanel::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
{
	pDC -> SetBkColor( COLOR_REPORT_BKGD );
	pDC -> SetTextColor( COLOR_REPORT_BKGD );

	return HBRUSH( m_BkgdBrush );
}


BOOL CControlPanel::OnEraseBkgnd( CDC *pDC )
{
	CBrush		BackgroundBrush( COLOR_PANEL_BKGD );
	CRect		BackgroundRectangle;
	CBrush		*pOldBrush = pDC -> SelectObject( &BackgroundBrush );

	GetClientRect( BackgroundRectangle );
	pDC -> FillRect( BackgroundRectangle, &BackgroundBrush );
	pDC -> SelectObject( pOldBrush );

	return TRUE;
}

