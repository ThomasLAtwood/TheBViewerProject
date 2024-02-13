// Study.cpp : Implementation file for the CStudy class, which implements a
//  repository for all the analysis and related data associated with a subject study.
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
//	*[4] 01/23/2024 by Tom Atwood
//		Fixed a study file size change the prevented reading studies from previous BViewer versions.
//		Changed the m_SDYFileVersion from 2 to 3.
//	*[3] 07/19/2023 by Tom Atwood
//		Fixed code security issues.
//	*[2] 03/10/2023 by Tom Atwood
//		Fixed code security issues.
//	*[1] 12/20/2022 by Tom Atwood
//		Fixed code security issues.
//
//
#include "StdAfx.h"
#include "BViewer.h"
#include "Study.h"
#include "Configuration.h"
#include "Customization.h"


extern CONFIGURATION				BViewerConfiguration;
extern CCustomization				*pBViewerCustomization;
extern CBViewerApp					ThisBViewerApp;


CStudy::CStudy()
{
	Initialize();
}


CStudy::~CStudy(void)
{
	EraseStudyComponents();						// *[4] Made function name more descriptive.
	InitStudyCommentFields();
}


void CStudy::EraseStudyComponents()				// *[4] Made function name more descriptive.
{
	DIAGNOSTIC_STUDY		*pDiagnosticStudy;
	DIAGNOSTIC_SERIES		*pDiagnosticSeries;
	DIAGNOSTIC_IMAGE		*pDiagnosticImage;
	DIAGNOSTIC_STUDY		*pPrevDiagnosticStudy;
	DIAGNOSTIC_SERIES		*pPrevDiagnosticSeries;
	DIAGNOSTIC_IMAGE		*pPrevDiagnosticImage;

	pDiagnosticStudy = m_pDiagnosticStudyList;
	while ( pDiagnosticStudy != 0 )
		{
		pDiagnosticSeries = pDiagnosticStudy -> pDiagnosticSeriesList;
		while ( pDiagnosticSeries != 0 )
			{
			pPrevDiagnosticSeries = pDiagnosticSeries;
			pDiagnosticImage = pDiagnosticSeries -> pDiagnosticImageList;
			while ( pDiagnosticImage != 0 )
				{
				pPrevDiagnosticImage = pDiagnosticImage;
				pDiagnosticImage = pDiagnosticImage -> pNextDiagnosticImage;
				free( pPrevDiagnosticImage );
				}
			pDiagnosticSeries = pDiagnosticSeries -> pNextDiagnosticSeries;
			free( pPrevDiagnosticSeries );
			}
		pPrevDiagnosticStudy = pDiagnosticStudy;
		pDiagnosticStudy = pDiagnosticStudy -> pNextDiagnosticStudy;
		free( pPrevDiagnosticStudy );
		}
}


void CStudy::InitializeInterpretation()
{
	m_TypeOfReading = BViewerConfiguration.TypeOfReadingDefault;
	
	m_bImageQualityVisited = FALSE;
		m_ImageQuality = IMAGE_QUALITY_UNSPECIFIED;
		m_ImageDefectOtherText = "";
	
	m_bParenchymalAbnormalitiesVisited = FALSE;
		m_AnyParenchymalAbnormalities = BOOL_NOT_SPECIFIED;
		m_ObservedParenchymalAbnormalities = 0L;

	m_bPleuralAbnormalitiesVisited = FALSE;
		m_AnyPleuralAbnormalities = BOOL_NOT_SPECIFIED;
		m_ObservedPleuralPlaqueSites = 0;
		m_ObservedPleuralCalcificationSites = 0;
		m_ObservedPlaqueExtent = 0;
		m_ObservedPlaqueWidth = 0;

		m_ObservedCostophrenicAngleObliteration = 0;
		m_ObservedPleuralThickeningSites = 0;
		m_ObservedThickeningCalcificationSites = 0;
		m_ObservedThickeningExtent = 0;
		m_ObservedThickeningWidth = 0;

	m_bOtherAbnormalitiesVisited = FALSE;
		m_AnyOtherAbnormalities = BOOL_NOT_SPECIFIED;
		m_ObservedOtherSymbols = 0L;
		m_ObservedOtherAbnormalities = 0L;
	m_OtherAbnormalitiesCommentsText = "";

	m_PhysicianNotificationStatus = 0;
	m_bReportViewed = FALSE;
	m_bReportApproved = FALSE;
}


