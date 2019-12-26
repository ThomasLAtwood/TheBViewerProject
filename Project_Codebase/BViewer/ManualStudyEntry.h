// ManualStudyEntry.h - Defines the data structures and functions for the manual
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
#pragma once

#include "TomGroup.h"
#include "TomStatic.h"
#include "TomButton.h"
#include "TomEdit.h"
#include "TomEditDate.h"


typedef struct
	{
	SYSTEMTIME		Date;
	BOOL			bDateHasBeenEdited;
	} EDITABLE_DATE;


// CManualStudyEntry dialog
class CManualStudyEntry : public CDialog
{
public:
	CManualStudyEntry( CWnd *pParent = NULL );   // standard constructor
	virtual ~CManualStudyEntry();


// Dialog Data
	enum { IDD = IDD_DIALOG_MANUAL_STUDY_ENTRY };
	CBrush				m_BkgdBrush;

	TomStatic			m_StaticTitle;
	TomStatic			m_StaticPatientFirstName;
	TomEdit				m_EditPatientFirstName;

	TomStatic			m_StaticPatientLastName;
	TomEdit				m_EditPatientLastName;

	TomStatic			m_StaticPatientID;
	TomEdit				m_EditPatientID;

	TomStatic			m_StaticDateOfBirth;
	TomEditDate			m_EditDateOfBirth;

	TomStatic			m_StaticPatientSex;
	TomEdit				m_EditPatientSex;

	TomStatic			m_StaticStudyDate;
	TomEditDate			m_EditStudyDate;

	TomStatic			m_StaticAccessionNumber;
	TomEdit				m_EditAccessionNumber;

	TomStatic			m_StaticOrderingInstitution;
	TomEdit				m_EditOrderingInstitution;

	TomStatic			m_StaticReferringPhysiciansName;
	TomEdit				m_EditReferringPhysiciansName;


	TomButton			m_ButtonCreateManualStudy;
	TomButton			m_ButtonCancel;

	TomGroup			m_GroupEditSequencing;

	char				m_SOPInstanceUID[ DICOM_ATTRIBUTE_UI_STRING_LENGTH ];
	char				m_PatientFirstName[ DICOM_ATTRIBUTE_UI_STRING_LENGTH ];
	char				m_PatientLastName[ DICOM_ATTRIBUTE_UI_STRING_LENGTH ];
	char				m_PatientID[ DICOM_ATTRIBUTE_UI_STRING_LENGTH ];
	EDITABLE_DATE		m_DateOfBirth;
	char				m_DateOfBirthText[ DICOM_ATTRIBUTE_UI_STRING_LENGTH ];
	char				m_PatientSex[ DICOM_ATTRIBUTE_UI_STRING_LENGTH ];
	EDITABLE_DATE		m_StudyDate;
	char				m_StudyDateText[ DICOM_ATTRIBUTE_UI_STRING_LENGTH ];
	char				m_AccessionNumber[ DICOM_ATTRIBUTE_UI_STRING_LENGTH ];
	char				m_OrderingInstitution[ DICOM_ATTRIBUTE_UI_STRING_LENGTH ];
	char				m_ReferringPhysiciansName[ DICOM_ATTRIBUTE_UI_STRING_LENGTH ];


protected:
// Overrides
	//{{AFX_VIRTUAL(CManualStudyEntry)
	virtual BOOL		OnInitDialog();
	virtual void		OnOK();
	//}}AFX_VIRTUAL


	DECLARE_MESSAGE_MAP()

protected:
	//{{AFX_MSG(CManualStudyEntry)
	afx_msg void		OnBnClickedCreateManualStudy( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedCancelManualStudy( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH		OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor );
	afx_msg BOOL		OnEraseBkgnd( CDC *pDC );
	afx_msg void		OnClose();
	//}}AFX_MSG
};


