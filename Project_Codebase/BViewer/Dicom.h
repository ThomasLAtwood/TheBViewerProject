// Dicom.h : Defines the data structures and functions related to
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
#pragma once


#include "DicomDictionary.h"
#include "Calibration.h"


#define DICOM_ERROR_INSUFFICIENT_MEMORY		1
#define DICOM_ERROR_FILE_OPEN				2
#define DICOM_ERROR_FILE_READ				3
#define DICOM_ERROR_DICOM_SIGNATURE			4
#define DICOM_ERROR_UNEVEN_VALUE_LENGTH		5
#define DICOM_ERROR_ALLOCATE_VALUE			6
#define DICOM_ERROR_TRANSFER_SYNTAX			7
#define DICOM_ERROR_DICOM_OPEN_WRITE		8

#define DICOM_ERROR_DICT_LENGTH				8


#define GROUP_ITEM_DELIMITERS				0xFFFE

#define ELEMENT_ITEM_INITIATOR				0xE000
#define ELEMENT_ITEM_TERMINATOR				0xE00D
#define ELEMENT_ITEM_SEQUENCE_TERMINATOR	0xE0DD

#define	VALUE_LENGTH_UNDEFINED				0xFFFFFFFF

#define MAX_DICOM_READ_BUFFER_SIZE			0x10000


#define LITTLE_ENDIAN_IMPLICIT_TRANSFER_SYNTAX											0	// Default Transfer Syntax for Dicom.
#define LITTLE_ENDIAN_EXPLICIT_TRANSFER_SYNTAX											1
#define BIG_ENDIAN_EXPLICIT_TRANSFER_SYNTAX												2
#define JPEG_PROCESS_1_TRANSFER_SYNTAX													3	// JPEG Baseline (Process 1): Default for Lossy JPEG 8-bit compression.
#define JPEG_PROCESS_2_4_TRANSFER_SYNTAX												4	// JPEG Extended (Process 2 & 4): Default for Lossy JPEG 12-bit
																							//						compression (Process 4 only).
#define JPEG_PROCESS_3_5_TRANSFER_SYNTAX												5	// JPEG Extended (Process 3 & 5).
#define JPEG_PROCESS_6_8_TRANSFER_SYNTAX												6	// JPEG Spectral Selection, Non-Hierarchical (Process 6 & 8).
#define JPEG_PROCESS_7_9_TRANSFER_SYNTAX												7	// JPEG Spectral Selection, Non-Hierarchical (Process 7 & 9).
#define JPEG_PROCESS_10_12_TRANSFER_SYNTAX												8	// JPEG Full Progression, Non-Hierarchical (Process 10 & 12).
#define JPEG_PROCESS_11_13_TRANSFER_SYNTAX												9	// JPEG Full Progression, Non-Hierarchical (Process 11 & 13).
#define JPEG_PROCESS_14_TRANSFER_SYNTAX													10	// JPEG Lossless, Non-Hierarchical (Process 14).
#define JPEG_PROCESS_15_TRANSFER_SYNTAX													11	// JPEG Lossless, Non-Hierarchical (Process 15).
#define JPEG_PROCESS_16_18_TRANSFER_SYNTAX												12	// JPEG Extended, Hierarchical (Process 16 & 18).
#define JPEG_PROCESS_17_19_TRANSFER_SYNTAX												13	// JPEG Extended, Hierarchical (Process 17 & 19).
#define JPEG_PROCESS_20_22_TRANSFER_SYNTAX												14	// JPEG Spectral Selection, Hierarchical (Process 20 & 22).
#define JPEG_PROCESS_21_23_TRANSFER_SYNTAX												15	// JPEG Spectral Selection, Hierarchical (Process 21 & 23).
#define JPEG_PROCESS_24_26_TRANSFER_SYNTAX												16	// JPEG Full Progression, Hierarchical (Process 24 & 26).
#define JPEG_PROCESS_25_27_TRANSFER_SYNTAX												17	// JPEG Full Progression, Hierarchical (Process 25 & 27).
#define JPEG_PROCESS_28_TRANSFER_SYNTAX													18	// JPEG Lossless, Hierarchical (Process 28).
#define JPEG_PROCESS_29_TRANSFER_SYNTAX													19	// JPEG Lossless, Hierarchical (Process 29).
#define JPEG_PROCESS_14SV1_TRANSFER_SYNTAX												20	// JPEG Lossless, Non-Hierarchical, First-Order Prediction
																							//	(Process 14 [Selection Value 1]):  Default for Lossless
																							//	JPEG compression.
