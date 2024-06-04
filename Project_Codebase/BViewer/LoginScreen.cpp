// LoginScreen.cpp : Implementation file for the login dialog box.
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
//	*[4] 05/07/2024 by Tom Atwood
//		Fixed a bug preventing access to the reader selection list.
//	*[3] 01/24/2024 by Tom Atwood
//		Converted user name field into a combo box.  Deleted unused function
//		OnEditLoginNameKillFocus().  Fixed a problem with cancelling a login.
//	*[2] 10/09/2023 by Tom Atwood
//		Added the READER_PERSONAL_INFO specification to the class declaration.
//	*[1] 02/15/2023 by Tom Atwood
//		Fixed code security issues.
//
//
#include "stdafx.h"
#include "BViewer.h"
#include "Module.h"
#include "ReportStatus.h"
#include "Configuration.h"
#include "Access.h"
#include "DiagnosticImage.h"
#include "Mouse.h"
#include "ImageView.h"
#include "MainFrm.h"
#include "ImageFrame.h"
#include "LoginScreen.h"


extern CBViewerApp				ThisBViewerApp;
extern CCustomization			BViewerCustomization;
extern LIST_HEAD				RegisteredUserList;


// CLoginScreen dialog
CLoginScreen::CLoginScreen( CWnd *pParent /*=NULL*/, READER_PERSONAL_INFO *pCurrReaderInfo ) : CDialog( CLoginScreen::IDD, pParent ),		// *[2] Added pCurrReaderInfo.
				m_StaticLoginBanner( "Welcome to BViewer Login!", 480, 40, 32, 16, 6, COLOR_CONFIG, COLOR_STANDARD, COLOR_STANDARD,
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE | CONTROL_VISIBLE,
								IDC_STATIC_LOGIN_BANNER ),
				m_StaticLoginTitle( "Please Enter Login\nName and Password.", 300, 60, 20, 10, 5, COLOR_WHITE, COLOR_STANDARD, COLOR_STANDARD,
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE | CONTROL_VISIBLE,
								IDC_STATIC_LOGIN_TITLE ),
				m_StaticLoginName( "Login Name", 120, 30, 18, 9, 5, COLOR_WHITE, COLOR_STANDARD, COLOR_STANDARD,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
								IDC_STATIC_LOGIN_NAME,
									"If you have supplied a login name with your previously\n"
									"entered reader information, enter it here.  (Your reader\n"
									"information is entered on the \"Set Up BViewer\" tab)." ),
				m_ComboBoxSelectReader( "", 220, 300, 18, 9, 5, VARIABLE_PITCH_FONT,														// *[3] Replaced single-user edit with combo box.
								COLOR_BLACK, COLOR_UNTOUCHED_LIGHT, COLOR_COMPLETED_LIGHT, COLOR_TOUCHED,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_VSCROLL | EDIT_BORDER | LIST_SORT | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_COMBO_SELECT_CURRENT_READER ),
				m_StaticLoginPassword( "Password", 100, 30, 18, 9, 5, COLOR_WHITE, COLOR_STANDARD, COLOR_STANDARD,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
								IDC_STATIC_LOGIN_PASSWORD,
									"If you have supplied a password with your previously\n"
									"entered reader information, enter it here.  (Your reader\n"
									"information is entered on the \"Set Up BViewer\" tab)" ),
				m_EditLoginPassword( "", 220, 30, 22, 11, 5, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_LOGIN_PASSWORD ),

				m_StaticErrorNotification( "", 370, 30, 18, 9, 5, COLOR_WHITE, COLOR_CANCEL, COLOR_CANCEL,
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_INVISIBLE,
								IDC_STATIC_LOGIN_ERROR_NOTICE ),
				m_ButtonLogin( "Log In", 120, 40, 18, 9, 6,
								COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								BUTTON_PUSHBUTTON | BUTTON_DEFAULT | CONTROL_VISIBLE |
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
								IDC_BUTTON_LOGIN ),
				m_ButtonCancelLogin( "Cancel", 120, 40, 18, 9, 6,
								COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								BUTTON_PUSHBUTTON | CONTROL_VISIBLE |
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
								IDC_BUTTON_CANCEL_LOGIN )

