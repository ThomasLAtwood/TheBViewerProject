// Dicom.cpp : Implements the data structures and functions related to
//	reading and writing files in the Dicom format.
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
#include "Module.h"
#include <stddef.h>
#include "ReportStatus.h"
#include "ServiceMain.h"
#include "Dicom.h"
#include "Abstract.h"
#include "Configuration.h"
#include "Operation.h"
#include "Exam.h"
#include "ProductDispatcher.h"
#include "ExamEdit.h"
#include "ExamReformat.h"


//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		DicomModuleInfo = { MODULE_DICOM, "Dicom Module", InitDicomModule, CloseDicomModule };


static ERROR_DICTIONARY_ENTRY	DicomErrorCodes[] =
			{
				{ DICOM_ERROR_INSUFFICIENT_MEMORY				, "An error occurred allocating a memory block for data storage." },
				{ DICOM_ERROR_FILE_OPEN							, "An error occurred attempting to open a Dicom file." },
				{ DICOM_ERROR_FILE_READ							, "An error occurred attempting to read a Dicom file." },
				{ DICOM_ERROR_DICOM_SIGNATURE					, "The standard Dicom signature was not found after the Dicom preamble." },
				{ DICOM_ERROR_UNEVEN_VALUE_LENGTH				, "An uneven Dicom element value length was encountered in the Dicom file." },
				{ DICOM_ERROR_ALLOCATE_VALUE					, "A Dicom element value memory allocation failed." },
				{ DICOM_ERROR_TRANSFER_SYNTAX					, "No Dicom transfer syntax was specified in (0002, 0010)." },
				{ DICOM_ERROR_EXAM_INFO_STRUCTURE				, "An invalid exam structure pointer was specified for a Dicom file parse." },
				{ DICOM_ERROR_DICOM_OPEN_WRITE					, "An error occurred opening and initializing a Dicom file for writing." },
				{ DICOM_ERROR_BASIC_OFFSET_TABLE				, "No Basic Offset Table item was found following the pixel data element." },
				{ DICOM_ERROR_PARSING_PAST_END_OF_DATA			, "An attempt was made to parse beyond the end of the final data buffer." },
				{ DICOM_ERROR_REQUIRED_DICOM_ELEMENT_MISSING	, "A Dicom element required for processing was missing from this file." },
				{ DICOM_ERROR_DICOM_STORE_CREATE				, "An error occurred creating the study's Dicom image file for storage." },
				{ DICOM_ERROR_DICOM_STORE_WRITE					, "An error occurred writing the study's Dicom image file for storage." },
				{ 0												, NULL }
			};

static ERROR_DICTIONARY_MODULE		DicomStatusErrorDictionary =
										{
										MODULE_DICOM,
										DicomErrorCodes,
										DICOM_ERROR_DICT_LENGTH,
										0
										};

extern unsigned short				GetTransferSyntaxIndex( char *pTransferSyntaxUID, unsigned short Length );

extern TRANSFER_SERVICE				TransferService;
extern CONFIGURATION				ServiceConfiguration;

static TRANSFER_SYNTAX				LocalMemoryByteOrder;

static void							SwapBytesFromFile( void *pData, long nValueSize, TRANSFER_SYNTAX TransferSyntax );

// This function must be called before any other function in this module.
void InitDicomModule()
{
	LinkModuleToList( &DicomModuleInfo );
	RegisterErrorDictionary( &DicomStatusErrorDictionary );
}


void CloseDicomModule()
{
	DeallocateDicomDictionary();
}


// Dicom elements that may be of special interest for logging, storing or processing are listed in the following table.
DICOM_ELEMENT_SEMANTICS		SpecialDicomElementList[] =
	{
		{ 0x0002, 0x0000, 0,							DATA_LOADING_TYPE_INT,  offsetof( DICOM_HEADER_SUMMARY, MetadataGroupLength )			, 0, 0, 0 },
		{ 0x0002, 0x0001, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, FileMetaInformationVersion )	, 0, 0, 0 },
		{ 0x0002, 0x0002, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, MediaStorageSOPClassUID )		, 0, 0, 0 },
		{ 0x0002, 0x0003, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, MediaStorageSOPInstanceUID )	, 0, 0, 0 },
		{ 0x0002, 0x0010, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, TransferSyntaxUniqueIdentifier ), 0, 0, 0 },
		{ 0x0002, 0x0012, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, ImplementationClassUID )		, 0, 0, 0 },
		{ 0x0002, 0x0013, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, ImplementationVersionName )		, 0, 0, 0 },
		{ 0x0002, 0x0016, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, SourceAE_TITLE )				, 0, 0, 0 },
		{ 0x0002, 0x0102, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, DestinationAE_TITLE )			, 0, 0, 0 },

		{ 0x0008, 0x0008, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, ImageTypeUniqueIdentifier )		, 0, 0, 0 },
		{ 0x0008, 0x0012, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, DataCreationDate )				, 0, 0, 0 },
		{ 0x0008, 0x0013, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, DataCreationTime )				, 0, 0, 0 },
		{ 0x0008, 0x0014, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, DataCreatorUniqueIdentifier )	, 0, 0, 0 },
		{ 0x0008, 0x0016, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, SOPClassUniqueIdentifier )		, 0, 0, 0 },
		{ 0x0008, 0x0018, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, SOPInstanceUniqueIdentifier )	, 0, 0, 0 },
		{ 0x0008, 0x0020, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, StudyDate )						, 0, 0, 0 },
		{ 0x0008, 0x0021, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, SeriesDate )					, 0, 0, 0 },
		{ 0x0008, 0x0022, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, AcquisitionDate )				, 0, 0, 0 },
		{ 0x0008, 0x0030, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, StudyTime )						, 0, 0, 0 },
		{ 0x0008, 0x0031, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, SeriesTime )					, 0, 0, 0 },
		{ 0x0008, 0x0032, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, AcquisitionTime )				, 0, 0, 0 },
		{ 0x0008, 0x0050, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, AccessionNumber )				, 0, 0, 0 },
		{ 0x0008, 0x0070, 0,							DATA_LOADING_TYPE_CALIBR | DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, Manufacturer )	, 0, 0, 0 },
		{ 0x0008, 0x0080, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, InstitutionName )				, 0, 0, 0 },
		{ 0x0008, 0x0090, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_NAME, offsetof( DICOM_HEADER_SUMMARY, ReferringPhysician )			, 0, 0, 0 },
		{ 0x0008, 0x1030, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, StudyDescription )				, 0, 0, 0 },
		{ 0x0008, 0x103E, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, SeriesDescription )				, 0, 0, 0 },
		{ 0x0008, 0x1080, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, AdmittingDiagnosesDescription )	, 0, 0, 0 },
		{ 0x0008, 0x1090, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, ManufacturersModelName )		, 0, 0, 0 },

		{ 0x0010, 0x0010, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_NAME, offsetof( DICOM_HEADER_SUMMARY, PatientName )					, 0, 0, 0 },
		{ 0x0010, 0x0020, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, PatientID )						, 0, 0, 0 },
		{ 0x0010, 0x0030, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, PatientBirthDate )				, 0, 0, 0 },
		{ 0x0010, 0x0040, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, PatientSex )					, 0, 0, 0 },
		{ 0x0010, 0x1010, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, PatientAge )					, 0, 0, 0 },
		{ 0x0010, 0x21B0, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, AdditionalPatientHistory )		, 0, 0, 0 },
		{ 0x0010, 0x4000, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, PatientComments )				, 0, 0, 0 },

		{ 0x0018, 0x0015, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, BodyPartExamined )				, 0, 0, 0 },
		{ 0x0018, 0x1164, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, ImagerPixelSpacing )			, 0, 0, 0 },
		{ 0x0018, 0x5100, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, PatientPosition )				, 0, 0, 0 },
		{ 0x0018, 0x5101, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, ViewPosition )					, 0, 0, 0 },

		{ 0x0020, 0x000D, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, StudyInstanceUID )				, 0, 0, 0 },
		{ 0x0020, 0x000E, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, SeriesInstanceUID )				, 0, 0, 0 },
		{ 0x0020, 0x0010, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, StudyID )						, 0, 0, 0 },
		{ 0x0020, 0x0011, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, SeriesNumber )					, 0, 0, 0 },
		{ 0x0020, 0x0012, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, AcquisitionNumber )				, 0, 0, 0 },
		{ 0x0020, 0x0013, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, InstanceNumber )				, 0, 0, 0 },
		{ 0x0020, 0x1002, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, ImagesInAcquisition )			, 0, 0, 0 },
		{ 0x0020, 0x4000, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, ImageComments )					, 0, 0, 0 },

		{ 0x0008, 0x0060, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_CALIBR | DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, Modality )	, 0, TRUE, 0 },
		{ 0x0028, 0x0002, 0,							DATA_LOADING_TYPE_INT,	offsetof( DICOM_HEADER_SUMMARY, SamplesPerPixel )				, 0, 0, 0 },
		{ 0x0028, 0x0004, 0, DATA_LOADING_TYPE_CALIBR | DATA_LOADING_TYPE_INT,	offsetof( DICOM_HEADER_SUMMARY, PhotometricInterpretation )		, 0, 0, 0 },
		{ 0x0028, 0x0010, 0, DATA_LOADING_TYPE_CALIBR | DATA_LOADING_TYPE_INT,	offsetof( DICOM_HEADER_SUMMARY, ImageRows )						, 0, 0, 0 },
		{ 0x0028, 0x0011, 0, DATA_LOADING_TYPE_CALIBR | DATA_LOADING_TYPE_INT,	offsetof( DICOM_HEADER_SUMMARY, ImageColumns )					, 0, 0, 0 },
		{ 0x0028, 0x0100, 0, DATA_LOADING_TYPE_CALIBR | DATA_LOADING_TYPE_INT,	offsetof( DICOM_HEADER_SUMMARY, BitsAllocated )					, 0, 0, 0 },
		{ 0x0028, 0x0101, 0, DATA_LOADING_TYPE_CALIBR | DATA_LOADING_TYPE_INT,	offsetof( DICOM_HEADER_SUMMARY, BitsStored )					, 0, 0, 0 },
		{ 0x0028, 0x0102, 0, DATA_LOADING_TYPE_CALIBR | DATA_LOADING_TYPE_INT,	offsetof( DICOM_HEADER_SUMMARY, HighBit )						, 0, 0, 0 },
		//
		// Image calibration data:
		//
		{ 0x0028, 0x0103, 0,							DATA_LOADING_TYPE_CALIBR,	offsetof( DICOM_HEADER_SUMMARY, CalibrationInfo )			, 0, 0, 0 },
		{ 0x0028, 0x1050, 0,							DATA_LOADING_TYPE_CALIBR,	offsetof( DICOM_HEADER_SUMMARY, CalibrationInfo )			, 0, 0, 0 },
		{ 0x0028, 0x1051, 0,							DATA_LOADING_TYPE_CALIBR,	offsetof( DICOM_HEADER_SUMMARY, CalibrationInfo )			, 0, 0, 0 },
		{ 0x0028, 0x1052, 0,							DATA_LOADING_TYPE_CALIBR,	offsetof( DICOM_HEADER_SUMMARY, CalibrationInfo )			, 0, 0, 0 },
		{ 0x0028, 0x1053, 0,							DATA_LOADING_TYPE_CALIBR,	offsetof( DICOM_HEADER_SUMMARY, CalibrationInfo )			, 0, 0, 0 },
		{ 0x0028, 0x1054, 0,							DATA_LOADING_TYPE_CALIBR,	offsetof( DICOM_HEADER_SUMMARY, CalibrationInfo )			, 0, 0, 0 },
		{ 0x0028, 0x1056, 0,							DATA_LOADING_TYPE_CALIBR,	offsetof( DICOM_HEADER_SUMMARY, CalibrationInfo )			, 0, 0, 0 },
		{ 0x0028, 0x3000, 0,							DATA_LOADING_TYPE_CALIBR,	offsetof( DICOM_HEADER_SUMMARY, CalibrationInfo )			, 0, 0, 0 },
		{ 0x0028, 0x3010, 0,							DATA_LOADING_TYPE_CALIBR,	offsetof( DICOM_HEADER_SUMMARY, CalibrationInfo )			, 0, 0, 0 },
		{ 0x0028, 0x3002, 0,							DATA_LOADING_TYPE_CALIBR,	offsetof( DICOM_HEADER_SUMMARY, CalibrationInfo )			, 0, 0, 0 },
		{ 0x0028, 0x3004, 0,							DATA_LOADING_TYPE_CALIBR,	offsetof( DICOM_HEADER_SUMMARY, CalibrationInfo )			, 0, 0, 0 },
		{ 0x0028, 0x3006, 0,							DATA_LOADING_TYPE_CALIBR,	offsetof( DICOM_HEADER_SUMMARY, CalibrationInfo )			, 0, 0, 0 },

		{ 0x50F1, 0x1020, 0,							DATA_LOADING_TYPE_CALIBR,	offsetof( DICOM_HEADER_SUMMARY, CalibrationInfo )			, 0, 0, 0 },	// Fujifilm private element for distinguishing format variations.

		{ 0, 0, 0, 0 }
	};


void InitSpecialDicomElements()
{
	BOOL						bFinished;
	DICOM_ELEMENT_SEMANTICS		*pDicomElementInfo;
	int							nSpecialElement;

	nSpecialElement = 0;
	bFinished = FALSE;
	while ( !bFinished )
		{
		pDicomElementInfo = &SpecialDicomElementList[ nSpecialElement ];
		if ( pDicomElementInfo -> Group == 0 && pDicomElementInfo -> Element == 0 )
			bFinished = TRUE;
		else
			{
			pDicomElementInfo -> DicomSequenceLevel = -1;		// Preset each item to not found.
			if ( pDicomElementInfo -> bRequiredElement )	// Preset the required element not found flag.
				pDicomElementInfo -> bRequiredElementPresent = FALSE;
			}
		nSpecialElement++;
		}
}


BOOL IsSpecialDicomElement( TAG DicomElementTag, DICOM_ELEMENT_SEMANTICS **ppDicomElementInfo )
{
	BOOL						bFinished;
	BOOL						bIsSpecial;
	DICOM_ELEMENT_SEMANTICS		*pDicomElementInfo;
	int							nSpecialElement;

	nSpecialElement = 0;
	bFinished = FALSE;
	while ( !bFinished )
		{
		pDicomElementInfo = &SpecialDicomElementList[ nSpecialElement ];
		if ( pDicomElementInfo -> Group == 0 && pDicomElementInfo -> Element == 0 )
			{
			bIsSpecial = FALSE;
			bFinished = TRUE;
			*ppDicomElementInfo = 0;
			}
		else if ( pDicomElementInfo -> Group == DicomElementTag.Group && pDicomElementInfo -> Element == DicomElementTag.Element )
			{
			bIsSpecial = TRUE;
			bFinished = TRUE;
			*ppDicomElementInfo = pDicomElementInfo;
			}
		nSpecialElement++;
		}

	return bIsSpecial;
}


BOOL LoadDicomHeaderElement( DICOM_ELEMENT *pDicomElement, DICOM_HEADER_SUMMARY *pDicomHeader )
{
	DICOM_ELEMENT_SEMANTICS		*pDicomElementInfo;
	long						DataOffset;
	unsigned short				DataLoadingType;
	char						**ppTextDataValue;
	BOOL						bIsLoadable;
	
	bIsLoadable = FALSE;
	if ( pDicomElement -> Tag.Group == 0x0028 && pDicomElement -> Tag.Element == 0x0010 )
		bIsLoadable = FALSE;
	if ( IsSpecialDicomElement( pDicomElement -> Tag, &pDicomElementInfo ) &&										// If this is a semantically significant element (as far as BViewer is concerned) AND ...
				( pDicomElementInfo -> DicomSequenceLevel == -1														// ...this element hasn't been loaded yet...
				|| (char)pDicomElement -> SequenceNestingLevel < pDicomElementInfo -> DicomSequenceLevel ) )		// ...or this element is from the primary sequence and the previous one wasn't.
		{
		if ( pDicomElementInfo != 0 && pDicomElementInfo -> DataStructureOffset >= 0 )
			{
			bIsLoadable = TRUE;
			if ( pDicomElementInfo -> bRequiredElement )	// If this is a required element, mark it as found.
				pDicomElementInfo -> bRequiredElementPresent = TRUE;

			DataLoadingType = pDicomElementInfo -> DataLoadingType;
			DataOffset = pDicomElementInfo -> DataStructureOffset;
			if ( ( DataLoadingType & ( DATA_LOADING_TYPE_TEXT | DATA_LOADING_TYPE_NAME | DATA_LOADING_TYPE_INT | DATA_LOADING_TYPE_FLOAT ) ) != 0 )
				{
				// Point to the location in the data structure.
				ppTextDataValue = (char**)( (char*)pDicomHeader + DataOffset );
				// Copy the allocated character string buffer pointer into the data structure.
				*ppTextDataValue = pDicomElement -> pConvertedValue;
				}
			if ( ( DataLoadingType & DATA_LOADING_TYPE_CALIBR ) != 0 )
				{
				LoadImageCalibrationData( pDicomElement, pDicomHeader );
				}
			pDicomElementInfo -> DicomSequenceLevel = (char)pDicomElement -> SequenceNestingLevel;
			}
		}

	return TRUE;		// Never deallocate the value buffer.  This is now handled during the Dicom element list deallocation.
}


BOOL IncludeThisElementInSummaryLog( TAG DicomElementTag )
{
	BOOL						bOkToLog = FALSE;
	DICOM_ELEMENT_SEMANTICS		*pDicomElementInfo;
	
	if ( IsSpecialDicomElement( DicomElementTag, &pDicomElementInfo ) )
		{
		if ( pDicomElementInfo -> LogSpecifications & MESSAGE_TYPE_SUPPLEMENTARY )
			bOkToLog = TRUE;		// Log this Dicom Element.
		}
	
	return bOkToLog;
}


