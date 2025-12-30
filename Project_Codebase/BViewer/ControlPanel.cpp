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
//	*[2] 08/12/2025 by Tom Atwood
//		Added scaling of display to compensate for resolution differences.
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
CControlPanel::CControlPanel( LPCTSTR pszCaption, CWnd *pParentWnd, UINT iSelectPage, double ActiveDisplayScaleFactor )			// *[1] Simplified constructor. *[2]
									: CPropertySheet( pszCaption, pParentWnd, iSelectPage )
{
	m_bPropertyPagesCreated = FALSE;
	m_bControlPanelInitialized = FALSE;
	m_ActiveDisplayScaleFactor = ActiveDisplayScaleFactor;								// *[2]
	m_pSelectStudyPage = new CSelectStudyPage( m_ActiveDisplayScaleFactor );			// *[2]
	m_pPerformAnalysisPage = new CAnalysisPage( m_ActiveDisplayScaleFactor );			// *[2]
	m_pComposeReportPage = new CComposeReportPage( m_ActiveDisplayScaleFactor );		// *[2]
	m_pViewLogPage = new CViewLogPage( m_ActiveDisplayScaleFactor );					// *[2]
	m_pCustomizePage = new CCustomizePage( m_ActiveDisplayScaleFactor );				// *[2]
	m_pUserManualPage = new CUserManualPage( m_ActiveDisplayScaleFactor );				// *[2]
	m_pMainFrame = (CMainFrame*)pParentWnd;												// *[2]
}


// *[1] Created this separate function.  
void CControlPanel::AddControlPanelPages()
{
	AddPage( m_pSelectStudyPage );		// *[2]
	AddPage( m_pPerformAnalysisPage );	// *[2]
	AddPage( m_pComposeReportPage );	// *[2]
	AddPage( m_pViewLogPage );			// *[2]
	AddPage( m_pCustomizePage );		// *[2]
	AddPage( m_pUserManualPage );		// *[2]

	m_bPropertyPagesCreated = TRUE;
}


CControlPanel::~CControlPanel()
{
	KillTimer( 1 );
	m_pCustomizePage -> WriteBViewerConfiguration();	// *[2]
	delete( m_pSelectStudyPage );						// *[2]
	delete( m_pPerformAnalysisPage );					// *[2]
	delete( m_pComposeReportPage );						// *[2]
	delete( m_pViewLogPage );							// *[2]
	delete( m_pCustomizePage );							// *[2]
	delete( m_pUserManualPage );						// *[2]
	DestroyWindow();
}


BEGIN_MESSAGE_MAP(CControlPanel, CPropertySheet)
	//{{AFX_MSG_MAP(CControlPanel)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// *[2] Revised substantially to support scaling.
BOOL CControlPanel::OnInitDialog()
{
	RECT			ClientRect;						// *[2]
	int				AdjustedControlX;				// *[2]
	int				AdjustedControlY;				// *[2]
	int				AdjustedControlWidth;			// *[2]
	int				AdjustedControlHeight;			// *[2]
	LOGFONT			ControlPanelTabLogicalFont;		// *[2] Added support for display scaling.
	CFont			*pControlPanelTabFont;			// *[2] Added support for display scaling.

	BOOL			bResult = CPropertySheet::OnInitDialog();

//	m_pMainFrame -> GetClientRect( &ClientRect );	// *[2]
	GetClientRect( &ClientRect );					// *[2]
	AdjustedControlX = ClientRect.left;
	AdjustedControlY = (int)( 29.0 * m_ActiveDisplayScaleFactor );
	AdjustedControlWidth = ClientRect.right;
	AdjustedControlHeight =ClientRect.bottom - AdjustedControlY;

	SetWindowPos( 0, AdjustedControlX, AdjustedControlY, AdjustedControlWidth, AdjustedControlHeight, 0 );

	m_BkgdBrush.CreateSolidBrush( COLOR_REPORT_BKGD );
	SetIcon( ThisBViewerApp.m_hApplicationIcon, FALSE );
	GetTabControl() -> ModifyStyle( 0, TCS_OWNERDRAWFIXED, 0 );
	m_PanelTabControl.SubclassHeaderCtrl( GetTabControl() );

	// *[2] Scale the study selection list header text font.
	pControlPanelTabFont =  m_PanelTabControl.GetFont();									// *[2] Added support for display scaling.
	pControlPanelTabFont -> GetLogFont( &ControlPanelTabLogicalFont );						// *[2] Added support for display scaling.
	ControlPanelTabLogicalFont.lfHeight = (int)( -12.0 * m_ActiveDisplayScaleFactor );		// *[2] Added support for display scaling.
	m_ControlPanelTabFont.CreateFontIndirect( &ControlPanelTabLogicalFont );				// *[2] Added support for display scaling.
	m_PanelTabControl.SetFont( &m_ControlPanelTabFont );									// *[2] Added support for display scaling.

	m_bControlPanelInitialized = TRUE;

	return bResult;
}


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
	int				ScaledTabHeight;		// *[2] Added support for display scaling.

	CPropertySheet::OnSize( nType, cx, cy );

	pTabControl = GetTabControl();
	if ( pTabControl != 0 )
		pTabControl -> SetWindowPos( 0, 7, 7, cx - 14, cy - 14, 0 );
	if ( m_bPropertyPagesCreated )
		{
		ScaledTabHeight = (int)( 26.0 * m_ActiveDisplayScaleFactor );									// *[2] Added support for display scaling.
		if ( m_pSelectStudyPage -> GetSafeHwnd() != 0 )
			m_pSelectStudyPage -> SetWindowPos( 0, 10, ScaledTabHeight, cx - 24, cy - 40, 0 );			// *[2] Added support for display scaling.
		if ( m_pPerformAnalysisPage -> GetSafeHwnd() != 0 )
			m_pPerformAnalysisPage -> SetWindowPos( 0, 10, ScaledTabHeight, cx - 24, cy - 40, 0 );		// *[2] Added support for display scaling.
		if ( m_pComposeReportPage -> GetSafeHwnd() != 0 )
			m_pComposeReportPage -> SetWindowPos( 0, 10, ScaledTabHeight, cx - 24, cy - 40, 0 );		// *[2] Added support for display scaling.
		if ( m_pViewLogPage -> GetSafeHwnd() != 0 )
			m_pViewLogPage -> SetWindowPos( 0, 10, ScaledTabHeight, cx - 24, cy - 40, 0 );				// *[2] Added support for display scaling.
		if ( m_pCustomizePage -> GetSafeHwnd() != 0 )
			m_pCustomizePage -> SetWindowPos( 0, 10, ScaledTabHeight, cx - 24, cy - 40, 0 );			// *[2] Added support for display scaling.
		if ( m_pUserManualPage -> GetSafeHwnd() != 0 )
			m_pUserManualPage -> SetWindowPos( 0, 10, ScaledTabHeight, cx - 24, cy - 40, 0 );			// *[2] Added support for display scaling.
		if ( pBViewerCustomization != 0 && strlen( pBViewerCustomization -> m_ReaderInfo.LastName ) == 0 )
			SetActivePage( SETUP_PAGE );
		}
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

