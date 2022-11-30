// ComposeReportPage.h : Header file defining the structure of the CComposeReportPage
//  class of CPropertyPage, which implements the "Produce Report" tab of the main
//  Control Panel.
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
#pragma once

#include "TomGroup.h"
#include "TomStatic.h"
#include "TomButton.h"
#include "TomEdit.h"
#include "TomEditDate.h"
#include "TomComboBox.h"

#include "ReportSelector.h"


// CComposeReportPage dialog

class CComposeReportPage : public CPropertyPage
{
public:
	CComposeReportPage();
	virtual ~CComposeReportPage();

	BOOL				m_bPageIsInitialized;
	CBrush				m_BkgdBrush;
	unsigned long		m_ReportFieldIncompleteMask;
							#define REPORT_FIELD_RADIOGRAPH_DATE			0x00000001
							#define REPORT_FIELD_TYPE_OF_READING			0x00000004
							#define REPORT_FIELD_SHOULD_SEE_PHYSICIAN		0x00000010
							#define REPORT_FIELD_READER_INITIALS			0x00000020
							#define REPORT_FIELD_DATE_OF_READING			0x00000040
							#define REPORT_FIELD_READER_LAST_NAME			0x00000080
							#define REPORT_FIELD_READER_STREET_ADDRESS		0x00000100
							#define REPORT_FIELD_READER_CITY				0x00000200
							#define REPORT_FIELD_READER_STATE				0x00000400
							#define REPORT_FIELD_READER_ZIP_CODE			0x00000800
							#define REPORT_FIELD_PATIENT_NAME				0x00001000
							#define REPORT_FIELD_DOB						0x00002000
							#define REPORT_FIELD_PATIENT_ID					0x00004000
							#define REPORT_FIELD_RADIOLOGY_FACILITY			0x00008000
							#define REPORT_FIELD_ORDERING_PHYSICIAN			0x00010000
							#define REPORT_FIELD_CLASSIFICATION_PURPOSE		0x00020000
							#define REPORT_FIELD_READER_NAME				0x00040000
							#define REPORT_FIELD_READER_SIGNATURE			0x00080000


	TomStatic			m_StaticPatientName;
		TomEdit				m_EditPatientName;
	TomStatic			m_StaticDateOfBirth;
		TomEditDate			m_EditDateOfBirth;
	TomStatic			m_StaticPatientID;
		TomEdit				m_EditPatientID;
	TomStatic			m_StaticDateOfRadiograph;
		TomEditDate			m_EditDateOfRadiograph;
	TomStatic			m_StaticOrderingPhysicianName;
		TomEdit				m_EditOrderingPhysicianName;
	TomStatic			m_StaticOrderingFacility;
		TomEdit				m_EditOrderingFacility;
	TomStatic			m_StaticClassificationPurpose;
		TomEdit				m_EditClassificationPurpose;
	TomStatic			m_StaticTypeOfReading;
		TomButton			m_ButtonTypeOfReadingA;
		TomButton			m_ButtonTypeOfReadingB;
		TomButton			m_ButtonTypeOfReadingF;
		TomButton			m_ButtonTypeOfReadingO;
		TomGroup			m_TypeOfReadingButtonGroup;
		TomEdit				m_EditTypeOfReadingOther;
		TomStatic			m_StaticTypeOfReadingOther;
	TomStatic			m_StaticDateOfReading;
		TomEditDate			m_EditDateOfReading;

	TomStatic			m_StaticSelectClient;
		TomComboBox			m_ComboBoxSelectClient;
		TomButton			m_ButtonAddClient;
		TomButton			m_ButtonEditClient;


	TomStatic			m_StaticSeePhysician;
		TomButton			m_ButtonSeePhysicianYes;
		TomButton			m_ButtonSeePhysicianNo;
		TomGroup			m_SeePhysicianYesNoButtonGroup;
		TomGroup			m_GroupEditSequencing;

	TomButton			m_ShowReportButton;
	TomButton			m_ApproveReportButton;

	TomStatic			m_StaticSavedReports;
	TomStatic			m_StaticUniqueReportCount;
	TomButton			m_PrintCheckedReportsButton;
	TomButton			m_DeleteCheckedReportsButton;
	TomButton			m_DeleteAllReportsButton;

	CReportSelector		*m_pReportListCtrl;

	TomButton			m_ButtonSetdDefaultClient;
	BOOL				m_bSetDefaultClient;
	int					m_nSelectedClientItem;
	CLIENT_INFO			*m_pDefaultClientInfo;
	char				m_ReportCountText[ MAX_CFG_STRING_LENGTH ];


// Dialog Data
	enum { IDD = IDD_PROP_PAGE_REPORT };

public:
	void					SetReportCount();
	BOOL					LoadClientSelectionList();
	void					ResetPage();
	void					PleaseSelectAStudy();
	void					TurnToggleButtonOn( TomButton *pButton );
	void					LoadReportDataFromCurrentStudy();
	void					LoadStudyDataFromScreens();
	void					ProduceAutomaticReport();

public:
// Overrides
	//{{AFX_VIRTUAL(CComposeReportPage)
	virtual BOOL			OnInitDialog();
	virtual BOOL			OnSetActive();
	virtual BOOL			OnKillActive();
	//}}AFX_VIRTUAL

protected:

	DECLARE_MESSAGE_MAP()

	//{{AFX_MSG(CComposeReportPage)
	afx_msg void			OnEditDateOfRadiographFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnEditDateOfBirthFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnEditDateOfReadingFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnBnClickedTypeOfReadingA( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnBnClickedTypeOfReadingB( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void			OnBnClickedTypeOfReadingF( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void			OnBnClickedTypeOfReadingO( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void			OnEditTypeOfReadingOtherFocus( NMHDR *pNMHDR, LRESULT *pResult );

	afx_msg void			OnClientSelected();
	afx_msg void			OnBnClickedAddClient( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnBnClickedEditClient( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnBnClickedSetDefaultClient( NMHDR *pNMHDR, LRESULT *pResult );

	afx_msg void			OnBnClickedSeePhysicianYes( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void			OnBnClickedSeePhysicianNo( NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void			OnBnClickedShowReportButton( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void			OnBnClickedApproveReportButton( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void			OnReportItemSelected();

	afx_msg void			OnBnClickedPrintCheckedReportsButton( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void			OnBnClickedDeleteCheckedReportsButton( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void			OnBnClickedDeleteAllReportsButton( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH			OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor );
	//}}AFX_MSG

protected:
	BOOL					CheckForReportDataCompletion();
};


