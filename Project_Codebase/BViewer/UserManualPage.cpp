// UserManualPage.cpp : Implementation file for the CUserManualPage class of
//  CPropertyPage, which implements the "View User Manual" tab of the main Control Panel.
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
//	*[2] 03/14/2023 by Tom Atwood
//		Fixed code security issues.
//	*[1] 02/16/2023 by Tom Atwood
//		Fixed code security issues.
//
//
#include "stdafx.h"
#include "BViewer.h"
#include "Module.h"
#include "ReportStatus.h"
#include "UserManualPage.h"
#include "ControlPanel.h"
#include <Htmlhelp.h>


extern CONFIGURATION				BViewerConfiguration;

//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		HelpModuleInfo = { MODULE_HELP, "Help Module", InitHelpModule, CloseHelpModule };


static ERROR_DICTIONARY_ENTRY	HelpErrorCodes[] =
			{
				{ HELP_ERROR_INSUFFICIENT_MEMORY	, "An error occurred allocating a memory block for data storage." },
				{ HELP_ERROR_FILE_NOT_FOUND			, "The help file could not be found." },
				{ 0									, NULL }
			};

static ERROR_DICTIONARY_MODULE		HelpStatusErrorDictionary =
										{
										MODULE_HELP,
										HelpErrorCodes,
										HELP_ERROR_DICT_LENGTH,
										0
										};

static DWORD		HTMLHelpCookie = NULL;

// This function must be called before any other function in this module.
void InitHelpModule()
{
	LinkModuleToList( &HelpModuleInfo );
	RegisterErrorDictionary( &HelpStatusErrorDictionary );

	::HtmlHelp( NULL, NULL, HH_INITIALIZE, (DWORD_PTR)&HTMLHelpCookie ) ; // Cookie returned by Hhctrl.ocx.
}


void CloseHelpModule()
{
	::HtmlHelp( NULL, NULL, HH_UNINITIALIZE, (DWORD_PTR)&HTMLHelpCookie ) ; // Cookie returned by Hhctrl.ocx.
}


// CUserManualPage dialog
CUserManualPage::CUserManualPage() : CPropertyPage( CUserManualPage::IDD )
{
	m_bHelpWindowIsOpen = FALSE;
	m_hHelpWindow = 0;
	m_bHelpPageIsActivated = FALSE;
}


CUserManualPage::~CUserManualPage()
{
}


BEGIN_MESSAGE_MAP( CUserManualPage, CPropertyPage )
	//{{AFX_MSG_MAP(CUserManualPage)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CUserManualPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	if ( !m_bHelpWindowIsOpen )
		OpenHelpFile();
	m_bHelpPageIsActivated = TRUE;

	return FALSE;
}


BOOL CUserManualPage::OnSetActive()
{
	CControlPanel		*pControlPanel;

	pControlPanel = (CControlPanel*)GetParent();
	if ( pControlPanel != 0 )
		pControlPanel -> m_CurrentlyActivePage = USER_MANUAL_PAGE;

	return CPropertyPage::OnSetActive();
}


