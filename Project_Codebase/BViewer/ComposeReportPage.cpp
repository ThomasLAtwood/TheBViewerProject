// ComposeReportPage.cpp : Implementation file for the CComposeReportPage
//  class of CPropertyPage, which implements the "Produce Report" tab of the main
//  Control Panel.
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
// UPDATE HISTORY:
//
//	*[5] 07/17/2023 by Tom Atwood
//		Fixed code security issues.
//	*[4] 06/09/2023 by Tom Atwood
//		Added to SetReportCount() to prevent crash.
//	*[3] 03/15/2023 by Tom Atwood
//		Fixed code security issues.
//	*[2] 03/03/2023 by Tom Atwood
//		Added report showing and saving diagnostics.
//	*[1] 01/09/2023 by Tom Atwood
//		Fixed code security issues.
//
//
#include "stdafx.h"
#include <stdio.h>
#include <process.h>
#include "Module.h"
#include "ReportStatus.h"
#include "BViewer.h"
#include "Configuration.h"
#include "DiagnosticImage.h"
#include "Mouse.h"
#include "ImageView.h"
#include "MainFrm.h"
#include "ImageFrame.h"
#include "ComposeReportPage.h"
#include "Export.h"
#include "Client.h"


CLIENT_INFO						*p_CurrentClientInfo = 0;
BOOL							bAttendedSession = TRUE;

extern CONFIGURATION			BViewerConfiguration;
extern CCustomization			BViewerCustomization;
extern BOOL						bMakeDumbButtons;
extern LIST_HEAD				AvailableClientList;


void CheckForIncompleteInterpretation( BOOL *pbOKToProceed );

// CComposeReportPage dialog
CComposeReportPage::CComposeReportPage() : CPropertyPage( CComposeReportPage::IDD ),

		m_StaticPatientName( "Patient Name", 180, 20, 14, 7, 6,
								COLOR_ANALYSIS_FONT, COLOR_ANALYSIS_BKGD, COLOR_ANALYSIS_BKGD,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_MULTILINE | CONTROL_VISIBLE,
								IDC_STATIC_REPORT_PATIENT_NAME ),
		m_EditPatientName( "", 400, 25, 18, 9, 5, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_UNTOUCHED_LIGHT, COLOR_COMPLETED_LIGHT, COLOR_TOUCHED,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_REPORT_PATIENT_NAME ),
		m_StaticDateOfBirth( "Date of Birth", 180, 20, 14, 7, 6,
								COLOR_ANALYSIS_FONT, COLOR_ANALYSIS_BKGD, COLOR_ANALYSIS_BKGD,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_MULTILINE | CONTROL_VISIBLE,
								IDC_STATIC_REPORT_DOB ),
		m_EditDateOfBirth( "", 150, 30, 20, 10, 5, COLOR_ANALYSIS_FONT, COLOR_UNTOUCHED_LIGHT, COLOR_COMPLETED_LIGHT, COLOR_TOUCHED,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_TOP_JUSTIFIED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								IDC_EDIT_REPORT_DOB ),
		m_StaticPatientID( "Patient ID", 260, 20, 14, 7, 6,
								COLOR_ANALYSIS_FONT, COLOR_ANALYSIS_BKGD, COLOR_ANALYSIS_BKGD,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_MULTILINE | CONTROL_VISIBLE,
								IDC_STATIC_REPORT_PATIENT_ID ),
		m_EditPatientID( "", 400, 25, 18, 9, 5, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_UNTOUCHED_LIGHT, COLOR_COMPLETED_LIGHT, COLOR_TOUCHED,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_REPORT_PATIENT_ID ),
		m_StaticOrderingPhysicianName( "Ordering Physician Name", 180, 20, 14, 7, 6,
								COLOR_ANALYSIS_FONT, COLOR_ANALYSIS_BKGD, COLOR_ANALYSIS_BKGD,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_MULTILINE | CONTROL_VISIBLE,
								IDC_STATIC_ORDERING_PHYSICIAN_NAME ),
		m_EditOrderingPhysicianName( "", 400, 25, 18, 9, 5, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_UNTOUCHED_LIGHT, COLOR_COMPLETED_LIGHT, COLOR_TOUCHED,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_ORDERING_PHYSICIAN_NAME ),
		m_StaticOrderingFacility( "Ordering Facility", 180, 20, 14, 7, 6,
								COLOR_ANALYSIS_FONT, COLOR_ANALYSIS_BKGD, COLOR_ANALYSIS_BKGD,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_MULTILINE | CONTROL_VISIBLE,
								IDC_STATIC_ORDERING_FACILITY ),
		m_EditOrderingFacility( "", 400, 25, 18, 9, 5, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_UNTOUCHED_LIGHT, COLOR_COMPLETED_LIGHT, COLOR_TOUCHED,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_ORDERING_FACILITY ),
		m_StaticClassificationPurpose( "Classification Purpose", 180, 20, 14, 7, 6,
								COLOR_ANALYSIS_FONT, COLOR_ANALYSIS_BKGD, COLOR_ANALYSIS_BKGD,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_MULTILINE | CONTROL_VISIBLE,
								IDC_STATIC_CLASSIFICATION_PURPOSE ),
		m_EditClassificationPurpose( "", 400, 25, 18, 9, 5, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_UNTOUCHED_LIGHT, COLOR_COMPLETED_LIGHT, COLOR_TOUCHED,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_CLASSIFICATION_PURPOSE ),

		m_StaticDateOfRadiograph( "Date of Radiograph", 180, 20, 14, 7, 6,
								COLOR_ANALYSIS_FONT, COLOR_ANALYSIS_BKGD, COLOR_ANALYSIS_BKGD,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_MULTILINE | CONTROL_VISIBLE,
								IDC_STATIC_DATE_OF_RADIOGRAPH ),
		m_EditDateOfRadiograph( "", 150, 30, 20, 10, 5, COLOR_ANALYSIS_FONT, COLOR_UNTOUCHED_LIGHT, COLOR_COMPLETED_LIGHT, COLOR_TOUCHED,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_TOP_JUSTIFIED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								IDC_EDIT_DATE_OF_RADIOGRAPH ),
		m_StaticTypeOfReading( "Type of Reading", 180, 20, 14, 7, 6,
								COLOR_ANALYSIS_FONT, COLOR_ANALYSIS_BKGD, COLOR_ANALYSIS_BKGD,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_MULTILINE | CONTROL_VISIBLE,
								IDC_STATIC_TYPE_OF_READING ),
		m_ButtonTypeOfReadingA( "A", 30, 30, 14, 7, 6, COLOR_ANALYSIS_FONT, COLOR_UNTOUCHED_LIGHT, COLOR_COMPLETED_LIGHT, COLOR_TOUCHED,
								BUTTON_CHECKBOX | CONTROL_TEXT_HORIZONTALLY_CENTERED |
								CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_A_READER ),
		m_ButtonTypeOfReadingB( "B", 30, 30, 14, 7, 6, COLOR_ANALYSIS_FONT, COLOR_UNTOUCHED_LIGHT, COLOR_COMPLETED_LIGHT, COLOR_TOUCHED,
								BUTTON_CHECKBOX | CONTROL_TEXT_HORIZONTALLY_CENTERED |
								CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_B_READER ),
		m_ButtonTypeOfReadingF( "F", 30, 30, 14, 7, 6, COLOR_ANALYSIS_FONT, COLOR_UNTOUCHED_LIGHT, COLOR_COMPLETED_LIGHT, COLOR_TOUCHED,
								BUTTON_CHECKBOX | CONTROL_TEXT_HORIZONTALLY_CENTERED |
								CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_FACILITY_READING ),
		m_ButtonTypeOfReadingO( "O", 30, 30, 14, 7, 6, COLOR_ANALYSIS_FONT, COLOR_UNTOUCHED_LIGHT, COLOR_COMPLETED_LIGHT, COLOR_TOUCHED,
								BUTTON_CHECKBOX | CONTROL_TEXT_HORIZONTALLY_CENTERED |
								CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_OTHER_READ ),
		m_TypeOfReadingButtonGroup( BUTTON_CHECKBOX, GROUP_SINGLE_SELECT | GROUP_ONE_TOUCHES_ALL, 4,
								&m_ButtonTypeOfReadingA, &m_ButtonTypeOfReadingB, &m_ButtonTypeOfReadingF, &m_ButtonTypeOfReadingO ),
		m_EditTypeOfReadingOther( "", 190, 25, 18, 9, 5, VARIABLE_PITCH_FONT,
								COLOR_BLACK, COLOR_UNTOUCHED_LIGHT, COLOR_COMPLETED_LIGHT, COLOR_TOUCHED,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_TYPE_OF_READING_OTHER ),
		m_StaticTypeOfReadingOther( "Other", 100, 20, 14, 7, 6,
								COLOR_ANALYSIS_FONT, COLOR_ANALYSIS_BKGD, COLOR_ANALYSIS_BKGD,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_MULTILINE | CONTROL_VISIBLE,
								IDC_STATIC_TYPE_OF_READING_OTHER ),
		m_StaticDateOfReading( "Date of Reading", 180, 20, 14, 7, 6,
								COLOR_ANALYSIS_FONT, COLOR_ANALYSIS_BKGD, COLOR_ANALYSIS_BKGD,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_MULTILINE | CONTROL_VISIBLE,
								IDC_STATIC_DATE_OF_READING ),
		m_EditDateOfReading( "", 150, 30, 20, 10, 5, COLOR_ANALYSIS_FONT, COLOR_UNTOUCHED_LIGHT, COLOR_COMPLETED_LIGHT, COLOR_TOUCHED,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_TOP_JUSTIFIED | CONTROL_CLIP | EDIT_BORDER | EDIT_READONLY | CONTROL_VISIBLE,
								IDC_EDIT_DATE_OF_READING ),

		m_StaticSelectClient( "Select client\nfor report\nheading", 100, 60, 14, 7, 6,
								COLOR_ANALYSIS_FONT, COLOR_ANALYSIS_BKGD, COLOR_ANALYSIS_BKGD,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_MULTILINE | CONTROL_VISIBLE,
								IDC_STATIC_SELECT_CLIENT ),
		m_ComboBoxSelectClient( "", 280, 300, 18, 9, 5, VARIABLE_PITCH_FONT,
								COLOR_ANALYSIS_FONT, COLOR_UNTOUCHED_LIGHT, COLOR_COMPLETED_LIGHT, COLOR_TOUCHED,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_VSCROLL | EDIT_BORDER | LIST_SORT | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_COMBO_SELECT_CLIENT ),
		m_ButtonAddClient( "Add Client", 150, 40, 16, 8, 6,
								COLOR_BLACK, COLOR_REPORT, COLOR_REPORT, COLOR_REPORT,
								BUTTON_PUSHBUTTON  | CONTROL_VISIBLE | CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
								IDC_BUTTON_ADD_CLIENT ),
		m_ButtonEditClient( "Edit Info for\nSelected Client", 150, 40, 16, 8, 6,
								COLOR_BLACK, COLOR_REPORT, COLOR_REPORT, COLOR_REPORT,
								BUTTON_PUSHBUTTON | CONTROL_MULTILINE | CONTROL_VISIBLE |
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
								IDC_BUTTON_EDIT_CLIENT ),
		m_ButtonSetdDefaultClient( "Set Default\nClient", 150, 40, 16, 8, 6,
								COLOR_BLACK, COLOR_REPORT, COLOR_REPORT, COLOR_REPORT,
								BUTTON_PUSHBUTTON | CONTROL_MULTILINE | CONTROL_VISIBLE |
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
								IDC_BUTTON_SET_DEFAULT_CLIENT ),
		
		m_StaticSeePhysician( "Should patient see personal\nphysician because of findings on\nthe Other Abnormalities screen?", 240, 50, 14, 7, 6,
								COLOR_ANALYSIS_FONT, COLOR_ANALYSIS_BKGD, COLOR_ANALYSIS_BKGD,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_MULTILINE | CONTROL_VISIBLE,
								IDC_STATIC_SEE_PHYSICIAN ),
		m_ButtonSeePhysicianYes( "YES", 40, 30, 14, 7, 6, COLOR_ANALYSIS_FONT, COLOR_UNTOUCHED, COLOR_COMPLETED, COLOR_TOUCHED,
								BUTTON_CHECKBOX | CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED |
								CONTROL_CLIP | CONTROL_VISIBLE, IDC_BUTTON_SEE_PHYSICIAN_YES ),
		m_ButtonSeePhysicianNo( "NO", 40, 30, 14, 7, 6, COLOR_ANALYSIS_FONT, COLOR_UNTOUCHED, COLOR_COMPLETED, COLOR_TOUCHED,
								BUTTON_CHECKBOX | CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED |
								CONTROL_CLIP | CONTROL_VISIBLE, IDC_BUTTON_SEE_PHYSICIAN_NO ),
		m_SeePhysicianYesNoButtonGroup( BUTTON_CHECKBOX, GROUP_SINGLE_SELECT | GROUP_ONE_TOUCHES_ALL, 2,
								&m_ButtonSeePhysicianYes, &m_ButtonSeePhysicianNo ),
		m_GroupEditSequencing( GROUP_EDIT, GROUP_SEQUENCING, 5,
								&m_EditPatientName, &m_EditPatientID,
								&m_EditOrderingPhysicianName, &m_EditOrderingFacility, &m_EditClassificationPurpose ),
		m_ShowReportButton( "Show\nReport", 120, 40, 16, 8, 6,
								COLOR_WHITE, COLOR_UNTOUCHED, COLOR_COMPLETED_LIGHT, COLOR_COMPLETED_LIGHT,
								BUTTON_PUSHBUTTON | CONTROL_MULTILINE | CONTROL_VISIBLE |
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
								IDC_BUTTON_SHOW_REPORT ),
		m_ApproveReportButton( "Approve\nReport", 120, 40, 16, 8, 6,
								COLOR_WHITE, COLOR_UNTOUCHED, COLOR_COMPLETED_LIGHT, COLOR_COMPLETED_LIGHT,
								BUTTON_PUSHBUTTON | CONTROL_MULTILINE | CONTROL_VISIBLE |
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
								IDC_BUTTON_APPROVE_REPORT ),
		m_StaticSavedReports( "Currently Saved Reports", 300, 30, 18, 9, 6,
								COLOR_ANALYSIS_FONT, COLOR_ANALYSIS_BKGD, COLOR_ANALYSIS_BKGD,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
								IDC_STATIC_SAVED_REPORTS ),
		m_StaticUniqueReportCount( "Unique Report Count:  ", 300, 20, 16, 8, 6,
								COLOR_ANALYSIS_FONT, COLOR_ANALYSIS_BKGD, COLOR_ANALYSIS_BKGD,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
								IDC_STATIC_UNIQUE_REPORT_COUNT ),
		m_PrintCheckedReportsButton( "Print Checked\nReports", 150, 40, 16, 8, 6,
								COLOR_BLACK, COLOR_REPORT, COLOR_REPORT, COLOR_REPORT,
								BUTTON_PUSHBUTTON | CONTROL_MULTILINE | CONTROL_VISIBLE |
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
								IDC_BUTTON_PRINT_CHECKED_REPORTS ),
		m_DeleteCheckedReportsButton( "Delete Checked\nReports", 150, 40, 16, 8, 6,
								COLOR_BLACK, COLOR_REPORT, COLOR_REPORT, COLOR_REPORT,
								BUTTON_PUSHBUTTON | CONTROL_MULTILINE | CONTROL_VISIBLE |
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
								IDC_BUTTON_DELETE_CHECKED_REPORTS ),
		m_DeleteAllReportsButton( "Delete All\nReports", 150, 40, 16, 8, 6,
								COLOR_WHITE, COLOR_RED, COLOR_RED, COLOR_RED,
								BUTTON_PUSHBUTTON | CONTROL_MULTILINE | CONTROL_VISIBLE |
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
								IDC_BUTTON_DELETE_ALL_REPORTS )
{
	m_pReportListCtrl = 0;
	m_bPageIsInitialized = FALSE;
	m_BkgdBrush.CreateSolidBrush( COLOR_REPORT_BKGD );
	m_ReportFieldIncompleteMask = 0L;
	p_CurrentClientInfo = 0;
}


