// ControlPanel.cpp : Implementation of the CControlPanel class.
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
#include "stdafx.h"
#include "Module.h"
#include "Configuration.h"
#include "ServiceController.h"
#include "ControlPanel.h"
#include "ServiceInterface.h"


extern CServiceControllerApp		ServiceControllerApp;
extern SERVICE_DESCRIPTOR			ServiceDescriptor;


// CControlPanel constructor.

CControlPanel::CControlPanel() :
			m_StaticLogo( IDB_B_BLACK_150W_BITMAP, 150, 120, STATIC_BITMAP | STATIC_VISIBLE, IDC_STATIC_LOGO ),
			m_StaticServiceController( "Service Controller for ", 170, 20, 14, 7, 6, COLOR_AUX_FONT, COLOR_MAIN_BKGD, COLOR_MAIN_BKGD,
						STATIC_TEXT_LEFT_JUSTIFIED | STATIC_TEXT_TOP_JUSTIFIED | STATIC_VISIBLE,
						IDC_STATIC_SERVICE_CONTROLLER ),
			m_StaticTitle( "", 340, 20, 14, 7, 6, COLOR_MAIN_FONT, COLOR_MAIN_BKGD, COLOR_MAIN_BKGD,
						STATIC_TEXT_LEFT_JUSTIFIED | STATIC_TEXT_TOP_JUSTIFIED | STATIC_VISIBLE,
						IDC_STATIC_MAIN_TITLE ),
			m_ButtonShowStatus( "Show Status", 120, 35, 14, 7, 6,
						COLOR_BUTTON_FONT, COLOR_BUTTON_BKGD, COLOR_BUTTON_BKGD, COLOR_BUTTON_BKGD,
						BUTTON_PUSHBUTTON | BUTTON_TEXT_HORIZONTALLY_CENTERED |
						BUTTON_TEXT_VERTICALLY_CENTERED | BUTTON_VISIBLE, IDC_BUTTON_SHOW_STATUS ),
			m_ButtonInstall( "Install\nthe Service", 120, 35, 14, 7, 6,
						COLOR_BUTTON_FONT, COLOR_BUTTON_BKGD, COLOR_BUTTON_BKGD, COLOR_BUTTON_BKGD,
						BUTTON_PUSHBUTTON | BUTTON_TEXT_HORIZONTALLY_CENTERED | BUTTON_MULTILINE |
						BUTTON_TEXT_VERTICALLY_CENTERED | BUTTON_VISIBLE, IDC_BUTTON_INSTALL ),
			m_ButtonStart( "Start\nthe Service", 120, 35, 14, 7, 6,
						COLOR_BUTTON_FONT, COLOR_BUTTON_BKGD, COLOR_BUTTON_BKGD, COLOR_BUTTON_BKGD,
						BUTTON_PUSHBUTTON | BUTTON_TEXT_HORIZONTALLY_CENTERED | BUTTON_MULTILINE |
						BUTTON_TEXT_VERTICALLY_CENTERED | BUTTON_VISIBLE, IDC_BUTTON_START ),
			m_ButtonStop( "Stop\nthe Service", 120, 35, 14, 7, 6,
						COLOR_BUTTON_FONT, COLOR_BUTTON_BKGD, COLOR_BUTTON_BKGD, COLOR_BUTTON_BKGD,
						BUTTON_PUSHBUTTON | BUTTON_TEXT_HORIZONTALLY_CENTERED | BUTTON_MULTILINE |
						BUTTON_TEXT_VERTICALLY_CENTERED | BUTTON_VISIBLE, IDC_BUTTON_STOP ),
			m_ButtonRemove( "Uninstall\nthe Service", 120, 35, 14, 7, 6,
						COLOR_BUTTON_FONT, COLOR_BUTTON_BKGD, COLOR_BUTTON_BKGD, COLOR_BUTTON_BKGD,
						BUTTON_PUSHBUTTON | BUTTON_TEXT_HORIZONTALLY_CENTERED | BUTTON_MULTILINE |
						BUTTON_TEXT_VERTICALLY_CENTERED | BUTTON_VISIBLE, IDC_BUTTON_REMOVE ),
			m_ButtonShowLogDetail( "Show\nDetailed Log", 120, 35, 14, 7, 6,
						COLOR_BUTTON_FONT, COLOR_BUTTON_BKGD, COLOR_BUTTON_BKGD, COLOR_BUTTON_BKGD,
						BUTTON_PUSHBUTTON | BUTTON_TEXT_HORIZONTALLY_CENTERED | BUTTON_MULTILINE |
						BUTTON_TEXT_VERTICALLY_CENTERED | BUTTON_VISIBLE, IDC_BUTTON_SHOW_LOG_DETAILS ),
			m_ButtonAbout( "About Service\nController", 120, 35, 14, 7, 6,
						COLOR_BUTTON_FONT, COLOR_BUTTON_BKGD, COLOR_BUTTON_BKGD, COLOR_BUTTON_BKGD,
						BUTTON_PUSHBUTTON | BUTTON_TEXT_HORIZONTALLY_CENTERED |
						BUTTON_TEXT_VERTICALLY_CENTERED | BUTTON_MULTILINE | BUTTON_VISIBLE, IDC_BUTTON_ABOUT ),
			m_ButtonExit( "Exit", 120, 35, 14, 7, 6, COLOR_WHITE, COLOR_CANCEL, COLOR_CANCEL, COLOR_CANCEL,
						BUTTON_PUSHBUTTON | BUTTON_TEXT_HORIZONTALLY_CENTERED |
						BUTTON_TEXT_VERTICALLY_CENTERED | BUTTON_VISIBLE, IDC_BUTTON_EXIT ),
			m_EditLog( "", 770, 400, 12, 6, 5, FIXED_PITCH_FONT, COLOR_LOG_FONT, COLOR_LOG_BKGD, COLOR_LOG_BKGD, COLOR_LOG_BKGD,
						EDIT_TEXT_LEFT_JUSTIFIED | EDIT_TEXT_TOP_JUSTIFIED | EDIT_MULTILINE | EDIT_VSCROLL | EDIT_CLIP | EDIT_VISIBLE,
						EDIT_VALIDATION_NONE, IDC_EDIT_LOG )
{
	m_bLogDisplayInitialized = FALSE;
	m_pLogText = 0;
	m_LogGranularity = SUMMARY_LOG;
}


