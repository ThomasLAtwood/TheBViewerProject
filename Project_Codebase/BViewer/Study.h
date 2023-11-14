// Study.h : Header file defining the structure of the CStudy class, which
//  implements a repository for all the analysis and related data associated
//  with a subject study.
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

#include "Module.h"
#include "Configuration.h"
#include "DicomDictionary.h"
#include "Abstract.h"
#include "Client.h"


typedef struct _DiagnosticImage
	{
	char						ImageType[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						InstanceNumber[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						InstanceCreationDate[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						InstanceCreationTime[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						ContentDate[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						ContentTime[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						AcquisitionNumber[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						AcquisitionDate[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						AcquisitionTime[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						SamplesPerPixel[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						PhotometricInterpretation[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						Rows[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						Columns[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						PixelAspectRatio[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						BitsAllocated[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						BitsStored[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						HighBit[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						PixelRepresentation[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						WindowCenter[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						WindowWidth[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						SOPInstanceUID[ DICOM_ATTRIBUTE_UI_STRING_LENGTH ];
	int							Reserved;
	struct _DiagnosticImage		*pNextDiagnosticImage;
	} DIAGNOSTIC_IMAGE;


typedef struct _DiagnosticSeries
	{
	char						Modality[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						SeriesNumber[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						Laterality[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						SeriesDate[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						SeriesTime[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						ProtocolName[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						SeriesDescription[ DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH ];
	char						BodyPartExamined[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						PatientPosition[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						PatientOrientation[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						SeriesInstanceUID[ DICOM_ATTRIBUTE_UI_STRING_LENGTH ];
	char						Manufacturer[ DICOM_ATTRIBUTE_UI_STRING_LENGTH ];
	DIAGNOSTIC_IMAGE			*pDiagnosticImageList;
	struct _DiagnosticSeries	*pNextDiagnosticSeries;
	} DIAGNOSTIC_SERIES;


typedef struct _DiagnosticStudy
	{
	char						AccessionNumber[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						StudyDate[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						StudyTime[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						ReferringPhysiciansName[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						ReferringPhysiciansPhone[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						ResponsibleOrganization[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						InstitutionName[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						StudyID[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char						StudyDescription[ DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH ];
	char						StudyInstanceUID[ DICOM_ATTRIBUTE_UI_STRING_LENGTH ];
	DIAGNOSTIC_SERIES			*pDiagnosticSeriesList;
	struct _DiagnosticStudy		*pNextDiagnosticStudy;
	} DIAGNOSTIC_STUDY;


typedef struct
	{
	SYSTEMTIME		Date;
	BOOL			bDateHasBeenEdited;
	} EDITED_DATE;


class CStudy
{
public:
	CStudy();
	virtual ~CStudy( void );

	UINT			m_nCurrentObjectID;			// Button ID for the currently active analysis screen.

	// Data from abstracts.
	char				m_ReaderAddressed[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char				m_PatientLastName[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char				m_PatientFirstName[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char				m_PatientID[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	EDITED_DATE			m_PatientsBirthDate;
	char				m_PatientsSex[ 4 ];
	char				m_PatientComments[ DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH ];
	DIAGNOSTIC_STUDY	*m_pDiagnosticStudyList;
	BOOL				m_bStudyHasBeenEdited;	// This member is not recorded, since a recorded study will always have it TRUE.
	DIAGNOSTIC_STUDY	*m_pCurrentStudyInfo;
	DIAGNOSTIC_SERIES	*m_pCurrentSeriesInfo;
	DIAGNOSTIC_IMAGE	*m_pCurrentImageInfo;
	
	double				m_Reserved;

	double				m_GammaSetting;			// Ranges from 0.2 to 10.0;
	double				m_WindowCenter;
	double				m_WindowWidth;
	double				m_MaxGrayscaleValue;
	char				m_TimeStudyFirstOpened[ 32 ];
	char				m_TimeReportApproved[ 32 ];

	// Data from interpretation.
	BOOL			m_bImageQualityVisited;
		unsigned long	m_ImageQuality;
						#define IMAGE_QUALITY_UNSPECIFIED		0x00000000
						#define IMAGE_GRADE_1					0x00000001
						#define IMAGE_GRADE_2					0x00000002
						#define IMAGE_GRADE_3					0x00000004
						#define IMAGE_GRADE_UR					0x00000008
						#define IMAGE_DEFECT_OVEREXPOSED		0x00000010
						#define IMAGE_DEFECT_UNDEREXPOSED		0x00000020
						#define IMAGE_DEFECT_ARTIFACTS			0x00000040
						#define IMAGE_DEFECT_POSITION			0x00000080
						#define IMAGE_DEFECT_CONTRAST			0x00000100
						#define IMAGE_DEFECT_PROCESSING			0x00000200
						#define IMAGE_DEFECT_UNDERINFLATION		0x00000400
						#define IMAGE_DEFECT_MOTTLE				0x00000800
						#define IMAGE_DEFECT_OTHER				0x00001000
						#define IMAGE_DEFECT_EXCESSIVE_EDGE		0x00002000
						#define IMAGE_DEFECT_SCAPULA_OVERLAY	0X00004000
						
						#define IMAGE_GRADE_MASK				0x0000000f
		CString			m_ImageDefectOtherText;

	BOOL			m_bParenchymalAbnormalitiesVisited;
		char			m_AnyParenchymalAbnormalities;
							#define BOOL_NOT_SPECIFIED		2
							#define BOOL_NO					0
							#define BOOL_YES				1
		unsigned long	m_ObservedParenchymalAbnormalities;
							#define SHAPE_SIZE_PRIMARY_P		0x00000001
							#define SHAPE_SIZE_PRIMARY_Q		0x00000002
							#define SHAPE_SIZE_PRIMARY_R		0x00000003
							#define SHAPE_SIZE_PRIMARY_S		0x00000004
							#define SHAPE_SIZE_PRIMARY_T		0x00000005
							#define SHAPE_SIZE_PRIMARY_U		0x00000006
							#define SHAPE_SIZE_PRIMARY_MASK		0x0000000f

							#define SHAPE_SIZE_SECONDARY_P		0x00000010
							#define SHAPE_SIZE_SECONDARY_Q		0x00000020
							#define SHAPE_SIZE_SECONDARY_R		0x00000030
							#define SHAPE_SIZE_SECONDARY_S		0x00000040
							#define SHAPE_SIZE_SECONDARY_T		0x00000050
							#define SHAPE_SIZE_SECONDARY_U		0x00000060
							#define SHAPE_SIZE_SECONDARY_MASK	0x000000f0
							
							#define OPACITY_ZONE_UPPER_RIGHT	0x00000100
							#define OPACITY_ZONE_UPPER_LEFT		0x00000200
							#define OPACITY_ZONE_MIDDLE_RIGHT	0x00000400
							#define OPACITY_ZONE_MIDDLE_LEFT	0x00000800
							#define OPACITY_ZONE_LOWER_RIGHT	0x00001000
							#define OPACITY_ZONE_LOWER_LEFT		0x00002000
							#define OPACITY_ZONE_MASK			0x00003f00
							
							#define PROFUSION_0MINUS			0x00010000
							#define PROFUSION_00				0x00020000
							#define PROFUSION_01				0x00030000
							#define PROFUSION_10				0x00040000
							#define PROFUSION_11				0x00050000
							#define PROFUSION_12				0x00060000
							#define PROFUSION_21				0x00070000
							#define PROFUSION_22				0x00080000
							#define PROFUSION_23				0x00090000
							#define PROFUSION_32				0x000a0000
							#define PROFUSION_33				0x000b0000
							#define PROFUSION_3PLUS				0x000c0000
							#define PROFUSION_MASK				0x000f0000
							
							#define LARGE_OPACITY_SIZE_0		0x00100000
							#define LARGE_OPACITY_SIZE_A		0x00200000
							#define LARGE_OPACITY_SIZE_B		0x00300000
							#define LARGE_OPACITY_SIZE_C		0x00400000
							#define LARGE_OPACITY_SIZE_MASK		0x00700000

	BOOL			m_bPleuralAbnormalitiesVisited;
		char			m_AnyPleuralAbnormalities;
		unsigned short	m_ObservedPleuralPlaqueSites;
							#define PLAQUES_CHEST_WALL_PROFILE_NONE			0x0001
							#define PLAQUES_CHEST_WALL_PROFILE_RIGHT		0x0002
							#define PLAQUES_CHEST_WALL_PROFILE_LEFT			0x0004
							#define PLAQUES_CHEST_WALL_FACE_ON_NONE			0x0010
							#define PLAQUES_CHEST_WALL_FACE_ON_RIGHT		0x0020
							#define PLAQUES_CHEST_WALL_FACE_ON_LEFT			0x0040
							#define PLAQUES_DIAPHRAGM_NONE					0x0100
							#define PLAQUES_DIAPHRAGM_RIGHT					0x0200
							#define PLAQUES_DIAPHRAGM_LEFT					0x0400
							#define PLAQUES_OTHER_SITES_NONE				0x1000
							#define PLAQUES_OTHER_SITES_RIGHT				0x2000
							#define PLAQUES_OTHER_SITES_LEFT				0x4000
		unsigned short	m_ObservedPleuralCalcificationSites;
							#define CALCIFICATION_CHEST_WALL_PROFILE_NONE	0x0001
							#define CALCIFICATION_CHEST_WALL_PROFILE_RIGHT	0x0002
							#define CALCIFICATION_CHEST_WALL_PROFILE_LEFT	0x0004
							#define	CALCIFICATION_CHEST_WALL_FACE_ON_NONE	0x0010
							#define	CALCIFICATION_CHEST_WALL_FACE_ON_RIGHT	0x0020
							#define CALCIFICATION_CHEST_WALL_FACE_ON_LEFT	0x0040
							#define CALCIFICATION_DIAPHRAGM_NONE			0x0100
							#define CALCIFICATION_DIAPHRAGM_RIGHT			0x0200
							#define CALCIFICATION_DIAPHRAGM_LEFT			0x0400
							#define CALCIFICATION_OTHER_SITES_NONE			0x1000
							#define CALCIFICATION_OTHER_SITES_RIGHT			0x2000
							#define CALCIFICATION_OTHER_SITES_LEFT			0x4000
		unsigned short	m_ObservedPlaqueExtent;
							#define PLAQUE_EXTENT_NONE_ON_RIGHT				0x0001
							#define PLAQUE_EXTENT_RIGHT						0x0002
							#define PLAQUE_EXTENT_RIGHT_SIZE1				0x0010
							#define PLAQUE_EXTENT_RIGHT_SIZE2				0x0020
							#define PLAQUE_EXTENT_RIGHT_SIZE3				0x0040
							#define PLAQUE_EXTENT_NONE_ON_LEFT				0x0100
							#define PLAQUE_EXTENT_LEFT						0x0200
							#define PLAQUE_EXTENT_LEFT_SIZE1				0x1000
							#define PLAQUE_EXTENT_LEFT_SIZE2				0x2000
							#define PLAQUE_EXTENT_LEFT_SIZE3				0x4000
		unsigned short	m_ObservedPlaqueWidth;
							#define PLAQUE_WIDTH_NONE_ON_RIGHT				0x0001
							#define PLAQUE_WIDTH_RIGHT						0x0002
							#define PLAQUE_WIDTH_RIGHT_SIZE1				0x0010
							#define PLAQUE_WIDTH_RIGHT_SIZE2				0x0020
							#define PLAQUE_WIDTH_RIGHT_SIZE3				0x0040
							#define PLAQUE_WIDTH_NONE_ON_LEFT				0x0100
							#define PLAQUE_WIDTH_LEFT						0x0200
							#define PLAQUE_WIDTH_LEFT_SIZE1					0x1000
							#define PLAQUE_WIDTH_LEFT_SIZE2					0x2000
							#define PLAQUE_WIDTH_LEFT_SIZE3					0x4000

		unsigned short	m_ObservedCostophrenicAngleObliteration;
							#define COSTOPHRENIC_ANGLE_OBLITERATION_NONE	0x0001
							#define COSTOPHRENIC_ANGLE_OBLITERATION_RIGHT	0x0002
							#define COSTOPHRENIC_ANGLE_OBLITERATION_LEFT	0x0004
		unsigned short	m_ObservedPleuralThickeningSites;
							#define THICKENING_CHEST_WALL_PROFILE_NONE		0x0001
							#define THICKENING_CHEST_WALL_PROFILE_RIGHT		0x0002
							#define THICKENING_CHEST_WALL_PROFILE_LEFT		0x0004
							#define THICKENING_CHEST_WALL_FACE_ON_NONE		0x0010
							#define THICKENING_CHEST_WALL_FACE_ON_RIGHT		0x0020
							#define THICKENING_CHEST_WALL_FACE_ON_LEFT		0x0040
		unsigned short	m_ObservedThickeningCalcificationSites;
							#define THICK_CALCIFICATION_CHEST_WALL_PROFILE_NONE		0x0001
							#define THICK_CALCIFICATION_CHEST_WALL_PROFILE_RIGHT	0x0002
							#define THICK_CALCIFICATION_CHEST_WALL_PROFILE_LEFT		0x0004
							#define	THICK_CALCIFICATION_CHEST_WALL_FACE_ON_NONE		0x0010
							#define	THICK_CALCIFICATION_CHEST_WALL_FACE_ON_RIGHT	0x0020
							#define THICK_CALCIFICATION_CHEST_WALL_FACE_ON_LEFT		0x0040
		unsigned short	m_ObservedThickeningExtent;
							#define THICKENING_EXTENT_NONE_ON_RIGHT			0x0001
							#define THICKENING_EXTENT_RIGHT					0x0002
							#define THICKENING_EXTENT_RIGHT_SIZE1			0x0010
							#define THICKENING_EXTENT_RIGHT_SIZE2			0x0020
							#define THICKENING_EXTENT_RIGHT_SIZE3			0x0040
							#define THICKENING_EXTENT_NONE_ON_LEFT			0x0100
							#define THICKENING_EXTENT_LEFT					0x0200
							#define THICKENING_EXTENT_LEFT_SIZE1			0x1000
							#define THICKENING_EXTENT_LEFT_SIZE2			0x2000
							#define THICKENING_EXTENT_LEFT_SIZE3			0x4000
		unsigned short	m_ObservedThickeningWidth;
							#define THICKENING_WIDTH_NONE_ON_RIGHT			0x0001
							#define THICKENING_WIDTH_RIGHT					0x0002
							#define THICKENING_WIDTH_RIGHT_SIZE1			0x0010
							#define THICKENING_WIDTH_RIGHT_SIZE2			0x0020
							#define THICKENING_WIDTH_RIGHT_SIZE3			0x0040
							#define THICKENING_WIDTH_NONE_ON_LEFT			0x0100
							#define THICKENING_WIDTH_LEFT					0x0200
							#define THICKENING_WIDTH_LEFT_SIZE1				0x1000
							#define THICKENING_WIDTH_LEFT_SIZE2				0x2000
							#define THICKENING_WIDTH_LEFT_SIZE3				0x4000

	BOOL			m_bOtherAbnormalitiesVisited;
		char			m_AnyOtherAbnormalities;
		unsigned long	m_ObservedOtherSymbols;
							#define OBSERVED_SYMBOL_AA						0x00000001
							#define OBSERVED_SYMBOL_AT						0x00000002
							#define OBSERVED_SYMBOL_AX						0x00000004
							#define OBSERVED_SYMBOL_BU						0x00000008
							#define OBSERVED_SYMBOL_CA						0x00000010
							#define OBSERVED_SYMBOL_CG						0x00000020
							#define OBSERVED_SYMBOL_CN						0x00000040
							#define OBSERVED_SYMBOL_CO						0x00000080
							#define OBSERVED_SYMBOL_CP						0x00000100
							#define OBSERVED_SYMBOL_CV						0x00000200
							#define OBSERVED_SYMBOL_DI						0x00000400
							#define OBSERVED_SYMBOL_EF						0x00000800
							#define OBSERVED_SYMBOL_EM						0x00001000
							#define OBSERVED_SYMBOL_ES						0x00002000
							#define OBSERVED_SYMBOL_FR						0x00004000
							#define OBSERVED_SYMBOL_HI						0x00008000
							#define OBSERVED_SYMBOL_HO						0x00010000
							#define OBSERVED_SYMBOL_ID						0x00020000
							#define OBSERVED_SYMBOL_IH						0x00040000
							#define OBSERVED_SYMBOL_KL						0x00080000
							#define OBSERVED_SYMBOL_ME						0x00100000
							#define OBSERVED_SYMBOL_PA						0x00200000
							#define OBSERVED_SYMBOL_PB						0x00400000
							#define OBSERVED_SYMBOL_PI						0x00800000
							#define OBSERVED_SYMBOL_PX						0x01000000
							#define OBSERVED_SYMBOL_RA						0x02000000
							#define OBSERVED_SYMBOL_RP						0x04000000
							#define OBSERVED_SYMBOL_TB						0x08000000
							#define OBSERVED_SYMBOL_OD						0x10000000

		unsigned long	m_ObservedOtherAbnormalities;
							#define OBSERVED_EVENTRATION					0x00000001
							#define OBSERVED_HIATAL_HERNIA					0x00000002
							#define OBSERVED_BRONCHOVASCULAR_MARKINGS		0x00000004
							#define OBSERVED_HYPERINFLATION					0x00000008
							#define OBSERVED_BONY_CHEST_CAGE				0x00000010
							#define OBSERVED_FRACTURE_HEALED				0x00000020
							#define OBSERVED_FRACTURE_NONHEALED				0x00000040
							#define OBSERVED_SCOLIOSIS						0x00000080
							#define OBSERVED_VERTEBRAL_COLUMN				0x00000100
							#define OBSERVED_AZYGOS_LOBE					0x00000200
							#define OBSERVED_LUNG_DENSITY					0x00000400
							#define OBSERVED_INFILTRATE						0x00000800
							#define OBSERVED_NODULAR_LESION					0x00001000
							#define OBSERVED_FOREIGN_BODY					0x00002000
							#define OBSERVED_POSTSURGICAL					0x00004000
							#define OBSERVED_CYST							0x00008000
							#define OBSERVED_AORTA_ANOMALY					0x00010000
							#define OBSERVED_VASCULAR_ABNORMALITY			0x00020000

		CString			m_OtherAbnormalitiesCommentsText;


	// Data from report production.
	//
	BOOL					m_bReportViewed;
	BOOL					m_bReportApproved;
	EDITED_DATE				m_DateOfRadiograph;
	char					m_Reserved1[ 12 ];		// This was previously the Worker SSN.  Keep the spacing for backward compatibility.
	unsigned short			m_TypeOfReading;		// Symbols for this entry are defined in Configuration.h.
	char					m_OtherTypeOfReading[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	char					m_FacilityIDNumber[ 10 ];
	EDITED_DATE				m_DateOfReading;

	EDITED_DATE				m_Reserved2;
	unsigned long			m_PhysicianNotificationStatus;
								#define OBSERVED_SEE_PHYSICIAN_YES				0x00040000
								#define OBSERVED_SEE_PHYSICIAN_NO				0x00080000


	// Data from report customization page.
	//
	READER_PERSONAL_INFO	m_ReaderInfo;
	char					m_AccessionNumber[ DICOM_ATTRIBUTE_STRING_LENGTH ];
	BOOL					m_bStudyWasPreviouslyInterpreted;
	unsigned long			m_SDYFileVersion;
	CLIENT_INFO				m_ClientInfo;
	EVENT_PARAMETERS		*m_pEventParameters;
	char					m_ReportPage1FilePath[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					m_ReportPage2FilePath[ FULL_FILE_SPEC_STRING_LENGTH ];




	// Method prototypes.
	//
	void			EraseStudyList();
	void			InitializeInterpretation();
	void			Initialize();
	void			ConvertDicomDAToSystemTime( char *pDicomTextDate, SYSTEMTIME *pSystemTime );
	void			PopulateFromAbstractDataRow( char *pTitleRow, char *pDataRow );
	void			UnpackData();
	BOOL			StudyButtonWasOn( int TargetButtonResourceSymbol );
	void			GetStudyEditField( int TargetTextResourceSymbol, char *pTextField );
	char			*GetStudyCommentField( int TargetTextResourceSymbol );
	void			InitStudyCommentFields();
	void			LoadStudyData( char *pTitleRow, char *pDataRow );
	void			LoadSeriesData( char *pTitleRow, char *pDataRow, DIAGNOSTIC_STUDY *pDiagnosticStudy );
	void			LoadImageData( char *pTitleRow, char *pDataRow, DIAGNOSTIC_SERIES *pDiagnosticSeries );
	BOOL			MergeWithExistingStudies( BOOL *pbNewStudyMergedWithExistingStudy );
	void			GetDateOfRadiographMMDDYY( char *pDateString );

	BOOL			SaveDiagnosticImage( FILE *pStudyFile, DIAGNOSTIC_IMAGE *pDiagnosticImage );
	BOOL			SaveDiagnosticSeries( FILE *pStudyFile, DIAGNOSTIC_SERIES *pDiagnosticSeries );
	BOOL			SaveDiagnosticStudy( FILE *pStudyFile, DIAGNOSTIC_STUDY *pDiagnosticStudy );
	void			GetStudyFileName( char *pStudyFileName, size_t BufferSize );
	BOOL			Save();

	BOOL			RestoreDiagnosticImage( FILE *pStudyFile, DIAGNOSTIC_IMAGE **ppDiagnosticImage );
	BOOL			RestoreDiagnosticSeries( FILE *pStudyFile, DIAGNOSTIC_SERIES **ppDiagnosticSeries, BOOL bOlderSeriesWithoutManufacturer );
	BOOL			RestoreDiagnosticStudy( FILE *pStudyFile, DIAGNOSTIC_STUDY **ppDiagnosticStudy );
	BOOL			Restore( char *pFullFilePath );

	void			DeleteStudyDataAndImages();
};