CComposeReportPage::~CComposeReportPage()
{
	if ( m_pReportListCtrl != 0 )
		delete m_pReportListCtrl;
	m_pReportListCtrl = 0;
}


BEGIN_MESSAGE_MAP( CComposeReportPage, CPropertyPage )
	//{{AFX_MSG_MAP(CComposeReportPage)
	ON_NOTIFY( WM_SETFOCUS, IDC_EDIT_DATE_OF_RADIOGRAPH, OnEditDateOfRadiographFocus )
	ON_NOTIFY( WM_SETFOCUS, IDC_EDIT_REPORT_DOB, OnEditDateOfBirthFocus )
	ON_NOTIFY( WM_SETFOCUS, IDC_EDIT_DATE_OF_READING, OnEditDateOfReadingFocus )
	ON_NOTIFY( WM_SETFOCUS, IDC_EDIT_TYPE_OF_READING_OTHER, OnEditTypeOfReadingOtherFocus )

	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_A_READER, OnBnClickedTypeOfReadingA )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_B_READER, OnBnClickedTypeOfReadingB )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_FACILITY_READING, OnBnClickedTypeOfReadingF )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_OTHER_READ, OnBnClickedTypeOfReadingO )

	ON_CBN_SELENDOK( IDC_COMBO_SELECT_CLIENT, OnClientSelected )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_ADD_CLIENT, OnBnClickedAddClient )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_EDIT_CLIENT, OnBnClickedEditClient )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_SET_DEFAULT_CLIENT, OnBnClickedSetDefaultClient )

	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_SEE_PHYSICIAN_YES, OnBnClickedSeePhysicianYes )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_SEE_PHYSICIAN_NO, OnBnClickedSeePhysicianNo )

	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_SHOW_REPORT, OnBnClickedShowReportButton )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_APPROVE_REPORT, OnBnClickedApproveReportButton )

	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_PRINT_CHECKED_REPORTS, OnBnClickedPrintCheckedReportsButton )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_DELETE_CHECKED_REPORTS, OnBnClickedDeleteCheckedReportsButton )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_DELETE_ALL_REPORTS, OnBnClickedDeleteAllReportsButton )

	ON_WM_CTLCOLOR()
	ON_WM_SHOWWINDOW()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CComposeReportPage::OnInitDialog()
{
	RECT			SelectorRect;

	CPropertyPage::OnInitDialog();

	if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
		{
		m_StaticPatientName.SetPosition( 20, 30, this );
		m_EditPatientName.SetPosition( 200, 25, this );

		m_StaticDateOfBirth.SetPosition( 20, 70, this );
		m_EditDateOfBirth.SetPosition( 200, 65, this );

		m_StaticPatientID.SetPosition( 20, 110, this );
		m_EditPatientID.SetPosition( 200, 105, this );

		m_StaticDateOfRadiograph.SetPosition( 20, 150, this );
		m_EditDateOfRadiograph.SetPosition( 200, 145, this );

		m_StaticOrderingPhysicianName.SetPosition( 20, 190, this );
		m_EditOrderingPhysicianName.SetPosition( 200, 185, this );

		m_StaticOrderingFacility.SetPosition( 20, 230, this );
		m_EditOrderingFacility.SetPosition( 200, 225, this );

		m_StaticClassificationPurpose.SetPosition( 20, 270, this );
		m_EditClassificationPurpose.SetPosition( 200, 265, this );

		m_StaticTypeOfReading.SetPosition( 20, 310, this );
		m_ButtonTypeOfReadingA.SetPosition( 200, 310, this );
		m_ButtonTypeOfReadingB.SetPosition( 240, 310, this );
		m_ButtonTypeOfReadingO.SetPosition( 350, 310, this );
		m_EditTypeOfReadingOther.SetPosition( 410, 310, this );
		m_StaticTypeOfReadingOther.SetPosition( 400, 340, this );
		m_TypeOfReadingButtonGroup.m_MemberCount = 4;

		m_StaticDateOfReading.SetPosition( 20, 360, this );
		m_EditDateOfReading.SetPosition( 200, 360, this );

		m_StaticSelectClient.SetPosition( 20, 410, this );
		m_ComboBoxSelectClient.SetPosition( 140, 420, this );
		m_ButtonAddClient.SetPosition( 450, 400, this );
		m_ButtonEditClient.SetPosition( 450, 460, this );
		m_ButtonSetdDefaultClient.SetPosition( 450, 520, this );
		m_bSetDefaultClient = FALSE;

		m_StaticSeePhysician.SetPosition( 20, 500, this );
		m_ButtonSeePhysicianYes.SetPosition( 80, 560, this );
		m_ButtonSeePhysicianNo.SetPosition( 160, 560, this );

		m_ShowReportButton.SetPosition( 100, 670, this );
		m_ApproveReportButton.SetPosition( 480, 670, this );
	
		m_StaticSavedReports.SetPosition( 800, 30, this );
		m_StaticUniqueReportCount.SetPosition( 820, 70, this );

		SelectorRect.top = 100;
		SelectorRect.bottom = 650;
		SelectorRect.left = 650;
		SelectorRect.right = 1200;
		m_pReportListCtrl = new CReportSelector();
		if ( m_pReportListCtrl != 0 )
			{
			m_pReportListCtrl -> Create( WS_CHILD | WS_TABSTOP | WS_DLGFRAME | WS_VISIBLE |
											LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL | LVS_SORTASCENDING,
											SelectorRect, this, IDC_REPORT_LIST );
			m_pReportListCtrl -> SetExtendedStyle( LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
			m_pReportListCtrl -> SetTextBkColor( COLOR_REPORT_SELECTOR_BKGD );
			m_pReportListCtrl -> SetTextColor( COLOR_WHITE );
			}
	
		m_PrintCheckedReportsButton.SetPosition( 680, 670, this );
		m_DeleteCheckedReportsButton.SetPosition( 850, 670, this );
		m_DeleteAllReportsButton.SetPosition( 1020, 670, this );
		m_GroupEditSequencing.m_MemberCount = 5;
		}
	else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
		{
		m_StaticTypeOfReading.SetPosition( 50, 200, this );
		m_ButtonTypeOfReadingA.SetPosition( 320, 200, this );
		m_ButtonTypeOfReadingB.SetPosition( 380, 200, this );
		m_ButtonTypeOfReadingF.SetPosition( 440, 200, this );
		m_TypeOfReadingButtonGroup.m_MemberCount = 3;

		m_StaticDateOfReading.SetPosition( 50, 300, this );
		m_EditDateOfReading.SetPosition( 320, 300, this );

		m_StaticSeePhysician.SetPosition( 50, 350, this );
		m_StaticSeePhysician.SetWindowText( "Should worker see personal\nphysician because of findings on\nthe Other Abnormalities screen?" );
		m_ButtonSeePhysicianYes.SetPosition( 80, 410, this );
		m_ButtonSeePhysicianNo.SetPosition( 160, 410, this );

		m_ShowReportButton.SetPosition( 100, 550, this );
		m_ApproveReportButton.SetPosition( 250, 550, this );
	
		m_StaticSavedReports.SetPosition( 750, 30, this );
		m_StaticUniqueReportCount.SetPosition( 770, 70, this );

		SelectorRect.top = 100;
		SelectorRect.bottom = 650;
		SelectorRect.left = 600;
		SelectorRect.right = 1150;
		m_pReportListCtrl = new CReportSelector();
		if ( m_pReportListCtrl != 0 )
			{
			m_pReportListCtrl -> Create( WS_CHILD | WS_TABSTOP | WS_DLGFRAME | WS_VISIBLE |
											LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL | LVS_SORTASCENDING,
											SelectorRect, this, IDC_REPORT_LIST );
			m_pReportListCtrl -> SetExtendedStyle( LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
			m_pReportListCtrl -> SetTextBkColor( COLOR_REPORT_SELECTOR_BKGD );
			m_pReportListCtrl -> SetTextColor( COLOR_WHITE );
			}
	
		m_GroupEditSequencing.m_MemberCount = 0;

		if ( bMakeDumbButtons )
			{
			m_StaticTypeOfReading.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
			m_ButtonTypeOfReadingA.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
			m_ButtonTypeOfReadingB.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
			m_ButtonTypeOfReadingF.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
			m_StaticDateOfReading.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
			m_EditDateOfReading.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );

			m_PrintCheckedReportsButton.SetPosition( 650, 670, this );
			m_DeleteCheckedReportsButton.SetPosition( 950, 670, this );
			}
		else
			{
			if ( BViewerConfiguration.bMakeDateOfReadingEditable )
				{
				m_StaticDateOfReading.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
				m_EditDateOfReading.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
				}
			else
				{
				m_StaticDateOfReading.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
				m_EditDateOfReading.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
				}

			m_PrintCheckedReportsButton.SetPosition( 630, 670, this );
			m_DeleteCheckedReportsButton.SetPosition( 800, 670, this );
			m_DeleteAllReportsButton.SetPosition( 970, 670, this );
			}
		}
	
	m_bPageIsInitialized = TRUE;

	return TRUE;
}


BOOL CComposeReportPage::LoadClientSelectionList()
{
	BOOL				bNoError = TRUE;
	CStudy				*pCurrentStudy;
	BOOL				bPreviouslyInterpretedStudy;
	LIST_ELEMENT		*pClientListElement;
	CLIENT_INFO			*pClientInfo;
	CLIENT_INFO			*pNewClientInfo;
	int					nItemIndex = 0;				// *[3] Initialize variable.
	int					nSelectedItem;
	CClient				*pClientInfoScreen;

	m_ComboBoxSelectClient.ResetContent();
	m_ComboBoxSelectClient.SetWindowTextA( "Client List" );
	pCurrentStudy = ThisBViewerApp.m_pCurrentStudy;
	if ( pCurrentStudy != 0 )
		bPreviouslyInterpretedStudy = pCurrentStudy -> m_bStudyWasPreviouslyInterpreted;
	else
		bPreviouslyInterpretedStudy = FALSE;

	pClientListElement = AvailableClientList;
	nSelectedItem = 0;
	while ( pClientListElement != 0 )
		{
		pClientInfo = (CLIENT_INFO*)pClientListElement -> pItem;
		nItemIndex = m_ComboBoxSelectClient.AddString( pClientInfo -> Name );
		if ( ( bPreviouslyInterpretedStudy || m_bSetDefaultClient ) && strcmp( pClientInfo -> Name, BViewerConfiguration.m_ClientInfo.Name ) == 0 )
			nSelectedItem = nItemIndex;
		m_ComboBoxSelectClient.SetItemDataPtr( nItemIndex, (void*)pClientInfo );
		pClientListElement = pClientListElement -> pNextListElement;
		}
	// If the current client came from an externally imported, previously completed study,
	// it may not be in the current client selection list.  If not, add it to the list.
	if ( bPreviouslyInterpretedStudy && nSelectedItem <= 0 )		// If current client list does not contain current client...
		{
		pNewClientInfo = (CLIENT_INFO*)malloc( sizeof(CLIENT_INFO) );
		if ( pNewClientInfo != 0 )
			{
			memcpy( pNewClientInfo, &BViewerConfiguration.m_ClientInfo, sizeof(CLIENT_INFO) );
			AppendToList( &AvailableClientList, (void*)pNewClientInfo );
			nSelectedItem = nItemIndex + 1;
			pClientInfoScreen = new CClient( NULL, pNewClientInfo );
			if ( pClientInfoScreen != 0 )
				{
				pClientInfoScreen -> WriteClientFile();
				delete pClientInfoScreen;
				}
			}
		}
	if ( nSelectedItem == 0 )
		memset( &BViewerConfiguration.m_ClientInfo, '\0', sizeof( CLIENT_INFO ) );
	m_ComboBoxSelectClient.SetCurSel( nSelectedItem );
	p_CurrentClientInfo = &BViewerConfiguration.m_ClientInfo;

	return bNoError;
}


void CComposeReportPage::ResetPage()
{
	if ( m_bPageIsInitialized )
		{
		if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
			{
			if ( !m_bSetDefaultClient )
				LoadClientSelectionList();
			else
				m_ComboBoxSelectClient.SetCurSel( m_nSelectedClientItem );
			}
		LoadReportDataFromCurrentStudy();
		if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_GENERAL )
			m_EditDateOfBirth.HasBeenCompleted( m_EditDateOfBirth.m_bHasReceivedInput );
		else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
			m_EditDateOfRadiograph.HasBeenCompleted( m_EditDateOfRadiograph.m_bHasReceivedInput );

		m_EditDateOfReading.HasBeenCompleted( m_EditDateOfReading.m_bHasReceivedInput );
		Invalidate( TRUE );
		}
}


