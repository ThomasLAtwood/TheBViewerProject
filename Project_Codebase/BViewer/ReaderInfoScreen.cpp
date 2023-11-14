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
// UPDATE HISTORY:
//
//	*[2] 08/08/2023 by Tom Atwood
//		Upgraded this class to handle all the reader info input, now that the
//		CustomizePage entries are read-only. Added LoadCurrentReaderInfo() function.
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
#include "ReaderInfoScreen.h"


extern CBViewerApp				ThisBViewerApp;
extern CONFIGURATION			BViewerConfiguration;
extern CCustomization			BViewerCustomization;


// CReaderInfoScreen dialog
// *[2] Added two creation parameters: pReaderInfo and Context.
CReaderInfoScreen::CReaderInfoScreen( CWnd *pParent /*=NULL*/, READER_PERSONAL_INFO *pReaderInfo, int Context )
			: CDialog( CReaderInfoScreen::IDD, pParent ),
				m_StaticReaderIdentification( "Reader Identification", 300, 50, 18, 9, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,		// *[2] Increased width.
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

				m_StaticReaderID( "ID", 100, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,										// *[2] Decreased width.
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_READER_SSN,
											"This appears in the NIOSH READER ID box on the report." ),
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
										IDC_STATIC_READER_INITIALS,
											"This appears in the READER'S INITIALS box on the report." ),
				m_EditReaderInitials( "", 70, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_READER_INITIALS ),

				m_StaticAE_Title( "Local Dicom Name\n   (AE_TITLE)", 150, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE | CONTROL_MULTILINE,
										IDC_STATIC_AE_TITLE,
											"The network name for this workstation.  Most people\n"
											"use BReader, but you can choose anything you like.\n"
											"However, if multiple readers use the same workstation,\n"
											"each reader should have a different AE Title so their\n"
											"studies can be kept separate." ),
				m_EditAE_Title( "", 120, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_AE_TITLE ),

				m_StaticReaderReportSignatureName( "Signature Name for Report", 200, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_READER_SIGNATURE_NAME,
										"This will appear as your printed signature on the report." ),
				m_EditReaderReportSignatureName( "", 460, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_NONE, IDC_EDIT_READER_SIGNATURE_NAME ),

				m_StaticReaderStreetAddress( "Street Address", 150, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_READER_STREET_ADDRESS,
											"This appears on the STREET ADDRESS line on the report." ),
				m_EditReaderStreetAddress( "", 300, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_READER_STREET_ADDRESS ),

				m_StaticReaderCity( "City", 100, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_READER_CITY,
											"This appears on the CITY line on the report." ),
				m_EditReaderCity( "", 200, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_READER_CITY ),

				m_StaticReaderState( "State", 60, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_READER_STATE,
											"This appears in the 2-character STATE abbreviation\n"
											"box on the report." ),
				m_EditReaderState( "", 50, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_READER_STATE ),

				m_StaticReaderZipCode( "Zip Code", 100, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_READER_ZIPCODE,
											"This appears in the ZIP CODE box on the report." ),
				m_EditReaderZipCode( "123", 120, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_READER_ZIPCODE ),
				m_StaticSelectCountry( "Select Country", 200, 30, 14, 7, 6,																					// *[2] Added
										COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_MULTILINE | CONTROL_VISIBLE,
										IDC_STATIC_SELECT_COUNTRY,
											"The Country selection determines the BViewer date formatting." ),
				m_ComboBoxSelectCountry( "", 280, 300, 18, 9, 5, VARIABLE_PITCH_FONT,																		// *[2] Added
										COLOR_BLACK, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_VSCROLL | EDIT_BORDER | LIST_SORT | CONTROL_VISIBLE,
										EDIT_VALIDATION_NONE, IDC_COMBO_SELECT_COUNTRY ),

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
	// *[2] Added the following
	if ( pReaderInfo != 0 )
		{
		m_pReaderInfo = pReaderInfo;
		memcpy( (void*)&m_ReaderInfo, (void*)pReaderInfo, sizeof(READER_PERSONAL_INFO) );
		m_bReaderInfoLoaded = TRUE;
		}
	else
		{
		m_pReaderInfo = &m_ReaderInfo;
		memset( (void*)&m_ReaderInfo, 0, sizeof(READER_PERSONAL_INFO) );
		m_bReaderInfoLoaded = FALSE;
		}
	m_ReaderInputContext = Context;
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
	static char		TextString[ 65 ];								// *[2] Added space for a null string terminator.
	int				PrimaryScreenWidth;
	int				PrimaryScreenHeight;

	CDialog::OnInitDialog();

	if ( m_ReaderInputContext == READER_INFO_CONTEXT_CONFIRM )		// *[2] Added text changes for reader info confirmation.
		{
		m_StaticReaderIdentification.m_ControlText = "Confirm Reader Information";
		m_ButtonSave.m_ControlText = "Save\nChanges";
		m_ButtonCancel.m_ControlText = "Confirm";
		}
	m_StaticReaderIdentification.SetPosition( 90, 30, this );
	m_StaticReaderLastName.SetPosition( 40, 70, this );
	m_EditReaderLastName.SetPosition( 240, 70, this );

	m_StaticLoginName.SetPosition( 420, 70, this );
	m_EditLoginName.SetPosition( 580, 70, this );
	
	m_StaticLoginPassword.SetPosition( 420, 100, this );
	m_EditLoginPassword.SetPosition( 580, 100, this );
	m_EditLoginPassword.SetWindowText( "" );						// *[2] Init password edit box.
	m_EditLoginPassword.SetPasswordChar( '*' );
	
	m_StaticAE_Title.SetPosition( 420, 130, this );
	m_EditAE_Title.SetPosition( 580, 130, this );
	m_EditAE_Title.SetWindowTextA( "BViewer" );						// *[2] Set default to what most readers are using.
	
	if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_GENERAL )
		{
		m_StaticReaderID.SetPosition( 40, 100, this );
		m_EditReaderID.SetPosition( 240, 100, this );

		m_StaticReaderInitials.SetPosition( 40, 130, this );
		m_EditReaderInitials.SetPosition( 240, 130, this );

		m_StaticReaderReportSignatureName.SetPosition( 40, 180, this );
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

	m_StaticReaderZipCode.SetPosition( 420, 270, this );			// *[2] Repositioned.
	m_EditReaderZipCode.SetPosition( 580, 270, this );				// *[2] Repositioned.

	m_StaticSelectCountry.SetPosition( 40, 300, this );				// *[2] Added.
	m_ComboBoxSelectCountry.SetPosition( 40, 325, this );			// *[2] Added.

	m_ButtonSave.SetPosition( 380, 330, this );						// *[2] Repositioned.
	m_ButtonCancel.SetPosition( 550, 330, this );					// *[2] Repositioned.

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

	memcpy( TextString, m_ReaderInfo.EncodedPassword, 64 );			// *[2] Changed password edit box initialization.
	TextString[ 64 ] = '\0';										// *[2] 
	m_EditLoginPassword.SetWindowText( TextString );				// *[2] 

	m_EditReaderStreetAddress.SetWindowText( "" );
	m_EditReaderCity.SetWindowText( "" );
	m_EditReaderState.SetWindowText( "" );
	m_EditReaderZipCode.SetWindowText( "" );

	LoadCountrySelectionList();										// *[2] Added.
	if ( m_bReaderInfoLoaded )										// *[2] Added.
		LoadCurrentReaderInfo();									// *[2] Added.
	InitializeControlTips();										// *[2] Added.

	SetWindowPos( &wndTop, ( PrimaryScreenWidth - 750 ) / 2, ( PrimaryScreenHeight - 430 ) / 2, 750, 430, SWP_SHOWWINDOW );		// *[2] Increased window height.

	return TRUE; 
}

