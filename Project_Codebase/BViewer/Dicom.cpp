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
#include "Dicom.h"
#include "Configuration.h"


extern unsigned short				GetTransferSyntaxIndex( char *pTransferSyntaxUID, unsigned short Length );

static TRANSFER_SYNTAX				LocalMemoryByteOrder;

static void							SwapBytesFromFile( void *pData, long nValueSize, TRANSFER_SYNTAX TransferSyntax );


//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		DicomModuleInfo = { MODULE_DICOM, "Dicom Module", InitDicomModule, CloseDicomModule };


static ERROR_DICTIONARY_ENTRY	DicomErrorCodes[] =
			{
				{ DICOM_ERROR_INSUFFICIENT_MEMORY	, "An error occurred allocating a memory block for data storage." },
				{ DICOM_ERROR_FILE_OPEN				, "An error occurred attempting to open a Dicom file." },
				{ DICOM_ERROR_FILE_READ				, "An error occurred attempting to read a Dicom file." },
				{ DICOM_ERROR_DICOM_SIGNATURE		, "The standard Dicom signature was not found after the Dicom preamble." },
				{ DICOM_ERROR_UNEVEN_VALUE_LENGTH	, "An uneven Dicom element value length was encountered in the Dicom file." },
				{ DICOM_ERROR_ALLOCATE_VALUE		, "A Dicom element value memory allocation failed." },
				{ DICOM_ERROR_TRANSFER_SYNTAX		, "No Dicom transfer syntax was specified in (0002, 0010)." },
				{ DICOM_ERROR_DICOM_OPEN_WRITE		, "An error occurred opening and initializing a Dicom file for writing." },
				{ 0									, NULL }
			};

static ERROR_DICTIONARY_MODULE		DicomStatusErrorDictionary =
										{
										MODULE_DICOM,
										DicomErrorCodes,
										DICOM_ERROR_DICT_LENGTH,
										0
										};

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
static DICOM_ELEMENT_SEMANTICS		SpecialDicomElementList[] =
	{
		{ 0x0002, 0x0000, 0,							DATA_LOADING_TYPE_INT,  offsetof( DICOM_HEADER_SUMMARY, MetadataGroupLength )			},
		{ 0x0002, 0x0001, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, FileMetaInformationVersion )	},
		{ 0x0002, 0x0002, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, MediaStorageSOPClassUID )		},
		{ 0x0002, 0x0003, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, MediaStorageSOPInstanceUID )	},
		{ 0x0002, 0x0010, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, TransferSyntaxUniqueIdentifier )},
		{ 0x0002, 0x0012, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, ImplementationClassUID )		},
		{ 0x0002, 0x0013, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, ImplementationVersionName )		},
		{ 0x0002, 0x0016, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, SourceApplicationName )			},

		{ 0x0008, 0x0008, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, ImageTypeUniqueIdentifier )		},
		{ 0x0008, 0x0012, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, DataCreationDate )				},
		{ 0x0008, 0x0013, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, DataCreationTime )				},
		{ 0x0008, 0x0014, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, DataCreatorUniqueIdentifier )	},
		{ 0x0008, 0x0016, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, SOPClassUniqueIdentifier )		},
		{ 0x0008, 0x0018, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, SOPInstanceUniqueIdentifier )	},
		{ 0x0008, 0x0020, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, StudyDate )						},
		{ 0x0008, 0x0021, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, SeriesDate )					},
		{ 0x0008, 0x0022, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, AcquisitionDate )				},
		{ 0x0008, 0x0030, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, StudyTime )						},
		{ 0x0008, 0x0031, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, SeriesTime )					},
		{ 0x0008, 0x0032, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, AcquisitionTime )				},
		{ 0x0008, 0x0050, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, AccessionNumber )				},
		{ 0x0008, 0x0060, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, Modality )						},
		{ 0x0008, 0x0080, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, InstitutionName )				},
		{ 0x0008, 0x0090, 0,							DATA_LOADING_TYPE_NAME, offsetof( DICOM_HEADER_SUMMARY, ReferringPhysician )			},
		{ 0x0008, 0x1030, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, StudyDescription )				},
		{ 0x0008, 0x103E, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, SeriesDescription )				},
		{ 0x0008, 0x1080, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, AdmittingDiagnosesDescription )	},

		{ 0x0010, 0x0010, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_NAME, offsetof( DICOM_HEADER_SUMMARY, PatientName )					},
		{ 0x0010, 0x0020, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, PatientID )						},
		{ 0x0010, 0x0030, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, PatientBirthDate )				},
		{ 0x0010, 0x0040, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, PatientSex )					},
		{ 0x0010, 0x1010, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, PatientAge )					},
		{ 0x0010, 0x21B0, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, AdditionalPatientHistory )		},
		{ 0x0010, 0x4000, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, PatientComments )				},

		{ 0x0018, 0x0015, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, BodyPartExamined )				},
		{ 0x0018, 0x1164, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, ImagerPixelSpacing )			},
		{ 0x0018, 0x5100, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, PatientPosition )				},
		{ 0x0018, 0x5101, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, ViewPosition )					},

		{ 0x0020, 0x000D, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, StudyInstanceUID )				},
		{ 0x0020, 0x000E, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, SeriesInstanceUID )				},
		{ 0x0020, 0x0010, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, StudyID )						},
		{ 0x0020, 0x0011, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, SeriesNumber )					},
		{ 0x0020, 0x0012, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, AcquisitionNumber )				},
		{ 0x0020, 0x0013, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, InstanceNumber )				},
		{ 0x0020, 0x1002, 0,							DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, ImagesInAcquisition )			},
		{ 0x0020, 0x4000, MESSAGE_TYPE_SUPPLEMENTARY,	DATA_LOADING_TYPE_TEXT, offsetof( DICOM_HEADER_SUMMARY, ImageComments )					},

		{ 0, 0, 0, 0 }
	};


