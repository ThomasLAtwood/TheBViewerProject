// SplashWnd.cpp : implementation file for the CSplashWnd class, which
//  implements the Splash window that displays immediately after login.
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
#include "SplashWnd.h"
#include "DiagnosticImage.h"
#include "Mouse.h"
#include "ImageView.h"
#include "MainFrm.h"


extern CBViewerApp			ThisBViewerApp;
extern CONFIGURATION		BViewerConfiguration;


// CSplashWnd
CSplashWnd::CSplashWnd():
				m_ButtonSplashOK( "OK", 100, 30, 14, 7, 6,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED | BUTTON_DEFAULT |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_SPLASH_OK )
{
}


CSplashWnd::~CSplashWnd()
{
	DestroyWindow();
}


// Caution:		Since this function creates the object, it must be called before any
//				functions such as Invalidate(), etc., that manipulate an active window.
BOOL CSplashWnd::SetPosition( int x, int y, CWnd *pParentWnd, CString WindowClass )
{
	BOOL			bResult;
	CRect			DialogRect;
	DWORD			WindowsStyle;

	WindowsStyle = DS_MODALFRAME | WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_EX_TOPMOST;
	DialogRect.SetRect( x, y, x + 620, y + 600 );
	bResult = CreateEx( WS_EX_DLGMODALFRAME, (const char*)WindowClass, "BViewer", WindowsStyle, DialogRect, pParentWnd, 0, NULL );
	
	m_ButtonSplashOK.SetPosition( 250, 530, this );
	SetFocus();

	return bResult;
}


BEGIN_MESSAGE_MAP( CSplashWnd, CWnd )
	//{{AFX_MSG_MAP(CSplashWnd)
	ON_WM_CREATE()
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_SPLASH_OK, OnBnClickedSplashOK )
	ON_WM_ERASEBKGND()
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


int CSplashWnd::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	BOOL				bNoError = TRUE;
	char				SplashBitmapFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	HBITMAP				hBitmap;

	if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
		return -1;
	
	// Read the splash window bitmap from the file.
	strcpy( SplashBitmapFileSpec, "" );
	strncat( SplashBitmapFileSpec, BViewerConfiguration.ProgramDataPath, FULL_FILE_SPEC_STRING_LENGTH - 1 );
	strncat( SplashBitmapFileSpec, "Config", FULL_FILE_SPEC_STRING_LENGTH - 2 - strlen( SplashBitmapFileSpec ) );
	LocateOrCreateDirectory( SplashBitmapFileSpec );	// Ensure directory exists.
	if ( SplashBitmapFileSpec[ strlen( SplashBitmapFileSpec ) - 1 ] != '\\' )
		strcat( SplashBitmapFileSpec, "\\" );
	strncat( SplashBitmapFileSpec, "BViewerSplash.bmp", FULL_FILE_SPEC_STRING_LENGTH - 1 - strlen( SplashBitmapFileSpec ) );

	hBitmap = (HBITMAP)LoadImage( GetModuleHandle( NULL ), SplashBitmapFileSpec, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE );

	if ( hBitmap != 0 )
		m_WelcomeBitmap.Attach( (HBITMAP)hBitmap );

	return 0;
}


void CSplashWnd::OnBnClickedSplashOK( NMHDR *pNMHDR, LRESULT *pResult )
{
	CMainFrame			*pMainFrame;

	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( pMainFrame != 0 )
		pMainFrame -> m_bSplashScreenIsUp = FALSE;
	*pResult = 0;
	DestroyWindow();
}


BOOL CSplashWnd::OnEraseBkgnd( CDC *pDC )
{
	CBrush		BackgroundBrush( COLORREF(0x000000) );
	CRect		BackgroundRectangle;
	CBrush		*pOldBrush = pDC -> SelectObject( &BackgroundBrush );
	CDC			dcMem;
	HBITMAP		hBmpOld;

	GetClientRect( BackgroundRectangle );
	pDC -> FillRect( BackgroundRectangle, &BackgroundBrush );
	dcMem.CreateCompatibleDC( pDC );
	hBmpOld = (HBITMAP)dcMem.SelectObject( m_WelcomeBitmap );
	GetDC() -> StretchBlt( BackgroundRectangle.left, BackgroundRectangle.top, BackgroundRectangle.Width(),
											BackgroundRectangle.Height(), &dcMem, 0, 10, 600, 510, SRCCOPY );
	dcMem.SelectObject( hBmpOld );
	pDC -> SelectObject( pOldBrush );

	return TRUE;
}



void CSplashWnd::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	m_ButtonSplashOK.OnChar( nChar, nRepCnt, nFlags );
}