void LogDicomElement( DICOM_ELEMENT *pDicomElement, long SequenceNestingLevel )
{
	char					TextLine[ MAX_LOGGING_STRING_LENGTH ];
	char					SummaryTextLine[ MAX_LOGGING_STRING_LENGTH ];
	char					TextField[ MAX_LOGGING_STRING_LENGTH ];
	char					TextValue[ MAX_LOGGING_STRING_LENGTH ];
	char					ValueRepresentation[ 20 ];
	int						nCharOffset;
	int						nCharSummaryOffset;
	int						nCharEOL;
	int						nCharSummaryEOL;
	long					nValueSizeInBytes;
	unsigned long			nValue;
	double					FloatingPointValue;
	BOOL					bIncludeInSummaryLog;
	BOOL					bPrivateData;
	int						nChars;

	bIncludeInSummaryLog = ( IncludeThisElementInSummaryLog( pDicomElement -> Tag ) );

	memset( TextLine, ' ', MAX_LOGGING_STRING_LENGTH );
	memset( SummaryTextLine, ' ', MAX_LOGGING_STRING_LENGTH );
	nCharEOL = 0;
	nCharSummaryEOL = 0;
		
	nCharOffset = 4 + 4 * SequenceNestingLevel;
	nCharSummaryOffset = 4;
		
	nValueSizeInBytes = pDicomElement -> ValueLength;

	ValueRepresentation[ 0 ] = (char)( ( (unsigned short)pDicomElement -> ValueRepresentation >> 8 ) & 0x00FF );
	ValueRepresentation[ 1 ] = (char)( ( (unsigned short)pDicomElement -> ValueRepresentation ) & 0x00FF );
	ValueRepresentation[ 2 ] = '\0';

	sprintf( TextField, "Dicom Element ( %X, %X )", pDicomElement -> Tag.Group, pDicomElement -> Tag.Element );
	memcpy( &TextLine[ nCharOffset ], TextField, strlen( TextField ) );
	nCharEOL = nCharOffset + (int)strlen( TextField );

	nCharOffset += 29;
	sprintf( TextField, " %2s  %d", ValueRepresentation, pDicomElement -> ValueLength );
	memcpy( &TextLine[ nCharOffset ], TextField, strlen( TextField ) );
	nCharEOL = nCharOffset + (int)strlen( TextField );

	nCharOffset += 12;
	if ( pDicomElement -> ValueMultiplicity > 1 )
		{
		sprintf( TextField, " %d", pDicomElement -> ValueMultiplicity );
		memcpy( &TextLine[ nCharOffset ], TextField, strlen( TextField ) );
		nCharEOL = nCharOffset + (int)strlen( TextField );
		}

	nCharOffset += 7;
	if ( pDicomElement -> pMatchingDictionaryItem != 0 )
		sprintf( TextField, "%s:", pDicomElement -> pMatchingDictionaryItem -> Description );
	else
		TextField[ 0 ] = '\0';
	memcpy( &TextLine[ nCharOffset ], TextField, strlen( TextField ) );
	nCharEOL = nCharOffset + (int)strlen( TextField );

	memcpy( &SummaryTextLine[ nCharSummaryOffset ], TextField, strlen( TextField ) );
	nCharSummaryEOL = nCharSummaryOffset + (int)strlen( TextField );


	if ( pDicomElement -> pConvertedValue != 0 )
		{
		nCharOffset += 32;
		nCharSummaryOffset += 32;
		switch( pDicomElement -> ValueRepresentation )
			{
			case SS:			// Signed short.
				// Log these integer values with %d.
				strcpy( TextField, "" );
				if ( pDicomElement -> ValueMultiplicity >= 1 && pDicomElement -> ValueMultiplicity <= 5 )
					{
					for ( nValue = 0; nValue < pDicomElement -> ValueMultiplicity; nValue++ )
						{
						if ( nValue > 0 )
							strcat( TextField, "|" );
						sprintf( TextValue, "%d", *( (short*)( pDicomElement -> pConvertedValue + nValue * sizeof(short) ) ) );
						strcat( TextField, TextValue );
						}
					}
				break;
			case US:			// Unsigned short.
			case AT:			// Attribute tag.
				// Log these integer values with %d.
				strcpy( TextField, "" );
				if ( pDicomElement -> ValueMultiplicity >= 1 && pDicomElement -> ValueMultiplicity <= 5 )
					{
					for ( nValue = 0; nValue < pDicomElement -> ValueMultiplicity; nValue++ )
						{
						if ( nValue > 0 )
							strcat( TextField, "|" );
						sprintf( TextValue, "%d", *( (unsigned short*)( pDicomElement -> pConvertedValue + nValue * sizeof(unsigned short) ) ) );
						strcat( TextField, TextValue );
						}
					}
				break;
			case SL:			// Signed long.
			case UL:			// Unsigned long.
				// Log these integer values with %d.
				strcpy( TextField, "" );
				if ( pDicomElement -> ValueMultiplicity >= 1 && pDicomElement -> ValueMultiplicity <= 5 )
					{
					for ( nValue = 0; nValue < pDicomElement -> ValueMultiplicity; nValue++ )
						{
						if ( nValue > 0 )
							strcat( TextField, "|" );
						sprintf( TextValue, "%d", *( (long*)( pDicomElement -> pConvertedValue + nValue * sizeof(long) ) ) );
						strcat( TextField, TextValue );
						}
					}
				break;
			case FL:			// Float (single precision).
				// Log these floating point values with %f.
				strcpy( TextField, "" );
				if ( pDicomElement -> ValueMultiplicity >= 1 && pDicomElement -> ValueMultiplicity <= 5 )
					{
					for ( nValue = 0; nValue < pDicomElement -> ValueMultiplicity; nValue++ )
						{
						if ( nValue > 0 )
							strcat( TextField, "|" );
						sprintf( TextValue, "%10.3f", *( (float*)( pDicomElement -> pConvertedValue + nValue * sizeof(float) ) ) );
						strcat( TextField, TextValue );
						}
					}
				break;
			case FD:			// Float (double precision).
				// Log these floating point values with %f.
				strcpy( TextField, "" );
				if ( pDicomElement -> ValueMultiplicity >= 1 && pDicomElement -> ValueMultiplicity <= 5 )
					{
					for ( nValue = 0; nValue < pDicomElement -> ValueMultiplicity; nValue++ )
						{
						if ( nValue > 0 )
							strcat( TextField, "|" );
						FloatingPointValue = *( (double*)( pDicomElement -> pConvertedValue + nValue * sizeof(double) ) );
						if ( FloatingPointValue > 10000000000000000.0 )
							strcpy( TextValue, "FD Coding Problem" );
						else
							{
							if ( FloatingPointValue < 0.00000000000000001 && FloatingPointValue != 0.0 )
								FloatingPointValue = 0.0;
							_snprintf( TextValue, MAX_LOGGING_STRING_LENGTH - 1, "%10.3f", FloatingPointValue );
							}
						strcat( TextField, TextValue );
						}
					}
				break;
			case LT:			// Long text.
			case ST:			// Short text.
			case UT:			// Unlimited text.
			case AE:			// Application entity.
			case AS:			// Age string.
			case CS:			// Code string.
			case DA:			// Date.
			case DS:			// Decimal string.
			case DT:			// Date time.
			case IS:			// Integer string.
			case LO:			// Long string.
			case SH:			// Short string.
			case TM:			// Time.
			case UI:			// Unique identifier.
				// Log these text strings with %s.
				strcpy( TextField, "" );
				strncat( TextField, pDicomElement -> pConvertedValue, MAX_LOGGING_STRING_LENGTH - 1 );
				break;
			case UN:			// Unknown:  Only log if this is a private data element.
				bPrivateData = ( ( pDicomElement -> Tag.Group & 0x0001 ) != 0 );
				if ( bPrivateData )
					{
					// Log these text strings with %s.
					strcpy( TextField, "" );
					strncat( TextField, pDicomElement -> pConvertedValue, MAX_LOGGING_STRING_LENGTH - 1 );
					}
				else
					{
					// Don't attempt to log these value.
					TextField[ 0 ] = '\0';
					}
				break;
			case PN:			// Person name.
				TextField[ 0 ] = '\0';
				CopyDicomNameToString( TextField, (PERSON_NAME*)pDicomElement -> pConvertedValue, MAX_LOGGING_STRING_LENGTH - 80 );
				break;
			case OB:			// Other byte string.
			case OW:			// Other word string.
			case SQ:			// Item sequence.
			default:
				// Don't attempt to log these value.
				TextField[ 0 ] = '\0';
				break;
			}
		nChars = strlen( TextField );
		if ( nChars > MAX_LOGGING_STRING_LENGTH - nCharOffset - 1 )
			nChars = MAX_LOGGING_STRING_LENGTH - nCharOffset - 4;
		memcpy( &TextLine[ nCharOffset ], TextField, nChars );
		nCharEOL = nCharOffset + nChars;
		if ( nChars < (int)strlen( TextField ) )
			{
			memcpy( &TextLine[ nCharEOL ], "...", 3 );
			nCharEOL += 3;
			}

		nChars = strlen( TextField );
		if ( nChars > MAX_LOGGING_STRING_LENGTH - nCharSummaryOffset - 1 )
			nChars = MAX_LOGGING_STRING_LENGTH - nCharSummaryOffset - 4;
		memcpy( &SummaryTextLine[ nCharSummaryOffset ], TextField, nChars );
		nCharSummaryEOL = nCharSummaryOffset + nChars;
		if ( nChars < (int)strlen( TextField ) )
			{
			memcpy( &SummaryTextLine[ nCharSummaryEOL ], "...", 3 );
			nCharSummaryEOL += 3;
			}
		}

	TextLine[ nCharEOL ] = '\0';
	SummaryTextLine[ nCharSummaryEOL ] = '\0';

	if ( IncludeThisElementInSummaryLog( pDicomElement -> Tag ) )
		LogMessage( SummaryTextLine, MESSAGE_TYPE_NORMAL_LOG | MESSAGE_TYPE_NO_TIME_STAMP | MESSAGE_TYPE_SUMMARY_ONLY );
	LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY | MESSAGE_TYPE_NO_TIME_STAMP );

}


BOOL CheckForRequiredDicomElements( DICOM_HEADER_SUMMARY *pDicomHeader )
{
	BOOL						bNoError = TRUE;
	BOOL						bFinished;
	DICOM_ELEMENT_SEMANTICS		*pDicomElementInfo;
	int							nSpecialElement;
	USER_NOTIFICATION			UserNoticeDescriptor;
	char						ElementDescriptionText[ 128 ];
	TAG							Tag;
	DICOM_DICTIONARY_ITEM		*pDictItem = 0;

	nSpecialElement = 0;
	bFinished = FALSE;
	while ( !bFinished )
		{
		pDicomElementInfo = &SpecialDicomElementList[ nSpecialElement ];
		if ( pDicomElementInfo -> Group == 0 && pDicomElementInfo -> Element == 0 )
			bFinished = TRUE;
		else if ( pDicomElementInfo -> bRequiredElement && pDicomElementInfo -> bRequiredElementPresent == FALSE )
			{
			bNoError = FALSE;
			// Notify the reader of the first encountered required element that is missing.
			Tag.Group = pDicomElementInfo -> Group;
			Tag.Element = pDicomElementInfo -> Element;
			pDictItem = GetDicomElementFromDictionary( Tag );
			if ( pDictItem != 0 )
				sprintf( ElementDescriptionText, "Dicom Element ( %X, %X )    %s", pDicomElementInfo -> Group, pDicomElementInfo -> Element, pDictItem -> Description );
			else
				sprintf( ElementDescriptionText, "Dicom Element ( %X, %X )", pDicomElementInfo -> Group, pDicomElementInfo -> Element );
			strcpy( UserNoticeDescriptor.Source, TransferService.ServiceName );
			UserNoticeDescriptor.ModuleCode = MODULE_DICOM;
			UserNoticeDescriptor.ErrorCode = DICOM_ERROR_REQUIRED_DICOM_ELEMENT_MISSING;

			UserNoticeDescriptor.TypeOfUserResponseSupported = USER_RESPONSE_TYPE_ERROR | USER_RESPONSE_TYPE_CONTINUE;
			UserNoticeDescriptor.UserNotificationCause = USER_NOTIFICATION_CAUSE_PRODUCT_PROCESSING_ERROR;
			UserNoticeDescriptor.UserResponseCode = 0L;
			sprintf( UserNoticeDescriptor.NoticeText, "The image file for \n\n%s, %s\n\ncould not be processed.", pDicomHeader -> PatientName -> pLastName, pDicomHeader -> PatientName -> pFirstName );
			strcpy( UserNoticeDescriptor.SuggestedActionText, "It is missing the required Dicom data element:\n" );
			strcat( UserNoticeDescriptor.SuggestedActionText, ElementDescriptionText );	
			UserNoticeDescriptor.TextLinesRequired = 9;
			SubmitUserNotification( &UserNoticeDescriptor );
			strcat( ElementDescriptionText, "  Required Dicom element is missing from this file.  Processing aborted." );
			LogMessage( ElementDescriptionText, MESSAGE_TYPE_ERROR );
			bFinished = TRUE;
			}
		nSpecialElement++;
		}

	return bNoError;
}


TRANSFER_SYNTAX GetTransferSyntaxForDicomElementParsing( unsigned char TransferSyntaxIndex )
{
	TRANSFER_SYNTAX			TransferSyntax;

	// If an encapsulated syntax is to be used, then the syntax for data elements needs to default to:
	TransferSyntax = LITTLE_ENDIAN | EXPLICIT_VR;
	switch ( TransferSyntaxIndex )
		{
		case LITTLE_ENDIAN_IMPLICIT_TRANSFER_SYNTAX:
			TransferSyntax = LITTLE_ENDIAN | IMPLICIT_VR;
			break;
		case LITTLE_ENDIAN_EXPLICIT_TRANSFER_SYNTAX:
			TransferSyntax = LITTLE_ENDIAN | EXPLICIT_VR;
			break;
		case BIG_ENDIAN_EXPLICIT_TRANSFER_SYNTAX:
			TransferSyntax = BIG_ENDIAN | EXPLICIT_VR;
			break;
		}

	return TransferSyntax;
}


// The following functions handle the buffers used for reading and storing the Dicom file
// information.  These buffers are linked members of the list at pDicomHeader -> ListOfInputBuffers.
void SaveBufferCursor( LIST_ELEMENT **ppBufferListElement, LIST_ELEMENT **ppSavedBufferListElement,
											unsigned long *pnSavedBufferBytesRemainingToBeProcessed )
{
	DICOM_DATA_BUFFER		*pSavedDicomBuffer;

	*ppSavedBufferListElement = *ppBufferListElement;
	pSavedDicomBuffer = (DICOM_DATA_BUFFER*)(*ppBufferListElement) -> pItem;
	*pnSavedBufferBytesRemainingToBeProcessed = pSavedDicomBuffer -> BytesRemainingToBeProcessed;
}


void RestoreBufferCursor( LIST_ELEMENT **ppBufferListElement, LIST_ELEMENT **ppSavedBufferListElement,
											unsigned long *pnSavedBufferBytesRemainingToBeProcessed )
{
	DICOM_DATA_BUFFER		*pSavedDicomBuffer;
	BOOL					bTrailingBufferNeedsResetting;
	LIST_ELEMENT			*pBufferListElement;
	DICOM_DATA_BUFFER		*pDicomBuffer;

	*ppBufferListElement = *ppSavedBufferListElement;
	pSavedDicomBuffer = (DICOM_DATA_BUFFER*)(*ppSavedBufferListElement) -> pItem;
	pSavedDicomBuffer -> BytesRemainingToBeProcessed = *pnSavedBufferBytesRemainingToBeProcessed;
	bTrailingBufferNeedsResetting = TRUE;
	pBufferListElement = (*ppBufferListElement) -> pNextListElement;
	while ( pBufferListElement != 0 && bTrailingBufferNeedsResetting )
		{
		pDicomBuffer = (DICOM_DATA_BUFFER*)pBufferListElement -> pItem;
		if ( pDicomBuffer -> BytesRemainingToBeProcessed == pDicomBuffer -> DataSize )
			bTrailingBufferNeedsResetting = FALSE;
		else
			pDicomBuffer -> BytesRemainingToBeProcessed = pDicomBuffer -> DataSize;
		pBufferListElement = pBufferListElement -> pNextListElement;
		}
}


// This function requires that sufficient memory be available at the destination in order
// to receive nBytesNeeded bytes.  If the end of an input buffer is reached, additional
// bytes are copied as needed from the next buffer in the list.
// This function returns FALSE if an attempt is made to read past the end of data in the
// final buffer in the buffer list.
BOOL CopyBytesFromBuffer( char *pDestinationAddress, unsigned long nBytesNeeded, LIST_ELEMENT **ppBufferListElement )
{
	BOOL					bNoError = TRUE;
	DICOM_DATA_BUFFER		*pDicomBuffer;
	LIST_ELEMENT			*pBufferListElement;
	unsigned long			nBufferBytesProcessed;
	char					*pBufferReadPoint;

	pBufferListElement = *ppBufferListElement;
	pDicomBuffer = (DICOM_DATA_BUFFER*)pBufferListElement -> pItem;
	nBufferBytesProcessed = pDicomBuffer -> DataSize - pDicomBuffer -> BytesRemainingToBeProcessed;
	pBufferReadPoint = pDicomBuffer -> pBeginningOfDicomData + nBufferBytesProcessed;
	do
		{
		if ( pDicomBuffer -> BytesRemainingToBeProcessed >= nBytesNeeded )
			{
			memcpy( pDestinationAddress, pBufferReadPoint, nBytesNeeded );
			pDicomBuffer -> BytesRemainingToBeProcessed -= nBytesNeeded;
			pBufferReadPoint += nBytesNeeded;
			nBytesNeeded = 0;
			}
		else
			{
			if ( pDicomBuffer -> BytesRemainingToBeProcessed > 0 )
				{
				memcpy( pDestinationAddress, pBufferReadPoint, pDicomBuffer -> BytesRemainingToBeProcessed );
				pDestinationAddress += pDicomBuffer -> BytesRemainingToBeProcessed;
				nBytesNeeded -= pDicomBuffer -> BytesRemainingToBeProcessed;
				}
			pDicomBuffer -> BytesRemainingToBeProcessed = 0;
			// Advance into the next buffer in the sequence.
			pBufferListElement = pBufferListElement -> pNextListElement;
			*ppBufferListElement = pBufferListElement;
			if ( pBufferListElement == 0 )
				bNoError = FALSE;
			else
				{
				pDicomBuffer = (DICOM_DATA_BUFFER*)pBufferListElement -> pItem;
				nBufferBytesProcessed = pDicomBuffer -> DataSize - pDicomBuffer -> BytesRemainingToBeProcessed;
				pBufferReadPoint = pDicomBuffer -> pBeginningOfDicomData + nBufferBytesProcessed;
				}
			}
		}
	while ( nBytesNeeded > 0 && bNoError );

	return bNoError;
}


void ResetOutputBufferCursors( DICOM_HEADER_SUMMARY *pDicomHeader )
{
	DICOM_DATA_BUFFER		*pDicomBuffer;
	LIST_ELEMENT			*pBufferListElement;

	pBufferListElement = pDicomHeader -> ListOfOutputBuffers;
	while ( pBufferListElement != 0 )
		{
		pDicomBuffer = (DICOM_DATA_BUFFER*)pBufferListElement -> pItem;
		if ( pDicomBuffer != 0 )
			pDicomBuffer -> BytesRemainingToBeProcessed = pDicomBuffer -> DataSize;
		pBufferListElement = pBufferListElement -> pNextListElement;
		}
}


// This function requires that sufficient memory be available at the destination in order
// to receive nBytesNeeded bytes.  If the end of an output buffer is reached, a new buffer
// is allocated, and additional bytes are copied as needed to the next buffer in the list.
BOOL CopyBytesToBuffer( char *pSourceAddress, unsigned long nBytesNeeded, unsigned long *pnBytesCopied, LIST_ELEMENT **ppBufferListElement )
{
	BOOL					bNoError = TRUE;
	DICOM_DATA_BUFFER		*pDicomBuffer;
	LIST_ELEMENT			*pBufferListElement;
	LIST_ELEMENT			*pNewBufferListElement;
	unsigned long			nBufferBytesProcessed;
	char					*pBufferWritePoint;
	char					*pNewBuffer;

	pBufferListElement = *ppBufferListElement;
	pDicomBuffer = (DICOM_DATA_BUFFER*)pBufferListElement -> pItem;
	nBufferBytesProcessed = pDicomBuffer -> DataSize;
	pBufferWritePoint = pDicomBuffer -> pBeginningOfDicomData + nBufferBytesProcessed;
	*pnBytesCopied = 0L;
	do
		{
		if ( pDicomBuffer -> BytesRemainingToBeProcessed >= nBytesNeeded )
			{
			memcpy( pBufferWritePoint, pSourceAddress, nBytesNeeded );
			*pnBytesCopied += nBytesNeeded;
			pDicomBuffer -> BytesRemainingToBeProcessed -= nBytesNeeded;
			pDicomBuffer -> DataSize += nBytesNeeded;
			pBufferWritePoint += nBytesNeeded;
			nBytesNeeded = 0;
			}
		else
			{
			if ( pDicomBuffer -> BytesRemainingToBeProcessed > 0 )
				{
				memcpy( pBufferWritePoint, pSourceAddress, pDicomBuffer -> BytesRemainingToBeProcessed );
				*pnBytesCopied += pDicomBuffer -> BytesRemainingToBeProcessed;
				pSourceAddress += pDicomBuffer -> BytesRemainingToBeProcessed;
				nBytesNeeded -= pDicomBuffer -> BytesRemainingToBeProcessed;
				pDicomBuffer -> DataSize += pDicomBuffer -> BytesRemainingToBeProcessed;
				pDicomBuffer -> BytesRemainingToBeProcessed = 0L;
				}

			// Create the next buffer in the sequence.
			pNewBuffer = (char*)malloc( MAX_DICOM_READ_BUFFER_SIZE );		// Allocate a 64K buffer.
			pDicomBuffer = (DICOM_DATA_BUFFER*)malloc( sizeof(DICOM_DATA_BUFFER) );
			pNewBufferListElement = (LIST_ELEMENT*)malloc( sizeof(LIST_ELEMENT) );
			if ( pNewBuffer == 0 || pDicomBuffer == 0 || pNewBufferListElement == 0 )
				{
				bNoError = FALSE;
				RespondToError( MODULE_DICOM, DICOM_ERROR_INSUFFICIENT_MEMORY );
				}
			else
				{
				pDicomBuffer -> pBuffer = pNewBuffer;
				pDicomBuffer -> pBeginningOfDicomData = pNewBuffer;
				pDicomBuffer -> BufferSize = MAX_DICOM_READ_BUFFER_SIZE;
				pDicomBuffer -> DataSize = 0L;
				pDicomBuffer -> BytesRemainingToBeProcessed = MAX_DICOM_READ_BUFFER_SIZE;
				// Link the new buffer to the list.
				pNewBufferListElement -> pNextListElement = 0;
				pNewBufferListElement -> pItem = (void*)pDicomBuffer;
				pBufferListElement -> pNextListElement = pNewBufferListElement;
				}
			if ( bNoError )
				{
				pBufferListElement = pNewBufferListElement;
				*ppBufferListElement = pBufferListElement;
				nBufferBytesProcessed = 0L;
				pBufferWritePoint = pDicomBuffer -> pBeginningOfDicomData;
				}
			}
		}
	while ( nBytesNeeded > 0 && bNoError );

	return bNoError;
}


void DeallocateInputImageBuffers( DICOM_HEADER_SUMMARY *pDicomHeader )
{
	LIST_ELEMENT			*pBufferListElement;
	LIST_ELEMENT			*pFirstBufferListElement;
	LIST_ELEMENT			*pPrevBufferListElement;
	DICOM_DATA_BUFFER		*pDicomBuffer;

	pFirstBufferListElement = pDicomHeader -> pBufferElementWithImageStart;
	// Deallocate all the input buffers after the one where the image starts.
	// This will retain all of the Dicom data element information except for the
	// image pixel data.
	if ( pFirstBufferListElement != 0 )
		{
		pBufferListElement = pFirstBufferListElement;
		while ( pBufferListElement != 0 )
			{
			pPrevBufferListElement = pBufferListElement;
			pBufferListElement = pBufferListElement -> pNextListElement;
			if ( pPrevBufferListElement != pFirstBufferListElement )
				free( pPrevBufferListElement );
			if ( pBufferListElement != 0 )
				{
				pDicomBuffer = (DICOM_DATA_BUFFER*)pBufferListElement -> pItem;
				free( pDicomBuffer -> pBuffer );
				free( pDicomBuffer );
				}
			}
		pFirstBufferListElement -> pNextListElement = 0;
		}
}


void DeallocateInputBuffers( DICOM_HEADER_SUMMARY *pDicomHeader )
{
	LIST_ELEMENT			*pBufferListElement;
	LIST_ELEMENT			*pPrevBufferListElement;
	DICOM_DATA_BUFFER		*pDicomBuffer;

	pBufferListElement = pDicomHeader -> ListOfInputBuffers;
	while ( pBufferListElement != 0 )
		{
		pPrevBufferListElement = pBufferListElement;
		pDicomBuffer = (DICOM_DATA_BUFFER*)pBufferListElement -> pItem;
		if ( pDicomBuffer != 0 )
			{
			if ( pDicomBuffer -> pBuffer != 0 )
				free( pDicomBuffer -> pBuffer );
			free( pDicomBuffer );
			}
		pBufferListElement = pBufferListElement -> pNextListElement;
		free( pPrevBufferListElement );
		}
	pDicomHeader -> ListOfInputBuffers = 0;
}


void DeallocateOutputBuffers( DICOM_HEADER_SUMMARY *pDicomHeader )
{
	LIST_ELEMENT			*pBufferListElement;
	LIST_ELEMENT			*pPrevBufferListElement;
	DICOM_DATA_BUFFER		*pDicomBuffer;

	pBufferListElement = pDicomHeader -> ListOfOutputBuffers;
	while ( pBufferListElement != 0 )
		{
		pPrevBufferListElement = pBufferListElement;
		pDicomBuffer = (DICOM_DATA_BUFFER*)pBufferListElement -> pItem;
		if ( pDicomBuffer != 0 )
			{
			if ( pDicomBuffer -> pBuffer != 0 )
				free( pDicomBuffer -> pBuffer );
			free( pDicomBuffer );
			}
		pBufferListElement = pBufferListElement -> pNextListElement;
		free( pPrevBufferListElement );
		}
	pDicomHeader -> ListOfOutputBuffers = 0;
}