BOOL IsSpecialDicomElement( TAG DicomElementTag, DICOM_ELEMENT_SEMANTICS **ppDicomElementInfo )
{
	BOOL						bFinished;
	BOOL						bIsSpecial;
	DICOM_ELEMENT_SEMANTICS		*pDicomElementInfo;
	int							nSpecialElement;

	bIsSpecial = FALSE;
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
	if ( IsSpecialDicomElement( pDicomElement -> Tag, &pDicomElementInfo ) )
		{
		if ( pDicomElementInfo != 0 && pDicomElementInfo -> DataStructureOffset >= 0 )
			{
			bIsLoadable = TRUE;
			DataLoadingType = pDicomElementInfo -> DataLoadingType;
			DataOffset = pDicomElementInfo -> DataStructureOffset;
			if ( ( DataLoadingType & ( DATA_LOADING_TYPE_TEXT | DATA_LOADING_TYPE_NAME | DATA_LOADING_TYPE_INT | DATA_LOADING_TYPE_FLOAT ) ) != 0 )
				{
				// Point to the location in the data structure.
				ppTextDataValue = (char**)( (char*)pDicomHeader + DataOffset );
				// If there is already a value for this entry, free the buffer:  it is about to be overwritten.
				if ( *ppTextDataValue != 0 )
					free( *ppTextDataValue );
				// Copy the allocated character string buffer pointer into the data structure.
				*ppTextDataValue = pDicomElement -> pConvertedValue;
				}
			}
		}
	// Process element (4,1430), if this is it.  Gather the info on a Dicom file set.
	GatherDicomDirectoryInfo( pDicomElement, pDicomHeader );

	return bIsLoadable;
}


void LogDicomElement( DICOM_ELEMENT *pDicomElement, long SequenceNestingLevel )
{
	char					TextLine[ MAX_LOGGING_STRING_LENGTH ];
	char					TextField[ MAX_LOGGING_STRING_LENGTH ];
	char					ValueRepresentation[ 20 ];
	unsigned int			nSpaceRemaining;
	unsigned int			nBytesToCopy;
	int						nCharOffset;
	int						nCharEOL;

	memset( TextLine, ' ', MAX_LOGGING_STRING_LENGTH );
	nSpaceRemaining = MAX_LOGGING_STRING_LENGTH - 1;
		
	nCharOffset = 4 + 4 * SequenceNestingLevel;
		
	ValueRepresentation[ 0 ] = (char)( ( (unsigned short)pDicomElement -> ValueRepresentation >> 8 ) & 0x00FF );
	ValueRepresentation[ 1 ] = (char)( ( (unsigned short)pDicomElement -> ValueRepresentation ) & 0x00FF );
	ValueRepresentation[ 2 ] = '\0';

	sprintf( TextField, "Dicom Element ( %X, %X )", pDicomElement -> Tag.Group, pDicomElement -> Tag.Element );
	nBytesToCopy = strlen( TextField );
	if ( nBytesToCopy > nSpaceRemaining )
		nBytesToCopy = nSpaceRemaining;
	if ( nCharOffset + nBytesToCopy < sizeof(TextLine) - 1 )
		memcpy( &TextLine[ nCharOffset ], TextField, nBytesToCopy );
	nSpaceRemaining -= nBytesToCopy;

	nCharOffset += 29;
	sprintf( TextField, " %2s  %d", ValueRepresentation, pDicomElement -> ValueLength );
	nBytesToCopy = strlen( TextField );
	if ( nBytesToCopy > nSpaceRemaining )
		nBytesToCopy = nSpaceRemaining;
	if ( nCharOffset + nBytesToCopy < sizeof(TextLine) - 1 )
		memcpy( &TextLine[ nCharOffset ], TextField, nBytesToCopy );
	nSpaceRemaining -= nBytesToCopy;

	nCharOffset += 15;
	if ( pDicomElement -> pMatchingDictionaryItem != 0 )
		sprintf_s( TextField, strlen(TextField), "%s:", pDicomElement -> pMatchingDictionaryItem -> Description );
	else
		TextField[ 0 ] = '\0';
	nBytesToCopy = strlen( TextField );
	if ( nBytesToCopy > nSpaceRemaining )
		nBytesToCopy = nSpaceRemaining;
	if ( nCharOffset + nBytesToCopy < sizeof(TextLine) - 1 )
		memcpy( &TextLine[ nCharOffset ], TextField, nBytesToCopy );
	nSpaceRemaining -= nBytesToCopy;
	nCharEOL = nCharOffset + (int)strlen( TextField );

	if ( pDicomElement -> pConvertedValue != 0 )
		{
		nCharOffset += 32;
		switch( pDicomElement -> ValueRepresentation )
			{
			case SS:			// Signed short.
				// Log these integer values with %d.
				sprintf( TextField, "%d", *( (short*)pDicomElement -> pConvertedValue)  );
				break;
			case US:			// Unsigned short.
			case AT:			// Attribute tag.
				// Log these integer values with %d.
				sprintf( TextField, "%d", *( (unsigned short*)pDicomElement -> pConvertedValue ) );
				break;
			case SL:			// Signed long.
			case UL:			// Unsigned long.
				// Log these integer values with %d.
				sprintf( TextField, "%d", *( (long*)pDicomElement -> pConvertedValue ) );
				break;
			case FL:			// Float (single precision).
				// Log these floating point values with %f.
				sprintf( TextField, "%10.3f", *( (float*)pDicomElement -> pConvertedValue ) );
				break;
			case FD:			// Float (double precision).
				// Log these floating point values with %f.
				sprintf( TextField, "%10.3f", *( (double*)pDicomElement -> pConvertedValue ) );
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
				strncpy_s( TextField, MAX_LOGGING_STRING_LENGTH, pDicomElement -> pConvertedValue, _TRUNCATE );
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
		nBytesToCopy = strlen( TextField );
		if ( nBytesToCopy > nSpaceRemaining )
			nBytesToCopy = nSpaceRemaining;
		if ( nCharOffset + strlen( TextField ) < sizeof(TextLine) - 1 )
			memcpy( &TextLine[ nCharOffset ], TextField, strlen( TextField ) );
		nSpaceRemaining -= nBytesToCopy;
		nCharEOL = nCharOffset + (int)strlen( TextField );
		}

	if ( nCharEOL < sizeof(TextLine) - 1 )
		TextLine[ nCharEOL ] = '\0';
	LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY | MESSAGE_TYPE_NO_TIME_STAMP );
	
	if ( !pDicomElement -> bRetainConvertedValue )
		{
		if ( pDicomElement -> pConvertedValue != 0 )
			free( pDicomElement -> pConvertedValue );
		pDicomElement -> pConvertedValue = 0;
		}
}


