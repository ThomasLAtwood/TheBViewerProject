// ManualStudyEntry.cpp - Implements the data structures and functions for the manual
//  study data entry dialog box.  This is where the user enters the patient
//  information required to create a study without an image file.
//
//	Written by Thomas L. Atwood
//	P.O. Box 1089
//	West Fork, Arkansas 72774
//	(479)445-4690
//	TomAtwood@Earthlink.net
//
//	Copyright © 2016 CDC
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
#include "ManualStudyEntry.h"


// CManualStudyEntry dialog
CManualStudyEntry::CManualStudyEntry( CWnd *pParent /*=NULL*/ )
			: CDialog( CManualStudyEntry::IDD, pParent ),
				m_StaticTitle( "Manual Entry of Study Information", 400, 50, 18, 9, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_TOP_JUSTIFIED | CONTROL_VISIBLE,
										IDC_STATIC_MANUAL_STUDY_ENTRY ),
				m_StaticPatientFirstName( "Patient First Name", 200, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_PATIENT_FIRST_NAME ),
				m_EditPatientFirstName( "", 300, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_PATIENT_FIRST_NAME ),

				m_StaticPatientLastName( "Patient Last Name", 200, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_PATIENT_LAST_NAME ),
				m_EditPatientLastName( "", 300, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_PATIENT_LAST_NAME ),

				m_StaticPatientID( "Patient ID Number", 300, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_PATIENT_ID_NUMBER ),
				m_EditPatientID( "", 200, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_PATIENT_ID_NUMBER ),

				m_StaticDateOfBirth( "Date of Birth", 150, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_DATE_OF_BIRTH ),
				m_EditDateOfBirth( "", 150, 30, 20, 10, 5, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_TOP_JUSTIFIED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
										IDC_EDIT_DATE_OF_BIRTH ),

				m_StaticPatientSex( "Sex ( M or F )", 100, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_PATIENT_SEX ),
				m_EditPatientSex( "", 70, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_PATIENT_SEX ),

				m_StaticStudyDate( "Date of Study", 150, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE | CONTROL_MULTILINE,
										IDC_STATIC_DATE_OF_STUDY ),
				m_EditStudyDate( "", 150, 30, 20, 10, 5, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_TOP_JUSTIFIED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
										IDC_EDIT_DATE_OF_STUDY ),

				m_StaticAccessionNumber( "Accession Number", 150, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_ACCESSION_NUMBER,
									"Change this to distinguish different studies\nwithin the same patient visit." ),
				m_EditAccessionNumber( "", 200, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_NONE, IDC_EDIT_ACCESSION_NUMBER ),

				m_StaticOrderingInstitution( "Institution Name", 150, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_ORDERING_INSTITUTION ),
				m_EditOrderingInstitution( "", 300, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_ORDERING_INSTITUTION ),

				m_StaticReferringPhysiciansName( "Referring Physician", 150, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_REFERRING_PHYSICIAN ),
				m_EditReferringPhysiciansName( "", 300, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_REFERRING_PHYSICIAN ),

				m_GroupEditSequencing( GROUP_EDIT, GROUP_SEQUENCING, 9,
									&m_EditPatientFirstName, &m_EditPatientLastName, &m_EditPatientID, &m_EditDateOfBirth,
									&m_EditPatientSex, &m_EditAccessionNumber,
									&m_EditOrderingInstitution, &m_EditReferringPhysiciansName, &m_EditStudyDate ),

				m_ButtonCreateManualStudy( "Create Manual Study", 200, 30, 16, 8, 6,
								COLOR_BLACK, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR,
								BUTTON_PUSHBUTTON | CONTROL_VISIBLE |
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
								IDC_BUTTON_CREATE_MANUAL_STUDY ),
				m_ButtonCancel( "Cancel", 150, 30, 16, 8, 6,
								COLOR_BLACK, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR,
								BUTTON_PUSHBUTTON | CONTROL_VISIBLE |
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
								IDC_BUTTON_CANCEL_MANUAL_STUDY )
{
	m_BkgdBrush.CreateSolidBrush( COLOR_CONFIG );
}


CManualStudyEntry::~CManualStudyEntry()
{
	DestroyWindow();
}