// *[2] Added this function.
static void ControlTipActivationFunction( CWnd *pDialogWindow, char *pTipText, CPoint MouseCursorLocation )
{
	CReaderInfoScreen			*pReaderInfoScreen;

	pReaderInfoScreen = (CReaderInfoScreen*)pDialogWindow;
	if ( pReaderInfoScreen != 0 )
		{
		// If there has been a change in the tip text, reset the tip display window.
		if ( pTipText != 0 && strlen( pTipText ) > 0 && pTipText != pReaderInfoScreen -> m_pControlTip -> m_pTipText &&
																	pReaderInfoScreen -> m_pControlTip -> m_pTipText != 0 )
			{
			pReaderInfoScreen -> m_pControlTip -> ShowWindow( SW_HIDE );
			pReaderInfoScreen -> m_pControlTip -> m_pTipText = pTipText;
			pReaderInfoScreen -> m_pControlTip -> ShowTipText( MouseCursorLocation, pReaderInfoScreen );
			}
		else if ( pTipText == 0 )
			pReaderInfoScreen -> m_pControlTip -> ShowWindow( SW_HIDE );
		else
			{
			pReaderInfoScreen -> m_pControlTip -> m_pTipText = pTipText;
			pReaderInfoScreen -> m_pControlTip -> ShowTipText( MouseCursorLocation, pReaderInfoScreen );
			}
		}
}


// *[2] Added this function.
void CReaderInfoScreen::InitializeControlTips()
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