void GatherDicomDirectoryInfo( DICOM_ELEMENT *pDicomElement, DICOM_HEADER_SUMMARY *pDicomHeader )
{
	IMAGE_FILE_SET_SPECIFICATION	*pNewImageFileSetSpecification;
	IMAGE_FILE_SET_SPECIFICATION	*pImageFileSetSpecification;
	char							FileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char							*pChar;

	if ( pDicomElement -> Tag.Group == 0x0004 && pDicomElement -> Tag.Element == 0x1430 )
		{
		// Add a new fileset item link, with a defined node type.
		pNewImageFileSetSpecification = (IMAGE_FILE_SET_SPECIFICATION*)calloc( 1, sizeof(IMAGE_FILE_SET_SPECIFICATION) );
		if ( pNewImageFileSetSpecification != 0 )
			{
			if ( _stricmp( pDicomElement -> pConvertedValue, "PATIENT" ) == 0 )
				pNewImageFileSetSpecification -> DicomNodeType = DICOM_NODE_PATIENT;
			else if ( _stricmp( pDicomElement -> pConvertedValue, "STUDY" ) == 0 )
				pNewImageFileSetSpecification -> DicomNodeType = DICOM_NODE_STUDY;
			else if ( _stricmp( pDicomElement -> pConvertedValue, "SERIES" ) == 0 )
				pNewImageFileSetSpecification -> DicomNodeType = DICOM_NODE_SERIES;
			else if ( _stricmp( pDicomElement -> pConvertedValue, "IMAGE" ) == 0 )
				pNewImageFileSetSpecification -> DicomNodeType = DICOM_NODE_IMAGE;
			pNewImageFileSetSpecification -> pNextFileSetStruct = 0;
			pNewImageFileSetSpecification -> hTreeHandle = 0;
			strncpy_s( pNewImageFileSetSpecification -> NodeInformation, FULL_FILE_SPEC_STRING_LENGTH, "", _TRUNCATE );
			strncpy_s( pNewImageFileSetSpecification -> DICOMDIRFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "", _TRUNCATE );
			pImageFileSetSpecification = pDicomHeader -> ListOfImageFileSetSpecifications;
			if ( pImageFileSetSpecification == 0 )
				pDicomHeader -> ListOfImageFileSetSpecifications = pNewImageFileSetSpecification;
			else
				{
				while ( pImageFileSetSpecification -> pNextFileSetStruct != 0 )
					pImageFileSetSpecification = pImageFileSetSpecification -> pNextFileSetStruct;
				pImageFileSetSpecification -> pNextFileSetStruct = pNewImageFileSetSpecification;
				}
			pDicomHeader -> pCurrentImageFileSetSpecification = pNewImageFileSetSpecification;
			}
		}
	else if ( pDicomElement -> Tag.Group == 0x0010 && pDicomElement -> Tag.Element == 0x0010 )
		{
		if ( pDicomHeader -> pCurrentImageFileSetSpecification != 0 &&
						pDicomHeader -> pCurrentImageFileSetSpecification -> DicomNodeType == DICOM_NODE_PATIENT )
			{
			// Record the patient name.
			strncpy_s( pDicomHeader -> pCurrentImageFileSetSpecification -> NodeInformation, FULL_FILE_SPEC_STRING_LENGTH, pDicomElement -> pConvertedValue, _TRUNCATE );
			CopyDicomNameToString( pDicomHeader -> pCurrentImageFileSetSpecification -> NodeInformation,
										(PERSON_NAME*)pDicomElement -> pConvertedValue, FULL_FILE_SPEC_STRING_LENGTH );
			}
		}
	else if ( pDicomElement -> Tag.Group == 0x0008 && pDicomElement -> Tag.Element == 0x0020 )
		{
		if ( pDicomHeader -> pCurrentImageFileSetSpecification != 0 &&
						pDicomHeader -> pCurrentImageFileSetSpecification -> DicomNodeType == DICOM_NODE_STUDY )
			{
			// Record the study date.
			strncpy_s( pDicomHeader -> pCurrentImageFileSetSpecification -> NodeInformation, FULL_FILE_SPEC_STRING_LENGTH, pDicomElement -> pConvertedValue, _TRUNCATE );
			}
		}
	else if ( pDicomElement -> Tag.Group == 0x0008 && pDicomElement -> Tag.Element == 0x0060 )
		{
		if ( pDicomHeader -> pCurrentImageFileSetSpecification != 0 &&
						pDicomHeader -> pCurrentImageFileSetSpecification -> DicomNodeType == DICOM_NODE_SERIES )
			{
			// Record the modality.
			strncpy_s( pDicomHeader -> pCurrentImageFileSetSpecification -> NodeInformation, FULL_FILE_SPEC_STRING_LENGTH, pDicomElement -> pConvertedValue, _TRUNCATE );
			strncat_s( pDicomHeader -> pCurrentImageFileSetSpecification -> NodeInformation, FULL_FILE_SPEC_STRING_LENGTH, " -  ", _TRUNCATE );
			}
		}
	else if ( pDicomElement -> Tag.Group == 0x0008 && pDicomElement -> Tag.Element == 0x103E )
		{
		if ( pDicomHeader -> pCurrentImageFileSetSpecification != 0 &&
						pDicomHeader -> pCurrentImageFileSetSpecification -> DicomNodeType == DICOM_NODE_SERIES )
			{
			// Record the modality.
			strncat_s( pDicomHeader -> pCurrentImageFileSetSpecification -> NodeInformation, FULL_FILE_SPEC_STRING_LENGTH, pDicomElement -> pConvertedValue, _TRUNCATE );
			}
		}
	else if ( pDicomElement -> Tag.Group == 0x0004 && pDicomElement -> Tag.Element == 0x1500 )
		{
		if ( pDicomHeader -> pCurrentImageFileSetSpecification != 0 &&
						pDicomHeader -> pCurrentImageFileSetSpecification -> DicomNodeType == DICOM_NODE_IMAGE )
			{
			strncpy_s( FileSpec, FULL_FILE_SPEC_STRING_LENGTH, pDicomElement -> pConvertedValue, _TRUNCATE );
			do
				{
				pChar = strchr( FileSpec, '|' );
				if ( pChar != 0 )
					*pChar = '\\';
				}
			while ( pChar != 0 );
			strncpy_s( pDicomHeader -> pCurrentImageFileSetSpecification -> NodeInformation, FULL_FILE_SPEC_STRING_LENGTH, FileSpec, _TRUNCATE );
			}
		}
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


// If the end of an input buffer is reached, additional
// bytes are copied as needed from the next buffer in the list.
BOOL AdvanceBufferCursor( unsigned long nBytesNeeded, LIST_ELEMENT **ppBufferListElement )
{
	BOOL					bNoError = TRUE;
	DICOM_DATA_BUFFER		*pDicomBuffer;
	LIST_ELEMENT			*pBufferListElement;
	unsigned long			nBufferBytesProcessed;
	char					*pBufferReadPoint;

	pBufferListElement = *ppBufferListElement;
	if ( pBufferListElement == 0 )
		bNoError = FALSE;
	if ( bNoError )
		{
		pDicomBuffer = (DICOM_DATA_BUFFER*)pBufferListElement -> pItem;
		if ( pDicomBuffer == 0 )
			bNoError = FALSE;
		}
	if ( bNoError )
		{
		nBufferBytesProcessed = pDicomBuffer -> DataSize - pDicomBuffer -> BytesRemainingToBeProcessed;
		pBufferReadPoint = pDicomBuffer -> pBeginningOfDicomData + nBufferBytesProcessed;
		do
			{
			if ( pDicomBuffer -> BytesRemainingToBeProcessed >= nBytesNeeded )
				{
				pDicomBuffer -> BytesRemainingToBeProcessed -= nBytesNeeded;
				pBufferReadPoint += nBytesNeeded;
				nBytesNeeded = 0;
				}
			else
				{
				if ( pDicomBuffer -> BytesRemainingToBeProcessed > 0 )
					nBytesNeeded -= pDicomBuffer -> BytesRemainingToBeProcessed;
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
		}

	return bNoError;
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


BOOL ReadDicomHeaderInfo( char *DicomFileSpecification, DICOM_HEADER_SUMMARY **ppDicomHeader, BOOL bLogDicomElements )
{
	BOOL					bNoError = TRUE;
	FILE					*pDicomFile;
	DICOM_DATA_BUFFER		*pDicomBuffer;
	LIST_ELEMENT			*pBufferListElement;
	BOOL					bEndOfFile;
	unsigned long			nBytesRead;
	char					*pBuffer;
	int						SystemErrorNumber;
	DICOM_HEADER_SUMMARY	*pDicomHeader;

	pDicomFile = 0;
	pDicomHeader = (DICOM_HEADER_SUMMARY*)malloc( sizeof(DICOM_HEADER_SUMMARY) );
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
	bEndOfFile = FALSE;
	pDicomHeader -> ListOfInputBuffers = 0;
	while ( bNoError && !bEndOfFile )
		{
		pBuffer = (char*)malloc( MAX_DICOM_READ_BUFFER_SIZE );		// Allocate a 64K buffer.
		if ( pBuffer == 0 )
			{
			bNoError = FALSE;
			RespondToError( MODULE_DICOM, DICOM_ERROR_INSUFFICIENT_MEMORY );
			}
		if ( bNoError )
			{
			pDicomBuffer = (DICOM_DATA_BUFFER*)malloc( sizeof(DICOM_DATA_BUFFER) );
			if ( pDicomBuffer == 0 )
				{
				bNoError = FALSE;
				RespondToError( MODULE_DICOM, DICOM_ERROR_INSUFFICIENT_MEMORY );
				free( pBuffer );
				pBuffer = 0;
				}
			}
		if ( bNoError )
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
			nBytesRead = (unsigned long)fread( pBuffer, 1, MAX_DICOM_READ_BUFFER_SIZE, pDicomFile );
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
	if ( bNoError )
		{
		pBufferListElement = pDicomHeader -> ListOfInputBuffers;
		bNoError = ParseDicomGroup2Info( &pBufferListElement, pDicomHeader, bLogDicomElements );
		if ( bNoError )
			bNoError = ParseDicomHeaderInfo( &pBufferListElement, pDicomHeader, bLogDicomElements );
		}

	return bNoError;
}


BOOL ParseDicomGroup2Info( LIST_ELEMENT **ppBufferListElement,
							DICOM_HEADER_SUMMARY *pDicomHeader, BOOL bLogDicomElements )
{
	BOOL					bNoError = TRUE;
	unsigned short			LocalStorageExample = 0x1234;
	char					TextLine[ MAX_LOGGING_STRING_LENGTH ];
	BOOL					bMoreHeaderInfoRemains = TRUE;
	DICOM_ELEMENT			DicomElement;
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
		// Read all the info about the next Dicom element except the value.
		bNoError = ParseDicomElement( ppBufferListElement, &DicomElement, &nBytesParsed,
							CurrentTransferSyntax, SequenceNestingLevel, pDicomHeader );
		if ( DicomElement.Tag.Group > 0x0002 )
			bMoreDicomElementsRemainInSequence = FALSE;
		if ( bNoError && bMoreDicomElementsRemainInSequence )
			{
			if ( DicomElement.ValueRepresentation == OB || DicomElement.ValueRepresentation == OW || DicomElement.ValueRepresentation == UN )
				{
				if ( bLogDicomElements )
					LogDicomElement( &DicomElement, 0 );
				}
			if ( DicomElement.ValueLength != VALUE_LENGTH_UNDEFINED )
				{
				// For each individual element, decide whether the value needs to be read and,
				//  if so, specify what to do with it.
				bNoError = ReadDicomElementValue( &DicomElement, ppBufferListElement, &nBytesParsed, TextLine, pDicomHeader );
				if ( bLogDicomElements )
					LogDicomElement( &DicomElement, 0 );
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


BOOL ParseDicomHeaderInfo( LIST_ELEMENT **ppBufferListElement,
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
	if ( bNoError )
		{
		bTerminateSequence = FALSE;
		// Parse the main data set sequence.  Other sequencences may be embedded within it, and these will be
		// processed by recursive calls made from inside ProcessDicomElementSequence().
		bNoError = ProcessDicomElementSequence( ppBufferListElement, 0L, CurrentTransferSyntax, pDicomHeader, &SequenceNestingLevel,
													&bTerminateSequence, &nBytesParsed, bLogDicomElements );
		}
	if ( bNoError )
		{
		if ( pDicomHeader -> TransferSyntaxUniqueIdentifier != 0 && strlen( pDicomHeader -> TransferSyntaxUniqueIdentifier ) > 0 )
			pDicomHeader -> FileDecodingPlan.nTransferSyntaxIndex = GetTransferSyntaxIndex( pDicomHeader -> TransferSyntaxUniqueIdentifier,
													(unsigned short)strlen( pDicomHeader -> TransferSyntaxUniqueIdentifier ) );
		}

	return bNoError;
}


// This function is called recursively to process each encountered Dicom data element sequence.
BOOL ProcessDicomElementSequence( LIST_ELEMENT **ppBufferListElement, unsigned long nBytesContainedInSequence,
									TRANSFER_SYNTAX CurrentTransferSyntax, DICOM_HEADER_SUMMARY *pDicomHeader,
									long *pNestingLevel, BOOL *pbTerminateSequence, size_t *pnBytesParsed,
									BOOL bLogDicomElements )
{
	BOOL					bNoError = TRUE;
	unsigned short			LocalStorageExample = 0x1234;
	char					TextLine[ MAX_LOGGING_STRING_LENGTH ];
	BOOL					bMoreDicomElementsRemainInSequence = TRUE;
	DICOM_ELEMENT			DicomElement;
	BOOL					bSequenceLengthWasSpecified;
	char					*IndentationString = "";
	BOOL					bBreak = FALSE;
	size_t					nBytesParsed;
	LIST_ELEMENT			*pSavedBufferListElement;
	unsigned long			nSavedBufferBytesRemainingToBeProcessed;
	BOOL					bHasByteSequenceDataValue;

	bSequenceLengthWasSpecified = ( nBytesContainedInSequence != VALUE_LENGTH_UNDEFINED && nBytesContainedInSequence != 0L );
	nBytesParsed = 0;
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
			bNoError = ParseDicomElement( ppBufferListElement, &DicomElement, &nBytesParsed,
								CurrentTransferSyntax, (unsigned char)*pNestingLevel, pDicomHeader );
			if ( DicomElement.Tag.Group == 0xfffc && DicomElement.Tag.Element == 0xfffc )
				{
				if ( bLogDicomElements )
					LogDicomElement( &DicomElement, *pNestingLevel );
				bMoreDicomElementsRemainInSequence = FALSE;
				*pbTerminateSequence = TRUE;
				}
			}
		if ( bNoError && bMoreDicomElementsRemainInSequence )
			{
			if ( DicomElement.ValueRepresentation == SQ && DicomElement.ValueLength > 0 )
				{
				( *pNestingLevel )++;
				if ( bLogDicomElements )
					LogDicomElement( &DicomElement, *pNestingLevel );
				LoadDicomHeaderElement( &DicomElement, pDicomHeader );
				if (  DicomElement.ValueLength != 0 )
					{
					// Make a recursive call to this function to handle the embedded sequence.
					bNoError = ProcessDicomElementSequence( ppBufferListElement, DicomElement.ValueLength, CurrentTransferSyntax,
															pDicomHeader, pNestingLevel, pbTerminateSequence, &nBytesParsed, bLogDicomElements );
					}
				( *pNestingLevel )--;
				if ( bSequenceLengthWasSpecified && nBytesContainedInSequence - nBytesParsed <= 0 )
					bMoreDicomElementsRemainInSequence = FALSE;
				if ( *pbTerminateSequence )
					return bNoError;
				}
			else if ( DicomElement.Tag.Group == GROUP_ITEM_DELIMITERS )
				{
				if ( DicomElement.Tag.Element == ELEMENT_ITEM_INITIATOR )
					{
					if ( bLogDicomElements )
						LogDicomElement( &DicomElement, *pNestingLevel );
					if ( DicomElement.ValueLength != 0 )
						{
						// Make a recursive call to this function to handle the embedded sequence.
						( *pNestingLevel )++;
						bNoError = ProcessDicomElementSequence( ppBufferListElement, DicomElement.ValueLength,
																	CurrentTransferSyntax, pDicomHeader, pNestingLevel,
																	pbTerminateSequence, &nBytesParsed, bLogDicomElements );
						( *pNestingLevel )--;
						if ( bSequenceLengthWasSpecified && nBytesContainedInSequence - nBytesParsed <= 0 )
							bMoreDicomElementsRemainInSequence = FALSE;
						if ( *pbTerminateSequence )
							return bNoError;
						}
					}
				else if ( DicomElement.Tag.Element == ELEMENT_ITEM_TERMINATOR )
					{
					if ( bLogDicomElements )
						LogDicomElement( &DicomElement, *pNestingLevel );
					bMoreDicomElementsRemainInSequence = FALSE;
					}
				else if ( DicomElement.Tag.Element == ELEMENT_ITEM_SEQUENCE_TERMINATOR )
					{
					if ( bLogDicomElements )
						LogDicomElement( &DicomElement, *pNestingLevel );
					bMoreDicomElementsRemainInSequence = FALSE;
					}
				else
					{
					if ( bLogDicomElements )
						LogDicomElement( &DicomElement, *pNestingLevel );
					}
				}
			else if ( bSequenceLengthWasSpecified && nBytesContainedInSequence - nBytesParsed <= 0 )
				{
				if ( bLogDicomElements )
					LogDicomElement( &DicomElement, *pNestingLevel );
				bMoreDicomElementsRemainInSequence = FALSE;
				}
			else
				{
				bHasByteSequenceDataValue = ( DicomElement.ValueRepresentation == OB ||
												DicomElement.ValueRepresentation == OW || DicomElement.ValueRepresentation == UN );
				if ( bHasByteSequenceDataValue )
					{
					if ( bLogDicomElements )
						LogDicomElement( &DicomElement, *pNestingLevel );
					}
				if ( DicomElement.ValueLength != VALUE_LENGTH_UNDEFINED && DicomElement.ValueLength != 0 &&
							 ( DicomElement.Tag.Group != 0x0004 || DicomElement.Tag.Element != 0x0000 ) )	// Ignore embedded file set elements.
					{
					// For each individual element, decide whether the value needs to be read and,
					//  if so, specify what to do with it.
					bNoError = ReadDicomElementValue( &DicomElement, ppBufferListElement, &nBytesParsed, TextLine, pDicomHeader );
					if ( bLogDicomElements && !bHasByteSequenceDataValue )
						LogDicomElement( &DicomElement, *pNestingLevel );
					if ( bSequenceLengthWasSpecified && nBytesContainedInSequence - nBytesParsed <= 0 )
						bMoreDicomElementsRemainInSequence = FALSE;
					}
				else
					{
					if ( bLogDicomElements && !bHasByteSequenceDataValue )
						LogDicomElement( &DicomElement, *pNestingLevel );
					}
				}
			}
		}
	*pnBytesParsed += nBytesParsed;

	return bNoError;
}


BOOL ReadDicomElementValue( DICOM_ELEMENT *pDicomElement, LIST_ELEMENT **ppBufferListElement, size_t *pnBytesParsed, char *TextLine,
								DICOM_HEADER_SUMMARY *pDicomHeader )
{
	BOOL					bNoError = TRUE;
	BOOL					bRetainConvertedValueBuffer;
	TRANSFER_SYNTAX			CurrentTransferSyntax;

	bNoError = ParseDicomElementValue( ppBufferListElement, pDicomElement, pnBytesParsed, pDicomElement -> ValueRepresentation, TextLine );
	if ( pDicomElement -> ValueRepresentation == PN )
		AllocateDicomPersonNameBuffer( pDicomElement );
	else
		AllocateDicomValueBuffer( pDicomElement );
	// Handle special cases.  Either reference the value from the DICOM_HEADER_SUMMARY or free it after printing.
	bRetainConvertedValueBuffer = LoadDicomHeaderElement( pDicomElement, pDicomHeader );
	if ( pDicomElement -> Tag.Group == 0x0002 && pDicomElement -> Tag.Element == 0x0010 )
		{
		CurrentTransferSyntax = InterpretUniqueTransferSyntaxIdentifier( pDicomHeader -> TransferSyntaxUniqueIdentifier );
		pDicomHeader -> FileDecodingPlan.ImageDataTransferSyntax = CurrentTransferSyntax & 0xFF00;
		}

	pDicomElement -> bRetainConvertedValue = bRetainConvertedValueBuffer;

	return bNoError;
}


char *AllocateDicomValueBuffer( DICOM_ELEMENT *pDicomElement )
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
	if ( pValue != 0 )
		free( pValue );
	pDicomElement -> Value.LT = 0;
	
	return pValueBuffer;
}


PERSON_NAME *AllocateDicomPersonNameBuffer( DICOM_ELEMENT *pDicomElement )
{
	BOOL			bNoError = TRUE;
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
			for ( nChar = 0; nChar < nValueSizeInBytes; nChar++ )
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
							if ( pNameBuffer -> pLastName == 0 )
								bNoError = FALSE;
							break;
						case 1:
							pNameBuffer -> pFirstName = (char*)malloc( nNameLength[ 1 ] + 1 );
							if ( pNameBuffer -> pFirstName == 0 )
								bNoError = FALSE;
							break;
						case 2:
							pNameBuffer -> pMiddleName = (char*)malloc( nNameLength[ 2 ] + 1 );
							if ( pNameBuffer -> pMiddleName == 0 )
								bNoError = FALSE;
							break;
						case 3:
							pNameBuffer -> pPrefix = (char*)malloc( nNameLength[ 3 ] + 1 );
							if ( pNameBuffer -> pPrefix == 0 )
								bNoError = FALSE;
							break;
						case 4:
							pNameBuffer -> pSuffix = (char*)malloc( nNameLength[ 4 ] + 1 );
							if ( pNameBuffer -> pSuffix == 0 )
								bNoError = FALSE;
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
				else if ( bNoError )
					switch ( nDelimiter )
						{
						case 0:
							pNameBuffer -> pLastName[ nChar - nBaseChar ] = pValue[ nChar ];
							break;
						case 1:
							pNameBuffer -> pFirstName[ nChar - nBaseChar ] = pValue[ nChar ];
							break;
						case 2:
							pNameBuffer -> pMiddleName[ nChar - nBaseChar ] = pValue[ nChar ];
							break;
						case 3:
							pNameBuffer -> pPrefix[ nChar - nBaseChar ] = pValue[ nChar ];
							break;
						case 4:
							pNameBuffer -> pSuffix[ nChar - nBaseChar ] = pValue[ nChar ];
							break;
						default:
							break;
						}
				}
			for ( nNamePart = 0; nNamePart < 5; nNamePart++ )
				if ( bNoError && nNameLength[ nNamePart ] > 0 )
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
			free( pValue );
			pValue = 0;
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

	nUnusedBytes = nTextStringSize - 1;			// Allow for null terminator character.
	if ( pName != 0 )
		{
		if ( pName -> pPrefix != 0 && nUnusedBytes >= (long)strlen( pName -> pPrefix ) + 1 )
			{
			strncat( TextString, pName -> pPrefix, nUnusedBytes - 1 );
			strncat( TextString, " ", 1 );
			nUnusedBytes = nTextStringSize - (long)strlen( TextString );
			}
		if ( pName -> pFirstName != 0 && nUnusedBytes >= (long)strlen( pName ->  pFirstName ) + 1 )
			{
			strncat( TextString, pName -> pFirstName, nUnusedBytes - 1 );
			strncat( TextString, " ", 1 );
			nUnusedBytes = nTextStringSize - (long)strlen( TextString );
			}
		if ( pName -> pMiddleName != 0 && nUnusedBytes >= (long)strlen( pName -> pMiddleName ) + 1 )
			{
			strncat( TextString, pName -> pMiddleName, nUnusedBytes - 1 );
			strncat( TextString, " ", 1 );
			nUnusedBytes = nTextStringSize - (long)strlen( TextString );
			}
		if ( pName -> pLastName != 0 && nUnusedBytes >= (long)strlen( pName -> pLastName ) + 1 )
			{
			strncat( TextString, pName -> pLastName, nUnusedBytes - 1 );
			strncat( TextString, " ", 1 );
			nUnusedBytes = nTextStringSize - (long)strlen( TextString );
			}
		if ( pName -> pSuffix != 0 && nUnusedBytes >= (long)strlen( pName -> pSuffix ) + 1 )
			{
			strncat( TextString, pName -> pSuffix, nUnusedBytes - 1 );
			strncat( TextString, " ", 1 );
			}
		// Eliminate the trailing blank.
		TextString[ strlen( TextString ) - 1 ] = '\0';
		}
}


void InitDicomHeaderSummary( DICOM_HEADER_SUMMARY *pDicomHeader )
{
	memset( (char*)pDicomHeader, '\0', sizeof(DICOM_HEADER_SUMMARY) );
	pDicomHeader -> bFileMetadataHasBeenRead = FALSE;
	pDicomHeader -> FileDecodingPlan.bTrustSpecifiedTransferSyntaxFromLocalStorage = FALSE;
	pDicomHeader -> FileDecodingPlan.bTrustSpecifiedTransferSyntaxFromNetwork = FALSE;
	pDicomHeader -> FileDecodingPlan.FileMetadataTransferSyntax = EXPLICIT_VR | LITTLE_ENDIAN;	// Always.
}


FILE *OpenDicomFileForOutput( char *DicomFileSpecification )
{
	FILE			*pDicomFile;
	FILE_STATUS		FileStatus = FILE_STATUS_OK;
	char			TextLine[ 1096 ];
	char			WriteBuffer[ 132 ];
	long			nBytesWritten;

	sprintf_s( TextLine, strlen(TextLine), "Open Dicom file for output:  %s", DicomFileSpecification );
	LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
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
		
	return pDicomFile;
}


FILE *OpenDicomFile( char *DicomFileSpecification )
{
	FILE			*pDicomFile;
	FILE_STATUS		FileStatus = FILE_STATUS_OK;
	char			TextLine[ 1096 ];
	char			ReadBuffer[ 512 ];
	DWORD			SystemErrorCode;

	sprintf_s( TextLine, sizeof(TextLine), "Open Dicom File:  %s", DicomFileSpecification );
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
			bHasRecognizedVR = TRUE;
			break;
		default:
			bHasRecognizedVR = FALSE;
			break;
		}

	return bHasRecognizedVR;
}


// Read all the info about the next Dicom element except the value.
BOOL ParseDicomElement( LIST_ELEMENT **ppBufferListElement, DICOM_ELEMENT *pDicomElement, size_t *pnBytesParsed,
								TRANSFER_SYNTAX TransferSyntax, unsigned char SequenceNestingLevel, DICOM_HEADER_SUMMARY *pDicomHeader )
{
	BOOL					bNoError = TRUE;
	char					TextLine[ MAX_LOGGING_STRING_LENGTH ];
	DICOM_DICTIONARY_ITEM	*pDictItem = 0;
	char					ValueRepresentation[2];
	BOOL					bBreak = FALSE;
	int						TempVR;

	pDicomElement -> ValueLength = 0L;
	pDicomElement -> pConvertedValue = 0;
	// Read the next Group and Element Tag.
	bNoError = CopyBytesFromBuffer( (char*)&pDicomElement -> Tag.Group, sizeof(unsigned short), ppBufferListElement );
	*pnBytesParsed += sizeof(unsigned short);
	if ( bNoError )
		{
		bNoError = CopyBytesFromBuffer( (char*)&pDicomElement -> Tag.Element, sizeof(unsigned short), ppBufferListElement );
		*pnBytesParsed += sizeof(unsigned short);
		}
	if ( bNoError )
		{	
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
		}
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
//		*((int*)( &pDicomElement -> ValueRepresentation )) =
//								(int)( ValueRepresentation[0] << 8 ) | (int)ValueRepresentation[1];
		TempVR = (int)( ValueRepresentation[0] << 8 ) | (int)ValueRepresentation[1];
		memcpy( &pDicomElement -> ValueRepresentation, &TempVR, 2 );
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
	// Check for uneven value length.
	if ( ( pDicomElement -> ValueLength & 1 ) != 0 && pDicomElement -> ValueLength != VALUE_LENGTH_UNDEFINED )
		{
		RespondToError( MODULE_DICOM, DICOM_ERROR_UNEVEN_VALUE_LENGTH );
		sprintf( TextLine, "Dicom Element ( %X, %X )", pDicomElement -> Tag.Group, pDicomElement -> Tag.Element );
		LogMessage( TextLine, MESSAGE_TYPE_ERROR );
		bNoError = FALSE;
		}
	pDicomElement -> SequenceNestingLevel = SequenceNestingLevel;

	return bNoError;
}


BOOL ParseDicomElementValue( LIST_ELEMENT **ppBufferListElement, DICOM_ELEMENT *pDicomElement, size_t *pnBytesParsed,
													VR ValueRepresentationOverride, char *pTextLine )
{
	BOOL					bNoError = TRUE;
	long					CopyLength;
	BOOL					bBreak = FALSE;

	// If the VR is unknown, use the override value.
	if ( pDicomElement -> ValueRepresentation == UN )
		pDicomElement -> ValueRepresentation = ValueRepresentationOverride;

	if ( pDicomElement -> ValueRepresentation != SQ &&
			pDicomElement -> ValueLength != VALUE_LENGTH_UNDEFINED &&
			!( pDicomElement -> Tag.Group == GROUP_ITEM_DELIMITERS ))
		{
		if ( pDicomElement -> ValueLength == 0 )
			pDicomElement -> Value.UN = 0L;
		else
			{
			// Allocate and clear a value buffer.
			pDicomElement -> Value.UN = calloc( 1, pDicomElement -> ValueLength );
			if ( pDicomElement -> Value.UN != 0 )
				{
				bNoError = CopyBytesFromBuffer( (char*)pDicomElement -> Value.UN, pDicomElement -> ValueLength, ppBufferListElement );
				*pnBytesParsed += pDicomElement -> ValueLength;
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
						strncpy_s( pTextLine, MAX_LOGGING_STRING_LENGTH, "", _TRUNCATE );
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
		pDicomElement -> Value.UN = 0L;
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
	if ( pDicomHeader -> SourceApplicationName != 0 )
		{
		free( pDicomHeader -> SourceApplicationName );
		pDicomHeader -> SourceApplicationName = 0;
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

	RealTransferSyntax = DeclaredTransferSyntax;		// Default.
	// Check if the first element is readable with the currently declared transfer syntax.  Since some applications
	// either don't follow the Dicom standard's formatting rules, or else they announce a different presentation
	// context than the one they are actually using, just rely on testing the data to determine what the actual
	// formatting is.  (The Dicom "standard" is not very standardized in its application.  Ease of implementation
	// does not appear to have been a consideration in its design.)
	DicomElement.Tag.Group = *((unsigned short*)pBufferReadPoint );
	DicomElement.Tag.Element = *((unsigned short*)pBufferReadPoint + 1 );
	pDictItem = GetDicomElementFromDictionary( DicomElement.Tag );
	memcpy( ExternalValueRepresentation, pBufferReadPoint + 4, 2 );
	*((unsigned long*)( &InternalValueRepresentation )) = 0L;			// Clear the stack variable.
	*((unsigned short*)( &InternalValueRepresentation )) = (unsigned short)( ExternalValueRepresentation[ 0 ] << 8 );
	*((unsigned short*)( &InternalValueRepresentation )) |= (unsigned short)( ExternalValueRepresentation[ 1 ] );
	if ( InternalValueRepresentation != pDictItem -> ValueRepresentation &&
				pDictItem -> ValueRepresentation != UN && ( DeclaredTransferSyntax & EXPLICIT_VR ) != 0 )
		{
		RealTransferSyntax &= ~EXPLICIT_VR;
		RealTransferSyntax |= IMPLICIT_VR;
		}
	else if ( ( InternalValueRepresentation == pDictItem -> ValueRepresentation ||
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


static TRANSFER_SYNTAX_TABLE_ENTRY		TransferSyntaxLookupTable[ NUMBER_OF_TRANSFER_SYNTAX_IDS ] =
	{
	{ 0x00000001, "1.2.840.10008.1.2" },
	{ 0x00000002, "1.2.840.10008.1.2.1" },
	{ 0x00000004, "1.2.840.10008.1.2.2" },
	{ 0x00000008, "1.2.840.10008.1.2.4.50" },
	{ 0x00000010, "1.2.840.10008.1.2.4.51" },
	{ 0x00000020, "1.2.840.10008.1.2.4.52" },
	{ 0x00000040, "1.2.840.10008.1.2.4.53" },
	{ 0x00000080, "1.2.840.10008.1.2.4.54" },
	{ 0x00000100, "1.2.840.10008.1.2.4.55" },
	{ 0x00000200, "1.2.840.10008.1.2.4.56" },
	{ 0x00000400, "1.2.840.10008.1.2.4.57" },
	{ 0x00000800, "1.2.840.10008.1.2.4.58" },
	{ 0x00001000, "1.2.840.10008.1.2.4.59" },
	{ 0x00002000, "1.2.840.10008.1.2.4.60" },
	{ 0x00004000, "1.2.840.10008.1.2.4.61" },
	{ 0x00008000, "1.2.840.10008.1.2.4.62" },
	{ 0x00010000, "1.2.840.10008.1.2.4.63" },
	{ 0x00020000, "1.2.840.10008.1.2.4.64" },
	{ 0x00040000, "1.2.840.10008.1.2.4.65" },
	{ 0x00080000, "1.2.840.10008.1.2.4.66" },
	{ 0x00100000, "1.2.840.10008.1.2.4.70" },
	{ 0x00200000, "1.2.840.10008.1.2.4.80" },
	{ 0x00400000, "1.2.840.10008.1.2.4.81" },
	{ 0x00800000, "1.2.840.10008.1.2.5" },
	{ 0x01000000, "1.2.840.10008.1.2.1.99" },
	{ 0x02000000, "1.2.840.10008.1.2.4.90" },
	{ 0x04000000, "1.2.840.10008.1.2.4.91" },
	{ 0x08000000, "1.2.840.10008.1.2.4.100 " },
	{ 0x10000000, "1.2.840.10008.1.2.4.92" },
	{ 0x20000000, "1.2.840.10008.1.2.4.93" }
	};


unsigned short GetTransferSyntaxIndex( char *pTransferSyntaxUID, unsigned short Length )
{
	BOOL				bNoError = TRUE;
	int					nTransferSyntaxIndex;
	BOOL				bTransferSyntaxFound;
	unsigned long		nChars;
	char				TransferSyntaxUIDString[ 256 ];

	memcpy( TransferSyntaxUIDString,  pTransferSyntaxUID, Length );
	TransferSyntaxUIDString[ Length ] = '\0';
	TrimTrailingSpaces( TransferSyntaxUIDString );
	nChars = (unsigned long)strlen( TransferSyntaxUIDString );
	nTransferSyntaxIndex = 0;
	bTransferSyntaxFound = FALSE;
	while ( !bTransferSyntaxFound && nTransferSyntaxIndex < NUMBER_OF_TRANSFER_SYNTAX_IDS )
		{
		if ( strncmp( TransferSyntaxUIDString, TransferSyntaxLookupTable[ nTransferSyntaxIndex ].pUIDString, nChars ) == 0 )
			bTransferSyntaxFound = TRUE;
		else
			nTransferSyntaxIndex++;
		}
	if ( !bTransferSyntaxFound )
		nTransferSyntaxIndex = NUMBER_OF_TRANSFER_SYNTAX_IDS;	// Set to an impossibly large number.

	return (unsigned short)nTransferSyntaxIndex;
}