void DeallocateDicomElement( DICOM_ELEMENT *pDicomElement )
{
	PERSON_NAME				*pNameBuffer;

	if ( pDicomElement != 0 )
		{
		if ( (char*)pDicomElement -> Value.UN != 0 )
			free( (char*)pDicomElement -> Value.UN );
		pDicomElement -> Value.UN = 0;
		if ( pDicomElement -> ValueRepresentation == PN )
			{
			pNameBuffer = (PERSON_NAME*)pDicomElement -> pConvertedValue;
			if ( pNameBuffer != 0 )
				{
				if ( pNameBuffer -> pFirstName != 0 )
					free( pNameBuffer -> pFirstName );
				if ( pNameBuffer -> pLastName != 0 )
					free( pNameBuffer -> pLastName );
				if ( pNameBuffer -> pMiddleName != 0 )
					free( pNameBuffer -> pMiddleName );
				if ( pNameBuffer -> pPrefix != 0 )
					free( pNameBuffer -> pPrefix );
				if ( pNameBuffer -> pSuffix != 0 )
					free( pNameBuffer -> pSuffix );
				free( pNameBuffer );
				}
			}
		else if ( pDicomElement -> pConvertedValue != 0 )
			free( pDicomElement -> pConvertedValue );
		pDicomElement -> pConvertedValue = 0;
		free( pDicomElement );
		}
}


void DeallocateListOfDicomElements( DICOM_HEADER_SUMMARY *pDicomHeader )
{
	LIST_ELEMENT			*pDicomElementListElement;
	LIST_ELEMENT			*pPrevDicomElementListElement;
	DICOM_ELEMENT			*pDicomElement;

	pDicomElementListElement = pDicomHeader -> ListOfDicomElements;
	pPrevDicomElementListElement = 0;
	while ( pDicomElementListElement != 0 )
		{
		pDicomElement = (DICOM_ELEMENT*)pDicomElementListElement -> pItem;
		if ( pDicomElement -> Tag.Group != 0x7fe0 )
			{
			DeallocateDicomElement( pDicomElement );
			pDicomElementListElement -> pItem = 0;
			}
		else if ( pDicomElement -> Tag.Group == 0x7fe0 )
			{
			free( pDicomElement );
			}
		pPrevDicomElementListElement = pDicomElementListElement;
		pDicomElementListElement = pDicomElementListElement -> pNextListElement;
		if ( pPrevDicomElementListElement != 0 )
			free( pPrevDicomElementListElement );
		}
	pDicomHeader -> ListOfDicomElements = 0;
}


BOOL ReadDicomHeaderInfo( char *DicomFileSpecification, EXAM_INFO *pExamInfo, DICOM_HEADER_SUMMARY **ppDicomHeader, BOOL bLogDicomElements )
{
	BOOL						bNoError = TRUE;
	FILE						*pDicomFile;
	DICOM_DATA_BUFFER			*pDicomBuffer;
	LIST_ELEMENT				*pBufferListElement;
	BOOL						bEndOfFile;
	long						nBytesRead;
	char						*pBuffer;
	char						*pChar;
	int							SystemErrorNumber;
	DICOM_HEADER_SUMMARY		*pDicomHeader;
	unsigned long				TotalImageSize;
	unsigned long				ExpandedImageSize;
	size_t						nBytesParsed;
	TAG							DicomElementTag;
	ABSTRACT_RECORD_TEXT_LINE	*pAbstractDataLineInfo;
	char						TextLine[ 1096 ];

	LogMessage( "Processing new image file.", MESSAGE_TYPE_NORMAL_LOG );
	InitSpecialDicomElements();
	pDicomHeader = (DICOM_HEADER_SUMMARY*)calloc( 1, sizeof(DICOM_HEADER_SUMMARY) );
	if ( pDicomHeader == 0 )
		{
		*ppDicomHeader = 0;
		bNoError = FALSE;
		RespondToError( MODULE_DICOM, DICOM_ERROR_INSUFFICIENT_MEMORY );
		}
	else
		{
		*ppDicomHeader = pDicomHeader;
		InitDicomHeaderSummary( pDicomHeader );
		}
	if ( bNoError )
		{
		pDicomFile = OpenDicomFile( DicomFileSpecification );
		bNoError = ( pDicomFile != 0 );
		}
	// Examine the file name for an "AutoLoad_" prefix.
	pDicomHeader -> bAutoLoadThisImage = FALSE;
	pChar = strrchr( DicomFileSpecification, '\\' );			// Locate the final '\' in the file path.
	pChar++;
	if ( strncmp( pChar, "AutoLoad_", 9 ) == 0 )
		{
		// This is a file to be automatically loaded into BViewer and brought up for immediate viewing.
		pDicomHeader -> bAutoLoadThisImage = TRUE;
		}

	// Read the Dicom data from the file into memory buffers.
	bEndOfFile = FALSE;
	pDicomHeader -> ListOfInputBuffers = 0;
	pDicomHeader -> ListOfDicomElements = 0;
	pDicomHeader -> pBufferElementWithImageStart = 0;
	while ( bNoError && !bEndOfFile )
		{
		pBuffer = (char*)malloc( MAX_DICOM_READ_BUFFER_SIZE );		// Allocate a 64K buffer.
		pDicomBuffer = (DICOM_DATA_BUFFER*)malloc( sizeof(DICOM_DATA_BUFFER) );
		if ( pBuffer == 0 || pDicomBuffer == 0 )
			{
			bNoError = FALSE;
			RespondToError( MODULE_DICOM, DICOM_ERROR_INSUFFICIENT_MEMORY );
			}
		else
			{
			pDicomBuffer -> pBuffer = pBuffer;
			pDicomBuffer -> pBeginningOfDicomData = pBuffer;
			// Link the new buffer to the list.
			bNoError = AppendToList( &pDicomHeader -> ListOfInputBuffers, (void*)pDicomBuffer );
			}
		if ( bNoError )
			{
			// Read from the beginning of the file and fill up the allocated buffer (if sufficient file data exists).
			// All the interesting Dicom information will be at the beginning of the file.
			nBytesRead = (long)fread( pBuffer, 1, MAX_DICOM_READ_BUFFER_SIZE, pDicomFile );
			pDicomBuffer -> BufferSize = MAX_DICOM_READ_BUFFER_SIZE;
			pDicomBuffer -> DataSize = nBytesRead;
			pDicomBuffer -> BytesRemainingToBeProcessed = nBytesRead;
			if ( nBytesRead != MAX_DICOM_READ_BUFFER_SIZE )
				{
				if ( feof( pDicomFile ) )
					bEndOfFile = TRUE;
				else
					{
					bNoError = FALSE;
					SystemErrorNumber = ferror( pDicomFile );
					if ( SystemErrorNumber != 0 )
						{
						RespondToError( MODULE_DICOM, DICOM_ERROR_FILE_READ );
						LogMessage( strerror( SystemErrorNumber ), MESSAGE_TYPE_ERROR );
						}
					}
				}
			}
		}
	if ( pDicomFile != 0 )
		CloseDicomFile( pDicomFile );
	// Read and parse the sequence of Dicom data elements.
	pBufferListElement = pDicomHeader -> ListOfInputBuffers;
	if ( bNoError )
		bNoError = ParseDicomGroup2Info( &pBufferListElement, pDicomHeader, bLogDicomElements );

	if ( bNoError )
		bNoError = OpenNewAbstractRecord();
	if ( bNoError )
		{
		// Prepend the AE_TITLE for the selection destination.
		DicomElementTag.Group = 0x2100;
		DicomElementTag.Element = 0x0140;
		if ( pDicomHeader -> DestinationAE_TITLE != 0 )
			{
			strcpy( TextLine, "" );
			strncat( TextLine, pDicomHeader -> DestinationAE_TITLE, 16 );
			TrimBlanks( TextLine );
			bNoError = AddNewAbstractDataElement( DicomElementTag, TextLine );
			}
		else
			bNoError = AddNewAbstractDataElement( DicomElementTag, "" );
		}

	if ( bNoError )
		{
		bNoError = ParseDicomHeaderInfo( &pBufferListElement, pExamInfo, pDicomHeader, bLogDicomElements );
		}
	if ( bNoError )
		{
		nBytesParsed = 0;
		bNoError = ProcessDicomImageDataElements( &pBufferListElement, pDicomHeader, &nBytesParsed, TRUE );
		}
	if ( bNoError )
		{
		TotalImageSize = pDicomHeader -> ImageLengthInBytes;
		// Get the full expanded image size from the dicom header:
		ExpandedImageSize = (unsigned long)*pDicomHeader -> ImageRows *
								(unsigned long)*pDicomHeader -> ImageColumns *
								(unsigned long)*pDicomHeader -> BitsAllocated / 8;
		// If we don't trust the stored image encoding transfer syntax specification,
		// override the syntax read from the file metadata.
		if ( !pDicomHeader -> FileDecodingPlan.bTrustSpecifiedTransferSyntaxFromLocalStorage )
			{
			if ( ExpandedImageSize > (unsigned long)TotalImageSize )
				pDicomHeader -> FileDecodingPlan.ImageDataTransferSyntax = COMPRESSED_UNKNOWN;
			else
				pDicomHeader -> FileDecodingPlan.ImageDataTransferSyntax = UNCOMPRESSED;
			}
		}
	if ( bNoError )
		{
		pAbstractDataLineInfo = CreateNewAbstractRecords();
		pDicomHeader -> pAbstractDataLineList = pAbstractDataLineInfo;

		sprintf( TextLine, "    Abstracted:  %s", DicomFileSpecification );
		LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
		}

	return bNoError;
}


BOOL ParseDicomGroup2Info( LIST_ELEMENT **ppBufferListElement, DICOM_HEADER_SUMMARY *pDicomHeader, BOOL bLogDicomElements )
{
	BOOL					bNoError = TRUE;
	unsigned short			LocalStorageExample = 0x1234;
	char					TextLine[ MAX_LOGGING_STRING_LENGTH ];
	BOOL					bMoreHeaderInfoRemains = TRUE;
	DICOM_ELEMENT			*pDicomElement;
	unsigned char			SequenceNestingLevel = 0;
	TRANSFER_SYNTAX			CurrentTransferSyntax;
	BOOL					bMoreDicomElementsRemainInSequence = TRUE;
	size_t					nBytesParsed;
	LIST_ELEMENT			*pSavedBufferListElement;
	unsigned long			nSavedBufferBytesRemainingToBeProcessed;

	// Set local computer's byte order based on reading a two-byte number from memory.
	if ( *( (unsigned char*)&LocalStorageExample ) == 0x12 )
		LocalMemoryByteOrder = BIG_ENDIAN;
	else
		LocalMemoryByteOrder = LITTLE_ENDIAN;
	// From the standard:
	// Except for the 128 byte preamble and the 4 byte prefix, the File Meta Information (Group = 0002) shall be
	// encoded using the Explicit VR Little Endian Transfer Syntax (UID=1.2.840.10008.1.2.1) as defined in DICOM PS 3.5.
	// Values of each File Meta Element shall be padded when necessary to achieve an even length as specified in PS 3.5
	// by their corresponding Value Representation.
	CurrentTransferSyntax = pDicomHeader -> FileDecodingPlan.FileMetadataTransferSyntax;
	if ( !pDicomHeader -> FileDecodingPlan.bTrustSpecifiedTransferSyntaxFromLocalStorage )
		CurrentTransferSyntax = GetConsistentTransferSyntax( CurrentTransferSyntax, ppBufferListElement );

	nBytesParsed = 0;
	while ( bNoError && bMoreDicomElementsRemainInSequence )
		{
		SaveBufferCursor( ppBufferListElement, &pSavedBufferListElement, &nSavedBufferBytesRemainingToBeProcessed );
		// Allocate a DICOM_ELEMENT structure, append it to the list and read all the info about the next Dicom element except the value.
		bNoError = ParseDicomElement( ppBufferListElement, &pDicomElement, &nBytesParsed,
							CurrentTransferSyntax, SequenceNestingLevel, pDicomHeader );
		if ( pDicomElement -> Tag.Group > 0x0002 )
			bMoreDicomElementsRemainInSequence = FALSE;
		if ( bNoError && bMoreDicomElementsRemainInSequence )
			{
			bNoError = AppendToList( &pDicomHeader -> ListOfDicomElements, (void*)pDicomElement );
			if ( pDicomElement -> ValueLength != VALUE_LENGTH_UNDEFINED )
				{
				// For each individual element, decide whether the value needs to be read and,
				//  if so, specify what to do with it.
				bNoError = ReadDicomElementValue( pDicomElement, ppBufferListElement, &nBytesParsed, TextLine, pDicomHeader, FALSE );
				if ( bLogDicomElements )
					LogDicomElement( pDicomElement, 0 );
				if ( bNoError )
					{
					// ReadDicomElementValue() returned a copy of the value converted to
					// an ascii text field in the TextLine buffer.  Insert it into the abstract
					// file(s).
					bNoError = AddNewAbstractDataElement( pDicomElement -> Tag, TextLine );
					}
				}
			}
		else if ( !bMoreDicomElementsRemainInSequence )
			RestoreBufferCursor( ppBufferListElement, &pSavedBufferListElement, &nSavedBufferBytesRemainingToBeProcessed );
		}			// ...end while group 2 data elements remain to be read.

	if ( bNoError )
		{
		// Set up the Dicom file decoding plan:
		pDicomHeader -> FileDecodingPlan.nTransferSyntaxIndex = GetTransferSyntaxIndex( pDicomHeader -> TransferSyntaxUniqueIdentifier,
												(unsigned short)strlen( pDicomHeader -> TransferSyntaxUniqueIdentifier ) );
		CurrentTransferSyntax = GetTransferSyntaxForDicomElementParsing( (char)pDicomHeader -> FileDecodingPlan.nTransferSyntaxIndex );
		if ( pDicomHeader -> FileDecodingPlan.bTrustSpecifiedTransferSyntaxFromLocalStorage )
			{
			// Set the syntax for the data set.
			pDicomHeader -> FileDecodingPlan.DataSetTransferSyntax = CurrentTransferSyntax;
			// Set the syntax for the image.
			CurrentTransferSyntax = InterpretUniqueTransferSyntaxIdentifier( pDicomHeader -> TransferSyntaxUniqueIdentifier );
			pDicomHeader -> FileDecodingPlan.ImageDataTransferSyntax = CurrentTransferSyntax & 0xFF00;
			}
		else
			{
			SaveBufferCursor( ppBufferListElement, &pSavedBufferListElement, &nSavedBufferBytesRemainingToBeProcessed );
			pDicomHeader -> FileDecodingPlan.DataSetTransferSyntax =
									GetConsistentTransferSyntax( CurrentTransferSyntax, ppBufferListElement );
			RestoreBufferCursor( ppBufferListElement, &pSavedBufferListElement, &nSavedBufferBytesRemainingToBeProcessed );
			// The image transfer syntax will have to be deduced from information in the
			// data set, so will have to wait until after the full data set parse...
			}
		}

	return bNoError;
}


BOOL ParseDicomHeaderInfo( LIST_ELEMENT **ppBufferListElement, EXAM_INFO *pExamInfo,
							DICOM_HEADER_SUMMARY *pDicomHeader, BOOL bLogDicomElements )
{
	BOOL					bNoError = TRUE;
	unsigned short			LocalStorageExample = 0x1234;
	BOOL					bMoreHeaderInfoRemains = TRUE;
	long					SequenceNestingLevel = 0;
	TRANSFER_SYNTAX			CurrentTransferSyntax;
	BOOL					bTerminateSequence;
	size_t					nBytesParsed;

	// Set local computer's byte order based on reading a two-byte number from memory.
	if ( *( (unsigned char*)&LocalStorageExample ) == 0x12 )
		LocalMemoryByteOrder = BIG_ENDIAN;
	else
		LocalMemoryByteOrder = LITTLE_ENDIAN;
	// From the standard:
	// Except for the 128 byte preamble and the 4 byte prefix, the File Meta Information (Group = 0002) shall be
	// encoded using the Explicit VR Little Endian Transfer Syntax (UID=1.2.840.10008.1.2.1) as defined in DICOM PS 3.5.
	// Values of each File Meta Element shall be padded when necessary to achieve an even length as specified in PS 3.5
	// by their corresponding Value Representation.
	CurrentTransferSyntax = pDicomHeader -> FileDecodingPlan.DataSetTransferSyntax;
	if ( pExamInfo == 0 )
		{
		bNoError = FALSE;
		RespondToError( MODULE_DICOM, DICOM_ERROR_EXAM_INFO_STRUCTURE );
		}
	if ( bNoError )
		{
		bTerminateSequence = FALSE;
		nBytesParsed = 0;
		// Parse the main data set sequence.  Other sequencences may be embedded within it, and these will be
		// processed by recursive calls made from inside ProcessDicomElementSequence().
		bNoError = ProcessDicomElementSequence( ppBufferListElement, 0L, CurrentTransferSyntax, pDicomHeader, &SequenceNestingLevel,
													&bTerminateSequence, &nBytesParsed, bLogDicomElements, FALSE );
		}
	if ( bNoError )
		bNoError = CheckForRequiredDicomElements( pDicomHeader );
	if ( bNoError )
		{
		if ( pDicomHeader -> TransferSyntaxUniqueIdentifier != 0 && strlen( pDicomHeader -> TransferSyntaxUniqueIdentifier ) > 0 )
			pDicomHeader -> FileDecodingPlan.nTransferSyntaxIndex = GetTransferSyntaxIndex( pDicomHeader -> TransferSyntaxUniqueIdentifier,
													(unsigned short)strlen( pDicomHeader -> TransferSyntaxUniqueIdentifier ) );
		LoadExamInfoFromDicomHeader( pExamInfo, pDicomHeader );
		}

	return bNoError;
}


// This function is called recursively to process each encountered Dicom data element sequence.
// A Dicom element sequence is labelled by an element with DicomElement.ValueRepresentation == SQ.
// This function is called whenever such an element is encountered (also, at the beginning of the
// primary sequence).
//
// If a nested sequence is encountered (DicomElement.ValueRepresentation == SQ), this function
// calls itsself recursively.
//
// If a Dicom data element is encountered with DicomElement.Tag.Group == 0xFFFE, this is recognized
// as an item delimiter.  If DicomElement.Tag.Element == ELEMENT_ITEM_INITIATOR is encountered,
// and DicomElement.ValueLength != 0, this function calls itsself recursively to decode the item
// sequence.  When it encounters DicomElement.Tag.Element == ELEMENT_ITEM_TERMINATOR, the embedded
// sequence is terminated.
//
// If image data are encountered, the current sequence is automatically terminated, and further
// Dicom element decoding is handled by the ProcessDicomImageDataElements() function.
BOOL ProcessDicomElementSequence( LIST_ELEMENT **ppBufferListElement, unsigned long nBytesContainedInSequence,
									TRANSFER_SYNTAX CurrentTransferSyntax, DICOM_HEADER_SUMMARY *pDicomHeader,
									long *pNestingLevel, BOOL *pbTerminateSequence, size_t *pnBytesParsed,
									BOOL bLogDicomElements, BOOL bSequenceIsPrivateData )
{
	BOOL					bNoError = TRUE;
	unsigned short			LocalStorageExample = 0x1234;
	unsigned short			PrevGroupNumber;
	char					TextLine[ MAX_LOGGING_STRING_LENGTH ];
	BOOL					bMoreDicomElementsRemainInSequence = TRUE;
	DICOM_ELEMENT			*pDicomElement;
	BOOL					bSequenceLengthWasSpecified;
	char					*IndentationString = "";
	BOOL					bBreak = FALSE;
	size_t					nBytesParsed;
	LIST_ELEMENT			*pSavedBufferListElement;
	unsigned long			nSavedBufferBytesRemainingToBeProcessed;
	BOOL					bHasByteSequenceDataValue;
	BOOL					bFirstPrivateElementInSequence;
	BOOL					bPrivateData;

	bSequenceLengthWasSpecified = ( nBytesContainedInSequence != VALUE_LENGTH_UNDEFINED && nBytesContainedInSequence != 0L );
	nBytesParsed = 0;
	bFirstPrivateElementInSequence = FALSE;
	PrevGroupNumber = 0;
	while ( bNoError && bMoreDicomElementsRemainInSequence )
		{
		SaveBufferCursor( ppBufferListElement, &pSavedBufferListElement, &nSavedBufferBytesRemainingToBeProcessed );
		if ( nSavedBufferBytesRemainingToBeProcessed == 0 )
			{
			bMoreDicomElementsRemainInSequence = FALSE;
			*pbTerminateSequence = TRUE;
			}
		if ( bMoreDicomElementsRemainInSequence )
			{
			// Read all the info about the next Dicom element except the value.
			bNoError = ParseDicomElement( ppBufferListElement, &pDicomElement, &nBytesParsed,
								CurrentTransferSyntax, (unsigned char)*pNestingLevel, pDicomHeader );
			
			// If the group number is odd, this is a private data sequence.
			bPrivateData = ( ( pDicomElement -> Tag.Group & 0x0001 ) != 0 );
			if ( !bPrivateData || PrevGroupNumber != pDicomElement -> Tag.Group )
				{
				// Arm the flag to be looking for the first private data element in this sequence.
				bFirstPrivateElementInSequence = TRUE;
				}
			if ( pDicomElement -> Tag.Group == 0x7fe0 && pDicomElement -> Tag.Element == 0x0010 && !bSequenceIsPrivateData )
				{
				LogDicomElement( pDicomElement, *pNestingLevel );
				bMoreDicomElementsRemainInSequence = FALSE;
				*pbTerminateSequence = TRUE;
				pDicomHeader -> pBufferElementWithImageStart = *ppBufferListElement;
				RestoreBufferCursor( ppBufferListElement, &pSavedBufferListElement, &nSavedBufferBytesRemainingToBeProcessed );
				}
			}
		if ( bNoError && bMoreDicomElementsRemainInSequence )
			{
			bNoError = AppendToList( &pDicomHeader -> ListOfDicomElements, (void*)pDicomElement );
			if ( pDicomElement -> ValueRepresentation == SQ && pDicomElement -> ValueLength > 0 )
				{
				( *pNestingLevel )++;
				if ( bLogDicomElements )
					LogDicomElement( pDicomElement, *pNestingLevel );
				if ( !bSequenceIsPrivateData )
					LoadDicomHeaderElement( pDicomElement, pDicomHeader );
				if (  pDicomElement -> ValueLength != 0 )
					{
					// If the group number is odd, this is a private data sequence.  Also, treat the IconImageSequence as private data.
					bPrivateData = ( ( pDicomElement -> Tag.Group & 0x0001 ) != 0 ||
								( pDicomElement -> Tag.Group == 0x0088 && pDicomElement -> Tag.Element == 0x0200 ) );
					// Make a recursive call to this function to handle the embedded sequence.
					bNoError = ProcessDicomElementSequence( ppBufferListElement, pDicomElement -> ValueLength, CurrentTransferSyntax,
															pDicomHeader, pNestingLevel, pbTerminateSequence, &nBytesParsed,
															bLogDicomElements, bPrivateData );
					}
				( *pNestingLevel )--;
				if ( bSequenceLengthWasSpecified && nBytesContainedInSequence - nBytesParsed <= 0 )
					bMoreDicomElementsRemainInSequence = FALSE;
				if ( *pbTerminateSequence )
					return bNoError;
				}
			else if ( pDicomElement -> Tag.Group == GROUP_ITEM_DELIMITERS )
				{
				pDicomElement -> ValueMultiplicity = 1;
				if ( pDicomElement -> Tag.Element == ELEMENT_ITEM_INITIATOR )
					{
					if ( bLogDicomElements )
						LogDicomElement( pDicomElement, *pNestingLevel );
					if ( pDicomElement -> ValueLength != 0 )
						{
						// Make a recursive call to this function to handle the embedded sequence.
						( *pNestingLevel )++;
						bNoError = ProcessDicomElementSequence( ppBufferListElement, pDicomElement -> ValueLength, CurrentTransferSyntax,
																	pDicomHeader, pNestingLevel, pbTerminateSequence, &nBytesParsed,
																	bLogDicomElements, bSequenceIsPrivateData );
						( *pNestingLevel )--;
						if ( bSequenceLengthWasSpecified && nBytesContainedInSequence - nBytesParsed <= 0 )
							bMoreDicomElementsRemainInSequence = FALSE;
						if ( *pbTerminateSequence )
							return bNoError;
						}
					}
				else if ( pDicomElement -> Tag.Element == ELEMENT_ITEM_TERMINATOR )
					{
					if ( bLogDicomElements )
						LogDicomElement( pDicomElement, *pNestingLevel );
					bMoreDicomElementsRemainInSequence = FALSE;
					}
				else if ( pDicomElement -> Tag.Element == ELEMENT_ITEM_SEQUENCE_TERMINATOR )
					{
					if ( bLogDicomElements )
						LogDicomElement( pDicomElement, *pNestingLevel );
					if ( *pNestingLevel != 0 || nSavedBufferBytesRemainingToBeProcessed == 0 )
						bMoreDicomElementsRemainInSequence = FALSE;
					}
				else
					{
					if ( bLogDicomElements )
						LogDicomElement( pDicomElement, *pNestingLevel );
					}
				}
			else if ( bSequenceLengthWasSpecified && nBytesContainedInSequence - nBytesParsed <= 0 )
				{
				if ( bLogDicomElements )
					LogDicomElement( pDicomElement, *pNestingLevel );
				bMoreDicomElementsRemainInSequence = FALSE;
				}
			else
				{
				bHasByteSequenceDataValue = ( pDicomElement -> ValueRepresentation == OB ||
												pDicomElement -> ValueRepresentation == OW ||
												( pDicomElement -> ValueRepresentation == UN && !bPrivateData ) );
				if ( bHasByteSequenceDataValue )
					{
					if ( bLogDicomElements )
						LogDicomElement( pDicomElement, *pNestingLevel );
					}
				if ( pDicomElement -> ValueLength != VALUE_LENGTH_UNDEFINED && pDicomElement -> ValueLength != 0 &&
							 ( pDicomElement -> Tag.Group != 0x0004 || pDicomElement -> Tag.Element != 0x0000 ) )	// Ignore embedded file set elements.
					{
					// For each individual element, decide whether the value needs to be read and,
					//  if so, specify what to do with it.
					bNoError = ReadDicomElementValue( pDicomElement, ppBufferListElement, &nBytesParsed, TextLine, pDicomHeader, bPrivateData );
					if ( bPrivateData && bFirstPrivateElementInSequence && pDicomElement -> pConvertedValue != 0 )
						{
						// The first element in a private data group identifies the private data dictionary to be used for decoding.
						IdentifyPrivateDicomDictionary( pDicomElement );
						bFirstPrivateElementInSequence = FALSE;			// Reset for the next potential private group.
						}
					if ( bLogDicomElements && !bHasByteSequenceDataValue )
						LogDicomElement( pDicomElement, *pNestingLevel );
					if ( bNoError && pDicomElement -> pMatchingDictionaryItem != 0 &&
								pDicomElement -> pMatchingDictionaryItem -> Description != 0 &&
								strlen( pDicomElement -> pMatchingDictionaryItem -> Description ) > 0 )
						{
						// ReadDicomElementValue() returned a copy of the value converted to
						// an ascii text field in the TextLine buffer.  Insert it into the abstract
						// file(s).
						if ( *pNestingLevel == 0 && !bPrivateData )	// Avoid nested reuse of primary tags.
							bNoError = AddNewAbstractDataElement( pDicomElement -> Tag, TextLine );
						}
					if ( bSequenceLengthWasSpecified && nBytesContainedInSequence - nBytesParsed <= 0 )
						bMoreDicomElementsRemainInSequence = FALSE;
					}
				else
					{
					if ( bLogDicomElements && !bHasByteSequenceDataValue )
						LogDicomElement( pDicomElement, *pNestingLevel );
					}
				}
			if ( pDicomElement -> Tag.Group == 0x7fe0 && pDicomElement -> Tag.Element == 0x0010 && !bSequenceIsPrivateData )
				{
				bMoreDicomElementsRemainInSequence = FALSE;
				*pbTerminateSequence = TRUE;
				}
			}
		PrevGroupNumber = pDicomElement -> Tag.Group;
		}
	*pnBytesParsed += nBytesParsed;

	return bNoError;
}


