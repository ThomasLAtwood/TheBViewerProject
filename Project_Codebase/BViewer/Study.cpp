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
	EraseStudyList();
	InitStudyCommentFields();
}


void CStudy::EraseStudyList()
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

	strcpy( m_ReaderAddressed, "" );
	strcpy( m_PatientLastName, "" );
	strcpy( m_PatientFirstName, "" );
	strcpy( m_PatientID, "" );
	memset( &m_PatientsBirthDate, '\0', sizeof( EDITED_DATE ) );
	strcpy( m_PatientsSex, "" );
	strcpy( m_PatientComments, "" );

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
	strcpy( m_TimeStudyFirstOpened, "" );
	strcpy( m_TimeReportApproved, "" );

	InitializeInterpretation();

	memset( &m_DateOfRadiograph, '\0', sizeof( EDITED_DATE ) );
	strcpy( m_OtherTypeOfReading, "" );
	strcpy( m_FacilityIDNumber, "" );
	memset( &m_DateOfReading, '\0', sizeof( EDITED_DATE ) );

	m_PhysicianNotificationStatus = 0L;
	memset( &m_Reserved2, '\0', sizeof( EDITED_DATE ) );

	memset( &m_ReaderInfo, '\0', sizeof( READER_PERSONAL_INFO ) );
	strcpy( m_AccessionNumber, "" );
	m_bStudyWasPreviouslyInterpreted = FALSE;
	m_SDYFileVersion = 2;
	m_pEventParameters = 0;
	strcpy( m_ReportPage1FilePath, "" );
	strcpy( m_ReportPage2FilePath, "" );
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
					sprintf( pStudyEditText -> EditText, "%2u/%2u/%4u", pDate -> wMonth, pDate -> wDay, pDate -> wYear );
				else
					strcpy( pStudyEditText -> EditText, "  /  /    " );
				break;
			case IDC_EDIT_READER_ID:
				if ( m_bStudyWasPreviouslyInterpreted )
					strcpy( pStudyEditText -> EditText, m_ReaderInfo.ID );
				else if ( pBViewerCustomization != 0 && strlen( pBViewerCustomization -> m_ReaderInfo.ID ) > 0 )
					strcpy( pStudyEditText -> EditText, pBViewerCustomization -> m_ReaderInfo.ID );
				else
					strcpy( pStudyEditText -> EditText, "         " );
				break;
			case IDC_EDIT_READER_INITIALS:
				if ( m_bStudyWasPreviouslyInterpreted )
					strcpy( pStudyEditText -> EditText, m_ReaderInfo.Initials );
				else if ( pBViewerCustomization != 0 && strlen( pBViewerCustomization -> m_ReaderInfo.Initials ) > 0 )
					strcpy( pStudyEditText -> EditText, pBViewerCustomization -> m_ReaderInfo.Initials );
				else
					strcpy( pStudyEditText -> EditText, "   " );
				break;
			case IDC_EDIT_DATE_OF_READING:

				pDate = &m_DateOfReading.Date;
				sprintf( pStudyEditText -> EditText, "%2u/%2u/%4u", pDate -> wMonth, pDate -> wDay, pDate -> wYear );
				break;
			case IDC_EDIT_READER_LAST_NAME:
				if ( m_bStudyWasPreviouslyInterpreted )
					strcpy( pStudyEditText -> EditText, m_ReaderInfo.LastName );
				else if ( pBViewerCustomization != 0 && strlen( pBViewerCustomization -> m_ReaderInfo.LastName ) > 0 )
					strcpy( pStudyEditText -> EditText, pBViewerCustomization -> m_ReaderInfo.LastName );
				else
					strcpy( pStudyEditText -> EditText, "                                " );
				break;
			case IDC_EDIT_READER_SIGNATURE_NAME:
				if ( m_bStudyWasPreviouslyInterpreted )
					strcpy( pStudyEditText -> EditText, m_ReaderInfo.ReportSignatureName );
				else if ( pBViewerCustomization != 0 && strlen( pBViewerCustomization -> m_ReaderInfo.ReportSignatureName ) > 0 )
					strcpy( pStudyEditText -> EditText, pBViewerCustomization -> m_ReaderInfo.ReportSignatureName );
				else
					strcpy( pStudyEditText -> EditText, "                                " );
				break;
			case IDC_EDIT_READER_STREET_ADDRESS:
				if ( m_bStudyWasPreviouslyInterpreted )
					strcpy( pStudyEditText -> EditText, m_ReaderInfo.StreetAddress );
				else if ( pBViewerCustomization != 0 && strlen( pBViewerCustomization -> m_ReaderInfo.StreetAddress ) > 0 )
					strcpy( pStudyEditText -> EditText, pBViewerCustomization -> m_ReaderInfo.StreetAddress );
				else
					strcpy( pStudyEditText -> EditText, "                                " );
				break;
			case IDC_EDIT_READER_CITY:
				if ( m_bStudyWasPreviouslyInterpreted )
					strcpy( pStudyEditText -> EditText, m_ReaderInfo.City );
				else if ( pBViewerCustomization != 0 && strlen( pBViewerCustomization -> m_ReaderInfo.City ) > 0 )
					strcpy( pStudyEditText -> EditText, pBViewerCustomization -> m_ReaderInfo.City );
				else
					strcpy( pStudyEditText -> EditText, "                      " );
				break;
			case IDC_EDIT_READER_STATE:
				if ( m_bStudyWasPreviouslyInterpreted )
					strcpy( pStudyEditText -> EditText, m_ReaderInfo.State );
				else if ( pBViewerCustomization != 0 && strlen( pBViewerCustomization -> m_ReaderInfo.State ) > 0 )
					strcpy( pStudyEditText -> EditText, pBViewerCustomization -> m_ReaderInfo.State );
				else
					strcpy( pStudyEditText -> EditText, "  " );
				break;
			case IDC_EDIT_READER_ZIPCODE:
				if ( m_bStudyWasPreviouslyInterpreted )
					strcpy( pStudyEditText -> EditText, m_ReaderInfo.ZipCode );
				else if ( pBViewerCustomization != 0 && strlen( pBViewerCustomization -> m_ReaderInfo.ZipCode ) > 0 )
					strcpy( pStudyEditText -> EditText, pBViewerCustomization -> m_ReaderInfo.ZipCode );
				else
					strcpy( pStudyEditText -> EditText, "     " );
				break;
			case IDC_EDIT_REPORT_PATIENT_NAME:
				if ( strlen( m_PatientLastName ) > 0 )
					{
					strcpy( pStudyEditText -> EditText, "" );
					strncat( pStudyEditText -> EditText, m_PatientLastName, MAX_EDIT_FIELD_SIZE - 1 );
					if ( strlen( m_PatientFirstName ) > 0 )
						{
						strncat( pStudyEditText -> EditText, ", ", MAX_EDIT_FIELD_SIZE - strlen( pStudyEditText -> EditText ) - 1 );
						strncat( pStudyEditText -> EditText, m_PatientFirstName, MAX_EDIT_FIELD_SIZE - strlen( pStudyEditText -> EditText ) - 1 );
						}
					}
				else
					strcpy( pStudyEditText -> EditText, "" );
				break;
			case IDC_EDIT_REPORT_DOB:
				pDate = &m_PatientsBirthDate.Date;
				if ( m_PatientsBirthDate.bDateHasBeenEdited )
					sprintf( pStudyEditText -> EditText, "%2u/%2u/%4u", pDate -> wMonth, pDate -> wDay, pDate -> wYear );
				else
					strcpy( pStudyEditText -> EditText, "01/01/1901" );
				break;
			case IDC_EDIT_REPORT_PATIENT_ID:
				if ( strlen( m_PatientID ) > 0 )
					strcpy( pStudyEditText -> EditText, m_PatientID );
				else
					strcpy( pStudyEditText -> EditText, "" );
				break;
			case IDC_EDIT_ORDERING_PHYSICIAN_NAME:
				if ( m_pDiagnosticStudyList != 0 && strlen( m_pDiagnosticStudyList -> ReferringPhysiciansName ) > 0 )
					strcpy( pStudyEditText -> EditText, m_pDiagnosticStudyList -> ReferringPhysiciansName );
				else if ( m_pDiagnosticStudyList != 0 && strlen( m_pDiagnosticStudyList -> ResponsibleOrganization ) > 0 )
					strcpy( pStudyEditText -> EditText, m_pDiagnosticStudyList -> ResponsibleOrganization );
				else
					strcpy( pStudyEditText -> EditText, "" );
				break;
			case IDC_EDIT_ORDERING_FACILITY:
				if ( m_pDiagnosticStudyList != 0 && strlen( m_pDiagnosticStudyList -> InstitutionName ) > 0 )
					strcpy( pStudyEditText -> EditText, m_pDiagnosticStudyList -> InstitutionName );
				else
					strcpy( pStudyEditText -> EditText, "" );
				break;
			case IDC_EDIT_CLASSIFICATION_PURPOSE:
				if ( strlen( m_PatientComments ) > 0 )
					strcpy( pStudyEditText -> EditText, m_PatientComments );
				else
					strcpy( pStudyEditText -> EditText, "" );
				break;
			case IDC_EDIT_TYPE_OF_READING_OTHER:
				if ( strlen( m_OtherTypeOfReading ) > 0 )
					strcpy( pStudyEditText -> EditText, m_OtherTypeOfReading );
				else
					strcpy( pStudyEditText -> EditText, "" );
				break;
			}
		nEdit++;
		pStudyEditText = &StudyEditTextArray[ nEdit ];
		}

	nEdit = 0;
	InitStudyCommentFields();
	pStudyCommentText = &StudyCommentTextArray[ nEdit ];
	while ( pStudyCommentText -> ResourceSymbol != 0 )
		{
		switch ( pStudyCommentText -> ResourceSymbol )
			{
			case IDC_EDIT_IMAGE_QUALITY_OTHER:
				if ( m_ImageDefectOtherText.GetLength() > 0 )
					{
					pStudyCommentText -> pEditText = (char*)malloc( m_ImageDefectOtherText.GetLength() + 1 );
					strcpy( pStudyCommentText -> pEditText, (const char*)m_ImageDefectOtherText );
					}
				else
					pStudyCommentText -> pEditText = 0;
				break;
			case IDC_EDIT_OTHER_COMMENTS:
				if ( m_OtherAbnormalitiesCommentsText.GetLength() > 0 )
					{
					pStudyCommentText -> pEditText = (char*)malloc( m_OtherAbnormalitiesCommentsText.GetLength() + 1 );
					strcpy( pStudyCommentText -> pEditText, (const char*)m_OtherAbnormalitiesCommentsText );
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
			strcpy( pTextField, pStudyEditText -> EditText );
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
		sscanf( pDicomTextDate, "%4d%2d%2d", (int*)&pSystemTime -> wYear, (int*)&pSystemTime -> wMonth, (int*)&pSystemTime -> wDay );
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
	if ( GetAbstractColumnValueForSpecifiedField( "DestinationAE", pTitleRow, pDataRow, ListItemText ) )
		{
		strcpy( m_ReaderAddressed, "" );
		strncat( m_ReaderAddressed, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
		}
	// Extract the patient last name and first name.
	bMatchingFieldFound = GetAbstractColumnValueForSpecifiedField( "PatientName", pTitleRow, pDataRow, ListItemText );
	if ( !bMatchingFieldFound )		// Check for older version of Dicom dictionary.
		bMatchingFieldFound = GetAbstractColumnValueForSpecifiedField( "PatientsName", pTitleRow, pDataRow, ListItemText );
	if ( bMatchingFieldFound )
		{
		pFieldText = ListItemText;
		pChar = strchr( pFieldText, '^' );
		if ( pChar != 0 )
			{
			*pChar = '\0';
			strcpy( m_PatientLastName, "" );
			strncat( m_PatientLastName, pFieldText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			pFieldText = pChar + 1;
			pChar = strchr( pFieldText, '^' );
			if ( pChar != 0 )
				*pChar = '\0';
			strcpy( m_PatientFirstName, "" );
			strncat( m_PatientFirstName, pFieldText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		else
			{
			strcpy( m_PatientLastName, "" );
			strncat( m_PatientLastName, pFieldText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		}
	if ( GetAbstractColumnValueForSpecifiedField( "PatientID", pTitleRow, pDataRow, ListItemText ) )
		{
		strcpy( m_PatientID, "" );
		strncat( m_PatientID, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
		}
	bMatchingFieldFound = GetAbstractColumnValueForSpecifiedField( "PatientsBirthDate", pTitleRow, pDataRow, ListItemText );
	if ( !bMatchingFieldFound )
		bMatchingFieldFound = GetAbstractColumnValueForSpecifiedField( "PatientBirthDate", pTitleRow, pDataRow, ListItemText );
	if ( bMatchingFieldFound )
		{
		ConvertDicomDAToSystemTime( ListItemText, &m_PatientsBirthDate.Date );
		if ( m_PatientsBirthDate.Date.wYear != 0 )
			m_PatientsBirthDate.bDateHasBeenEdited = TRUE;
		}
	bMatchingFieldFound = GetAbstractColumnValueForSpecifiedField( "PatientsSex", pTitleRow, pDataRow, ListItemText );
	if ( !bMatchingFieldFound )
		bMatchingFieldFound = GetAbstractColumnValueForSpecifiedField( "PatientSex", pTitleRow, pDataRow, ListItemText );
	if ( bMatchingFieldFound )
		{
		strcpy( m_PatientsSex, "" );
		strncat( m_PatientsSex, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
		}
	if ( GetAbstractColumnValueForSpecifiedField( "PatientComments", pTitleRow, pDataRow, ListItemText ) )
		{
		strcpy( m_PatientComments, "" );
		strncat( m_PatientComments, ListItemText, DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH - 1 );
		}
	if ( GetAbstractColumnValueForSpecifiedField( "AccessionNumber", pTitleRow, pDataRow, ListItemText ) )
		{
		strcpy( m_AccessionNumber, "" );
		strncat( m_AccessionNumber, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
		}
	if ( GetAbstractColumnValueForSpecifiedField( "StudyDate", pTitleRow, pDataRow, ListItemText ) )
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
		if ( GetAbstractColumnValueForSpecifiedField( "AccessionNumber", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticStudy -> AccessionNumber, "" );
			strncat( pNewDiagnosticStudy -> AccessionNumber, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "StudyDate", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticStudy -> StudyDate, "" );
			strncat( pNewDiagnosticStudy -> StudyDate, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "StudyTime", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticStudy -> StudyTime, "" );
			strncat( pNewDiagnosticStudy -> StudyTime, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		bMatchingFieldFound = GetAbstractColumnValueForSpecifiedField( "ReferringPhysiciansName", pTitleRow, pDataRow, ListItemText );
		if ( !bMatchingFieldFound )
			bMatchingFieldFound = GetAbstractColumnValueForSpecifiedField( "ReferringPhysicianName", pTitleRow, pDataRow, ListItemText );
		if ( bMatchingFieldFound )
			{
			strcpy( pNewDiagnosticStudy -> ReferringPhysiciansName, "" );
			strncat( pNewDiagnosticStudy -> ReferringPhysiciansName, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		bMatchingFieldFound = GetAbstractColumnValueForSpecifiedField( "ReferringPhysiciansTelephoneNumbers", pTitleRow, pDataRow, ListItemText );
		if ( !bMatchingFieldFound )
			bMatchingFieldFound = GetAbstractColumnValueForSpecifiedField( "ReferringPhysicianTelephoneNumbers", pTitleRow, pDataRow, ListItemText );
		if ( bMatchingFieldFound )
			{
			strcpy( pNewDiagnosticStudy -> ReferringPhysiciansPhone, "" );
			strncat( pNewDiagnosticStudy -> ReferringPhysiciansPhone, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "ResponsibleOrganization", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticStudy -> ResponsibleOrganization, "" );
			strncat( pNewDiagnosticStudy -> ResponsibleOrganization, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "InstitutionName", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticStudy -> InstitutionName, "" );
			strncat( pNewDiagnosticStudy -> InstitutionName, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "StudyID", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticStudy -> StudyID, "" );
			strncat( pNewDiagnosticStudy -> StudyID, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "StudyDescription", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticStudy -> StudyDescription, "" );
			strncat( pNewDiagnosticStudy -> StudyDescription, ListItemText, DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "StudyInstanceUID", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticStudy -> StudyInstanceUID, "" );
			strncat( pNewDiagnosticStudy -> StudyInstanceUID, ListItemText, DICOM_ATTRIBUTE_UI_STRING_LENGTH - 1 );
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
		if ( GetAbstractColumnValueForSpecifiedField( "Modality", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticSeries -> Modality, "" );
			strncat( pNewDiagnosticSeries -> Modality, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "SeriesNumber", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticSeries -> SeriesNumber, "" );
			strncat( pNewDiagnosticSeries -> SeriesNumber, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "Laterality", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticSeries -> Laterality, "" );
			strncat( pNewDiagnosticSeries -> Laterality, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "SeriesDate", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticSeries -> SeriesDate, "" );
			strncat( pNewDiagnosticSeries -> SeriesDate, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "SeriesTime", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticSeries -> SeriesTime, "" );
			strncat( pNewDiagnosticSeries -> SeriesTime, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "ProtocolName", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticSeries -> ProtocolName, "" );
			strncat( pNewDiagnosticSeries -> ProtocolName, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "SeriesDescription", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticSeries -> SeriesDescription, "" );
			strncat( pNewDiagnosticSeries -> SeriesDescription, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "BodyPartExamined", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticSeries -> BodyPartExamined, "" );
			strncat( pNewDiagnosticSeries -> BodyPartExamined, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "PatientPosition", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticSeries -> PatientPosition, "" );
			strncat( pNewDiagnosticSeries -> PatientPosition, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "PatientOrientation", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticSeries -> PatientOrientation, "" );
			strncat( pNewDiagnosticSeries -> PatientOrientation, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "SeriesInstanceUID", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticSeries -> SeriesInstanceUID, "" );
			strncat( pNewDiagnosticSeries -> SeriesInstanceUID, ListItemText, DICOM_ATTRIBUTE_UI_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "Manufacturer", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticSeries -> Manufacturer, "" );
			strncat( pNewDiagnosticSeries -> Manufacturer, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
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
		if ( GetAbstractColumnValueForSpecifiedField( "ImageType", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticImage -> ImageType, "" );
			strncat( pNewDiagnosticImage -> ImageType, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "InstanceNumber", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticImage -> InstanceNumber, "" );
			strncat( pNewDiagnosticImage -> InstanceNumber, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "InstanceCreationDate", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticImage -> InstanceCreationDate, "" );
			strncat( pNewDiagnosticImage -> InstanceCreationDate, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "InstanceCreationTime", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticImage -> InstanceCreationTime, "" );
			strncat( pNewDiagnosticImage -> InstanceCreationTime, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "ContentDate", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticImage -> ContentDate, "" );
			strncat( pNewDiagnosticImage -> ContentDate, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "ContentTime", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticImage -> ContentTime, "" );
			strncat( pNewDiagnosticImage -> ContentTime, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "AcquisitionNumber", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticImage -> AcquisitionNumber, "" );
			strncat( pNewDiagnosticImage -> AcquisitionNumber, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "AcquisitionDate", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticImage -> AcquisitionDate, "" );
			strncat( pNewDiagnosticImage -> AcquisitionDate, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "AcquisitionTime", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticImage -> AcquisitionTime, "" );
			strncat( pNewDiagnosticImage -> AcquisitionTime, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "SamplesPerPixel", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticImage -> SamplesPerPixel, "" );
			strncat( pNewDiagnosticImage -> SamplesPerPixel, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "PhotometricInterpretation", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticImage -> PhotometricInterpretation, "" );
			strncat( pNewDiagnosticImage -> PhotometricInterpretation, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "Rows", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticImage -> Rows, "" );
			strncat( pNewDiagnosticImage -> Rows, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "Columns", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticImage -> Columns, "" );
			strncat( pNewDiagnosticImage -> Columns, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "PixelAspectRatio", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticImage -> PixelAspectRatio, "" );
			strncat( pNewDiagnosticImage -> PixelAspectRatio, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "BitsAllocated", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticImage -> BitsAllocated, "" );
			strncat( pNewDiagnosticImage -> BitsAllocated, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "BitsStored", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticImage -> BitsStored, "" );
			strncat( pNewDiagnosticImage -> BitsStored, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "HighBit", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticImage -> HighBit, "" );
			strncat( pNewDiagnosticImage -> HighBit, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "PixelRepresentation", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticImage -> PixelRepresentation, "" );
			strncat( pNewDiagnosticImage -> PixelRepresentation, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "WindowCenter", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticImage -> WindowCenter, "" );
			strncat( pNewDiagnosticImage -> WindowCenter, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "WindowWidth", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticImage -> WindowWidth, "" );
			strncat( pNewDiagnosticImage -> WindowWidth, ListItemText, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
			}
		if ( GetAbstractColumnValueForSpecifiedField( "SOPInstanceUID", pTitleRow, pDataRow, ListItemText ) )
			{
			strcpy( pNewDiagnosticImage -> SOPInstanceUID, "" );
			strncat( pNewDiagnosticImage -> SOPInstanceUID, ListItemText, DICOM_ATTRIBUTE_UI_STRING_LENGTH - 1 );
			}
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
	DIAGNOSTIC_STUDY		*pNewDiagnosticStudy;
	DIAGNOSTIC_STUDY		*pExistingDiagnosticStudy;
	DIAGNOSTIC_SERIES		*pNewDiagnosticSeries;
	DIAGNOSTIC_SERIES		*pExistingDiagnosticSeries;
	DIAGNOSTIC_IMAGE		*pNewDiagnosticImage;
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
		bNoError = pExistingStudy -> Save();
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

	sprintf( pDateString, "%02d%02d%04d",
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


void CStudy::GetStudyFileName( char *pStudyFileName )
{
	strcpy( pStudyFileName, m_PatientLastName );
	strcat( pStudyFileName, "_" );
	strcat( pStudyFileName, m_PatientFirstName );
	strcat( pStudyFileName, "_" );
	strcat( pStudyFileName, m_PatientID );
	strcat( pStudyFileName, "_" );
	if ( strlen( m_AccessionNumber ) == 0 )
		strcat( m_AccessionNumber, this -> m_pDiagnosticStudyList -> AccessionNumber );
	strcat( pStudyFileName, m_AccessionNumber );
	strcat( pStudyFileName, ".sdy" );
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
	unsigned long		LengthInBytes;
	DIAGNOSTIC_STUDY	*pDiagnosticStudy;
	unsigned long		nStudies;
	char				Msg[ FULL_FILE_SPEC_STRING_LENGTH ];

	bFileWrittenSuccessfully = FALSE;
	strcpy( DataDirectory, "" );
	strncat( DataDirectory, BViewerConfiguration.DataDirectory, FILE_PATH_STRING_LENGTH );
	if ( DataDirectory[ strlen( DataDirectory ) - 1 ] != '\\' )
		strcat( DataDirectory, "\\" );
	// Check existence of path to configuration directory.
	bNoError = SetCurrentDirectory( DataDirectory );
	if ( !bNoError )
		bNoError = CreateDirectory( DataDirectory, NULL );
	if ( bNoError )
		{
		strcpy( FileSpec, DataDirectory );
		GetStudyFileName( &FileSpec[ strlen( FileSpec ) ] );
		sprintf( Msg, "Saving study data to file %s.", FileSpec );
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
		nBytesRead = fread( pDiagnosticImage, 1, nBytesToRead, pStudyFile );
		bNoError = ( nBytesRead == nBytesToRead );
		pDiagnosticImage -> pNextDiagnosticImage = 0;
		}
	if ( bNoError )
		*ppDiagnosticImage = pDiagnosticImage;
	else
		*ppDiagnosticImage = 0;

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
		nBytesRead = fread( pDiagnosticSeries, 1, nBytesToRead, pStudyFile );
		bNoError = ( nBytesRead == nBytesToRead );
		pDiagnosticSeries -> pNextDiagnosticSeries = 0;
		if ( bOlderSeriesWithoutManufacturer )
			strcpy( pDiagnosticSeries -> Manufacturer, "" );
		}
	if ( bNoError )
		{
		// Read the number of images for the current study.
		nBytesToRead = sizeof(unsigned long);
		nBytesRead = fread( &nImageCount, 1, nBytesToRead, pStudyFile );
		bNoError = ( nBytesRead == nBytesToRead );
		}
	if ( bNoError )
		{
		// Read the length of the image structure.
		nBytesToRead = sizeof(unsigned long);
		nBytesRead = fread( &LengthInBytes, 1, nBytesToRead, pStudyFile );
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
		*ppDiagnosticSeries = 0;

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
		nBytesRead = fread( pDiagnosticStudy, 1, nBytesToRead, pStudyFile );
		bNoError = ( nBytesRead == nBytesToRead );
		pDiagnosticStudy -> pNextDiagnosticStudy = 0;
		}
	if ( bNoError )
		{
		strcpy( m_AccessionNumber, "" );
		strncat( m_AccessionNumber, pDiagnosticStudy -> AccessionNumber, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
		}
	if ( bNoError )
		{
		// Read the number of series recorded for this study.
		nBytesToRead = sizeof(unsigned long);
		nBytesRead = fread( &nSeriesCount, 1, nBytesToRead, pStudyFile );
		bNoError = ( nBytesRead == nBytesToRead );
		}
	if ( bNoError )
		{
		// Read the length of the series structure.
		nBytesToRead = sizeof(unsigned long);
		nBytesRead = fread( &LengthInBytes, 1, nBytesToRead, pStudyFile );
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
		*ppDiagnosticStudy = 0;

	return bNoError;
}


BOOL CStudy::Restore( char *pFullFilePath )
{
	BOOL				bNoError = TRUE;
	BOOL				bFileReadSuccessfully;
	FILE				*pStudyFile;
	size_t				nBytesToRead;
	size_t				nBytesRead;
	unsigned long		LengthInBytes;
	DIAGNOSTIC_STUDY	**ppDiagnosticStudy;
	unsigned long		nStudyCount;
	unsigned long		nStudy;
	char				*pTextBuffer;
	char				Msg[ 256 ];

	bFileReadSuccessfully = FALSE;
	pStudyFile = fopen( pFullFilePath, "rb" );
	if ( pStudyFile != 0 )
		{
		nBytesToRead = DICOM_ATTRIBUTE_STRING_LENGTH;
		nBytesRead = fread( m_ReaderAddressed, 1, nBytesToRead, pStudyFile );
		bNoError = ( nBytesRead == nBytesToRead );
		if ( bNoError )
			{
			nBytesRead = fread( m_PatientLastName, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( m_PatientFirstName, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( m_PatientID, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(EDITED_DATE);
			nBytesRead = fread( &m_PatientsBirthDate, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = 4;
			nBytesRead = fread( m_PatientsSex, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH;
			nBytesRead = fread( m_PatientComments, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(double);
			nBytesRead = fread( &m_Reserved, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( &m_Reserved, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( &m_GammaSetting, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( &m_WindowCenter, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( &m_WindowWidth, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( &m_MaxGrayscaleValue, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = 32;
			nBytesRead = fread( m_TimeStudyFirstOpened, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( m_TimeReportApproved, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(UINT);
			nBytesRead = fread( &m_nCurrentObjectID, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(BOOL);
			nBytesRead = fread( &m_bImageQualityVisited, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( &m_bParenchymalAbnormalitiesVisited, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( &m_bPleuralAbnormalitiesVisited, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( &m_bOtherAbnormalitiesVisited, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(char);
			nBytesRead = fread( &m_AnyParenchymalAbnormalities, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( &m_AnyPleuralAbnormalities, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( &m_AnyOtherAbnormalities, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(unsigned long);
			nBytesRead = fread( &m_ImageQuality, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( &m_ObservedParenchymalAbnormalities, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(unsigned short);
			nBytesRead = fread( &m_ObservedPleuralPlaqueSites, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( &m_ObservedPleuralCalcificationSites, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( &m_ObservedPlaqueExtent, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( &m_ObservedPlaqueWidth, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( &m_ObservedCostophrenicAngleObliteration, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( &m_ObservedPleuralThickeningSites, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( &m_ObservedThickeningCalcificationSites, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( &m_ObservedThickeningExtent, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( &m_ObservedThickeningWidth, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(unsigned long);
			nBytesRead = fread( &m_ObservedOtherSymbols, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( &m_ObservedOtherAbnormalities, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(unsigned long);
			nBytesRead = fread( &LengthInBytes, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead && LengthInBytes < DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH );
			if ( bNoError )
				{
				pTextBuffer = (char*)malloc( LengthInBytes + 1 );
				bNoError = ( pTextBuffer != 0 );
				}
			if ( bNoError )
				{
				nBytesToRead = LengthInBytes;
				nBytesRead = fread( pTextBuffer, 1, nBytesToRead, pStudyFile );
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
			nBytesRead = fread( &LengthInBytes, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead && LengthInBytes < DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH );
			if ( bNoError )
				{
				pTextBuffer = (char*)malloc( LengthInBytes + 1 );
				bNoError = ( pTextBuffer != 0 );
				}
			if ( bNoError )
				{
				nBytesToRead = LengthInBytes;
				nBytesRead = fread( pTextBuffer, 1, nBytesToRead, pStudyFile );
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
			nBytesRead = fread( &m_PhysicianNotificationStatus, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(EDITED_DATE);
			nBytesRead = fread( &m_Reserved2, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(EDITED_DATE);
			nBytesRead = fread( &m_DateOfRadiograph, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}

		if ( bNoError )
			{
			nBytesToRead = sizeof(unsigned long);
			nBytesRead = fread( &LengthInBytes, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead && LengthInBytes < DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH );
			if ( bNoError )
				{
				pTextBuffer = (char*)malloc( LengthInBytes + 1 );
				bNoError = ( pTextBuffer != 0 );
				}
			if ( bNoError )
				{
				nBytesToRead = LengthInBytes;
				nBytesRead = fread( pTextBuffer, 1, nBytesToRead, pStudyFile );
				bNoError = ( nBytesRead == nBytesToRead );
				if ( bNoError )
					strcpy( m_Reserved1, pTextBuffer );
				free( pTextBuffer );
				}
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(unsigned short);
			nBytesRead = fread( &m_TypeOfReading, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(unsigned long);
			nBytesRead = fread( &LengthInBytes, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead && LengthInBytes < DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH );
			if ( bNoError )
				{
				pTextBuffer = (char*)malloc( LengthInBytes + 1 );
				bNoError = ( pTextBuffer != 0 );
				}
			if ( bNoError )
				{
				nBytesToRead = LengthInBytes;
				nBytesRead = fread( pTextBuffer, 1, nBytesToRead, pStudyFile );
				bNoError = ( nBytesRead == nBytesToRead );
				if ( bNoError )
					{
					strcpy( m_OtherTypeOfReading, "" );
					strncat( m_OtherTypeOfReading, pTextBuffer, DICOM_ATTRIBUTE_STRING_LENGTH - 1 );
					}
				free( pTextBuffer );
				}
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(unsigned long);
			nBytesRead = fread( &LengthInBytes, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead && LengthInBytes < DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH );
			if ( bNoError )
				{
				pTextBuffer = (char*)malloc( LengthInBytes + 1 );
				bNoError = ( pTextBuffer != 0 );
				}
			if ( bNoError )
				{
				nBytesToRead = LengthInBytes;
				nBytesRead = fread( pTextBuffer, 1, nBytesToRead, pStudyFile );
				bNoError = ( nBytesRead == nBytesToRead );
				if ( bNoError )
					{
					strcpy( m_FacilityIDNumber, "" );
					strncat( m_FacilityIDNumber, pTextBuffer, 9 );
					}
				free( pTextBuffer );
				}
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(EDITED_DATE);
			nBytesRead = fread( &m_DateOfReading, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}

		if ( bNoError )
			{
			nBytesToRead = sizeof(BOOL);
			nBytesRead = fread( &m_bReportViewed, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesToRead = sizeof(BOOL);
			nBytesRead = fread( &m_bReportApproved, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			// Read the number of studies recorded for this patient.
			nBytesToRead = sizeof(unsigned long);
			nBytesRead = fread( &nStudyCount, 1, nBytesToRead, pStudyFile );
			bNoError = ( nBytesRead == nBytesToRead );
			if ( bNoError )
				{
				// Read the length of the study structure.
				nBytesToRead = sizeof(unsigned long);
				nBytesRead = fread( &LengthInBytes, 1, nBytesToRead, pStudyFile );
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
				nBytesRead = fread( &m_SDYFileVersion, 1, nBytesToRead, pStudyFile );
				if ( nBytesRead == 0 )
					m_SDYFileVersion = 0;
				if ( m_SDYFileVersion >= 1 )
					{
					nBytesToRead = sizeof(READER_PERSONAL_INFO);
					nBytesRead = fread( &m_ReaderInfo, 1, nBytesToRead, pStudyFile );
					bNoError = ( nBytesRead == nBytesToRead );
					if ( bNoError )
						{
						nBytesToRead = sizeof(BOOL);
						nBytesRead = fread( &m_bStudyWasPreviouslyInterpreted, 1, nBytesToRead, pStudyFile );
						bNoError = ( nBytesRead == nBytesToRead );
						}
					}
				if ( m_SDYFileVersion >= 2 )
					{
					nBytesToRead = sizeof(CLIENT_INFO);
					nBytesRead = fread( &m_ClientInfo, 1, nBytesToRead, pStudyFile );
					bNoError = ( nBytesRead == nBytesToRead );
					}
				}
			if ( !bNoError )
				{
				sprintf( Msg, "Unable to restore study file for %s", m_PatientLastName );
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

	strcpy( ImagePath, "" );
	strncat( ImagePath, BViewerConfiguration.ImageDirectory, FULL_FILE_SPEC_STRING_LENGTH );
	if ( ImagePath[ strlen( ImagePath ) - 1 ] != '\\' )
		strcat( ImagePath, "\\" );

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
					strcpy( ImageFileName, "" );
					strncat( ImageFileName, pImageDataRow -> SOPInstanceUID,
											DICOM_ATTRIBUTE_UI_STRING_LENGTH - 1 );
					strcpy( FullImageFileSpecification, ImagePath );
					strcat( FullImageFileSpecification, ImageFileName );
					strcat( FullImageFileSpecification, ".png" );
					DeleteFile( FullImageFileSpecification );
					}
				pImageDataRow = pImageDataRow -> pNextDiagnosticImage;
				}
			pSeriesDataRow = pSeriesDataRow -> pNextDiagnosticSeries;
			}
		pStudyDataRow = pStudyDataRow -> pNextDiagnosticStudy;
		}	

	// Delete the data file for this study.
	strcpy( DataDirectory, "" );
	strncat( DataDirectory, BViewerConfiguration.DataDirectory, FILE_PATH_STRING_LENGTH );
	if ( DataDirectory[ strlen( DataDirectory ) - 1 ] != '\\' )
		strcat( DataDirectory, "\\" );
	// Check existence of path to configuration directory.
	strcpy( DataFileSpec, DataDirectory );
	strcat( DataFileSpec, m_PatientLastName );
	strcat( DataFileSpec, "_" );
	strcat( DataFileSpec, m_PatientFirstName );
	strcat( DataFileSpec, "_" );
	strcat( DataFileSpec, m_PatientID );
	strcat( DataFileSpec, "_" );
	strcat( DataFileSpec, m_AccessionNumber );
	strcat( DataFileSpec, ".sdy" );
	sprintf( Msg, "Deleting study data file %s.", DataFileSpec );
	LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );

	DeleteFile( DataFileSpec );
}


