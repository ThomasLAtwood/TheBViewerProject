// ViewLogPage.cpp : Implementation file for the CViewLogPage class of
//  CPropertyPage, which implements the "View Log" tab of the main Control Panel.
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
//	*[1] 01/20/2023 by Tom Atwood
//		Fixed code security issues.
//
//
#include "stdafx.h"
#include "BViewer.h"
#include "Module.h"
#include "Configuration.h"
#include "DiagnosticImage.h"
#include "Mouse.h"
#include "ImageView.h"
#include "MainFrm.h"
#include "ViewLogPage.h"


extern CONFIGURATION		BViewerConfiguration;

// CViewLogPage dialog
CViewLogPage::CViewLogPage() : CPropertyPage(CViewLogPage::IDD),
			m_EditLog( "", 1000, 730, 12, 6, 5, FIXED_PITCH_FONT, COLOR_LOG_FONT, COLOR_LOG_BKGD, COLOR_LOG_BKGD, COLOR_LOG_BKGD,
						CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_TOP_JUSTIFIED | CONTROL_MULTILINE | EDIT_VSCROLL | CONTROL_CLIP | CONTROL_VISIBLE,
						EDIT_VALIDATION_NONE, IDC_EDIT_LOG )
{
	m_bLogDisplayInitialized = FALSE;
	m_LogGranularity = SUMMARY_LOG;
	m_pLogText = 0;
	m_BkgdBrush.CreateSolidBrush( COLOR_LOG_BKGD );
}


CViewLogPage::~CViewLogPage()
{
	if ( m_pLogText != 0 )
		{
		free( m_pLogText );
		m_pLogText = 0;
		}
}


BEGIN_MESSAGE_MAP( CViewLogPage, CPropertyPage )
	//{{AFX_MSG_MAP(CViewLogPage)
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CViewLogPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_EditLog.SetPosition( 10, 10, this );
	m_bLogDisplayInitialized = TRUE;

	return TRUE;
}


BOOL CViewLogPage::ReadLogFile()
{
	char						*pFullLogFileSpecification;
	FILE						*pLogFile;
	WIN32_FIND_DATA				FindFileInfo;
	HANDLE						hFindFile;
	BOOL						bFileFound;
	size_t						LogFileSizeInBytes;
	char						*pLogTextBuffer;
	size_t						nBytesRead;
	BOOL						bLogReadSuccessfully;

	bLogReadSuccessfully = FALSE;
	if ( m_pLogText != 0 )
		{
		free( m_pLogText );
		m_pLogText = 0;
		}
	if ( m_LogGranularity == SUMMARY_LOG )
		pFullLogFileSpecification = BViewerConfiguration.BViewerLogFile;
	else
		pFullLogFileSpecification = BViewerConfiguration.BViewerSupplementaryLogFile;
	hFindFile = FindFirstFile( pFullLogFileSpecification, &FindFileInfo );
	bFileFound = ( hFindFile != INVALID_HANDLE_VALUE );
	if ( hFindFile != INVALID_HANDLE_VALUE )
		FindClose( hFindFile );
	if ( bFileFound )
		{
		pLogTextBuffer = 0;
		LogFileSizeInBytes = (size_t)FindFileInfo.nFileSizeLow;
		if ( LogFileSizeInBytes > 0 )
			pLogTextBuffer = (char*)malloc( LogFileSizeInBytes + 1 );
		if ( pLogTextBuffer != 0 )
			{
			pLogFile = fopen( pFullLogFileSpecification, "rb" );
			if ( pLogFile != 0 )
				{
				nBytesRead = fread_s( pLogTextBuffer, LogFileSizeInBytes + 1, 1, LogFileSizeInBytes, pLogFile );		// *[1] Converted from fread to fread_s.
				fclose( pLogFile );
				pLogTextBuffer[ nBytesRead ] = '\0';
				m_pLogText = pLogTextBuffer;
				bLogReadSuccessfully = TRUE;
				}
			else
				free ( pLogTextBuffer );			// *[1] Fixed potential memory leak.
			}
		}

	return bLogReadSuccessfully;
}


BOOL CViewLogPage::OnSetActive()
{
	CMainFrame			*pMainFrame;
	CControlPanel		*pControlPanel;

	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( pMainFrame != 0 )
		{
		pMainFrame -> m_wndDlgBar.m_ButtonShowLogDetail.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
		pMainFrame -> m_wndDlgBar.m_ButtonShowLogDetail.EnableWindow( TRUE );
		pMainFrame -> m_wndDlgBar.m_ButtonShowLogDetail.Invalidate( TRUE );
		}
	if ( ReadLogFile() )
		{
		m_EditLog.SetWindowText( m_pLogText );
		m_EditLog.SendMessage( WM_VSCROLL, SB_BOTTOM, 0 );
		}

	pControlPanel = (CControlPanel*)GetParent();
	if ( pControlPanel != 0 )
		pControlPanel -> m_CurrentlyActivePage = LOG_PAGE;

	return CPropertyPage::OnSetActive();
}


BOOL CViewLogPage::OnKillActive()
{
	CMainFrame			*pMainFrame;

	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( pMainFrame != 0 )
		{
		pMainFrame -> m_wndDlgBar.m_ButtonShowLogDetail.EnableWindow( FALSE );
		pMainFrame -> m_wndDlgBar.m_ButtonShowLogDetail.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
		pMainFrame -> m_wndDlgBar.m_ButtonShowLogDetail.Invalidate( TRUE );
		pMainFrame -> m_wndDlgBar.Invalidate( TRUE );
		}
	if ( m_pLogText != 0 )
		{
		free( m_pLogText );
		m_pLogText = 0;
		}

	return CPropertyPage::OnKillActive();
}


void CViewLogPage::OnShowLogDetail()
{
	CMainFrame			*pMainFrame;

	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( pMainFrame != 0 )
		{
		if ( m_LogGranularity == SUMMARY_LOG )
			{
			m_LogGranularity = DETAIL_LOG;
			pMainFrame -> m_wndDlgBar.m_ButtonShowLogDetail.m_ControlText = "Show\nSummary Log";
			}
		else
			{
			m_LogGranularity = SUMMARY_LOG;
			pMainFrame -> m_wndDlgBar.m_ButtonShowLogDetail.m_ControlText = "Show\nDetailed Log";
			}
		pMainFrame -> m_wndDlgBar.m_ButtonShowLogDetail.Invalidate( TRUE );
		}
	if ( ReadLogFile() )
		{
		m_EditLog.SetWindowText( m_pLogText );
		m_EditLog.SendMessage( WM_VSCROLL, SB_BOTTOM, 0 );
		}
}


HBRUSH CViewLogPage::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
{
	CBrush				BkgdBrush;

   if ( nCtlColor == CTLCOLOR_EDIT )
		{
		pDC -> SetBkColor( ( (TomEdit*)pWnd ) -> m_IdleBkgColor );
		pDC -> SetTextColor( ( (TomEdit*)pWnd ) -> m_TextColor );
		}

	return HBRUSH( m_BkgdBrush );
}


void CViewLogPage::OnSize( UINT nType, int cx, int cy )
{
	CPropertyPage::OnSize( nType, cx, cy );

	if ( m_bLogDisplayInitialized )
		m_EditLog.SetWindowPos( 0, 7, 7, cx - 14, cy - 14, 0 );

}