{
	m_BkgdBrush.CreateSolidBrush( COLOR_CONFIG );
	m_bUserRecognized = FALSE;
	m_bAccessGranted = FALSE;
	m_bUserRecognizedOnLastPass = TRUE;
	m_bAccessGrantedOnLastPass = TRUE;
	m_bLoginCancelled = FALSE;					// *[3] Added cancellation flag initialization.
	m_NumberOfRegisteredUsers = 0;
	m_pControlTip = 0;
	m_pCurrReaderInfo = pCurrReaderInfo;		// *[2] Added m_pCurrReaderInfo.
}


CLoginScreen::~CLoginScreen()
{
	if ( m_pControlTip != 0 )
		{
		delete m_pControlTip;
		m_pControlTip = 0;
		}
	DestroyWindow();
}


BEGIN_MESSAGE_MAP( CLoginScreen, CDialog )
	//{{AFX_MSG_MAP(CLoginScreen)
	ON_CBN_SELENDOK( IDC_COMBO_SELECT_CURRENT_READER, OnReaderSelected )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_LOGIN_PASSWORD, OnEditLoginPasswordKillFocus )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_LOGIN, OnBnClickedLogin )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_CANCEL_LOGIN, OnBnClickedCancelLogin )
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CLoginScreen::OnInitDialog()
{
	RECT			ClientRect;
	INT				ClientWidth;
	static char		TextString[ 64 ];

	CDialog::OnInitDialog();

	SetDefID( IDC_BUTTON_LOGIN );	// Set default button.

	GetClientRect( &ClientRect );
	ClientWidth = ClientRect.right - ClientRect.left;

	m_StaticLoginBanner.SetPosition( 15, 30, this );
	m_StaticLoginTitle.SetPosition( 180, 120, this );
	m_StaticLoginName.SetPosition( 70, 200, this );
	m_ComboBoxSelectReader.SetPosition( ClientWidth - 220 - 70, 200, this );		// *[3] Replaced single-user edit with combo box.

	m_StaticLoginPassword.SetPosition( 70, 240, this );
	m_EditLoginPassword.SetPosition( ClientWidth - 220 - 70, 240, this );
	m_EditLoginPassword.SetWindowText( "" );										// *[2] Initialize password edit box.
	m_EditLoginPassword.SetPasswordChar( '*' );
	m_StaticErrorNotification.SetPosition( 70, 290, this );
	m_ButtonLogin.SetPosition( 100, 340, this );
	m_ButtonCancelLogin.SetPosition( ClientWidth -120 - 100, 340, this );
	
	SetIcon( ThisBViewerApp.m_hApplicationIcon, FALSE );
	
	if ( !m_bUserRecognizedOnLastPass )
		{
		strncpy_s( TextString, 64, "Invalid User Name", _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
		m_StaticErrorNotification.m_ControlText = TextString;
		m_StaticErrorNotification.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
		}
	else if ( !m_bAccessGrantedOnLastPass )
		{
		strncpy_s( TextString, 64, "Invalid Password", _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
		m_StaticErrorNotification.m_ControlText = TextString;
		m_StaticErrorNotification.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
		}
	InitializeControlTips();
	
	LoadReaderSelectionList();														// *[3] Replaced single-user edit with combo box.

	m_EditLoginPassword.SetFocus();													// *[3] Replaced single-user edit with combo box.

	return FALSE;
}


static void ControlTipActivationFunction( CWnd *pDialogWindow, char *pTipText, CPoint MouseCursorLocation )
{
	CLoginScreen			*pLoginScreen;

	pLoginScreen = (CLoginScreen*)pDialogWindow;
	if ( pLoginScreen != 0 )
		{
		// If there has been a change in the tip text, reset the tip display window.
		if ( pTipText != 0 && strlen( pTipText ) > 0 && pTipText != pLoginScreen -> m_pControlTip -> m_pTipText &&
																	pLoginScreen -> m_pControlTip -> m_pTipText != 0 )
			{
			pLoginScreen -> m_pControlTip -> ShowWindow( SW_HIDE );
			pLoginScreen -> m_pControlTip -> m_pTipText = pTipText;
			pLoginScreen -> m_pControlTip -> ShowTipText( MouseCursorLocation, pLoginScreen );
			}
		else if ( pTipText == 0 )
			pLoginScreen -> m_pControlTip -> ShowWindow( SW_HIDE );
		else
			{
			pLoginScreen -> m_pControlTip -> m_pTipText = pTipText;
			pLoginScreen -> m_pControlTip -> ShowTipText( MouseCursorLocation, pLoginScreen );
			}
		}
}


void CLoginScreen::InitializeControlTips()
{
	CWnd					*pChildWindow;
	CRuntimeClass			*pRuntimeClassInfo;

	m_pControlTip = new CControlTip();
	if ( m_pControlTip != 0 )
		{
		m_pControlTip -> ActivateTips();
		pChildWindow = GetWindow( GW_CHILD );		// Get the first child window.
		while ( pChildWindow != NULL )
			{
			pRuntimeClassInfo = pChildWindow -> GetRuntimeClass();
			if ( pRuntimeClassInfo != 0 && 
						( _stricmp( pRuntimeClassInfo -> m_lpszClassName, "TomButton" ) == 0 ||
						_stricmp( pRuntimeClassInfo -> m_lpszClassName, "TomStatic" ) == 0 ) )
				if ( ( (TomButton*)pChildWindow ) -> IsVisible() )
					( (TomButton*)pChildWindow ) -> m_ControlTipActivator = ControlTipActivationFunction;
			pChildWindow = pChildWindow -> GetWindow( GW_HWNDNEXT );
			}
		}
}


// *[3] Added tis function as part of the replacement of single-user edit with combo box.
BOOL CLoginScreen::LoadReaderSelectionList()
{
	BOOL					bNoError = TRUE;
	LIST_ELEMENT			*pReaderListElement;
	READER_PERSONAL_INFO	*pReaderInfo;
	int						nItemIndex = 0;				// *[3] Initialize variable.
	int						nSelectedItem;

	m_ComboBoxSelectReader.ResetContent();
	m_ComboBoxSelectReader.SetWindowTextA( "Registered Readers" );
	pReaderListElement = RegisteredUserList;
	nSelectedItem = 0;
	while ( pReaderListElement != 0 )
		{
		pReaderInfo = (READER_PERSONAL_INFO*)pReaderListElement -> pItem;
		nItemIndex = m_ComboBoxSelectReader.AddString( pReaderInfo -> LoginName );

		if ( pReaderInfo -> IsDefaultReader )
			memcpy( &m_DefaultReaderInfo, pReaderInfo, sizeof( READER_PERSONAL_INFO ) );

		if ( strcmp( pReaderInfo -> ReportSignatureName, BViewerCustomization.m_ReaderInfo.ReportSignatureName ) == 0 )
			nSelectedItem = nItemIndex;
		m_ComboBoxSelectReader.SetItemDataPtr( nItemIndex, (void*)pReaderInfo );
		pReaderListElement = pReaderListElement -> pNextListElement;
		}
	m_ComboBoxSelectReader.SetCurSel( nSelectedItem );

	return bNoError;
}


// *[3] Added tis function as part of the replacement of single-user edit with combo box.
void CLoginScreen::OnReaderSelected()
{
	READER_PERSONAL_INFO	*pReaderInfo;
	int						nItemIndex;

	nItemIndex = m_ComboBoxSelectReader.GetCurSel();
	m_nSelectedReaderItem = nItemIndex;
	pReaderInfo = (READER_PERSONAL_INFO*)m_ComboBoxSelectReader.GetItemDataPtr( nItemIndex );
	ClearDefaultReaderFlag();
	pReaderInfo -> IsDefaultReader = TRUE;
	memcpy( (void*)&m_DefaultReaderInfo, pReaderInfo, sizeof( READER_PERSONAL_INFO ) );
}


// *[3] Added tis function as part of the replacement of single-user edit with combo box.
void CLoginScreen::ClearDefaultReaderFlag()
{
	LIST_ELEMENT			*pReaderListElement;
	READER_PERSONAL_INFO	*pReaderInfo;
	
	memset( (void*)&m_DefaultReaderInfo, 0, sizeof(READER_PERSONAL_INFO) );
	pReaderListElement = RegisteredUserList;
	while ( pReaderListElement != 0 )
		{
		pReaderInfo = (READER_PERSONAL_INFO*)pReaderListElement -> pItem;
		pReaderInfo -> IsDefaultReader = FALSE;
		pReaderListElement = pReaderListElement -> pNextListElement;
		}
}


void CLoginScreen::OnEditLoginPasswordKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{															// *[2] Removed unnecessary password GetWindowText() call.
	CWnd		*pWindow;

	pWindow = GetFocus();									// *[4] Disable automatic exit if the user has
	if ( pWindow != (CWnd*)&m_ComboBoxSelectReader )		// *[4]  clicked on the reader selection combo box.
		{
		m_EditLoginPassword.Invalidate( TRUE );
		m_ButtonLogin.SetFocus();
		OnBnClickedLogin( pNMHDR, pResult );
		}
	*pResult = 0;
}


// Note:  The user list must be read in before the login screen is created.
void CLoginScreen::OnBnClickedLogin( NMHDR *pNMHDR, LRESULT *pResult )
{
	char					TextString[ ( 2 * MAX_USER_INFO_LENGTH ) + 1 ];		// *[2] Add allowance for null string terminator.
	LIST_ELEMENT			*pUserListElement;

	m_ButtonLogin.HasBeenPressed( TRUE );
	m_NumberOfRegisteredUsers = BViewerCustomization.m_NumberOfRegisteredUsers;
	// Verify the user name.
	m_bAccessGranted = FALSE;
	// Read the pw text that was typed in.
	m_ComboBoxSelectReader.GetWindowText( TextString, MAX_USER_INFO_LENGTH );	// *[3] Replaced single-user edit with combo box.

	// Loop through the user list to try to locate the unique user name.
	m_bUserRecognized = FALSE;
	pUserListElement = RegisteredUserList;
	while ( pUserListElement != 0 && !m_bUserRecognized )
		{
		m_pCurrReaderInfo = (READER_PERSONAL_INFO*)pUserListElement -> pItem;
		m_bUserRecognized = ( strcmp( TextString, m_pCurrReaderInfo -> LoginName ) == 0 );
		if ( !m_bUserRecognized )
			pUserListElement = pUserListElement -> pNextListElement;
		}
	if ( m_bUserRecognized )
		memcpy( &BViewerCustomization.m_ReaderInfo, (void*)m_pCurrReaderInfo, sizeof( READER_PERSONAL_INFO ) );
	if ( RegisteredUserList == 0 )
		{
		m_bAccessGranted = TRUE;
		m_pCurrReaderInfo = (READER_PERSONAL_INFO*)calloc( 1, sizeof( READER_PERSONAL_INFO ) );
		if ( m_pCurrReaderInfo != 0 )
			{
			m_NumberOfRegisteredUsers = 1;
			strncpy_s( m_pCurrReaderInfo -> LoginName, MAX_USER_INFO_LENGTH, TextString, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
			if ( strlen( m_pCurrReaderInfo -> LoginName ) > 0 )
				AppendToList( &RegisteredUserList, (void*)m_pCurrReaderInfo );
			}
		}
	// Authenticate.
	if ( m_bUserRecognized )
		{
		// Read the pw text that was typed in.
		m_EditLoginPassword.GetWindowText( TextString, MAX_USER_INFO_LENGTH );
		if ( !BViewerCustomization.m_ReaderInfo.bPasswordEntered && strlen( TextString ) == 0 )
			m_bAccessGranted = TRUE;
		else
			m_bAccessGranted = GrantAccess( &BViewerCustomization.m_ReaderInfo, TextString );
		}
	*pResult = 0;

	CDialog::OnOK();
}


void CLoginScreen::OnBnClickedCancelLogin( NMHDR *pNMHDR, LRESULT *pResult )
{
	m_ButtonCancelLogin.HasBeenPressed( TRUE );
	m_bLoginCancelled = TRUE;
	CDialog::OnCancel();
}


BOOL CLoginScreen::CertifyLogin()
{
	return m_bAccessGranted;
}


void CLoginScreen::OnMouseMove( UINT nFlags, CPoint MouseCursorLocation )
{
	// If the window owning the controls (this window) receives a WM_MOUSEMOVE message, this
	// indicates that the mouse is not over any of the controls.  Therefore, disable the
	// control tip.
	if ( m_pControlTip -> m_pTipText != 0 && strlen( m_pControlTip -> m_pTipText ) > 0 )
		{
		m_pControlTip -> m_pTipText = "";
		m_pControlTip -> ShowTipText( MouseCursorLocation, this );
		}
	
	CDialog::OnMouseMove( nFlags, MouseCursorLocation );
}


HBRUSH CLoginScreen::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
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


BOOL CLoginScreen::OnEraseBkgnd( CDC *pDC )
{
	CBrush		BackgroundBrush( COLOR_STANDARD );
	CRect		BackgroundRectangle;
	CBrush		*pOldBrush = pDC -> SelectObject( &BackgroundBrush );

	GetClientRect( BackgroundRectangle );
	pDC -> FillRect( BackgroundRectangle, &BackgroundBrush );
	pDC -> SelectObject( pOldBrush );

	return TRUE;
}