CControlPanel::~CControlPanel()
{
	if ( m_pLogText != 0 )
		{
		free( m_pLogText );
		m_pLogText = 0;
		}
}


BEGIN_MESSAGE_MAP(CControlPanel, CWnd)
	ON_COMMAND( IDC_BUTTON_INSTALL, OnButtonInstall )
	ON_COMMAND( IDC_BUTTON_START, OnButtonStart )
	ON_COMMAND( IDC_BUTTON_SHOW_LOG_DETAILS, OnShowLogDetail )
	ON_COMMAND( IDC_BUTTON_SHOW_STATUS, OnButtonShowStatus )
	ON_COMMAND( IDC_BUTTON_STOP, OnButtonStop )
	ON_COMMAND( IDC_BUTTON_REMOVE, OnButtonRemove )
	ON_COMMAND( IDC_BUTTON_ABOUT, OnAppAbout )
	ON_COMMAND( IDC_BUTTON_EXIT, OnAppExit )
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()



// CControlPanel message handlers

BOOL CControlPanel::PreCreateWindow( CREATESTRUCT& cs ) 
{
	if ( !CWnd::PreCreateWindow( cs ) )
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, 
		::LoadCursor( NULL, IDC_ARROW ), reinterpret_cast<HBRUSH>( COLOR_WINDOW + 1 ), NULL );

	return TRUE;
}


int CControlPanel::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
		return -1;

	m_StaticLogo.SetPosition( 10, 10, this );
	m_StaticServiceController.SetPosition( 180, 5, this );
	m_StaticTitle.m_StaticText = ServiceDescriptor.DisplayedServiceName;
	m_StaticTitle.SetPosition( 350, 5, this );
	
	m_ButtonInstall.SetPosition( 180, 30, this );
	m_ButtonStart.SetPosition( 180, 80, this );
	m_ButtonShowLogDetail.SetPosition( 320, 30, this );
	m_ButtonShowStatus.SetPosition( 320, 80, this );
	m_ButtonStop.SetPosition( 460, 80, this );
	m_ButtonRemove.SetPosition( 460, 30, this );

	m_ButtonAbout.SetPosition( 660, 30, this );
	m_ButtonExit.SetPosition( 660, 80, this );

	m_EditLog.SetPosition( 10, 140, this );

	m_bLogDisplayInitialized = TRUE;

	return 0;
}