// This function requires that the Dicom file data set has been read into the buffer
// and parsed.  The data set parsing ends at the image data.  This function picks
// up at that point, reads the image size, and positions the returned buffer read
// pointer at the beginning of the image data.
BOOL ProcessDicomImageDataElements( LIST_ELEMENT **ppBufferListElement, DICOM_HEADER_SUMMARY *pDicomHeader,
										size_t *pnBytesParsed, BOOL bLogDicomElements )
{
	BOOL				bNoError = TRUE;
	unsigned char		SequenceNestingLevel;
	size_t				nTotalBytesParsed;
	TRANSFER_SYNTAX		CurrentTransferSyntax;
	DICOM_ELEMENT		*pDicomElement;
	size_t				nBytesInImageItem;
	long				EmbeddedSequenceNestingLevel = 0;
	size_t				nBytesParsed;
	char				TextLine[ MAX_LOGGING_STRING_LENGTH ];

	nBytesInImageItem = 0;
	nTotalBytesParsed = 0;
	CurrentTransferSyntax = pDicomHeader -> FileDecodingPlan.DataSetTransferSyntax;
	SequenceNestingLevel = 0;
	// Parse the pixel data element, if there is one.
	bNoError = ParseDicomElement( ppBufferListElement, &pDicomElement, &nTotalBytesParsed,
							CurrentTransferSyntax, SequenceNestingLevel, pDicomHeader );
	if ( bNoError )
		bNoError = AppendToList( &pDicomHeader -> ListOfDicomElements, (void*)pDicomElement );

	nBytesInImageItem = 0L;
	nBytesParsed = 0;
	if ( pDicomElement -> Tag.Group == 0x7fe0 && pDicomElement -> Tag.Element == 0x0010 )
		{
		// This is the pixel data element.  Commence the isolation of the pixel data stream.
		if ( pDicomElement -> ValueLength != VALUE_LENGTH_UNDEFINED )
			nBytesInImageItem = pDicomElement -> ValueLength;
		else
			{
			// Read the Basic Offset Table, if any.
			bNoError = ParseDicomElement( ppBufferListElement, &pDicomElement, &nBytesParsed,
								CurrentTransferSyntax, SequenceNestingLevel, pDicomHeader );
			if ( bNoError )
				{
				bNoError = AppendToList( &pDicomHeader -> ListOfDicomElements, (void*)pDicomElement );
				nTotalBytesParsed += nBytesParsed;
				if ( pDicomElement -> Tag.Group == 0xfffe && pDicomElement -> Tag.Element == 0xe000 )
					{
					pDicomElement -> ValueMultiplicity = 1;
					// Process the Dicom Basic Offset Table for this image.
					EmbeddedSequenceNestingLevel++;
					if ( pDicomElement -> ValueLength != VALUE_LENGTH_UNDEFINED && pDicomElement -> ValueLength != 0 )
						{
						// The basic offset table value is an array of unsigned long integers:
						pDicomElement -> ValueRepresentation = UL;
						// For each individual element, decide whether the value needs to be read and,
						//  if so, specify what to do with it.
						nBytesParsed = 0;
						bNoError = ReadDicomElementValue( pDicomElement, ppBufferListElement, &nBytesParsed, TextLine, pDicomHeader, FALSE );
						if ( bNoError )
							nTotalBytesParsed += nBytesParsed;
						}
					if ( bLogDicomElements )
						LogDicomElement( pDicomElement, EmbeddedSequenceNestingLevel );
					EmbeddedSequenceNestingLevel--;
					}
				else
					{
					bNoError = FALSE;
					RespondToError( MODULE_DICOM, DICOM_ERROR_BASIC_OFFSET_TABLE );
					}
				}
			if ( bNoError )
				{
				nBytesParsed = 0;
				bNoError = ParseDicomElement( ppBufferListElement, &pDicomElement, &nBytesParsed,
									CurrentTransferSyntax, SequenceNestingLevel, pDicomHeader );
				if ( bNoError )
					bNoError = AppendToList( &pDicomHeader -> ListOfDicomElements, (void*)pDicomElement );
				if ( bNoError && pDicomElement -> Tag.Group == 0xfffe && pDicomElement -> Tag.Element == 0xe000 )
					{
					pDicomElement -> ValueMultiplicity = 1;
					nTotalBytesParsed += nBytesParsed;
					// Process the first item for this image.
					if ( bLogDicomElements )
						LogDicomElement( pDicomElement, 1 );
					if ( pDicomElement -> ValueLength != VALUE_LENGTH_UNDEFINED )
						nBytesInImageItem = pDicomElement -> ValueLength;
					}
				}
			}
		}
	pDicomHeader -> ImageLengthInBytes = (unsigned long)nBytesInImageItem;
	if ( pDicomHeader -> ImageLengthInBytes > 0 )
		{
		pDicomHeader -> pImageData = (char*)malloc( pDicomHeader -> ImageLengthInBytes );
		if ( pDicomHeader -> pImageData != 0 )
			bNoError = CopyBytesFromBuffer( pDicomHeader -> pImageData, pDicomHeader -> ImageLengthInBytes, ppBufferListElement );
		if ( bNoError )
			nTotalBytesParsed += pDicomHeader -> ImageLengthInBytes;
		}
	*pnBytesParsed = nTotalBytesParsed;
			
	return bNoError;
}


BOOL ReadDicomElementValue( DICOM_ELEMENT *pDicomElement, LIST_ELEMENT **ppBufferListElement, size_t *pnBytesParsed, char *TextLine,
								DICOM_HEADER_SUMMARY *pDicomHeader, BOOL bIsPrivateData )
{
	BOOL					bNoError = TRUE;
	BOOL					bRetainConvertedValueBuffer;
	TRANSFER_SYNTAX			CurrentTransferSyntax;

	if ( pDicomElement -> Tag.Group == 0x0018 && pDicomElement -> Tag.Element == 0x9313 )
		bNoError = TRUE;
	bNoError = ParseDicomElementValue( ppBufferListElement, pDicomElement, pDicomHeader, pnBytesParsed, pDicomElement -> ValueRepresentation, TextLine );
	if ( pDicomElement -> ValueRepresentation == PN )
		AllocateDicomPersonNameBuffer( pDicomElement );
	else
		AllocateDicomValueBuffer( pDicomElement, pDicomHeader );
	// Handle special cases.  Either reference the value from the DICOM_HEADER_SUMMARY or free it after printing.
	if ( bIsPrivateData )
		{
		bRetainConvertedValueBuffer = TRUE;
		if ( pDicomElement -> Tag.Group == 0x50F1 && pDicomElement -> Tag.Element == 0x1020 )
			LoadDicomHeaderElement( pDicomElement, pDicomHeader );			// Record Fujifilm format qualifier from private data.
		}
	else
		{
		// Don't load private data into the Dicom header.  Info such as thumbnail dimensions can overwrite
		// the image specifications, etc.
		bRetainConvertedValueBuffer = LoadDicomHeaderElement( pDicomElement, pDicomHeader );
		}
	if ( pDicomElement -> Tag.Group == 0x0002 && pDicomElement -> Tag.Element == 0x0010 )
		{
		CurrentTransferSyntax = InterpretUniqueTransferSyntaxIdentifier( pDicomHeader -> TransferSyntaxUniqueIdentifier );
		pDicomHeader -> FileDecodingPlan.ImageDataTransferSyntax = CurrentTransferSyntax & 0xFF00;
		}

	pDicomElement -> bRetainConvertedValue = bRetainConvertedValueBuffer;

	return bNoError;
}


char *AllocateDicomValueBuffer( DICOM_ELEMENT *pDicomElement, DICOM_HEADER_SUMMARY *pDicomHeader )
{
	char			*pValueBuffer;
	char			*pValue;
	long			nValueSizeInBytes;

	pValue = pDicomElement -> Value.LT;
	nValueSizeInBytes = pDicomElement -> ValueLength;
	if ( pValue != 0 && nValueSizeInBytes > 0 )
		{
		pValueBuffer = (char*)malloc( nValueSizeInBytes + 1 );
		if ( pValueBuffer != 0 )
			{
			memcpy( pValueBuffer, pValue, nValueSizeInBytes );
			switch( pDicomElement -> ValueRepresentation )
				{
				case SS:			// Signed short.
				case US:			// Unsigned short.
				case AT:			// Attribute tag.
				case FL:			// Float (single precision).
				case SL:			// Signed long.
				case UL:			// Unsigned long.
				case FD:			// Float (double precision).
				case OB:			// Other byte string.
					break;
				default:
					pValueBuffer[ nValueSizeInBytes ] = '\0';
					TrimBlanks( pValueBuffer );
					break;
				}
			}
		else
			RespondToError( MODULE_DICOM, DICOM_ERROR_ALLOCATE_VALUE );
		}
	else
		pValueBuffer = 0;
	pDicomElement -> pConvertedValue = pValueBuffer;
	
	return pValueBuffer;
}


PERSON_NAME *AllocateDicomPersonNameBuffer( DICOM_ELEMENT *pDicomElement )
{
	char			*pValue;
	long			nValueSizeInBytes;
	PERSON_NAME		*pNameBuffer;
	long			nDelimiter;
	long			nNamePart;
	long			nChar;
	long			nBaseChar;
	long			nNameLength[5];
	
	pValue = pDicomElement -> Value.LT;
	nValueSizeInBytes = pDicomElement -> ValueLength;
	if ( nValueSizeInBytes > 0 )
		{
		pNameBuffer = (PERSON_NAME*)malloc( sizeof( PERSON_NAME ) );
		if ( pNameBuffer != 0 )
			{
			// Initialize pointers to name parts and length of each part.
			pNameBuffer -> pLastName = 0;
			pNameBuffer -> pFirstName = 0;
			pNameBuffer -> pMiddleName = 0;
			pNameBuffer -> pPrefix = 0;
			pNameBuffer -> pSuffix = 0;
			for ( nNamePart = 0; nNamePart < 5; nNamePart++ )
				nNameLength[ nNamePart ] = 0;

			// Get the length of each name part..
			nDelimiter = 0;
			nBaseChar = 0;
			for ( nChar = 0; nChar < nValueSizeInBytes && nDelimiter < 5; nChar++ )
				{
				if ( pValue[ nChar ] == '^' )
					{
					nNameLength[ nDelimiter ] = nChar - nBaseChar;
					nDelimiter++;
					nBaseChar = nChar + 1;
					}
				}
			if ( nDelimiter < 4 )
				nNameLength[ nDelimiter ] = nChar - nBaseChar;

			// Allocate a buffer for each non-null name part.
			for ( nNamePart = 0; nNamePart < 5; nNamePart++ )
				if ( nNameLength[ nNamePart ] > 0 )
					switch ( nNamePart )
						{
						case 0:
							pNameBuffer -> pLastName = (char*)malloc( nNameLength[ 0 ] + 1 );
							break;
						case 1:
							pNameBuffer -> pFirstName = (char*)malloc( nNameLength[ 1 ] + 1 );
							break;
						case 2:
							pNameBuffer -> pMiddleName = (char*)malloc( nNameLength[ 2 ] + 1 );
							break;
						case 3:
							pNameBuffer -> pPrefix = (char*)malloc( nNameLength[ 3 ] + 1 );
							break;
						case 4:
							pNameBuffer -> pSuffix = (char*)malloc( nNameLength[ 4 ] + 1 );
							break;
						}
			nDelimiter = 0;
			nBaseChar = 0;
			for ( nChar = 0; nChar < nValueSizeInBytes; nChar++ )
				{
				if ( pValue[ nChar ] == '^' )
					{
					nDelimiter++;
					nBaseChar = nChar + 1;
					}
				else
					switch ( nDelimiter )
						{
						case 0:
							if ( pNameBuffer -> pLastName != 0 )
								pNameBuffer -> pLastName[ nChar - nBaseChar ] = pValue[ nChar ];
							break;
						case 1:
							if ( pNameBuffer -> pFirstName != 0 )
								pNameBuffer -> pFirstName[ nChar - nBaseChar ] = pValue[ nChar ];
							break;
						case 2:
							if ( pNameBuffer -> pMiddleName != 0 )
								pNameBuffer -> pMiddleName[ nChar - nBaseChar ] = pValue[ nChar ];
							break;
						case 3:
							if ( pNameBuffer -> pPrefix != 0 )
								pNameBuffer -> pPrefix[ nChar - nBaseChar ] = pValue[ nChar ];
							break;
						case 4:
							if ( pNameBuffer -> pSuffix != 0 )
								pNameBuffer -> pSuffix[ nChar - nBaseChar ] = pValue[ nChar ];
							break;
						default:
							break;
						}
				}
			for ( nNamePart = 0; nNamePart < 5; nNamePart++ )
				if ( nNameLength[ nNamePart ] > 0 )
					switch ( nNamePart )
						{
						case 0:
							pNameBuffer -> pLastName[ nNameLength[ 0 ] ] = '\0';
							TrimBlanks( pNameBuffer -> pLastName );
							break;
						case 1:
							pNameBuffer -> pFirstName[ nNameLength[ 1 ] ] = '\0';
							TrimBlanks( pNameBuffer -> pFirstName );
							break;
						case 2:
							pNameBuffer -> pMiddleName[ nNameLength[ 2 ] ] = '\0';
							TrimBlanks( pNameBuffer -> pMiddleName );
							break;
						case 3:
							pNameBuffer -> pPrefix[ nNameLength[ 3 ] ] = '\0';
							TrimBlanks( pNameBuffer -> pPrefix );
							break;
						case 4:
							pNameBuffer -> pSuffix[ nNameLength[ 4 ] ] = '\0';
							TrimBlanks( pNameBuffer -> pSuffix );
							break;
						}
			}
		}
	else
		pNameBuffer = 0;
	pDicomElement -> pConvertedValue = (char*)pNameBuffer;

	return pNameBuffer;
}


void CopyDicomNameToString( char *TextString, PERSON_NAME *pName, long nTextStringSize )
{
	long			nUnusedBytes;

	nUnusedBytes = nTextStringSize;
	if ( pName != 0 )
		{
		if ( pName -> pPrefix != 0 && nUnusedBytes >= (long)strlen( pName -> pPrefix ) + 1 )
			{
			strcat( TextString, pName -> pPrefix );
			strcat( TextString, " " );
			nUnusedBytes = nTextStringSize - (long)strlen( TextString );
			}
		if ( pName -> pFirstName != 0 && nUnusedBytes >= (long)strlen( pName ->  pFirstName ) + 1 )
			{
			strcat( TextString, pName -> pFirstName );
			strcat( TextString, " " );
			nUnusedBytes = nTextStringSize - (long)strlen( TextString );
			}
		if ( pName -> pMiddleName != 0 && nUnusedBytes >= (long)strlen( pName -> pMiddleName ) + 1 )
			{
			strcat( TextString, pName -> pMiddleName );
			strcat( TextString, " " );
			nUnusedBytes = nTextStringSize - (long)strlen( TextString );
			}
		if ( pName -> pLastName != 0 && nUnusedBytes >= (long)strlen( pName -> pLastName ) + 1 )
			{
			strcat( TextString, pName -> pLastName );
			strcat( TextString, " " );
			nUnusedBytes = nTextStringSize - (long)strlen( TextString );
			}
		if ( pName -> pSuffix != 0 && nUnusedBytes >= (long)strlen( pName -> pSuffix ) + 1 )
			{
			strcat( TextString, pName -> pSuffix );
			strcat( TextString, " " );
			nUnusedBytes = nTextStringSize - (long)strlen( TextString );
			}
		// Eliminate the trailing blank.
		TextString[ strlen( TextString ) - 1 ] = '\0';
		}
}


void InitDicomHeaderSummary( DICOM_HEADER_SUMMARY *pDicomHeader )
{
	memset( (char*)pDicomHeader, '\0', sizeof(DICOM_HEADER_SUMMARY) );
	pDicomHeader -> bFileMetadataHasBeenRead = FALSE;
	pDicomHeader -> FileDecodingPlan.bTrustSpecifiedTransferSyntaxFromLocalStorage =
						ServiceConfiguration.bTrustSpecifiedTransferSyntaxFromLocalStorage;
	pDicomHeader -> FileDecodingPlan.bTrustSpecifiedTransferSyntaxFromNetwork =
						ServiceConfiguration.bTrustSpecifiedTransferSyntaxFromNetwork;
	pDicomHeader -> FileDecodingPlan.FileMetadataTransferSyntax = EXPLICIT_VR | LITTLE_ENDIAN;	// Always.
	// set default calibration values in case these elements are not specified.
	pDicomHeader -> CalibrationInfo.RescaleIntercept = 0.0;
	pDicomHeader -> CalibrationInfo.RescaleSlope = 1.0;
	pDicomHeader -> CalibrationInfo.bPixelValuesAreSigned = FALSE;
}


FILE *OpenDicomFileForOutput( char *DicomFileSpecification )
{
	FILE			*pDicomFile = 0;
	FILE_STATUS		FileStatus = FILE_STATUS_OK;
	char			TextLine[ 1096 ];
	char			WriteBuffer[ 132 ];
	long			nBytesWritten;

	sprintf( TextLine, "Open Dicom file for output:  %s", DicomFileSpecification );
	LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
	if ( StorageCapacityIsAdequate() )
		{
		pDicomFile = fopen( DicomFileSpecification, "wb" );
		if ( pDicomFile != 0 )
			{
			memset( WriteBuffer, '\0', 132 );
			memcpy( &WriteBuffer[ 128 ], "DICM", 4 );
			nBytesWritten = (long)fwrite( WriteBuffer, 1, 132, pDicomFile );
			if ( nBytesWritten != 132L )
				{
				RespondToError( MODULE_DICOM, DICOM_ERROR_DICOM_OPEN_WRITE );
				LogMessage( "Aborting file open for writing.", MESSAGE_TYPE_ERROR );
				fclose( pDicomFile );
				pDicomFile = 0;
				}
			}
		else
			{
			LogMessage( TextLine, MESSAGE_TYPE_NORMAL_LOG );
			RespondToError( MODULE_DICOM, DICOM_ERROR_FILE_OPEN );
			}
		}
		
	return pDicomFile;
}