#define JPEGLS_LOSSLESS_TRANSFER_SYNTAX													21
#define JPEGLS_LOSSY_TRANSFER_SYNTAX													22	// JPEG-LS Lossy (Near-Lossless) compression.
#define RLE_LOSSLESS_TRANSFER_SYNTAX													23
#define DEFLATED_EXPLICIT_VR_LITTLE_ENDIAN_TRANSFER_SYNTAX								24
#define JPEG2000_LOSSLESS_ONLY_TRANSFER_SYNTAX											25
#define JPEG2000_TRANSFER_SYNTAX														26	// JPEG 2000 compression (lossless or lossy).
#define MPEG2_MAIN_PROFILE_AT_MAIN_LEVEL_TRANSFER_SYNTAX								27
#define JPEG2000_PART2_MULTICOMPONENT_IMAGE_COMPRESSION_LOSSLESS_ONLY_TRANSFER_SYNTAX	28
#define JPEG2000_PART2_MULTICOMPONENT_IMAGE_COMPRESSION_TRANSFER_SYNTAX					29	// JPEG 2000 Part 2 Multi-component compression (lossless or lossy).

#define NUMBER_OF_TRANSFER_SYNTAX_IDS													30


typedef struct
	{
	char				*pBuffer;
	char				*pBeginningOfDicomData;
	unsigned long		BufferSize;
	unsigned long		DataSize;
	unsigned long		BytesRemainingToBeProcessed;
	} DICOM_DATA_BUFFER;


// This data element structure represents both the external and the internal
// representation of a Dicom data element.  Each element is linked to a list
// of data elements representing the information content of a Dicom file.
typedef struct
	{
	TAG						Tag;
	VR						ValueRepresentation;
	unsigned long			ValueLength;
	union
		{
		TAG						*AT;
		double					*FD;
		float					*FL;
		unsigned long			*UL;
		long					*SL;
		unsigned short			*OW;
		unsigned short			*US;
		short					*SS;
		unsigned char			*OB;
		char					**AE;
		char					**AS;
		char					**CS;
		char					**DA;
		char					**DS;
		char					**DT;
		char					**IS;
		char					**LO;
		char					*LT;
		char					**PN;
		char					**SH;
		char					*ST;
		char					**TM;
		char					**UI;
		char					*UT;
		void					*SQ;
		void					*UN;
		}					Value;
	char					*pConvertedValue;
	BOOL					bRetainConvertedValue;
	DICOM_DICTIONARY_ITEM	*pMatchingDictionaryItem;	// Set to zero if the transfer syntax is explicit.
	unsigned long			ValueMultiplicity;
	unsigned char			SequenceNestingLevel;		// Zero, unless the element is part of a nested sequence.
	TAG	 					ParentSequenceTag;
	} DICOM_ELEMENT;


typedef unsigned short TRANSFER_SYNTAX;

// The following describe the byte ordering in integers, etc.
#define LITTLE_ENDIAN					0x0001
#define BIG_ENDIAN						0x0002
// The following indicate whether the Value Representation (VR) is included with each
// Dicom element in the data set.
#define IMPLICIT_VR						0x0010		// The VR is obtained from the Dicom Dictionary.
#define	EXPLICIT_VR						0x0020		// The VR is encoded as part of the Dicom data element.
// The following describe the encoding of the encapsulated image data.
#define UNCOMPRESSED					0x0100
#define COMPRESSED_UNKNOWN				0x0200
#define COMPRESSED_LOSSLESS				0x0400
#define COMPRESSED_LOSSY				0x0800
#define COMPRESSED_RUN_LENGTH_ENCODED	0x1000


typedef struct
	{
	// If the boolean members are TRUE, the transfer syntax is obtained from the file
	// metadata or, in the case of network transfers, from the accepted presentation
	// context.
	//
	// In cases where this information is not reliable, (which, given the absurd
	// complexity of the Dicom standard for network transfers, occasionaly happens) the
	// boolean member can be set to FALSE.  In this case, the transfer syntax is 
	// deduced from the actual file data, as follows:  If VR is present, the syntax is
	// EXPLICIT_VR, otherwise IMPLICIT_VR.  Almost every computer is LITTLE_ENDIAN,
	// anyway.  And the size of the encapsulated data, if uncompressed, will equal the
	// product of the raster length in bytes times the number of rasters, assuming this
	// information is included in the file and is valid.  If compressed, it is probably
	// JPEG, and the JPEG library will figure out which variety.
	BOOL				bTrustSpecifiedTransferSyntaxFromLocalStorage;
	BOOL				bTrustSpecifiedTransferSyntaxFromNetwork;
	// The following transfer syntaxes are set according to the boolean members above:
	// The file is assumed to be formatted in the following transfer syntaxes.
	// According to the Dicom standard, the group 2 data is always EXPLICIT_VR | LITTLE_ENDIAN:
	TRANSFER_SYNTAX		FileMetadataTransferSyntax;		// Group 2 data, if any.
	TRANSFER_SYNTAX		DataSetTransferSyntax;			// All other non-image data.
	TRANSFER_SYNTAX		ImageDataTransferSyntax;		// Image data.
	unsigned short		nTransferSyntaxIndex;
	} DECODING_PLAN;


