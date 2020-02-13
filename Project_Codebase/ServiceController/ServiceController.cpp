// ServiceController.cpp : Implements the application class for this program.
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
#include "ReportStatus.h"
#include "Configuration.h"
#include "ServiceController.h"
#include "MainFrm.h"


// The one and only CServiceControllerApp object
CServiceControllerApp		ServiceControllerApp;

SERVICE_DESCRIPTOR			ServiceDescriptor;

//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		MainModuleInfo = { MODULE_MAIN, "Main Module", InitMainModule, CloseMainModule };


static ERROR_DICTIONARY_ENTRY	MainModuleErrorCodes[] =
			{
				{ MAIN_ERROR_INSUFFICIENT_MEMORY		, "There is not enough memory to allocate a data structure." },
				{ MAIN_ERROR_OPEN_CFG_FILE				, "An error occurred attempting to open the configuration file." },
				{ MAIN_ERROR_READ_CFG_FILE				, "An error occurred while reading the configuration file." },
				{ 0										, NULL }
			};


static ERROR_DICTIONARY_MODULE		MainModuleStatusErrorDictionary =
										{
										MODULE_MAIN,
										MainModuleErrorCodes,
										MAIN_ERROR_DICT_LENGTH,
										0
										};

void InitMainModule()
{
	BOOL			bNoError = TRUE;

	LinkModuleToList( &MainModuleInfo );
	RegisterErrorDictionary( &MainModuleStatusErrorDictionary );
}


void CloseMainModule()
{
}


// CServiceControllerApp

BEGIN_MESSAGE_MAP(CServiceControllerApp, CWinApp)
	ON_COMMAND( ID_APP_ABOUT, OnAppAbout )
	ON_COMMAND( ID_APP_EXIT, OnAppExit )
END_MESSAGE_MAP()


// CServiceControllerApp construction

CServiceControllerApp::CServiceControllerApp()
{
	memset( &ServiceDescriptor, '\0', sizeof( SERVICE_DESCRIPTOR ) );
}



// CServiceControllerApp initialization

BOOL CServiceControllerApp::InitInstance()
{
	BOOL			bNoError = TRUE;
	
	CWinApp::InitInstance();

	GetCmdLine();
	m_hApplicationIcon = (HICON)::LoadImage( m_hInstance, MAKEINTRESOURCE( IDR_MAINFRAME ), IMAGE_ICON, 16, 16, LR_SHARED );

	InitializeSoftwareModules();
	// The ServiceDescriptor structure has been initialized with hard-coded values.
	bNoError = ReadConfigurationFile();

	CMainFrame			*pFrame = new CMainFrame;

	if ( pFrame == 0 )
		bNoError = FALSE;
	
	if ( bNoError )
		{
		m_pMainWnd = pFrame;
		m_MainWindowClassName = AfxRegisterWndClass( CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW, ::LoadCursor( NULL, IDC_ARROW ),
											(HBRUSH)::GetStockObject( BLACK_BRUSH ), m_hApplicationIcon );
		pFrame -> CreateEx( WS_EX_APPWINDOW | WS_EX_OVERLAPPEDWINDOW | WS_EX_CONTEXTHELP, m_MainWindowClassName, "Service Controller",
									WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE |
									WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU,
									CRect( 0, 0, 800, 600 ), NULL, 0, NULL );
		if ( m_CommandLineInstructions != 0L )
			{
			if ( m_CommandLineInstructions & CMD_LINE_INSTALL )
				pFrame -> m_ControlPanel.OnButtonInstall();
			if ( m_CommandLineInstructions & CMD_LINE_START )
				pFrame -> m_ControlPanel.OnButtonStart();
			if ( m_CommandLineInstructions & CMD_LINE_RESTART )
				{
				pFrame -> m_ControlPanel.OnButtonStop();
				LogMessage( "The service will be restarted in 20 seconds.", MESSAGE_TYPE_NORMAL_LOG );
				Sleep( 20000 );		// Allow plenty of time for the service to wind down.
				pFrame -> m_ControlPanel.OnButtonStart();
				}
			if ( m_CommandLineInstructions & CMD_LINE_STOP )
				pFrame -> m_ControlPanel.OnButtonStop();
			if ( m_CommandLineInstructions & CMD_LINE_REMOVE )
				pFrame -> m_ControlPanel.OnButtonRemove();
			pFrame -> m_ControlPanel.OnAppExit();
			}
		else
			{
			pFrame -> ShowWindow( SW_SHOW );
			pFrame -> UpdateWindow();
			}
		}

	return bNoError;
}


int CServiceControllerApp::ExitInstance()
{
	CloseSoftwareModules();

	return CWinApp::ExitInstance();
}


// CAboutDlg dialog used for App About

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}


void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// App command to run the dialog
void CServiceControllerApp::OnAppAbout()
{
	CAboutDlg			aboutDlg;

	aboutDlg.DoModal();
}


void GetCmdLine()
{
	char			*pCmdLine;
	
	pCmdLine = GetCommandLine();
	LogMessage( pCmdLine, MESSAGE_TYPE_NORMAL_LOG );

	ServiceControllerApp.m_CommandLineInstructions = 0L;
	if ( strstr( pCmdLine, "/install" ) != 0 )
		ServiceControllerApp.m_CommandLineInstructions |= CMD_LINE_INSTALL;
	if ( strstr( pCmdLine, "/start" ) != 0 )
		ServiceControllerApp.m_CommandLineInstructions |= CMD_LINE_START;
	if ( strstr( pCmdLine, "/restart" ) != 0 )
		ServiceControllerApp.m_CommandLineInstructions |= CMD_LINE_RESTART;
	if ( strstr( pCmdLine, "/stop" ) != 0 )
		ServiceControllerApp.m_CommandLineInstructions |= CMD_LINE_STOP;
	if ( strstr( pCmdLine, "/remove" ) != 0 )
		ServiceControllerApp.m_CommandLineInstructions |= CMD_LINE_REMOVE;
}


void CServiceControllerApp::OnAppExit()
{
	CWinApp::OnAppExit();
}