static COUNTRY_INFO		CountryInfoArray[] =						// *[2] Relocated here from CustomizePage.cpp.
{
	{ "Afghanistan", DATE_FORMAT_DMY },
	{ "Åland Islands", DATE_FORMAT_YMD },
	{ "Albania", DATE_FORMAT_DMY },
	{ "Algeria", DATE_FORMAT_DMY },
	{ "American Samoa", DATE_FORMAT_MDY },
	{ "Andorra", DATE_FORMAT_DMY },
	{ "Angola", DATE_FORMAT_DMY },
	{ "Anguilla", DATE_FORMAT_DMY },
	{ "Antigua and Barbuda", DATE_FORMAT_DMY },
	{ "Argentina", DATE_FORMAT_DMY },
	{ "Armenia", DATE_FORMAT_DMY },
	{ "Aruba", DATE_FORMAT_DMY },
	{ "Ascension", DATE_FORMAT_DMY },
	{ "Australia", DATE_FORMAT_DMY },
	{ "Austria", DATE_FORMAT_DMY },
	{ "Azerbaijan", DATE_FORMAT_DMY },
	{ "Bahamas", DATE_FORMAT_DMY },
	{ "Bahrain", DATE_FORMAT_DMY },
	{ "Bangladesh", DATE_FORMAT_DMY },
	{ "Barbados", DATE_FORMAT_DMY },
	{ "Belarus", DATE_FORMAT_DMY },
	{ "Belgium", DATE_FORMAT_DMY },
	{ "Belize", DATE_FORMAT_DMY },
	{ "Benin", DATE_FORMAT_DMY },
	{ "Bermuda", DATE_FORMAT_DMY },
	{ "Bhutan", DATE_FORMAT_YMD },
	{ "Bolivia", DATE_FORMAT_DMY },
	{ "Bonaire", DATE_FORMAT_DMY },
	{ "Bosnia and Herzegovina", DATE_FORMAT_DMY },
	{ "Botswana", DATE_FORMAT_DMY },
	{ "Brazil", DATE_FORMAT_DMY },
	{ "British Indian Ocean Territory", DATE_FORMAT_DMY },
	{ "British Virgin Islands", DATE_FORMAT_DMY },
	{ "Brunei", DATE_FORMAT_DMY },
	{ "Bulgaria", DATE_FORMAT_DMY },
	{ "Burkina Faso", DATE_FORMAT_DMY },
	{ "Burundi", DATE_FORMAT_DMY },
	{ "Cambodia", DATE_FORMAT_DMY },
	{ "Cameroon", DATE_FORMAT_DMY },
	{ "Canada", DATE_FORMAT_YMD },
	{ "Cape Verde", DATE_FORMAT_DMY },
	{ "Cayman Islands", DATE_FORMAT_DMY },
	{ "Central African Republic", DATE_FORMAT_DMY },
	{ "Chad", DATE_FORMAT_DMY },
	{ "Chile", DATE_FORMAT_DMY },
	{ "China", DATE_FORMAT_DMY },
	{ "Cocos (Keeling) Islands", DATE_FORMAT_DMY },
	{ "Colombia", DATE_FORMAT_DMY },
	{ "Comoros", DATE_FORMAT_DMY },
	{ "Congo", DATE_FORMAT_DMY },
	{ "Cook Islands", DATE_FORMAT_DMY },
	{ "Costa Rica", DATE_FORMAT_DMY },
	{ "Croatia", DATE_FORMAT_DMY },
	{ "Cuba", DATE_FORMAT_DMY },
	{ "Curaçao", DATE_FORMAT_DMY },
	{ "Cyprus", DATE_FORMAT_DMY },
	{ "Czech Republic", DATE_FORMAT_DMY },
	{ "Denmark", DATE_FORMAT_DMY },
	{ "Djibouti", DATE_FORMAT_DMY },
	{ "Dominica", DATE_FORMAT_DMY },
	{ "Dominican Republic", DATE_FORMAT_DMY },
	{ "East Timor", DATE_FORMAT_DMY },
	{ "Ecuador", DATE_FORMAT_DMY },
	{ "Egypt", DATE_FORMAT_DMY },
	{ "El Salvador", DATE_FORMAT_DMY },
	{ "Equatorial Guinea", DATE_FORMAT_DMY },
	{ "Eritrea", DATE_FORMAT_DMY },
	{ "Estonia", DATE_FORMAT_DMY },
	{ "Ethiopia", DATE_FORMAT_DMY },
	{ "Falkland Islands", DATE_FORMAT_DMY },
	{ "Faroe Islands", DATE_FORMAT_DMY },
	{ "Federated States of Micronesia", DATE_FORMAT_MDY },
	{ "Finland", DATE_FORMAT_DMY },
	{ "Fiji", DATE_FORMAT_DMY },
	{ "France", DATE_FORMAT_DMY },
	{ "French Guiana", DATE_FORMAT_DMY },
	{ "French Polynesia", DATE_FORMAT_DMY },
	{ "Gabon", DATE_FORMAT_DMY },
	{ "Gambia", DATE_FORMAT_DMY },
	{ "Georgia", DATE_FORMAT_DMY },
	{ "Germany", DATE_FORMAT_YMD },
	{ "Ghana", DATE_FORMAT_DMY },
	{ "Gibraltar", DATE_FORMAT_DMY },
	{ "Greece", DATE_FORMAT_DMY },
	{ "Greenland", DATE_FORMAT_MDY },
	{ "Grenada", DATE_FORMAT_DMY },
	{ "Guadeloupe", DATE_FORMAT_DMY },
	{ "Guam", DATE_FORMAT_MDY },
	{ "Guatemala", DATE_FORMAT_DMY },
	{ "Guernsey", DATE_FORMAT_DMY },
	{ "Guinea", DATE_FORMAT_DMY },
	{ "Guinea-Bissau", DATE_FORMAT_DMY },
	{ "Guyana", DATE_FORMAT_DMY },
	{ "Haiti", DATE_FORMAT_DMY },
	{ "Hong Kong", DATE_FORMAT_DMY },
	{ "Honduras", DATE_FORMAT_DMY },
	{ "Hungary", DATE_FORMAT_YMD },
	{ "Iceland", DATE_FORMAT_DMY },
	{ "India", DATE_FORMAT_DMY },
	{ "Indonesia", DATE_FORMAT_DMY },
	{ "Iran, Islamic Republic of", DATE_FORMAT_YMD },
	{ "Iraq", DATE_FORMAT_DMY },
	{ "Ireland", DATE_FORMAT_DMY },
	{ "Isle of Man", DATE_FORMAT_DMY },
	{ "Israel", DATE_FORMAT_DMY },
	{ "Italy", DATE_FORMAT_DMY },
	{ "Ivory Coast", DATE_FORMAT_DMY },
	{ "Jamaica", DATE_FORMAT_DMY },
	{ "Jan Mayen", DATE_FORMAT_DMY },
	{ "Japan", DATE_FORMAT_YMD },
	{ "Jersey", DATE_FORMAT_DMY },
	{ "Jordan", DATE_FORMAT_DMY },
	{ "Kazakhstan", DATE_FORMAT_DMY },
	{ "Kenya", DATE_FORMAT_DMY },
	{ "Kiribati", DATE_FORMAT_DMY },
	{ "North Korea", DATE_FORMAT_YMD },
	{ "South Korea", DATE_FORMAT_YMD },
	{ "Kosovo", DATE_FORMAT_DMY },
	{ "Kuwait", DATE_FORMAT_DMY },
	{ "Kyrgyz Republic", DATE_FORMAT_DMY },
	{ "Lao People's Democratic Republic", DATE_FORMAT_DMY },
	{ "Latvia", DATE_FORMAT_DMY },
	{ "Lebanon", DATE_FORMAT_DMY },
	{ "Lesotho", DATE_FORMAT_DMY },
	{ "Liberia", DATE_FORMAT_DMY },
	{ "Libya", DATE_FORMAT_DMY },
	{ "Liechtenstein", DATE_FORMAT_DMY },
	{ "Lithuania", DATE_FORMAT_YMD },
	{ "Luxembourg", DATE_FORMAT_DMY },
	{ "Macau", DATE_FORMAT_DMY },
	{ "Macedonia", DATE_FORMAT_DMY },
	{ "Madagascar", DATE_FORMAT_DMY },
	{ "Malawi", DATE_FORMAT_DMY },
	{ "Malaysia", DATE_FORMAT_DMY },
	{ "Maldives", DATE_FORMAT_YMD },
	{ "Mali", DATE_FORMAT_DMY },
	{ "Malta", DATE_FORMAT_DMY },
	{ "Marshall Islands", DATE_FORMAT_MDY },
	{ "Martinique", DATE_FORMAT_DMY },
	{ "Mauritania", DATE_FORMAT_DMY },
	{ "Mauritius", DATE_FORMAT_DMY },
	{ "Mayotte", DATE_FORMAT_DMY },
	{ "Mexico", DATE_FORMAT_DMY },
	{ "Moldova", DATE_FORMAT_DMY },
	{ "Monaco", DATE_FORMAT_DMY },
	{ "Mongolia", DATE_FORMAT_YMD },
	{ "Montenegro", DATE_FORMAT_DMY },
	{ "Montserrat", DATE_FORMAT_DMY },
	{ "Morocco", DATE_FORMAT_DMY },
	{ "Mozambique", DATE_FORMAT_DMY },
	{ "Myanmar", DATE_FORMAT_DMY },
	{ "Nagorno-Karabakh Republic", DATE_FORMAT_DMY },
	{ "Namibia", DATE_FORMAT_DMY },
	{ "Nauru", DATE_FORMAT_DMY },
	{ "Nepal", DATE_FORMAT_YMD },
	{ "Netherlands", DATE_FORMAT_DMY },
	{ "New Caledonia", DATE_FORMAT_DMY },
	{ "New Zealand", DATE_FORMAT_DMY },
	{ "Nicaragua", DATE_FORMAT_DMY },
	{ "Niger", DATE_FORMAT_DMY },
	{ "Nigeria", DATE_FORMAT_DMY },
	{ "Niue", DATE_FORMAT_DMY },
	{ "Norfolk Island", DATE_FORMAT_DMY },
	{ "Northern Mariana Islands", DATE_FORMAT_MDY },
	{ "Norway", DATE_FORMAT_DMY },
	{ "Oman", DATE_FORMAT_DMY },
	{ "Pakistan", DATE_FORMAT_DMY },
	{ "Palestine", DATE_FORMAT_DMY },
	{ "Palau", DATE_FORMAT_DMY },
	{ "Panama", DATE_FORMAT_MDY },
	{ "Papua New Guinea", DATE_FORMAT_DMY },
	{ "Paraguay", DATE_FORMAT_DMY },
	{ "Peru", DATE_FORMAT_DMY },
	{ "Philippines", DATE_FORMAT_MDY },
	{ "Pitcairn Islands", DATE_FORMAT_DMY },
	{ "Poland", DATE_FORMAT_DMY },
	{ "Portugal", DATE_FORMAT_DMY },
	{ "Puerto Rico", DATE_FORMAT_MDY },
	{ "Qatar", DATE_FORMAT_DMY },
	{ "Réunion", DATE_FORMAT_DMY },
	{ "Romania", DATE_FORMAT_DMY },
	{ "Russian Federation", DATE_FORMAT_DMY },
	{ "Rwanda", DATE_FORMAT_DMY },
	{ "Saba", DATE_FORMAT_DMY },
	{ "Saint Barthélemy", DATE_FORMAT_DMY },
	{ "Saint Helena", DATE_FORMAT_DMY },
	{ "Saint Kitts and Nevis", DATE_FORMAT_DMY },
	{ "Saint Lucia", DATE_FORMAT_DMY },
	{ "Saint Martin", DATE_FORMAT_DMY },
	{ "Saint Pierre and Miquelon", DATE_FORMAT_DMY },
	{ "Saint Vincent and the Grenadines", DATE_FORMAT_DMY },
	{ "Samoa", DATE_FORMAT_DMY },
	{ "São Tomé and Príncipe", DATE_FORMAT_DMY },
	{ "Saudi Arabia", DATE_FORMAT_DMY },
	{ "Senegal", DATE_FORMAT_DMY },
	{ "Serbia", DATE_FORMAT_DMY },
	{ "Seychelles", DATE_FORMAT_DMY },
	{ "Sierra Leone", DATE_FORMAT_DMY },
	{ "Singapore", DATE_FORMAT_DMY },
	{ "Sint Eustatius", DATE_FORMAT_DMY },
	{ "Sint Maarten", DATE_FORMAT_DMY },
	{ "Slovakia", DATE_FORMAT_DMY },
	{ "Slovenia", DATE_FORMAT_DMY },
	{ "Solomon Islands", DATE_FORMAT_DMY },
	{ "Somalia", DATE_FORMAT_DMY },
	{ "South Africa", DATE_FORMAT_YMD },
	{ "Spain", DATE_FORMAT_DMY },
	{ "Sri Lanka", DATE_FORMAT_YMD },
	{ "Sudan", DATE_FORMAT_DMY },
	{ "Suriname", DATE_FORMAT_DMY },
	{ "Svalbard", DATE_FORMAT_DMY },
	{ "Swaziland", DATE_FORMAT_DMY },
	{ "Sweden", DATE_FORMAT_YMD },
	{ "Switzerland", DATE_FORMAT_DMY },
	{ "Syrian Arab Republic", DATE_FORMAT_DMY },
	{ "Taiwan", DATE_FORMAT_YMD },
	{ "Tajikistan", DATE_FORMAT_DMY },
	{ "Tanzania", DATE_FORMAT_DMY },
	{ "Thailand", DATE_FORMAT_DMY },
	{ "Togo", DATE_FORMAT_DMY },
	{ "Tokelau", DATE_FORMAT_DMY },
	{ "Tonga", DATE_FORMAT_DMY },
	{ "Trinidad and Tobago", DATE_FORMAT_DMY },
	{ "Tristan da Cunha", DATE_FORMAT_DMY },
	{ "Tunisia", DATE_FORMAT_DMY },
	{ "Turkey", DATE_FORMAT_DMY },
	{ "Turkmenistan", DATE_FORMAT_DMY },
	{ "Turks and Caicos Islands", DATE_FORMAT_DMY },
	{ "Tuvalu", DATE_FORMAT_DMY },
	{ "Uganda", DATE_FORMAT_DMY },
	{ "Ukraine", DATE_FORMAT_DMY },
	{ "United Arab Emirates", DATE_FORMAT_DMY },
	{ "United Kingdom", DATE_FORMAT_DMY },
	{ "United States Minor Outlying Islands", DATE_FORMAT_MDY },
	{ "United States of America", DATE_FORMAT_MDY },
	{ "United States Virgin Islands", DATE_FORMAT_MDY },
	{ "Uruguay", DATE_FORMAT_DMY },
	{ "Uzbekistan", DATE_FORMAT_DMY },
	{ "Vanuatu", DATE_FORMAT_DMY },
	{ "Venezuela", DATE_FORMAT_DMY },
	{ "Vietnam", DATE_FORMAT_DMY },
	{ "Wallis and Futuna", DATE_FORMAT_DMY },
	{ "Yemen", DATE_FORMAT_DMY },
	{ "Zambia", DATE_FORMAT_DMY },
	{ "Zimbabwe", DATE_FORMAT_DMY },
	{ "", DATE_FORMAT_UNSPECIFIED }
};