typedef struct
	{
	char		*pPrefix;
	char		*pFirstName;
	char		*pMiddleName;
	char		*pLastName;
	char		*pSuffix;
	} PERSON_NAME;


typedef struct _ImageFileSetSpecification
	{
	unsigned short						DicomNodeType;
											#define	DICOM_NODE_PATIENT		0x0001		// NodeInformation is the patient name.
											#define	DICOM_NODE_STUDY		0x0002		// NodeInformation is the study date.
											#define	DICOM_NODE_SERIES		0x0004		// NodeInformation is the modality and description.
											#define	DICOM_NODE_IMAGE		0x0008		// NodeInformation is the image file specification.
	char								NodeInformation[ FULL_FILE_SPEC_STRING_LENGTH ];
	char								DICOMDIRFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	INT_PTR								hTreeHandle;
	struct _ImageFileSetSpecification	*pNextFileSetStruct;
	} IMAGE_FILE_SET_SPECIFICATION;


// Selected information of potential semantic significance, extracted from the Dicom file.
//
typedef struct
	{
	BOOL						bFileMetadataHasBeenRead;
	unsigned long				*MetadataGroupLength;
	unsigned char				*FileMetaInformationVersion;
	char						*MediaStorageSOPClassUID;
	char						*MediaStorageSOPInstanceUID;
	char						*TransferSyntaxUniqueIdentifier;
	DECODING_PLAN				FileDecodingPlan;
	char						*ImplementationClassUID;
	char						*ImplementationVersionName;
	char						*SourceApplicationName;
	char						*ImageTypeUniqueIdentifier;
	char						*DataCreationDate;
	char						*DataCreationTime;
	char						*DataCreatorUniqueIdentifier;
	char						*SOPClassUniqueIdentifier;
	char						*SOPInstanceUniqueIdentifier;
	char						*StudyDate;
	char						*SeriesDate;
	char						*AcquisitionDate;
	char						*StudyTime;
	char						*SeriesTime;
	char						*AcquisitionTime;
	char						*AccessionNumber;
	char						*Modality;
	char						*InstitutionName;
	PERSON_NAME					*ReferringPhysician;
	char						*StudyDescription;
	char						*SeriesDescription;
	char						*AdmittingDiagnosesDescription;
	PERSON_NAME					*PatientName;
	char						*PatientID;
	char						*PatientBirthDate;
	char						*PatientSex;
	char						*PatientAge;
	char						*AdditionalPatientHistory;
	char						*PatientComments;
	char						*BodyPartExamined;
	char						*ImagerPixelSpacing;
	char						*PatientPosition;
	char						*ViewPosition;
	char						*StudyInstanceUID;
	char						*SeriesInstanceUID;
	char						*StudyID;
	char						*SeriesNumber;
	char						*AcquisitionNumber;
	char						*InstanceNumber;
	char						*ImagesInAcquisition;
	char						*ImageComments;
	LIST_HEAD					ListOfInputBuffers;
	IMAGE_FILE_SET_SPECIFICATION	*ListOfImageFileSetSpecifications;
	IMAGE_FILE_SET_SPECIFICATION	*pCurrentImageFileSetSpecification;
	} DICOM_HEADER_SUMMARY;


// Buffer structure for transmitting exam summary information for database indexing.
typedef struct
	{
	char					*pFirstName;
	char					*pLastName;
	char					*pExamID;
	char					*pAppointmentDate;
	char					*pAppointmentTime;
	char					*pSeriesNumber;
	char					*pSeriesDescription;
	DICOM_HEADER_SUMMARY	*pDicomInfo;
	} EXAM_INFO;


typedef struct
	{
	unsigned short		Group;
	unsigned short		Element;
	unsigned short		LogSpecifications;
	unsigned short		DataLoadingType;
							#define	DATA_LOADING_UNSPECIFIED	0x00
							#define	DATA_LOADING_TYPE_TEXT		0x01
							#define	DATA_LOADING_TYPE_NAME		0x02
							#define	DATA_LOADING_TYPE_INT		0x04
							#define	DATA_LOADING_TYPE_FLOAT		0x08
							#define	DATA_LOADING_TYPE_CALIBR	0x10
	long				DataStructureOffset;
	} DICOM_ELEMENT_SEMANTICS;


typedef struct
	{
	unsigned long		BitCode;		// Used to specify one or more transfer syntaxes to be
										// included in a presentation context.
	char				*pUIDString;	// Pointer to the transfer syntax UID.
	} TRANSFER_SYNTAX_TABLE_ENTRY;


// Function prototypes.
//
void					InitDicomModule();
void					CloseDicomModule();