FILE *OpenDicomFile( char *DicomFileSpecification )
{
	FILE			*pDicomFile;
	FILE_STATUS		FileStatus = FILE_STATUS_OK;
	char			TextLine[ 1096 ];
	char			ReadBuffer[ 512 ];
	DWORD			SystemErrorCode;

	sprintf( TextLine, "Open Dicom File:  %s", DicomFileSpecification );
	LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
	pDicomFile = fopen( DicomFileSpecification, "rb" );
	if ( pDicomFile != 0 )
		{
		// Read the first 132 bytes to check for the standard DICOM identifier at the
		// beginning of the file.
		FileStatus = ReadFileData( pDicomFile, ReadBuffer, 132 );
		if ( FileStatus == FILE_STATUS_OK )
			{
			if ( strncmp( ReadBuffer + 128, "DICM", 4 ) != 0 )
				{
				RespondToError( MODULE_DICOM, DICOM_ERROR_DICOM_SIGNATURE );
				LogMessage( "Aborting file read.", MESSAGE_TYPE_ERROR );
				fclose( pDicomFile );
				pDicomFile = 0;
				}
			}
		}
	else
		{
		SystemErrorCode = GetLastError();
		RespondToError( MODULE_DICOM, DICOM_ERROR_FILE_OPEN );
		sprintf( TextLine, "   Open Dicom File:  system error code %d", SystemErrorCode );
		LogMessage( TextLine, MESSAGE_TYPE_ERROR );
		}
		
	return pDicomFile;
}


FILE_STATUS ReadFileData( FILE *pDicomFile, char *Buffer, long nBytesToBeRead )
{
	long			nBytesRead;
	FILE_STATUS		FileStatus = FILE_STATUS_OK;
	int				SystemErrorNumber;

	// Read the next Group and Element Tag.
	nBytesRead = (long)fread( Buffer, 1, nBytesToBeRead, pDicomFile );
	if ( nBytesRead != nBytesToBeRead )
		{
		if ( feof( pDicomFile ) )
			FileStatus |= FILE_STATUS_EOF;
		SystemErrorNumber = ferror( pDicomFile );
		if ( SystemErrorNumber != 0 )
			{
			FileStatus |= FILE_STATUS_READ_ERROR;
			RespondToError( MODULE_DICOM, DICOM_ERROR_FILE_READ );
			LogMessage( strerror( SystemErrorNumber ), MESSAGE_TYPE_ERROR );
			}
		}

	return FileStatus;
}


BOOL HasRecognizedValueRepresentation( VR ValueRepresentation )
{
	BOOL			bHasRecognizedVR;

	switch( ValueRepresentation )
		{
		case LT :			// Long text.
		case OB :			// Other byte string.
		case OW :			// Other word string.
		case SQ :			// Item sequence.
		case ST :			// Short text.
		case UT :			// Unlimited text.
		case SS :			// Signed short.
		case US :			// Unsigned short.
		case AT :			// Attribute tag.
		case FL :			// Float (single precision).
		case SL :			// Signed long.
		case UL :			// Unsigned long.
		case FD :			// Float (double precision).
		case AE :			// Application entity.
		case AS :			// Age string.
		case CS :			// Code string.
		case DA :			// Date.
		case DS :			// Decimal string.
		case DT :			// Date time.
		case IS :			// Integer string.
		case LO :			// Long string.
		case PN :			// Person name.
		case SH :			// Short string.
		case TM :			// Time.
		case UI :			// Unique identifier.
		case UN :			// Undeclared representation.
			bHasRecognizedVR = TRUE;
			break;
		default:
			bHasRecognizedVR = FALSE;
			break;
		}

	return bHasRecognizedVR;
}


// Read all the info about the next Dicom element except the value.
BOOL ParseDicomElement( LIST_ELEMENT **ppBufferListElement, DICOM_ELEMENT **ppDicomElement, size_t *pnBytesParsed,
								TRANSFER_SYNTAX TransferSyntax, unsigned char SequenceNestingLevel, DICOM_HEADER_SUMMARY *pDicomHeader )
{
	BOOL					bNoError = TRUE;
	char					TextLine[ MAX_LOGGING_STRING_LENGTH ];
	DICOM_ELEMENT			*pDicomElement;
	DICOM_DICTIONARY_ITEM	*pDictItem = 0;
	char					ValueRepresentation[ 2 ];
	char					DicomElementTextValueRepresentation[ 4 ];
	char					DictionaryTextValueRepresentation[ 4 ];
	BOOL					bPrivateData;
	BOOL					bBreak = FALSE;

	pDicomElement = (DICOM_ELEMENT*)calloc( 1, sizeof(DICOM_ELEMENT) );
	bNoError = ( pDicomElement != 0 );
	if ( bNoError )
		{
		*ppDicomElement = pDicomElement;
		}
	if ( bNoError )
		{
		pDicomElement -> ValueLength = 0L;
		pDicomElement -> pConvertedValue = 0;
		// Read the next Group and Element Tag.  An error indicates an attempt to read past end of data.
		bNoError = CopyBytesFromBuffer( (char*)&pDicomElement -> Tag.Group, sizeof(unsigned short), ppBufferListElement );
		}
	else
		*ppDicomElement = 0;
	if ( bNoError )
		{
		*pnBytesParsed += sizeof(unsigned short);
		bNoError = CopyBytesFromBuffer( (char*)&pDicomElement -> Tag.Element, sizeof(unsigned short), ppBufferListElement );
		*pnBytesParsed += sizeof(unsigned short);
		}
	if ( !bNoError )
		RespondToError( MODULE_DICOM, DICOM_ERROR_PARSING_PAST_END_OF_DATA );

	if ( pDicomElement -> Tag.Group == 0x0008 && pDicomElement -> Tag.Element == 0x0008 )
		bNoError = TRUE;
	if ( bNoError )
		{	
		// If the group number is odd, this is a private data sequence.
		bPrivateData = ( ( pDicomElement -> Tag.Group & 0x0001 ) != 0 );

		// From the standard:
		// Except for the 128 byte preamble and the 4 byte prefix, the File Meta Information (Group = 0002) shall be
		// encoded using the Explicit VR Little Endian Transfer Syntax (UID=1.2.840.10008.1.2.1) as defined in DICOM PS 3.5.
		// Values of each File Meta Element shall be padded when necessary to achieve an even length as specified in PS 3.5
		// by their corresponding Value Representation.
		//
		// This means that group 2 data will always have the correct transfer syntax set, and the
		// following swap will always be correct for group 2.  For subsequent groups, the effective transfer
		// syntax may not be set correctly, but in this case, even the incorrectly swapped group
		// number will be distinguishable from group 2.
		SwapBytesFromFile( &pDicomElement -> Tag.Group, 2, TransferSyntax );
		SwapBytesFromFile( &pDicomElement -> Tag.Element, 2, TransferSyntax );

		// If a non-image-related transfer syntax was read in the Group 0002 metadata, use it after Group 0002
		// has been read.  Otherwise, stay with the default transfer syntax.
		if ( pDicomElement -> Tag.Group != 0x0002 && !pDicomHeader -> bFileMetadataHasBeenRead )
			pDicomHeader -> bFileMetadataHasBeenRead = TRUE;	// We are now past the Group 2 data.
		// Read the value representation and the value length.
		//
		pDictItem = GetDicomElementFromDictionary( pDicomElement -> Tag );
		pDicomElement -> pMatchingDictionaryItem = pDictItem;
		// If this Element's value representation implicit (look it up in the dictionary) or is a private representation...
		if ( ( TransferSyntax & IMPLICIT_VR ) || pDicomElement -> Tag.Group == GROUP_ITEM_DELIMITERS )
			{
			if ( pDicomElement -> pMatchingDictionaryItem != 0 )
				pDicomElement -> ValueRepresentation = pDicomElement -> pMatchingDictionaryItem -> ValueRepresentation;
			else
				pDicomElement -> ValueRepresentation = UN;
			// Read the value length.
			bNoError = CopyBytesFromBuffer( (char*)&pDicomElement -> ValueLength, sizeof(unsigned long), ppBufferListElement );
			*pnBytesParsed += sizeof(unsigned long);
			SwapBytesFromFile( &pDicomElement -> ValueLength, 4, TransferSyntax );
			}			// ... end if implicit VR.

		// Otherwise, if this Element's value representation is recorded explicitly...
		else
			{
			bNoError = CopyBytesFromBuffer( (char*)&ValueRepresentation, 2, ppBufferListElement );
			*pnBytesParsed += 2;
			*((int*)( &pDicomElement -> ValueRepresentation )) =
									(int)( ValueRepresentation[0] << 8 ) | (int)ValueRepresentation[1];
			if ( bNoError )
				{
				// Read the value length.
				switch( pDicomElement -> ValueRepresentation )
					{
					case OB:
					case OW:
					case SQ:
					case UN:
					case UT:
						// Do a dummy read to skip 2 bytes in the buffer.
						bNoError = CopyBytesFromBuffer( (char*)&pDicomElement -> ValueLength, 2, ppBufferListElement );
						*pnBytesParsed += 2;
						if ( bNoError )
							{
							// Read the value length.
							bNoError = CopyBytesFromBuffer( (char*)&pDicomElement -> ValueLength, sizeof(unsigned long), ppBufferListElement );
							*pnBytesParsed += sizeof(unsigned long);
							SwapBytesFromFile( &pDicomElement -> ValueLength, 4, TransferSyntax );
							pDicomElement -> ValueMultiplicity = 1;
							}
						break;
					default:
						// Read the value length.
						bNoError = CopyBytesFromBuffer( (char*)&pDicomElement -> ValueLength, sizeof(unsigned short), ppBufferListElement );
						*pnBytesParsed += sizeof(unsigned short);
						SwapBytesFromFile( &pDicomElement -> ValueLength, 2, TransferSyntax );
						break;
					}
				}
			}
		// Handle newer, complex value representation codes for implicit VR Dicom data elements.
		switch( pDicomElement -> ValueRepresentation )
			{
			case xs:
				if ( pDicomHeader -> CalibrationInfo.bPixelValuesAreSigned )
					pDicomElement -> ValueRepresentation = SS;
				else
					pDicomElement -> ValueRepresentation = US;
				break;
			case ox:
				if ( *pDicomHeader -> BitsAllocated > 8 )
					pDicomElement -> ValueRepresentation = OW;
				else
					pDicomElement -> ValueRepresentation = OB;
				break;
			default:
				break;

			}
		// For newer, complex value representation codes, reset the dictionary item to turn off log notifications.
		if ( pDicomElement -> pMatchingDictionaryItem != 0 )
			switch( pDicomElement -> pMatchingDictionaryItem -> ValueRepresentation )
				{
				case xs:
				case ox:
					pDicomElement -> pMatchingDictionaryItem -> ValueRepresentation = pDicomElement -> ValueRepresentation;
					break;
				}
		if ( pDicomElement -> pMatchingDictionaryItem != 0 && pDicomElement -> ValueRepresentation != pDicomElement -> pMatchingDictionaryItem -> ValueRepresentation )
			{
			DicomElementTextValueRepresentation[ 0 ] = (char)( ( (unsigned short)pDicomElement -> ValueRepresentation >> 8 ) & 0x00FF );
			DicomElementTextValueRepresentation[ 1 ] = (char)( ( (unsigned short)pDicomElement -> ValueRepresentation ) & 0x00FF );
			DicomElementTextValueRepresentation[ 2 ] = '\0';
			DictionaryTextValueRepresentation[ 0 ] = (char)( ( (unsigned short)pDictItem -> ValueRepresentation >> 8 ) & 0x00FF );
			DictionaryTextValueRepresentation[ 1 ] = (char)( ( (unsigned short)pDictItem -> ValueRepresentation ) & 0x00FF );
			DictionaryTextValueRepresentation[ 2 ] = '\0';
			sprintf( TextLine, "VR Mismatch:  ( %X, %X ) VR = %s vs dict. %s",
									pDicomElement -> Tag.Group, pDicomElement -> Tag.Element, DicomElementTextValueRepresentation, DictionaryTextValueRepresentation );
			LogMessage( TextLine, MESSAGE_TYPE_ERROR );
			}

		// ZZZ:  Fuji is declaring every private element as UN and encoding the value length that way.  But to decode it, the dictionary value
		// representation has to be used.  This is a gross violation of the Dicom standard.  This exception may need to be broadened if other
		// manufacturers are doing the same thing, or if Fuji is doing this with different values for Manufacturer.
		if ( bNoError && pDicomElement -> ValueRepresentation == UN && bPrivateData && _stricmp( pDicomHeader -> CalibrationInfo.Manufacturer, "FUJIFILM Corporation" ) == 0 )
			{
			if ( pDicomElement -> pMatchingDictionaryItem != 0 )
				pDicomElement -> ValueRepresentation = pDicomElement -> pMatchingDictionaryItem -> ValueRepresentation;
			}
		// Check for uneven value length.
		if ( ( pDicomElement -> ValueLength & 1 ) != 0 && pDicomElement -> ValueLength != VALUE_LENGTH_UNDEFINED )
			{
			// Make an exception for those who don't follow the rules.
			if ( _strnicmp( pDicomHeader -> ImplementationVersionName, "NovaRad", 7 ) != 0 || ( pDicomHeader -> SourceAE_TITLE != 0 && _strnicmp( pDicomHeader -> SourceAE_TITLE, "novapacs", 7 ) != 0 ) )
				{
				RespondToError( MODULE_DICOM, DICOM_ERROR_UNEVEN_VALUE_LENGTH );
				sprintf( TextLine, "Dicom Element ( %X, %X )", pDicomElement -> Tag.Group, pDicomElement -> Tag.Element );
				LogMessage( TextLine, MESSAGE_TYPE_ERROR );
				bNoError = FALSE;
				}
			}
		pDicomElement -> SequenceNestingLevel = SequenceNestingLevel;
		}

	return bNoError;
}


BOOL ParseDicomElementValue( LIST_ELEMENT **ppBufferListElement, DICOM_ELEMENT *pDicomElement, DICOM_HEADER_SUMMARY *pDicomHeader,
															size_t *pnBytesParsed, VR ValueRepresentationOverride, char *pTextLine )
{
	BOOL					bNoError = TRUE;
	long					CopyLength;
	long					nByte;
	long					vByte;
	unsigned char			SavedByteValue;
	long					nByteSwapLength;
	BOOL					bBreak = FALSE;

	// If the VR is unknown, use the override value.
	if ( pDicomElement -> ValueRepresentation == UN )
		pDicomElement -> ValueRepresentation = ValueRepresentationOverride;

	if ( pDicomElement -> ValueRepresentation != SQ &&
			pDicomElement -> ValueLength != VALUE_LENGTH_UNDEFINED )
		{
		if ( pDicomElement -> ValueLength == 0 )
			pDicomElement -> Value.UN = 0L;
		else
			{
			// Allocate and clear a value buffer.
			if ( pDicomElement -> Tag.Group == 0x0008 && pDicomElement -> Tag.Element == 0x0018 && pDicomHeader -> bAutoLoadThisImage )
				{
				pDicomElement -> Value.UN = calloc( pDicomElement -> ValueLength + 9, 1 );			// Allow space for the "AutoLoad_" prefix.
				}
			else
				pDicomElement -> Value.UN = calloc( pDicomElement -> ValueLength, 1 );
			if ( pDicomElement -> Value.UN != 0 )	// If the value buffer was allocated successfully...
				{
				// Copy the Dicom element value bytes into the element's value buffer.
				if ( pDicomElement -> Tag.Group == 0x0008 && pDicomElement -> Tag.Element == 0x0018 && pDicomHeader -> bAutoLoadThisImage )
					{
					// Prepend the "AutoLoad_" prefix to the SOPInstanceUniqueIdentifier so BViewer will know that
					// it should process this image as automatically loaded, selected and viewed.
					strcpy( (char*)pDicomElement -> Value.UN, "AutoLoad_" );
					bNoError = CopyBytesFromBuffer( (char*)pDicomElement -> Value.UN + 9, pDicomElement -> ValueLength, ppBufferListElement );
					*pnBytesParsed += pDicomElement -> ValueLength;
					pDicomElement -> ValueLength += 9;
					}
				else
					{
					bNoError = CopyBytesFromBuffer( (char*)pDicomElement -> Value.UN, pDicomElement -> ValueLength, ppBufferListElement );
					*pnBytesParsed += pDicomElement -> ValueLength;
					}

				// Handle Big Endian transfer syntax byte swapping.
				if ( pDicomHeader -> FileDecodingPlan.nTransferSyntaxIndex == BIG_ENDIAN_EXPLICIT_TRANSFER_SYNTAX &&
							pDicomElement -> Tag.Group > 0x0002 )
					{
					CopyLength = pDicomElement -> ValueLength;
					switch( pDicomElement -> ValueRepresentation )
						{
						CopyLength = pDicomElement -> ValueLength;
						// 2-byte values
						case US:			// Unsigned short.
						case SS:			// Signed short.
						case OW:			// Other word string.
						case AT:			// Attribute tag.
							nByteSwapLength = 2;
							break;
						// 4-byte values
						case UL:			// Unsigned long.
						case SL:			// Signed long.
						case FL:			// Float (single precision).
							nByteSwapLength = 4;
							break;
						// 8-byte values
						case FD:			// Float (double precision).
							nByteSwapLength = 8;
							break;
						default:
							nByteSwapLength = 2;	// Bypass the following loops.
							CopyLength = 0;
							break;
						}
					for ( nByte = 0; nByte <= CopyLength - nByteSwapLength; nByte += nByteSwapLength )
						{
						for ( vByte = 0; vByte < nByteSwapLength / 2; vByte++ )
							{
							SavedByteValue = (char)*( pDicomElement -> Value.LT + nByte + vByte );
							*(char*)( pDicomElement -> Value.LT + nByte + vByte ) = *(char*)( pDicomElement -> Value.LT + nByte + nByteSwapLength - vByte - 1 );
							*(char*)( pDicomElement -> Value.LT + nByte + nByteSwapLength - vByte - 1 ) = SavedByteValue;
							}
						}
					}
				// Also, output a copy of the value, converted to a text field:
				switch( pDicomElement -> ValueRepresentation )
					{
					case LT:			// Long text.
					case ST:			// Short text.
					case UT:			// Unlimited text.
					case AS:			// Age string.
					case CS:			// Code string.
					case DA:			// Date.
					case DS:			// Decimal string.
					case DT:			// Date time.
					case IS:			// Integer string.
					case LO:			// Long string.
					case PN:			// Person name.
					case SH:			// Short string.
					case AE:			// Application entity.
					case TM:			// Time.
					case UI:			// Unique identifier.
						CopyLength = pDicomElement -> ValueLength;
						if ( CopyLength > MAX_LOGGING_STRING_LENGTH - 1 )
							CopyLength = MAX_LOGGING_STRING_LENGTH - 1;
						memcpy( pTextLine, (char*)pDicomElement -> Value.UN, CopyLength );
						pTextLine[ CopyLength ] = '\0';
						break;
					case SS:			// Signed short.
						_itoa( (short)*pDicomElement -> Value.SS, pTextLine, 10 );
						break;
					case US:			// Unsigned short.
						_itoa( (unsigned short)*pDicomElement -> Value.US, pTextLine, 10 );
						break;
					case SL:			// Signed long.
						_ltoa( (long)*pDicomElement -> Value.SL, pTextLine, 10 );
						break;
					case UL:			// Unsigned long.
						_ultoa( (unsigned long)*pDicomElement -> Value.UL, pTextLine, 10 );
						break;
					case FL:			// Float (single precision).
						_gcvt( (float)*pDicomElement -> Value.FL, 16, pTextLine );
						break;
					case FD:			// Float (double precision).
						_gcvt( (double)*pDicomElement -> Value.FD, 16, pTextLine );
						break;
					// The following cases don't have a text representation.
					case AT:			// Attribute tag.
					case OB:			// Other byte string.
					case OW:			// Other word string.
					case SQ:			// Item sequence.
						strcpy( pTextLine, "" );
						break;
					case UN:			// Unsigned long.
						if ( pDicomElement -> Tag.Group == GROUP_ITEM_DELIMITERS )
							_ultoa( (unsigned long)*pDicomElement -> Value.UL, pTextLine, 10 );
						break;
					}
				}
			else
				{
				bNoError = FALSE;
				RespondToError( MODULE_DICOM, DICOM_ERROR_ALLOCATE_VALUE );
				}
			}
		}
	else			// Indicate no value read.
		{
		pDicomElement -> Value.UN = 0L;
		if ( pDicomElement -> ValueLength != 0 )
			{
			pDicomElement -> Value.UN = calloc( pDicomElement -> ValueLength, 1 );
			if ( pDicomElement -> Value.UN != 0 )
				bNoError = CopyBytesFromBuffer( (char*)pDicomElement -> Value.UN, pDicomElement -> ValueLength, ppBufferListElement );
			*pnBytesParsed += pDicomElement -> ValueLength;
			}
		}

	SetDicomElementValueMultiplicity( pDicomElement );

	return bNoError;
}