void CComposeReportPage::TurnToggleButtonOn( TomButton *pButton )
{
 	pButton -> m_ToggleState = BUTTON_ON;
	pButton -> SetCheckBoxColor();
	pButton -> m_pGroup -> RespondToSelection( (void*)pButton );
}


void CComposeReportPage::LoadReportDataFromCurrentStudy()
{
	CStudy			*pCurrentStudy;
	char			TextField[ MAX_CFG_STRING_LENGTH ];

	// Initialize all controls to their "off" state.  InitializeMembers() does
	// not affect visibility.
	m_TypeOfReadingButtonGroup.InitializeMembers();
	m_SeePhysicianYesNoButtonGroup.InitializeMembers();

	// If a study has been selected, load the control values from the study.
	pCurrentStudy = ThisBViewerApp.m_pCurrentStudy;
	if ( pCurrentStudy != 0 )
		{
		if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
			{
			m_EditDateOfRadiograph.SetTime( &pCurrentStudy -> m_DateOfRadiograph.Date );
			m_EditDateOfRadiograph.m_bHasReceivedInput = pCurrentStudy -> m_DateOfRadiograph.bDateHasBeenEdited;
			m_EditDateOfRadiograph.HasBeenCompleted( m_EditDateOfRadiograph.m_bHasReceivedInput );
			}

		if ( pCurrentStudy -> m_bStudyWasPreviouslyInterpreted )
			{
			m_StaticDateOfReading.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
			m_EditDateOfReading.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
			}
		else if ( BViewerConfiguration.bMakeDateOfReadingEditable )
			{
			m_StaticDateOfReading.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
			m_EditDateOfReading.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
			}
		else
			{
			m_StaticDateOfReading.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
			m_EditDateOfReading.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
			}

		if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
			{
			if ( pCurrentStudy -> m_TypeOfReading & READING_TYPE_OTHER )
				TurnToggleButtonOn( &m_ButtonTypeOfReadingO );

			strncpy_s( TextField, MAX_CFG_STRING_LENGTH, pCurrentStudy -> m_PatientLastName, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
			if ( strlen( pCurrentStudy -> m_PatientLastName ) > 0 && strlen( pCurrentStudy -> m_PatientFirstName ) > 0 )
				strncat_s( TextField, MAX_CFG_STRING_LENGTH, ", ", _TRUNCATE );									// *[5] Replaced strcat with strncat_s.
			strncat_s( TextField, MAX_CFG_STRING_LENGTH, pCurrentStudy -> m_PatientFirstName, _TRUNCATE );		// *[5] Replaced strcat with strncat_s.
			m_EditPatientName.SetWindowText( TextField );
			m_EditPatientName.m_bHasReceivedInput = ( strlen( TextField ) > 0 );
			m_EditPatientName.HasBeenCompleted( m_EditPatientName.m_bHasReceivedInput );

			if ( pCurrentStudy -> m_PatientsBirthDate.Date.wYear > 0 )
				{
				m_EditDateOfBirth.SetTime( &pCurrentStudy -> m_PatientsBirthDate.Date );
				m_EditDateOfBirth.m_bHasReceivedInput = pCurrentStudy -> m_PatientsBirthDate.bDateHasBeenEdited;
				}
			else
				{
				m_EditDateOfBirth.m_bHasReceivedInput = FALSE;
				SYSTEMTIME			FakeDob;
				memset(&FakeDob, 0, sizeof(FakeDob));
				FakeDob.wYear = 1901;
				FakeDob.wMonth = 1;
				FakeDob.wDay = 1;
				m_EditDateOfBirth.SetTime( &FakeDob );
				}
			m_EditDateOfBirth.HasBeenCompleted( m_EditDateOfBirth.m_bHasReceivedInput );

			m_EditPatientID.SetWindowText( pCurrentStudy -> m_PatientID );
			m_EditPatientID.m_bHasReceivedInput = ( strlen( pCurrentStudy -> m_PatientID ) > 0 );
			m_EditPatientID.HasBeenCompleted( m_EditPatientID.m_bHasReceivedInput );

			m_EditClassificationPurpose.SetWindowText( pCurrentStudy -> m_PatientComments );
			m_EditClassificationPurpose.m_bHasReceivedInput = ( strlen( pCurrentStudy -> m_PatientComments ) > 0 );
			m_EditClassificationPurpose.HasBeenCompleted( m_EditClassificationPurpose.m_bHasReceivedInput );

			// Load the type of reading button group.
			if ( ( pCurrentStudy -> m_TypeOfReading & READING_TYPE_A_READER ) != 0 )
				{
				TurnToggleButtonOn( &m_ButtonTypeOfReadingA );
				m_EditTypeOfReadingOther.HasBeenCompleted( TRUE );
				}
			else if ( ( pCurrentStudy -> m_TypeOfReading & READING_TYPE_B_READER ) != 0 )
				{
				TurnToggleButtonOn( &m_ButtonTypeOfReadingB );
				m_EditTypeOfReadingOther.HasBeenCompleted( TRUE );
				}
			else if ( ( pCurrentStudy -> m_TypeOfReading & READING_TYPE_OTHER ) != 0 )
				{
				TurnToggleButtonOn( &m_ButtonTypeOfReadingO );
				m_EditTypeOfReadingOther.HasBeenCompleted( strlen( pCurrentStudy -> m_OtherTypeOfReading ) > 0 );
				}
			m_EditTypeOfReadingOther.SetWindowText( pCurrentStudy -> m_OtherTypeOfReading );
			m_EditTypeOfReadingOther.m_bHasReceivedInput = ( strlen( pCurrentStudy -> m_OtherTypeOfReading ) > 0 );

			if ( pCurrentStudy -> m_pDiagnosticStudyList != 0 )
				{
				m_EditOrderingPhysicianName.SetWindowText( pCurrentStudy -> m_pDiagnosticStudyList -> ReferringPhysiciansName );
				m_EditOrderingPhysicianName.m_bHasReceivedInput = ( strlen( pCurrentStudy -> m_pDiagnosticStudyList -> ReferringPhysiciansName ) > 0 );
				m_EditOrderingPhysicianName.HasBeenCompleted( m_EditOrderingPhysicianName.m_bHasReceivedInput );

				m_EditOrderingFacility.SetWindowText( pCurrentStudy -> m_pDiagnosticStudyList -> InstitutionName );
				m_EditOrderingFacility.m_bHasReceivedInput = ( strlen( pCurrentStudy -> m_pDiagnosticStudyList -> InstitutionName ) > 0 );
				m_EditOrderingFacility.HasBeenCompleted( m_EditOrderingFacility.m_bHasReceivedInput );
				}
			}
		else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
			{
			// Load the type of reading button group.
			if ( pCurrentStudy -> m_TypeOfReading & READING_TYPE_A_READER )
				TurnToggleButtonOn( &m_ButtonTypeOfReadingA );
			else if ( pCurrentStudy -> m_TypeOfReading & READING_TYPE_B_READER )
				TurnToggleButtonOn( &m_ButtonTypeOfReadingB );
			else if ( pCurrentStudy -> m_TypeOfReading & READING_TYPE_FACILITY )
				TurnToggleButtonOn( &m_ButtonTypeOfReadingF );
			}

		m_EditDateOfReading.SetTime( &pCurrentStudy -> m_DateOfReading.Date );
		m_EditDateOfReading.m_bHasReceivedInput = pCurrentStudy -> m_DateOfReading.bDateHasBeenEdited;
		m_EditDateOfReading.HasBeenCompleted( m_EditDateOfReading.m_bHasReceivedInput );

		if ( pCurrentStudy -> m_PhysicianNotificationStatus & OBSERVED_SEE_PHYSICIAN_YES )
			TurnToggleButtonOn( &m_ButtonSeePhysicianYes );
		if ( pCurrentStudy -> m_PhysicianNotificationStatus & OBSERVED_SEE_PHYSICIAN_NO )
			TurnToggleButtonOn( &m_ButtonSeePhysicianNo );

		m_ShowReportButton.HasBeenPressed( pCurrentStudy -> m_bReportViewed );
		m_ApproveReportButton.HasBeenPressed( pCurrentStudy -> m_bReportApproved );
		}
}


void CComposeReportPage::LoadStudyDataFromScreens()
{
	CStudy			*pCurrentStudy;
	char			TextField[ MAX_CFG_STRING_LENGTH ];
	char			*pDelimiter;
	int				nChars;

	// If a study has been selected, load the control values into the study.
	pCurrentStudy = ThisBViewerApp.m_pCurrentStudy;
	if ( pCurrentStudy != 0 )
		{
		if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
			{
			m_EditDateOfRadiograph.GetTime( &pCurrentStudy -> m_DateOfRadiograph.Date );
			pCurrentStudy -> m_DateOfRadiograph.bDateHasBeenEdited = m_EditDateOfRadiograph.m_bHasReceivedInput;
			}

		pCurrentStudy -> m_TypeOfReading = 0;
		if ( m_ButtonTypeOfReadingA.m_ToggleState == BUTTON_ON )
			pCurrentStudy -> m_TypeOfReading = READING_TYPE_A_READER;
		if ( m_ButtonTypeOfReadingB.m_ToggleState == BUTTON_ON )
			pCurrentStudy -> m_TypeOfReading = READING_TYPE_B_READER;

		if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
			{
			if ( m_ButtonTypeOfReadingO.m_ToggleState == BUTTON_ON )
				pCurrentStudy -> m_TypeOfReading = READING_TYPE_OTHER;

			pCurrentStudy -> m_PatientLastName[ 0 ] = '\0';				// *[1] Eliminated call to strcpy.
			pCurrentStudy -> m_PatientFirstName[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
			m_EditPatientName.GetWindowText( TextField, sizeof( TextField ) );
			pDelimiter = strstr( TextField, ", " );
			if ( pDelimiter != 0 )
				{
				nChars = pDelimiter - TextField;
				strncpy_s( pCurrentStudy -> m_PatientLastName, DICOM_ATTRIBUTE_STRING_LENGTH, TextField, nChars );						// *[3] Replaced strncat with strncat_s.
				strncat_s( pCurrentStudy -> m_PatientFirstName, DICOM_ATTRIBUTE_STRING_LENGTH, &TextField[ nChars + 2 ], _TRUNCATE );	// *[3] Replaced strcat with strncat_s. );
				}
			else
				strncat_s( pCurrentStudy -> m_PatientLastName, DICOM_ATTRIBUTE_STRING_LENGTH, TextField, _TRUNCATE );					// *[3] Replaced strcat with strncat_s. 

			m_EditDateOfBirth.GetTime( &pCurrentStudy -> m_PatientsBirthDate.Date );
			pCurrentStudy -> m_PatientsBirthDate.bDateHasBeenEdited = m_EditDateOfBirth.m_bHasReceivedInput;

			m_EditPatientID.GetWindowText( pCurrentStudy -> m_PatientID, sizeof( pCurrentStudy -> m_PatientID ) );
			m_EditClassificationPurpose.GetWindowText( pCurrentStudy -> m_PatientComments, sizeof( pCurrentStudy -> m_PatientComments ) );
			m_EditTypeOfReadingOther.GetWindowText( pCurrentStudy -> m_OtherTypeOfReading, sizeof( pCurrentStudy -> m_OtherTypeOfReading ) );
			if ( pCurrentStudy -> m_pDiagnosticStudyList != 0 )
				{
				m_EditOrderingPhysicianName.GetWindowText( pCurrentStudy -> m_pDiagnosticStudyList -> ReferringPhysiciansName, sizeof( pCurrentStudy -> m_pDiagnosticStudyList -> ReferringPhysiciansName ) );
				m_EditOrderingFacility.GetWindowText( pCurrentStudy -> m_pDiagnosticStudyList -> InstitutionName, sizeof( pCurrentStudy -> m_pDiagnosticStudyList -> InstitutionName ) );
				}
			}
		else
			{
			if ( m_ButtonTypeOfReadingF.m_ToggleState == BUTTON_ON )
				pCurrentStudy -> m_TypeOfReading = READING_TYPE_FACILITY;
			}
		m_EditDateOfReading.GetTime( &pCurrentStudy -> m_DateOfReading.Date );
		m_EditDateOfReading.UpdateData( TRUE );
		pCurrentStudy -> m_DateOfReading.bDateHasBeenEdited = m_EditDateOfReading.m_bHasReceivedInput;

		pCurrentStudy -> m_PhysicianNotificationStatus = 0L;
		if ( m_ButtonSeePhysicianYes.m_ToggleState == BUTTON_ON )
			pCurrentStudy -> m_PhysicianNotificationStatus |= OBSERVED_SEE_PHYSICIAN_YES;
		if ( m_ButtonSeePhysicianNo.m_ToggleState == BUTTON_ON )
			pCurrentStudy -> m_PhysicianNotificationStatus |= OBSERVED_SEE_PHYSICIAN_NO;
		}
}


static void DeletePopupDialog( void *pResponseDialog )
{
	CPopupDialog			*pPopupDialog;
	
	pPopupDialog = (CPopupDialog*)pResponseDialog;
	if ( pPopupDialog != 0 )
		delete pPopupDialog;
}


// Count the number of unique saved reports.
void CComposeReportPage::SetReportCount()
{
	REPORT_INFO				*pReportInfo;
	int						TotalCount = 0;					// *[4] Initialize the counter.
	int						UniqueReportCount = 0;
	char					**pReportNameArray = 0;			// *[4] Initialize the pointer.
	char					*pReportName;
	int						nReport;
	int						mReport;

	if ( m_pReportListCtrl != 0 )
		{
		// First, count the number of reports in the list.
		pReportInfo = m_pReportListCtrl -> m_pFirstReport;
		while ( pReportInfo != 0 )
			{
			TotalCount++;
			pReportInfo = pReportInfo -> pNextReportInfo;
			}
		sprintf_s( m_ReportCountText, MAX_CFG_STRING_LENGTH, "Total Report Count:  %d", TotalCount );
		LogMessage( m_ReportCountText, MESSAGE_TYPE_SUPPLEMENTARY );					// *[4] Log the total report count.
		// Second, remove the non-unique reports rom the count.
		if ( TotalCount > 0 )
			{
			// Allocate an array to save all the report names.
			pReportNameArray = (char**)calloc( TotalCount, sizeof(char*) );
			if ( pReportNameArray != 0 )
				{
				pReportInfo = m_pReportListCtrl -> m_pFirstReport;
				while ( pReportInfo != 0 )
					{
					pReportName = (char*)malloc( 96 );
					if ( pReportName != 0 )
						{
						strncpy_s( pReportName, 96, pReportInfo -> SubjectName, _TRUNCATE );	// *[4] Save space for null terminator.
						pReportNameArray[ UniqueReportCount++ ] = pReportName;
						}
					pReportInfo = pReportInfo -> pNextReportInfo;
					}
				// Cycle through the list to detect duplicates.
				for ( nReport = 1; nReport < TotalCount; nReport++ )
					{
					for ( mReport = 0; mReport < nReport; mReport++ )
						{
						if ( strcmp( pReportNameArray[ nReport ], pReportNameArray[ mReport ] ) == 0 )
							{
							UniqueReportCount--;
							break;
							}
						}
					}
				}
			}
		}
	sprintf_s( m_ReportCountText, MAX_CFG_STRING_LENGTH, "Unique Report Count:  %d", UniqueReportCount );
	LogMessage( m_ReportCountText, MESSAGE_TYPE_SUPPLEMENTARY );						// *[4] Log the unique report count.
	m_StaticUniqueReportCount.m_ControlText = m_ReportCountText;
	m_StaticUniqueReportCount.Invalidate( TRUE );
	// Deallocate the report name array.
	if ( pReportNameArray != 0 )														// *[3] Added NULL check.
		{
		for ( nReport = 0; nReport < TotalCount; nReport++ )
			if ( pReportNameArray[ nReport ] != 0 )										// *[4] Added NULL check.
				{
				free( pReportNameArray[ nReport ] );
				pReportNameArray[ nReport ] = 0;										// *[4] Indicate no report name allocated.
				}
		free( pReportNameArray );
		}
}


BOOL CComposeReportPage::OnSetActive()
{
 	CStudy							*pCurrentStudy;
	CMainFrame						*pMainFrame;
	static USER_NOTIFICATION_INFO	UserNotificationInfo;
	CControlPanel					*pControlPanel;

	if ( m_bPageIsInitialized )
		{
		bAttendedSession = TRUE;
		LogMessage( "Entering Report Tab.", MESSAGE_TYPE_SUPPLEMENTARY );
		ResetPage();
		LogMessage( "Report Tab Reset OK.", MESSAGE_TYPE_SUPPLEMENTARY );
		m_pReportListCtrl -> UpdateSelectionList();
		LogMessage( "Report Selection List Updated.", MESSAGE_TYPE_SUPPLEMENTARY );
		SetReportCount();
		LogMessage( "Report Count Set.", MESSAGE_TYPE_SUPPLEMENTARY );

		pControlPanel = (CControlPanel*)GetParent();
		if ( pControlPanel != 0 )
			pControlPanel -> m_CurrentlyActivePage = REPORT_PAGE;
		if ( BViewerConfiguration.bAutoGeneratePDFReportsFromAXTFiles )
			bAttendedSession = FALSE;

		pCurrentStudy = ThisBViewerApp.m_pCurrentStudy;
		if ( pCurrentStudy == 0 && !BViewerConfiguration.bAutoGeneratePDFReportsFromAXTFiles )
			{
			// Remind user to select a study.
			pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
			if ( pMainFrame != 0 )
				{
				UserNotificationInfo.WindowWidth = 400;
				UserNotificationInfo.WindowHeight = 300;
				UserNotificationInfo.FontHeight = 0;	// Use default setting;
				UserNotificationInfo.FontWidth = 0;		// Use default setting;
				UserNotificationInfo.UserInputType = USER_INPUT_TYPE_OK;
				UserNotificationInfo.pUserNotificationMessage = "Please select a\nStudy before\nentering study\ndata.";
				UserNotificationInfo.CallbackFunction = DeletePopupDialog;
				pMainFrame -> PerformUserInput( &UserNotificationInfo );
				}
			}
		LogMessage( "Report Tab Activated.", MESSAGE_TYPE_SUPPLEMENTARY );
		}

	return CPropertyPage::OnSetActive();
}


BOOL CComposeReportPage::OnKillActive()
{
	CStudy			*pCurrentStudy;

	if ( !BViewerConfiguration.bAutoGeneratePDFReportsFromAXTFiles )
		{
		pCurrentStudy = ThisBViewerApp.m_pCurrentStudy;
		if ( pCurrentStudy != 0 )
			{
			LoadStudyDataFromScreens();
			pCurrentStudy -> Save();			// *[3] Removed unreferenced return value assignment.
			pCurrentStudy -> UnpackData();		// Refresh the current study data blocks.
			}
		}

	return CPropertyPage::OnKillActive();
}


void CComposeReportPage::ProduceAutomaticReport()
{
	CStudy			*pCurrentStudy;
	NMHDR			NMhdr;
	LRESULT			Result;

	if ( BViewerConfiguration.bAutoGeneratePDFReportsFromAXTFiles )
		{
		bAttendedSession = FALSE;
		pCurrentStudy = ThisBViewerApp.m_pCurrentStudy;
		if ( pCurrentStudy != 0 && pCurrentStudy -> m_bStudyWasPreviouslyInterpreted )
			{
			p_CurrentClientInfo = &pCurrentStudy -> m_ClientInfo;
			ResetPage();
			UpdateWindow();			// Bypass the windows message queue by sending a WM_PAINT message directly.
			OnBnClickedApproveReportButton( &NMhdr, &Result );

			Invalidate( TRUE );		// Signal to repaint the entire client area, erasing the background.
			UpdateWindow();			// Bypass the windows message queue by sending a WM_PAINT message directly.
			}
		}
}


BOOL CComposeReportPage::CheckForReportDataCompletion()
{
	BOOL 			bMandatoryFieldsArePopulated = TRUE;
	CStudy			*pCurrentStudy;

	m_ReportFieldIncompleteMask = 0L;
	pCurrentStudy = ThisBViewerApp.m_pCurrentStudy;
	if ( pCurrentStudy != 0 )
		{
		if ( pCurrentStudy -> m_DateOfRadiograph.Date.wMonth == 0 || pCurrentStudy -> m_DateOfRadiograph.Date.wMonth > 12 )
			m_ReportFieldIncompleteMask |= REPORT_FIELD_RADIOGRAPH_DATE;
		if ( pCurrentStudy -> m_TypeOfReading == READING_TYPE_UNSPECIFIED )
			m_ReportFieldIncompleteMask |= REPORT_FIELD_TYPE_OF_READING;
		if ( ( pCurrentStudy -> m_PhysicianNotificationStatus & ( OBSERVED_SEE_PHYSICIAN_YES | OBSERVED_SEE_PHYSICIAN_NO ) ) == 0 )
			m_ReportFieldIncompleteMask |= REPORT_FIELD_SHOULD_SEE_PHYSICIAN;
		if ( strlen( pCurrentStudy -> m_PatientID ) == 0 )
			m_ReportFieldIncompleteMask |= REPORT_FIELD_PATIENT_ID;
		if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
			{
			if ( strlen( pCurrentStudy -> m_PatientLastName ) == 0 )
				m_ReportFieldIncompleteMask |= REPORT_FIELD_PATIENT_NAME;
			if ( pCurrentStudy -> m_PatientsBirthDate.Date.wMonth == 0 || pCurrentStudy -> m_PatientsBirthDate.Date.wMonth > 12 )
				m_ReportFieldIncompleteMask |= REPORT_FIELD_DOB;
			if ( strlen( pCurrentStudy -> m_pDiagnosticStudyList -> InstitutionName ) == 0 )
				m_ReportFieldIncompleteMask |= REPORT_FIELD_RADIOLOGY_FACILITY;
			if ( strlen( pCurrentStudy -> m_pDiagnosticStudyList -> ReferringPhysiciansName ) == 0 )
				m_ReportFieldIncompleteMask |= REPORT_FIELD_ORDERING_PHYSICIAN;
			if ( strlen( pCurrentStudy -> m_PatientComments ) == 0 )
				m_ReportFieldIncompleteMask |= REPORT_FIELD_CLASSIFICATION_PURPOSE;
			if ( strlen( BViewerCustomization.m_ReaderInfo.ReportSignatureName ) == 0 )
				m_ReportFieldIncompleteMask |= REPORT_FIELD_READER_NAME;
			if ( BViewerCustomization.m_ReaderInfo.pSignatureBitmap == 0 )
				m_ReportFieldIncompleteMask |= REPORT_FIELD_READER_SIGNATURE;
			}
		else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
			{
			if ( strlen( BViewerCustomization.m_ReaderInfo.Initials ) == 0 )
				m_ReportFieldIncompleteMask |= REPORT_FIELD_READER_INITIALS;
			if ( pCurrentStudy -> m_DateOfReading.Date.wMonth == 0 || pCurrentStudy -> m_DateOfReading.Date.wMonth > 12 )
				m_ReportFieldIncompleteMask |= REPORT_FIELD_DATE_OF_READING;
			if ( strlen( BViewerCustomization.m_ReaderInfo.LastName ) == 0 )
				m_ReportFieldIncompleteMask |= REPORT_FIELD_READER_LAST_NAME;
			}
		}	
	bMandatoryFieldsArePopulated = ( m_ReportFieldIncompleteMask == 0L );

	return bMandatoryFieldsArePopulated;
}


void CComposeReportPage::OnEditDateOfRadiographFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
		{
		m_StaticDateOfRadiograph.HasBeenCompleted( TRUE );
		m_EditDateOfRadiograph.HasBeenCompleted( TRUE );
		}
	*pResult = 0;
	Invalidate( TRUE );
}


void CComposeReportPage::OnEditDateOfBirthFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	m_StaticDateOfBirth.HasBeenCompleted( TRUE );
	m_EditDateOfBirth.HasBeenCompleted( TRUE );
	*pResult = 0;
	Invalidate( TRUE );
}