BOOL CReaderInfoScreen::LoadCountrySelectionList()						// *[2] Relocated here from CustomizePage.cpp.
{
	BOOL				bNoError = TRUE;
	BOOL				bEndOfList;
	BOOL				bCountryWasPreviouslySelected;
	COUNTRY_INFO		*pCountryInfo;
	int					nCountry;
	int					nItemIndex;
	int					nDefaultCountry;

	m_ComboBoxSelectCountry.ResetContent();
	m_ComboBoxSelectCountry.SetWindowTextA( "Country List" );

	nCountry = 0;
	nDefaultCountry = 0;
	bCountryWasPreviouslySelected = ( strlen( BViewerCustomization.m_CountryInfo.CountryName ) != 0 );
	do
		{
		pCountryInfo = &CountryInfoArray[ nCountry ];
		bEndOfList = ( strlen( pCountryInfo -> CountryName ) == 0 );
		if ( !bEndOfList )
			{
			nItemIndex = m_ComboBoxSelectCountry.AddString( pCountryInfo -> CountryName );
			if ( !bCountryWasPreviouslySelected )
				{
				if ( _stricmp( pCountryInfo -> CountryName, "United States of America" ) == 0 )
					nDefaultCountry = nItemIndex;
				}
			else
				{
				if ( _stricmp( pCountryInfo -> CountryName, BViewerCustomization.m_CountryInfo.CountryName ) == 0 )
					nDefaultCountry = nItemIndex;
				}
			m_ComboBoxSelectCountry.SetItemDataPtr( nItemIndex, (void*)pCountryInfo );
			}
		nCountry++;
		}
	while ( !bEndOfList );
	m_ComboBoxSelectCountry.SetCurSel( nDefaultCountry );

	return bNoError;
}


