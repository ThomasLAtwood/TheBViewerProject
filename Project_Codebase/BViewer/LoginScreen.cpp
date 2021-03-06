// LoginScreen.cpp : Implementation file for the login dialog box.
//
//	Written by Thomas L. Atwood
//	P.O. Box 1089
//	West Fork, Arkansas 72774
//	(479)445-4690
//	TomAtwood@Earthlink.net
//
//	Copyright � 2010 CDC
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
extern READER_PERSONAL_INFO		*pCurrentReaderInfo;		// Points at item in user list that matches login.


// CLoginScreen dialog
CLoginScreen::CLoginScreen( CWnd *pParent /*=NULL*/ ) : CDialog( CLoginScreen::IDD, pParent ),
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
				m_EditLoginName( "", 220, 30, 22, 11, 5, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_LOGIN_NAME ),
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
	m_NumberOfRegisteredUsers = 0;
	m_pControlTip = 0;
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
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_LOGIN_NAME, OnEditLoginNameKillFocus )
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
	INT				ClientHeight;
	static char		TextString[ 64 ];

	CDialog::OnInitDialog();

	SetDefID( IDC_BUTTON_LOGIN );	// Set default button.

	GetClientRect( &ClientRect );
	ClientWidth = ClientRect.right - ClientRect.left;
	ClientHeight = ClientRect.bottom - ClientRect.top;

	m_StaticLoginBanner.SetPosition( 15, 30, this );
	m_StaticLoginTitle.SetPosition( 180, 120, this );
	m_StaticLoginName.SetPosition( 70, 200, this );
	m_EditLoginName.SetPosition( ClientWidth - 220 - 70, 200, this );
	m_StaticLoginPassword.SetPosition( 70, 240, this );
	m_EditLoginPassword.SetPosition( ClientWidth - 220 - 70, 240, this );
	m_EditLoginPassword.SetPasswordChar( '*' );
	m_StaticErrorNotification.SetPosition( 70, 290, this );
	m_ButtonLogin.SetPosition( 100, 340, this );
	m_ButtonCancelLogin.SetPosition( ClientWidth -120 - 100, 340, this );
	
	SetIcon( ThisBViewerApp.m_hApplicationIcon, FALSE );
	
	if ( !m_bUserRecognizedOnLastPass )
		{
		strcpy( TextString, "Invalid User Name" );
		m_StaticErrorNotification.m_ControlText = TextString;
		m_StaticErrorNotification.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
		}
	else if ( !m_bAccessGrantedOnLastPass )
		{
		strcpy( TextString, "Invalid Password" );
		m_StaticErrorNotification.m_ControlText = TextString;
		m_StaticErrorNotification.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
		}
	m_EditLoginName.SetFocus();
	InitializeControlTips();
	
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


void CLoginScreen::OnEditLoginNameKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	char				TextString[ 64 ];

	m_EditLoginName.GetWindowText( TextString, 64 );
	m_EditLoginName.Invalidate( TRUE );
	if ( strlen( TextString ) > 0 )
		m_EditLoginPassword.SetFocus();

	*pResult = 0;
}


void CLoginScreen::OnEditLoginPasswordKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	char				TextString[ 2 * MAX_USER_INFO_LENGTH ];

	m_EditLoginPassword.GetWindowText( TextString, 2 * MAX_USER_INFO_LENGTH );
	m_EditLoginPassword.Invalidate( TRUE );
	m_ButtonLogin.SetFocus();

	*pResult = 0;
}


// Note:  The user list must be read in before the login screen is created.
void CLoginScreen::OnBnClickedLogin( NMHDR *pNMHDR, LRESULT *pResult )
{
	char					TextString[ 2 * MAX_USER_INFO_LENGTH ];
	LIST_ELEMENT			*pUserListElement;

	m_ButtonLogin.HasBeenPressed( TRUE );
	m_NumberOfRegisteredUsers = BViewerCustomization.m_NumberOfRegisteredUsers;
	// Verify the user name.
	m_bAccessGranted = FALSE;
	m_EditLoginName.GetWindowText( TextString, MAX_USER_INFO_LENGTH );
	// Loop through the user list to try to locate the unique user name.
	m_bUserRecognized = FALSE;
	pUserListElement = RegisteredUserList;
	while ( pUserListElement != 0 && !m_bUserRecognized )
		{
		pCurrentReaderInfo = (READER_PERSONAL_INFO*)pUserListElement -> pItem;
		m_bUserRecognized = ( strcmp( TextString, pCurrentReaderInfo -> LoginName ) == 0 );
		if ( !m_bUserRecognized )
			pUserListElement = pUserListElement -> pNextListElement;
		}
	if ( m_bUserRecognized )
		memcpy( &BViewerCustomization.m_ReaderInfo, (void*)pCurrentReaderInfo, sizeof( READER_PERSONAL_INFO ) );
	if ( RegisteredUserList == 0 )
		{
		m_bAccessGranted = TRUE;
		pCurrentReaderInfo = (READER_PERSONAL_INFO*)calloc( 1, sizeof( READER_PERSONAL_INFO ) );
		if ( pCurrentReaderInfo != 0 )
			{
			m_NumberOfRegisteredUsers = 1;
			strcpy( pCurrentReaderInfo -> LoginName, TextString );
			if ( strlen( pCurrentReaderInfo -> LoginName ) > 0 )
				AppendToList( &RegisteredUserList, (void*)pCurrentReaderInfo );
			}
		}
	// Check the password.
	if ( m_bUserRecognized )
		{
		m_EditLoginPassword.GetWindowText( TextString, 2 * MAX_USER_INFO_LENGTH );
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