void SetDicomElementValueMultiplicity( DICOM_ELEMENT *pDicomElement )
{
	char			*pChar;
	unsigned long	nChar;

	if ( pDicomElement -> ValueLength == 0 )
		pDicomElement -> ValueMultiplicity = 0;
	else if ( pDicomElement -> ValueLength == VALUE_LENGTH_UNDEFINED )
		pDicomElement -> ValueMultiplicity = 1;
	else
		{
		switch( pDicomElement -> ValueRepresentation )
			{
			case LT:			// Long text.
			case OB:			// Other byte string.
			case OW:			// Other word string.
			case SQ:			// Item sequence.
			case ST:			// Short text.
			case UT:			// Unlimited text.
				pDicomElement -> ValueMultiplicity = 1;
				break;
			case SS:			// Signed short.
			case US:			// Unsigned short.
				pDicomElement -> ValueMultiplicity = pDicomElement -> ValueLength / 2;	// Two bytes per value.
				break;
			case AT:			// Attribute tag.
			case FL:			// Float (single precision).
			case SL:			// Signed long.
			case UL:			// Unsigned long.
				pDicomElement -> ValueMultiplicity = pDicomElement -> ValueLength / 4;	// Four bytes per value.
				break;
			case FD:			// Float (double precision).
				pDicomElement -> ValueMultiplicity = pDicomElement -> ValueLength / 8;	// Eight bytes per value.
				break;
			case AE:			// Application entity.
			case AS:			// Age string.
			case CS:			// Code string.
			case DA:			// Date.
			case DS:			// Decimal string.
			case DT:			// Date time.
			case IS:			// Integer string.
			case LO:			// Long string.
			case PN:			// Person name.
			case SH:			// Short string.
			case TM:			// Time.
			case UI:			// Unique identifier.
				pDicomElement -> ValueMultiplicity = 1;
				pChar = (char*)pDicomElement -> Value.UN;
				for ( nChar = 1; nChar < pDicomElement -> ValueLength; nChar++ )
					{
					if ( *pChar == '\\' )
						{
						*pChar = '|';
						pDicomElement -> ValueMultiplicity++;
						}
					pChar++;
					}

				// Reallocate the value buffer, allowing room for a set of pointers and a final null string terminator.
				pDicomElement -> Value.UN = realloc( pDicomElement -> Value.UN, pDicomElement -> ValueLength + 1 );
				if ( pDicomElement -> Value.UN == 0 )
					RespondToError( MODULE_DICOM, DICOM_ERROR_ALLOCATE_VALUE );
				else
					{
					// Point just beyond the end of the original value string.
					pChar = pDicomElement -> Value.LT + pDicomElement -> ValueLength;
					// Terminate the final character string (An extra byte was reallocated fir this).
					*pChar = 0;
					// If the value length is even and there was a trailing padding space, trim it off.
					if ( ( pDicomElement -> ValueLength & 1 ) == 0 )
						if ( *--pChar == ' ' )
							*pChar = 0;
					if ( *--pChar == '\n' )
						*pChar = 0;
					}
				break;
			default :
				pDicomElement -> ValueMultiplicity = 1;
				break;
			}			// ...end switch on VR.
		}
}


// From the standard:
// A.4	TRANSFER SYNTAXES FOR ENCAPSULATION OF ENCODED PIXEL DATA
// These Transfer Syntaxes apply to the encoding of the entire DICOM Data Set, even though the image Pixel Data (7FE0,0010)
// portion of the DICOM Data Set is the only portion that is encoded by an encapsulated format. This implies that when a DICOM
// Message is being encoded according to an encapsulation Transfer Syntax the following requirements shall be met:
//   a)	The Data Elements contained in the Data Set structure shall be encoded with Explicit VR (with a VR Field) as
//		specified in Section 7.1.2.
//	 b)	The encoding of the overall Data Set structure (Data Element Tags, Value Length, etc.) shall be in Little Endian as
//		specified in Section 7.3.
TRANSFER_SYNTAX InterpretUniqueTransferSyntaxIdentifier( char *pTransferSyntaxUniqueIdentifier )
{
	TRANSFER_SYNTAX			ImageDataTransferSyntax;
	TRANSFER_SYNTAX			DataSetTransferSyntax;

	ImageDataTransferSyntax = UNCOMPRESSED;			// Default.
	DataSetTransferSyntax = 0;
	if ( pTransferSyntaxUniqueIdentifier == 0 )
		RespondToError( MODULE_DICOM, DICOM_ERROR_TRANSFER_SYNTAX );
	else if ( strncmp( pTransferSyntaxUniqueIdentifier, "1.2.840.113619.5.2", 18 ) == 0 )
		{
		DataSetTransferSyntax = LITTLE_ENDIAN | IMPLICIT_VR;
		}
	else if ( strncmp( pTransferSyntaxUniqueIdentifier, "1.2.840.10008.1.2", 17 ) != 0 )
		RespondToError( MODULE_DICOM, DICOM_ERROR_TRANSFER_SYNTAX );
	// If this is a JPEG transfer syntax...
	else if ( strncmp( pTransferSyntaxUniqueIdentifier, "1.2.840.10008.1.2.4", 19 ) == 0 )
		{
		if (	strncmp( pTransferSyntaxUniqueIdentifier, "1.2.840.10008.1.2.4.50", 22 ) == 0 ||
				strncmp( pTransferSyntaxUniqueIdentifier, "1.2.840.10008.1.2.4.51", 22 ) == 0 ||
				strncmp( pTransferSyntaxUniqueIdentifier, "1.2.840.10008.1.2.4.52", 22 ) == 0 ||
				strncmp( pTransferSyntaxUniqueIdentifier, "1.2.840.10008.1.2.4.53", 22 ) == 0 ||
				strncmp( pTransferSyntaxUniqueIdentifier, "1.2.840.10008.1.2.4.54", 22 ) == 0 ||
				strncmp( pTransferSyntaxUniqueIdentifier, "1.2.840.10008.1.2.4.55", 22 ) == 0 ||
				strncmp( pTransferSyntaxUniqueIdentifier, "1.2.840.10008.1.2.4.56", 22 ) == 0 ||
				strncmp( pTransferSyntaxUniqueIdentifier, "1.2.840.10008.1.2.4.59", 22 ) == 0 ||
				strncmp( pTransferSyntaxUniqueIdentifier, "1.2.840.10008.1.2.4.60", 22 ) == 0 ||
				strncmp( pTransferSyntaxUniqueIdentifier, "1.2.840.10008.1.2.4.61", 22 ) == 0 ||
				strncmp( pTransferSyntaxUniqueIdentifier, "1.2.840.10008.1.2.4.62", 22 ) == 0 ||
				strncmp( pTransferSyntaxUniqueIdentifier, "1.2.840.10008.1.2.4.63", 22 ) == 0 ||
				strncmp( pTransferSyntaxUniqueIdentifier, "1.2.840.10008.1.2.4.64", 22 ) == 0 )
			{
			ImageDataTransferSyntax = COMPRESSED_LOSSY;
			// If an encapsulated syntax is to be used, then the syntax for data elements needs to default to:
			DataSetTransferSyntax = LITTLE_ENDIAN | EXPLICIT_VR;
			}
		else if (	strncmp( pTransferSyntaxUniqueIdentifier, "1.2.840.10008.1.2.4.57", 22 ) == 0 ||
					strncmp( pTransferSyntaxUniqueIdentifier, "1.2.840.10008.1.2.4.58", 22 ) == 0 ||
					strncmp( pTransferSyntaxUniqueIdentifier, "1.2.840.10008.1.2.4.65", 22 ) == 0 ||
					strncmp( pTransferSyntaxUniqueIdentifier, "1.2.840.10008.1.2.4.66", 22 ) == 0 ||
					strncmp( pTransferSyntaxUniqueIdentifier, "1.2.840.10008.1.2.4.70", 22 ) == 0 )
				{
				ImageDataTransferSyntax = COMPRESSED_LOSSLESS;
				// If an encapsulated syntax is to be used, then the syntax for data elements needs to default to:
				DataSetTransferSyntax = LITTLE_ENDIAN | EXPLICIT_VR;
				}
		else
			{
			ImageDataTransferSyntax = COMPRESSED_UNKNOWN;
			// If an encapsulated syntax is to be used, then the syntax for data elements needs to default to:
			DataSetTransferSyntax = LITTLE_ENDIAN | EXPLICIT_VR;
			}
		}
	else if ( strncmp( pTransferSyntaxUniqueIdentifier, "1.2.840.10008.1.2.5", 19 ) == 0 )
			{
			ImageDataTransferSyntax = COMPRESSED_RUN_LENGTH_ENCODED;
			// If an encapsulated syntax is to be used, then the syntax for data elements needs to default to:
			DataSetTransferSyntax = LITTLE_ENDIAN | EXPLICIT_VR;
			}
	else
		{
		if ( ( pTransferSyntaxUniqueIdentifier )[17] != '.' )
			// This is the default Dicom transfer syntax.
			DataSetTransferSyntax = LITTLE_ENDIAN | IMPLICIT_VR;
		else
			{
			switch( ( pTransferSyntaxUniqueIdentifier )[18] )
				{
				case '1':
				case '4':
					break;
				case '2':
					DataSetTransferSyntax = BIG_ENDIAN | EXPLICIT_VR;
					break;
				default:
					RespondToError( MODULE_DICOM, DICOM_ERROR_TRANSFER_SYNTAX );
					break;
				}
			}
		}
	if ( DataSetTransferSyntax != 0 )
		DataSetTransferSyntax |= ImageDataTransferSyntax;

	return DataSetTransferSyntax;
}


// Unpack/copy a few of the Dicom data elements needed for manipulating the file.
BOOL LoadExamInfoFromDicomHeader( EXAM_INFO *pExamInfo, DICOM_HEADER_SUMMARY *pDicomHeader )
{
	BOOL				bNoError = TRUE;
	char				TempString[ MAX_CFG_STRING_LENGTH ];
	char				*pStringPtr;
	unsigned long		nChars;

	DeallocateExamInfoAttributes( pExamInfo );
	pExamInfo -> pDicomInfo = pDicomHeader;

	if ( pDicomHeader -> PatientName != 0 && pDicomHeader -> PatientName -> pFirstName != 0 )
		{
		nChars = (unsigned long)strlen( pDicomHeader -> PatientName -> pFirstName ) + 1L;
		pExamInfo -> pFirstName = (char*)malloc( nChars );
		if ( pExamInfo -> pFirstName == 0 )
			bNoError = FALSE;
		else
			strcpy( pExamInfo -> pFirstName, pDicomHeader -> PatientName -> pFirstName );
		}
	if ( bNoError )
		{
		if ( pDicomHeader -> PatientName != 0 && pDicomHeader -> PatientName -> pLastName != 0 )
			{
			nChars = (unsigned long)strlen( pDicomHeader -> PatientName -> pLastName ) + 1L;
			pExamInfo -> pLastName = (char*)malloc( nChars );
			if ( pExamInfo -> pLastName == 0 )
				bNoError = FALSE;
			else
				{
				if ( pExamInfo -> pFirstName == 0 )
					{
					strcpy( TempString, pDicomHeader -> PatientName -> pLastName );
					pStringPtr = strchr( TempString, ' ' );
					if ( pStringPtr != NULL )
						{
						*pStringPtr = '\0';
						pExamInfo -> pFirstName = (char*)malloc( strlen(TempString) + 1L );
						if ( pExamInfo -> pFirstName == 0 )
							bNoError = FALSE;
						else
							{
							strcpy( pExamInfo -> pFirstName, TempString );
							strcpy( pExamInfo -> pLastName, ++pStringPtr );
							}
						}
					else
						{
						strcpy( pExamInfo -> pLastName, pDicomHeader -> PatientName -> pLastName );
						pExamInfo -> pFirstName = (char*)malloc( 2 );
						if ( pExamInfo -> pFirstName == 0 )
							bNoError = FALSE;
						else
							strcpy( pExamInfo -> pFirstName, "_" );
						}
					}
				else
					strcpy( pExamInfo -> pLastName, pDicomHeader -> PatientName -> pLastName );
				}
			}
		}
	if ( bNoError )
		{
		if ( pDicomHeader -> PatientID != 0 )
			{
			nChars = (unsigned long)strlen( pDicomHeader -> PatientID ) + 1L;
			pExamInfo -> pExamID = (char*)malloc( nChars );
			if ( pExamInfo -> pExamID == 0 )
				bNoError = FALSE;
			else
				strcpy( pExamInfo -> pExamID, pDicomHeader -> PatientID );
			}
		}
	if ( bNoError )
		{
		if ( pDicomHeader -> StudyDate != 0 )
			{
			nChars = 11L;
			pExamInfo -> pAppointmentDate = (char*)malloc( nChars );
			if ( pExamInfo -> pAppointmentDate == 0 )
				bNoError = FALSE;
			else
				{
				strcpy( pExamInfo -> pAppointmentDate, "" );
				strncat( pExamInfo -> pAppointmentDate, pDicomHeader -> StudyDate, 4 );
				strncat( pExamInfo -> pAppointmentDate, "-", 1 );
				strncat( pExamInfo -> pAppointmentDate, &pDicomHeader -> StudyDate[4], 2 );
				strncat( pExamInfo -> pAppointmentDate, "-", 1 );
				strncat( pExamInfo -> pAppointmentDate, &pDicomHeader -> StudyDate[6], 2 );
				}
			}
		}
	if ( bNoError )
		{
		if ( pDicomHeader -> StudyTime != 0 )
			{
			nChars = 9L;
			pExamInfo -> pAppointmentTime = (char*)malloc( nChars );
			if ( pExamInfo -> pAppointmentTime == 0 )
				bNoError = FALSE;
			else
				{
				strcpy( pExamInfo -> pAppointmentTime, "" );
				strncat( pExamInfo -> pAppointmentTime, pDicomHeader -> StudyTime, 2 );
				strncat( pExamInfo -> pAppointmentTime, ":", 1 );
				strncat( pExamInfo -> pAppointmentTime, &pDicomHeader -> StudyTime[2], 2 );
				strncat( pExamInfo -> pAppointmentTime, ":", 1 );
				strncat( pExamInfo -> pAppointmentTime, &pDicomHeader -> StudyTime[4], 2 );
				}
			}
		}
	if ( bNoError )
		{
		if ( pDicomHeader -> SeriesNumber != 0 )
			{
			nChars = (unsigned long)strlen( pDicomHeader -> SeriesNumber ) + 1L;
			pExamInfo -> pSeriesNumber = (char*)malloc( nChars );
			if ( pExamInfo -> pSeriesNumber == 0 )
				bNoError = FALSE;
			else
				strcpy( pExamInfo -> pSeriesNumber, pDicomHeader -> SeriesNumber );
			}
		}
	if ( bNoError )
		{
		if ( pDicomHeader -> SeriesDescription != 0 )
			{
			nChars = (unsigned long)strlen( pDicomHeader -> SeriesDescription ) + 1L;
			pExamInfo -> pSeriesDescription = (char*)malloc( nChars );
			if ( pExamInfo -> pSeriesDescription == 0 )
				bNoError = FALSE;
			else
				strcpy( pExamInfo -> pSeriesDescription, pDicomHeader -> SeriesDescription );
			}
		}
	if ( !bNoError )
		RespondToError( MODULE_DICOM, DICOM_ERROR_ALLOCATE_VALUE );

	return bNoError;
}


void FreeDicomHeaderInfo( DICOM_HEADER_SUMMARY *pDicomHeader )
{
	if ( pDicomHeader -> MetadataGroupLength != 0 )
		{
		free( pDicomHeader -> MetadataGroupLength );
		pDicomHeader -> MetadataGroupLength = 0;
		}
	if ( pDicomHeader -> FileMetaInformationVersion != 0 )
		{
		free( pDicomHeader -> FileMetaInformationVersion );
		pDicomHeader -> FileMetaInformationVersion = 0;
		}
	if ( pDicomHeader -> MediaStorageSOPClassUID != 0 )
		{
		free( pDicomHeader -> MediaStorageSOPClassUID );
		pDicomHeader -> MediaStorageSOPClassUID = 0;
		}
	if ( pDicomHeader -> MediaStorageSOPInstanceUID != 0 )
		{
		free( pDicomHeader -> MediaStorageSOPInstanceUID );
		pDicomHeader -> MediaStorageSOPInstanceUID = 0;
		}
	if ( pDicomHeader -> TransferSyntaxUniqueIdentifier != 0 )
		{
		free( pDicomHeader -> TransferSyntaxUniqueIdentifier );
		pDicomHeader -> TransferSyntaxUniqueIdentifier = 0;
		}
	if ( pDicomHeader -> ImplementationClassUID != 0 )
		{
		free( pDicomHeader -> ImplementationClassUID );
		pDicomHeader -> ImplementationClassUID = 0;
		}
	if ( pDicomHeader -> ImplementationVersionName != 0 )
		{
		free( pDicomHeader -> ImplementationVersionName );
		pDicomHeader -> ImplementationVersionName = 0;
		}
	if ( pDicomHeader -> SourceAE_TITLE != 0 )
		{
		free( pDicomHeader -> SourceAE_TITLE );
		pDicomHeader -> SourceAE_TITLE = 0;
		}
	if ( pDicomHeader -> DestinationAE_TITLE != 0 )
		{
		free( pDicomHeader -> DestinationAE_TITLE );
		pDicomHeader -> DestinationAE_TITLE = 0;
		}
	if ( pDicomHeader -> ImageTypeUniqueIdentifier != 0 )
		{
		free( pDicomHeader -> ImageTypeUniqueIdentifier );
		pDicomHeader -> ImageTypeUniqueIdentifier = 0;
		}
	if ( pDicomHeader -> DataCreationDate != 0 )
		{
		free( pDicomHeader -> DataCreationDate );
		pDicomHeader -> DataCreationDate = 0;
		}
	if ( pDicomHeader -> DataCreationTime != 0 )
		{
		free( pDicomHeader -> DataCreationTime );
		pDicomHeader -> DataCreationTime = 0;
		}
	if ( pDicomHeader -> DataCreatorUniqueIdentifier != 0 )
		{
		free( pDicomHeader -> DataCreatorUniqueIdentifier );
		pDicomHeader -> DataCreatorUniqueIdentifier = 0;
		}
	if ( pDicomHeader -> SOPClassUniqueIdentifier != 0 )
		{
		free( pDicomHeader -> SOPClassUniqueIdentifier );
		pDicomHeader -> SOPClassUniqueIdentifier = 0; 
		}
	if ( pDicomHeader -> SOPInstanceUniqueIdentifier != 0 )
		{
		free( pDicomHeader -> SOPInstanceUniqueIdentifier );
		pDicomHeader -> SOPInstanceUniqueIdentifier = 0;
		}
	if ( pDicomHeader -> StudyDate != 0 )
		{
		free( pDicomHeader -> StudyDate );
		pDicomHeader -> StudyDate = 0;
		}
	if ( pDicomHeader -> SeriesDate != 0 )
		{
		free( pDicomHeader -> SeriesDate );
		pDicomHeader -> SeriesDate = 0;
		}
	if ( pDicomHeader -> AcquisitionDate != 0 )
		{
		free( pDicomHeader -> AcquisitionDate );
		pDicomHeader -> AcquisitionDate = 0;
		}
	if ( pDicomHeader -> StudyTime != 0 )
		{
		free( pDicomHeader -> StudyTime );
		pDicomHeader -> StudyTime = 0;
		}
	if ( pDicomHeader -> SeriesTime != 0 )
		{
		free( pDicomHeader -> SeriesTime );
		pDicomHeader -> SeriesTime = 0;
		}
	if ( pDicomHeader -> AcquisitionTime != 0 )
		{
		free( pDicomHeader -> AcquisitionTime );
		pDicomHeader -> AcquisitionTime = 0;
		}
	if ( pDicomHeader -> AccessionNumber != 0 )
		{
		free( pDicomHeader -> AccessionNumber );
		pDicomHeader -> AccessionNumber = 0;
		}
	if ( pDicomHeader -> Modality != 0 )
		{
		free( pDicomHeader -> Modality );
		pDicomHeader -> Modality = 0;
		}
	if ( pDicomHeader -> InstitutionName != 0 )
		{
		free( pDicomHeader -> InstitutionName );
		pDicomHeader -> InstitutionName = 0;
		}
	if ( pDicomHeader -> ReferringPhysician != 0 )
		{
		if ( pDicomHeader -> ReferringPhysician -> pFirstName != 0 )
			free( pDicomHeader -> ReferringPhysician -> pFirstName );
		if ( pDicomHeader -> ReferringPhysician -> pLastName != 0 )
			free( pDicomHeader -> ReferringPhysician -> pLastName );
		if ( pDicomHeader -> ReferringPhysician -> pMiddleName != 0 )
			free( pDicomHeader -> ReferringPhysician -> pMiddleName );
		if ( pDicomHeader -> ReferringPhysician -> pPrefix != 0 )
			free( pDicomHeader -> ReferringPhysician -> pPrefix );
		if ( pDicomHeader -> ReferringPhysician -> pSuffix != 0 )
			free( pDicomHeader -> ReferringPhysician -> pSuffix );
		free( pDicomHeader -> ReferringPhysician );
		pDicomHeader -> ReferringPhysician = 0;
		}
	if ( pDicomHeader -> StudyDescription != 0 )
		{
		free( pDicomHeader -> StudyDescription );
		pDicomHeader -> StudyDescription = 0;
		}
	if ( pDicomHeader -> SeriesDescription != 0 )
		{
		free(pDicomHeader -> SeriesDescription  );
		pDicomHeader -> SeriesDescription = 0;
		}
	if ( pDicomHeader -> AdmittingDiagnosesDescription != 0 )
		{
		free(pDicomHeader -> AdmittingDiagnosesDescription  );
		pDicomHeader -> AdmittingDiagnosesDescription = 0;
		}
	if ( pDicomHeader -> PatientName != 0 )
		{
		if ( pDicomHeader -> PatientName -> pFirstName != 0 )
			free( pDicomHeader -> PatientName -> pFirstName );
		if ( pDicomHeader -> PatientName -> pLastName != 0 )
			free( pDicomHeader -> PatientName -> pLastName );
		if ( pDicomHeader -> PatientName -> pMiddleName != 0 )
			free( pDicomHeader -> PatientName -> pMiddleName );
		if ( pDicomHeader -> PatientName -> pPrefix != 0 )
			free( pDicomHeader -> PatientName -> pPrefix );
		if ( pDicomHeader -> PatientName -> pSuffix != 0 )
			free( pDicomHeader -> PatientName -> pSuffix );
		free( pDicomHeader -> PatientName );
		pDicomHeader -> PatientName = 0;
		}
	if ( pDicomHeader -> PatientID != 0 )
		{
		free( pDicomHeader -> PatientID );
		pDicomHeader -> PatientID = 0;
		}
	if ( pDicomHeader -> PatientBirthDate != 0 )
		{
		free( pDicomHeader -> PatientBirthDate );
		pDicomHeader -> PatientBirthDate = 0;
		}
	if ( pDicomHeader -> PatientSex != 0 )
		{
		free( pDicomHeader -> PatientSex );
		pDicomHeader -> PatientSex = 0;
		}
	if ( pDicomHeader -> PatientAge != 0 )
		{
		free( pDicomHeader -> PatientAge );
		pDicomHeader -> PatientAge = 0;
		}
	if ( pDicomHeader -> AdditionalPatientHistory != 0 )
		{
		free( pDicomHeader -> AdditionalPatientHistory );
		pDicomHeader -> AdditionalPatientHistory = 0;
		}
	if ( pDicomHeader -> PatientComments != 0 )
		{
		free( pDicomHeader -> PatientComments );
		pDicomHeader -> PatientComments = 0;
		}
	if ( pDicomHeader -> BodyPartExamined != 0 )
		{
		free( pDicomHeader -> BodyPartExamined );
		pDicomHeader -> BodyPartExamined = 0;
		}
	if ( pDicomHeader -> ImagerPixelSpacing != 0 )
		{
		free( pDicomHeader -> ImagerPixelSpacing );
		pDicomHeader -> ImagerPixelSpacing = 0;
		}
	if ( pDicomHeader -> PatientPosition != 0 )
		{
		free( pDicomHeader -> PatientPosition );
		pDicomHeader -> PatientPosition = 0;
		}
	if ( pDicomHeader -> ViewPosition != 0 )
		{
		free( pDicomHeader -> ViewPosition );
		pDicomHeader -> ViewPosition = 0;
		}
	if ( pDicomHeader -> StudyInstanceUID != 0 )
		{
		free( pDicomHeader -> StudyInstanceUID );
		pDicomHeader -> StudyInstanceUID = 0;
		}
	if ( pDicomHeader -> SeriesInstanceUID != 0 )
		{
		free( pDicomHeader -> SeriesInstanceUID );
		pDicomHeader -> SeriesInstanceUID = 0;
		}
	if ( pDicomHeader -> StudyID != 0 )
		{
		free( pDicomHeader -> StudyID );
		pDicomHeader -> StudyID = 0;
		}
	if ( pDicomHeader -> SeriesNumber != 0 )
		{
		free( pDicomHeader -> SeriesNumber );
		pDicomHeader -> SeriesNumber = 0;
		}
	if ( pDicomHeader -> AcquisitionNumber != 0 )
		{
		free( pDicomHeader -> AcquisitionNumber );
		pDicomHeader -> AcquisitionNumber = 0;
		}
	if ( pDicomHeader -> InstanceNumber != 0 )
		{
		free( pDicomHeader -> InstanceNumber );
		pDicomHeader -> InstanceNumber = 0;
		}
	if ( pDicomHeader -> ImagesInAcquisition != 0 )
		{
		free( pDicomHeader -> ImagesInAcquisition );
		pDicomHeader -> ImagesInAcquisition = 0;
		}
	if ( pDicomHeader -> ImageComments != 0 )
		{
		free( pDicomHeader -> ImageComments );
		pDicomHeader -> ImageComments = 0;
		}
}