void CReaderInfoScreen::LoadCurrentReaderInfo()					// *[2] Added this function, combining the OnKillFocus functions
{																// *[2]  from CustomizePage.cpp.
	char							TextString[ 65 ];			// *[2] Added space for a terminating null character.

	strncpy_s( TextString, 64, m_ReaderInfo.LastName, _TRUNCATE );
	m_EditReaderLastName.SetWindowText( TextString );

	strncpy_s( TextString, 64, m_ReaderInfo.LoginName, _TRUNCATE );
	m_EditLoginName.SetWindowText( TextString );
	m_ReaderInfo.bLoginNameEntered = ( strlen( TextString ) > 0 );

	if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_NIOSH )
		{
		strncpy_s( TextString, 64, m_ReaderInfo.ID, _TRUNCATE );
		m_EditReaderID.SetWindowText( TextString );

		strncpy_s( TextString, 64, m_ReaderInfo.Initials, _TRUNCATE );
		m_EditReaderInitials.SetWindowText( TextString );

		strncpy_s( TextString, 64, m_ReaderInfo.ReportSignatureName, _TRUNCATE );
		m_EditReaderReportSignatureName.SetWindowText( TextString );
		}
	else if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_TEST )
		{
		strncpy_s( TextString, 64, m_ReaderInfo.ID, _TRUNCATE );
		m_EditReaderID.SetWindowText( TextString );

		strncpy_s( TextString, 64, m_ReaderInfo.Initials, _TRUNCATE );
		m_EditReaderInitials.SetWindowText( TextString );

		strncpy_s( TextString, 64, m_ReaderInfo.ReportSignatureName, _TRUNCATE );
		m_EditReaderReportSignatureName.SetWindowText( TextString );
		}
	else if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
		{
		strncpy_s( TextString, 64, m_ReaderInfo.ReportSignatureName, _TRUNCATE );
		m_EditReaderReportSignatureName.SetWindowText( TextString );
		}

