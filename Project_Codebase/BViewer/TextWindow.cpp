// TextWindow.cpp : Implementation file for the CTextWindow class, which
//  implements a window that displays an arbitrary buffer of text.
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
//	*[3] 10/24/2025 by Tom Atwood
//		Added scaling of display to compensate for resolution differences.
//	*[2] 02/01/2024 by Tom Atwood
//		Fixed code security issues.
//	*[1] 01/20/2023 by Tom Atwood
//		Fixed code security issues.
//
//
#include "stdafx.h"
#include "BViewer.h"
#include "TextWindow.h"

extern CONFIGURATION				BViewerConfiguration;	// *[3]


// CTextWindow
CTextWindow::CTextWindow( CWnd *pParent /*=NULL*/, unsigned short TextWindowType, double ActiveDisplayScaleFactor ) : CDialog( CTextWindow::IDD, pParent ),				// *[3]
				m_EditControl( "", 760, 500, 14, 7, 6, VARIABLE_PITCH_FONT, ActiveDisplayScaleFactor, COLOR_LOG_FONT, COLOR_LOG_BKGD, COLOR_LOG_BKGD, COLOR_LOG_BKGD,	// *[3]
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_TOP_JUSTIFIED | CONTROL_MULTILINE | EDIT_VSCROLL | CONTROL_CLIP | CONTROL_VISIBLE,
									EDIT_VALIDATION_NONE, IDC_EDIT_TEXT ),
				m_ButtonTextWindowOK( "OK", 100, 30, 14, 7, 6, ActiveDisplayScaleFactor,																				// *[3]
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_TEXT_WINDOW_OK )
{
	m_pTextForDisplay = 0;
	m_BkgdBrush.CreateSolidBrush( COLOR_LOG_BKGD );
	m_TextWindowType = TextWindowType;								// *[3]
	m_ActiveDisplayScaleFactor = ActiveDisplayScaleFactor;			// *[3]
}


CTextWindow::~CTextWindow()
{
	if ( m_pTextForDisplay != 0 )
		{
		free( m_pTextForDisplay );
		m_pTextForDisplay = 0;
		}
	DestroyWindow();				// *[3]
}


BEGIN_MESSAGE_MAP( CTextWindow, CDialog )
	//{{AFX_MSG_MAP(CTextWindow)
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_TEXT_WINDOW_OK, OnBnClickedTextWindowOK )
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CTextWindow::OnInitDialog()	// *[3] Added method.
{
	static char		TextString[ 65 ];				// *[2] Added space for a null string terminator.
	int				PrimaryScreenWidth;
	int				PrimaryScreenHeight;
	int				ScaledX;						// *[5] Added support for display scaling.
	int				ScaledY;						// *[5] Added support for display scaling.
	int				ScaledWidth;					// *[5] Added support for display scaling.
	int				ScaledHeight;					// *[5] Added support for display scaling.

	CDialog::OnInitDialog();

	m_EditControl.SetPosition( 10, 10, this );
	m_ButtonTextWindowOK.SetPosition( 330, 520, this );
//		pAboutBox -> ReadTextFileForDisplay( BViewerConfiguration.BViewerAboutFile );
	if ( m_TextWindowType == TEXT_WINDOW_ABOUT_BOX )
		ReadTextFileForDisplay( BViewerConfiguration.BViewerAboutFile );
	else if ( m_TextWindowType == TEXT_WINDOW_TECHNICAL_REQUIREMENTS )
		ReadTextFileForDisplay( BViewerConfiguration.BViewerTechnicalRequirementsFile );

	PrimaryScreenWidth = ::GetSystemMetrics( SM_CXSCREEN );
	PrimaryScreenHeight = ::GetSystemMetrics( SM_CYSCREEN );
	ScaledX =( PrimaryScreenWidth - (int)( 780.0 * m_ActiveDisplayScaleFactor ) ) / 2;		// *[5] Added support for display scaling.
	ScaledY = ( PrimaryScreenHeight - (int)( 600.0 * m_ActiveDisplayScaleFactor ) ) / 2;	// *[5] Added support for display scaling.
	ScaledWidth = (int)( 780.0 * m_ActiveDisplayScaleFactor + 0.5 );						// *[5] Added support for display scaling.
	ScaledHeight = (int)( 600.0 * m_ActiveDisplayScaleFactor + 0.5 );						// *[5] Added support for display scaling.

	SetWindowPos( &wndTop, ScaledX, ScaledY, ScaledWidth, ScaledHeight, SWP_SHOWWINDOW );	// *[2] *[5] Increased window height.

	return TRUE; 
}