void CopyImageFileToSortTreeDirectory( DICOM_HEADER_SUMMARY *pDicomHeader, char *DicomFileSpecification,
										char *Manufacturer, char *Modality )
{
	BOOL		bNoError = TRUE;
	char		DestinationFileSpec[ MAX_FILE_SPEC_LENGTH ];
	char		TempText[ MAX_FILE_SPEC_LENGTH ];

	strcpy( DestinationFileSpec, "C:\\tom\\Dicom Images\\SortedImages\\" );
	if ( Manufacturer != 0 )
		{
		if ( strlen( Manufacturer ) > 0 )
			{
			strcpy( TempText, Manufacturer );
			PruneEmbeddedSpaceAndPunctuation( TempText );
			}
		else
			strcpy( TempText, "Unspecified" );
		}
	else
		strcpy( TempText, "Unspecified" );
	strcat( DestinationFileSpec, TempText );
	LocateOrCreateDirectory( DestinationFileSpec );
	strcat( DestinationFileSpec, "\\" );
	if ( Modality != 0 )
		{
		if ( strlen( Modality ) > 0 )
			strcat( DestinationFileSpec, Modality );
		else
			strcat( DestinationFileSpec, "Blank" );
		}
	else
		strcat( DestinationFileSpec, "Blank" );
	LocateOrCreateDirectory( DestinationFileSpec );
	strcat( DestinationFileSpec, "\\" );
	strcat( DestinationFileSpec, pDicomHeader -> MediaStorageSOPInstanceUID );
	strcat( DestinationFileSpec, ".dcm" );
	bNoError = CopyFile( DicomFileSpecification, DestinationFileSpec, FALSE );
}


void AddImageToSurvey( DICOM_HEADER_SUMMARY *pDicomHeader, char *DicomFileSpecification )
{
	FILE						*pSurveyFile;
	char						SurveyFileSpec[ MAX_FILE_SPEC_LENGTH ];
	char						OutputTextLine[ 2048 ];
	char						TextValue[ 64 ];
	DWORD						SurveyFileSize;

	strcpy( SurveyFileSpec, "" );
	strncat( SurveyFileSpec, ServiceConfiguration.ExportsDirectory, MAX_FILE_SPEC_LENGTH - 1 );
	LocateOrCreateDirectory( SurveyFileSpec );	// Ensure directory exists.
	if ( SurveyFileSpec[ strlen( SurveyFileSpec ) - 1 ] != '\\' )
		strcat( SurveyFileSpec, "\\" );
	strncat( SurveyFileSpec, "ImageSurvey.txt", MAX_FILE_SPEC_LENGTH - 1 - strlen( SurveyFileSpec ) );
	SurveyFileSize = GetCompressedFileSize( SurveyFileSpec, NULL );
	pSurveyFile = fopen( SurveyFileSpec, "at" );
	if ( pSurveyFile != 0 )
		{
		// Prefix with a line of column headings, if this is a new file.
		if ( SurveyFileSize == 0 || SurveyFileSize == INVALID_FILE_SIZE )
			{
			strcpy( OutputTextLine, "" );
			strcat( OutputTextLine, "SOPInstanceUniqueIdentifier," );
			strcat( OutputTextLine, "DicomFileName," );
			strcat( OutputTextLine, "Modality," );
			strcat( OutputTextLine, "Manufacturer," );
			strcat( OutputTextLine, "ManufacturersModelName," );
			strcat( OutputTextLine, "PhotometricInterpretation," );
			strcat( OutputTextLine, "SignedPixelValues," );
			strcat( OutputTextLine, "ImageRows," );
			strcat( OutputTextLine, "ImageColumns," );
			strcat( OutputTextLine, "BitsAllocated," );
			strcat( OutputTextLine, "BitsStored," );
			strcat( OutputTextLine, "SamplesPerPixel," );
			strcat( OutputTextLine, "CalibrationTypes," );
			strcat( OutputTextLine, "RescaleIntercept," );
			strcat( OutputTextLine, "RescaleSlope," );
			strcat( OutputTextLine, "ModalityOutputUnits," );
			strcat( OutputTextLine, "ModalityLUTElementCount," );
			strcat( OutputTextLine, "ModalityLUTThresholdPixelValue," );
			strcat( OutputTextLine, "ModalityLUTBitDepth," );
			strcat( OutputTextLine, "ModalityLUTDataBufferSize," );
			strcat( OutputTextLine, "WindowCenter," );
			strcat( OutputTextLine, "WindowWidth," );
			strcat( OutputTextLine, "WindowFunction," );
			strcat( OutputTextLine, "VOI_LUTElementCount," );
			strcat( OutputTextLine, "VOI_LUTThresholdPixelValue," );
			strcat( OutputTextLine, "VOI_LUTBitDepth," );
			strcat( OutputTextLine, "VOI_LUTDataBufferSize" );
			strcat( OutputTextLine, "\n" );
			fputs( OutputTextLine, pSurveyFile );
			}
		strcpy( OutputTextLine, "" );
		if ( pDicomHeader -> MediaStorageSOPInstanceUID != NULL )
			strcat( OutputTextLine, pDicomHeader -> MediaStorageSOPInstanceUID );
		strcat( OutputTextLine, "," );
		if ( DicomFileSpecification != 0 )
			strcat( OutputTextLine, DicomFileSpecification );
		strcat( OutputTextLine, "," );
		if ( pDicomHeader -> Modality != 0 )
			strcat( OutputTextLine, pDicomHeader -> Modality );
		strcat( OutputTextLine, "," );
		if ( pDicomHeader -> Manufacturer != 0 )
			strcat( OutputTextLine, pDicomHeader -> Manufacturer );
		strcat( OutputTextLine, "," );
		if ( pDicomHeader -> ManufacturersModelName != 0 )
			strcat( OutputTextLine, pDicomHeader -> ManufacturersModelName );
		strcat( OutputTextLine, "," );
		if ( pDicomHeader -> PhotometricInterpretation != 0 )
			strcat( OutputTextLine, pDicomHeader -> PhotometricInterpretation );
		strcat( OutputTextLine, "," );
		if ( pDicomHeader -> CalibrationInfo.bPixelValuesAreSigned )
			strcat( OutputTextLine, "Yes," );
		else
			strcat( OutputTextLine, "No," );
		_itoa( *pDicomHeader -> ImageRows, TextValue, 10 );
		strcat( OutputTextLine, TextValue );
		strcat( OutputTextLine, "," );
		_itoa( *pDicomHeader -> ImageColumns, TextValue, 10 );
		strcat( OutputTextLine, TextValue );
		strcat( OutputTextLine, "," );
		_itoa( *pDicomHeader -> BitsAllocated, TextValue, 10 );
		strcat( OutputTextLine, TextValue );
		strcat( OutputTextLine, "," );
		_itoa( *pDicomHeader -> BitsStored, TextValue, 10 );
		strcat( OutputTextLine, TextValue );
		strcat( OutputTextLine, "," );
		_itoa( *pDicomHeader -> SamplesPerPixel, TextValue, 10 );
		strcat( OutputTextLine, TextValue );
		strcat( OutputTextLine, "," );
		strcat( OutputTextLine, "> MODALITY_" );
		if ( pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes & CALIBRATION_TYPE_MODALITY_RESCALE  )
			strcat( OutputTextLine, "RESCALE " );
		if ( pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes & CALIBRATION_TYPE_MODALITY_LUT  )
			strcat( OutputTextLine, "LUT " );
		if ( pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes & CALIBRATION_ACTIVE_MODALITY_LUT  )
			strcat( OutputTextLine, "ACTIVE " );
		strcat( OutputTextLine, "> VOI_" );
		if ( pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes & CALIBRATION_TYPE_VOI_WINDOW  )
			strcat( OutputTextLine, "WINDOW " );
		if ( pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes & CALIBRATION_TYPE_VOI_LUT  )
			strcat( OutputTextLine, "LUT " );
		if ( pDicomHeader -> CalibrationInfo.SpecifiedCalibrationTypes & CALIBRATION_ACTIVE_VOI_LUT  )
			strcat( OutputTextLine, "ACTIVE " );
		strcat( OutputTextLine, "," );
		sprintf( TextValue, "%8.3f", pDicomHeader -> CalibrationInfo.RescaleIntercept );
		strcat( OutputTextLine, TextValue );
		strcat( OutputTextLine, "," );
		sprintf( TextValue, "%8.3f", pDicomHeader -> CalibrationInfo.RescaleSlope );
		strcat( OutputTextLine, TextValue );
		strcat( OutputTextLine, "," );
		if ( pDicomHeader -> CalibrationInfo.ModalityOutputUnits != 0 )
			strcat( OutputTextLine, pDicomHeader -> CalibrationInfo.ModalityOutputUnits );
		strcat( OutputTextLine, "," );
		_itoa( pDicomHeader -> CalibrationInfo.ModalityLUTElementCount, TextValue, 10 );
		strcat( OutputTextLine, TextValue );
		strcat( OutputTextLine, "," );
		_itoa( pDicomHeader -> CalibrationInfo.ModalityLUTThresholdPixelValue, TextValue, 10 );
		strcat( OutputTextLine, TextValue );
		strcat( OutputTextLine, "," );
		_itoa( pDicomHeader -> CalibrationInfo.ModalityLUTBitDepth, TextValue, 10 );
		strcat( OutputTextLine, TextValue );
		strcat( OutputTextLine, "," );
		_itoa( pDicomHeader -> CalibrationInfo.ModalityLUTDataBufferSize, TextValue, 10 );
		strcat( OutputTextLine, TextValue );
		strcat( OutputTextLine, "," );
		sprintf( TextValue, "%8.1f", pDicomHeader -> CalibrationInfo.WindowCenter );
		strcat( OutputTextLine, TextValue );
		strcat( OutputTextLine, "," );
		sprintf( TextValue, "%8.1f", pDicomHeader -> CalibrationInfo.WindowWidth );
		strcat( OutputTextLine, TextValue );
		strcat( OutputTextLine, "," );
		if ( pDicomHeader -> CalibrationInfo.WindowFunction == WINDOW_FUNCTION_LINEAR )
			strcat( OutputTextLine, "LINEAR," );
		else if ( pDicomHeader -> CalibrationInfo.WindowFunction == WINDOW_FUNCTION_SIGMOID )
			strcat( OutputTextLine, "SIGMOID," );
		else
			strcat( OutputTextLine, "," );
		_itoa( pDicomHeader -> CalibrationInfo.VOI_LUTElementCount, TextValue, 10 );
		strcat( OutputTextLine, TextValue );
		strcat( OutputTextLine, "," );
		_itoa( pDicomHeader -> CalibrationInfo.VOI_LUTThresholdPixelValue, TextValue, 10 );
		strcat( OutputTextLine, TextValue );
		strcat( OutputTextLine, "," );
		_itoa( pDicomHeader -> CalibrationInfo.VOI_LUTBitDepth, TextValue, 10 );
		strcat( OutputTextLine, TextValue );
		strcat( OutputTextLine, "," );
		_itoa( pDicomHeader -> CalibrationInfo.VOI_LUTDataBufferSize, TextValue, 10 );
		strcat( OutputTextLine, TextValue );
		strcat( OutputTextLine, "\n" );

		// Output the text line describing this image.
		fputs( OutputTextLine, pSurveyFile );
		fclose( pSurveyFile );
		}

}


void CloseDicomFile( FILE *pDicomFile )
{
	if ( pDicomFile != 0 )
		fclose( pDicomFile );
}


TRANSFER_SYNTAX GetConsistentTransferSyntaxFromBuffer( TRANSFER_SYNTAX DeclaredTransferSyntax, char *pBufferReadPoint )
{
	TRANSFER_SYNTAX			RealTransferSyntax;
	DICOM_ELEMENT			DicomElement;
	DICOM_DICTIONARY_ITEM	*pDictItem;
	char					ExternalValueRepresentation[2];
	VR						InternalValueRepresentation;
	BOOL					bElementWasFoundInDictionary;

	RealTransferSyntax = DeclaredTransferSyntax;		// Default.
	// Check if the first element is readable with the currently declared transfer syntax.  Since some applications
	// either don't follow the Dicom standard's formatting rules, or else they announce a different presentation
	// context than the one they are actually using, just rely on testing the data to determine what the actual
	// formatting is.  (The Dicom "standard" is not very standardized in its application.  Ease of implementation
	// does not appear to have been a consideration in its design.)
	DicomElement.Tag.Group = *((unsigned short*)pBufferReadPoint );
	DicomElement.Tag.Element = *((unsigned short*)pBufferReadPoint + 1 );
	pDictItem = GetDicomElementFromDictionary( DicomElement.Tag );
	if ( pDictItem != 0 )
		bElementWasFoundInDictionary = ( DicomElement.Tag.Group == pDictItem -> Group && DicomElement.Tag.Element == pDictItem -> Element );
	else
		bElementWasFoundInDictionary = FALSE;
	memcpy( ExternalValueRepresentation, pBufferReadPoint + 4, 2 );
	*((unsigned long*)( &InternalValueRepresentation )) = 0L;
	*((unsigned short*)( &InternalValueRepresentation )) = (unsigned short)( ExternalValueRepresentation[ 0 ] << 8 );
	*((unsigned short*)( &InternalValueRepresentation )) |= (unsigned short)( ExternalValueRepresentation[ 1 ] );
	// Handle declared explicit VR.
	if ( bElementWasFoundInDictionary && InternalValueRepresentation != pDictItem -> ValueRepresentation &&
				pDictItem -> ValueRepresentation != UN && ( DeclaredTransferSyntax & EXPLICIT_VR ) != 0 &&
				!HasRecognizedValueRepresentation( InternalValueRepresentation ) )
		{
		RealTransferSyntax &= ~EXPLICIT_VR;
		RealTransferSyntax |= IMPLICIT_VR;
		}
	// Handle declared implicit VR.
	else if ( bElementWasFoundInDictionary && ( InternalValueRepresentation == pDictItem -> ValueRepresentation ||
				( pDictItem -> ValueRepresentation == UN && HasRecognizedValueRepresentation( InternalValueRepresentation ) ) ) &&
							( DeclaredTransferSyntax & IMPLICIT_VR ) != 0 )
		{
		RealTransferSyntax &= ~IMPLICIT_VR;
		RealTransferSyntax |= EXPLICIT_VR;
		}
	
	return RealTransferSyntax;
}


TRANSFER_SYNTAX GetConsistentTransferSyntax( TRANSFER_SYNTAX DeclaredTransferSyntax, LIST_ELEMENT **ppBufferListElement )
{
	DICOM_DATA_BUFFER		*pDicomBuffer;
	unsigned long			nBytesProcessed;
	char					*pBufferReadPoint;
	TRANSFER_SYNTAX			RealTransferSyntax;

	pDicomBuffer = (DICOM_DATA_BUFFER*)(*ppBufferListElement) -> pItem;
	nBytesProcessed = pDicomBuffer -> DataSize - pDicomBuffer -> BytesRemainingToBeProcessed;
	pBufferReadPoint = pDicomBuffer -> pBeginningOfDicomData + nBytesProcessed;
	RealTransferSyntax = GetConsistentTransferSyntaxFromBuffer( DeclaredTransferSyntax, pBufferReadPoint );
	
	return RealTransferSyntax;
}


// Typically, nValueSize is 2 for a short integer and 4 for a long integer.
// This function will work for integers of any even byte count.
static void SwapBytesFromFile( void *pData, long nValueSize, TRANSFER_SYNTAX TransferSyntax )
{
	long			nByte;
	char			TempChar;
	char			*pFirstChar;
	char			*pLastChar;
	
	// If the current transfer syntax byte order is different from this computer's
	// internal memory byte order, perform the byte swap.
	if ( ( TransferSyntax & LocalMemoryByteOrder ) == 0 )
		{
		pFirstChar = (char*)pData;
		pLastChar = (char*)pData + nValueSize - 1;
		for ( nByte = 0; nByte < nValueSize / 2; nByte++ )
			{
			TempChar = *pFirstChar;
			*pFirstChar = *pLastChar;
			*pLastChar = TempChar;
			pFirstChar++;
			pLastChar--;
			}
		}
}


ASSOCIATED_IMAGE_INFO *CreateAssociatedImageInfo()
{
	ASSOCIATED_IMAGE_INFO	*pAssociatedImageInfo;

	pAssociatedImageInfo = (ASSOCIATED_IMAGE_INFO*)malloc( sizeof(ASSOCIATED_IMAGE_INFO) );
	if ( pAssociatedImageInfo != 0 )
		{
		strcpy( pAssociatedImageInfo -> StudyName, "" );
		strcpy( pAssociatedImageInfo -> CurrentDicomFileName, "" );
		strcpy( pAssociatedImageInfo -> LocalImageFileSpecification, "" );
		pAssociatedImageInfo -> pImageDataFile = 0;
		}
	else
		RespondToError( MODULE_DICOM, DICOM_ERROR_INSUFFICIENT_MEMORY );

	return pAssociatedImageInfo;
}


BOOL ArchiveDicomImageFile( char *pQueuedDicomFileSpec, char *pPNGImageFileName )
{
	BOOL						bNoError = TRUE;
	char						DicomImageFileName[ MAX_FILE_SPEC_LENGTH ];
	char						DicomImageFileSpec[ MAX_FILE_SPEC_LENGTH ];
	char						DicomImageArchiveFileSpec[ MAX_FILE_SPEC_LENGTH ];
	char						*pChar;
	char						Msg[ 1024 ];
	DWORD						SystemErrorCode;

	// Archive the Dicom image file, if requested.
	if ( strlen( ServiceConfiguration.DicomImageArchiveDirectory ) > 0 )
		{
		strcpy( DicomImageFileName, pPNGImageFileName );
		pChar = strstr( DicomImageFileName, ".png" );
		strcpy( pChar, ".dcm" );
		// Get the file specification for the current Dicom image file.
		strcpy( DicomImageFileSpec, pQueuedDicomFileSpec );
		
		// Get the file specification for the destination (archived) Dicom image file.
		strcpy( DicomImageArchiveFileSpec, "" );
		strncat( DicomImageArchiveFileSpec, ServiceConfiguration.DicomImageArchiveDirectory, MAX_FILE_SPEC_LENGTH - 1 );
		LocateOrCreateDirectory( DicomImageArchiveFileSpec );	// Ensure directory exists.
		if ( DicomImageFileSpec[ strlen( DicomImageArchiveFileSpec ) - 1 ] != '\\' )
			strcat( DicomImageArchiveFileSpec, "\\" );
		strncat( DicomImageArchiveFileSpec, DicomImageFileName, MAX_FILE_SPEC_LENGTH - 1 - strlen( DicomImageArchiveFileSpec ) );
		
		// Copy the current Dicom image file to the archive directory.
		sprintf( Msg, "    Copying current Dicom image file:  %s to the archive folder", DicomImageFileSpec );
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );

		bNoError = CopyFile( DicomImageFileSpec, DicomImageArchiveFileSpec, FALSE );
		if ( !bNoError )
			{
			SystemErrorCode = GetLastError();
			sprintf( Msg, "   >>> Copy to Dicom image archive system error code %d", SystemErrorCode );
			LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
			}
		}

	return bNoError;
}


// This utility function is useful for debugging/examining the Dicom output.
void ExamineDicomElementList( DICOM_HEADER_SUMMARY *pDicomHeader )
{
	BOOL						bNoError = TRUE;
	LIST_ELEMENT				*pDicomDataListElement;
	DICOM_ELEMENT				*pDicomElement;

	pDicomDataListElement = pDicomHeader -> ListOfDicomElements;
	while ( pDicomDataListElement != 0 )
		{
		pDicomElement = (DICOM_ELEMENT*)pDicomDataListElement -> pItem;
		if ( pDicomElement -> Tag.Group == 0x0008 && pDicomElement -> Tag.Element == 0x0008 )
			bNoError = TRUE;
		pDicomDataListElement = pDicomDataListElement -> pNextListElement;
		}

}