//	_strnset_s( TextString, 64, '*', strlen( m_ReaderInfo.EncodedPassword ) );
	memcpy( TextString, m_ReaderInfo.EncodedPassword, 64 );
	TextString[ m_ReaderInfo.pwLength ] = '\0';
	m_EditLoginPassword.SetWindowText( TextString );

	strncpy_s( TextString, 64, m_ReaderInfo.AE_TITLE, _TRUNCATE );
	m_EditAE_Title.SetWindowText( TextString );

	strncpy_s( TextString, 64, m_ReaderInfo.StreetAddress, _TRUNCATE );
	m_EditReaderStreetAddress.SetWindowText( TextString );

	strncpy_s( TextString, 64, m_ReaderInfo.City, _TRUNCATE );
	m_EditReaderCity.SetWindowText( TextString );

	strncpy_s( TextString, 64, m_ReaderInfo.State, _TRUNCATE );
	m_EditReaderState.SetWindowText( TextString );

	strncpy_s( TextString, 64, m_ReaderInfo.ZipCode, _TRUNCATE );
	m_EditReaderZipCode.SetWindowText( TextString );
}


BOOL CReaderInfoScreen::ValidateReaderInfo()					// *[2] Added this function.
{
	BOOL							bReaderInfoIsOK;
	char							TextString[ 65 ];
 	CMainFrame						*pMainFrame;
	static USER_NOTIFICATION_INFO	UserNotificationInfo;

	bReaderInfoIsOK = TRUE;
	UserNotificationInfo.WindowWidth = 500;
	UserNotificationInfo.WindowHeight = 200;
	UserNotificationInfo.FontHeight = 0;	// Use default setting;
	UserNotificationInfo.FontWidth = 0;		// Use default setting;
	UserNotificationInfo.UserInputType = USER_INPUT_TYPE_OK;
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	m_EditReaderLastName.GetWindowText( TextString, 64 );
	if ( strlen( TextString ) == 0 )
		{
		bReaderInfoIsOK = FALSE;
		if ( pMainFrame != 0 )
			{
			UserNotificationInfo.pUserNotificationMessage = "Reader last name\nmust be specified.";
			UserNotificationInfo.CallbackFunction = FinishReaderInfoResponse;
			pMainFrame -> PerformUserInput( &UserNotificationInfo );
			}
		}
	m_EditReaderReportSignatureName.GetWindowText( TextString, 64 );
	if ( strlen( TextString ) == 0 )
		{
		bReaderInfoIsOK = FALSE;
		if ( pMainFrame != 0 )
			{
			UserNotificationInfo.pUserNotificationMessage = "The Signature Name for Report\nmust be specified.";
			UserNotificationInfo.CallbackFunction = FinishReaderInfoResponse;
			pMainFrame -> PerformUserInput( &UserNotificationInfo );
			}
		}
	m_EditLoginPassword.GetWindowText( TextString, MAX_USER_INFO_LENGTH + 1 );
	if ( strchr( TextString, '*' ) != NULL )
		{
		bReaderInfoIsOK = FALSE;
		if ( pMainFrame != 0 )
			{
			UserNotificationInfo.pUserNotificationMessage = "The password must not\ncontain the * character";
			UserNotificationInfo.CallbackFunction = FinishReaderInfoResponse;
			pMainFrame -> PerformUserInput( &UserNotificationInfo );
			}
		}

	return bReaderInfoIsOK;
}


