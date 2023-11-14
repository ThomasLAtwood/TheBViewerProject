// Export.cpp : Implementation file that handle data exporting via the Abstracts.
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
//	*[2] 03/14/2023 by Tom Atwood
//		Fixed code security issues.
//	*[1] 12/21/2022 by Tom Atwood
//		Fixed code security issues.
//
//
#include "StdAfx.h"
#include "Module.h"
#include "ReportStatus.h"
#include "BViewer.h"
#include "Export.h"
#include "Study.h"
#include "Configuration.h"
#include "Customization.h"
#include "DiagnosticImage.h"
#include "Mouse.h"
#include "ImageView.h"
#include "MainFrm.h"


extern CONFIGURATION				BViewerConfiguration;
extern CCustomization				*pBViewerCustomization;
extern BOOL							bOKToSaveReaderInfo;


#define TRANSLATE_AS_TEXT				1
#define TRANSLATE_AS_BOOLEAN			2
#define TRANSLATE_AS_BITFIELD			3

EXPORT_COLUMN_FORMAT ExportList[] =
	{
		//
		// Information from the Dicom image file:
		{ "PatientLastName",			ABSTRACT_SOURCE_PATIENT,	TRANSLATE_AS_TEXT,			offsetof( CStudy, m_PatientLastName )					},
		{ "PatientFirstName",			ABSTRACT_SOURCE_PATIENT,	TRANSLATE_AS_TEXT,			offsetof( CStudy, m_PatientFirstName )					},
		{ "PatientID",					ABSTRACT_SOURCE_PATIENT,	TRANSLATE_AS_TEXT,			offsetof( CStudy, m_PatientID )							},
		{ "PatientsBirthDate",			ABSTRACT_SOURCE_PATIENT,	TRANSLATE_AS_DATE,			offsetof( CStudy, m_PatientsBirthDate )					},
		{ "PatientsSex",				ABSTRACT_SOURCE_PATIENT,	TRANSLATE_AS_TEXT,			offsetof( CStudy, m_PatientsSex )						},
		{ "PatientComments",			ABSTRACT_SOURCE_PATIENT,	TRANSLATE_AS_TEXT,			offsetof( CStudy, m_PatientComments )					},

		{ "StudyDate",					ABSTRACT_SOURCE_STUDY,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_STUDY, StudyDate )					},
		{ "StudyTime",					ABSTRACT_SOURCE_STUDY,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_STUDY, StudyTime )					},
		{ "AccessionNumber",			ABSTRACT_SOURCE_STUDY,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_STUDY, AccessionNumber )			},
		{ "InstitutionName",			ABSTRACT_SOURCE_STUDY,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_STUDY, InstitutionName )			},
		{ "ReferringPhysiciansName",	ABSTRACT_SOURCE_STUDY,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_STUDY, ReferringPhysiciansName )	},
		{ "StudyID",					ABSTRACT_SOURCE_STUDY,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_STUDY, StudyID )					},
		{ "StudyDescription",			ABSTRACT_SOURCE_STUDY,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_STUDY, StudyDescription )			},
		{ "Modality",					ABSTRACT_SOURCE_SERIES,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_SERIES, Modality )					},
		{ "SeriesNumber",				ABSTRACT_SOURCE_SERIES,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_SERIES, SeriesNumber )				},
		{ "Laterality",					ABSTRACT_SOURCE_SERIES,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_SERIES, Laterality )				},
		{ "SeriesDate",					ABSTRACT_SOURCE_SERIES,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_SERIES, SeriesDate )				},
		{ "SeriesTime",					ABSTRACT_SOURCE_SERIES,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_SERIES, SeriesTime )				},
		{ "ProtocolName",				ABSTRACT_SOURCE_SERIES,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_SERIES, ProtocolName )				},
		{ "SeriesDescription",			ABSTRACT_SOURCE_SERIES,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_SERIES, SeriesDescription )		},
		{ "BodyPartExamined",			ABSTRACT_SOURCE_SERIES,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_SERIES, BodyPartExamined )			},
		{ "PatientPosition",			ABSTRACT_SOURCE_SERIES,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_SERIES, PatientPosition )			},
		{ "PatientOrientation",			ABSTRACT_SOURCE_SERIES,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_SERIES, PatientOrientation )		},
		{ "InstanceNumber",				ABSTRACT_SOURCE_IMAGE,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_IMAGE, InstanceNumber )			},
		{ "ContentDate",				ABSTRACT_SOURCE_IMAGE,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_IMAGE, ContentDate )				},
		{ "ContentTime",				ABSTRACT_SOURCE_IMAGE,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_IMAGE, ContentTime )				},
		{ "AcquisitionNumber",			ABSTRACT_SOURCE_IMAGE,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_IMAGE, AcquisitionNumber )			},
		{ "ImageType",					ABSTRACT_SOURCE_IMAGE,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_IMAGE, ImageType )					},
		{ "InstanceCreationDate",		ABSTRACT_SOURCE_IMAGE,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_IMAGE, InstanceCreationDate )		},
		{ "InstanceCreationTime",		ABSTRACT_SOURCE_IMAGE,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_IMAGE, InstanceCreationTime )		},
		{ "AcquisitionDate",			ABSTRACT_SOURCE_IMAGE,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_IMAGE, AcquisitionDate )			},
		{ "AcquisitionTime",			ABSTRACT_SOURCE_IMAGE,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_IMAGE, AcquisitionTime )			},
		{ "SamplesPerPixel",			ABSTRACT_SOURCE_IMAGE,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_IMAGE, SamplesPerPixel )			},
		{ "PhotometricInterpretation",	ABSTRACT_SOURCE_IMAGE,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_IMAGE, PhotometricInterpretation )	},
		{ "Rows",						ABSTRACT_SOURCE_IMAGE,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_IMAGE, Rows )						},
		{ "Columns",					ABSTRACT_SOURCE_IMAGE,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_IMAGE, Columns )					},
		{ "PixelAspectRatio",			ABSTRACT_SOURCE_IMAGE,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_IMAGE, PixelAspectRatio )			},
		{ "BitsAllocated",				ABSTRACT_SOURCE_IMAGE,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_IMAGE, BitsAllocated )				},
		{ "BitsStored",					ABSTRACT_SOURCE_IMAGE,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_IMAGE, BitsStored )				},
		{ "HighBit",					ABSTRACT_SOURCE_IMAGE,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_IMAGE, HighBit )					},
		{ "PixelRepresentation",		ABSTRACT_SOURCE_IMAGE,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_IMAGE, PixelRepresentation )		},
		{ "WindowCenter",				ABSTRACT_SOURCE_IMAGE,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_IMAGE, WindowCenter )				},
		{ "WindowWidth",				ABSTRACT_SOURCE_IMAGE,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_IMAGE, WindowWidth )				},
		{ "StudyInstanceUID",			ABSTRACT_SOURCE_STUDY,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_STUDY, StudyInstanceUID )			},
		{ "SeriesInstanceUID",			ABSTRACT_SOURCE_SERIES,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_SERIES, SeriesInstanceUID )		},
		{ "SOPInstanceUID",				ABSTRACT_SOURCE_IMAGE,		TRANSLATE_AS_TEXT,			offsetof( DIAGNOSTIC_IMAGE, SOPInstanceUID )			},
		//
		// Information entered identifying the B reader:
		{ "ReaderLastName",				ABSTRACT_SOURCE_READER,		TRANSLATE_AS_TEXT,			offsetof( READER_PERSONAL_INFO, LastName )				},
		{ "PrintedSignatureName",		ABSTRACT_SOURCE_READER,		TRANSLATE_AS_TEXT,			offsetof( READER_PERSONAL_INFO, ReportSignatureName )	},
		{ "IDNumber",					ABSTRACT_SOURCE_READER,		TRANSLATE_AS_TEXT,			offsetof( READER_PERSONAL_INFO, ID )					},
		{ "Initials",					ABSTRACT_SOURCE_READER,		TRANSLATE_AS_TEXT,			offsetof( READER_PERSONAL_INFO, Initials )				},
		{ "StreetAddress",				ABSTRACT_SOURCE_READER,		TRANSLATE_AS_TEXT,			offsetof( READER_PERSONAL_INFO, StreetAddress )			},
		{ "City",						ABSTRACT_SOURCE_READER,		TRANSLATE_AS_TEXT,			offsetof( READER_PERSONAL_INFO, City )					},
		{ "State",						ABSTRACT_SOURCE_READER,		TRANSLATE_AS_TEXT,			offsetof( READER_PERSONAL_INFO, State )					},
		{ "ZipCode",					ABSTRACT_SOURCE_READER,		TRANSLATE_AS_TEXT,			offsetof( READER_PERSONAL_INFO, ZipCode )				},
		{ "AE_TITLE",					ABSTRACT_SOURCE_READER,		TRANSLATE_AS_TEXT,			offsetof( READER_PERSONAL_INFO, AE_TITLE )				},
		//
		// Information entered identifying the client:
		{ "ClientName",					ABSTRACT_SOURCE_CLIENT,		TRANSLATE_AS_TEXT,			offsetof( CLIENT_INFO, Name )							},
		{ "ClientStreetAddress",		ABSTRACT_SOURCE_CLIENT,		TRANSLATE_AS_TEXT,			offsetof( CLIENT_INFO, StreetAddress )					},
		{ "ClientCity",					ABSTRACT_SOURCE_CLIENT,		TRANSLATE_AS_TEXT,			offsetof( CLIENT_INFO, City )							},
		{ "ClientState",				ABSTRACT_SOURCE_CLIENT,		TRANSLATE_AS_TEXT,			offsetof( CLIENT_INFO, State )							},
		{ "ClientZipCode",				ABSTRACT_SOURCE_CLIENT,		TRANSLATE_AS_TEXT,			offsetof( CLIENT_INFO, ZipCode )						},
		{ "ClientPhone",				ABSTRACT_SOURCE_CLIENT,		TRANSLATE_AS_TEXT,			offsetof( CLIENT_INFO, Phone )							},
		{ "ClientOtherContactInfo",		ABSTRACT_SOURCE_CLIENT,		TRANSLATE_AS_TEXT,			offsetof( CLIENT_INFO, OtherContactInfo )				},
		//
		// Information from the report screen.
		{ "DateOfRadiograph",			ABSTRACT_SOURCE_REPORT,		TRANSLATE_AS_DATE,			offsetof( CStudy, m_DateOfRadiograph )					},
		{ "DateOfReading",				ABSTRACT_SOURCE_REPORT,		TRANSLATE_AS_DATE,			offsetof( CStudy, m_DateOfReading )						},
		{ "TypeOfReading",				ABSTRACT_SOURCE_REPORT,		TRANSLATE_AS_BITFIELD_4,	offsetof( CStudy, m_TypeOfReading )						},
		{ "FacilityIDNumber",			ABSTRACT_SOURCE_REPORT,		TRANSLATE_AS_TEXT,			offsetof( CStudy, m_FacilityIDNumber )					},
		{ "PhysicianNotificationStatus",ABSTRACT_SOURCE_REPORT,		TRANSLATE_AS_BITFIELD_32,	offsetof( CStudy, m_PhysicianNotificationStatus )		},
		//
		// Results of interpretation, screen 1.
		{ "ImageQuality",				ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_BITFIELD_16,	offsetof( CStudy, m_ImageQuality )						},
		{ "ImageDefectOther",			ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_TEXT_STRING,	offsetof( CStudy, m_ImageDefectOtherText )				},
		//
		// Results of interpretation, screen 2.
		{ "AnyParenchymalAbnorm",		ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_BITFIELD_4,	offsetof( CStudy, m_AnyParenchymalAbnormalities )		},
		{ "ParenchymalAbnormalities",	ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_BITFIELD_32,	offsetof( CStudy, m_ObservedParenchymalAbnormalities )	},
		//
		// Results of interpretation, screen 3.
		{ "AnyPleuralAbnormalities",	ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_BITFIELD_4,	offsetof( CStudy, m_AnyPleuralAbnormalities )			},
		{ "PleuralPlaqueSites",			ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_BITFIELD_16,	offsetof( CStudy, m_ObservedPleuralPlaqueSites )		},
		{ "PleuralCalcificationSites",	ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_BITFIELD_16,	offsetof( CStudy, m_ObservedPleuralCalcificationSites )	},
		{ "PlaqueExtent",				ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_BITFIELD_16,	offsetof( CStudy, m_ObservedPlaqueExtent )				},
		{ "PlaqueWidth",				ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_BITFIELD_16,	offsetof( CStudy, m_ObservedPlaqueWidth )				},
		{ "CostophrenicAngleOblit",		ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_BITFIELD_4,	offsetof( CStudy, m_ObservedCostophrenicAngleObliteration )	},
		{ "PleuralThickeningSites",		ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_BITFIELD_8,	offsetof( CStudy, m_ObservedPleuralThickeningSites )	},
		{ "ThickenCalcificationSites",	ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_BITFIELD_8,	offsetof( CStudy, m_ObservedThickeningCalcificationSites )	},
		{ "ThickeningExtent",			ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_BITFIELD_16,	offsetof( CStudy, m_ObservedThickeningExtent )			},
		{ "ThickeningWidth",			ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_BITFIELD_16,	offsetof( CStudy, m_ObservedThickeningWidth )			},
		//
		// Results of interpretation, screen 4.
		{ "AnyOtherAbnormalities",		ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_BITFIELD_4,	offsetof( CStudy, m_AnyOtherAbnormalities )				},
		{ "OtherSymbols",				ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_BITFIELD_32,	offsetof( CStudy, m_ObservedOtherSymbols )				},
		{ "OtherAbnormalities",			ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_BITFIELD_32,	offsetof( CStudy, m_ObservedOtherAbnormalities )		},
		{ "OtherAbnormalitiesComments",	ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_TEXT_STRING,	offsetof( CStudy, m_OtherAbnormalitiesCommentsText )	},
		//
		// Image manipulation settings.
		{ "AdjustedWindowWidth",		ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_FLOAT,			offsetof( CStudy, m_WindowWidth )						},
		{ "AdjustedWindowCenter",		ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_FLOAT,			offsetof( CStudy, m_WindowCenter )						},
		{ "MaxGrayscaleValue",			ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_FLOAT,			offsetof( CStudy, m_MaxGrayscaleValue )					},
		{ "AdjustedGamma",				ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_FLOAT,			offsetof( CStudy, m_GammaSetting )						},
		{ "TimeStudyFirstOpened",		ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_TEXT,			offsetof( CStudy, m_TimeStudyFirstOpened )				},
		{ "TimeReportApproved",			ABSTRACT_SOURCE_ANALYSIS,	TRANSLATE_AS_TEXT,			offsetof( CStudy, m_TimeReportApproved )				},

		
		{ 0, 0, 0 }
	};