BOOL CControlPanel::OnEraseBkgnd( CDC* pDC )
{
	CBrush			BackgroundBrush( COLOR_MAIN_BKGD );
	CRect			BackgroundRectangle;
	CBrush			*pOldBrush = pDC -> SelectObject( &BackgroundBrush );

	GetClientRect( BackgroundRectangle );
	pDC -> FillRect( BackgroundRectangle, &BackgroundBrush );
	pDC -> SelectObject( pOldBrush );

	return TRUE;
}


BOOL CControlPanel::ReadLogFile()
{
	FILE						*pLogFile;
	WIN32_FIND_DATA				FindFileInfo;
	HANDLE						hFindFile;
	BOOL						bFileFound;
	size_t						LogFileSizeInBytes;
	char						*pLogTextBuffer;
	size_t						nBytesRead;
	BOOL						bLogReadSuccessfully;
	char						*pFullLogFileSpecification;

	bLogReadSuccessfully = FALSE;
	if ( m_pLogText != 0 )
		{
		free( m_pLogText );
		m_pLogText = 0;
		}
	if ( m_LogGranularity == SUMMARY_LOG )
		pFullLogFileSpecification = ServiceDescriptor.LogFileSpecification;
	else
		pFullLogFileSpecification = ServiceDescriptor.SupplementaryLogFileSpecification;
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
				nBytesRead = fread( pLogTextBuffer, 1, LogFileSizeInBytes, pLogFile );
				fclose( pLogFile );
				pLogTextBuffer[ nBytesRead ] = '\0';
				m_pLogText = pLogTextBuffer;
				bLogReadSuccessfully = TRUE;
				}
			}
		}

	return bLogReadSuccessfully;
}


void CControlPanel::OnShowLogDetail()
{
	if ( m_LogGranularity == SUMMARY_LOG )
		{
		m_LogGranularity = DETAIL_LOG;
		m_ButtonShowLogDetail.m_ButtonText = "Show\nSummary Log";
		}
	else
		{
		m_LogGranularity = SUMMARY_LOG;
		m_ButtonShowLogDetail.m_ButtonText = "Show\nDetailed Log";
		}
	m_ButtonShowLogDetail.Invalidate( TRUE );
	if ( ReadLogFile() )
		{
		m_EditLog.SetWindowText( m_pLogText );
		m_EditLog.SendMessage( WM_VSCROLL, SB_BOTTOM, 0 );
		}
}


void CControlPanel::OnButtonInstall()
{
	// Install the service.
	InstallTheService();

	if ( ReadLogFile() )
		{
		m_EditLog.SetWindowText( m_pLogText );
		m_EditLog.SendMessage( WM_VSCROLL, SB_BOTTOM, 0 );
		}
}


void CControlPanel::OnButtonStart()
{
	// Start the service.
	StartTheService();

	OnButtonShowStatus();
}


void CControlPanel::OnButtonShowStatus()
{
	CheckTheService();
	if ( ReadLogFile() )
		{
		m_EditLog.SetWindowText( m_pLogText );
		m_EditLog.SendMessage( WM_VSCROLL, SB_BOTTOM, 0 );
		}
}


void CControlPanel::OnButtonStop()
{
	// Stop the service.
	StopTheService();

	Sleep( 1000 );
	if ( ReadLogFile() )
		{
		m_EditLog.SetWindowText( m_pLogText );
		m_EditLog.SendMessage( WM_VSCROLL, SB_BOTTOM, 0 );
		}
}


void CControlPanel::OnButtonRemove()
{
	// Remove the currently installed service.
	RemoveTheService();

	// Don't recheck the status, but do update the log.
	if ( ReadLogFile() )
		{
		m_EditLog.SetWindowText( m_pLogText );
		m_EditLog.SendMessage( WM_VSCROLL, SB_BOTTOM, 0 );
		}
}


void CControlPanel::OnSize( UINT nType, int cx, int cy )
{
	CWnd::OnSize( nType, cx, cy );

	if ( m_bLogDisplayInitialized )
		m_EditLog.SetWindowPos( &wndTop, 10, 130, cx - 20, cy - 140, 0 );
}


void CControlPanel::OnAppAbout()
{
	ServiceControllerApp.OnAppAbout();
}


void CControlPanel::OnAppExit()
{
	ServiceControllerApp.OnAppExit();
}