// *[2] Added qualification requirements for a valid reader.
void CReaderInfoScreen::OnBnClickedSaveReaderInfo( NMHDR *pNMHDR, LRESULT *pResult )	// *[2] Copied reader input to local m_ReaderInfo instead of BViewerCustomization.
{
	char							TextString[ 65 ];
	COUNTRY_INFO					*pCountryInfo;										// *[2] Added variable.
	int								nItemIndex;											// *[2] Added variable.
	BOOL							bCountryWasSelected;								// *[2] Added variable.

	if ( ValidateReaderInfo() )
		{
		m_EditReaderLastName.GetWindowText( TextString, 64 );
		strncpy_s( m_ReaderInfo.LastName, MAX_USER_INFO_LENGTH, TextString, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.

		m_EditLoginName.GetWindowText( TextString, 64 );
		if ( strlen( TextString ) == 0 )
			m_EditLoginPassword.SetWindowText( "" );
		strncpy_s( m_ReaderInfo.LoginName, MAX_USER_INFO_LENGTH, TextString, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
		m_ReaderInfo.bLoginNameEntered = ( strlen( TextString ) > 0 );

		if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_NIOSH )
			{
			m_EditReaderID.GetWindowText( TextString, 12 );
			strncpy_s( m_ReaderInfo.ID, 12, TextString, _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.

			m_EditReaderInitials.GetWindowText( TextString, 4 );
			strncpy_s( m_ReaderInfo.Initials, 4, TextString, _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.

			m_EditReaderReportSignatureName.GetWindowText( TextString, 64 );
			strncpy_s( m_ReaderInfo.ReportSignatureName, 64, TextString, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
			}
		else if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_TEST )
			{
			m_EditReaderID.GetWindowText( TextString, 12 );
			strncpy_s( m_ReaderInfo.ID, 12, TextString, _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.

			m_EditReaderInitials.GetWindowText( TextString, 4 );
			strncpy_s( m_ReaderInfo.Initials, 4, TextString, _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.

			m_EditReaderReportSignatureName.GetWindowText( TextString, 64 );
			strncpy_s( m_ReaderInfo.ReportSignatureName, 64, TextString, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
			}
		else if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
			{
			m_ReaderInfo.ID[ 0 ] = '\0';													// *[1] Eliminated call to strcpy.
			m_ReaderInfo.Initials[ 0 ] = '\0';												// *[1] Eliminated call to strcpy.

			m_EditReaderReportSignatureName.GetWindowText( TextString, 64 );
			strncpy_s( m_ReaderInfo.ReportSignatureName, 64, TextString, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
			}

		m_EditLoginPassword.GetWindowText( TextString, MAX_USER_INFO_LENGTH + 1 );			// *[2] Allow for null terminator.
		SaveAccessCode( &m_ReaderInfo, TextString );
		m_ReaderInfo.pwLength = (char)strlen( TextString );								// *[2] Set new length parameter.
		m_ReaderInfo.bPasswordEntered = ( strlen( TextString ) > 0 );

		m_EditAE_Title.GetWindowText( TextString, 16 );
		if ( strlen( TextString ) == 0 )
			strncpy_s( TextString, 64, "BViewer", _TRUNCATE );							// *[2] Preset default value.
		strncpy_s( m_ReaderInfo.AE_TITLE, 20, TextString, _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.

		m_EditReaderStreetAddress.GetWindowText( TextString, 64 );
		strncpy_s( m_ReaderInfo.StreetAddress, 64, TextString, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.

		m_EditReaderCity.GetWindowText( TextString, MAX_USER_INFO_LENGTH );
		strncpy_s( m_ReaderInfo.City, MAX_USER_INFO_LENGTH, TextString, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.

		m_EditReaderState.GetWindowText( TextString, 4 );
		strncpy_s( m_ReaderInfo.State, 4, TextString, _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.

		m_EditReaderZipCode.GetWindowText( TextString, 12 );
		strncpy_s( m_ReaderInfo.ZipCode, 12, TextString, _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.

		// *[2] Record country selection.
		nItemIndex = m_ComboBoxSelectCountry.GetCurSel();
		bCountryWasSelected = ( nItemIndex >= 0 );
		if ( !bCountryWasSelected )
			{
			strncpy_s( m_ReaderInfo.m_CountryInfo.CountryName, MAX_NAME_LENGTH, "United States of America", _TRUNCATE );
			m_ReaderInfo.m_CountryInfo.DateFormat = DATE_FORMAT_MDY;
			}
		else
			{
			pCountryInfo = (COUNTRY_INFO*)m_ComboBoxSelectCountry.GetItemDataPtr( nItemIndex );
			memcpy( &m_ReaderInfo.m_CountryInfo, pCountryInfo, sizeof(COUNTRY_INFO) );
			}

		// *[2] Save the newly entered reader info as indicated by the CReaderInfoScreen invoker.
		if ( m_bReaderInfoLoaded )
			memcpy( m_pReaderInfo, &m_ReaderInfo, sizeof(READER_PERSONAL_INFO) );

		m_ButtonSave.HasBeenPressed( TRUE );
		CDialog::OnOK();
		}

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