static char*		LeadingZeros = "00000000000000000000000000000000";

BOOL CreateAbstractExportFile( CStudy *pStudy )
{
	BOOL					bOK = TRUE;
	char					AbstractFileName[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					AbstractFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	FILE					*pAbstractFile;
	DIAGNOSTIC_STUDY		*pDiagnosticStudy;
	DIAGNOSTIC_SERIES		*pDiagnosticSeries;
	DIAGNOSTIC_IMAGE		*pDiagnosticImage;
	READER_PERSONAL_INFO	*pReaderInfo;
	CLIENT_INFO				*pClientInfo;
	char					*pDataStructure;
	unsigned long			nAbstractField;
	EXPORT_COLUMN_FORMAT	*pAbstractFieldFormat;
	BOOL					bEndOfExportList;
	char					*pExportFieldValue;
	CString					*pExportStringValue;
	BOOL					bExportBooleanValue;
	unsigned long			ExportBitFieldValue;
	EDITED_DATE				*pExportDateValue;
	double					*pExportFloatValue;
	char					TextString[ DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH ];
	char					TempString[ DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH ];
	char					IntegerText[ 65 ];
	
	strncpy_s( AbstractFileName, FULL_FILE_SPEC_STRING_LENGTH, pStudy -> m_pCurrentImageInfo -> SOPInstanceUID, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
	strncat_s( AbstractFileName, FULL_FILE_SPEC_STRING_LENGTH, ".axt", _TRUNCATE );													// *[2] Replaced strcat with strncat_s.
	strncpy_s( AbstractFileSpec, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.ExportsDirectory, _TRUNCATE );					// *[2] Replaced strncat with strncpy_s.
	LocateOrCreateDirectory( AbstractFileSpec );	// Ensure directory exists.
	if ( AbstractFileSpec[ strlen( AbstractFileSpec ) - 1 ] != '\\' )
		strncat_s( AbstractFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );												// *[2] Replaced strcat with strncat_s.
	strncat_s( AbstractFileSpec, FULL_FILE_SPEC_STRING_LENGTH, AbstractFileName, _TRUNCATE );										// *[2] Replaced strncat with strncat_s.
	if ( pStudy -> m_pEventParameters != 0 )
		strncpy_s( pStudy -> m_pEventParameters -> AXTFilePath, FULL_FILE_SPEC_STRING_LENGTH, AbstractFileSpec, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
	pAbstractFile = fopen( AbstractFileSpec, "wt" );
	if ( pAbstractFile != 0 )
		{
		// Write the row of abstract field titles.
		nAbstractField = 0;
		bEndOfExportList = FALSE;
		do
			{
			pAbstractFieldFormat = &ExportList[ nAbstractField ];
			if ( pAbstractFieldFormat -> pColumnTitle == 0 )
				bEndOfExportList = TRUE;
			if ( !bEndOfExportList )
				{
				if ( nAbstractField > 0 )
					fputs( ",", pAbstractFile );
				FormatCSVField( pAbstractFieldFormat -> pColumnTitle, TextString, MAX_LOGGING_STRING_LENGTH - 1 );
				fputs( TextString, pAbstractFile );
				}
			nAbstractField++;
			}
		while ( !bEndOfExportList );
		fputs( "\n", pAbstractFile );
		
		// Write the row of abstract field values.
		pDiagnosticStudy = pStudy -> m_pCurrentStudyInfo;
		pDiagnosticSeries = pStudy -> m_pCurrentSeriesInfo;
		pDiagnosticImage = pStudy -> m_pCurrentImageInfo;
		pReaderInfo = &pBViewerCustomization -> m_ReaderInfo;
		pClientInfo = &BViewerConfiguration.m_ClientInfo;
		nAbstractField = 0;
		bEndOfExportList = FALSE;
		do
			{
			pAbstractFieldFormat = &ExportList[ nAbstractField ];
			if ( pAbstractFieldFormat -> pColumnTitle == 0 )
				bEndOfExportList = TRUE;
			if ( !bEndOfExportList )
				{
				switch ( pAbstractFieldFormat -> DataStructureID )
					{
					case ABSTRACT_SOURCE_PATIENT:
					case ABSTRACT_SOURCE_ANALYSIS:
					case ABSTRACT_SOURCE_REPORT:
						pDataStructure = (char*)pStudy;
						break;
					case ABSTRACT_SOURCE_STUDY:
						pDataStructure = (char*)pDiagnosticStudy;
						break;
					case ABSTRACT_SOURCE_SERIES:
						pDataStructure = (char*)pDiagnosticSeries;
						break;
					case ABSTRACT_SOURCE_IMAGE:
						pDataStructure = (char*)pDiagnosticImage;
						break;
					case ABSTRACT_SOURCE_READER:
						pDataStructure = (char*)pReaderInfo;
						break;
					case ABSTRACT_SOURCE_CLIENT:
						pDataStructure = (char*)pClientInfo;
						break;
					default:									// *[2] Added default case.
						bOK = FALSE;
						break;
					}
				if ( bOK )									// *[2] Added error check.
					{
					if ( nAbstractField > 0 )
						fputs( ",", pAbstractFile );
					TextString[ 0 ] = '\0';						// *[1] Eliminated call to strcpy.
					switch ( pAbstractFieldFormat -> DataType )
						{
						case TRANSLATE_AS_TEXT:
							pExportFieldValue = (char*)( pDataStructure + pAbstractFieldFormat -> DataStructureOffset );
							FormatCSVField( pExportFieldValue, TextString, DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH - 1 );
							break;
						case TRANSLATE_AS_TEXT_STRING:
							pExportStringValue = (CString*)( pDataStructure + pAbstractFieldFormat -> DataStructureOffset );
							strncpy_s( TempString, DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH, (const char*)*pExportStringValue, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
							FormatCSVField( TempString, TextString, DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH - 1 );
							break;
						case TRANSLATE_AS_BOOLEAN:
							bExportBooleanValue = *(BOOL*)( pDataStructure + pAbstractFieldFormat -> DataStructureOffset );
							if ( bExportBooleanValue )
								strncpy_s( TextString, DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH, "Y", _TRUNCATE );								// *[1] Replaced strcpy with strncpy_s.
							else
								strncpy_s( TextString, DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH, "N", _TRUNCATE );								// *[1] Replaced strcpy with strncpy_s.
							break;
						case TRANSLATE_AS_BITFIELD_32:
							ExportBitFieldValue = *(unsigned long*)( pDataStructure + pAbstractFieldFormat -> DataStructureOffset );
							ExportBitFieldValue &= 0xFFFFFFFF;
							_ultoa( ExportBitFieldValue, IntegerText, 2 );
							strncpy_s( TextString, DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH, LeadingZeros, _TRUNCATE );						// *[2] Replaced strncat with strncpy_s.
							strncat_s( TextString, DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH, IntegerText, _TRUNCATE );							// *[2] Replaced strcat with strncat_s.
							break;
						case TRANSLATE_AS_BITFIELD_16:
							ExportBitFieldValue = *(unsigned long*)( pDataStructure + pAbstractFieldFormat -> DataStructureOffset );
							ExportBitFieldValue &= 0x0000FFFF;
							_ultoa( ExportBitFieldValue, IntegerText, 2 );
							strncpy_s( TextString, DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH, LeadingZeros, _TRUNCATE );						// *[2] Replaced strncat with strncpy_s.
							strncat_s( TextString, DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH, IntegerText, _TRUNCATE );							// *[2] Replaced strcat with strncat_s.
							break;
						case TRANSLATE_AS_BITFIELD_8:
							ExportBitFieldValue = *(unsigned long*)( pDataStructure + pAbstractFieldFormat -> DataStructureOffset );
							ExportBitFieldValue &= 0x000000FF;
							_ultoa( ExportBitFieldValue, IntegerText, 2 );
							strncpy_s( TextString, DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH, LeadingZeros, _TRUNCATE );						// *[2] Replaced strncat with strncpy_s.
							strncat_s( TextString, DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH, IntegerText, _TRUNCATE );							// *[2] Replaced strcat with strncat_s.
							break;
						case TRANSLATE_AS_BITFIELD_4:
							ExportBitFieldValue = *(unsigned long*)( pDataStructure + pAbstractFieldFormat -> DataStructureOffset );
							ExportBitFieldValue &= 0x0000000F;
							_ultoa( ExportBitFieldValue, IntegerText, 2 );
							strncpy_s( TextString, DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH, LeadingZeros, _TRUNCATE );						// *[2] Replaced strncat with strncpy_s.
							strncat_s( TextString, DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH, IntegerText, _TRUNCATE );							// *[2] Replaced strcat with strncat_s.
							break;
						case TRANSLATE_AS_DATE:
							pExportDateValue = (EDITED_DATE*)( pDataStructure + pAbstractFieldFormat -> DataStructureOffset );
							if ( pExportDateValue -> bDateHasBeenEdited )
								_snprintf_s( TextString, DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH, _TRUNCATE,									// *[2] Replaced sprintf() with _snprintf_s.
														"%2u/%2u/%4u", pExportDateValue -> Date.wMonth,
														pExportDateValue -> Date.wDay, pExportDateValue -> Date.wYear );
							else
								strncpy_s( TextString, DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH, "  /  /    ", _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
							break;
						case TRANSLATE_AS_FLOAT:
							pExportFloatValue = (double*)( pDataStructure + pAbstractFieldFormat -> DataStructureOffset );
							_snprintf_s( TextString, DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH, _TRUNCATE, "%7.1f", *pExportFloatValue );		// *[2] Replaced sprintf() with _snprintf_s.
							break;
						}
					fputs( TextString, pAbstractFile );
					nAbstractField++;
					}
				}
			}
		while ( bOK && !bEndOfExportList );					// *[2] Added error check.
		fputs( "\n", pAbstractFile );
		fclose( pAbstractFile );
		}
	
	return bOK;
}

BOOL CreateStudyfromAbstractExportFile( CStudy *pStudy, char *pTitleRow, char *pDataRow )
{
	BOOL					bOK = TRUE;
	EXPORT_COLUMN_FORMAT	*pImportList = ExportList;
	char					ListItemText[ 2048 ];
	char					*pDataStructure = 0;			// *[2] Initialized pointer.
	DIAGNOSTIC_STUDY		*pDiagnosticStudy = 0;			// *[2] Initialized pointer.
	DIAGNOSTIC_SERIES		*pDiagnosticSeries = 0;			// *[2] Initialized pointer.
	DIAGNOSTIC_IMAGE		*pDiagnosticImage = 0;			// *[2] Initialized pointer.
	READER_PERSONAL_INFO	*pReaderInfo;
	unsigned long			nAbstractField;
	EXPORT_COLUMN_FORMAT	*pAbstractFieldFormat;
	BOOL					bEndOfImportList;
	CString					*pImportStringValue;
	BOOL					*pbImportBooleanValue;
	unsigned long			*pImportBitFieldValue;
	unsigned long			ImportBitFieldValue;
	unsigned short			*pImport16BitFieldValue;
	unsigned char			*pImport8BitFieldValue;
	EDITED_DATE				*pImportDateValue;
	SYSTEMTIME				TempTime;
	int						Year;
	int						Month;
	int						Day;
	int						ReturnedValue;
	double					*pImportFloatValue;
	char					*pCharThatStopsBinaryTextScan;

	// Write the row of abstract field values.
	bOK = ( pStudy != 0 );
	if ( bOK )
		{
		pStudy -> m_pCurrentStudyInfo = (DIAGNOSTIC_STUDY*)calloc( 1, sizeof( DIAGNOSTIC_STUDY ) );
		pDiagnosticStudy = pStudy -> m_pCurrentStudyInfo;
		bOK = ( pDiagnosticStudy != 0 );
		if ( bOK )
			{
			pDiagnosticStudy -> pNextDiagnosticStudy = 0;
			pStudy -> m_pDiagnosticStudyList = pDiagnosticStudy;

			pStudy -> m_pCurrentSeriesInfo = (DIAGNOSTIC_SERIES*)calloc( 1, sizeof( DIAGNOSTIC_SERIES ) );
			pDiagnosticSeries = pStudy -> m_pCurrentSeriesInfo;
			bOK = ( pDiagnosticSeries != 0 );
			if ( bOK )
				{
				pDiagnosticSeries -> pNextDiagnosticSeries = 0;
				pDiagnosticStudy -> pDiagnosticSeriesList = pDiagnosticSeries;

				pStudy -> m_pCurrentImageInfo = (DIAGNOSTIC_IMAGE*)calloc( 1, sizeof( DIAGNOSTIC_IMAGE ) );
				pDiagnosticImage = pStudy -> m_pCurrentImageInfo;
				bOK = ( pDiagnosticImage != 0 );
				if ( bOK )
					{
					pDiagnosticImage -> pNextDiagnosticImage = 0;
					pDiagnosticSeries -> pDiagnosticImageList = pDiagnosticImage;
					}
				}
			}
		}
	if ( bOK )
		{
		pReaderInfo = &pBViewerCustomization -> m_ReaderInfo;
		nAbstractField = 0;
		bEndOfImportList = FALSE;
		do
			{
			pAbstractFieldFormat = &pImportList[ nAbstractField ];
			if ( pAbstractFieldFormat -> pColumnTitle == 0 )
				bEndOfImportList = TRUE;
			if ( !bEndOfImportList )
				{
				switch ( pAbstractFieldFormat -> DataStructureID )
					{
					case ABSTRACT_SOURCE_PATIENT:
					case ABSTRACT_SOURCE_ANALYSIS:
					case ABSTRACT_SOURCE_REPORT:
						pDataStructure = (char*)pStudy;
						break;
					case ABSTRACT_SOURCE_STUDY:
						pDataStructure = (char*)pDiagnosticStudy;
						break;
					case ABSTRACT_SOURCE_SERIES:
						pDataStructure = (char*)pDiagnosticSeries;
						break;
					case ABSTRACT_SOURCE_IMAGE:
						pDataStructure = (char*)pDiagnosticImage;
						break;
					case ABSTRACT_SOURCE_READER:
						pDataStructure = (char*)pReaderInfo;
						break;
					case ABSTRACT_SOURCE_CLIENT:
						pDataStructure = (char*)&pStudy -> m_ClientInfo;
						break;
					default:									// *[2] Added default case.
						bOK = FALSE;
						break;
					}
				if ( bOK && GetAbstractColumnValueForSpecifiedField( pAbstractFieldFormat -> pColumnTitle, pTitleRow, pDataRow, ListItemText, 2048 ) )		// *[2] Added error check.
					{
					switch ( pAbstractFieldFormat -> DataType )
						{
						case TRANSLATE_AS_TEXT:
							strncpy_s( (char*)pDataStructure + pAbstractFieldFormat -> DataStructureOffset, 4, ListItemText, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
							break;
						case TRANSLATE_AS_TEXT_STRING:
							pImportStringValue = (CString*)( pDataStructure + pAbstractFieldFormat -> DataStructureOffset );
							pImportStringValue -> SetString( ListItemText );
							break;
						case TRANSLATE_AS_BOOLEAN:
							pbImportBooleanValue = (BOOL*)( pDataStructure + pAbstractFieldFormat -> DataStructureOffset );
							if ( _stricmp( ListItemText, "Y" ) == 0 )
								*pbImportBooleanValue = TRUE;
							else if ( _stricmp( ListItemText, "N" ) == 0 )
								*pbImportBooleanValue = FALSE;
							else
								bOK = FALSE;
							break;
						case TRANSLATE_AS_BITFIELD_32:
							pImportBitFieldValue = (unsigned long*)( pDataStructure + pAbstractFieldFormat -> DataStructureOffset );
							*pImportBitFieldValue = strtoul( ListItemText, &pCharThatStopsBinaryTextScan, 2 );
							bOK = ( pCharThatStopsBinaryTextScan != NULL && *pCharThatStopsBinaryTextScan == '\0' );
							break;
						case TRANSLATE_AS_BITFIELD_16:
							ImportBitFieldValue = strtoul( ListItemText, &pCharThatStopsBinaryTextScan, 2 );
							bOK = ( pCharThatStopsBinaryTextScan != NULL && *pCharThatStopsBinaryTextScan == '\0' );
							if ( bOK )
								bOK = ( ( ImportBitFieldValue & 0xFFFF0000 ) == 0 );
							if ( bOK )
								{
								pImport16BitFieldValue = (unsigned short*)( pDataStructure + pAbstractFieldFormat -> DataStructureOffset );
								*pImport16BitFieldValue = (unsigned short)ImportBitFieldValue;
								}
							break;
						case TRANSLATE_AS_BITFIELD_8:
							ImportBitFieldValue = strtoul( ListItemText, &pCharThatStopsBinaryTextScan, 2 );
							bOK = ( pCharThatStopsBinaryTextScan != NULL && *pCharThatStopsBinaryTextScan == '\0' );
							if ( bOK )
								bOK = ( ( ImportBitFieldValue & 0xFFFFFF00 ) == 0 );
							if ( bOK )
								{
								pImport8BitFieldValue = (unsigned char*)( pDataStructure + pAbstractFieldFormat -> DataStructureOffset );
								*pImport8BitFieldValue = (unsigned char)ImportBitFieldValue;
								}
							break;
						case TRANSLATE_AS_BITFIELD_4:
							ImportBitFieldValue = strtoul( ListItemText, &pCharThatStopsBinaryTextScan, 2 );
							bOK = ( pCharThatStopsBinaryTextScan != NULL && *pCharThatStopsBinaryTextScan == '\0' );
							if ( bOK )
								bOK = ( ( ImportBitFieldValue & 0xFFFFFFF0 ) == 0 );
							if ( bOK )
								{
								if ( pAbstractFieldFormat -> DataStructureID == ABSTRACT_SOURCE_REPORT )
									{
									// the 4-bit field in the report section is declared as an unsigned short.
									pImport16BitFieldValue = (unsigned short*)( pDataStructure + pAbstractFieldFormat -> DataStructureOffset );
									*pImport16BitFieldValue = (unsigned short)ImportBitFieldValue;
									}
								else
									{
									pImport8BitFieldValue = (unsigned char*)( pDataStructure + pAbstractFieldFormat -> DataStructureOffset );
									*pImport8BitFieldValue = (unsigned char)ImportBitFieldValue;
									}
								}
							break;
						case TRANSLATE_AS_DATE:
							pImportDateValue = (EDITED_DATE*)( pDataStructure + pAbstractFieldFormat -> DataStructureOffset );
							ReturnedValue = sscanf_s( ListItemText, "%2d/%2d/%4d", (int*)&Month, (int*)&Day, (int*)&Year );			// *[2] Replaced sscanf with sscanf_s.
							memset( &TempTime, '\0', sizeof(SYSTEMTIME) );
							if ( ReturnedValue != 0 && ReturnedValue != EOF )
								{
								TempTime.wYear = (WORD)Year;
								TempTime.wMonth = (WORD)Month;
								TempTime.wDay = (WORD)Day;
								}
							memcpy( (char*)&pImportDateValue -> Date, (char*)&TempTime, sizeof(SYSTEMTIME) );
							pImportDateValue -> bDateHasBeenEdited = ( ReturnedValue != 0 && ReturnedValue != EOF );
							break;

						case TRANSLATE_AS_FLOAT:
							pImportFloatValue = (double*)( pDataStructure + pAbstractFieldFormat -> DataStructureOffset );
							*pImportFloatValue =  strtod( ListItemText, &pCharThatStopsBinaryTextScan );
							bOK = ( pCharThatStopsBinaryTextScan != NULL && *pCharThatStopsBinaryTextScan == '\0' );
							break;
						}
					}
				nAbstractField++;
				}
			}
		while ( !bEndOfImportList && bOK );
		if ( bOK )
			{
			pStudy -> m_bStudyWasPreviouslyInterpreted = TRUE;
			memcpy( &pStudy -> m_ReaderInfo, pReaderInfo, sizeof( READER_PERSONAL_INFO ) );
			}
		}

	return bOK;
}


void ImportAbstractStudyRow( char *pTitleRow, char *pDataRow )
{
	BOOL					bNoError = TRUE;
	CStudy					*pNewStudy;
	BOOL					bNewStudyMergedWithExistingStudy;
	char					TextString[ FILE_PATH_STRING_LENGTH ];
	char					*pSOPInstanceUID;

	pNewStudy = new CStudy();
	if ( pNewStudy != 0 )
		{
		pNewStudy ->m_bStudyWasPreviouslyInterpreted = TRUE;
		bOKToSaveReaderInfo = FALSE;
		LogMessage( "    NewImports.axt:  Populating new study info.", MESSAGE_TYPE_SUPPLEMENTARY );
		bNoError = CreateStudyfromAbstractExportFile( pNewStudy, pTitleRow, pDataRow );
		if ( !bNoError )										// *[2] Added an error response.
			LogMessage( "   *** An error occcurred creating a study from an abstract export file.", MESSAGE_TYPE_SUPPLEMENTARY );

		pSOPInstanceUID = pNewStudy -> m_pCurrentImageInfo -> SOPInstanceUID;

		LogMessage( "    NewImports.axt:  Check for merging with existing study.", MESSAGE_TYPE_SUPPLEMENTARY );
		bNoError = pNewStudy -> MergeWithExistingStudies( &bNewStudyMergedWithExistingStudy );
		if ( bNoError )
			{
			if ( bNewStudyMergedWithExistingStudy )
				{
				LogMessage( "Adding new image to existing study.", MESSAGE_TYPE_SUPPLEMENTARY );
				delete pNewStudy;
				pNewStudy = 0;
				}
			else
				{
				sprintf_s( TextString, FILE_PATH_STRING_LENGTH, "Adding new study for AE_TITLE %s.", pNewStudy -> m_ReaderInfo.AE_TITLE );		// *[1] Replaced sprintf with sprint_s.
				LogMessage( TextString, MESSAGE_TYPE_SUPPLEMENTARY );
				if ( BViewerConfiguration.bAutoGeneratePDFReportsFromAXTFiles )
					{
					AppendToList( &ThisBViewerApp.m_AvailableStudyList, (void*)pNewStudy );
					strncpy_s( ThisBViewerApp.m_AutoLoadSOPInstanceUID, DICOM_ATTRIBUTE_UI_STRING_LENGTH, pSOPInstanceUID, _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
					}
				else
					AppendToList( &ThisBViewerApp.m_NewlyArrivedStudyList, (void*)pNewStudy );
				}
			}
		else if ( pNewStudy != 0 )			// *[1] Prevent memory leak if an error occurs.
			delete pNewStudy;				// *[1]
		}

}