void CUserManualPage::OpenHelpFile()
{
	RECT			HelpWindowRect;
	CWnd			*pHelpWindow;
	char			HelpFileSpec[ FILE_PATH_STRING_LENGTH ];
	
	if ( !m_bHelpWindowIsOpen )
		{
		strncpy_s( HelpFileSpec, FILE_PATH_STRING_LENGTH, BViewerConfiguration.ProgramPath, _TRUNCATE );		// *[2] Replaced strncat with strncpy_s.
		if ( HelpFileSpec[ strlen( HelpFileSpec ) - 1 ] != '\\' )
			strncat_s( HelpFileSpec, FILE_PATH_STRING_LENGTH, "\\", _TRUNCATE );								// *[2] Replaced strcat with strncat_s.
		if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
			strncat_s( HelpFileSpec, FILE_PATH_STRING_LENGTH, "Docs\\BViewerGP.chm", _TRUNCATE );				// *[2] Replaced strcat with strncat_s.
		else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
			strncat_s( HelpFileSpec, FILE_PATH_STRING_LENGTH, "Docs\\BViewer.chm", _TRUNCATE );					// *[2] Replaced strcat with strncat_s.

		GetClientRect( &HelpWindowRect );
		HelpWindowRect.top += 7;
		HelpWindowRect.bottom -= 7;
		HelpWindowRect.left += 7;
		HelpWindowRect.right -= 7;

		memset( (void*)&m_HelpWindowType, 0, sizeof( HH_WINTYPE ) );
		m_HelpWindowType.cbStruct = sizeof( m_HelpWindowType );
		if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
			m_HelpWindowType.pszType = "BViewerGPType";
		else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
			m_HelpWindowType.pszType = "BViewerType";

		m_HelpWindowType.fsValidMembers =	HHWIN_PARAM_EXSTYLES |		// The dwExStyles member is valid.  
												HHWIN_PARAM_PROPERTIES |	// The fsWinProperties member is valid.  
												HHWIN_PARAM_STYLES |		// The dwStyles member is valid.  
												HHWIN_PARAM_RECT |			// The rcWindowPos member is valid.  
												HHWIN_PARAM_SHOWSTATE |		// The nShowState member is valid.  
												HHWIN_PARAM_TB_FLAGS |
												HHWIN_PARAM_TABPOS |
												HHWIN_PARAM_EXPANSION |
												HHWIN_PARAM_NAV_WIDTH |
												HHWIN_PARAM_CUR_TAB;

		m_HelpWindowType.fsWinProperties =	HHWIN_PROP_NOTITLEBAR |		// Creates a window without a title bar.
												HHWIN_PROP_TAB_ADVSEARCH |	// Creates a window whose Navigation pane includes a full-text Search tab.
												HHWIN_PROP_TAB_SEARCH |		// Creates a window whose Navigation pane includes a Search tab.
												HHWIN_PROP_TRI_PANE |		// Creates the HTML Help Viewer (with a Topic pane, Navigation pane, and toolbar).
												HHWIN_PROP_TRACKING |
												HHWIN_PROP_NODEF_STYLES |
												HHWIN_PROP_NODEF_EXSTYLES |
												HHWIN_PROP_AUTO_SYNC;

	m_HelpWindowType.pszCaption = "BViewer Help";
		m_HelpWindowType.dwStyles = WS_CHILD | WS_VISIBLE;
		m_HelpWindowType.dwExStyles = WS_EX_CLIENTEDGE;
		m_HelpWindowType.rcWindowPos = HelpWindowRect;
		m_HelpWindowType.nShowState = SW_SHOW;
		m_HelpWindowType.hwndHelp = 0;
		m_HelpWindowType.hwndCaller = GetSafeHwnd();
		m_HelpWindowType.idNotify = IDC_HELP_CONTROL;
		m_HelpWindowType.fsToolBarFlags =	HHWIN_BUTTON_BACK | HHWIN_BUTTON_FORWARD |
												HHWIN_BUTTON_PRINT | HHWIN_BUTTON_REFRESH |
												HHWIN_BUTTON_ZOOM | HHWIN_BUTTON_EXPAND;
		if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
			{
			m_HelpWindowType.pszToc = "BViewerGP.chm::/Table of Contents.hhc";     
			m_HelpWindowType.pszIndex = "BViewerGP.chm::/Index.hhk";   
			m_HelpWindowType.pszHome = "BViewerGP.chm::/TopicMain.htm";
			}
		else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
			{
			m_HelpWindowType.pszToc = "BViewer.chm::/Table of Contents.hhc";     
			m_HelpWindowType.pszIndex = "BViewer.chm::/Index.hhk";   
			m_HelpWindowType.pszHome = "BViewer.chm::/TopicMain.htm";
			}
		m_HelpWindowType.pszFile = "TopicMain.htm";    
		m_HelpWindowType.fNotExpanded = FALSE;
		m_HelpWindowType.curNavType = HHWIN_NAVTYPE_TOC;
		m_HelpWindowType.iNavWidth = 175;
		m_HelpWindowType.tabpos = HHWIN_NAVTAB_TOP;

		::HtmlHelp( GetSafeHwnd(), HelpFileSpec, HH_SET_WIN_TYPE, (DWORD_PTR)&m_HelpWindowType );			// *[2] Removed unused return value assignment.

		if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
			m_hHelpWindow = ::HtmlHelp( GetSafeHwnd(), "BViewerGP.chm::/TopicMain.htm>BViewerGPType", HH_DISPLAY_TOPIC, 0 );
		else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
			m_hHelpWindow = ::HtmlHelp( GetSafeHwnd(), "BViewer.chm::/TopicMain.htm>BViewerType", HH_DISPLAY_TOPIC, 0 );
		if ( m_hHelpWindow != 0 )
			{
			m_HelpWindowType.hwndHelp = m_hHelpWindow;
			pHelpWindow = FromHandle( m_hHelpWindow );
			if ( pHelpWindow != 0 )
				{
				pHelpWindow -> Invalidate( TRUE );
				pHelpWindow -> SetFocus();
				}
			}
		else
			{
			ThisBViewerApp.NotifyUserOfImageFileError( HELP_ERROR_FILE_NOT_FOUND, "An error occurred opening\nthe help file.\n\n", "File not found in BViewer\\Docs" );
			RespondToError( MODULE_HELP, HELP_ERROR_FILE_NOT_FOUND );
			}
		m_bHelpWindowIsOpen = TRUE;
		}
}


void CUserManualPage::CloseHelpFile()
{
	CWnd		*pHelpWindow;

	if ( m_bHelpWindowIsOpen )
		{
		if ( m_hHelpWindow != 0 )
			{
			pHelpWindow = FromHandle( m_hHelpWindow );
			if ( pHelpWindow != 0 )
				pHelpWindow -> SendMessage( WM_CLOSE, 0, 0 );
			}
		}
	m_bHelpWindowIsOpen = FALSE;
}


void CUserManualPage::OnSize( UINT nType, int cx, int cy )
{
	CPropertyPage::OnSize( nType, cx, cy );

	if ( m_bHelpPageIsActivated )
		{
		if ( m_bHelpWindowIsOpen )
			CloseHelpFile();
		if ( !m_bHelpWindowIsOpen )
			OpenHelpFile();
		}
}


BOOL CUserManualPage::OnNotify( WPARAM wParam, LPARAM lParam, LRESULT *pResult )
{
	// This function override is required to handle windows message notifications
	// from some of the child controls.
	return CPropertyPage::OnNotify( wParam, lParam, pResult );
}


BOOL CUserManualPage::PreTranslateMessage( MSG *pMsg )
{
	HWND			hResult;
	BOOL			bMessageWasTranslated;
	
	hResult = ::HtmlHelp( NULL, NULL, HH_PRETRANSLATEMESSAGE, (DWORD_PTR)pMsg );
	bMessageWasTranslated = ( hResult != 0 );
	
	return bMessageWasTranslated;
}

