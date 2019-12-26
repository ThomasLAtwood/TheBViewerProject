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
//
#include "stdafx.h"
#include "BViewer.h"
#include "TextWindow.h"


// CTextWindow
CTextWindow::CTextWindow():
				m_EditControl( "", 760, 500, 10, 5, 5, FIXED_PITCH_FONT, COLOR_LOG_FONT, COLOR_LOG_BKGD, COLOR_LOG_BKGD, COLOR_LOG_BKGD,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_TOP_JUSTIFIED | CONTROL_MULTILINE | EDIT_VSCROLL | CONTROL_CLIP | CONTROL_VISIBLE,
									EDIT_VALIDATION_NONE, IDC_EDIT_TEXT ),
				m_ButtonTextWindowOK( "OK", 100, 30, 14, 7, 6,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_TEXT_WINDOW_OK )
{
	m_pTextForDisplay = 0;
	m_BkgdBrush.CreateSolidBrush( COLOR_LOG_BKGD );
}


CTextWindow::~CTextWindow()
{
	if ( m_pTextForDisplay != 0 )
		{
		free( m_pTextForDisplay );
		m_pTextForDisplay = 0;
		}
}


// Caution:		Since this function creates the object, it must be called before any
//				functions such as Invalidate(), etc., that manipulate an active window.
BOOL CTextWindow::SetPosition( int x, int y, CWnd *pParentWnd, CString WindowClass )
{
	BOOL			bResult;
	CRect			DialogRect;
	DWORD			WindowsStyle;
 
	WindowsStyle = DS_MODALFRAME | WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_EX_TOPMOST;
	DialogRect.SetRect( x, y, x + 780, y + 580 );
	bResult = CreateEx( WS_EX_DLGMODALFRAME, (const char*)WindowClass, "About BViewer", WindowsStyle, DialogRect, pParentWnd, 0, NULL );

	m_EditControl.SetPosition( 10, 10, this );
	m_ButtonTextWindowOK.SetPosition( 330, 520, this );

	return bResult;
}


BEGIN_MESSAGE_MAP( CTextWindow, CWnd )
	//{{AFX_MSG_MAP(CTextWindow)
	ON_WM_CREATE()
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_TEXT_WINDOW_OK, OnBnClickedTextWindowOK )
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


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
				nBytesRead = fread( pTextBuffer, 1, TextFileSizeInBytes, pTextFile );
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
	CloseWindow();

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