BOOL ComposeDicomFileOutput( char *pQueuedDicomFileSpec, char *pPNGImageFileName, EXAM_INFO *pExamInfo )
{
	BOOL						bNoError = TRUE;
	BOOL						bEditSpecsReadOK = FALSE;
	BOOL						bDicomBuffersAllocatedOK = FALSE;
	BOOL						bCropImage = FALSE;
	char						DicomImageFileName[ MAX_FILE_SPEC_LENGTH ];
	char						DicomImageArchiveFileSpec[ MAX_FILE_SPEC_LENGTH ];
	char						*pChar;
	char						TransferSyntaxUniqueIdentifier[] = "1.2.840.10008.1.2.1";		// Transfer syntax for uncompressed.
	DICOM_HEADER_SUMMARY		*pDicomHeader;
	EDIT_SPECIFICATION			*pEditSpecification;
	char						*pBuffer;
	DICOM_DATA_BUFFER			*pDicomBuffer;
	LIST_ELEMENT				*pDicomDataListElement;
	LIST_ELEMENT				*pBufferListElement;
	LIST_ELEMENT				*pEditSpecificationListElement;
	DICOM_ELEMENT				*pDicomElement;
	BOOL						bEditSpecifiedForThisElement;
	BOOL						bLogDicomElements = TRUE;
	TRANSFER_SYNTAX				CurrentEncodingType;
	char						EditedFieldValue[ MAX_CFG_STRING_LENGTH ];
	char						*pEditFieldText;
	unsigned long				nCroppedImageX0;
	unsigned long				nCroppedImageY0;
	unsigned long				nCroppedImageWidth;
	unsigned long				nCroppedImageHeight;
	char						OriginalImageSizeText[ 32 ];
	char						OverlayImageWidthText[ 32 ];
	char						OverlayImageHeightText[ 32 ];
	char						OverlayImageX0Text[ 32 ];
	char						OverlayImageY0Text[ 32 ];
	char						OverlayImageFileSizeText[ 32 ];
	char						OverlayImageFileSpec[ 128 ];
	FILE						*pOverlayImageFile;
	char						*pJPEGOverlayImageBuffer;
	unsigned long				nBytesRead;
	unsigned long				nOverlayImageWidth;
	unsigned long				nOverlayImageHeight;
	unsigned long				nOverlayImageX0;
	unsigned long				nOverlayImageY0;
	unsigned long				nOverlayImageFileSize;
	size_t						nBytesComposed;
	FILE						*pOutputFile;
	long						nBytesToBeWritten;
	long						nBytesWritten;
	unsigned long				ImageWidthInPixels;
	unsigned long				ImageHeightInPixels;
	char						*pDecompressedImageData;
	unsigned long				DecompressedImageSizeInBytes;
	char						Msg[ 1024 ];

	// Archive the Dicom image file, if requested.
	if ( ServiceConfiguration.bComposeDicomOutputFile )
		{
		strcpy( DicomImageFileName, pPNGImageFileName );
		pChar = strstr( DicomImageFileName, ".png" );
		strcpy( pChar, ".dcm" );
		// Get the file specification for the edited Dicom image output file.
		strcpy( DicomImageArchiveFileSpec, "" );
		strncat( DicomImageArchiveFileSpec, ServiceConfiguration.DicomImageArchiveDirectory, MAX_FILE_SPEC_LENGTH - 1 );
		LocateOrCreateDirectory( DicomImageArchiveFileSpec );	// Ensure directory exists.
		if ( DicomImageArchiveFileSpec[ strlen( DicomImageArchiveFileSpec ) - 1 ] != '\\' )
			strcat( DicomImageArchiveFileSpec, "\\" );
		strncat( DicomImageArchiveFileSpec, DicomImageFileName, MAX_FILE_SPEC_LENGTH - 1 - strlen( DicomImageArchiveFileSpec ) );
		
		// Read in any general edit specifications for the Dicom elements to be output.
		if ( pExamInfo != 0 )
			{
			pDicomHeader = pExamInfo -> pDicomInfo;
			if ( pDicomHeader != 0 )
				{
				pDicomHeader -> ListOfEditSpecifications = 0;
				if ( ServiceConfiguration.bApplyManualDicomEdits )
					bEditSpecsReadOK = ReadExamEditSpecificationFile( &pDicomHeader -> ListOfEditSpecifications );
				// Set the transfer syntax of the output image to be uncompressed.
				if ( strlen( pDicomHeader -> TransferSyntaxUniqueIdentifier ) < strlen( TransferSyntaxUniqueIdentifier ) )
					pDicomHeader -> TransferSyntaxUniqueIdentifier = (char*)realloc( pDicomHeader -> TransferSyntaxUniqueIdentifier, strlen( TransferSyntaxUniqueIdentifier + 2 ) );

				bNoError = ( pDicomHeader -> TransferSyntaxUniqueIdentifier != 0 );
				if ( bNoError )
					{
					strcpy( pDicomHeader -> TransferSyntaxUniqueIdentifier, TransferSyntaxUniqueIdentifier );
					pDicomHeader -> FileDecodingPlan.nTransferSyntaxIndex = GetTransferSyntaxIndex( pDicomHeader -> TransferSyntaxUniqueIdentifier,
																			(unsigned short)strlen( pDicomHeader -> TransferSyntaxUniqueIdentifier ) );
					pDicomHeader -> FileDecodingPlan.ImageDataTransferSyntax = UNCOMPRESSED;
					}
				else
					RespondToError( MODULE_DICOM, DICOM_ERROR_INSUFFICIENT_MEMORY );
				if ( bNoError )
					{
					// Create the first buffer.
					pDicomHeader -> ListOfOutputBuffers = 0;
					pBuffer = (char*)malloc( MAX_DICOM_READ_BUFFER_SIZE );		// Allocate a 64K buffer.
					pDicomBuffer = (DICOM_DATA_BUFFER*)malloc( sizeof(DICOM_DATA_BUFFER) );
					if ( pBuffer == 0 || pDicomBuffer == 0 )
						{
						bNoError = FALSE;
						RespondToError( MODULE_DICOM, DICOM_ERROR_INSUFFICIENT_MEMORY );
						}
					else
						{
						bDicomBuffersAllocatedOK = TRUE;
						pDicomBuffer -> pBuffer = pBuffer;
						pDicomBuffer -> pBeginningOfDicomData = pBuffer;
						pDicomBuffer -> BufferSize = MAX_DICOM_READ_BUFFER_SIZE;
						pDicomBuffer -> DataSize = 0L;
						pDicomBuffer -> BytesRemainingToBeProcessed = MAX_DICOM_READ_BUFFER_SIZE;
						// Link the new buffer to the list.
						bNoError = AppendToList( &pDicomHeader -> ListOfOutputBuffers, (void*)pDicomBuffer );
						}
					pDicomDataListElement = pDicomHeader -> ListOfDicomElements;
					pBufferListElement = pDicomHeader -> ListOfOutputBuffers;
					}

				if ( bNoError )
					{
					// Loop through the Dicom elements.
					while ( bNoError && pDicomDataListElement != 0 )
						{
						pDicomElement = (DICOM_ELEMENT*)pDicomDataListElement -> pItem;
						if ( pDicomElement != 0 )
							{
							// Do temporary edit to rename the NIOSH exam images.
				//			if ( pDicomElement -> Tag.Group == 0x0010 && pDicomElement -> Tag.Element == 0x0010 )
				//				{
				//				if ( strnicmp( pDicomElement -> Value.LT, "Exam Set 1 NIOSH", 16 ) == 0 )
				//					{
				//					pDicomElement -> Value.UN = realloc( pDicomElement -> Value.UN, 26 );
				//					strcpy( pDicomElement -> Value.LT, "NIOSH Recert Exam - 010 " );
				//					pDicomElement -> ValueLength = 24;
				//					}
				//				else if ( strnicmp( pDicomElement -> Value.LT, "Exam Set 38 NIOSH", 17 ) == 0 )
				//					{
				//					memcpy( pDicomElement -> Value.LT, "NIOSH Certif Exam", 17 );
				//					}
				//				}
			
							// Check each dicom element for possible edit specifications before packing into the buffer.
							pEditSpecificationListElement = pDicomHeader -> ListOfEditSpecifications;
							bEditSpecifiedForThisElement = FALSE;
							while ( pEditSpecificationListElement != 0 && !bEditSpecifiedForThisElement )
								{
								pEditSpecification = (EDIT_SPECIFICATION*)pEditSpecificationListElement -> pItem;
								if ( pEditSpecification -> DicomFieldIdentifier.Group == pDicomElement -> Tag.Group &&
											pEditSpecification -> DicomFieldIdentifier.Element == pDicomElement -> Tag.Element )
									{
									bEditSpecifiedForThisElement = TRUE;
									if ( pDicomElement -> Tag.Group == 0x7FE0 && pDicomElement -> Tag.Element == 0x0010 )
										{
										strcpy( EditedFieldValue, pEditSpecification -> EditedFieldValue );
										pEditFieldText = strtok( EditedFieldValue, " \n" );
										strcpy( OriginalImageSizeText, pEditFieldText );
										pDicomElement -> ValueLength = atol( OriginalImageSizeText );
										pEditFieldText = strtok( NULL, " \n" );
										strcpy( OverlayImageWidthText, pEditFieldText );
										nOverlayImageWidth = atol( OverlayImageWidthText );
										pEditFieldText = strtok( NULL, " \n" );
										strcpy( OverlayImageHeightText, pEditFieldText );
										nOverlayImageHeight = atol( OverlayImageHeightText );
										pEditFieldText = strtok( NULL, " \n" );
										strcpy( OverlayImageX0Text, pEditFieldText );
										nOverlayImageX0 = atol( OverlayImageX0Text );
										pEditFieldText = strtok( NULL, " \n" );
										strcpy( OverlayImageY0Text, pEditFieldText );
										nOverlayImageY0 = atol( OverlayImageY0Text );
										pEditFieldText = strtok( NULL, " \n" );
										strcpy( OverlayImageFileSizeText, pEditFieldText );
										nOverlayImageFileSize = atol( OverlayImageFileSizeText );
										pEditFieldText = strtok( NULL, " \n" );
										strcpy( OverlayImageFileSpec, pEditFieldText );
										pOverlayImageFile = fopen( OverlayImageFileSpec, "rb" );
										bCropImage = FALSE;
										if ( bCropImage )
											{
											nCroppedImageWidth = 1900;
											nCroppedImageHeight = 2048;
											nCroppedImageX0 = 148;
											nCroppedImageY0 = 0;
											bNoError = CropImage( pDicomHeader, nCroppedImageWidth, nCroppedImageHeight, nCroppedImageX0, nCroppedImageY0 );
											}
										else
											{
											if ( pOverlayImageFile != 0 )
												{
												pJPEGOverlayImageBuffer = (char*)malloc( nOverlayImageFileSize );
												if ( pJPEGOverlayImageBuffer != 0 )
													{
													nBytesRead = fread( pJPEGOverlayImageBuffer, 1, nOverlayImageFileSize, pOverlayImageFile );
													bNoError = ( nBytesRead == nOverlayImageFileSize );
													}
												fclose( pOverlayImageFile );
												if ( bNoError )
													{
													// Convert the JPEG overlay image to an uncompressed image.
													bNoError = Decompress8BitJpegImage( pJPEGOverlayImageBuffer, nOverlayImageFileSize,
														&ImageWidthInPixels, &ImageHeightInPixels,	&pDecompressedImageData, &DecompressedImageSizeInBytes );

													// Enscribe the overlay image over the original image at the specified coordinates.
													// (This assumes the Dicom image is uncompressed.
													if ( bNoError )
														{
														// For the specific case of labeling the standard reference images, calculate the overlay position
														// to be at the bottom center of the Dicom image:
														nOverlayImageX0 = ( (unsigned long)( *pDicomHeader -> ImageColumns ) - ImageWidthInPixels ) / 2;
														nOverlayImageY0 = (unsigned long)( *pDicomHeader -> ImageRows ) - ImageHeightInPixels;
														bNoError = EnscribeImageOverlay( pDicomHeader, pDecompressedImageData, ImageWidthInPixels,
																							ImageHeightInPixels, 1, nOverlayImageX0, nOverlayImageY0 );
														free( pDecompressedImageData );
														}
													free( pJPEGOverlayImageBuffer );
													}
												}
											}
										}
									else if ( pDicomElement -> ValueRepresentation == US )
										*pDicomElement -> Value.US = (unsigned short)atoi( pEditSpecification -> EditedFieldValue );
									else if ( pDicomElement -> ValueRepresentation == PN )
										{
										strcat( pEditSpecification -> EditedFieldValue, " " );
										pDicomElement -> Value.UN = realloc( pDicomElement -> Value.UN, strlen( pEditSpecification -> EditedFieldValue ) + 20 );
										if ( pDicomElement -> Value.UN == 0 )
											{
											bNoError = FALSE;
											RespondToError( MODULE_DICOM, DICOM_ERROR_ALLOCATE_VALUE );
											}
										strcpy( pDicomElement -> Value.LT, "^" );
										strcat( pDicomElement -> Value.LT, pEditSpecification -> EditedFieldValue );
										pDicomElement -> ValueLength = strlen( pDicomElement -> Value.LT );
										if ( ( pDicomElement -> ValueLength & 1 ) != 0 )
											pDicomElement -> ValueLength--;
										if ( pDicomElement -> pConvertedValue != 0 )
											{
											free( pDicomElement -> pConvertedValue );
											pDicomElement -> pConvertedValue = 0;
											}
										AllocateDicomPersonNameBuffer( pDicomElement );
										}
									else
										{
										pDicomElement -> ValueLength = (unsigned long)strlen( pEditSpecification -> EditedFieldValue );
										if ( ( pDicomElement -> ValueLength & 1 ) != 0 )
											{
											pDicomElement -> ValueLength++;
											strcat( pEditSpecification -> EditedFieldValue, " " );
											}
										pDicomElement -> Value.UN = realloc( pDicomElement -> Value.UN, strlen( pEditSpecification -> EditedFieldValue ) + 1 );
										pDicomElement -> pConvertedValue = (char*)realloc( (void*)pDicomElement -> pConvertedValue, strlen( pEditSpecification -> EditedFieldValue ) + 1 );
										if ( pDicomElement -> Value.UN == 0 || pDicomElement -> pConvertedValue == 0 )
											{
											bNoError = FALSE;
											RespondToError( MODULE_DICOM, DICOM_ERROR_ALLOCATE_VALUE );
											}
										else
											{
											// Copy the new value into the Dicom element structure and make sure the length is even.
											strcpy( pDicomElement -> Value.LT, pEditSpecification -> EditedFieldValue );
											strcpy( pDicomElement -> pConvertedValue, pEditSpecification -> EditedFieldValue );
											}
										}
									}
								pEditSpecificationListElement = pEditSpecificationListElement -> pNextListElement;
								}

							if ( pDicomElement -> Tag.Group == 0x0002 )
								CurrentEncodingType = EXPLICIT_VR | LITTLE_ENDIAN;	// Always the case for the file metadata.
							else
								CurrentEncodingType = pDicomHeader -> FileDecodingPlan.DataSetTransferSyntax;
							nBytesComposed = 0;
							bNoError = ComposeDicomElementForOutput( &pBufferListElement, pDicomElement, &nBytesComposed, CurrentEncodingType );
							if ( !bNoError )
								bDicomBuffersAllocatedOK = FALSE;
							// Compose the data element value.
							nBytesComposed = 0;
							if ( pDicomElement -> Tag.Group == 0x7fe0 && pDicomElement -> Tag.Element == 0x0010 )

								//zzz preserve format didn't set image buffer.

								bNoError = CopyBytesToBuffer( (char*)pDicomHeader -> pImageData, pDicomHeader -> ImageLengthInBytes,
																				(unsigned long*)&nBytesComposed, &pBufferListElement );
							else
								bNoError = ComposeDicomElementValueForOutput( &pBufferListElement, pDicomElement, &nBytesComposed );
							if ( bLogDicomElements )
								LogDicomElement( pDicomElement, 1 );
							}
						pDicomDataListElement = pDicomDataListElement -> pNextListElement;
						}
					}
				if ( bEditSpecsReadOK )
					DeallocateEditSpecifications( &pDicomHeader -> ListOfEditSpecifications );
				}
			}
		if ( bNoError )
			{
			pOutputFile = OpenDicomFileForOutput( DicomImageArchiveFileSpec );
			if ( pOutputFile != 0 )
				{
				ResetOutputBufferCursors( pExamInfo -> pDicomInfo );
				pBufferListElement = pExamInfo -> pDicomInfo -> ListOfOutputBuffers;
				while ( bNoError && pBufferListElement != 0 )
					{
					pDicomBuffer = (DICOM_DATA_BUFFER*)pBufferListElement -> pItem;
					if ( pDicomBuffer != 0 )
						{
						if ( pDicomBuffer -> pBuffer != 0 )
							{
							nBytesToBeWritten = pDicomBuffer -> DataSize;
							nBytesWritten = (long)fwrite( pDicomBuffer -> pBuffer, 1, (long)nBytesToBeWritten, pOutputFile );
							if ( nBytesWritten != nBytesToBeWritten )
								{
								bNoError = FALSE;
								RespondToError( MODULE_EXAM, DICOM_ERROR_DICOM_STORE_WRITE );
								}
							}
						}
					pBufferListElement = pBufferListElement -> pNextListElement;
					}
				fclose( pOutputFile );
				if ( bDicomBuffersAllocatedOK )
					DeallocateOutputBuffers( pDicomHeader );
				}
			else
				{
				bNoError = FALSE;
				RespondToError( MODULE_EXAM, DICOM_ERROR_DICOM_STORE_CREATE );
				}
			}
		if ( bNoError )
			{
			sprintf( Msg, "File:  saved as %s", DicomImageArchiveFileSpec );
			LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
			}
		else
			{
			sprintf( Msg, "Error saving file as %s", DicomImageArchiveFileSpec );
			LogMessage( Msg, MESSAGE_TYPE_ERROR );
			}
		}

	return bNoError;
}


// Compose the output buffer contents representing a single Dicom data element.
BOOL ComposeDicomElementForOutput( LIST_ELEMENT **ppBufferListElement, DICOM_ELEMENT *pDicomElement,
											size_t *pnBytesComposed, TRANSFER_SYNTAX TransferSyntax )
{
	BOOL					bNoError = TRUE;
	char					TextLine[ MAX_LOGGING_STRING_LENGTH ];
	char					ValueRepresentation[4];
	BOOL					bBreak = FALSE;
	unsigned long			nBytesCopied;
	unsigned short			TwoEmptyBytes = 0;

	SwapBytesFromFile( &pDicomElement -> Tag.Group, 2, TransferSyntax );
	SwapBytesFromFile( &pDicomElement -> Tag.Element, 2, TransferSyntax );
	// Write out the Group and Element Tag.
	bNoError = CopyBytesToBuffer( (char*)&pDicomElement -> Tag.Group, sizeof(unsigned short), &nBytesCopied, ppBufferListElement );
	*pnBytesComposed += sizeof(unsigned short);
	if ( bNoError )
		{
		bNoError = CopyBytesToBuffer( (char*)&pDicomElement -> Tag.Element, sizeof(unsigned short), &nBytesCopied, ppBufferListElement );
		*pnBytesComposed += sizeof(unsigned short);
		}
	// Write the value representation and the value length.
	//
	// If this Element's value representation is implicit (look it up in the dictionary) or if it is a delimiter...
	if ( ( TransferSyntax & IMPLICIT_VR ) || pDicomElement -> Tag.Group == GROUP_ITEM_DELIMITERS )
		{
		SwapBytesFromFile( &pDicomElement -> ValueLength, 4, TransferSyntax );
		bNoError = CopyBytesToBuffer( (char*)&pDicomElement -> ValueLength, sizeof(unsigned long), &nBytesCopied, ppBufferListElement );
		*pnBytesComposed += sizeof(unsigned long);
		}
	// Otherwise, if this Element's value representation is recorded explicitly...
	else
		{
		// This Element's value representation is recorded explicitly...
		ValueRepresentation[ 0 ] = (char)( ( (unsigned short)pDicomElement -> ValueRepresentation >> 8 ) & 0x00FF );
		ValueRepresentation[ 1 ] = (char)( ( (unsigned short)pDicomElement -> ValueRepresentation ) & 0x00FF );
		ValueRepresentation[ 2 ] = '\0';
		bNoError = CopyBytesToBuffer( (char*)ValueRepresentation, 2, &nBytesCopied, ppBufferListElement );
		*pnBytesComposed += 2;
		if ( bNoError )
			{
			// Write the value length.
			switch( pDicomElement -> ValueRepresentation )
				{
				case OB:
				case OW:
				case SQ:
				case UN:
				case UT:
					// Do a dummy write to add 2 bytes to the buffer.
					bNoError = CopyBytesToBuffer( (char*)&TwoEmptyBytes, 2, &nBytesCopied, ppBufferListElement );
					*pnBytesComposed += 2;
					if ( bNoError )
						{
						// Write the value length.
						bNoError = CopyBytesToBuffer( (char*)&pDicomElement -> ValueLength, sizeof(unsigned long), &nBytesCopied, ppBufferListElement );
						*pnBytesComposed += sizeof(unsigned long);
						SwapBytesFromFile( &pDicomElement -> ValueLength, 4, TransferSyntax );
						}
					break;
				default:
					// Write the value length.
					bNoError = CopyBytesToBuffer( (char*)&pDicomElement -> ValueLength, sizeof(unsigned short), &nBytesCopied, ppBufferListElement );
					*pnBytesComposed += sizeof(unsigned short);
					SwapBytesFromFile( &pDicomElement -> ValueLength, 2, TransferSyntax );
					break;
				}
			}
		}

	// Check for uneven value length.
	if ( ( pDicomElement -> ValueLength & 1 ) != 0 && pDicomElement -> ValueLength != VALUE_LENGTH_UNDEFINED )
		{
		RespondToError( MODULE_DICOM, DICOM_ERROR_UNEVEN_VALUE_LENGTH );
		sprintf( TextLine, "Dicom Element ( %X, %X )", pDicomElement -> Tag.Group, pDicomElement -> Tag.Element );
		LogMessage( TextLine, MESSAGE_TYPE_ERROR );
		bNoError = FALSE;
		}

	return bNoError;
}


BOOL ComposeDicomElementValueForOutput( LIST_ELEMENT **ppBufferListElement, DICOM_ELEMENT *pDicomElement, size_t *pnBytesComposed )
{
	BOOL					bNoError = TRUE;
	unsigned long			nBytesCopied;

	if ( pDicomElement -> ValueRepresentation != SQ &&
			pDicomElement -> ValueLength != VALUE_LENGTH_UNDEFINED &&
			!( pDicomElement -> Tag.Group == GROUP_ITEM_DELIMITERS ))
		{
		bNoError = CopyBytesToBuffer( (char*)pDicomElement -> Value.UN, pDicomElement -> ValueLength, &nBytesCopied, ppBufferListElement );
		*pnBytesComposed += pDicomElement -> ValueLength;
		}

	return bNoError;
}