void CStudy::Initialize()
{
	m_nCurrentObjectID = 0;

	m_ReaderAddressed[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
	m_PatientLastName[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
	m_PatientFirstName[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
	m_PatientID[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
	memset( &m_PatientsBirthDate, '\0', sizeof( EDITED_DATE ) );
	m_PatientsSex[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
	m_PatientComments[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.

	// Before doing this, make sure the lists are emptied to prevent memory leaks:
	m_pDiagnosticStudyList = 0;
	m_bStudyHasBeenEdited = FALSE;
	m_pCurrentStudyInfo = 0;
	m_pCurrentSeriesInfo = 0;
	m_pCurrentImageInfo = 0;

	m_Reserved = 0.0;
	m_GammaSetting = 1.0;			// Ranges from 0.2 to 10.0;
	m_WindowCenter = 0.0;
	m_WindowWidth = 0.0;
	m_MaxGrayscaleValue = 256.0;
	m_TimeStudyFirstOpened[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
	m_TimeReportApproved[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.

	InitializeInterpretation();

	memset( &m_DateOfRadiograph, '\0', sizeof( EDITED_DATE ) );
	m_OtherTypeOfReading[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
	m_FacilityIDNumber[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
	memset( &m_DateOfReading, '\0', sizeof( EDITED_DATE ) );

	m_PhysicianNotificationStatus = 0L;
	memset( &m_Reserved2, '\0', sizeof( EDITED_DATE ) );

	memset( &m_ReaderInfo, '\0', sizeof( READER_PERSONAL_INFO ) );
	m_AccessionNumber[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
	m_bStudyWasPreviouslyInterpreted = FALSE;
	m_SDYFileVersion = 3;					// *[4]	Changed the m_SDYFileVersion from 2 to 3 due to READER_PERSONAL_INFO size change.
	m_pEventParameters = 0;
	m_ReportPage1FilePath[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
	m_ReportPage2FilePath[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
	memset( &m_ClientInfo, '\0', sizeof( CLIENT_INFO ) );
	memset( &BViewerConfiguration.m_ClientInfo, '\0', sizeof( CLIENT_INFO ) );
}


typedef struct
	{
	int					ResourceSymbol;
	char				*pEditText;
	} STUDY_COMMENT_TEXT;


STUDY_COMMENT_TEXT		StudyCommentTextArray[] =
					{
						{	IDC_EDIT_IMAGE_QUALITY_OTHER		, 0 },
						{	IDC_EDIT_OTHER_COMMENTS				, 0 },
						{	0									, 0 }
					};


void CStudy::InitStudyCommentFields()
{
	STUDY_COMMENT_TEXT		*pStudyCommentText;
	int						nEdit;
	
	nEdit = 0;
	pStudyCommentText = &StudyCommentTextArray[ nEdit ];
	while ( pStudyCommentText -> ResourceSymbol != 0 )
		{
		if ( pStudyCommentText -> pEditText != 0 )
			{
			free( pStudyCommentText -> pEditText );
			pStudyCommentText -> pEditText = 0;
			}
		nEdit++;
		pStudyCommentText = &StudyCommentTextArray[ nEdit ];
		}

}


typedef struct
	{
	int					ResourceSymbol;
	BOOL				bButtonToggledOn;
	} STUDY_BUTTON_STATE;


STUDY_BUTTON_STATE		StudyButtonStateArray[] =
					{
						{	IDC_BUTTON_A_READER					, FALSE },
						{	IDC_BUTTON_B_READER					, FALSE },
						{	IDC_BUTTON_FACILITY_READING			, FALSE },
						{	IDC_BUTTON_OTHER_READ				, FALSE },

						{	IDC_BUTTON_IMAGE_GRADE_1			, FALSE },
						{	IDC_BUTTON_IMAGE_GRADE_2			, FALSE },
						{	IDC_BUTTON_IMAGE_GRADE_3			, FALSE },
						{	IDC_BUTTON_IMAGE_GRADE_UR			, FALSE },
						{	IDC_BUTTON_IMAGE_OVEREXPOSED		, FALSE },
						{	IDC_BUTTON_IMAGE_UNDEREXPOSED		, FALSE },
						{	IDC_BUTTON_IMAGE_ARTIFACTS			, FALSE },
						{	IDC_BUTTON_IMAGE_IMPROPER_POSITION	, FALSE },
						{	IDC_BUTTON_IMAGE_POOR_CONTRAST		, FALSE },
						{	IDC_BUTTON_IMAGE_POOR_PROCESSING	, FALSE },
						{	IDC_BUTTON_IMAGE_UNDERINFLATION		, FALSE },
						{	IDC_BUTTON_IMAGE_MOTTLE				, FALSE },
						{	IDC_BUTTON_IMAGE_EXCESSIVE_EDGE		, FALSE },
						{	IDC_BUTTON_IMAGE_SCAPULA_OVERLAY	, FALSE },
						{	IDC_BUTTON_IMAGE_OTHER				, FALSE },

						{	IDC_BUTTON_PARENCHYMAL_YES			, FALSE },
						{	IDC_BUTTON_PARENCHYMAL_NO			, FALSE },
						{	IDC_BUTTON_PRIMARY_P				, FALSE },
						{	IDC_BUTTON_PRIMARY_S				, FALSE },
						{	IDC_BUTTON_PRIMARY_Q				, FALSE },
						{	IDC_BUTTON_PRIMARY_T				, FALSE },
						{	IDC_BUTTON_PRIMARY_R				, FALSE },
						{	IDC_BUTTON_PRIMARY_U				, FALSE },

						{	IDC_BUTTON_SECONDARY_P				, FALSE },
						{	IDC_BUTTON_SECONDARY_S				, FALSE },
						{	IDC_BUTTON_SECONDARY_Q				, FALSE },
						{	IDC_BUTTON_SECONDARY_T				, FALSE },
						{	IDC_BUTTON_SECONDARY_R				, FALSE },
						{	IDC_BUTTON_SECONDARY_U				, FALSE },

						{	IDC_BUTTON_ZONE_UPPER_RIGHT			, FALSE },
						{	IDC_BUTTON_ZONE_UPPER_LEFT			, FALSE },
						{	IDC_BUTTON_ZONE_MIDDLE_RIGHT		, FALSE },
						{	IDC_BUTTON_ZONE_MIDDLE_LEFT			, FALSE },
						{	IDC_BUTTON_ZONE_LOWER_RIGHT			, FALSE },
						{	IDC_BUTTON_ZONE_LOWER_LEFT			, FALSE },

						{	IDC_BUTTON_PROFUSION_0MINUS			, FALSE },
						{	IDC_BUTTON_PROFUSION_00				, FALSE },
						{	IDC_BUTTON_PROFUSION_01				, FALSE },
						{	IDC_BUTTON_PROFUSION_10				, FALSE },
						{	IDC_BUTTON_PROFUSION_11				, FALSE },
						{	IDC_BUTTON_PROFUSION_12				, FALSE },
						{	IDC_BUTTON_PROFUSION_21				, FALSE },
						{	IDC_BUTTON_PROFUSION_22				, FALSE },
						{	IDC_BUTTON_PROFUSION_23				, FALSE },
						{	IDC_BUTTON_PROFUSION_32				, FALSE },
						{	IDC_BUTTON_PROFUSION_33				, FALSE },
						{	IDC_BUTTON_PROFUSION_3PLUS			, FALSE },

						{	IDC_BUTTON_LARGE_OPACITY_0			, FALSE },
						{	IDC_BUTTON_LARGE_OPACITY_A			, FALSE },
						{	IDC_BUTTON_LARGE_OPACITY_B			, FALSE },
						{	IDC_BUTTON_LARGE_OPACITY_C			, FALSE },
						
						{	IDC_BUTTON_PLEURAL_YES				, FALSE },
						{	IDC_BUTTON_PLEURAL_NO				, FALSE },

						{	IDC_BUTTON_PLEURAL_SITE_PROFILE_0	, FALSE },
						{	IDC_BUTTON_PLEURAL_SITE_PROFILE_R	, FALSE },
						{	IDC_BUTTON_PLEURAL_SITE_PROFILE_L	, FALSE },
						{	IDC_BUTTON_PLEURAL_SITE_FACE_ON_0	, FALSE },
						{	IDC_BUTTON_PLEURAL_SITE__FACE_ON_R	, FALSE },
						{	IDC_BUTTON_PLEURAL_SITE__FACE_ON_L	, FALSE },
						{	IDC_BUTTON_PLEURAL_SITE_DIAPHRAGM_0	, FALSE },
						{	IDC_BUTTON_PLEURAL_SITE_DIAPHRAGM_R	, FALSE },
						{	IDC_BUTTON_PLEURAL_SITE_DIAPHRAGM_L	, FALSE },
						{	IDC_BUTTON_PLEURAL_SITE_OTHER_0		, FALSE },
						{	IDC_BUTTON_PLEURAL_SITE_OTHER_R		, FALSE },
						{	IDC_BUTTON_PLEURAL_SITE_OTHER_L		, FALSE },

						{	IDC_BUTTON_CALCIFICATION_PROFILE_0	, FALSE },
						{	IDC_BUTTON_CALCIFICATION_PROFILE_R	, FALSE },
						{	IDC_BUTTON_CALCIFICATION_PROFILE_L	, FALSE },
						{	IDC_BUTTON_CALCIFICATION_FACE_ON_0	, FALSE },
						{	IDC_BUTTON_CALCIFICATION__FACE_ON_R	, FALSE },
						{	IDC_BUTTON_CALCIFICATION__FACE_ON_L	, FALSE },
						{	IDC_BUTTON_CALCIFICATION_DIAPHRAGM_0, FALSE },
						{	IDC_BUTTON_CALCIFICATION_DIAPHRAGM_R, FALSE },
						{	IDC_BUTTON_CALCIFICATION_DIAPHRAGM_L, FALSE },
						{	IDC_BUTTON_CALCIFICATION_OTHER_0	, FALSE },
						{	IDC_BUTTON_CALCIFICATION_OTHER_R	, FALSE },
						{	IDC_BUTTON_CALCIFICATION_OTHER_L	, FALSE },

						{	IDC_BUTTON_PLEURAL_EXTENT_NO_RIGHT	, FALSE },
						{	IDC_BUTTON_PLEURAL_EXTENT_RIGHT		, FALSE },
						{	IDC_BUTTON_PLEURAL_EXTENT_RSIZE1	, FALSE },
						{	IDC_BUTTON_PLEURAL_EXTENT_RSIZE2	, FALSE },
						{	IDC_BUTTON_PLEURAL_EXTENT_RSIZE3	, FALSE },
						{	IDC_BUTTON_PLEURAL_EXTENT_NO_LEFT	, FALSE },
						{	IDC_BUTTON_PLEURAL_EXTENT_LEFT		, FALSE },
						{	IDC_BUTTON_PLEURAL_EXTENT_LSIZE1	, FALSE },
						{	IDC_BUTTON_PLEURAL_EXTENT_LSIZE2	, FALSE },
						{	IDC_BUTTON_PLEURAL_EXTENT_LSIZE3	, FALSE },

						{	IDC_BUTTON_PLEURAL_WIDTH_NO_RIGHT	, FALSE },
						{	IDC_BUTTON_PLEURAL_WIDTH_RIGHT		, FALSE },
						{	IDC_BUTTON_PLEURAL_WIDTH_RSIZE1		, FALSE },
						{	IDC_BUTTON_PLEURAL_WIDTH_RSIZE2		, FALSE },
						{	IDC_BUTTON_PLEURAL_WIDTH_RSIZE3		, FALSE },
						{	IDC_BUTTON_PLEURAL_WIDTH_NO_LEFT	, FALSE },
						{	IDC_BUTTON_PLEURAL_WIDTH_LEFT		, FALSE },
						{	IDC_BUTTON_PLEURAL_WIDTH_LSIZE1		, FALSE },
						{	IDC_BUTTON_PLEURAL_WIDTH_LSIZE2		, FALSE },
						{	IDC_BUTTON_PLEURAL_WIDTH_LSIZE3		, FALSE },

						{	IDC_BUTTON_ANGLE_OBLIT_R			, FALSE },
						{	IDC_BUTTON_ANGLE_OBLIT_L			, FALSE },
						{	IDC_BUTTON_ANGLE_OBLIT_0			, FALSE },

						{	IDC_BUTTON_PLEURAL_THICK_SITE_PROFILE_0	, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_SITE_PROFILE_R	, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_SITE_PROFILE_L	, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_SITE_FACE_ON_0	, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_SITE_FACE_ON_R	, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_SITE_FACE_ON_L	, FALSE },

						{	IDC_BUTTON_THICK_CALCIFICATION_PROFILE_0, FALSE },
						{	IDC_BUTTON_THICK_CALCIFICATION_PROFILE_R, FALSE },
						{	IDC_BUTTON_THICK_CALCIFICATION_PROFILE_L, FALSE },
						{	IDC_BUTTON_THICK_CALCIFICATION_FACE_ON_0, FALSE },
						{	IDC_BUTTON_THICK_CALCIFICATION_FACE_ON_R, FALSE },
						{	IDC_BUTTON_THICK_CALCIFICATION_FACE_ON_L, FALSE },

						{	IDC_BUTTON_PLEURAL_THICK_EXTENT_NO_RIGHT, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_EXTENT_RIGHT	, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_EXTENT_RSIZE1	, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_EXTENT_RSIZE2	, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_EXTENT_RSIZE3	, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_EXTENT_NO_LEFT	, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_EXTENT_LEFT	, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_EXTENT_LSIZE1	, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_EXTENT_LSIZE2	, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_EXTENT_LSIZE3	, FALSE },

						{	IDC_BUTTON_PLEURAL_THICK_WIDTH_NO_RIGHT	, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_WIDTH_RIGHT	, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_WIDTH_RSIZE1	, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_WIDTH_RSIZE2	, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_WIDTH_RSIZE3	, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_WIDTH_NO_LEFT	, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_WIDTH_LEFT		, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_WIDTH_LSIZE1	, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_WIDTH_LSIZE2	, FALSE },
						{	IDC_BUTTON_PLEURAL_THICK_WIDTH_LSIZE3	, FALSE },

						{	IDC_BUTTON_OTHER_YES				, FALSE },
						{	IDC_BUTTON_OTHER_NO					, FALSE },

						{	IDC_BUTTON_SYMBOL_AA				, FALSE },
						{	IDC_BUTTON_SYMBOL_AT				, FALSE },
						{	IDC_BUTTON_SYMBOL_AX				, FALSE },
						{	IDC_BUTTON_SYMBOL_BU				, FALSE },
						{	IDC_BUTTON_SYMBOL_CA				, FALSE },
						{	IDC_BUTTON_SYMBOL_CG				, FALSE },
						{	IDC_BUTTON_SYMBOL_CN				, FALSE },
						{	IDC_BUTTON_SYMBOL_CO				, FALSE },
						{	IDC_BUTTON_SYMBOL_CP				, FALSE },
						{	IDC_BUTTON_SYMBOL_CV				, FALSE },
						{	IDC_BUTTON_SYMBOL_DI				, FALSE },
						{	IDC_BUTTON_SYMBOL_EF				, FALSE },
						{	IDC_BUTTON_SYMBOL_EM				, FALSE },
						{	IDC_BUTTON_SYMBOL_ES				, FALSE },
						{	IDC_BUTTON_SYMBOL_FR				, FALSE },
						{	IDC_BUTTON_SYMBOL_HI				, FALSE },
						{	IDC_BUTTON_SYMBOL_HO				, FALSE },
						{	IDC_BUTTON_SYMBOL_ID				, FALSE },
						{	IDC_BUTTON_SYMBOL_IH				, FALSE },
						{	IDC_BUTTON_SYMBOL_KL				, FALSE },
						{	IDC_BUTTON_SYMBOL_ME				, FALSE },
						{	IDC_BUTTON_SYMBOL_PA				, FALSE },
						{	IDC_BUTTON_SYMBOL_PB				, FALSE },
						{	IDC_BUTTON_SYMBOL_PI				, FALSE },
						{	IDC_BUTTON_SYMBOL_PX				, FALSE },
						{	IDC_BUTTON_SYMBOL_RA				, FALSE },
						{	IDC_BUTTON_SYMBOL_RP				, FALSE },
						{	IDC_BUTTON_SYMBOL_TB				, FALSE },
						{	IDC_BUTTON_SYMBOL_OD				, FALSE },

						{	IDC_BUTTON_EVENTRATION				, FALSE },
						{	IDC_BUTTON_HIATAL_HERNIA			, FALSE },
						{	IDC_BUTTON_BRONCHOVASCULAR_MARKINGS	, FALSE },
						{	IDC_BUTTON_HYPERINFLATION			, FALSE },
						{	IDC_BUTTON_BONY_CHEST_CAGE			, FALSE },
						{	IDC_BUTTON_FRACTURE_HEALED			, FALSE },
						{	IDC_BUTTON_FRACTURE_NONHEALED		, FALSE },
						{	IDC_BUTTON_SCOLIOSIS				, FALSE },
						{	IDC_BUTTON_VERTEBRAL_COLUMN			, FALSE },

						{	IDC_BUTTON_AZYGOS_LOBE				, FALSE },
						{	IDC_BUTTON_LUNG_DENSITY				, FALSE },
						{	IDC_BUTTON_INFILTRATE				, FALSE },
						{	IDC_BUTTON_NODULAR_LESION			, FALSE },
						{	IDC_BUTTON_FOREIGN_BODY				, FALSE },
						{	IDC_BUTTON_POSTSURGICAL				, FALSE },
						{	IDC_BUTTON_CYST						, FALSE },
						{	IDC_BUTTON_AORTA_ANOMALY			, FALSE },
						{	IDC_BUTTON_VASCULAR_ABNORMALITY		, FALSE },

						{	IDC_BUTTON_SEE_PHYSICIAN_YES		, FALSE },
						{	IDC_BUTTON_SEE_PHYSICIAN_NO			, FALSE },

						{	0									, FALSE }
					};


#define MAX_EDIT_FIELD_SIZE		34

typedef struct
	{
	int					ResourceSymbol;
	char				EditText[ MAX_EDIT_FIELD_SIZE ];
	} STUDY_EDIT_TEXT;


STUDY_EDIT_TEXT		StudyEditTextArray[] =
					{
						{	IDC_EDIT_DATE_OF_RADIOGRAPH			, {0} },
						{	IDC_EDIT_READER_ID					, {0} },
						{	IDC_EDIT_READER_INITIALS			, {0} },
						{	IDC_EDIT_DATE_OF_READING			, {0} },
						{	IDC_EDIT_READER_LAST_NAME			, {0} },
						{	IDC_EDIT_READER_SIGNATURE_NAME		, {0} },
						{	IDC_EDIT_READER_STREET_ADDRESS		, {0} },
						{	IDC_EDIT_READER_CITY				, {0} },
						{	IDC_EDIT_READER_STATE				, {0} },
						{	IDC_EDIT_READER_ZIPCODE				, {0} },
						{	IDC_EDIT_REPORT_PATIENT_NAME		, {0} },
						{	IDC_EDIT_REPORT_DOB					, {0} },
						{	IDC_EDIT_REPORT_PATIENT_ID			, {0} },
						{	IDC_EDIT_ORDERING_PHYSICIAN_NAME	, {0} },
						{	IDC_EDIT_ORDERING_FACILITY			, {0} },
						{	IDC_EDIT_CLASSIFICATION_PURPOSE		, {0} },
						{	IDC_EDIT_TYPE_OF_READING_OTHER		, {0} },
						{	0									, {0} }
					};


// Load the button (and other control) states from the stored data for this study.
void CStudy::UnpackData()
{
	STUDY_BUTTON_STATE		*pStudyButtonState;
	STUDY_EDIT_TEXT			*pStudyEditText;
	STUDY_COMMENT_TEXT		*pStudyCommentText;
	int						nButton;
	int						nEdit;
	SYSTEMTIME				*pDate;
	
	nButton = 0;
	pStudyButtonState = &StudyButtonStateArray[ nButton ];
	while ( pStudyButtonState -> ResourceSymbol != 0 )
		{
		switch ( pStudyButtonState -> ResourceSymbol )
			{
			case IDC_BUTTON_A_READER:
				pStudyButtonState -> bButtonToggledOn = ( ( m_TypeOfReading & READING_TYPE_A_READER ) != 0 );
				break;
			case IDC_BUTTON_B_READER:
				pStudyButtonState -> bButtonToggledOn = ( ( m_TypeOfReading & READING_TYPE_B_READER ) != 0 );
				break;
			case IDC_BUTTON_FACILITY_READING:
				pStudyButtonState -> bButtonToggledOn = ( ( m_TypeOfReading & READING_TYPE_FACILITY ) != 0 );
				break;
			case IDC_BUTTON_OTHER_READ:
				pStudyButtonState -> bButtonToggledOn = ( ( m_TypeOfReading & READING_TYPE_OTHER ) != 0 );
				break;

			case IDC_BUTTON_IMAGE_GRADE_1:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ImageQuality & IMAGE_GRADE_1 ) != 0 );
				break;
			case IDC_BUTTON_IMAGE_GRADE_2:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ImageQuality & IMAGE_GRADE_2 ) != 0 );
				break;
			case IDC_BUTTON_IMAGE_GRADE_3:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ImageQuality & IMAGE_GRADE_3 ) != 0 );
				break;
			case IDC_BUTTON_IMAGE_GRADE_UR:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ImageQuality & IMAGE_GRADE_UR ) != 0 );
				break;
			case IDC_BUTTON_IMAGE_OVEREXPOSED:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ImageQuality & IMAGE_DEFECT_OVEREXPOSED ) != 0 );
				break;
			case IDC_BUTTON_IMAGE_UNDEREXPOSED:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ImageQuality & IMAGE_DEFECT_UNDEREXPOSED ) != 0 );
				break;
			case IDC_BUTTON_IMAGE_ARTIFACTS:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ImageQuality & IMAGE_DEFECT_ARTIFACTS ) != 0 );
				break;
			case IDC_BUTTON_IMAGE_IMPROPER_POSITION:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ImageQuality & IMAGE_DEFECT_POSITION ) != 0 );
				break;
			case IDC_BUTTON_IMAGE_POOR_CONTRAST:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ImageQuality & IMAGE_DEFECT_CONTRAST ) != 0 );
				break;
			case IDC_BUTTON_IMAGE_POOR_PROCESSING:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ImageQuality & IMAGE_DEFECT_PROCESSING ) != 0 );
				break;
			case IDC_BUTTON_IMAGE_UNDERINFLATION:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ImageQuality & IMAGE_DEFECT_UNDERINFLATION ) != 0 );
				break;
			case IDC_BUTTON_IMAGE_MOTTLE:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ImageQuality & IMAGE_DEFECT_MOTTLE ) != 0 );
				break;
			case IDC_BUTTON_IMAGE_EXCESSIVE_EDGE:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ImageQuality & IMAGE_DEFECT_EXCESSIVE_EDGE ) != 0 );
				break;
			case IDC_BUTTON_IMAGE_SCAPULA_OVERLAY:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ImageQuality & IMAGE_DEFECT_SCAPULA_OVERLAY ) != 0 );
				break;
			case IDC_BUTTON_IMAGE_OTHER:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ImageQuality & IMAGE_DEFECT_OTHER ) != 0 );
				break;

			case IDC_BUTTON_PARENCHYMAL_YES:
				pStudyButtonState -> bButtonToggledOn = ( m_AnyParenchymalAbnormalities == BOOL_YES );
				break;
			case IDC_BUTTON_PARENCHYMAL_NO:
				pStudyButtonState -> bButtonToggledOn = ( m_AnyParenchymalAbnormalities == BOOL_NO );
				break;
			case IDC_BUTTON_PRIMARY_P:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & SHAPE_SIZE_PRIMARY_MASK ) == SHAPE_SIZE_PRIMARY_P );
				break;
			case IDC_BUTTON_PRIMARY_S:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & SHAPE_SIZE_PRIMARY_MASK ) == SHAPE_SIZE_PRIMARY_S );
				break;
			case IDC_BUTTON_PRIMARY_Q:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & SHAPE_SIZE_PRIMARY_MASK ) == SHAPE_SIZE_PRIMARY_Q );
				break;
			case IDC_BUTTON_PRIMARY_T:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & SHAPE_SIZE_PRIMARY_MASK ) == SHAPE_SIZE_PRIMARY_T );
				break;
			case IDC_BUTTON_PRIMARY_R:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & SHAPE_SIZE_PRIMARY_MASK ) == SHAPE_SIZE_PRIMARY_R );
				break;
			case IDC_BUTTON_PRIMARY_U:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & SHAPE_SIZE_PRIMARY_MASK ) == SHAPE_SIZE_PRIMARY_U );
				break;

			case IDC_BUTTON_SECONDARY_P:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & SHAPE_SIZE_SECONDARY_MASK ) == SHAPE_SIZE_SECONDARY_P );
				break;
			case IDC_BUTTON_SECONDARY_S:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & SHAPE_SIZE_SECONDARY_MASK ) == SHAPE_SIZE_SECONDARY_S );
				break;
			case IDC_BUTTON_SECONDARY_Q:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & SHAPE_SIZE_SECONDARY_MASK ) == SHAPE_SIZE_SECONDARY_Q );
				break;
			case IDC_BUTTON_SECONDARY_T:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & SHAPE_SIZE_SECONDARY_MASK ) == SHAPE_SIZE_SECONDARY_T );
				break;
			case IDC_BUTTON_SECONDARY_R:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & SHAPE_SIZE_SECONDARY_MASK ) == SHAPE_SIZE_SECONDARY_R );
				break;
			case IDC_BUTTON_SECONDARY_U:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & SHAPE_SIZE_SECONDARY_MASK ) == SHAPE_SIZE_SECONDARY_U );
				break;

			case IDC_BUTTON_ZONE_UPPER_RIGHT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & OPACITY_ZONE_UPPER_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_ZONE_UPPER_LEFT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & OPACITY_ZONE_UPPER_LEFT ) != 0 );
				break;
			case IDC_BUTTON_ZONE_MIDDLE_RIGHT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & OPACITY_ZONE_MIDDLE_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_ZONE_MIDDLE_LEFT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & OPACITY_ZONE_MIDDLE_LEFT ) != 0 );
				break;
			case IDC_BUTTON_ZONE_LOWER_RIGHT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & OPACITY_ZONE_LOWER_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_ZONE_LOWER_LEFT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & OPACITY_ZONE_LOWER_LEFT ) != 0 );
				break;

			case IDC_BUTTON_PROFUSION_0MINUS:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & PROFUSION_MASK ) == PROFUSION_0MINUS );
				break;
			case IDC_BUTTON_PROFUSION_00:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & PROFUSION_MASK ) == PROFUSION_00 );
				break;
			case IDC_BUTTON_PROFUSION_01:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & PROFUSION_MASK ) == PROFUSION_01 );
				break;
			case IDC_BUTTON_PROFUSION_10:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & PROFUSION_MASK ) == PROFUSION_10 );
				break;
			case IDC_BUTTON_PROFUSION_11:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & PROFUSION_MASK ) == PROFUSION_11 );
				break;
			case IDC_BUTTON_PROFUSION_12:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & PROFUSION_MASK ) == PROFUSION_12 );
				break;
			case IDC_BUTTON_PROFUSION_21:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & PROFUSION_MASK ) == PROFUSION_21 );
				break;
			case IDC_BUTTON_PROFUSION_22:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & PROFUSION_MASK ) == PROFUSION_22 );
				break;
			case IDC_BUTTON_PROFUSION_23:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & PROFUSION_MASK ) == PROFUSION_23 );
				break;
			case IDC_BUTTON_PROFUSION_32:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & PROFUSION_MASK ) == PROFUSION_32 );
				break;
			case IDC_BUTTON_PROFUSION_33:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & PROFUSION_MASK ) == PROFUSION_33 );
				break;
			case IDC_BUTTON_PROFUSION_3PLUS:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & PROFUSION_MASK ) == PROFUSION_3PLUS );
				break;

			case IDC_BUTTON_LARGE_OPACITY_0:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & LARGE_OPACITY_SIZE_MASK ) == LARGE_OPACITY_SIZE_0 );
				break;
			case IDC_BUTTON_LARGE_OPACITY_A:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & LARGE_OPACITY_SIZE_MASK ) == LARGE_OPACITY_SIZE_A );
				break;
			case IDC_BUTTON_LARGE_OPACITY_B:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & LARGE_OPACITY_SIZE_MASK ) == LARGE_OPACITY_SIZE_B );
				break;
			case IDC_BUTTON_LARGE_OPACITY_C:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedParenchymalAbnormalities & LARGE_OPACITY_SIZE_MASK ) == LARGE_OPACITY_SIZE_C );
				break;

			case IDC_BUTTON_PLEURAL_YES:
				pStudyButtonState -> bButtonToggledOn = ( m_AnyPleuralAbnormalities == BOOL_YES );
				break;
			case IDC_BUTTON_PLEURAL_NO:
				pStudyButtonState -> bButtonToggledOn = ( m_AnyPleuralAbnormalities == BOOL_NO );
				break;

			case IDC_BUTTON_PLEURAL_SITE_PROFILE_0:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralPlaqueSites & PLAQUES_CHEST_WALL_PROFILE_NONE ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_SITE_PROFILE_R:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralPlaqueSites & PLAQUES_CHEST_WALL_PROFILE_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_SITE_PROFILE_L:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralPlaqueSites & PLAQUES_CHEST_WALL_PROFILE_LEFT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_SITE_FACE_ON_0:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralPlaqueSites & PLAQUES_CHEST_WALL_FACE_ON_NONE ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_SITE__FACE_ON_R:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralPlaqueSites & PLAQUES_CHEST_WALL_FACE_ON_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_SITE__FACE_ON_L:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralPlaqueSites & PLAQUES_CHEST_WALL_FACE_ON_LEFT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_SITE_DIAPHRAGM_0:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralPlaqueSites & PLAQUES_DIAPHRAGM_NONE ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_SITE_DIAPHRAGM_R:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralPlaqueSites & PLAQUES_DIAPHRAGM_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_SITE_DIAPHRAGM_L:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralPlaqueSites & PLAQUES_DIAPHRAGM_LEFT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_SITE_OTHER_0:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralPlaqueSites & PLAQUES_OTHER_SITES_NONE ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_SITE_OTHER_R:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralPlaqueSites & PLAQUES_OTHER_SITES_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_SITE_OTHER_L:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralPlaqueSites & PLAQUES_OTHER_SITES_LEFT ) != 0 );
				break;

			case IDC_BUTTON_CALCIFICATION_PROFILE_0:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralCalcificationSites & CALCIFICATION_CHEST_WALL_PROFILE_NONE ) != 0 );
				break;
			case IDC_BUTTON_CALCIFICATION_PROFILE_R:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralCalcificationSites & CALCIFICATION_CHEST_WALL_PROFILE_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_CALCIFICATION_PROFILE_L:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralCalcificationSites & CALCIFICATION_CHEST_WALL_PROFILE_LEFT ) != 0 );
				break;
			case IDC_BUTTON_CALCIFICATION_FACE_ON_0:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralCalcificationSites & CALCIFICATION_CHEST_WALL_FACE_ON_NONE ) != 0 );
				break;
			case IDC_BUTTON_CALCIFICATION__FACE_ON_R:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralCalcificationSites & CALCIFICATION_CHEST_WALL_FACE_ON_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_CALCIFICATION__FACE_ON_L:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralCalcificationSites & CALCIFICATION_CHEST_WALL_FACE_ON_LEFT ) != 0 );
				break;
			case IDC_BUTTON_CALCIFICATION_DIAPHRAGM_0:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralCalcificationSites & CALCIFICATION_DIAPHRAGM_NONE ) != 0 );
				break;
			case IDC_BUTTON_CALCIFICATION_DIAPHRAGM_R:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralCalcificationSites & CALCIFICATION_DIAPHRAGM_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_CALCIFICATION_DIAPHRAGM_L:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralCalcificationSites & CALCIFICATION_DIAPHRAGM_LEFT ) != 0 );
				break;
			case IDC_BUTTON_CALCIFICATION_OTHER_0:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralCalcificationSites & CALCIFICATION_OTHER_SITES_NONE ) != 0 );
				break;
			case IDC_BUTTON_CALCIFICATION_OTHER_R:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralCalcificationSites & CALCIFICATION_OTHER_SITES_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_CALCIFICATION_OTHER_L:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralCalcificationSites & CALCIFICATION_OTHER_SITES_LEFT ) != 0 );
				break;

			case IDC_BUTTON_PLEURAL_EXTENT_NO_RIGHT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPlaqueExtent & PLAQUE_EXTENT_NONE_ON_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_EXTENT_RIGHT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPlaqueExtent & PLAQUE_EXTENT_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_EXTENT_RSIZE1:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPlaqueExtent & PLAQUE_EXTENT_RIGHT_SIZE1 ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_EXTENT_RSIZE2:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPlaqueExtent & PLAQUE_EXTENT_RIGHT_SIZE2 ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_EXTENT_RSIZE3:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPlaqueExtent & PLAQUE_EXTENT_RIGHT_SIZE3 ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_EXTENT_NO_LEFT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPlaqueExtent & PLAQUE_EXTENT_NONE_ON_LEFT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_EXTENT_LEFT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPlaqueExtent & PLAQUE_EXTENT_LEFT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_EXTENT_LSIZE1:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPlaqueExtent & PLAQUE_EXTENT_LEFT_SIZE1 ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_EXTENT_LSIZE2:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPlaqueExtent & PLAQUE_EXTENT_LEFT_SIZE2 ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_EXTENT_LSIZE3:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPlaqueExtent & PLAQUE_EXTENT_LEFT_SIZE3 ) != 0 );
				break;

			case IDC_BUTTON_PLEURAL_WIDTH_NO_RIGHT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPlaqueWidth & PLAQUE_WIDTH_NONE_ON_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_WIDTH_RIGHT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPlaqueWidth & PLAQUE_WIDTH_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_WIDTH_RSIZE1:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPlaqueWidth & PLAQUE_WIDTH_RIGHT_SIZE1 ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_WIDTH_RSIZE2:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPlaqueWidth & PLAQUE_WIDTH_RIGHT_SIZE2 ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_WIDTH_RSIZE3:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPlaqueWidth & PLAQUE_WIDTH_RIGHT_SIZE3 ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_WIDTH_NO_LEFT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPlaqueWidth & PLAQUE_WIDTH_NONE_ON_LEFT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_WIDTH_LEFT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPlaqueWidth & PLAQUE_WIDTH_LEFT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_WIDTH_LSIZE1:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPlaqueWidth & PLAQUE_WIDTH_LEFT_SIZE1 ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_WIDTH_LSIZE2:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPlaqueWidth & PLAQUE_WIDTH_LEFT_SIZE2 ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_WIDTH_LSIZE3:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPlaqueWidth & PLAQUE_WIDTH_LEFT_SIZE3 ) != 0 );
				break;

			case IDC_BUTTON_ANGLE_OBLIT_R:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedCostophrenicAngleObliteration & COSTOPHRENIC_ANGLE_OBLITERATION_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_ANGLE_OBLIT_L:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedCostophrenicAngleObliteration & COSTOPHRENIC_ANGLE_OBLITERATION_LEFT ) != 0 );
				break;
			case IDC_BUTTON_ANGLE_OBLIT_0:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedCostophrenicAngleObliteration & COSTOPHRENIC_ANGLE_OBLITERATION_NONE ) != 0 );
				break;

			case IDC_BUTTON_PLEURAL_THICK_SITE_PROFILE_0:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralThickeningSites & THICKENING_CHEST_WALL_PROFILE_NONE ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_SITE_PROFILE_R:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralThickeningSites & THICKENING_CHEST_WALL_PROFILE_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_SITE_PROFILE_L:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralThickeningSites & THICKENING_CHEST_WALL_PROFILE_LEFT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_SITE_FACE_ON_0:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralThickeningSites & THICKENING_CHEST_WALL_FACE_ON_NONE ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_SITE_FACE_ON_R:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralThickeningSites & THICKENING_CHEST_WALL_FACE_ON_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_SITE_FACE_ON_L:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedPleuralThickeningSites & THICKENING_CHEST_WALL_FACE_ON_LEFT ) != 0 );
				break;

			case IDC_BUTTON_THICK_CALCIFICATION_PROFILE_0:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningCalcificationSites & THICK_CALCIFICATION_CHEST_WALL_PROFILE_NONE ) != 0 );
				break;
			case IDC_BUTTON_THICK_CALCIFICATION_PROFILE_R:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningCalcificationSites & THICK_CALCIFICATION_CHEST_WALL_PROFILE_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_THICK_CALCIFICATION_PROFILE_L:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningCalcificationSites & THICK_CALCIFICATION_CHEST_WALL_PROFILE_LEFT ) != 0 );
				break;
			case IDC_BUTTON_THICK_CALCIFICATION_FACE_ON_0:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningCalcificationSites & THICK_CALCIFICATION_CHEST_WALL_FACE_ON_NONE ) != 0 );
				break;
			case IDC_BUTTON_THICK_CALCIFICATION_FACE_ON_R:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningCalcificationSites & THICK_CALCIFICATION_CHEST_WALL_FACE_ON_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_THICK_CALCIFICATION_FACE_ON_L:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningCalcificationSites & THICK_CALCIFICATION_CHEST_WALL_FACE_ON_LEFT ) != 0 );
				break;

			case IDC_BUTTON_PLEURAL_THICK_EXTENT_NO_RIGHT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningExtent & THICKENING_EXTENT_NONE_ON_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_EXTENT_RIGHT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningExtent & THICKENING_EXTENT_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_EXTENT_RSIZE1:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningExtent & THICKENING_EXTENT_RIGHT_SIZE1 ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_EXTENT_RSIZE2:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningExtent & THICKENING_EXTENT_RIGHT_SIZE2 ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_EXTENT_RSIZE3:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningExtent & THICKENING_EXTENT_RIGHT_SIZE3 ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_EXTENT_NO_LEFT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningExtent & THICKENING_EXTENT_NONE_ON_LEFT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_EXTENT_LEFT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningExtent & THICKENING_EXTENT_LEFT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_EXTENT_LSIZE1:
				pStudyButtonState -> bButtonToggledOn = ( (  m_ObservedThickeningExtent & THICKENING_EXTENT_LEFT_SIZE1 ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_EXTENT_LSIZE2:
				pStudyButtonState -> bButtonToggledOn = ( (  m_ObservedThickeningExtent & THICKENING_EXTENT_LEFT_SIZE2 ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_EXTENT_LSIZE3:
				pStudyButtonState -> bButtonToggledOn = ( (  m_ObservedThickeningExtent & THICKENING_EXTENT_LEFT_SIZE3 ) != 0 );
				break;

			case IDC_BUTTON_PLEURAL_THICK_WIDTH_NO_RIGHT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningWidth & THICKENING_WIDTH_NONE_ON_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_WIDTH_RIGHT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningWidth & THICKENING_WIDTH_RIGHT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_WIDTH_RSIZE1:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningWidth & THICKENING_WIDTH_RIGHT_SIZE1 ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_WIDTH_RSIZE2:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningWidth & THICKENING_WIDTH_RIGHT_SIZE2 ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_WIDTH_RSIZE3:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningWidth & THICKENING_WIDTH_RIGHT_SIZE3 ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_WIDTH_NO_LEFT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningWidth & THICKENING_WIDTH_NONE_ON_LEFT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_WIDTH_LEFT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningWidth & THICKENING_WIDTH_LEFT ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_WIDTH_LSIZE1:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningWidth & THICKENING_WIDTH_LEFT_SIZE1 ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_WIDTH_LSIZE2:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningWidth & THICKENING_WIDTH_LEFT_SIZE2 ) != 0 );
				break;
			case IDC_BUTTON_PLEURAL_THICK_WIDTH_LSIZE3:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedThickeningWidth & THICKENING_WIDTH_LEFT_SIZE3 ) != 0 );
				break;

			case IDC_BUTTON_OTHER_YES:
				pStudyButtonState -> bButtonToggledOn = ( ( m_AnyOtherAbnormalities == BOOL_YES ) != 0 );
				break;
			case IDC_BUTTON_OTHER_NO:
				pStudyButtonState -> bButtonToggledOn = ( ( m_AnyOtherAbnormalities == BOOL_NO ) != 0 );
				break;

			case IDC_BUTTON_SYMBOL_AA:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_AA ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_AT:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_AT ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_AX:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_AX ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_BU:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_BU ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_CA:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_CA ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_CG:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_CG ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_CN:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_CN ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_CO:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_CO ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_CP:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_CP ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_CV:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_CV ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_DI:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_DI ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_EF:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_EF ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_EM:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_EM ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_ES:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_ES ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_FR:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_FR ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_HI:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_HI ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_HO:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_HO ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_ID:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_ID ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_IH:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_IH ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_KL:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_KL ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_ME:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_ME ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_PA:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_PA ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_PB:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_PB ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_PI:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_PI ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_PX:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_PX ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_RA:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_RA ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_RP:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_RP ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_TB:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_TB ) != 0 );
				break;
			case IDC_BUTTON_SYMBOL_OD:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherSymbols & OBSERVED_SYMBOL_OD ) != 0 );
				break;

			case IDC_BUTTON_EVENTRATION:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherAbnormalities & OBSERVED_EVENTRATION  ) != 0 );
				break;
			case IDC_BUTTON_HIATAL_HERNIA:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherAbnormalities & OBSERVED_HIATAL_HERNIA ) != 0 );
				break;
			case IDC_BUTTON_BRONCHOVASCULAR_MARKINGS:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherAbnormalities & OBSERVED_BRONCHOVASCULAR_MARKINGS ) != 0 );
				break;
			case IDC_BUTTON_HYPERINFLATION:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherAbnormalities & OBSERVED_HYPERINFLATION ) != 0 );
				break;
			case IDC_BUTTON_BONY_CHEST_CAGE:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherAbnormalities & OBSERVED_BONY_CHEST_CAGE ) != 0 );
				break;
			case IDC_BUTTON_FRACTURE_HEALED:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherAbnormalities & OBSERVED_FRACTURE_HEALED ) != 0 );
				break;
			case IDC_BUTTON_FRACTURE_NONHEALED:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherAbnormalities & OBSERVED_FRACTURE_NONHEALED ) != 0 );
				break;
			case IDC_BUTTON_SCOLIOSIS:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherAbnormalities & OBSERVED_SCOLIOSIS ) != 0 );
				break;
			case IDC_BUTTON_VERTEBRAL_COLUMN:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherAbnormalities & OBSERVED_VERTEBRAL_COLUMN ) != 0 );
				break;

			case IDC_BUTTON_AZYGOS_LOBE:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherAbnormalities & OBSERVED_AZYGOS_LOBE ) != 0 );
				break;
			case IDC_BUTTON_LUNG_DENSITY:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherAbnormalities & OBSERVED_LUNG_DENSITY ) != 0 );
				break;
			case IDC_BUTTON_INFILTRATE:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherAbnormalities & OBSERVED_INFILTRATE ) != 0 );
				break;
			case IDC_BUTTON_NODULAR_LESION:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherAbnormalities & OBSERVED_NODULAR_LESION ) != 0 );
				break;
			case IDC_BUTTON_FOREIGN_BODY:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherAbnormalities & OBSERVED_FOREIGN_BODY ) != 0 );
				break;
			case IDC_BUTTON_POSTSURGICAL:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherAbnormalities & OBSERVED_POSTSURGICAL ) != 0 );
				break;
			case IDC_BUTTON_CYST:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherAbnormalities & OBSERVED_CYST ) != 0 );
				break;
			case IDC_BUTTON_AORTA_ANOMALY:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherAbnormalities & OBSERVED_AORTA_ANOMALY ) != 0 );
				break;
			case IDC_BUTTON_VASCULAR_ABNORMALITY:
				pStudyButtonState -> bButtonToggledOn = ( ( m_ObservedOtherAbnormalities & OBSERVED_VASCULAR_ABNORMALITY ) != 0 );
				break;

			case IDC_BUTTON_SEE_PHYSICIAN_YES:
				pStudyButtonState -> bButtonToggledOn = ( ( m_PhysicianNotificationStatus & OBSERVED_SEE_PHYSICIAN_YES ) != 0 );
				break;
			case IDC_BUTTON_SEE_PHYSICIAN_NO:
				pStudyButtonState -> bButtonToggledOn = ( ( m_PhysicianNotificationStatus & OBSERVED_SEE_PHYSICIAN_NO ) != 0 );
				break;
			}

		nButton++;
		pStudyButtonState = &StudyButtonStateArray[ nButton ];
		}

	nEdit = 0;
	pStudyEditText = &StudyEditTextArray[ nEdit ];
	while ( pStudyEditText -> ResourceSymbol != 0 )
		{
		switch ( pStudyEditText -> ResourceSymbol )
			{
			case IDC_EDIT_DATE_OF_RADIOGRAPH:
				pDate = &m_DateOfRadiograph.Date;
				if ( m_DateOfRadiograph.bDateHasBeenEdited )
					_snprintf_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, _TRUNCATE,
									"%2u/%2u/%4u", pDate -> wMonth, pDate -> wDay, pDate -> wYear );									// *[2] Replaced sprintf() with _snprintf_s.
				else
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, "  /  /    ", _TRUNCATE );								// *[1] Replaced strcpy with strncpy_s.
				break;
			case IDC_EDIT_READER_ID:
				if ( m_bStudyWasPreviouslyInterpreted )
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, m_ReaderInfo.ID, _TRUNCATE );							// *[1] Replaced strcpy with strncpy_s.
				else if ( pBViewerCustomization != 0 && strlen( pBViewerCustomization -> m_ReaderInfo.ID ) > 0 )
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, pBViewerCustomization -> m_ReaderInfo.ID, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
				else
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, "         ", _TRUNCATE );								// *[1] Replaced strcpy with strncpy_s.
				break;
			case IDC_EDIT_READER_INITIALS:
				if ( m_bStudyWasPreviouslyInterpreted )
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, m_ReaderInfo.Initials, _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.
				else if ( pBViewerCustomization != 0 && strlen( pBViewerCustomization -> m_ReaderInfo.Initials ) > 0 )
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, pBViewerCustomization -> m_ReaderInfo.Initials, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
				else
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, "   ", _TRUNCATE );										// *[1] Replaced strcpy with strncpy_s.
				break;
			case IDC_EDIT_DATE_OF_READING:

				pDate = &m_DateOfReading.Date;
				_snprintf_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, _TRUNCATE,
								"%2u/%2u/%4u", pDate -> wMonth, pDate -> wDay, pDate -> wYear );										// *[2] Replaced sprintf() with _snprintf_s.
				break;
			case IDC_EDIT_READER_LAST_NAME:
				if ( m_bStudyWasPreviouslyInterpreted )
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, m_ReaderInfo.LastName, _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.
				else if ( pBViewerCustomization != 0 && strlen( pBViewerCustomization -> m_ReaderInfo.LastName ) > 0 )
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, pBViewerCustomization -> m_ReaderInfo.LastName, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
				else
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, "                                ", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
				break;
			case IDC_EDIT_READER_SIGNATURE_NAME:
				if ( m_bStudyWasPreviouslyInterpreted )
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, m_ReaderInfo.ReportSignatureName, _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
				else if ( pBViewerCustomization != 0 && strlen( pBViewerCustomization -> m_ReaderInfo.ReportSignatureName ) > 0 )
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, pBViewerCustomization -> m_ReaderInfo.ReportSignatureName, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
				else
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, "                                ", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
				break;
			case IDC_EDIT_READER_STREET_ADDRESS:
				if ( m_bStudyWasPreviouslyInterpreted )
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, m_ReaderInfo.StreetAddress, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
				else if ( pBViewerCustomization != 0 && strlen( pBViewerCustomization -> m_ReaderInfo.StreetAddress ) > 0 )
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, pBViewerCustomization -> m_ReaderInfo.StreetAddress, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
				else
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, "                                ", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
				break;
			case IDC_EDIT_READER_CITY:
				if ( m_bStudyWasPreviouslyInterpreted )
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, m_ReaderInfo.City, _TRUNCATE );							// *[1] Replaced strcpy with strncpy_s.
				else if ( pBViewerCustomization != 0 && strlen( pBViewerCustomization -> m_ReaderInfo.City ) > 0 )
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, pBViewerCustomization -> m_ReaderInfo.City, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
				else
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, "                      ", _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
				break;
			case IDC_EDIT_READER_STATE:
				if ( m_bStudyWasPreviouslyInterpreted )
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, m_ReaderInfo.State, _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.
				else if ( pBViewerCustomization != 0 && strlen( pBViewerCustomization -> m_ReaderInfo.State ) > 0 )
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, pBViewerCustomization -> m_ReaderInfo.State, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
				else
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, "  ", _TRUNCATE );										// *[1] Replaced strcpy with strncpy_s.
				break;
			case IDC_EDIT_READER_ZIPCODE:
				if ( m_bStudyWasPreviouslyInterpreted )
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, m_ReaderInfo.ZipCode, _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.
				else if ( pBViewerCustomization != 0 && strlen( pBViewerCustomization -> m_ReaderInfo.ZipCode ) > 0 )
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, pBViewerCustomization -> m_ReaderInfo.ZipCode, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
				else
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, "     ", _TRUNCATE );									// *[1] Replaced strcpy with strncpy_s.
				break;
			case IDC_EDIT_REPORT_PATIENT_NAME:
				if ( strlen( m_PatientLastName ) > 0 )
					{
					pStudyEditText -> EditText[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
					strncat_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, m_PatientLastName, _TRUNCATE );							// *[1] Replaced strncat with strncat_s.
					if ( strlen( m_PatientFirstName ) > 0 )
						{
						strncat_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, ", ", _TRUNCATE );									// *[1] Replaced strncat with strncat_s.
						strncat_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, m_PatientFirstName, _TRUNCATE );					// *[1] Replaced strncat with strncat_s.
						}
					}
				else
					pStudyEditText -> EditText[ 0 ] = '\0';																				// *[1] Eliminated call to strcpy.
				break;
			case IDC_EDIT_REPORT_DOB:
				pDate = &m_PatientsBirthDate.Date;
				if ( m_PatientsBirthDate.bDateHasBeenEdited )
					_snprintf_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, _TRUNCATE,
									"%2u/%2u/%4u", pDate -> wMonth, pDate -> wDay, pDate -> wYear );									// *[2] Replaced sprintf() with _snprintf_s.
				else
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, "01/01/1901", _TRUNCATE );								// *[1] Replaced strcpy with strncpy_s.
				break;
			case IDC_EDIT_REPORT_PATIENT_ID:
				if ( strlen( m_PatientID ) > 0 )
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, m_PatientID, _TRUNCATE );								// *[1] Replaced strcpy with strncpy_s.
				else
					pStudyEditText -> EditText[ 0 ] = '\0';																				// *[1] Eliminated call to strcpy.
				break;
			case IDC_EDIT_ORDERING_PHYSICIAN_NAME:
				if ( m_pDiagnosticStudyList != 0 && strlen( m_pDiagnosticStudyList -> ReferringPhysiciansName ) > 0 )
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, m_pDiagnosticStudyList -> ReferringPhysiciansName, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
				else if ( m_pDiagnosticStudyList != 0 && strlen( m_pDiagnosticStudyList -> ResponsibleOrganization ) > 0 )
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, m_pDiagnosticStudyList -> ResponsibleOrganization, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
				else
					pStudyEditText -> EditText[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
				break;
			case IDC_EDIT_ORDERING_FACILITY:
				if ( m_pDiagnosticStudyList != 0 && strlen( m_pDiagnosticStudyList -> InstitutionName ) > 0 )
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, m_pDiagnosticStudyList -> InstitutionName, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
				else
					pStudyEditText -> EditText[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
				break;
			case IDC_EDIT_CLASSIFICATION_PURPOSE:
				if ( strlen( m_PatientComments ) > 0 )
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, m_PatientComments, _TRUNCATE );							// *[1] Replaced strcpy with strncpy_s.
				else
					pStudyEditText -> EditText[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
				break;
			case IDC_EDIT_TYPE_OF_READING_OTHER:
				if ( strlen( m_OtherTypeOfReading ) > 0 )
					strncpy_s( pStudyEditText -> EditText, MAX_EDIT_FIELD_SIZE, m_OtherTypeOfReading, _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.
				else
					pStudyEditText -> EditText[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
				break;
			}
		nEdit++;
		pStudyEditText = &StudyEditTextArray[ nEdit ];
		}

	nEdit = 0;
	InitStudyCommentFields();
	pStudyCommentText = &StudyCommentTextArray[ nEdit ];

	size_t			StringLength;
	while ( pStudyCommentText -> ResourceSymbol != 0 )
		{
		switch ( pStudyCommentText -> ResourceSymbol )
			{
			case IDC_EDIT_IMAGE_QUALITY_OTHER:
				StringLength = (size_t)m_ImageDefectOtherText.GetLength();					// *[1] Forced allocation size to be unsigned.
				if ( StringLength > 0 && StringLength < 4096 )								// *[1] 
					{
					pStudyCommentText -> pEditText = (char*)malloc( StringLength + 1 );		// *[1] Forced allocation size to be unsigned.
					if ( pStudyCommentText -> pEditText != 0 )								// *[2[ Added check for successful allocation.
						strncpy_s( pStudyCommentText -> pEditText, StringLength, (const char*)m_ImageDefectOtherText, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
					}
				else
					pStudyCommentText -> pEditText = 0;
				break;
			case IDC_EDIT_OTHER_COMMENTS:
				StringLength = (size_t)m_OtherAbnormalitiesCommentsText.GetLength();		// *[1] Forced allocation size to be unsigned.
				if ( StringLength > 0 && StringLength < 4096 )								// *[1] 
					{
					pStudyCommentText -> pEditText = (char*)malloc( StringLength + 1 );		// *[1] Forced allocation size to be unsigned.
					if ( pStudyCommentText -> pEditText != 0 )								// *[2[ Added check for successful allocation.
						strncpy_s( pStudyCommentText -> pEditText, StringLength, (const char*)m_OtherAbnormalitiesCommentsText, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
					}
				else
					pStudyCommentText -> pEditText = 0;
				break;
			}
		nEdit++;
		pStudyCommentText = &StudyCommentTextArray[ nEdit ];
		}
}


BOOL CStudy::StudyButtonWasOn( int TargetButtonResourceSymbol )
{
	STUDY_BUTTON_STATE		*pStudyButtonState;
	int						nButton;
	BOOL					bButtonFound;
	BOOL					bButtonIsToggledOn;
	
	nButton = 0;
	pStudyButtonState = &StudyButtonStateArray[ nButton ];
	bButtonFound = FALSE;
	bButtonIsToggledOn = FALSE;
	while ( pStudyButtonState -> ResourceSymbol != 0 && !bButtonFound )
		{
		if ( pStudyButtonState -> ResourceSymbol == TargetButtonResourceSymbol )
			{
			bButtonFound = TRUE;
			bButtonIsToggledOn = pStudyButtonState -> bButtonToggledOn;
			}
		nButton++;
		pStudyButtonState = &StudyButtonStateArray[ nButton ];
		}

	return bButtonIsToggledOn;
}


void CStudy::GetStudyEditField( int TargetTextResourceSymbol, char *pTextField )
{
	STUDY_EDIT_TEXT			*pStudyEditText;
	int						nEdit;
	BOOL					bEditFound;
	
	nEdit = 0;
	pStudyEditText = &StudyEditTextArray[ nEdit ];
	bEditFound = FALSE;
	while ( pStudyEditText -> ResourceSymbol != 0 && !bEditFound )
		{
		if ( pStudyEditText -> ResourceSymbol == TargetTextResourceSymbol )
			{
			bEditFound = TRUE;
			strncpy_s( pTextField, FILE_PATH_STRING_LENGTH, pStudyEditText -> EditText, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
			}
		nEdit++;
		pStudyEditText = &StudyEditTextArray[ nEdit ];
		}
}


char *CStudy::GetStudyCommentField( int TargetTextResourceSymbol )
{
	STUDY_COMMENT_TEXT		*pStudyCommentText;
	int						nEdit;
	BOOL					bEditFound;
	char					*pTextField;
	
	nEdit = 0;
	pTextField = 0;
	pStudyCommentText = &StudyCommentTextArray[ nEdit ];
	bEditFound = FALSE;
	while ( pStudyCommentText -> ResourceSymbol != 0 && !bEditFound )
		{
		if ( pStudyCommentText -> ResourceSymbol == TargetTextResourceSymbol )
			{
			bEditFound = TRUE;
			pTextField = pStudyCommentText -> pEditText;
			}
		nEdit++;
		pStudyCommentText = &StudyCommentTextArray[ nEdit ];
		}

	return pTextField;
}


void CStudy::ConvertDicomDAToSystemTime( char *pDicomTextDate, SYSTEMTIME *pSystemTime )
{

	memset( (void*)pSystemTime, 0, sizeof( SYSTEMTIME ) );
	if ( strlen( pDicomTextDate ) == 8 )
		sscanf_s( pDicomTextDate, "%4u%2u%2u", (unsigned int*)&pSystemTime -> wYear, (unsigned int*)&pSystemTime -> wMonth,  (unsigned int*)&pSystemTime -> wDay );	// *[1] Replaced sscanf with sscanf_s.
}


void CStudy::PopulateFromAbstractDataRow( char *pTitleRow, char *pDataRow )
{
	char					ListItemText[ 2048 ];
	char					*pFieldText;
	char					*pChar;
	BOOL					bMatchingFieldFound;

	// Load the attributes for this subject.
	//
	// Extract the reader to whom this study was addressed.
	if ( GetAbstractColumnValueForSpecifiedField( "DestinationAE", pTitleRow, pDataRow, ListItemText, 2048 ) )
		{
		m_ReaderAddressed[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
		strncat_s( m_ReaderAddressed, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );		// *[1] Replaced strncat with strncat_s.
		}
	// Extract the patient last name and first name.
	bMatchingFieldFound = GetAbstractColumnValueForSpecifiedField( "PatientName", pTitleRow, pDataRow, ListItemText, 2048 );
	if ( !bMatchingFieldFound )		// Check for older version of Dicom dictionary.
		bMatchingFieldFound = GetAbstractColumnValueForSpecifiedField( "PatientsName", pTitleRow, pDataRow, ListItemText, 2048 );
	if ( bMatchingFieldFound )
		{
		pFieldText = ListItemText;
		pChar = strchr( pFieldText, '^' );
		if ( pChar != 0 )
			{
			*pChar = '\0';
			m_PatientLastName[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( m_PatientLastName, DICOM_ATTRIBUTE_STRING_LENGTH, pFieldText, _TRUNCATE );	// *[1] Replaced strncat with strncat_s.
			pFieldText = pChar + 1;
			pChar = strchr( pFieldText, '^' );
			if ( pChar != 0 )
				*pChar = '\0';
			m_PatientFirstName[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( m_PatientFirstName, DICOM_ATTRIBUTE_STRING_LENGTH, pFieldText, _TRUNCATE );	// *[1] Replaced strncat with strncat_s.
			}
		else
			{
			m_PatientLastName[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( m_PatientLastName, DICOM_ATTRIBUTE_STRING_LENGTH, pFieldText, _TRUNCATE );	// *[1] Replaced strncat with strncat_s.
			}
		}
	if ( GetAbstractColumnValueForSpecifiedField( "PatientID", pTitleRow, pDataRow, ListItemText, 2048 ) )
		{
		m_PatientID[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
		strncat_s( m_PatientID, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );			// *[1] Replaced strncat with strncat_s.
		}
	bMatchingFieldFound = GetAbstractColumnValueForSpecifiedField( "PatientsBirthDate", pTitleRow, pDataRow, ListItemText, 2048 );
	if ( !bMatchingFieldFound )
		bMatchingFieldFound = GetAbstractColumnValueForSpecifiedField( "PatientBirthDate", pTitleRow, pDataRow, ListItemText, 2048 );
	if ( bMatchingFieldFound )
		{
		ConvertDicomDAToSystemTime( ListItemText, &m_PatientsBirthDate.Date );
		if ( m_PatientsBirthDate.Date.wYear != 0 )
			m_PatientsBirthDate.bDateHasBeenEdited = TRUE;
		}
	bMatchingFieldFound = GetAbstractColumnValueForSpecifiedField( "PatientsSex", pTitleRow, pDataRow, ListItemText, 2048 );
	if ( !bMatchingFieldFound )
		bMatchingFieldFound = GetAbstractColumnValueForSpecifiedField( "PatientSex", pTitleRow, pDataRow, ListItemText, 2048 );
	if ( bMatchingFieldFound )
		{
		m_PatientsSex[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
		strncat_s( m_PatientsSex, 4, ListItemText, _TRUNCATE );													// *[1] Replaced strncat with strncat_s.
		}
	if ( GetAbstractColumnValueForSpecifiedField( "PatientComments", pTitleRow, pDataRow, ListItemText, 2048 ) )
		{
		m_PatientComments[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
		strncat_s( m_PatientComments, DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH, ListItemText, _TRUNCATE );		// *[1] Replaced strncat with strncat_s.
		}
	if ( GetAbstractColumnValueForSpecifiedField( "AccessionNumber", pTitleRow, pDataRow, ListItemText, 2048 ) )
		{
		m_AccessionNumber[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
		strncat_s( m_AccessionNumber, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );					// *[1] Replaced strncat with strncat_s.
		}
	if ( GetAbstractColumnValueForSpecifiedField( "StudyDate", pTitleRow, pDataRow, ListItemText, 2048 ) )
		{
		ConvertDicomDAToSystemTime( ListItemText, &m_DateOfRadiograph.Date );
		if ( m_DateOfRadiograph.Date.wYear != 0 )
			m_DateOfRadiograph.bDateHasBeenEdited = TRUE;
		}
	
	LoadStudyData( pTitleRow, pDataRow );
}


void CStudy::LoadStudyData( char *pTitleRow, char *pDataRow )
{
	DIAGNOSTIC_STUDY		*pDiagnosticStudy;
	DIAGNOSTIC_STUDY		*pNewDiagnosticStudy;
	char					ListItemText[ 2048 ];
	BOOL					bMatchingFieldFound;

	// Load the attributes for this study.
	pNewDiagnosticStudy = (DIAGNOSTIC_STUDY*)calloc( 1, sizeof( DIAGNOSTIC_STUDY ) );
	if ( pNewDiagnosticStudy != 0 )
		{
		if ( GetAbstractColumnValueForSpecifiedField( "AccessionNumber", pTitleRow, pDataRow, ListItemText, 2048 ) )
			{
			pNewDiagnosticStudy -> AccessionNumber[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( pNewDiagnosticStudy -> AccessionNumber, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );				// *[1] Replaced strncat with strncat_s.
			}
		if ( GetAbstractColumnValueForSpecifiedField( "StudyDate", pTitleRow, pDataRow, ListItemText, 2048 ) )
			{
			pNewDiagnosticStudy -> StudyDate[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( pNewDiagnosticStudy -> StudyDate, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );						// *[1] Replaced strncat with strncat_s.
			}
		if ( GetAbstractColumnValueForSpecifiedField( "StudyTime", pTitleRow, pDataRow, ListItemText, 2048 ) )
			{
			pNewDiagnosticStudy -> StudyTime[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( pNewDiagnosticStudy -> StudyTime, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );						// *[1] Replaced strncat with strncat_s.
			}
		bMatchingFieldFound = GetAbstractColumnValueForSpecifiedField( "ReferringPhysiciansName", pTitleRow, pDataRow, ListItemText, 2048 );
		if ( !bMatchingFieldFound )
			bMatchingFieldFound = GetAbstractColumnValueForSpecifiedField( "ReferringPhysicianName", pTitleRow, pDataRow, ListItemText, 2048 );
		if ( bMatchingFieldFound )
			{
			pNewDiagnosticStudy -> ReferringPhysiciansName[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( pNewDiagnosticStudy -> ReferringPhysiciansName, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );		// *[1] Replaced strncat with strncat_s.
			}
		bMatchingFieldFound = GetAbstractColumnValueForSpecifiedField( "ReferringPhysiciansTelephoneNumbers", pTitleRow, pDataRow, ListItemText, 2048 );
		if ( !bMatchingFieldFound )
			bMatchingFieldFound = GetAbstractColumnValueForSpecifiedField( "ReferringPhysicianTelephoneNumbers", pTitleRow, pDataRow, ListItemText, 2048 );
		if ( bMatchingFieldFound )
			{
			pNewDiagnosticStudy -> ReferringPhysiciansPhone[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( pNewDiagnosticStudy -> ReferringPhysiciansPhone, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );		// *[1] Replaced strncat with strncat_s.
			}
		if ( GetAbstractColumnValueForSpecifiedField( "ResponsibleOrganization", pTitleRow, pDataRow, ListItemText, 2048 ) )
			{
			pNewDiagnosticStudy -> ResponsibleOrganization[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( pNewDiagnosticStudy -> ResponsibleOrganization, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );		// *[1] Replaced strncat with strncat_s.
			}
		if ( GetAbstractColumnValueForSpecifiedField( "InstitutionName", pTitleRow, pDataRow, ListItemText, 2048 ) )
			{
			pNewDiagnosticStudy -> InstitutionName[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( pNewDiagnosticStudy -> InstitutionName, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );				// *[1] Replaced strncat with strncat_s.
			}
		if ( GetAbstractColumnValueForSpecifiedField( "StudyID", pTitleRow, pDataRow, ListItemText, 2048 ) )
			{
			pNewDiagnosticStudy -> StudyID[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( pNewDiagnosticStudy -> StudyID, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );						// *[1] Replaced strncat with strncat_s.
			}
		if ( GetAbstractColumnValueForSpecifiedField( "StudyDescription", pTitleRow, pDataRow, ListItemText, 2048 ) )
			{
			pNewDiagnosticStudy -> StudyDescription[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( pNewDiagnosticStudy -> StudyDescription, DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH, ListItemText, _TRUNCATE );	// *[1] Replaced strncat with strncat_s.
			}
		if ( GetAbstractColumnValueForSpecifiedField( "StudyInstanceUID", pTitleRow, pDataRow, ListItemText, 2048 ) )
			{
			pNewDiagnosticStudy -> StudyInstanceUID[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( pNewDiagnosticStudy -> StudyInstanceUID, DICOM_ATTRIBUTE_UI_STRING_LENGTH, ListItemText, _TRUNCATE );			// *[1] Replaced strncat with strncat_s.
			}
		pNewDiagnosticStudy -> pDiagnosticSeriesList = 0;
		pNewDiagnosticStudy -> pNextDiagnosticStudy = 0;
		if ( m_pDiagnosticStudyList == 0 )
			m_pDiagnosticStudyList = pNewDiagnosticStudy;
		else
			{
			// Append the new study to the end of the list.
			pDiagnosticStudy = m_pDiagnosticStudyList;
			while ( pDiagnosticStudy != 0 && pDiagnosticStudy -> pNextDiagnosticStudy != 0 )
				pDiagnosticStudy = pDiagnosticStudy -> pNextDiagnosticStudy;
			pDiagnosticStudy -> pNextDiagnosticStudy = pNewDiagnosticStudy;
			}
		LoadSeriesData( pTitleRow, pDataRow, pNewDiagnosticStudy );
		}
}


void CStudy::LoadSeriesData( char *pTitleRow, char *pDataRow, DIAGNOSTIC_STUDY *pDiagnosticStudy )
{
	DIAGNOSTIC_SERIES		*pDiagnosticSeries;
	DIAGNOSTIC_SERIES		*pNewDiagnosticSeries;
	char					ListItemText[ 2048 ];

	// Load the attributes for this series.
	pNewDiagnosticSeries = (DIAGNOSTIC_SERIES*)calloc( 1, sizeof( DIAGNOSTIC_SERIES ) );
	if ( pNewDiagnosticSeries != 0 )
		{
		if ( GetAbstractColumnValueForSpecifiedField( "Modality", pTitleRow, pDataRow, ListItemText, 2048 ) )
			{
			pNewDiagnosticSeries -> Modality[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( pNewDiagnosticSeries -> Modality, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );							// *[1] Replaced strncat with strncat_s.
			}
		if ( GetAbstractColumnValueForSpecifiedField( "SeriesNumber", pTitleRow, pDataRow, ListItemText, 2048 ) )
			{
			pNewDiagnosticSeries -> SeriesNumber[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( pNewDiagnosticSeries -> SeriesNumber, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );						// *[1] Replaced strncat with strncat_s.
			}
		if ( GetAbstractColumnValueForSpecifiedField( "Laterality", pTitleRow, pDataRow, ListItemText, 2048 ) )
			{
			pNewDiagnosticSeries -> Laterality[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( pNewDiagnosticSeries -> Laterality, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );						// *[1] Replaced strncat with strncat_s.
			}
		if ( GetAbstractColumnValueForSpecifiedField( "SeriesDate", pTitleRow, pDataRow, ListItemText, 2048 ) )
			{
			pNewDiagnosticSeries -> SeriesDate[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( pNewDiagnosticSeries -> SeriesDate, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );						// *[1] Replaced strncat with strncat_s.
			}
		if ( GetAbstractColumnValueForSpecifiedField( "SeriesTime", pTitleRow, pDataRow, ListItemText, 2048 ) )
			{
			pNewDiagnosticSeries -> SeriesTime[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( pNewDiagnosticSeries -> SeriesTime, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );						// *[1] Replaced strncat with strncat_s.
			}
		if ( GetAbstractColumnValueForSpecifiedField( "ProtocolName", pTitleRow, pDataRow, ListItemText, 2048 ) )
			{
			pNewDiagnosticSeries -> ProtocolName[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( pNewDiagnosticSeries -> ProtocolName, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );						// *[1] Replaced strncat with strncat_s.
			}
		if ( GetAbstractColumnValueForSpecifiedField( "SeriesDescription", pTitleRow, pDataRow, ListItemText, 2048 ) )
			{
			pNewDiagnosticSeries -> SeriesDescription[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( pNewDiagnosticSeries -> SeriesDescription, DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH, ListItemText, _TRUNCATE );		// *[1] Replaced strncat with strncat_s.
			}
		if ( GetAbstractColumnValueForSpecifiedField( "BodyPartExamined", pTitleRow, pDataRow, ListItemText, 2048 ) )
			{
			pNewDiagnosticSeries -> BodyPartExamined[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( pNewDiagnosticSeries -> BodyPartExamined, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );					// *[1] Replaced strncat with strncat_s.
			}
		if ( GetAbstractColumnValueForSpecifiedField( "PatientPosition", pTitleRow, pDataRow, ListItemText, 2048 ) )
			{
			pNewDiagnosticSeries -> PatientPosition[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( pNewDiagnosticSeries -> PatientPosition, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );					// *[1] Replaced strncat with strncat_s.
			}
		if ( GetAbstractColumnValueForSpecifiedField( "PatientOrientation", pTitleRow, pDataRow, ListItemText, 2048 ) )
			{
			pNewDiagnosticSeries -> PatientOrientation[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( pNewDiagnosticSeries -> PatientOrientation, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );				// *[1] Replaced strncat with strncat_s.
			}
		if ( GetAbstractColumnValueForSpecifiedField( "SeriesInstanceUID", pTitleRow, pDataRow, ListItemText, 2048 ) )
			{
			pNewDiagnosticSeries -> SeriesInstanceUID[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( pNewDiagnosticSeries -> SeriesInstanceUID, DICOM_ATTRIBUTE_UI_STRING_LENGTH, ListItemText, _TRUNCATE );				// *[1] Replaced strncat with strncat_s.
			}
		if ( GetAbstractColumnValueForSpecifiedField( "Manufacturer", pTitleRow, pDataRow, ListItemText, 2048 ) )
			{
			pNewDiagnosticSeries -> Manufacturer[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
			strncat_s( pNewDiagnosticSeries -> Manufacturer, DICOM_ATTRIBUTE_UI_STRING_LENGTH, ListItemText, _TRUNCATE );					// *[1] Replaced strncat with strncat_s.
			}
		pNewDiagnosticSeries -> pDiagnosticImageList = 0;
		pNewDiagnosticSeries -> pNextDiagnosticSeries = 0;
		if ( pDiagnosticStudy -> pDiagnosticSeriesList == 0 )
			pDiagnosticStudy -> pDiagnosticSeriesList = pNewDiagnosticSeries;
		else
			{
			// Append the new study to the end of the series list for the specified study.
			pDiagnosticSeries = pDiagnosticStudy -> pDiagnosticSeriesList;
			while ( pDiagnosticSeries != 0 && pDiagnosticSeries -> pNextDiagnosticSeries != 0 )
				pDiagnosticSeries = pDiagnosticSeries -> pNextDiagnosticSeries;
			pDiagnosticSeries -> pNextDiagnosticSeries = pNewDiagnosticSeries;
			}

		LoadImageData( pTitleRow, pDataRow, pNewDiagnosticSeries );
		}
}


void CStudy::LoadImageData( char *pTitleRow, char *pDataRow, DIAGNOSTIC_SERIES *pDiagnosticSeries )
{
	DIAGNOSTIC_IMAGE		*pDiagnosticImage;
	DIAGNOSTIC_IMAGE		*pNewDiagnosticImage;
	char					ListItemText[ 2048 ];

	// Load the attributes for this image.
	pNewDiagnosticImage = (DIAGNOSTIC_IMAGE*)calloc( 1, sizeof( DIAGNOSTIC_IMAGE ) );
	if ( pNewDiagnosticImage != 0 )
		{
		if ( GetAbstractColumnValueForSpecifiedField( "ImageType", pTitleRow, pDataRow, ListItemText, 2048 ) )
			strncpy_s( pNewDiagnosticImage -> ImageType, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );					// *[2] Replaced strncat with strncpy_s.
		if ( GetAbstractColumnValueForSpecifiedField( "InstanceNumber", pTitleRow, pDataRow, ListItemText, 2048 ) )
			strncpy_s( pNewDiagnosticImage -> InstanceNumber, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );				// *[2] Replaced strncat with strncpy_s.
		if ( GetAbstractColumnValueForSpecifiedField( "InstanceCreationDate", pTitleRow, pDataRow, ListItemText, 2048 ) )
			strncpy_s( pNewDiagnosticImage -> InstanceCreationDate, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );		// *[2] Replaced strncat with strncpy_s.
		if ( GetAbstractColumnValueForSpecifiedField( "InstanceCreationTime", pTitleRow, pDataRow, ListItemText, 2048 ) )
			strncpy_s( pNewDiagnosticImage -> InstanceCreationTime, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );		// *[2] Replaced strncat with strncpy_s.
		if ( GetAbstractColumnValueForSpecifiedField( "ContentDate", pTitleRow, pDataRow, ListItemText, 2048 ) )
			strncpy_s( pNewDiagnosticImage -> ContentDate, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );				// *[2] Replaced strncat with strncpy_s.
		if ( GetAbstractColumnValueForSpecifiedField( "ContentTime", pTitleRow, pDataRow, ListItemText, 2048 ) )
			strncpy_s( pNewDiagnosticImage -> ContentTime, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );				// *[2] Replaced strncat with strncpy_s.
		if ( GetAbstractColumnValueForSpecifiedField( "AcquisitionNumber", pTitleRow, pDataRow, ListItemText, 2048 ) )
			strncpy_s( pNewDiagnosticImage -> AcquisitionNumber, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );			// *[2] Replaced strncat with strncpy_s.
		if ( GetAbstractColumnValueForSpecifiedField( "AcquisitionDate", pTitleRow, pDataRow, ListItemText, 2048 ) )
			strncpy_s( pNewDiagnosticImage -> AcquisitionDate, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );			// *[2] Replaced strncat with strncpy_s.
		if ( GetAbstractColumnValueForSpecifiedField( "AcquisitionTime", pTitleRow, pDataRow, ListItemText, 2048 ) )
			strncpy_s( pNewDiagnosticImage -> AcquisitionTime, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );			// *[2] Replaced strncat with strncpy_s.
		if ( GetAbstractColumnValueForSpecifiedField( "SamplesPerPixel", pTitleRow, pDataRow, ListItemText, 2048 ) )
			strncpy_s( pNewDiagnosticImage -> SamplesPerPixel, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );			// *[2] Replaced strncat with strncpy_s.
		if ( GetAbstractColumnValueForSpecifiedField( "PhotometricInterpretation", pTitleRow, pDataRow, ListItemText, 2048 ) )
			strncpy_s( pNewDiagnosticImage -> PhotometricInterpretation, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );	// *[2] Replaced strncat with strncpy_s.
		if ( GetAbstractColumnValueForSpecifiedField( "Rows", pTitleRow, pDataRow, ListItemText, 2048 ) )
			strncpy_s( pNewDiagnosticImage -> Rows, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );						// *[2] Replaced strncat with strncpy_s.
		if ( GetAbstractColumnValueForSpecifiedField( "Columns", pTitleRow, pDataRow, ListItemText, 2048 ) )
			strncpy_s( pNewDiagnosticImage -> Columns, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );					// *[2] Replaced strncat with strncpy_s.
		if ( GetAbstractColumnValueForSpecifiedField( "PixelAspectRatio", pTitleRow, pDataRow, ListItemText, 2048 ) )
			strncpy_s( pNewDiagnosticImage -> PixelAspectRatio, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );			// *[2] Replaced strncat with strncpy_s.
		if ( GetAbstractColumnValueForSpecifiedField( "BitsAllocated", pTitleRow, pDataRow, ListItemText, 2048 ) )
			strncpy_s( pNewDiagnosticImage -> BitsAllocated, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );				// *[2] Replaced strncat with strncpy_s.
		if ( GetAbstractColumnValueForSpecifiedField( "BitsStored", pTitleRow, pDataRow, ListItemText, 2048 ) )
			strncpy_s( pNewDiagnosticImage -> BitsStored, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );					// *[2] Replaced strncat with strncpy_s.
		if ( GetAbstractColumnValueForSpecifiedField( "HighBit", pTitleRow, pDataRow, ListItemText, 2048 ) )
			strncpy_s( pNewDiagnosticImage -> HighBit, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );					// *[2] Replaced strncat with strncpy_s.
		if ( GetAbstractColumnValueForSpecifiedField( "PixelRepresentation", pTitleRow, pDataRow, ListItemText, 2048 ) )
			strncpy_s( pNewDiagnosticImage -> PixelRepresentation, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );		// *[2] Replaced strncat with strncpy_s.
		if ( GetAbstractColumnValueForSpecifiedField( "WindowCenter", pTitleRow, pDataRow, ListItemText, 2048 ) )
			strncpy_s( pNewDiagnosticImage -> WindowCenter, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );				// *[2] Replaced strncat with strncpy_s.
		if ( GetAbstractColumnValueForSpecifiedField( "WindowWidth", pTitleRow, pDataRow, ListItemText, 2048 ) )
			strncpy_s( pNewDiagnosticImage -> WindowWidth, DICOM_ATTRIBUTE_STRING_LENGTH, ListItemText, _TRUNCATE );				// *[2] Replaced strncat with strncpy_s.
		if ( GetAbstractColumnValueForSpecifiedField( "SOPInstanceUID", pTitleRow, pDataRow, ListItemText, 2048 ) )
			strncpy_s( pNewDiagnosticImage -> SOPInstanceUID, DICOM_ATTRIBUTE_UI_STRING_LENGTH, ListItemText, _TRUNCATE );			// *[2] Replaced strncat with strncpy_s.
		pNewDiagnosticImage -> pNextDiagnosticImage = 0;
		if ( pDiagnosticSeries -> pDiagnosticImageList == 0 )
			pDiagnosticSeries -> pDiagnosticImageList = pNewDiagnosticImage;
		else
			{
			// Append the new study to the end of the series list for the specified study.
			pDiagnosticImage = pDiagnosticSeries -> pDiagnosticImageList;
			while ( pDiagnosticImage != 0 && pDiagnosticImage -> pNextDiagnosticImage != 0 )
				pDiagnosticImage = pDiagnosticImage -> pNextDiagnosticImage;
			pDiagnosticImage -> pNextDiagnosticImage = pNewDiagnosticImage;
			}
		}
}


BOOL CStudy::MergeWithExistingStudies( BOOL *pbNewStudyMergedWithExistingStudy )
{
	BOOL					bNoError = TRUE;
	BOOL					bAvailableStudiesExamined;
	CStudy					*pExistingStudy;
	LIST_ELEMENT			*pStudyListElement;
	BOOL					bMatchingPatientFound;
	BOOL					bMatchingStudyFound;
	BOOL					bMatchingSeriesFound;
	BOOL					bMatchingImageFound;
	DIAGNOSTIC_STUDY		*pNewDiagnosticStudy = 0;					// *[2] Initialize pointer.
	DIAGNOSTIC_STUDY		*pExistingDiagnosticStudy;
	DIAGNOSTIC_SERIES		*pNewDiagnosticSeries = 0;					// *[2] Added redundant initialization to please Fortify.
	DIAGNOSTIC_SERIES		*pExistingDiagnosticSeries;
	DIAGNOSTIC_IMAGE		*pNewDiagnosticImage = 0;					// *[2] Initialize pointer.
	DIAGNOSTIC_IMAGE		*pExistingDiagnosticImage;

	*pbNewStudyMergedWithExistingStudy = FALSE;
	// Add to the study list the unedited studies from the abstract database.
	bMatchingPatientFound = FALSE;
	bMatchingStudyFound = FALSE;
	bMatchingSeriesFound = FALSE;
	bMatchingImageFound = FALSE;
	pExistingStudy = 0;
	pExistingDiagnosticStudy = 0;
	pExistingDiagnosticSeries = 0;
	pExistingDiagnosticImage = 0;
	// The new (current) study only includes a single study, series and image, since it was imported
	// from a single Dicom image file:
	// Check for a match with existing studies.
	bAvailableStudiesExamined = FALSE;
	pStudyListElement = ThisBViewerApp.m_NewlyArrivedStudyList;
	while ( pStudyListElement != 0 && !bMatchingPatientFound )
		{
		pExistingStudy = (CStudy*)pStudyListElement -> pItem;
		if ( pExistingStudy != 0 &&
				strcmp( pExistingStudy -> m_PatientLastName, m_PatientLastName ) == 0 &&
				strcmp( pExistingStudy -> m_PatientFirstName, m_PatientFirstName ) == 0 &&
				strcmp( pExistingStudy -> m_AccessionNumber, m_AccessionNumber ) == 0 &&
				strcmp( pExistingStudy -> m_PatientID, m_PatientID ) == 0 )
			{
			bMatchingPatientFound = TRUE;
			// A matching patient was found in the existing list.  See if this patient has a
			// study that matches the new one.
			pNewDiagnosticStudy = m_pDiagnosticStudyList;		// There is only one study in this list.
			pExistingDiagnosticStudy = pExistingStudy -> m_pDiagnosticStudyList;
			while ( pNewDiagnosticStudy != 0 && pExistingDiagnosticStudy != 0 && !bMatchingStudyFound )
				{
				if ( strcmp( pNewDiagnosticStudy -> StudyInstanceUID, pExistingDiagnosticStudy -> StudyInstanceUID ) == 0 )
					{
					bMatchingStudyFound = TRUE;
					// A matching study was found in the existing list for this patient.  See if this study has a
					// series that matches the new one.
					pNewDiagnosticSeries = pNewDiagnosticStudy -> pDiagnosticSeriesList;	// There is only one series in this list.
					pExistingDiagnosticSeries = pExistingDiagnosticStudy -> pDiagnosticSeriesList;
					while ( pExistingDiagnosticSeries != 0 && !bMatchingSeriesFound )
						{
						if ( strcmp( pNewDiagnosticSeries -> SeriesInstanceUID, pExistingDiagnosticSeries -> SeriesInstanceUID ) == 0 )
							{
							bMatchingSeriesFound = TRUE;
							// A matching series was found in the existing list for this study.  See if this series has an
							// image that matches the new one.
							pNewDiagnosticImage = pNewDiagnosticSeries -> pDiagnosticImageList;	// There is only one image in this list.
							pExistingDiagnosticImage = pExistingDiagnosticSeries -> pDiagnosticImageList;
							while ( pExistingDiagnosticImage != 0 && !bMatchingImageFound )
								{
								if ( strcmp( pNewDiagnosticImage -> SOPInstanceUID, pExistingDiagnosticImage -> SOPInstanceUID ) == 0 )
									{
									// A matching image was found in the existing list for this series.
									bMatchingImageFound = TRUE;
									}
								if ( !bMatchingImageFound )
									pExistingDiagnosticImage = pExistingDiagnosticImage -> pNextDiagnosticImage;
								}
							}
						if ( !bMatchingSeriesFound )
							pExistingDiagnosticSeries = pExistingDiagnosticSeries -> pNextDiagnosticSeries;
						}
					}
				if ( !bMatchingStudyFound )
					pExistingDiagnosticStudy = pExistingDiagnosticStudy -> pNextDiagnosticStudy;
				}
			}
		if ( !bMatchingPatientFound )
			{
			pStudyListElement = pStudyListElement -> pNextListElement;
			if ( pStudyListElement == 0 && !bAvailableStudiesExamined )
				{
				pStudyListElement = ThisBViewerApp.m_AvailableStudyList;
				bAvailableStudiesExamined = TRUE;
				}
			}
		}
	if ( !bMatchingImageFound )			// If the new image isn't a duplicate...
		{
		if ( bMatchingSeriesFound )
			{
			// Move to the end of the list of existing images for this study.
			if ( pExistingDiagnosticSeries != 0 )
				{
				pExistingDiagnosticImage = pExistingDiagnosticSeries -> pDiagnosticImageList;
				while ( pExistingDiagnosticImage != 0 && pExistingDiagnosticImage -> pNextDiagnosticImage != 0 )
					pExistingDiagnosticImage = pExistingDiagnosticImage -> pNextDiagnosticImage;
				if ( pExistingDiagnosticImage != 0 )
					pExistingDiagnosticImage -> pNextDiagnosticImage = pNewDiagnosticImage;
				// Zero the pointer to the transferred image, so it won't be deleted along with the rest of the new study.
				pNewDiagnosticSeries -> pDiagnosticImageList = 0;
				}
			}
		else if ( bMatchingStudyFound )			// If the series doesn't match, but the study does...
			{
			// Move to the end of the list of existing series for this study.
			if ( pExistingDiagnosticStudy != 0 )
				{
				pExistingDiagnosticSeries = pExistingDiagnosticStudy -> pDiagnosticSeriesList;
				while ( pExistingDiagnosticSeries != 0 && pExistingDiagnosticSeries -> pNextDiagnosticSeries != 0 )
					pExistingDiagnosticSeries = pExistingDiagnosticSeries -> pNextDiagnosticSeries;
				if ( pExistingDiagnosticSeries != 0 )
					pExistingDiagnosticSeries -> pNextDiagnosticSeries = pNewDiagnosticSeries;
				// Zero the pointer to the transferred series, so it won't be deleted along with the rest of the new study.
				pNewDiagnosticStudy -> pDiagnosticSeriesList = 0;
				}
			}
		else if ( bMatchingPatientFound )			// If the study doesn't match, but the patient does...
			{
			// Move to the end of the list of existing studies for this patient.
			if ( pExistingStudy != 0 )
				{
				pExistingDiagnosticStudy = pExistingStudy -> m_pDiagnosticStudyList;
				while ( pExistingDiagnosticStudy != 0 && pExistingDiagnosticStudy -> pNextDiagnosticStudy != 0 )
					pExistingDiagnosticStudy = pExistingDiagnosticStudy -> pNextDiagnosticStudy;
				if ( pExistingDiagnosticStudy != 0 )
					{
					pExistingDiagnosticStudy -> pNextDiagnosticStudy = pNewDiagnosticStudy;
					// Zero the pointer to the transferred study, so it won't be deleted along with the rest of the new study.
					m_pDiagnosticStudyList = 0;
					}
				else
					{
					RemoveFromList( &ThisBViewerApp.m_AvailableStudyList, (void*)pExistingStudy );
					delete pExistingStudy;
					bMatchingPatientFound = FALSE;		// Allow the new patient to be added in place of the corrupt one.
					}
				}
			}
		}
	else
		LogMessage( "Matching image found.", MESSAGE_TYPE_SUPPLEMENTARY );
	if ( bMatchingImageFound || bMatchingSeriesFound || bMatchingStudyFound || bMatchingPatientFound )
		{
		// If the new study object contributed to an existing one, mark the new one for deletion and save the enhanced existing study.
		*pbNewStudyMergedWithExistingStudy = TRUE;
		LogMessage( "Enhancing existing study.", MESSAGE_TYPE_SUPPLEMENTARY );
		if ( pExistingStudy != 0 )
			bNoError = pExistingStudy -> Save();			// *[1] Ensure there is not null pointer dereference.
		}
	else
		{
		*pbNewStudyMergedWithExistingStudy = FALSE;
		bNoError = Save();
		}

	return bNoError;
}


void CStudy::GetDateOfRadiographMMDDYY( char *pDateString )
{
	int							nChars;

	_snprintf_s( pDateString, 32, _TRUNCATE, "%02d%02d%04d",	// *[2] Replaced sprintf() with _snprintf_s.
				m_DateOfRadiograph.Date.wMonth, m_DateOfRadiograph.Date.wDay, m_DateOfRadiograph.Date.wYear );
	nChars = strlen( pDateString );
	pDateString[ nChars - 4 ] = pDateString[ nChars - 2 ];
	pDateString[ nChars - 3 ] = pDateString[ nChars - 1 ];
	pDateString[ nChars - 2 ] = '\0';
}


BOOL CStudy::SaveDiagnosticImage( FILE *pStudyFile, DIAGNOSTIC_IMAGE *pDiagnosticImage )
{
	BOOL					bNoError = TRUE;
	size_t					nBytesToWrite;
	size_t					nBytesWritten;

	nBytesToWrite = sizeof(DIAGNOSTIC_IMAGE);
	nBytesWritten = fwrite( pDiagnosticImage, 1, nBytesToWrite, pStudyFile );
	bNoError = ( nBytesWritten == nBytesToWrite );

	return bNoError;
}


BOOL CStudy::SaveDiagnosticSeries( FILE *pStudyFile, DIAGNOSTIC_SERIES *pDiagnosticSeries )
{
	BOOL					bNoError = TRUE;
	size_t					nBytesToWrite;
	size_t					nBytesWritten;
	DIAGNOSTIC_IMAGE		*pDiagnosticImage;
	unsigned long			nImages;
	unsigned long			LengthInBytes;

	nBytesToWrite = sizeof(DIAGNOSTIC_SERIES);
	nBytesWritten = fwrite( pDiagnosticSeries, 1, nBytesToWrite, pStudyFile );
	bNoError = ( nBytesWritten == nBytesToWrite );
	if ( bNoError )
		{
		// Record the number of images for the current study.
		pDiagnosticImage = pDiagnosticSeries -> pDiagnosticImageList;
		nImages = 0;
		while ( pDiagnosticImage != 0 )
			{
			nImages++;
			pDiagnosticImage = pDiagnosticImage -> pNextDiagnosticImage;
			}
		nBytesToWrite = sizeof(unsigned long);
		nBytesWritten = fwrite( &nImages, 1, nBytesToWrite, pStudyFile );
		bNoError = ( nBytesWritten == nBytesToWrite );
		if ( bNoError )
			{
			// Record the length of the image structure.
			nBytesToWrite = sizeof(unsigned long);
			LengthInBytes = sizeof(DIAGNOSTIC_IMAGE);
			nBytesWritten = fwrite( &LengthInBytes, 1, nBytesToWrite, pStudyFile );
			bNoError = ( nBytesWritten == nBytesToWrite );
			}
		if ( bNoError )
			{
			// Record the list of images for the current study.
			pDiagnosticImage = pDiagnosticSeries -> pDiagnosticImageList;
			nImages = 0;
			while ( bNoError && pDiagnosticImage != 0 )
				{
				bNoError = SaveDiagnosticImage( pStudyFile, pDiagnosticImage );
				pDiagnosticImage = pDiagnosticImage -> pNextDiagnosticImage;
				}
			}
		}

	return bNoError;
}


BOOL CStudy::SaveDiagnosticStudy( FILE *pStudyFile, DIAGNOSTIC_STUDY *pDiagnosticStudy )
{
	BOOL					bNoError = TRUE;
	size_t					nBytesToWrite;
	size_t					nBytesWritten;
	DIAGNOSTIC_SERIES		*pDiagnosticSeries;
	unsigned long			nSeries;
	unsigned long			LengthInBytes;

	nBytesToWrite = sizeof(DIAGNOSTIC_STUDY);
	nBytesWritten = fwrite( pDiagnosticStudy, 1, nBytesToWrite, pStudyFile );
	bNoError = ( nBytesWritten == nBytesToWrite );
	if ( bNoError )
		{
		// Record the number of series for the current study.
		pDiagnosticSeries = pDiagnosticStudy -> pDiagnosticSeriesList;
		nSeries = 0;
		while ( pDiagnosticSeries != 0 )
			{
			nSeries++;
			pDiagnosticSeries = pDiagnosticSeries -> pNextDiagnosticSeries;
			}
		nBytesToWrite = sizeof(unsigned long);
		nBytesWritten = fwrite( &nSeries, 1, nBytesToWrite, pStudyFile );
		bNoError = ( nBytesWritten == nBytesToWrite );
		if ( bNoError )
			{
			// Record the length of the series structure.
			nBytesToWrite = sizeof(unsigned long);
			LengthInBytes = sizeof(DIAGNOSTIC_SERIES);
			nBytesWritten = fwrite( &LengthInBytes, 1, nBytesToWrite, pStudyFile );
			bNoError = ( nBytesWritten == nBytesToWrite );
			}
		if ( bNoError )
			{
			// Record the list of series for the current study.
			pDiagnosticSeries = pDiagnosticStudy -> pDiagnosticSeriesList;
			while ( bNoError && pDiagnosticSeries != 0 )
				{
				bNoError = SaveDiagnosticSeries( pStudyFile, pDiagnosticSeries );
				pDiagnosticSeries = pDiagnosticSeries -> pNextDiagnosticSeries;
				}
			}
		}

	return bNoError;
}


void CStudy::GetStudyFileName( char *pStudyFileName, size_t BufferSize )
{
	strncpy_s( pStudyFileName, BufferSize, m_PatientLastName, _TRUNCATE );																// *[1] Replaced strcpy with strncpy_s.
	strncat_s( pStudyFileName, BufferSize, "_", _TRUNCATE );																			// *[3] Replaced strcat with strncat_s.
	strncat_s( pStudyFileName, BufferSize, m_PatientFirstName, _TRUNCATE );																// *[3] Replaced strcat with strncat_s.
	strncat_s( pStudyFileName, BufferSize, "_", _TRUNCATE );																			// *[3] Replaced strcat with strncat_s.
	strncat_s( pStudyFileName, BufferSize, m_PatientID, _TRUNCATE );																	// *[3] Replaced strcat with strncat_s.
	strncat_s( pStudyFileName, BufferSize, "_", _TRUNCATE );																			// *[3] Replaced strcat with strncat_s.
	if ( strlen( m_AccessionNumber ) == 0 )
		strncat_s( m_AccessionNumber, DICOM_ATTRIBUTE_STRING_LENGTH, this -> m_pDiagnosticStudyList -> AccessionNumber, _TRUNCATE );	// *[3] Replaced strcat with strncat_s.
	strncat_s( pStudyFileName, BufferSize, m_AccessionNumber, _TRUNCATE );																// *[3] Replaced strcat with strncat_s.
	strncat_s( pStudyFileName, BufferSize, ".sdy", _TRUNCATE );																			// *[3] Replaced strcat with strncat_s.
}


BOOL CStudy::Save()
{
	BOOL				bNoError = TRUE;
	BOOL				bFileWrittenSuccessfully;
	char				FileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				DataDirectory[ FILE_PATH_STRING_LENGTH ];
	FILE				*pStudyFile;
	size_t				nBytesToWrite;
	size_t				nBytesWritten;
	size_t				nRemainingCharacters;
	unsigned long		LengthInBytes;
	DIAGNOSTIC_STUDY	*pDiagnosticStudy;
	unsigned long		nStudies;
	char				Msg[ FULL_FILE_SPEC_STRING_LENGTH ];

	strncpy_s( DataDirectory, FILE_PATH_STRING_LENGTH, BViewerConfiguration.DataDirectory, _TRUNCATE );			// *[2] Replaced strncat with strncpy_s.
	if ( DataDirectory[ strlen( DataDirectory ) - 1 ] != '\\' )
		strncat_s( DataDirectory, FILE_PATH_STRING_LENGTH, "\\", _TRUNCATE );									// *[2] Replaced strcat with strncat_s.
	// Check existence of path to configuration directory.
	bNoError = SetCurrentDirectory( DataDirectory );
	if ( !bNoError )
		bNoError = CreateDirectory( DataDirectory, NULL );
	if ( bNoError )
		{
		strncpy_s( FileSpec, FULL_FILE_SPEC_STRING_LENGTH, DataDirectory, _TRUNCATE );							// *[1] Replaced strcpy with strncpy_s.
		nRemainingCharacters = FULL_FILE_SPEC_STRING_LENGTH - strlen( FileSpec );
		GetStudyFileName( &FileSpec[ strlen( FileSpec ) ], nRemainingCharacters );
		_snprintf_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, _TRUNCATE, "Saving study data to file %s.", FileSpec );	// *[2] Replaced sprintf() with _snprintf_s.
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
		pStudyFile = fopen( FileSpec, "wb" );
		if ( pStudyFile != 0 )
			{
			nBytesToWrite = DICOM_ATTRIBUTE_STRING_LENGTH;

			nBytesWritten = fwrite( m_ReaderAddressed, 1, nBytesToWrite, pStudyFile );
			bNoError = ( nBytesWritten == nBytesToWrite );
			if ( bNoError )
				{
				nBytesWritten = fwrite( m_PatientLastName, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( m_PatientFirstName, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( m_PatientID, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesToWrite = sizeof(EDITED_DATE);
				nBytesWritten = fwrite( &m_PatientsBirthDate, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesToWrite = 4;
				nBytesWritten = fwrite( m_PatientsSex, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesToWrite = DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH;
				nBytesWritten = fwrite( m_PatientComments, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesToWrite = sizeof(double);
				nBytesWritten = fwrite( &m_Reserved, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( &m_Reserved, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( &m_GammaSetting, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( &m_WindowCenter, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( &m_WindowWidth, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( &m_MaxGrayscaleValue, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesToWrite = 32;
				nBytesWritten = fwrite( m_TimeStudyFirstOpened, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( m_TimeReportApproved, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesToWrite = sizeof(UINT);
				nBytesWritten = fwrite( &m_nCurrentObjectID, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesToWrite = sizeof(BOOL);
				nBytesWritten = fwrite( &m_bImageQualityVisited, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( &m_bParenchymalAbnormalitiesVisited, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( &m_bPleuralAbnormalitiesVisited, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( &m_bOtherAbnormalitiesVisited, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesToWrite = sizeof(char);
				nBytesWritten = fwrite( &m_AnyParenchymalAbnormalities, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( &m_AnyPleuralAbnormalities, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( &m_AnyOtherAbnormalities, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesToWrite = sizeof(unsigned long);
				nBytesWritten = fwrite( &m_ImageQuality, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( &m_ObservedParenchymalAbnormalities, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesToWrite = sizeof(unsigned short);
				nBytesWritten = fwrite( &m_ObservedPleuralPlaqueSites, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( &m_ObservedPleuralCalcificationSites, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( &m_ObservedPlaqueExtent, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( &m_ObservedPlaqueWidth, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( &m_ObservedCostophrenicAngleObliteration, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( &m_ObservedPleuralThickeningSites, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( &m_ObservedThickeningCalcificationSites, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( &m_ObservedThickeningExtent, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( &m_ObservedThickeningWidth, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesToWrite = sizeof(unsigned long);
				nBytesWritten = fwrite( &m_ObservedOtherSymbols, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesWritten = fwrite( &m_ObservedOtherAbnormalities, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesToWrite = sizeof(unsigned long);
				LengthInBytes = m_ImageDefectOtherText.GetLength();
				nBytesWritten = fwrite( &LengthInBytes, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				if ( bNoError )
					{
					nBytesToWrite = LengthInBytes;
					nBytesWritten = fwrite( (const char*)m_ImageDefectOtherText, 1, nBytesToWrite, pStudyFile );
					bNoError = ( nBytesWritten == nBytesToWrite );
					}
				}
			if ( bNoError )
				{
				nBytesToWrite = sizeof(unsigned long);
				LengthInBytes = m_OtherAbnormalitiesCommentsText.GetLength();
				nBytesWritten = fwrite( &LengthInBytes, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				if ( bNoError )
					{
					nBytesToWrite = LengthInBytes;
					nBytesWritten = fwrite( (const char*)m_OtherAbnormalitiesCommentsText, 1, nBytesToWrite, pStudyFile );
					bNoError = ( nBytesWritten == nBytesToWrite );
					}
				}
			if ( bNoError )
				{
				nBytesToWrite = sizeof(unsigned long);
				nBytesWritten = fwrite( &m_PhysicianNotificationStatus, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesToWrite = sizeof(EDITED_DATE);
				nBytesWritten = fwrite( &m_Reserved2, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesToWrite = sizeof(EDITED_DATE);
				nBytesWritten = fwrite( &m_DateOfRadiograph, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesToWrite = sizeof(unsigned long);
				LengthInBytes = sizeof( m_Reserved1 );
				nBytesWritten = fwrite( &LengthInBytes, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				if ( bNoError )
					{
					nBytesToWrite = LengthInBytes;
					nBytesWritten = fwrite( m_Reserved1, 1, nBytesToWrite, pStudyFile );
					bNoError = ( nBytesWritten == nBytesToWrite );
					}
				}
			if ( bNoError )
				{
				nBytesToWrite = sizeof(unsigned short);
				nBytesWritten = fwrite( &m_TypeOfReading, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}
			if ( bNoError )
				{
				nBytesToWrite = sizeof(unsigned long);
				LengthInBytes = sizeof( m_OtherTypeOfReading );
				nBytesWritten = fwrite( &LengthInBytes, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				if ( bNoError )
					{
					nBytesToWrite = LengthInBytes;
					nBytesWritten = fwrite( m_OtherTypeOfReading, 1, nBytesToWrite, pStudyFile );
					bNoError = ( nBytesWritten == nBytesToWrite );
					}
				}
			if ( bNoError )
				{
				nBytesToWrite = sizeof(unsigned long);
				LengthInBytes = sizeof( m_FacilityIDNumber );
				nBytesWritten = fwrite( &LengthInBytes, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				if ( bNoError )
					{
					nBytesToWrite = LengthInBytes;
					nBytesWritten = fwrite( m_FacilityIDNumber, 1, nBytesToWrite, pStudyFile );
					bNoError = ( nBytesWritten == nBytesToWrite );
					}
				}
			if ( bNoError )
				{
				nBytesToWrite = sizeof(EDITED_DATE);
				nBytesWritten = fwrite( &m_DateOfReading, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}

			if ( bNoError )
				{
				nBytesToWrite = sizeof(BOOL);
				nBytesWritten = fwrite( &m_bReportViewed, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}

			if ( bNoError )
				{
				nBytesToWrite = sizeof(BOOL);
				nBytesWritten = fwrite( &m_bReportApproved, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				}

			if ( bNoError )
				{
				// Record the number of studies for the current patient.
				pDiagnosticStudy = m_pDiagnosticStudyList;
				nStudies = 0;
				while (pDiagnosticStudy != 0 )
					{
					nStudies++;
					pDiagnosticStudy = pDiagnosticStudy -> pNextDiagnosticStudy;
					}
				nBytesToWrite = sizeof(unsigned long);
				nBytesWritten = fwrite( &nStudies, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				if ( bNoError )
					{
					// Record the length of the study structure.
					nBytesToWrite = sizeof(unsigned long);
					LengthInBytes = sizeof(DIAGNOSTIC_STUDY);
					nBytesWritten = fwrite( &LengthInBytes, 1, nBytesToWrite, pStudyFile );
					bNoError = ( nBytesWritten == nBytesToWrite );
					}
				if ( bNoError )
					{
					// Record the list of studies for the current patient.
					pDiagnosticStudy = m_pDiagnosticStudyList;
					while ( bNoError && pDiagnosticStudy != 0 )
						{
						bNoError = SaveDiagnosticStudy( pStudyFile, pDiagnosticStudy );
						pDiagnosticStudy = pDiagnosticStudy -> pNextDiagnosticStudy;
						}
					}
				}
			if ( bNoError )
				{
				nBytesToWrite = sizeof( m_SDYFileVersion );
				nBytesWritten = fwrite( &m_SDYFileVersion, 1, nBytesToWrite, pStudyFile );
				bNoError = ( nBytesWritten == nBytesToWrite );
				if ( bNoError )
					{
					nBytesToWrite = sizeof( READER_PERSONAL_INFO );
					nBytesWritten = fwrite( &m_ReaderInfo, 1, nBytesToWrite, pStudyFile );
					bNoError = ( nBytesWritten == nBytesToWrite );
					}
				if ( bNoError )
					{
					nBytesToWrite = sizeof( BOOL );
					nBytesWritten = fwrite( &m_bStudyWasPreviouslyInterpreted, 1, nBytesToWrite, pStudyFile );
					bNoError = ( nBytesWritten == nBytesToWrite );
					}
				if ( bNoError )
					{
					nBytesToWrite = sizeof( CLIENT_INFO );
					nBytesWritten = fwrite( &m_ClientInfo, 1, nBytesToWrite, pStudyFile );
					bNoError = ( nBytesWritten == nBytesToWrite );
					}
				}
			fclose( pStudyFile );
			}
		else
			bNoError = FALSE;
		}
	bFileWrittenSuccessfully = bNoError;
	
	return bFileWrittenSuccessfully;
}


BOOL CStudy::RestoreDiagnosticImage( FILE *pStudyFile, DIAGNOSTIC_IMAGE **ppDiagnosticImage )
{
	BOOL					bNoError = TRUE;
	size_t					nBytesToRead;
	size_t					nBytesRead;
	DIAGNOSTIC_IMAGE		*pDiagnosticImage;

	nBytesToRead = sizeof(DIAGNOSTIC_IMAGE);
	pDiagnosticImage = ( DIAGNOSTIC_IMAGE* )malloc( sizeof(DIAGNOSTIC_IMAGE) );
	bNoError = ( pDiagnosticImage != 0 );
	if ( bNoError )
		{
		nBytesRead = fread_s( pDiagnosticImage, sizeof(DIAGNOSTIC_IMAGE), 1, nBytesToRead, pStudyFile );		// *[2] Converted from fread to fread_s.
		bNoError = ( nBytesRead == nBytesToRead );
		pDiagnosticImage -> pNextDiagnosticImage = 0;
		}
	if ( bNoError )
		*ppDiagnosticImage = pDiagnosticImage;
	else
		{
		*ppDiagnosticImage = 0;
		if ( pDiagnosticImage != 0 )			// *[1] Prevent memory leak ater bad file read.
			free( pDiagnosticImage );			// *[1]
		}

	return bNoError;
}


BOOL CStudy::RestoreDiagnosticSeries( FILE *pStudyFile, DIAGNOSTIC_SERIES **ppDiagnosticSeries, BOOL bOlderSeriesWithoutManufacturer )
{
	BOOL					bNoError = TRUE;
	size_t					nBytesToRead;
	size_t					nBytesRead;
	DIAGNOSTIC_SERIES		*pDiagnosticSeries;
	DIAGNOSTIC_IMAGE		**ppDiagnosticImage;
	unsigned long			nImageCount;
	unsigned long			nImage;
	unsigned long			LengthInBytes;

	nBytesToRead = sizeof(DIAGNOSTIC_SERIES);
	if ( bOlderSeriesWithoutManufacturer )
		nBytesToRead -= DICOM_ATTRIBUTE_UI_STRING_LENGTH;
	pDiagnosticSeries = (DIAGNOSTIC_SERIES*)malloc( sizeof(DIAGNOSTIC_SERIES) );
	bNoError = ( pDiagnosticSeries != 0 );
	if ( bNoError )
		{
		nBytesRead = fread_s( pDiagnosticSeries, sizeof(DIAGNOSTIC_SERIES), 1, nBytesToRead, pStudyFile );		// *[2] Converted from fread to fread_s.
		bNoError = ( nBytesRead == nBytesToRead );
		pDiagnosticSeries -> pNextDiagnosticSeries = 0;
		if ( bOlderSeriesWithoutManufacturer )
			pDiagnosticSeries -> Manufacturer[ 0 ] = '\0';		// *[1] Eliminated call to strcpy.
		}
	if ( bNoError )
		{
		// Read the number of images for the current study.
		nBytesToRead = sizeof(unsigned long);
		nBytesRead = fread_s( &nImageCount, sizeof(unsigned long), 1, nBytesToRead, pStudyFile );				// *[2] Converted from fread to fread_s.
		bNoError = ( nBytesRead == nBytesToRead );
		}
	if ( bNoError )
		{
		// Read the length of the image structure.
		nBytesToRead = sizeof(unsigned long);
		nBytesRead = fread_s( &LengthInBytes, sizeof(unsigned long), 1, nBytesToRead, pStudyFile );				// *[2] Converted from fread to fread_s.
		bNoError = ( nBytesRead == nBytesToRead );
		if ( bNoError )
			if ( LengthInBytes != sizeof(DIAGNOSTIC_IMAGE) )
				bNoError = FALSE;
		}
	if ( bNoError )
		{
		// Read the list of images for the current study.
		ppDiagnosticImage = &pDiagnosticSeries -> pDiagnosticImageList;
		for ( nImage = 0; nImage < nImageCount; nImage++ )
			{
			bNoError = RestoreDiagnosticImage( pStudyFile, ppDiagnosticImage );
			ppDiagnosticImage = &( *ppDiagnosticImage ) -> pNextDiagnosticImage;
			}
		}
	if ( bNoError )
		*ppDiagnosticSeries = pDiagnosticSeries;
	else
		{
		*ppDiagnosticSeries = 0;
		if ( pDiagnosticSeries != 0 )			// *[1] Fixed potential memory leak.
			free( pDiagnosticSeries );			// *[1]
		}

	return bNoError;
}


BOOL CStudy::RestoreDiagnosticStudy( FILE *pStudyFile, DIAGNOSTIC_STUDY **ppDiagnosticStudy )
{
	BOOL					bNoError = TRUE;
	BOOL					bOlderSeriesWithoutManufacturer;
	size_t					nBytesToRead;
	size_t					nBytesRead;
	DIAGNOSTIC_STUDY		*pDiagnosticStudy;
	DIAGNOSTIC_SERIES		**ppDiagnosticSeries;
	unsigned long			nSeriesCount;
	unsigned long			nSeries;
	unsigned long			LengthInBytes;

	bOlderSeriesWithoutManufacturer = FALSE;
	nBytesToRead = sizeof(DIAGNOSTIC_STUDY);
	pDiagnosticStudy = (DIAGNOSTIC_STUDY*)malloc( sizeof(DIAGNOSTIC_STUDY) );
	bNoError = ( pDiagnosticStudy != 0 );
	if ( bNoError )
		{
		nBytesRead = fread_s( pDiagnosticStudy, sizeof(DIAGNOSTIC_STUDY), 1, nBytesToRead, pStudyFile );				// *[2] Converted from fread to fread_s.
		bNoError = ( nBytesRead == nBytesToRead );
		pDiagnosticStudy -> pNextDiagnosticStudy = 0;
		}
	if ( bNoError )
		strncpy_s( m_AccessionNumber, DICOM_ATTRIBUTE_STRING_LENGTH, pDiagnosticStudy -> AccessionNumber, _TRUNCATE );	// *[2] Replaced strncat with strncpy_s.
	if ( bNoError )
		{
		// Read the number of series recorded for this study.
		nBytesToRead = sizeof(unsigned long);
		nBytesRead = fread_s( &nSeriesCount, sizeof(unsigned long), 1, nBytesToRead, pStudyFile );						// *[2] Converted from fread to fread_s.
		bNoError = ( nBytesRead == nBytesToRead );
		}
	if ( bNoError )
		{
		// Read the length of the series structure.
		nBytesToRead = sizeof(unsigned long);
		nBytesRead = fread_s( &LengthInBytes, sizeof(unsigned long), 1, nBytesToRead, pStudyFile );					// *[2] Converted from fread to fread_s.
		bNoError = ( nBytesRead == nBytesToRead );
		if ( bNoError )
			if ( LengthInBytes != sizeof(DIAGNOSTIC_SERIES) )
				{
				if ( LengthInBytes == sizeof(DIAGNOSTIC_SERIES) - DICOM_ATTRIBUTE_UI_STRING_LENGTH )
					bOlderSeriesWithoutManufacturer = TRUE;
				else
					bNoError = FALSE;
				}
		}
	if ( bNoError )
		{
		// Read the list of series for the current study.
		ppDiagnosticSeries = &pDiagnosticStudy -> pDiagnosticSeriesList;
		for ( nSeries = 0; nSeries < nSeriesCount; nSeries++ )
			{
			bNoError = RestoreDiagnosticSeries( pStudyFile, ppDiagnosticSeries, bOlderSeriesWithoutManufacturer );
			ppDiagnosticSeries = &( *ppDiagnosticSeries ) -> pNextDiagnosticSeries;
			}
		}
	if ( bNoError )
		*ppDiagnosticStudy = pDiagnosticStudy;
	else
		{
		*ppDiagnosticStudy = 0;
		if  ( pDiagnosticStudy != 0 )
			free( pDiagnosticStudy );
		}

	return bNoError;
}


BOOL CStudy::Restore( char *pFullFilePath )
{
	BOOL				bNoError = TRUE;
	BOOL				bFileReadSuccessfully;
	FILE				*pStudyFile;
	size_t				nBytesToRead;
	size_t				nBytesRead;
	size_t				nBytesInEarlierBViewerVersions;
	unsigned long		LengthInBytes;
	size_t				StringLength;
	DIAGNOSTIC_STUDY	**ppDiagnosticStudy;
	unsigned long		nStudyCount;
	unsigned long		nStudy;
	char				*pTextBuffer = 0;	// *[2] Added redundant initialization to please Fortify.
	char				Msg[ FULL_FILE_SPEC_STRING_LENGTH ];

	pStudyFile = fopen( pFullFilePath, "rb" );
	if ( pStudyFile != 0 )
		{
		nBytesToRead = DICOM_ATTRIBUTE_STRING_LENGTH;
		nBytesRead = fread_s( m_ReaderAddressed, DICOM_ATTRIBUTE_STRING_LENGTH, 1, nBytesToRead, pStudyFile );					// *[2] Converted from fread to fread_s.
		bNoError = ( nBytesRead == nBytesToRead );
		if ( bNoError )
			{
			nBytesRead = fread_s( m_PatientLastName, DICOM_ATTRIBUTE_UI_STRING_LENGTH, 1, nBytesToRead, pStudyFile );			// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( m_PatientFirstName, DICOM_ATTRIBUTE_UI_STRING_LENGTH, 1, nBytesToRead, pStudyFile );			// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( m_PatientID, DICOM_ATTRIBUTE_UI_STRING_LENGTH, 1, nBytesToRead, pStudyFile );					// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(EDITED_DATE);
			nBytesRead = fread_s( &m_PatientsBirthDate, sizeof(EDITED_DATE), 1, nBytesToRead, pStudyFile );						// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = 4;
			nBytesRead = fread_s( m_PatientsSex, 4, 1, nBytesToRead, pStudyFile );												// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH;
			nBytesRead = fread_s( m_PatientComments, DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH, 1, nBytesToRead, pStudyFile );	// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(double);
			nBytesRead = fread_s( &m_Reserved, sizeof(double), 1, nBytesToRead, pStudyFile );									// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( &m_Reserved, sizeof(double), 1, nBytesToRead, pStudyFile );									// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( &m_GammaSetting, sizeof(double), 1, nBytesToRead, pStudyFile );								// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( &m_WindowCenter, sizeof(double), 1, nBytesToRead, pStudyFile );								// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( &m_WindowWidth, sizeof(double), 1, nBytesToRead, pStudyFile );								// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( &m_MaxGrayscaleValue, sizeof(double), 1, nBytesToRead, pStudyFile );							// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = 32;
			nBytesRead = fread_s( m_TimeStudyFirstOpened, 32, 1, nBytesToRead, pStudyFile );									// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( m_TimeReportApproved, 32, 1, nBytesToRead, pStudyFile );										// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(UINT);
			nBytesRead = fread_s( &m_nCurrentObjectID, sizeof(UINT), 1, nBytesToRead, pStudyFile );								// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(BOOL);
			nBytesRead = fread_s( &m_bImageQualityVisited, sizeof(BOOL), 1, nBytesToRead, pStudyFile );							// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( &m_bParenchymalAbnormalitiesVisited, sizeof(BOOL), 1, nBytesToRead, pStudyFile );				// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( &m_bPleuralAbnormalitiesVisited, sizeof(BOOL), 1, nBytesToRead, pStudyFile );					// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( &m_bOtherAbnormalitiesVisited, sizeof(BOOL), 1, nBytesToRead, pStudyFile );					// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(char);
			nBytesRead = fread_s( &m_AnyParenchymalAbnormalities, sizeof(char), 1, nBytesToRead, pStudyFile );					// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( &m_AnyPleuralAbnormalities, sizeof(char), 1, nBytesToRead, pStudyFile );						// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( &m_AnyOtherAbnormalities, sizeof(char), 1, nBytesToRead, pStudyFile );						// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(unsigned long);
			nBytesRead = fread_s( &m_ImageQuality, sizeof(unsigned long), 1, nBytesToRead, pStudyFile );						// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( &m_ObservedParenchymalAbnormalities, sizeof(unsigned long), 1, nBytesToRead, pStudyFile );	// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(unsigned short);
			nBytesRead = fread_s( &m_ObservedPleuralPlaqueSites, sizeof(unsigned short), 1, nBytesToRead, pStudyFile );			// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( &m_ObservedPleuralCalcificationSites, sizeof(unsigned short), 1, nBytesToRead, pStudyFile );	// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( &m_ObservedPlaqueExtent, sizeof(unsigned short), 1, nBytesToRead, pStudyFile );				// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( &m_ObservedPlaqueWidth, sizeof(unsigned short), 1, nBytesToRead, pStudyFile );				// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( &m_ObservedCostophrenicAngleObliteration, sizeof(unsigned short), 1, nBytesToRead, pStudyFile );	// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( &m_ObservedPleuralThickeningSites, sizeof(unsigned short), 1, nBytesToRead, pStudyFile );			// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( &m_ObservedThickeningCalcificationSites, sizeof(unsigned short), 1, nBytesToRead, pStudyFile );	// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( &m_ObservedThickeningExtent, sizeof(unsigned short), 1, nBytesToRead, pStudyFile );				// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( &m_ObservedThickeningWidth, sizeof(unsigned short), 1, nBytesToRead, pStudyFile );				// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(unsigned long);
			nBytesRead = fread_s( &m_ObservedOtherSymbols, sizeof(unsigned long), 1, nBytesToRead, pStudyFile );					// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread_s( &m_ObservedOtherAbnormalities, sizeof(unsigned long), 1, nBytesToRead, pStudyFile );				// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(unsigned long);
			nBytesRead = fread_s( &LengthInBytes, sizeof(unsigned long), 1, nBytesToRead, pStudyFile );								// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead && LengthInBytes < DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH );
			if ( bNoError )
				{
				StringLength = (size_t)LengthInBytes;																				// *[1] Forced allocation size to be data type size_t.
				pTextBuffer = (char*)malloc( StringLength + 1 );																	// *[1]
				bNoError = ( pTextBuffer != 0 );
				}
			if ( bNoError )
				{
				nBytesToRead = LengthInBytes;
				nBytesRead = fread_s( pTextBuffer, StringLength + 1, 1, nBytesToRead, pStudyFile );												// *[2] Converted from fread to fread_s.
				pTextBuffer[ nBytesToRead ] = '\0';
				bNoError = ( nBytesRead == nBytesToRead );
				if ( bNoError )
					m_ImageDefectOtherText = pTextBuffer;
				free( pTextBuffer );
				}
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(unsigned long);
			nBytesRead = fread_s( &LengthInBytes, sizeof(unsigned long), 1, nBytesToRead, pStudyFile );								// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead && LengthInBytes < DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH );
			if ( bNoError )
				{
				StringLength = (size_t)LengthInBytes;																				// *[1] Forced allocation size to be data type size_t.
				pTextBuffer = (char*)malloc( StringLength + 1 );																	// *[1]
				bNoError = ( pTextBuffer != 0 );
				}
			if ( bNoError )
				{
				nBytesToRead = LengthInBytes;
				nBytesRead = fread_s( pTextBuffer, StringLength + 1, 1, nBytesToRead, pStudyFile );									// *[2] Converted from fread to fread_s.
				pTextBuffer[ nBytesToRead ] = '\0';
				bNoError = ( nBytesRead == nBytesToRead );
				if ( bNoError )
					m_OtherAbnormalitiesCommentsText = pTextBuffer;
				free( pTextBuffer );
				}
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(unsigned long);
			nBytesRead = fread_s( &m_PhysicianNotificationStatus, sizeof(unsigned long), 1, nBytesToRead, pStudyFile );				// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(EDITED_DATE);
			nBytesRead = fread_s( &m_Reserved2, sizeof(EDITED_DATE), 1, nBytesToRead, pStudyFile );									// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(EDITED_DATE);
			nBytesRead = fread_s( &m_DateOfRadiograph, sizeof(EDITED_DATE), 1, nBytesToRead, pStudyFile );							// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}

		if ( bNoError )
			{
			nBytesToRead = sizeof(unsigned long);
			nBytesRead = fread_s( &LengthInBytes, sizeof(unsigned long), 1, nBytesToRead, pStudyFile );								// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead && LengthInBytes < DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH );
			if ( bNoError )
				{
				StringLength = (size_t)LengthInBytes;																				// *[1] Forced allocation size to be data type size_t.
				pTextBuffer = (char*)malloc( StringLength + 1 );																	// *[1]
				bNoError = ( pTextBuffer != 0 );
				}
			if ( bNoError )
				{
				nBytesToRead = LengthInBytes;
				nBytesRead = fread_s( pTextBuffer, StringLength + 1, 1, nBytesToRead, pStudyFile );									// *[2] Converted from fread to fread_s.
				bNoError = ( nBytesRead == nBytesToRead );
				if ( bNoError )
					strncpy_s( m_Reserved1, 12, pTextBuffer, _TRUNCATE );															// *[1] Replaced strcpy with strncpy_s.
				free( pTextBuffer );
				pTextBuffer = 0;																									// *[1] Added this for code safety.
				}
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(unsigned short);
			nBytesRead = fread_s( &m_TypeOfReading, sizeof(unsigned short), 1, nBytesToRead, pStudyFile );							// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(unsigned long);
			nBytesRead = fread_s( &LengthInBytes, sizeof(unsigned long), 1, nBytesToRead, pStudyFile );								// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead && LengthInBytes < DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH );
			if ( bNoError )
				{
				StringLength = (size_t)LengthInBytes;																				// *[1] Forced allocation size to be data type size_t.
				pTextBuffer = (char*)malloc( StringLength + 1 );																	// *[1]
				bNoError = ( pTextBuffer != 0 );
				}
			if ( bNoError )
				{
				nBytesToRead = LengthInBytes;
				nBytesRead = fread_s( pTextBuffer, StringLength + 1, 1, nBytesToRead, pStudyFile );									// *[2] Converted from fread to fread_s.
				bNoError = ( nBytesRead == nBytesToRead );
				if ( bNoError )
					strncpy_s( m_OtherTypeOfReading, DICOM_ATTRIBUTE_STRING_LENGTH, pTextBuffer, _TRUNCATE );						// *[2] Replaced strncat with strncpy_s.
				free( pTextBuffer );
				pTextBuffer = 0;																									// *[1] Added this for code safety.
				}
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(unsigned long);
			nBytesRead = fread_s( &LengthInBytes, sizeof(unsigned long), 1, nBytesToRead, pStudyFile );								// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead && LengthInBytes < DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH );
			if ( bNoError )
				{
				StringLength = (size_t)LengthInBytes;																				// *[1] Forced allocation size to be data type size_t.
				pTextBuffer = (char*)malloc( StringLength + 1 );																	// *[1]
				bNoError = ( pTextBuffer != 0 );
				}
			if ( bNoError )
				{
				nBytesToRead = LengthInBytes;
				nBytesRead = fread_s( pTextBuffer, StringLength + 1, 1, nBytesToRead, pStudyFile );									// *[2] Converted from fread to fread_s.
				bNoError = ( nBytesRead == nBytesToRead );
				if ( bNoError )
					strncpy_s( m_FacilityIDNumber, 10, pTextBuffer, _TRUNCATE );													// *[2] Replaced strncat with strncpy_s.
				free( pTextBuffer );
				pTextBuffer = 0;																									// *[1] Added this for code safety.
				}
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(EDITED_DATE);
			nBytesRead = fread_s( &m_DateOfReading, sizeof(EDITED_DATE), 1, nBytesToRead, pStudyFile );								// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}

		if ( bNoError )
			{
			nBytesToRead = sizeof(BOOL);
			nBytesRead = fread_s( &m_bReportViewed, sizeof(BOOL), 1, nBytesToRead, pStudyFile );									// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(BOOL);
			nBytesRead = fread_s( &m_bReportApproved, sizeof(BOOL), 1, nBytesToRead, pStudyFile );									// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			// Read the number of studies recorded for this patient.
			nBytesToRead = sizeof(unsigned long);
			nBytesRead = fread_s( &nStudyCount, sizeof(unsigned long), 1, nBytesToRead, pStudyFile );								// *[2] Converted from fread to fread_s.
			bNoError = ( nBytesRead == nBytesToRead );
			if ( bNoError )
				{
				// Read the length of the study structure.
				nBytesToRead = sizeof(unsigned long);
				nBytesRead = fread_s( &LengthInBytes, sizeof(unsigned long), 1, nBytesToRead, pStudyFile );							// *[2] Converted from fread to fread_s.
				bNoError = ( nBytesRead == nBytesToRead );
				if ( bNoError )
					if ( LengthInBytes != sizeof(DIAGNOSTIC_STUDY) )
						bNoError = FALSE;
				}
			if ( bNoError )
				{
				// Read the list of studies for the current patient.
				ppDiagnosticStudy = &m_pDiagnosticStudyList;
				for ( nStudy = 0; nStudy < nStudyCount && bNoError; nStudy++ )
					{
					bNoError = RestoreDiagnosticStudy( pStudyFile, ppDiagnosticStudy );
					ppDiagnosticStudy = &( *ppDiagnosticStudy ) -> pNextDiagnosticStudy;
					}
				}
			if ( bNoError )
				{
				nBytesToRead = sizeof(m_SDYFileVersion);
				nBytesRead = fread_s( &m_SDYFileVersion, sizeof(unsigned long), 1, nBytesToRead, pStudyFile );						// *[2] Converted from fread to fread_s.
				if ( nBytesRead == 0 )
					m_SDYFileVersion = 0;
				switch ( m_SDYFileVersion )
					{
					case 1:
					case 2:
						nBytesToRead = nBytesInEarlierBViewerVersions = 352;
						// The more recently added members of READER_PERSONAL_INFO, namely IsDefaultReader, m_CountryInfo, and pwLength will not be modified by this read, but will be preserved.
						nBytesRead = fread_s( &m_ReaderInfo, sizeof(READER_PERSONAL_INFO), 1, nBytesToRead, pStudyFile );				// *[2] Converted from fread to fread_s.
						bNoError = ( nBytesRead == nBytesInEarlierBViewerVersions );													// *[4] READER_PERSONAL_INFO size changed in BViewer 1.2u.
						if ( bNoError )
							{
							nBytesToRead = sizeof(BOOL);
							nBytesRead = fread_s( &m_bStudyWasPreviouslyInterpreted, sizeof(BOOL), 1, nBytesToRead, pStudyFile );		// *[2] Converted from fread to fread_s.
							bNoError = ( nBytesRead == nBytesToRead );
							}
						if ( m_SDYFileVersion == 1 )
							break;
						nBytesToRead = sizeof(CLIENT_INFO);
						nBytesRead = fread_s( &m_ClientInfo, sizeof(CLIENT_INFO), 1, nBytesToRead, pStudyFile );						// *[2] Converted from fread to fread_s.
						bNoError = ( nBytesRead == nBytesToRead );
						break;
					case 3:
						nBytesToRead = sizeof(READER_PERSONAL_INFO);
						nBytesRead = fread_s( &m_ReaderInfo, sizeof(READER_PERSONAL_INFO), 1, nBytesToRead, pStudyFile );				// *[2] Converted from fread to fread_s.
						bNoError = ( nBytesRead == nBytesToRead );						// *[4] READER_PERSONAL_INFO size changed in BViewer 1.2u.
						if ( bNoError )
							{
							nBytesToRead = sizeof(BOOL);
							nBytesRead = fread_s( &m_bStudyWasPreviouslyInterpreted, sizeof(BOOL), 1, nBytesToRead, pStudyFile );		// *[2] Converted from fread to fread_s.
							bNoError = ( nBytesRead == nBytesToRead );
							}
						if ( bNoError )
							{
							nBytesToRead = sizeof(CLIENT_INFO);
							nBytesRead = fread_s( &m_ClientInfo, sizeof(CLIENT_INFO), 1, nBytesToRead, pStudyFile );						// *[2] Converted from fread to fread_s.
							bNoError = ( nBytesRead == nBytesToRead );
							}
						break;
					}
				}
			if ( !bNoError )
				{
				_snprintf_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, _TRUNCATE, "Unable to restore study file for %s", m_PatientLastName );	// *[2] Replaced sprintf() with _snprintf_s.
				LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
				}
			}
		fclose( pStudyFile );
		}
	else
		bNoError = FALSE;
	bFileReadSuccessfully = bNoError;
	if ( bNoError )
		UnpackData();
	
	return bFileReadSuccessfully;
}


void CStudy::DeleteStudyDataAndImages()
{
	DIAGNOSTIC_STUDY		*pStudyDataRow;
	DIAGNOSTIC_SERIES		*pSeriesDataRow;
	DIAGNOSTIC_IMAGE		*pImageDataRow;
	char					ImagePath[ FILE_PATH_STRING_LENGTH ];
	char					ImageFileName[ FILE_PATH_STRING_LENGTH ];
	char					FullImageFileSpecification[ FILE_PATH_STRING_LENGTH ];
	char					DataDirectory[ FILE_PATH_STRING_LENGTH ];
	char					DataFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					Msg[ FULL_FILE_SPEC_STRING_LENGTH ];

	strncpy_s( ImagePath, FILE_PATH_STRING_LENGTH, BViewerConfiguration.ImageDirectory, _TRUNCATE );	// *[2] Replaced strncat with strncpy_s.
	if ( ImagePath[ strlen( ImagePath ) - 1 ] != '\\' )
		strncat_s( ImagePath, FILE_PATH_STRING_LENGTH, "\\", _TRUNCATE );								// *[2] Replaced strcat with strncat_s.

	// Delete all the image files for this study.
	pStudyDataRow = m_pDiagnosticStudyList;
	while ( pStudyDataRow != 0 )
		{
		pSeriesDataRow = pStudyDataRow -> pDiagnosticSeriesList;
		while ( pSeriesDataRow != 0 )
			{
			pImageDataRow = pSeriesDataRow -> pDiagnosticImageList;
			while ( pImageDataRow != 0 )
				{
				if ( pImageDataRow -> SOPInstanceUID != 0 )
					{
					strncpy_s( ImageFileName, FILE_PATH_STRING_LENGTH, pImageDataRow -> SOPInstanceUID, _TRUNCATE );	// *[2] Replaced strncat with strncpy_s.
					strncpy_s( FullImageFileSpecification, FILE_PATH_STRING_LENGTH, ImagePath, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
					strncat_s( FullImageFileSpecification, FILE_PATH_STRING_LENGTH, ImageFileName, _TRUNCATE );			// *[2] Replaced strcat with strncat_s.
					strncat_s( FullImageFileSpecification, FILE_PATH_STRING_LENGTH, ".png", _TRUNCATE );				// *[2] Replaced strcat with strncat_s.
					DeleteFile( FullImageFileSpecification );
					}
				pImageDataRow = pImageDataRow -> pNextDiagnosticImage;
				}
			pSeriesDataRow = pSeriesDataRow -> pNextDiagnosticSeries;
			}
		pStudyDataRow = pStudyDataRow -> pNextDiagnosticStudy;
		}	

	// Delete the data file for this study.
	strncpy_s( DataDirectory, FILE_PATH_STRING_LENGTH, BViewerConfiguration.DataDirectory, _TRUNCATE );	// *[2] Replaced strncat with strncpy_s.
	if ( DataDirectory[ strlen( DataDirectory ) - 1 ] != '\\' )
		strncat_s( DataDirectory, FILE_PATH_STRING_LENGTH, "\\", _TRUNCATE );					// *[2] Replaced strcat with strncat_s.
	// Check existence of path to configuration directory.
	strncpy_s( DataFileSpec, FULL_FILE_SPEC_STRING_LENGTH, DataDirectory, _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
	strncat_s( DataFileSpec, FULL_FILE_SPEC_STRING_LENGTH, m_PatientLastName, _TRUNCATE );		// *[1] Replaced strcat with strncat_s.
	strncat_s( DataFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "_", _TRUNCATE );					// *[1] Replaced strcat with strncat_s.
	strncat_s( DataFileSpec, FULL_FILE_SPEC_STRING_LENGTH, m_PatientFirstName, _TRUNCATE );		// *[1] Replaced strcat with strncat_s.
	strncat_s( DataFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "_", _TRUNCATE );					// *[1] Replaced strcat with strncat_s.
	strncat_s( DataFileSpec, FULL_FILE_SPEC_STRING_LENGTH, m_PatientID, _TRUNCATE );			// *[1] Replaced strcat with strncat_s.
	strncat_s( DataFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "_", _TRUNCATE );					// *[1] Replaced strcat with strncat_s.
	strncat_s( DataFileSpec, FULL_FILE_SPEC_STRING_LENGTH, m_AccessionNumber, _TRUNCATE );		// *[1] Replaced strcat with strncat_s.
	strncat_s( DataFileSpec, FULL_FILE_SPEC_STRING_LENGTH, ".sdy", _TRUNCATE );					// *[1] Replaced strcat with strncat_s.
	sprintf_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, "Deleting study data file %s.", DataFileSpec );	// *[1] Replaced sprintf with sprintf_s.
	LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );

	DeleteFile( DataFileSpec );
}