BOOL					IsSpecialDicomElement( TAG DicomElementTag, DICOM_ELEMENT_SEMANTICS **ppDicomElementInfo );
void					GatherDicomDirectoryInfo( DICOM_ELEMENT *pDicomElement, DICOM_HEADER_SUMMARY *pDicomHeader );
BOOL					LoadDicomHeaderElement( DICOM_ELEMENT *pDicomElement, DICOM_HEADER_SUMMARY *pDicomHeader );
void					LogDicomElement( DICOM_ELEMENT *pDicomElement, long SequenceNestingLevel );
void					SaveBufferCursor( LIST_ELEMENT **ppBufferListElement, LIST_ELEMENT **ppSavedBufferListElement,
											unsigned long *pnSavedBufferBytesRemainingToBeProcessed );
void					RestoreBufferCursor( LIST_ELEMENT **ppBufferListElement, LIST_ELEMENT **ppSavedBufferListElement,
											unsigned long *pnSavedBufferBytesRemainingToBeProcessed );
void					DeallocateInputBuffers( DICOM_HEADER_SUMMARY *pDicomHeader );
BOOL					ReadDicomHeaderInfo( char *DicomFileSpecification, DICOM_HEADER_SUMMARY **ppDicomHeader,
												BOOL bLogDicomElements );
BOOL					ParseDicomGroup2Info( LIST_ELEMENT	**ppBufferListElement,
												DICOM_HEADER_SUMMARY *pDicomHeader, BOOL bLogDicomElements );
BOOL					ParseDicomHeaderInfo( LIST_ELEMENT **ppBufferListElement,
												DICOM_HEADER_SUMMARY *pDicomHeader, BOOL bLogDicomElements );
BOOL					ProcessDicomElementSequence( LIST_ELEMENT **ppBufferListElement, unsigned long nBytesContainedInSequence,
										TRANSFER_SYNTAX TransferSyntax, DICOM_HEADER_SUMMARY *pDicomHeader,
										long *pNestingLevel, BOOL *bTerminateSequence, size_t *pnBytesParsed,
										BOOL bLogDicomElements );
BOOL					ReadDicomElementValue( DICOM_ELEMENT *pDicomElement, LIST_ELEMENT **ppBufferListElement, size_t *pnBytesParsed,
														char *TextLine, DICOM_HEADER_SUMMARY *pDicomHeader );
void					InitDicomHeaderSummary( DICOM_HEADER_SUMMARY *pDicomHeader );
void					FreeDicomHeaderInfo( DICOM_HEADER_SUMMARY *pDicomHeader );
FILE					*OpenDicomFileForOutput( char *DicomFileSpecification );
FILE					*OpenDicomFile( char *DicomFileSpecification );
FILE_STATUS				ReadFileData( FILE *pDicomFile, char *Buffer, long nBytesToBeRead );
BOOL					HasRecognizedValueRepresentation( VR ValueRepresentation );
BOOL					CopyBytesFromBuffer( char *pDestinationAddress, unsigned long nBytesNeeded, LIST_ELEMENT **ppBufferListElement );
BOOL					AdvanceBufferCursor( unsigned long nBytesNeeded, LIST_ELEMENT **ppBufferListElement );
BOOL					ParseDicomElement( LIST_ELEMENT **ppBufferListElement, DICOM_ELEMENT *pDicomElement, size_t *pnBytesParsed,
									TRANSFER_SYNTAX TransferSyntax, unsigned char SequenceNestingLevel, DICOM_HEADER_SUMMARY *pDicomHeader );
BOOL					ParseDicomElementValue( LIST_ELEMENT **ppBufferListElement, DICOM_ELEMENT *pDicomElement,
									size_t *pnBytesParsed, VR ValueRepresentationOverride, char *pTextLine );
char					*AllocateDicomValueBuffer( DICOM_ELEMENT *pDicomElement );
PERSON_NAME				*AllocateDicomPersonNameBuffer( DICOM_ELEMENT *pDicomElement );
void					CopyDicomNameToString( char *TextString, PERSON_NAME *pName, long nTextStringSize );
void					SetDicomElementValueMultiplicity( DICOM_ELEMENT *pDicomElement );
TRANSFER_SYNTAX			GetTransferSyntaxForDicomElementParsing( unsigned char AcceptedTransferSyntaxIndex );
TRANSFER_SYNTAX			InterpretUniqueTransferSyntaxIdentifier( char *pTransferSyntaxUniqueIdentifier );
void					CloseDicomFile( FILE *pDicomFile );
TRANSFER_SYNTAX			GetConsistentTransferSyntax( TRANSFER_SYNTAX DeclaredTransferSyntax, LIST_ELEMENT **ppBufferListElement );
TRANSFER_SYNTAX			GetConsistentTransferSyntaxFromBuffer( TRANSFER_SYNTAX DeclaredTransferSyntax, char *pBufferReadPoint );