void CComposeReportPage::OnBnClickedTypeOfReadingA( NMHDR *pNMHDR, LRESULT *pResult )
{
	m_ButtonTypeOfReadingA.m_pGroup -> RespondToSelection( (void*)&m_ButtonTypeOfReadingA );
	m_StaticTypeOfReading.HasBeenCompleted( TRUE );

	if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
		{
		m_EditTypeOfReadingOther.SetWindowText( "" );
		m_EditTypeOfReadingOther.HasBeenCompleted( TRUE );
		}

	Invalidate( TRUE );

	*pResult = 0;
}


void CComposeReportPage::OnBnClickedTypeOfReadingB( NMHDR *pNMHDR, LRESULT *pResult )
{
	m_ButtonTypeOfReadingB.m_pGroup -> RespondToSelection( (void*)&m_ButtonTypeOfReadingB );
	m_StaticTypeOfReading.HasBeenCompleted( TRUE );
	Invalidate( TRUE );

	if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
		{
		m_EditTypeOfReadingOther.SetWindowText( "" );
		m_EditTypeOfReadingOther.HasBeenCompleted( TRUE );
		}

	*pResult = 0;
}


void CComposeReportPage::OnBnClickedTypeOfReadingF( NMHDR *pNMHDR, LRESULT *pResult )
{
	m_ButtonTypeOfReadingF.m_pGroup -> RespondToSelection( (void*)&m_ButtonTypeOfReadingF );
	m_StaticTypeOfReading.HasBeenCompleted( TRUE );
	Invalidate( TRUE );

	if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
		{
		m_EditTypeOfReadingOther.SetWindowText( "" );
		m_EditTypeOfReadingOther.HasBeenCompleted( TRUE );
		}

	*pResult = 0;
}


