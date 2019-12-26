// ReaderInfoScreen.cpp : Implementation file for the reader information dialog
//  box.  This is where the user enters his or her identifying information.
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
#include "ReaderInfoScreen.h"


extern CBViewerApp				ThisBViewerApp;
extern CONFIGURATION			BViewerConfiguration;
extern CCustomization			BViewerCustomization;


// CReaderInfoScreen dialog
CReaderInfoScreen::CReaderInfoScreen( CWnd *pParent /*=NULL*/ )
			: CDialog( CReaderInfoScreen::IDD, pParent ),
				m_StaticReaderIdentification( "Reader Identification", 250, 50, 18, 9, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_TOP_JUSTIFIED | CONTROL_VISIBLE,
										IDC_STATIC_READER_IDENTIFICATION ),
				m_StaticReaderLastName( "Last Name (Family Name)", 200, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_READER_LAST_NAME ),
				m_EditReaderLastName( "", 120, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_READER_LAST_NAME ),

				m_StaticLoginName( "Login Name", 150, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_LOGIN_NAME ),
				m_EditLoginName( "", 120, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_LOGIN_NAME ),

				m_StaticReaderID( "ID", 300, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_READER_SSN ),
				m_EditReaderID( "", 120, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_READER_ID ),

				m_StaticLoginPassword( "Login Password", 150, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_LOGIN_PASSWORD ),
				m_EditLoginPassword( "", 120, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_LOGIN_PASSWORD ),

				m_StaticReaderInitials( "Initials", 100, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_READER_INITIALS ),
				m_EditReaderInitials( "", 70, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_READER_INITIALS ),

				m_StaticAE_Title( "Local Dicom Name\n   (AE_TITLE)", 150, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE | CONTROL_MULTILINE,
										IDC_STATIC_AE_TITLE ),
				m_EditAE_Title( "", 120, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_AE_TITLE ),

				m_StaticReaderReportSignatureName( "Signature Name for Report ", 200, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_READER_SIGNATURE_NAME,
										"This will appear on the report." ),
				m_EditReaderReportSignatureName( "", 460, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_NONE, IDC_EDIT_READER_SIGNATURE_NAME ),

				m_StaticReaderStreetAddress( "Street Address", 150, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_READER_STREET_ADDRESS ),
				m_EditReaderStreetAddress( "", 300, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_READER_STREET_ADDRESS ),

				m_StaticReaderCity( "City", 100, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_READER_CITY ),
				m_EditReaderCity( "", 200, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_READER_CITY ),

				m_StaticReaderState( "State", 60, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_READER_STATE ),
				m_EditReaderState( "", 50, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_READER_STATE ),

				m_StaticReaderZipCode( "Zip Code", 100, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_READER_ZIPCODE ),
				m_EditReaderZipCode( "123", 120, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_READER_ZIPCODE ),

				m_GroupEditSequencing( GROUP_EDIT, GROUP_SEQUENCING, 11,
									&m_EditReaderLastName, &m_EditLoginName, &m_EditReaderID, &m_EditLoginPassword,
									&m_EditReaderInitials, &m_EditAE_Title, &m_EditReaderReportSignatureName,
									&m_EditReaderStreetAddress, &m_EditReaderCity, &m_EditReaderState, &m_EditReaderZipCode ),

				m_ButtonSave( "Save Reader\nIdentification", 150, 40, 16, 8, 6,
								COLOR_BLACK, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR,
								BUTTON_PUSHBUTTON | CONTROL_VISIBLE | CONTROL_MULTILINE |
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
								IDC_BUTTON_SAVE_READER_INFO ),
				m_ButtonCancel( "Cancel", 150, 40, 16, 8, 6,
								COLOR_BLACK, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR,
								BUTTON_PUSHBUTTON | CONTROL_VISIBLE |
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
								IDC_BUTTON_CANCEL_READER_INFO )
{
	m_BkgdBrush.CreateSolidBrush( COLOR_CONFIG );
}


CReaderInfoScreen::~CReaderInfoScreen()
{
	DestroyWindow();
}