BOOL CTextWindow::ReadTextFileForDisplay( char *pFullTextFileSpecification )
{
	FILE						*pTextFile;
	WIN32_FIND_DATA				FindFileInfo;
	HANDLE						hFindFile;
	BOOL						bFileFound;
	size_t						TextFileSizeInBytes;
	char						*pTextBuffer;
	size_t						nBytesRead;
	BOOL						bTextFileReadSuccessfully;

	bTextFileReadSuccessfully = FALSE;
	if ( m_pTextForDisplay != 0 )
		{
		free( m_pTextForDisplay );
		m_pTextForDisplay = 0;
		}
	hFindFile = FindFirstFile( pFullTextFileSpecification, &FindFileInfo );
	bFileFound = ( hFindFile != INVALID_HANDLE_VALUE );
	if ( hFindFile != INVALID_HANDLE_VALUE )
		FindClose( hFindFile );
	if ( bFileFound )
		{
		pTextBuffer = 0;
		TextFileSizeInBytes = (size_t)FindFileInfo.nFileSizeLow;
		if ( TextFileSizeInBytes > 0 )
			pTextBuffer = (char*)malloc( TextFileSizeInBytes + 1 );
		if ( pTextBuffer != 0 )
			{
			pTextFile = fopen( pFullTextFileSpecification, "rb" );
			if ( pTextFile != 0 )
				{
				nBytesRead = fread_s( pTextBuffer, TextFileSizeInBytes + 1, 1, TextFileSizeInBytes, pTextFile );		// *[1] Converted from fread to fread_s.
				fclose( pTextFile );
				pTextBuffer[ nBytesRead ] = '\0';
				m_pTextForDisplay = pTextBuffer;
				bTextFileReadSuccessfully = TRUE;
				}
			}
		}
	if ( bTextFileReadSuccessfully )
		m_EditControl.SetWindowText( m_pTextForDisplay );

	return bTextFileReadSuccessfully;
}


void CTextWindow::OnBnClickedTextWindowOK( NMHDR *pNMHDR, LRESULT *pResult )
{
	CDialog::OnOK();	// *[3]

	*pResult = 0;
}


HBRUSH CTextWindow::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
{
	HBRUSH			hBrush;

	if ( nCtlColor == CTLCOLOR_EDIT )
		{
		pDC -> SetBkColor( ( (TomEdit*)pWnd ) -> m_IdleBkgColor );
		pDC -> SetTextColor( ( (TomEdit*)pWnd ) -> m_TextColor );
		pDC -> SetBkMode( OPAQUE );
		hBrush = HBRUSH( *( (TomEdit*)pWnd ) -> m_pCurrentBkgdBrush );
		}
	else
		hBrush = HBRUSH( m_BkgdBrush );

	return hBrush;
}


BOOL CTextWindow::OnEraseBkgnd( CDC *pDC )
{
	CBrush		BackgroundBrush( COLORREF(0x000000) );
	CRect		BackgroundRectangle;
	CBrush		*pOldBrush = pDC -> SelectObject( &BackgroundBrush );

	GetClientRect( BackgroundRectangle );
	pDC -> FillRect( BackgroundRectangle, &BackgroundBrush );
	pDC -> SelectObject( pOldBrush );

	return TRUE;
}