void CComposeReportPage::OnBnClickedTypeOfReadingO( NMHDR *pNMHDR, LRESULT *pResult )
{
	m_ButtonTypeOfReadingO.m_pGroup -> RespondToSelection( (void*)&m_ButtonTypeOfReadingO );
	m_StaticTypeOfReading.HasBeenCompleted( TRUE );
	Invalidate( TRUE );

	*pResult = 0;
}


void CComposeReportPage::OnEditDateOfReadingFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	m_StaticDateOfReading.HasBeenCompleted( TRUE );
	m_EditDateOfReading.HasBeenCompleted( TRUE );
	*pResult = 0;
	Invalidate( TRUE );
}


void CComposeReportPage::OnEditTypeOfReadingOtherFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	m_StaticTypeOfReadingOther.HasBeenCompleted( TRUE );
	m_EditTypeOfReadingOther.HasBeenCompleted( TRUE );
	*pResult = 0;
	Invalidate( TRUE );
	TurnToggleButtonOn( &m_ButtonTypeOfReadingO );
}


void CComposeReportPage::OnBnClickedSeePhysicianYes( NMHDR *pNMHDR, LRESULT *pResult )
{
	m_ButtonSeePhysicianYes.m_pGroup -> RespondToSelection( (void*)&m_ButtonSeePhysicianYes );
	if ( m_ButtonSeePhysicianYes.m_ToggleState == BUTTON_ON )
		m_StaticSeePhysician.HasBeenCompleted( TRUE );
	else
		m_StaticSeePhysician.HasBeenCompleted( FALSE );
	Invalidate( TRUE );

	*pResult = 0;
}


void CComposeReportPage::OnBnClickedSeePhysicianNo( NMHDR *pNMHDR, LRESULT *pResult )
{
	m_ButtonSeePhysicianNo.m_pGroup -> RespondToSelection( (void*)&m_ButtonSeePhysicianNo );
	if ( m_ButtonSeePhysicianNo.m_ToggleState == BUTTON_ON )
		m_StaticSeePhysician.HasBeenCompleted( TRUE );
	else
		m_StaticSeePhysician.HasBeenCompleted( FALSE );
	Invalidate( TRUE );

	*pResult = 0;
}


void CComposeReportPage::OnBnClickedShowReportButton( NMHDR *pNMHDR, LRESULT *pResult )
{
 	CMainFrame				*pMainFrame;
	CImageFrame				*pReportImageFrame;
	CStudy					*pCurrentStudy;
	WINDOWPLACEMENT			WindowPlacement;
	BOOL					bUseCurrentStudy;
	char					Msg[ MAX_LOGGING_STRING_LENGTH ];

	LogMessage( "Viewing Report.", MESSAGE_TYPE_SUPPLEMENTARY );
	pCurrentStudy = ThisBViewerApp.m_pCurrentStudy;
	if ( pCurrentStudy != 0 )
		{
		LoadStudyDataFromScreens();
		pCurrentStudy -> Save();								// *[3] Removed unreferenced error flag.
		pCurrentStudy -> UnpackData();		// Refresh the current study data blocks.
		pCurrentStudy -> m_bReportViewed = TRUE;
		if ( !BViewerConfiguration.bMakeDateOfReadingEditable )
			m_EditDateOfReading.SetTime( &pCurrentStudy -> m_DateOfReading.Date );
		m_EditDateOfReading.m_bHasReceivedInput = pCurrentStudy -> m_DateOfReading.bDateHasBeenEdited;
		m_EditDateOfReading.HasBeenCompleted( m_EditDateOfReading.m_bHasReceivedInput );
		}
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( pMainFrame != 0 )
		{
		pReportImageFrame = pMainFrame -> m_pImageFrame[ IMAGE_FRAME_REPORT ];
		if ( pReportImageFrame != 0 )
			{
			if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
				strncpy_s( pReportImageFrame -> m_CurrentReportFileName, FULL_FILE_SPEC_STRING_LENGTH, "GPReport", _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
			else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
				strncpy_s( pReportImageFrame -> m_CurrentReportFileName, FULL_FILE_SPEC_STRING_LENGTH, "CWHSPReport", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.

			sprintf_s( Msg, MAX_LOGGING_STRING_LENGTH, "Show Report:  %s",  pReportImageFrame -> m_CurrentReportFileName );				// *[2] Added report logging.
			LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );

			pReportImageFrame -> m_ImageView.m_PageNumber = 1;
			pReportImageFrame -> LoadReportPage( 1, &bUseCurrentStudy );
			pReportImageFrame -> m_wndDlgBar.m_ButtonViewAlternatePage.m_ControlText = "Show Page 2";
			// If the report image window is minimized, restore it to normal viewing.
			pReportImageFrame -> GetWindowPlacement( &WindowPlacement );
			if ( WindowPlacement.showCmd == SW_SHOWMINIMIZED )
				{
				WindowPlacement.showCmd = SW_SHOWNORMAL;
				pReportImageFrame -> SetWindowPlacement( &WindowPlacement );
				}
			}
		}
	m_ShowReportButton.HasBeenPressed( TRUE );
	m_ShowReportButton.Invalidate();

	*pResult = 0;
}


static void DeleteCurrentStudy( CStudy *pStudy )
{
	CMainFrame				*pMainFrame;
	
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	ThisBViewerApp.m_pCurrentStudy = 0;
	if ( pMainFrame != 0 )
		{
		// Remove any displayed image from subject study monitor.
		if ( pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] != 0 )
			pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> ClearImageDisplay();
		// Erase any displayed report.
		if ( pMainFrame -> m_pImageFrame[ IMAGE_FRAME_REPORT ] != 0 )
			pMainFrame -> m_pImageFrame[ IMAGE_FRAME_REPORT ] -> ClearImageDisplay();
		}
	pStudy -> DeleteStudyDataAndImages();
	RemoveFromList( &ThisBViewerApp.m_AvailableStudyList, (void*)pStudy );
	delete pStudy;
}


static void ProcessStudyDeletionResponse( void *pResponseDialog )
{
	CStudy					*pStudy;
	CPopupDialog			*pPopupDialog;
	CMainFrame				*pMainFrame;
	
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	pPopupDialog = (CPopupDialog*)pResponseDialog;
	pStudy = (CStudy*)pPopupDialog -> m_pUserNotificationInfo -> pUserData;
	if ( pPopupDialog -> m_pUserNotificationInfo -> UserResponse == POPUP_RESPONSE_YES )
		DeleteCurrentStudy( pStudy );
	delete pPopupDialog;
	if ( pMainFrame != 0 )
		{
		// Activate the subject study list tab on the control panel.
		if ( pMainFrame -> m_pControlPanel != 0 )
			pMainFrame -> m_pControlPanel -> SetActivePage( STUDY_SELECTION_PAGE );
		}
}