BEGIN_MESSAGE_MAP( CReaderInfoScreen, CDialog )
	//{{AFX_MSG_MAP(CReaderInfoScreen)
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_SAVE_READER_INFO, OnBnClickedSaveReaderInfo )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_CANCEL_READER_INFO, OnBnClickedCancelReaderInfo )
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CReaderInfoScreen::OnInitDialog()
{
	RECT			ClientRect;
	INT				ClientWidth;
	INT				ClientHeight;
	static char		TextString[ 64 ];
	int				PrimaryScreenWidth;
	int				PrimaryScreenHeight;

	CDialog::OnInitDialog();

	GetClientRect( &ClientRect );
	ClientWidth = ClientRect.right - ClientRect.left;
	ClientHeight = ClientRect.bottom - ClientRect.top;

	m_StaticReaderIdentification.SetPosition( 90, 30, this );
	m_StaticReaderLastName.SetPosition( 40, 70, this );
	m_EditReaderLastName.SetPosition( 240, 70, this );

	m_StaticLoginName.SetPosition( 420, 70, this );
	m_EditLoginName.SetPosition( 580, 70, this );
	
	m_StaticLoginPassword.SetPosition( 420, 100, this );
	m_EditLoginPassword.SetPosition( 580, 100, this );
	m_EditLoginPassword.SetPasswordChar( '*' );
	
	m_StaticAE_Title.SetPosition( 420, 130, this );
	m_EditAE_Title.SetPosition( 580, 130, this );
	
	if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_GENERAL )
		{
		m_StaticReaderID.SetPosition( 40, 100, this );
		m_EditReaderID.SetPosition( 240, 100, this );

		m_StaticReaderInitials.SetPosition( 40, 130, this );
		m_EditReaderInitials.SetPosition( 240, 130, this );

		m_StaticReaderReportSignatureName.SetPosition( 40, 180, this );
		m_StaticReaderReportSignatureName.m_ControlText = "Name (Last, First, Middle)";
		m_StaticReaderReportSignatureName.Invalidate();
		m_EditReaderReportSignatureName.SetPosition( 240, 180, this );
		}
	else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
		{
		m_StaticReaderReportSignatureName.SetPosition( 40, 180, this );
		m_EditReaderReportSignatureName.SetPosition( 240, 180, this );
		}

	m_StaticReaderStreetAddress.SetPosition( 40, 210, this );
	m_EditReaderStreetAddress.SetPosition( 200, 210, this );

	m_StaticReaderCity.SetPosition( 40, 240, this );
	m_EditReaderCity.SetPosition( 200, 240, this );

	m_StaticReaderState.SetPosition( 40, 270, this );
	m_EditReaderState.SetPosition( 200, 270, this );

	m_StaticReaderZipCode.SetPosition( 40, 300, this );
	m_EditReaderZipCode.SetPosition( 200, 300, this );

	m_ButtonSave.SetPosition( 380, 300, this );
	m_ButtonCancel.SetPosition( 550, 300, this );

	PrimaryScreenWidth = ::GetSystemMetrics( SM_CXSCREEN );
	PrimaryScreenHeight = ::GetSystemMetrics( SM_CYSCREEN );

	m_EditReaderLastName.SetWindowText( "" );
	m_EditLoginName.SetWindowText( "" );
	if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
		m_EditReaderReportSignatureName.SetWindowText( "" );
	else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
		{
		m_EditReaderID.SetWindowText( "" );
		m_EditReaderInitials.SetWindowText( "" );
		m_EditReaderReportSignatureName.SetWindowText( "" );
		}

	if ( BViewerCustomization.m_ReaderInfo.bPasswordEntered )
		m_EditLoginPassword.SetWindowText( "************" );
	else
		m_EditLoginPassword.SetWindowText( "" );
	m_EditAE_Title.SetWindowText( "" );
	m_EditReaderStreetAddress.SetWindowText( "" );
	m_EditReaderCity.SetWindowText( "" );
	m_EditReaderState.SetWindowText( "" );
	m_EditReaderZipCode.SetWindowText( "" );

	SetWindowPos( &wndTop, ( PrimaryScreenWidth - 750 ) / 2, ( PrimaryScreenHeight - 350 ) / 2, 750, 380, SWP_SHOWWINDOW );

	return TRUE; 
}