BEGIN_MESSAGE_MAP( CManualStudyEntry, CDialog )
	//{{AFX_MSG_MAP(CManualStudyEntry)
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_CREATE_MANUAL_STUDY, OnBnClickedCreateManualStudy )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_CANCEL_MANUAL_STUDY, OnBnClickedCancelManualStudy )
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CManualStudyEntry::OnInitDialog()
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

	m_StaticTitle.SetPosition( 100, 30, this );

	m_StaticPatientFirstName.SetPosition( 40, 70, this );
	m_EditPatientFirstName.SetPosition( 240, 70, this );

	m_StaticPatientLastName.SetPosition( 40, 100, this );
	m_EditPatientLastName.SetPosition( 240, 100, this );
	
	m_StaticPatientID.SetPosition( 40, 130, this );
	m_EditPatientID.SetPosition( 240, 130, this );
	
	m_StaticDateOfBirth.SetPosition( 40, 160, this );
	m_EditDateOfBirth.SetPosition( 240, 160, this );
	
	m_StaticPatientSex.SetPosition( 40, 200, this );
	m_EditPatientSex.SetPosition( 240, 200, this );

	m_StaticAccessionNumber.SetPosition( 40, 230, this );
	m_EditAccessionNumber.SetPosition( 240, 230, this );

	m_StaticOrderingInstitution.SetPosition( 40, 260, this );
	m_EditOrderingInstitution.SetPosition( 240, 260, this );

	m_StaticReferringPhysiciansName.SetPosition( 40, 290, this );
	m_EditReferringPhysiciansName.SetPosition( 240, 290, this );

	m_StaticStudyDate.SetPosition( 40, 320, this );
	m_EditStudyDate.SetPosition( 240, 320, this );

	m_ButtonCreateManualStudy.SetPosition( 100, 370, this );
	m_ButtonCancel.SetPosition( 400, 370, this );

	PrimaryScreenWidth = ::GetSystemMetrics( SM_CXSCREEN );
	PrimaryScreenHeight = ::GetSystemMetrics( SM_CYSCREEN );

	m_EditPatientFirstName.SetWindowText( "" );
	m_EditPatientLastName.SetWindowText( "" );
	m_EditPatientID.SetWindowText( "" );
	m_EditDateOfBirth.SetWindowText( "" );
	m_EditPatientSex.SetWindowText( "" );
	m_EditStudyDate.SetWindowText( "" );
	m_EditAccessionNumber.SetWindowText( "1" );
	m_EditOrderingInstitution.SetWindowText( "" );
	m_EditReferringPhysiciansName.SetWindowText( "" );

	SetWindowPos( &wndTop, ( PrimaryScreenWidth - 750 ) / 2, ( PrimaryScreenHeight - 350 ) / 2, 600, 460, SWP_SHOWWINDOW );

	m_EditPatientFirstName.SetFocus();

	return TRUE; 
}


void CManualStudyEntry::OnBnClickedCreateManualStudy( NMHDR *pNMHDR, LRESULT *pResult )
{
	SYSTEMTIME				*pDate;

	m_EditPatientFirstName.GetWindowText( m_PatientFirstName, DICOM_ATTRIBUTE_UI_STRING_LENGTH );
	m_EditPatientLastName.GetWindowText( m_PatientLastName, DICOM_ATTRIBUTE_UI_STRING_LENGTH );
	m_EditPatientID.GetWindowText( m_PatientID, DICOM_ATTRIBUTE_UI_STRING_LENGTH );

	m_EditDateOfBirth.GetTime( &m_DateOfBirth.Date );
	pDate = &m_DateOfBirth.Date;
	if ( m_EditDateOfBirth.m_bHasReceivedInput )
		sprintf( m_DateOfBirthText, "%4u%2u%2u", pDate -> wYear, pDate -> wMonth, pDate -> wDay );
	else
		strcpy( m_DateOfBirthText, "" );

	m_EditPatientSex.GetWindowText( m_PatientSex, DICOM_ATTRIBUTE_UI_STRING_LENGTH );
	m_PatientSex[ 0 ] = toupper( m_PatientSex[ 0 ] );
	m_PatientSex[ 1 ] = '\0';

	m_EditStudyDate.GetTime( &m_StudyDate.Date );
	pDate = &m_StudyDate.Date;
	if ( m_EditStudyDate.m_bHasReceivedInput )
		sprintf( m_StudyDateText, "%4u%2u%2u", pDate -> wYear, pDate -> wMonth, pDate -> wDay );
	else
		strcpy( m_StudyDateText, "" );

	m_EditAccessionNumber.GetWindowText( m_AccessionNumber, DICOM_ATTRIBUTE_UI_STRING_LENGTH );
	m_EditOrderingInstitution.GetWindowText( m_OrderingInstitution, DICOM_ATTRIBUTE_UI_STRING_LENGTH );
	m_EditReferringPhysiciansName.GetWindowText( m_ReferringPhysiciansName, DICOM_ATTRIBUTE_UI_STRING_LENGTH );

	m_ButtonCreateManualStudy.HasBeenPressed( TRUE );
	CDialog::OnOK();

	*pResult = 0;
}


void CManualStudyEntry::OnBnClickedCancelManualStudy( NMHDR *pNMHDR, LRESULT *pResult )
{
	m_ButtonCancel.HasBeenPressed( TRUE );
	CDialog::OnCancel();

	*pResult = 0;
}


HBRUSH CManualStudyEntry::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
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


BOOL CManualStudyEntry::OnEraseBkgnd( CDC *pDC )
{
	CBrush		BackgroundBrush( COLOR_CONFIG );
	CRect		BackgroundRectangle;
	CBrush		*pOldBrush = pDC -> SelectObject( &BackgroundBrush );

	GetClientRect( BackgroundRectangle );
	pDC -> FillRect( BackgroundRectangle, &BackgroundBrush );
	pDC -> SelectObject( pOldBrush );

	return TRUE;
}


void CManualStudyEntry::OnOK()
{
	// CDialog::OnOK();		The only way to call this is via the button.
}


void CManualStudyEntry::OnClose()
{
	CDialog::OnClose();
}