void CComposeReportPage::OnBnClickedApproveReportButton( NMHDR *pNMHDR, LRESULT *pResult )
{
 	BOOL							bNoError = TRUE;
 	CMainFrame						*pMainFrame;
 	CStudy							*pStudy;
	static USER_NOTIFICATION_INFO	UserNotificationInfo;
	BOOL							bMandatoryFieldsArePopulated;
	CImageFrame						*pReportImageFrame;
	int								nFieldsUnpopulated;
	int								nBit;
	unsigned long					TestBit;
	static USER_NOTIFICATION		NoticeOfViewableReport;
	BOOL							bProceedWithApproval;
	static USER_NOTIFICATION		NoticeOfMissingReportData;
	LRESULT							Result;
	EVENT_PARAMETERS				*pEventParameters = 0;				// *[1]
	char							ImageFileName[ FULL_FILE_SPEC_STRING_LENGTH ];
	char							PDFReportFileName[ FULL_FILE_SPEC_STRING_LENGTH ];
	int								nChar;
	char							*pInputChar;
	char							*pOutputChar;
	BOOL							bFirstUnderscoreEncountered;
	int								RenameResult;
	char							Msg[ MAX_EXTRA_LONG_STRING_LENGTH ];

	bProceedWithApproval = !bAttendedSession;
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	// Update study with any changes that may have been made.
	pStudy = ThisBViewerApp.m_pCurrentStudy;
	if ( pStudy != 0 && bAttendedSession)
		{
		CheckForIncompleteInterpretation( &bProceedWithApproval );
		if ( bProceedWithApproval && !pStudy -> m_bReportViewed )
			{
			OnBnClickedShowReportButton( pNMHDR, pResult );		// This will mark pStudy -> m_bReportViewed = TRUE;
			strncpy_s( NoticeOfViewableReport.Source, 16, BViewerConfiguration.ProgramName, _TRUNCATE );											// *[1] Replaced strcpy with strncpy_s.
			NoticeOfViewableReport.ModuleCode = 0;
			NoticeOfViewableReport.ErrorCode = 0;
			strncpy_s( NoticeOfViewableReport.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, "The report is now being displayed.\n\n", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
			NoticeOfViewableReport.TypeOfUserResponseSupported = USER_RESPONSE_TYPE_YESNO;
			NoticeOfViewableReport.UserNotificationCause = USER_NOTIFICATION_CAUSE_NEEDS_ACKNOWLEDGMENT;
			strncpy_s( NoticeOfViewableReport.SuggestedActionText, MAX_CFG_STRING_LENGTH, "Proceed with approval?", _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
			NoticeOfViewableReport.UserResponseCode = 0L;
			NoticeOfViewableReport.TextLinesRequired = 10;
			if ( pMainFrame != 0 )
				pMainFrame -> ProcessUserNotificationAndWaitForResponse( &NoticeOfViewableReport );
			if ( NoticeOfViewableReport.UserResponseCode == USER_RESPONSE_CODE_NO )
				bProceedWithApproval = FALSE;
			}
		}
	if ( pStudy != 0 && bProceedWithApproval )
		{
		pReportImageFrame = pMainFrame -> m_pImageFrame[ IMAGE_FRAME_REPORT ];
		if ( pReportImageFrame != 0 )
			{
			sprintf_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "Approve Report:  %s",  pReportImageFrame -> m_CurrentReportFileName );					// *[2] Added report logging.
			LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
			}
		LoadStudyDataFromScreens();
		memcpy( &BViewerConfiguration.m_ClientInfo, &pStudy -> m_ClientInfo, sizeof( CLIENT_INFO ) );
		if ( bAttendedSession && !BViewerConfiguration.bAutoGeneratePDFReportsFromAXTFiles )
			{
			GetLocalTime( &pStudy -> m_DateOfReading.Date );
			pStudy -> m_DateOfReading.bDateHasBeenEdited = TRUE;
			}
		else
			m_EditDateOfReading.SetTime( &pStudy -> m_DateOfReading.Date );
		pStudy -> Save();
		pStudy -> UnpackData();		// Refresh the current study data blocks.
		// Check for mandatory field completion.
		if ( bMakeDumbButtons )		// If this is Test mode, don't ask questions about missing fields.
			bMandatoryFieldsArePopulated = TRUE;
		else
			bMandatoryFieldsArePopulated = CheckForReportDataCompletion();
		// Count the number of unpopulated mandatory fields.
		if ( bAttendedSession && !bMandatoryFieldsArePopulated )
			{
			nFieldsUnpopulated = 0;
			TestBit = 0x00000001;
			for ( nBit = 0; nBit < 32; nBit++ )
				{
				if ( ( m_ReportFieldIncompleteMask & TestBit ) != 0 )
					nFieldsUnpopulated++;
				TestBit <<= 1;
				}
			strncpy_s( NoticeOfMissingReportData.Source, 16, BViewerConfiguration.ProgramName, _TRUNCATE );										// *[1] Replaced strcpy with strncpy_s.
			NoticeOfMissingReportData.ModuleCode = 0;
			NoticeOfMissingReportData.ErrorCode = 0;
			strncpy_s( NoticeOfMissingReportData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH,
						"The following information for the report\nform is missing or incomplete:\n\n", _TRUNCATE );							// *[1] Replaced strcpy with strncpy_s.
			if ( m_ReportFieldIncompleteMask & REPORT_FIELD_RADIOGRAPH_DATE )
				strncat_s( NoticeOfMissingReportData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, "Date of Radiograph\n", _TRUNCATE );				// *[5] Replaced strcat with strncat_s.
			if ( m_ReportFieldIncompleteMask & REPORT_FIELD_TYPE_OF_READING )
				strncat_s( NoticeOfMissingReportData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, "Type Of Reading\n", _TRUNCATE );				// *[5] Replaced strcat with strncat_s.
			if ( m_ReportFieldIncompleteMask & REPORT_FIELD_SHOULD_SEE_PHYSICIAN )
				strncat_s( NoticeOfMissingReportData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, "Should worker see physician?\n", _TRUNCATE );	// *[5] Replaced strcat with strncat_s.
			if ( m_ReportFieldIncompleteMask & REPORT_FIELD_READER_INITIALS )
				strncat_s( NoticeOfMissingReportData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, "Reader's Initials\n", _TRUNCATE );				// *[5] Replaced strcat with strncat_s.
			if ( m_ReportFieldIncompleteMask & REPORT_FIELD_READER_LAST_NAME )
				strncat_s( NoticeOfMissingReportData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, "Reader\'s Last Name\n", _TRUNCATE );			// *[5] Replaced strcat with strncat_s.
			if ( m_ReportFieldIncompleteMask & REPORT_FIELD_READER_STREET_ADDRESS )
				strncat_s( NoticeOfMissingReportData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, "Reader\'s Street Address\n", _TRUNCATE );		// *[5] Replaced strcat with strncat_s.
			if ( m_ReportFieldIncompleteMask & REPORT_FIELD_READER_CITY )
				strncat_s( NoticeOfMissingReportData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, "Reader\'s City\n", _TRUNCATE );					// *[5] Replaced strcat with strncat_s.
			if ( m_ReportFieldIncompleteMask & REPORT_FIELD_READER_STATE )
				strncat_s( NoticeOfMissingReportData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, "Reader\'s State\n", _TRUNCATE );				// *[5] Replaced strcat with strncat_s.
			if ( m_ReportFieldIncompleteMask & REPORT_FIELD_READER_ZIP_CODE )
				strncat_s( NoticeOfMissingReportData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, "Reader\'s Zip Code\n", _TRUNCATE );				// *[5] Replaced strcat with strncat_s.
			if ( m_ReportFieldIncompleteMask & REPORT_FIELD_DATE_OF_READING )
				strncat_s( NoticeOfMissingReportData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, "Date Of Reading\n", _TRUNCATE );				// *[5] Replaced strcat with strncat_s.

			if ( m_ReportFieldIncompleteMask & REPORT_FIELD_PATIENT_NAME )
				strncat_s( NoticeOfMissingReportData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, "Patient\'s Name\n", _TRUNCATE );				// *[5] Replaced strcat with strncat_s.
			if ( m_ReportFieldIncompleteMask & REPORT_FIELD_DOB )
				strncat_s( NoticeOfMissingReportData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, "Patient\'s Date of Birth\n", _TRUNCATE );		// *[5] Replaced strcat with strncat_s.
			if ( m_ReportFieldIncompleteMask & REPORT_FIELD_PATIENT_ID )
				strncat_s( NoticeOfMissingReportData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, "Patient\'s I.D.\n", _TRUNCATE );				// *[5] Replaced strcat with strncat_s.
			if ( m_ReportFieldIncompleteMask & REPORT_FIELD_RADIOLOGY_FACILITY )
				strncat_s( NoticeOfMissingReportData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, "Radiology Facility\n", _TRUNCATE );				// *[5] Replaced strcat with strncat_s.
			if ( m_ReportFieldIncompleteMask & REPORT_FIELD_ORDERING_PHYSICIAN )
				strncat_s( NoticeOfMissingReportData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, "Ordering Physician\n", _TRUNCATE );				// *[5] Replaced strcat with strncat_s.
			if ( m_ReportFieldIncompleteMask & REPORT_FIELD_CLASSIFICATION_PURPOSE )
				strncat_s( NoticeOfMissingReportData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, "Classification Purpose\n", _TRUNCATE );			// *[5] Replaced strcat with strncat_s.
			if ( m_ReportFieldIncompleteMask & REPORT_FIELD_READER_NAME )
				strncat_s( NoticeOfMissingReportData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, "Reader\'s Name\n", _TRUNCATE );					// *[5] Replaced strcat with strncat_s.
			if ( m_ReportFieldIncompleteMask & REPORT_FIELD_READER_SIGNATURE )
				strncat_s( NoticeOfMissingReportData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, "Reader\'s Digital Signature\n", _TRUNCATE );	// *[5] Replaced strcat with strncat_s.

			NoticeOfMissingReportData.TypeOfUserResponseSupported = USER_RESPONSE_TYPE_ERROR | USER_RESPONSE_TYPE_YESNO;
			NoticeOfMissingReportData.UserNotificationCause = USER_NOTIFICATION_CAUSE_REPORT_FIELDS;
			strncpy_s( NoticeOfMissingReportData.SuggestedActionText, MAX_CFG_STRING_LENGTH, "Do you wish to proceed anyway?", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
			NoticeOfMissingReportData.UserResponseCode = 0L;
			NoticeOfMissingReportData.TextLinesRequired = nFieldsUnpopulated + 10;

			if ( pMainFrame != 0 )
				{
				pMainFrame -> ProcessUserNotificationAndWaitForResponse( &NoticeOfMissingReportData );
				if ( NoticeOfMissingReportData.UserResponseCode == USER_RESPONSE_CODE_YES )
					bMandatoryFieldsArePopulated = TRUE;
				}
			}
		if ( bProceedWithApproval )
			{
			// Prepare to process any external subscriptions to this report approval event.
			pEventParameters = (EVENT_PARAMETERS*)malloc( sizeof(EVENT_PARAMETERS) );
			if ( pEventParameters != 0 )
				{
				pEventParameters -> EventID = EVENT_REPORT_APPROVAL;
				pStudy -> m_pEventParameters = pEventParameters;
				strncpy_s( pEventParameters -> PatientFirstName, MAX_USER_INFO_LENGTH, pStudy -> m_PatientFirstName, _TRUNCATE );									// *[1] Replaced strcpy with strncpy_s.
				strncpy_s( pEventParameters -> PatientLastName, MAX_USER_INFO_LENGTH, pStudy -> m_PatientLastName, _TRUNCATE );										// *[1] Replaced strcpy with strncpy_s.
				strncpy_s( pEventParameters -> PatientID, MAX_USER_INFO_LENGTH, pStudy -> m_PatientID, _TRUNCATE );													// *[1] Replaced strcpy with strncpy_s.
				strncpy_s( pEventParameters -> PatientName, MAX_CFG_STRING_LENGTH, pEventParameters -> PatientFirstName, _TRUNCATE );								// *[1] Replaced strcpy with strncpy_s.
				strncat_s( pEventParameters -> PatientName, MAX_CFG_STRING_LENGTH,  " ", _TRUNCATE );																// *[5] Replaced strcat with strncat_s.
				strncat_s( pEventParameters -> PatientName, MAX_CFG_STRING_LENGTH, pEventParameters -> PatientLastName, _TRUNCATE );								// *[5] Replaced strcat with strncat_s.
				if ( pStudy -> m_pCurrentImageInfo != 0 )
					strncpy_s( pEventParameters -> SOPInstanceUID, MAX_CFG_STRING_LENGTH, pStudy -> m_pCurrentImageInfo -> SOPInstanceUID, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
				if ( pStudy -> m_pCurrentStudyInfo != 0 )
					strncpy_s( pEventParameters -> StudyInstanceUID, MAX_CFG_STRING_LENGTH, pStudy -> m_pCurrentStudyInfo -> StudyInstanceUID, _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
				strncpy_s( pEventParameters -> ReaderLastName, MAX_USER_INFO_LENGTH, pStudy -> m_ReaderInfo.LastName, _TRUNCATE );									// *[1] Replaced strcpy with strncpy_s.
				strncpy_s( pEventParameters -> ReaderSignedName, MAX_CFG_STRING_LENGTH, pStudy -> m_ReaderInfo.ReportSignatureName, _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
				if ( strlen( BViewerConfiguration.DicomImageArchiveDirectory ) > 0 )
					{
					strncpy_s( pEventParameters -> DicomImageFilePath, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.DicomImageArchiveDirectory, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
					if ( pEventParameters -> DicomImageFilePath[ strlen( pEventParameters -> DicomImageFilePath ) - 1 ] != '\\' )
						strncat_s( pEventParameters -> DicomImageFilePath, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );											// *[5] Replaced strcat with strncat_s.
					ImageFileName[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
					if ( pStudy ->m_pDiagnosticStudyList != 0 )
						if ( pStudy ->m_pDiagnosticStudyList -> pDiagnosticSeriesList != 0 )
							if ( pStudy ->m_pDiagnosticStudyList -> pDiagnosticSeriesList -> pDiagnosticImageList != 0 )
								{
								strncat_s( ImageFileName, FULL_FILE_SPEC_STRING_LENGTH,
												pStudy -> m_pDiagnosticStudyList -> pDiagnosticSeriesList -> pDiagnosticImageList -> SOPInstanceUID, _TRUNCATE );	// *[3] Replaced strncat with strncat_s.
								strncat_s( ImageFileName, FULL_FILE_SPEC_STRING_LENGTH, ".dcm", _TRUNCATE );														// *[3] Replaced strncat with strncat_s.
								}
					if ( strlen( ImageFileName ) > 0 )
						strncat_s( pEventParameters -> DicomImageFilePath, FULL_FILE_SPEC_STRING_LENGTH, ImageFileName, _TRUNCATE );								// *[3] Replaced strncat with strncat_s.
					else
						pEventParameters -> DicomImageFilePath[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
					}
				}
			if ( bAttendedSession && bMandatoryFieldsArePopulated )
				{
				pStudy -> m_bReportApproved = TRUE;
				GetDateAndTimeForFileName( pStudy -> m_TimeReportApproved, 32 );
				pStudy -> m_TimeReportApproved[ strlen( pStudy -> m_TimeReportApproved ) - 1 ] = '\0';
				}
			CreateAbstractExportFile( pStudy );
			// Save the report file(s).
			if ( pMainFrame != 0 )
				{
				pReportImageFrame = pMainFrame -> m_pImageFrame[ IMAGE_FRAME_REPORT ];
				if ( pReportImageFrame != 0 )
					{
					if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
						strncpy_s( pReportImageFrame -> m_CurrentReportFileName, FULL_FILE_SPEC_STRING_LENGTH, "GPReport", _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
					else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
						strncpy_s( pReportImageFrame -> m_CurrentReportFileName, FULL_FILE_SPEC_STRING_LENGTH, "CWHSPReport", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.


					sprintf_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "Save Report:  %s",  pReportImageFrame -> m_CurrentReportFileName );			// *[2] Added report logging.
					LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );

					pReportImageFrame -> OnSaveReport( 0, &Result );
					bNoError = pReportImageFrame -> m_bReportSavedSuccessfully;
					}
				else
					{
					bNoError = FALSE;
					sprintf_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "*** An error occurred saving report:  %s",  pReportImageFrame -> m_CurrentReportFileName );	// *[2] Added report logging.
					LogMessage( Msg, MESSAGE_TYPE_ERROR );
					}

				// Process any external subscriptions to this report approval event.
				if ( pEventParameters != 0 )
					{
					bNoError = ProcessEventSubscription( pEventParameters );

					if ( bNoError && BViewerConfiguration.bAutoGeneratePDFReportsFromAXTFiles )
						{
						// Rename the .pdf report file according to U.I.H.'s desires
						pInputChar = pEventParameters -> ReportPDFFilePath;
						pOutputChar = PDFReportFileName;
						bFirstUnderscoreEncountered = FALSE;
						for ( nChar = 0; nChar <= (int)strlen( pEventParameters -> ReportPDFFilePath ); nChar++ )
							{
							if ( pEventParameters -> ReportPDFFilePath[ nChar ] != '_' )
								*pOutputChar++ = *pInputChar++;
							else
								{
								if ( !bFirstUnderscoreEncountered )
									{
									bFirstUnderscoreEncountered = TRUE;
									*pOutputChar++ = ',';
									*pOutputChar++ = ' ';
									pInputChar++;
									}
								else
									{
									*pOutputChar++ = ' ';
									pInputChar++;
									}
								}
							}
						RenameResult = rename( pEventParameters -> ReportPDFFilePath, PDFReportFileName );
						if ( RenameResult != 0 )
							{
							bNoError = FALSE;
							strncpy_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "An error occurred attempting to rename the .pdf report file:  \n", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
							strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, pEventParameters -> ReportPDFFilePath, _TRUNCATE );									// *[5] Replaced strcat with strncat_s.
							LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
							}
						}
					pStudy -> m_pEventParameters = 0;
					}
				if ( bAttendedSession )
					{
					// Ask if it is OK to delete the current study.
					if ( bNoError && BViewerConfiguration.bPromptForStudyDeletion &&
								( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL ||
								  BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_NIOSH ) )
						{
						// Ask if it is OK to delete the current study.
						UserNotificationInfo.WindowWidth = 400;
						UserNotificationInfo.WindowHeight = 300;
						UserNotificationInfo.UserInputType = USER_INPUT_TYPE_BOOLEAN;
						UserNotificationInfo.pUserNotificationMessage = "OK to delete\nthe current study?";
						UserNotificationInfo.CallbackFunction = ProcessStudyDeletionResponse;
						UserNotificationInfo.pUserData = (void*)pStudy;
						pMainFrame -> PerformUserInput( &UserNotificationInfo );
						}
					}
				else
					{
					LogMessage( "Deleting current study.", MESSAGE_TYPE_SUPPLEMENTARY );			// *[2] Added activity logging.
					DeleteCurrentStudy( pStudy );
					pStudy = 0;
					}
				}
			if ( pEventParameters != 0 )			// *[1] Fixed potential memory leak.
				{
				free( pEventParameters );			// *[1]
				pEventParameters = 0;				// *[1]
				}
			}			// ... end if mandatory fields have been populated.
		m_pReportListCtrl -> UpdateSelectionList();
		SetReportCount();
		if ( bNoError && pStudy != 0 && pStudy -> m_bReportViewed && pStudy -> m_bReportApproved )
			m_ApproveReportButton.HasBeenPressed( TRUE );
		else
			{
			m_ApproveReportButton.HasBeenPressed( FALSE );
			m_ApproveReportButton.Invalidate( TRUE );
			}
		if ( bNoError && bAttendedSession )
			{
			// Activate the subject study list tab on the control panel.
			if ( pMainFrame != 0 && pMainFrame -> m_pControlPanel != 0 )				// *[3] Added NULL check.
				{
				pMainFrame -> m_pControlPanel -> SetActivePage( STUDY_SELECTION_PAGE );
				CheckWindowsMessages();
				}
			}
		}			// ... end if bProceedWithApproval == TRUE.

	*pResult = 0;
}


void CComposeReportPage::PleaseSelectAStudy()
{
	CMainFrame						*pMainFrame;
	static USER_NOTIFICATION_INFO	UserNotificationInfo;

	// Remind user to select a study.
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( pMainFrame != 0 )
		{
		UserNotificationInfo.WindowWidth = 400;
		UserNotificationInfo.WindowHeight = 300;
		UserNotificationInfo.FontHeight = 0;	// Use default setting;
		UserNotificationInfo.FontWidth = 0;		// Use default setting;
		UserNotificationInfo.UserInputType = USER_INPUT_TYPE_OK;
		UserNotificationInfo.pUserNotificationMessage = "Please select a\nStudy before\nselecting a\nclient.";
		UserNotificationInfo.CallbackFunction = DeletePopupDialog;
		pMainFrame -> PerformUserInput( &UserNotificationInfo );
		}
}


void CComposeReportPage::OnClientSelected()
{
	CLIENT_INFO				*pClientInfo;
	int						nItemIndex;
 	CStudy					*pCurrentStudy;

	nItemIndex = m_ComboBoxSelectClient.GetCurSel();
	pClientInfo = (CLIENT_INFO*)m_ComboBoxSelectClient.GetItemDataPtr( nItemIndex );
	p_CurrentClientInfo = pClientInfo;
	memcpy( &BViewerConfiguration.m_ClientInfo, p_CurrentClientInfo, sizeof( CLIENT_INFO ) );
	pCurrentStudy = ThisBViewerApp.m_pCurrentStudy;
	if ( pCurrentStudy != 0 )
		{
		memcpy( &pCurrentStudy -> m_ClientInfo, p_CurrentClientInfo, sizeof( CLIENT_INFO ) );
		if ( m_bSetDefaultClient )
			{
			m_pDefaultClientInfo = p_CurrentClientInfo;
			m_nSelectedClientItem = m_ComboBoxSelectClient.GetCurSel();
			}
		}
	else
		PleaseSelectAStudy();

}


void CComposeReportPage::OnBnClickedAddClient( NMHDR *pNMHDR, LRESULT *pResult )
{
	CClient					*pClientInfoScreen;
	CLIENT_INFO				*pNewClientInfo;
	BOOL					bCancel;

	pClientInfoScreen = new( CClient );
	if ( pClientInfoScreen != 0 )
		{
		bCancel = !( pClientInfoScreen -> DoModal() == IDOK );
		if ( !bCancel )
			{
			pNewClientInfo = (CLIENT_INFO*)malloc( sizeof(CLIENT_INFO) );
			if ( pNewClientInfo != 0 )
				{
				memcpy( pNewClientInfo, &pClientInfoScreen -> m_ClientInfo, sizeof(CLIENT_INFO) );
				AppendToList( &AvailableClientList, (void*)pNewClientInfo );
				}
			}
		delete pClientInfoScreen;
		}

	LoadClientSelectionList();
	m_bSetDefaultClient = FALSE;
	m_ButtonSetdDefaultClient.m_ControlText = "Set Default\nClient";

	Invalidate( TRUE );

	*pResult = 0;
}


void CComposeReportPage::OnBnClickedEditClient( NMHDR *pNMHDR, LRESULT *pResult )
{
	CClient					*pClientInfoScreen;
	CLIENT_INFO				*pClientInfo;
	int						nItemIndex;
	BOOL					bCancel;

	nItemIndex = m_ComboBoxSelectClient.GetCurSel();
	pClientInfo = (CLIENT_INFO*)m_ComboBoxSelectClient.GetItemDataPtr( nItemIndex );
	pClientInfoScreen = new CClient( NULL, pClientInfo );
	if ( pClientInfoScreen != 0 )
		{
		bCancel = !( pClientInfoScreen -> DoModal() == IDOK );
		if ( !bCancel )
			{
			memcpy( pClientInfo, &pClientInfoScreen -> m_ClientInfo, sizeof(CLIENT_INFO) );
			pClientInfoScreen -> WriteClientFile();
			}
		delete pClientInfoScreen;
		}

	LoadClientSelectionList();
	m_bSetDefaultClient = FALSE;
	m_ButtonSetdDefaultClient.m_ControlText = "Set Default\nClient";

	Invalidate( TRUE );

	*pResult = 0;
}


void CComposeReportPage::OnBnClickedSetDefaultClient( NMHDR *pNMHDR, LRESULT *pResult )
{
	CLIENT_INFO				*pClientInfo;

	if ( m_bSetDefaultClient )
		{
		m_bSetDefaultClient = FALSE;
		m_ButtonSetdDefaultClient.m_ControlText = "Set Default\nClient";
		m_nSelectedClientItem = 0;
		m_ComboBoxSelectClient.SetCurSel( m_nSelectedClientItem );
		memset( &BViewerConfiguration.m_ClientInfo, '\0', sizeof( CLIENT_INFO ) );
		p_CurrentClientInfo = &BViewerConfiguration.m_ClientInfo;
		}
	else
		{
		m_nSelectedClientItem = m_ComboBoxSelectClient.GetCurSel();
		pClientInfo = (CLIENT_INFO*)m_ComboBoxSelectClient.GetItemDataPtr( m_nSelectedClientItem );
		m_pDefaultClientInfo = pClientInfo;
		m_bSetDefaultClient = TRUE;
		m_ButtonSetdDefaultClient.m_ControlText = "Clear Default\nClient";
		}

	Invalidate( TRUE );

	*pResult = 0;
}


void CComposeReportPage::OnBnClickedPrintCheckedReportsButton( NMHDR *pNMHDR, LRESULT *pResult )
{
	int						nItem;
	int						nItems;
	int						nReportsPrinted;
	BOOL					bPrinterOpenedOK;
	REPORT_INFO				*pCheckedReportInfo;
 	CMainFrame				*pMainFrame;
	CImageFrame				*pReportImageFrame;
	CWaitCursor				DisplaysHourglass;
	BOOL					bUseCurrentStudy;

	bPrinterOpenedOK = FALSE;
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( m_pReportListCtrl != 0 && pMainFrame != 0 )
		{
		pReportImageFrame = pMainFrame -> m_pImageFrame[ IMAGE_FRAME_REPORT ];
		if ( pReportImageFrame != 0 )
			{
			nItems = m_pReportListCtrl -> GetItemCount();
			nReportsPrinted = 0;
			for ( nItem = 0; nItem < nItems; nItem++ )
				{
				// If the report list item is checked, remove the item from the list and delete the associated report.
				if ( m_pReportListCtrl -> GetCheck( nItem ) != 0 )
					{
					pCheckedReportInfo = (REPORT_INFO*)m_pReportListCtrl -> GetItemData( nItem );
					if ( pCheckedReportInfo != 0 )
						{
						strncpy_s( pReportImageFrame -> m_CurrentReportFileName, FULL_FILE_SPEC_STRING_LENGTH, pCheckedReportInfo -> ReportFileName, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
						pReportImageFrame -> m_ImageView.m_PageNumber = 1;
						pReportImageFrame -> LoadReportPage( 1, &bUseCurrentStudy );
						bPrinterOpenedOK = pReportImageFrame -> m_ImageView.OpenReportForPrinting( ( nReportsPrinted == 0 ) );
						if ( bPrinterOpenedOK )
							{
							pReportImageFrame -> m_ImageView.PrintReportPage( bUseCurrentStudy );
							pReportImageFrame -> m_ImageView.m_PageNumber = 2;
							pReportImageFrame -> LoadReportPage( 2, &bUseCurrentStudy );
							pReportImageFrame -> m_ImageView.PrintReportPage( bUseCurrentStudy );
							pReportImageFrame -> m_ImageView.CloseReportForPrinting();
							}
						nReportsPrinted++;
						}
					}
				}
			if ( bPrinterOpenedOK )
				pReportImageFrame -> m_ImageView.m_PrinterDC.Detach();
			pReportImageFrame -> ClearImageDisplay();
			}
		}

	*pResult = 0;
}


void CComposeReportPage::OnBnClickedDeleteCheckedReportsButton( NMHDR *pNMHDR, LRESULT *pResult )
{
	BOOL							bNoError;
	int								nItem;
	int								nItems;
	int								nCheckedItems;
	REPORT_INFO						*pCheckedReportInfo;
	REPORT_INFO						*pReportInfo;
	REPORT_INFO						*pPrevReportInfo;
	char							ReportDirectory[ FILE_PATH_STRING_LENGTH ];
	char							FullImageFileSpecification[ FILE_PATH_STRING_LENGTH ];
 	CMainFrame						*pMainFrame;
	BOOL							bCheckedReportFound;
	static USER_NOTIFICATION		NoticeOfExistingData;
	BOOL							bDeleteRequestWasConfirmed = FALSE;

	strncpy_s( ReportDirectory, FILE_PATH_STRING_LENGTH, BViewerConfiguration.ReportDirectory, _TRUNCATE );		// *[3] Replaced strncat with strncpy_s.
	if ( ReportDirectory[ strlen( ReportDirectory ) - 1 ] != '\\' )
		strncat_s( ReportDirectory, FILE_PATH_STRING_LENGTH, "\\", _TRUNCATE );									// *[3] Replaced strcat with strncat_s.
	// Check existence of path to configuration directory.
	bNoError = SetCurrentDirectory( ReportDirectory );
	if ( bNoError && m_pReportListCtrl != 0 )
		{
		nItems = m_pReportListCtrl -> GetItemCount();
		nCheckedItems = 0;
		for ( nItem = 0; nItem < nItems && bNoError; nItem++ )
			{
			// Count the checked items in the report list control.
			if ( m_pReportListCtrl -> GetCheck( nItem ) != 0 )
				nCheckedItems++;
			}
		strncpy_s( NoticeOfExistingData.Source, 16, BViewerConfiguration.ProgramName, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
		NoticeOfExistingData.ModuleCode = 0;
		NoticeOfExistingData.ErrorCode = 0;
		NoticeOfExistingData.UserResponseCode = 0L;
		NoticeOfExistingData.TextLinesRequired = 10;
		NoticeOfExistingData.UserNotificationCause = USER_NOTIFICATION_CAUSE_NEEDS_ACKNOWLEDGMENT;
		pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
		if ( nCheckedItems > 0 )
			{
			if ( nCheckedItems == 1 )
				strncpy_s( NoticeOfExistingData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, "You are about to delete the saved report.\n", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
			else
				_snprintf_s( NoticeOfExistingData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, _TRUNCATE,														// *[3] Replaced sprintf() with _snprintf_s.
										"You are about to delete the %d saved reports.\n", nCheckedItems );
			NoticeOfExistingData.TypeOfUserResponseSupported = USER_RESPONSE_TYPE_YESNO_NO_CANCEL;
			strncpy_s( NoticeOfExistingData.SuggestedActionText, MAX_CFG_STRING_LENGTH, "Are you sure you wish to proceed?", _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
			if ( pMainFrame != 0 )
				pMainFrame -> ProcessUserNotificationAndWaitForResponse( &NoticeOfExistingData );
			if ( NoticeOfExistingData.UserResponseCode == USER_RESPONSE_CODE_YES )
				bDeleteRequestWasConfirmed = TRUE;
			}
		else
			{
			strncpy_s( NoticeOfExistingData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, "No report items are checked.\n", _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.
			NoticeOfExistingData.TypeOfUserResponseSupported = USER_RESPONSE_TYPE_CONTINUE;
			strncpy_s( NoticeOfExistingData.SuggestedActionText, MAX_CFG_STRING_LENGTH, "Please check the items you wish to delete.", _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
			if ( pMainFrame != 0 )
				pMainFrame -> ProcessUserNotificationAndWaitForResponse( &NoticeOfExistingData );
			}
		if ( bDeleteRequestWasConfirmed )
			{
			for ( nItem = 0; nItem < nItems && bNoError; nItem++ )
				{
				// If the report list item is checked, remove the item from the list and delete the associated report.
				if ( m_pReportListCtrl -> GetCheck( nItem ) != 0 )
					{
					pCheckedReportInfo = (REPORT_INFO*)m_pReportListCtrl -> GetItemData( nItem );
					if ( pCheckedReportInfo != 0 )
						{
						// Delete the two report image files.
						strncpy_s( FullImageFileSpecification, FILE_PATH_STRING_LENGTH, ReportDirectory, _TRUNCATE );							// *[1] Replaced strcpy with strncpy_s.
						strncat_s( FullImageFileSpecification, FILE_PATH_STRING_LENGTH, pCheckedReportInfo -> ReportFileName, _TRUNCATE );		// *[5] Replaced strcat with strncat_s.
						strncat_s( FullImageFileSpecification, FILE_PATH_STRING_LENGTH, "__ReportPage1.png", _TRUNCATE );						// *[5] Replaced strcat with strncat_s.
						DeleteFile( FullImageFileSpecification );
						strncpy_s( FullImageFileSpecification, FILE_PATH_STRING_LENGTH, ReportDirectory, _TRUNCATE );							// *[1] Replaced strcpy with strncpy_s.
						strncat_s( FullImageFileSpecification, FILE_PATH_STRING_LENGTH, pCheckedReportInfo -> ReportFileName, _TRUNCATE );		// *[5] Replaced strcat with strncat_s.
						strncat_s( FullImageFileSpecification, FILE_PATH_STRING_LENGTH, "__ReportPage2.png", _TRUNCATE );						// *[5] Replaced strcat with strncat_s.
						DeleteFile( FullImageFileSpecification );

						// Delete the report info structure from the linked list.
						pReportInfo = m_pReportListCtrl -> m_pFirstReport;
						pPrevReportInfo = 0;
						bCheckedReportFound = FALSE;
						while ( pReportInfo != 0 && !bCheckedReportFound )
							{
							if ( pReportInfo == pCheckedReportInfo )
								{
								// Remove the matching item from the linked list.
								bCheckedReportFound = TRUE;
								if ( pPrevReportInfo != 0 )
									pPrevReportInfo -> pNextReportInfo = pReportInfo -> pNextReportInfo;
								else
									m_pReportListCtrl -> m_pFirstReport = pReportInfo -> pNextReportInfo;
								free( pReportInfo );
								pReportInfo = 0;			// *[1] Added this for code safety.
								}
							else
								{
								pPrevReportInfo = pReportInfo;
								pReportInfo = pReportInfo -> pNextReportInfo;
								}
							}
						}
					}
				}
			m_pReportListCtrl -> UpdateReportListDisplay();			// Update the list control showing that the deleted reports are gone.
			pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
			if ( pMainFrame != 0 && pMainFrame -> m_pImageFrame[ IMAGE_FRAME_REPORT ] != 0 )
				pMainFrame -> m_pImageFrame[ IMAGE_FRAME_REPORT ] -> ClearImageDisplay();
			}
		}

	*pResult = 0;
}


void CComposeReportPage::OnBnClickedDeleteAllReportsButton( NMHDR *pNMHDR, LRESULT *pResult )
{
	BOOL							bNoError;
	int								nItem;
	int								nReportItems;
	REPORT_INFO						*pCheckedReportInfo;
	REPORT_INFO						*pReportInfo;
	REPORT_INFO						*pPrevReportInfo;
	char							ReportDirectory[ FILE_PATH_STRING_LENGTH ];
	char							FullImageFileSpecification[ FILE_PATH_STRING_LENGTH ];
 	CMainFrame						*pMainFrame;
	BOOL							bReportFound;
	static USER_NOTIFICATION		NoticeOfExistingData;
	BOOL							bDeleteRequestWasConfirmed = FALSE;

	strncpy_s( ReportDirectory, FILE_PATH_STRING_LENGTH, BViewerConfiguration.ReportDirectory, _TRUNCATE );		// *[3] Replaced strncat with strncpy_s.
	if ( ReportDirectory[ strlen( ReportDirectory ) - 1 ] != '\\' )
		strncat_s( ReportDirectory, FILE_PATH_STRING_LENGTH, "\\", _TRUNCATE );									// *[3] Replaced strcat with strncat_s.
	// Check existence of path to configuration directory.
	bNoError = SetCurrentDirectory( ReportDirectory );
	if ( bNoError && m_pReportListCtrl != 0 )
		{
		nReportItems = m_pReportListCtrl -> GetItemCount();
		strncpy_s( NoticeOfExistingData.Source, 16, BViewerConfiguration.ProgramName, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
		NoticeOfExistingData.ModuleCode = 0;
		NoticeOfExistingData.ErrorCode = 0;
		NoticeOfExistingData.UserResponseCode = 0L;
		NoticeOfExistingData.TextLinesRequired = 10;
		NoticeOfExistingData.UserNotificationCause = USER_NOTIFICATION_CAUSE_NEEDS_ACKNOWLEDGMENT;
		pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
		if ( nReportItems > 0 )
			{
			if ( nReportItems == 1 )
				strncpy_s( NoticeOfExistingData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, "You are about to delete the saved report.\n", _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
			else
				_snprintf_s( NoticeOfExistingData.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, _TRUNCATE,													// *[3] Replaced sprintf() with _snprintf_s.
											"You are about to delete the %d saved reports.\n", nReportItems );
			NoticeOfExistingData.TypeOfUserResponseSupported = USER_RESPONSE_TYPE_YESNO_NO_CANCEL;
			strncpy_s( NoticeOfExistingData.SuggestedActionText, MAX_CFG_STRING_LENGTH, "Are you sure you wish to proceed?", _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
			if ( pMainFrame != 0 )
				pMainFrame -> ProcessUserNotificationAndWaitForResponse( &NoticeOfExistingData );
			if ( NoticeOfExistingData.UserResponseCode == USER_RESPONSE_CODE_YES )
				bDeleteRequestWasConfirmed = TRUE;
			}
		if ( bDeleteRequestWasConfirmed )
			{
			for ( nItem = 0; nItem < nReportItems && bNoError; nItem++ )
				{
				// Remove each item from the list and delete the associated report.
				pCheckedReportInfo = (REPORT_INFO*)m_pReportListCtrl -> GetItemData( nItem );
				if ( pCheckedReportInfo != 0 )
					{
					// Delete the two report image files.
					strncpy_s( FullImageFileSpecification, FILE_PATH_STRING_LENGTH, ReportDirectory, _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.
					strncat_s( FullImageFileSpecification, FILE_PATH_STRING_LENGTH, pCheckedReportInfo -> ReportFileName, _TRUNCATE );	// *[5] Replaced strcat with strncat_s.
					strncat_s( FullImageFileSpecification, FILE_PATH_STRING_LENGTH, "__ReportPage1.png", _TRUNCATE );					// *[5] Replaced strcat with strncat_s.
					DeleteFile( FullImageFileSpecification );
					strncpy_s( FullImageFileSpecification, FILE_PATH_STRING_LENGTH, ReportDirectory, _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.
					strncat_s( FullImageFileSpecification, FILE_PATH_STRING_LENGTH, pCheckedReportInfo -> ReportFileName, _TRUNCATE );	// *[5] Replaced strcat with strncat_s.
					strncat_s( FullImageFileSpecification, FILE_PATH_STRING_LENGTH, "__ReportPage2.png", _TRUNCATE );					// *[5] Replaced strcat with strncat_s.
					DeleteFile( FullImageFileSpecification );

					// Delete the report info structure from the linked list.
					pReportInfo = m_pReportListCtrl -> m_pFirstReport;
					pPrevReportInfo = 0;
					bReportFound = FALSE;
					while ( pReportInfo != 0 && !bReportFound )
						{
						if ( pReportInfo == pCheckedReportInfo )
							{
							bReportFound = TRUE;
							// Remove the matching item from the linked list.
							if ( pPrevReportInfo != 0 )
								pPrevReportInfo -> pNextReportInfo = pReportInfo -> pNextReportInfo;
							else
								m_pReportListCtrl -> m_pFirstReport = pReportInfo -> pNextReportInfo;
							free( pReportInfo );
							pReportInfo = 0;			// *[1] Added this for code safety.
							}
						else
							{
							pPrevReportInfo = pReportInfo;
							pReportInfo = pReportInfo -> pNextReportInfo;
							}
						}
					}
				}
			m_pReportListCtrl -> UpdateReportListDisplay();
			pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
			if ( pMainFrame != 0 && pMainFrame -> m_pImageFrame[ IMAGE_FRAME_REPORT ] != 0 )
				pMainFrame -> m_pImageFrame[ IMAGE_FRAME_REPORT ] -> ClearImageDisplay();
			}
		}

	*pResult = 0;
}


void CComposeReportPage::OnReportItemSelected()
{
	m_pReportListCtrl -> OnReportItemSelected();
}


HBRUSH CComposeReportPage::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
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