void CReaderInfoScreen::OnBnClickedSaveReaderInfo( NMHDR *pNMHDR, LRESULT *pResult )
{
	char							TextString[ 64 ];
 	CMainFrame						*pMainFrame;
	static USER_NOTIFICATION_INFO	UserNotificationInfo;

	m_EditReaderLastName.GetWindowText( TextString, 64 );
	strcpy( BViewerCustomization.m_ReaderInfo.LastName, TextString );

	m_EditLoginName.GetWindowText( TextString, 64 );
	strcpy( BViewerCustomization.m_ReaderInfo.LoginName, TextString );
	BViewerCustomization.m_ReaderInfo.bLoginNameEntered = ( strlen( TextString ) > 0 );

	if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_NIOSH )
		{
		m_EditReaderID.GetWindowText( TextString, 12 );
		strcpy( BViewerCustomization.m_ReaderInfo.ID, TextString );

		m_EditReaderInitials.GetWindowText( TextString, 4 );
		strcpy( BViewerCustomization.m_ReaderInfo.Initials, TextString );

		m_EditReaderReportSignatureName.GetWindowText( TextString, 64 );
		strcpy( BViewerCustomization.m_ReaderInfo.ReportSignatureName, TextString );
		}
	else if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_TEST )
		{
		m_EditReaderID.GetWindowText( TextString, 12 );
		strcpy( BViewerCustomization.m_ReaderInfo.ID, TextString );

		m_EditReaderInitials.GetWindowText( TextString, 4 );
		strcpy( BViewerCustomization.m_ReaderInfo.Initials, TextString );

		strcpy( BViewerCustomization.m_ReaderInfo.ReportSignatureName, "" );

		m_EditReaderReportSignatureName.GetWindowText( TextString, 64 );
		strcpy( BViewerCustomization.m_ReaderInfo.ReportSignatureName, TextString );
		}
	else if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
		{
		strcpy( BViewerCustomization.m_ReaderInfo.ID, "" );
		strcpy( BViewerCustomization.m_ReaderInfo.Initials, "" );

		m_EditReaderReportSignatureName.GetWindowText( TextString, 64 );
		strcpy( BViewerCustomization.m_ReaderInfo.ReportSignatureName, TextString );
		}

	m_EditLoginPassword.GetWindowText( TextString, MAX_USER_INFO_LENGTH );
	// Don't allow the password to contain the * character, since it is used as the
	// mask and may be entered by mistake if not intentionally deleted.
	if ( strchr( TextString, '*' ) == NULL )
		{
		SaveAccessCode( &BViewerCustomization.m_ReaderInfo, TextString );
		BViewerCustomization.m_ReaderInfo.bPasswordEntered = ( strlen( TextString ) > 0 );
		}
	else
		{
		pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
		if ( pMainFrame != 0 )
			{
			UserNotificationInfo.WindowWidth = 500;
			UserNotificationInfo.WindowHeight = 400;
			UserNotificationInfo.FontHeight = 0;	// Use default setting;
			UserNotificationInfo.FontWidth = 0;		// Use default setting;
			UserNotificationInfo.UserInputType = USER_INPUT_TYPE_OK;
			UserNotificationInfo.pUserNotificationMessage = "The password must not\ncontain the * character";
			UserNotificationInfo.CallbackFunction = FinishReaderInfoResponse;
			pMainFrame -> PerformUserInput( &UserNotificationInfo );
			}
		}

	m_EditAE_Title.GetWindowText( TextString, 16 );
	strcpy( BViewerCustomization.m_ReaderInfo.AE_TITLE, TextString );

	m_EditReaderStreetAddress.GetWindowText( TextString, 64 );
	strcpy( BViewerCustomization.m_ReaderInfo.StreetAddress, TextString );

	m_EditReaderCity.GetWindowText( TextString, 32 );
	strcpy( BViewerCustomization.m_ReaderInfo.City, TextString );

	m_EditReaderState.GetWindowText( TextString, 4 );
	strcpy( BViewerCustomization.m_ReaderInfo.State, TextString );

	m_EditReaderZipCode.GetWindowText( TextString, 12 );
	strcpy( BViewerCustomization.m_ReaderInfo.ZipCode, TextString );

	m_ButtonSave.HasBeenPressed( TRUE );
	CDialog::OnOK();

	*pResult = 0;
}


void CReaderInfoScreen::OnBnClickedCancelReaderInfo( NMHDR *pNMHDR, LRESULT *pResult )
{
	m_ButtonCancel.HasBeenPressed( TRUE );
	CDialog::OnCancel();

	*pResult = 0;
}


HBRUSH CReaderInfoScreen::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
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


BOOL CReaderInfoScreen::OnEraseBkgnd( CDC *pDC )
{
	CBrush		BackgroundBrush( COLOR_CONFIG );
	CRect		BackgroundRectangle;
	CBrush		*pOldBrush = pDC -> SelectObject( &BackgroundBrush );

	GetClientRect( BackgroundRectangle );
	pDC -> FillRect( BackgroundRectangle, &BackgroundBrush );
	pDC -> SelectObject( pOldBrush );

	return TRUE;
}


