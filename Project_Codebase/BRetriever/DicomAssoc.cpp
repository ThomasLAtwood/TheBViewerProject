// DicomAssoc.cpp - Implements data structures and functions that support the
// Dicom association interactions.
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
//	*[2] 03/11/2024 by Tom Atwood
//		Convert windows headers byte packing to the Win32 default for compatibility
//		with Visual Studio 2022.
//	*[1] 03/05/2024 by Tom Atwood
//		Fixed security issues.
//
//

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#pragma pack(push, 8)		// *[2] Pack structure members on 8-byte boundaries to overcome 64-bit Microsoft errors.
#include <winsock2.h>
#pragma pack(pop)			// *[2]
#include "Module.h"
#include "ReportStatus.h"
#include "ServiceMain.h"
#include "Dicom.h"
#include "Abstract.h"
#include "Configuration.h"
#include "Operation.h"
#include "ProductDispatcher.h"
#include "WinSocketsAPI.h"
#include "DicomAssoc.h"
#include "DicomCommand.h"
#include "DicomAcceptor.h"
#include "DicomInitiator.h"
#include "DicomCommunication.h"


//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		DicomAssocModuleInfo = { MODULE_DICOMASSOC, "Dicom Association Module", InitDicomAssocModule, 0 };


static ERROR_DICTIONARY_ENTRY	DicomAssocErrorCodes[] =
			{
				{ DICOMASSOC_ERROR_INSUFFICIENT_MEMORY			, "There is not enough memory to allocate a data structure." },
				{ DICOMASSOC_ERROR_INVALID_CALLED_AE_TITLE		, "The Called AE Title had an invalid length." },
				{ DICOMASSOC_ERROR_TEMP_IMAGE_FILE_WRITE		, "An error occurred writing image data to the Dicom image file." },
				{ DICOMASSOC_ERROR_TEMP_IMAGE_FILE_OPEN			, "An error occurred opening a new file for storing an incoming Dicom image." },
				{ DICOMASSOC_ERROR_TEMP_IMAGE_FILE_CLOSED		, "The local image file was found to be closed during new data reception for appending." },
				{ DICOMASSOC_ERROR_NO_PRES_CONTEXT_FOUND		, "No accepted presentation syntax was available for controlling data formatting." },
				{ 0												, NULL }
			};


static ERROR_DICTIONARY_MODULE		DicomAssocStatusErrorDictionary =
										{
										MODULE_DICOMASSOC,
										DicomAssocErrorCodes,
										DICOMASSOC_ERROR_DICT_LENGTH,
										0
										};


extern CONFIGURATION			ServiceConfiguration;
extern ENDPOINT					EndPointWatchFolder;

static char		*TheApplicationContextUID = "1.2.840.10008.3.1.1.1";	// This signifies the first and only version of Dicom standard.
static char		*pTransferServiceImplementationClassUID = "1.2.840.9999999.7.4.2010.1.1";
static char		*pTransferServiceImplementationVersionName = "BRetriever 1";

static TRANSFER_SYNTAX_TABLE_ENTRY		TransferSyntaxLookupTable[ NUMBER_OF_TRANSFER_SYNTAX_IDS ] =
	{
	{ 0x00000001, "1.2.840.10008.1.2" },		// Implicit VR Little Endian: Default Transfer Syntax for DICOM.
	{ 0x00000002, "1.2.840.10008.1.2.1" },		// Explicit VR Little Endian.
	{ 0x00000004, "1.2.840.10008.1.2.2" },		// Explicit VR Big Endian.
	{ 0x00000008, "1.2.840.10008.1.2.4.50" },	// JPEG Baseline (Process 1): Default Transfer Syntax for Lossy JPEG 8 Bit Image Compression.
	{ 0x00000010, "1.2.840.10008.1.2.4.51" },	// JPEG Extended (Process 2 & 4): Default Transfer Syntax for Lossy JPEG 12 Bit Image Compression (Process 4 only).
	{ 0x00000020, "1.2.840.10008.1.2.4.52" },	// JPEG Extended (Process 3 & 5).
	{ 0x00000040, "1.2.840.10008.1.2.4.53" },	// JPEG Spectral Selection, Non-Hierarchical (Process 6 & 8).
	{ 0x00000080, "1.2.840.10008.1.2.4.54" },	// JPEG Spectral Selection, Non-Hierarchical (Process 7 & 9).
	{ 0x00000100, "1.2.840.10008.1.2.4.55" },	// JPEG Full Progression, Non-Hierarchical (Process 10 & 12).
	{ 0x00000200, "1.2.840.10008.1.2.4.56" },	// JPEG Full Progression, Non-Hierarchical (Process 11 & 13).
	{ 0x00000400, "1.2.840.10008.1.2.4.57" },	// JPEG Lossless, Non-Hierarchical (Process 14).
	{ 0x00000800, "1.2.840.10008.1.2.4.58" },	// JPEG Lossless, Non-Hierarchical (Process 15).
	{ 0x00001000, "1.2.840.10008.1.2.4.59" },	// JPEG Extended, Hierarchical (Process 16 & 18).
	{ 0x00002000, "1.2.840.10008.1.2.4.60" },	// JPEG Extended, Hierarchical (Process 17 & 19).
	{ 0x00004000, "1.2.840.10008.1.2.4.61" },	// JPEG Spectral Selection, Hierarchical (Process 20 & 22).
	{ 0x00008000, "1.2.840.10008.1.2.4.62" },	// JPEG Spectral Selection, Hierarchical (Process 21 & 23).
	{ 0x00010000, "1.2.840.10008.1.2.4.63" },	// JPEG Full Progression, Hierarchical (Process 24 & 26).
	{ 0x00020000, "1.2.840.10008.1.2.4.64" },	// JPEG Full Progression, Hierarchical (Process 25 & 27).
	{ 0x00040000, "1.2.840.10008.1.2.4.65" },	// JPEG Lossless, Hierarchical (Process 28).
	{ 0x00080000, "1.2.840.10008.1.2.4.66" },	// JPEG Lossless, Hierarchical (Process 29).
	{ 0x00100000, "1.2.840.10008.1.2.4.70" },	// JPEG Lossless, Non-Hierarchical, First-Order Prediction (Process 14 [Selection Value 1]):
												//			Default Transfer Syntax for Lossless JPEG Image Compression.
	{ 0x00200000, "1.2.840.10008.1.2.4.80" },	// JPEG-LS Lossless Image Compression.
	{ 0x00400000, "1.2.840.10008.1.2.4.81" },	// JPEG-LS Lossy (Near-Lossless) Image Compression.
	{ 0x00800000, "1.2.840.10008.1.2.5" },		// RLE Lossless.
	{ 0x01000000, "1.2.840.10008.1.2.1.99" },	// Deflated Explicit VR Little Endian.
	{ 0x02000000, "1.2.840.10008.1.2.4.90" },	// JPEG 2000 Image Compression (Lossless Only).
	{ 0x04000000, "1.2.840.10008.1.2.4.91" },	// JPEG 2000 Image Compression (Lossless or Lossy).
	{ 0x08000000, "1.2.840.10008.1.2.4.100 " },	// MPEG2 Main Profile @ Main Level.
	{ 0x10000000, "1.2.840.10008.1.2.4.92" },	// JPEG 2000 Part 2 Multi-component Image Compression (Lossless Only).
	{ 0x20000000, "1.2.840.10008.1.2.4.93" }	// JPEG 2000 Part 2 Multi-component Image Compression (Lossless or Lossy).
	};


static unsigned char	AcceptableTransferSyntaxes[] =
							{ 
							JPEG_PROCESS_14SV1_TRANSFER_SYNTAX,	
							JPEGLS_LOSSLESS_TRANSFER_SYNTAX,
							JPEG2000_LOSSLESS_ONLY_TRANSFER_SYNTAX,
							JPEG_PROCESS_14_TRANSFER_SYNTAX,
							JPEG_PROCESS_15_TRANSFER_SYNTAX,
							JPEG_PROCESS_28_TRANSFER_SYNTAX,
							JPEG_PROCESS_29_TRANSFER_SYNTAX,
							JPEG2000_TRANSFER_SYNTAX,
							JPEGLS_LOSSY_TRANSFER_SYNTAX,
							JPEG_PROCESS_2_4_TRANSFER_SYNTAX,
							JPEG_PROCESS_1_TRANSFER_SYNTAX,
							LITTLE_ENDIAN_IMPLICIT_TRANSFER_SYNTAX,
							LITTLE_ENDIAN_EXPLICIT_TRANSFER_SYNTAX,
							BIG_ENDIAN_EXPLICIT_TRANSFER_SYNTAX
							};


static ABSTRACT_SYNTAX_TABLE_ENTRY		AbstractSyntaxLookupTable[ NUMBER_OF_ABSTRACT_SYNTAX_IDS ] =
	{
	{ SOP_CLASS_VERIFICATION,						TRUE,	"1.2.840.10008.1.1",			"Echo Communications Verification",
				{ '\0', '\0', '\0', '\0' },		14, AcceptableTransferSyntaxes },
	{ SOP_CLASS_COMPUTED_RADIOGRAPHY_IMAGE_STORAGE,	TRUE,	"1.2.840.10008.5.1.4.1.1.1",	"Store Computed Radiography (CR) Image(s)",
				{ 'C', 'R', '\0', '\0' },		14, AcceptableTransferSyntaxes },
	{ SOP_CLASS_COMPUTED_TOMOGRAPHY_IMAGE_STORAGE,	TRUE,	"1.2.840.10008.5.1.4.1.1.2",	"Store Computed Tomography (CT) Image(s)",
				{ 'C', 'T', '\0', '\0' },		14, AcceptableTransferSyntaxes },
	{ SOP_CLASS_MAGNETIC_RESONANCE_IMAGE_STORAGE,	TRUE,	"1.2.840.10008.5.1.4.1.1.4",	"Store Magnetic Resonance (MR) Image(s)",
				{ 'M', 'R', '\0', '\0' },		14, AcceptableTransferSyntaxes },
	{ SOP_CLASS_NUCLEAR_MEDICINE_IMAGE_STORAGE,		TRUE,	"1.2.840.10008.5.1.4.1.1.20",	"Store Nuclear Medicine (NM) Image(s)",
				{ 'N', 'M', '\0', '\0' },		14, AcceptableTransferSyntaxes },
	{ SOP_CLASS_ULTRASOUND_IMAGE_STORAGE,			TRUE,	"1.2.840.10008.5.1.4.1.1.6.1",	"Store Untrasound (US) Image(s)",
				{ 'U', 'S', '\0', '\0' },		14, AcceptableTransferSyntaxes },
	{ SOP_CLASS_SECONDARY_CAPTURE_IMAGE_STORAGE,	TRUE,	"1.2.840.10008.5.1.4.1.1.7",	"Store Secondary Capture (SC) Image(s)",
				{ 'C', 'R', '\0', '\0' },		14, AcceptableTransferSyntaxes },
	{ SOP_CLASS_GRAYSCALE_SOFTCOPY_PRES_STORAGE,	TRUE,	"1.2.840.10008.5.1.4.1.1.11.1",	"Grayscale Softcopy Presentation State Storage",
				{ '\0', '\0', '\0', '\0' },		14, AcceptableTransferSyntaxes },
	{ SOP_CLASS_DIGITAL_XRAY_FOR_PRES_STORAGE,		TRUE,	"1.2.840.10008.5.1.4.1.1.1.1",	"Store Digital X-Ray Image For Presentation",
				{ 'D', 'X', '\0', '\0' },		14, AcceptableTransferSyntaxes }
	};


// This function must be called before any other function in this module.
void InitDicomAssocModule()
{
	LinkModuleToList( &DicomAssocModuleInfo );
	RegisterErrorDictionary( &DicomAssocStatusErrorDictionary );
}


DICOM_ASSOCIATION *CreateAssociationStructure( PRODUCT_OPERATION *pProductOperation )
{
	BOOL					bNoError = TRUE;
	DICOM_ASSOCIATION		*pAssociation;

	pAssociation = (DICOM_ASSOCIATION*)malloc( sizeof(DICOM_ASSOCIATION) );
	if ( pAssociation != 0 )
		{
		pAssociation -> pProductOperation = pProductOperation;
		pAssociation -> bAssociationIsActive = FALSE;
		pAssociation -> DicomAssociationSocket = INVALID_SOCKET;
		pAssociation -> CurrentStateID = STATE1_IDLE;
		pAssociation -> EventIDReadyToBeProcessed = EVENT_NO_EVENT_ID_SPECIFIED;
		pAssociation -> AssociationBufferList = 0;
		pAssociation -> pReceivedBuffer = 0;
		pAssociation -> ReceivedBufferLength = 0L;
		pAssociation -> bAssociationAccepted = FALSE;
		pAssociation -> bReleaseReplyReceived = FALSE;
		pAssociation -> bAssociationClosed = FALSE;
		pAssociation -> bReturnAfterAssociationEstablished = FALSE;
		pAssociation -> bReadyToProceedWithAssociation = FALSE;
		pAssociation -> nAcceptedAbstractSyntax = NUMBER_OF_ABSTRACT_SYNTAX_IDS;
		memset( (void*)&pAssociation -> PresentationContextSelector, '\0', sizeof( PRESENTATION_CONTEXT_SELECTOR ) );
		pAssociation -> PresentationContextSelector.AcceptedPresentationContext = 255;
		pAssociation -> PresentationContextSelector.AcceptedTransferSyntax = 255;
		pAssociation -> PresentationContextSelector.MostPreferableTransferSyntaxProposed = 255;
		pAssociation -> pImplementationClassUID = 0;
		pAssociation -> pImplementationVersionName = 0;
		pAssociation -> bAssociationSyntaxIsBigEndian = TRUE;
		pAssociation -> bCurrentSyntaxIsLittleEndian = TRUE;
		pAssociation -> CurrentMessageID = 0;
		pAssociation -> bSentMessageExpectsResponse = FALSE;
		pAssociation -> ProposedPresentationContextList = 0;
		pAssociation -> AssociatedImageList = 0;
		pAssociation -> pCurrentAssociatedImageInfo = 0;
		if ( strlen( pProductOperation -> pOutputEndPoint -> AE_TITLE ) <= 16 )
			{
			memset( pAssociation -> RemoteAE_Title, ' ', 16 );
			memcpy( pAssociation -> RemoteAE_Title, pProductOperation -> pOutputEndPoint -> AE_TITLE, strlen( pProductOperation -> pOutputEndPoint -> AE_TITLE ) );
			pAssociation -> RemoteAE_Title[ 16 ] = '\0';
			}
		else
			{
			bNoError = FALSE;
			RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_INVALID_CALLED_AE_TITLE );
			}
		memset( pAssociation -> LocalAE_Title, ' ', 16 );
		pAssociation -> LocalAE_Title[ 16 ] = '\0';
		if ( !bNoError )
			{
			DeleteAssociationStructure( pAssociation );
			pAssociation = 0;
			}
		}
	else
		{
		bNoError = FALSE;
		RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_INSUFFICIENT_MEMORY );
		}

	return pAssociation;
}


void DeleteAssociationStructure( DICOM_ASSOCIATION *pAssociation )
{
	LIST_ELEMENT				*pListElement;
	LIST_ELEMENT				*pPrevListElement;
	ASSOCIATED_IMAGE_INFO		*pAssociatedImageInfo;
	PRESENTATION_CONTEXT_ITEM	*pPresentationContextItem;

	if ( pAssociation != 0 )
		{
		if ( pAssociation -> pImplementationClassUID != 0 )
			free( pAssociation -> pImplementationClassUID );
		if ( pAssociation -> pImplementationVersionName != 0 )
			free( pAssociation -> pImplementationVersionName );
		if ( pAssociation -> AssociationBufferList != 0 )
			free( pAssociation -> AssociationBufferList );
		
		pListElement = pAssociation -> AssociatedImageList;
		while ( pListElement != 0 )
			{
			pAssociatedImageInfo = (ASSOCIATED_IMAGE_INFO*)pListElement -> pItem;
			if ( pAssociatedImageInfo != 0 )
				free( pAssociatedImageInfo );
			pPrevListElement = pListElement;
			pListElement = pListElement -> pNextListElement;
			free( pPrevListElement );
			}

		pListElement = pAssociation -> ProposedPresentationContextList;
		while ( pListElement != 0 )
			{
			pPresentationContextItem = (PRESENTATION_CONTEXT_ITEM*)pListElement -> pItem;
			if ( pPresentationContextItem != 0 )
				free( pPresentationContextItem );
			pPrevListElement = pListElement;
			pListElement = pListElement -> pNextListElement;
			free( pPrevListElement );
			}	
		free( pAssociation );
		}
}


PRESENTATION_CONTEXT_ITEM *CreatePresentationContextItem()
{
	PRESENTATION_CONTEXT_ITEM	*pPresentationContextItem;

	pPresentationContextItem = (PRESENTATION_CONTEXT_ITEM*)malloc( sizeof(PRESENTATION_CONTEXT_ITEM) );
	if ( pPresentationContextItem == 0 )
		RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_INSUFFICIENT_MEMORY );
	else
		{
		pPresentationContextItem -> AcceptedPresentationContextID = 0;
		pPresentationContextItem -> AcceptedTransferSyntaxes = 0;
		pPresentationContextItem -> AcceptedTransferSyntaxIndex = 0;		// Dicom default transfer syntax.
		pPresentationContextItem -> pNextPresContextItem = 0;
		}

	return pPresentationContextItem;
}


// This function is only applicable for the association layer of communications.
// Typically, nValueSize is 2 for a short integer and 4 for a long integer.
// This function will work for integers of any even byte count.
void AssociationSwapBytes( DICOM_ASSOCIATION *pAssociation, void *pData, long nValueSize )
{
	long			nByte;
	char			TempChar;
	char			*pFirstChar;
	char			*pLastChar;
	
	if ( pAssociation -> bAssociationSyntaxIsBigEndian )
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


// This function is only applicable for the message content layer of communications.
// Typically, nValueSize is 2 for a short integer and 4 for a long integer.
// This function will work for integers of any even byte count.
static void MsgContentSwapBytes( DICOM_ASSOCIATION *pAssociation, void *pData, long nValueSize )
{
	long			nByte;
	char			TempChar;
	char			*pFirstChar;
	char			*pLastChar;
	
	if ( !pAssociation -> bCurrentSyntaxIsLittleEndian )
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


BOOL ParseReceivedDicomBuffer( DICOM_ASSOCIATION *pAssociation )
{
	BOOL							bNoError = TRUE;
	unsigned char					PDU_Type;
	BOOL							bThisMessageIsNotComplete;
	BOOL							bFirstBufferInSeries;
	unsigned long					PrevPDUBytesToBeRead = 0L;

	PDU_Type = (unsigned char)pAssociation -> pReceivedBuffer[ 0 ];					// *[1] Recast to eliminate data type mismatch.
	switch ( PDU_Type )
		{
		case 0x01:
			LogMessage( "Parsing association request.", MESSAGE_TYPE_SUPPLEMENTARY );
			bNoError = ParseAssociationRequestBuffer( pAssociation );
			break;
		case 0x02:
			LogMessage( "Error:  Received association acceptance response.", MESSAGE_TYPE_SUPPLEMENTARY );
			break;
		case 0x03:
			LogMessage( "Error:  Received association rejection response.", MESSAGE_TYPE_SUPPLEMENTARY );
			break;
		case 0x04:
			// Process incoming Dicom commands (C-Echo or C-Store).
			LogMessage( "Parsing command data set.", MESSAGE_TYPE_SUPPLEMENTARY );
			bThisMessageIsNotComplete = FALSE;
			bFirstBufferInSeries = TRUE;
			do
				{
				bNoError = ParseAssociationReceivedDataSetBuffer( pAssociation, bFirstBufferInSeries, &bThisMessageIsNotComplete, &PrevPDUBytesToBeRead );
				if ( pAssociation -> ReceivedCommandID != DICOM_CMD_STORE )	// This will be changed when the image data arrives.
					bFirstBufferInSeries = FALSE;
				if ( bNoError && bThisMessageIsNotComplete )
					bNoError = ReceiveDicomBuffer( pAssociation );
				else if ( pAssociation -> ReceivedCommandID == DICOM_CMD_DATA )
					pAssociation -> ReceivedCommandID = DICOM_CMD_STORE;
				}
			while ( bNoError && bThisMessageIsNotComplete );
			if ( pAssociation -> ReceivedCommandID == DICOM_CMD_STORE )
				{
				LogMessage( "Preparing C-Store Response.", MESSAGE_TYPE_SUPPLEMENTARY );
				bNoError = PrepareCStoreResponseBuffer( pAssociation, bNoError );
				if ( bNoError )
					{
					pAssociation -> EventIDReadyToBeProcessed = EVENT_THIS_NODE_REQUESTS_TO_SEND_MESSAGE;
					pAssociation -> bSentMessageExpectsResponse = TRUE;		// Expect a release request or another command.
					}
				}
			else if ( pAssociation -> ReceivedCommandID == DICOM_CMD_STORE_RESPONSE )
				{
				pAssociation -> EventIDReadyToBeProcessed = EVENT_THIS_NODE_REQUESTS_ASSOCIATION_RELEASE;
				pAssociation -> bSentMessageExpectsResponse = TRUE;			// Expect a release request response.
				}
			break;
		case 0x05:
			LogMessage( "Parsing association release request.", MESSAGE_TYPE_SUPPLEMENTARY );
			bNoError = ParseAssociationReleaseRequestBuffer( pAssociation );
			break;
		case 0x06:
			LogMessage( "Parsing association release reply.", MESSAGE_TYPE_SUPPLEMENTARY );
			bNoError = ParseAssociationReleaseReplyBuffer( pAssociation );
			break;
		case 0x07:
			LogMessage( "Parsing association abort.", MESSAGE_TYPE_SUPPLEMENTARY );
			bNoError = ParseAssociationAbortBuffer( pAssociation );
			break;
		}
	return bNoError;
}


BOOL PrepareApplicationContextBuffer( DICOM_ASSOCIATION *pAssociation )
{
	BOOL							bNoError = TRUE;
	BUFFER_LIST_ELEMENT				*pBufferDescriptor;
	A_APPLICATION_CONTEXT_BUFFER	*pBufferElement;

	pBufferDescriptor = (BUFFER_LIST_ELEMENT*)malloc( sizeof(BUFFER_LIST_ELEMENT) );
	if ( pBufferDescriptor == 0 )
		{
		bNoError = FALSE;
		RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_INSUFFICIENT_MEMORY );
		}
	if ( bNoError )
		{
		pBufferElement = (A_APPLICATION_CONTEXT_BUFFER*)malloc( sizeof(A_APPLICATION_CONTEXT_BUFFER) );
		if ( pBufferElement == 0 )
			{
			bNoError = FALSE;
			RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_INSUFFICIENT_MEMORY );
			free( pBufferDescriptor );																	// *[1] Fix potential memory leak.
		}
		else
			{
			memset( (char*)pBufferElement, '\0', sizeof(A_APPLICATION_CONTEXT_BUFFER) );
			pBufferElement -> PDU_Type = 0x10;
			pBufferElement -> Reserved1 = 0x00;
			pBufferElement -> Length = 21;
			}
		}
	if ( bNoError )
		{
		pBufferDescriptor -> BufferType = BUFTYPE_A_APPLICATION_CONTEXT_BUFFER;
		pBufferDescriptor -> MaxBufferLength = sizeof(A_APPLICATION_CONTEXT_BUFFER);
		pBufferDescriptor -> InsertedBufferLength = pBufferElement -> Length + 4;
		pBufferDescriptor -> bInsertedLengthIsFinalized = TRUE;
		pBufferDescriptor -> pBuffer = (void*)pBufferElement;

		strncpy_s( pBufferElement -> ApplicationContextName, 64, TheApplicationContextUID, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
		AssociationSwapBytes( pAssociation, &pBufferElement -> Length, 2 );
		bNoError = PrefixToList( &pAssociation -> AssociationBufferList, (void*)pBufferDescriptor );
		}

	return bNoError;
}


BOOL PrepareTransferSyntaxBuffer( DICOM_ASSOCIATION *pAssociation, int TransferSyntaxIndex )
{
	BOOL							bNoError = TRUE;
	BUFFER_LIST_ELEMENT				*pBufferDescriptor;
	A_TRANSFER_SYNTAX_BUFFER		*pBufferElement;
	char							*pTransferSyntaxUID;

	pBufferDescriptor = (BUFFER_LIST_ELEMENT*)malloc( sizeof(BUFFER_LIST_ELEMENT) );
	if ( pBufferDescriptor == 0 )
		{
		bNoError = FALSE;
		RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_INSUFFICIENT_MEMORY );
		}
	if ( bNoError )
		{
		pBufferElement = (A_TRANSFER_SYNTAX_BUFFER*)malloc( sizeof(A_TRANSFER_SYNTAX_BUFFER) );
		if ( pBufferElement == 0 )
			{
			bNoError = FALSE;
			RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_INSUFFICIENT_MEMORY );
			free( pBufferDescriptor );																// *[1] Clean up prior to exit.
			}
		else
			{
			memset( (char*)pBufferElement, '\0', sizeof(A_TRANSFER_SYNTAX_BUFFER) );
			pBufferElement -> PDU_Type = 0x40;
			pBufferElement -> Reserved1 = 0x00;
			}
		}
	if ( bNoError )
		{
		pTransferSyntaxUID = TransferSyntaxLookupTable[ TransferSyntaxIndex ].pUIDString;
		pBufferElement -> Length = (unsigned short)strlen( pTransferSyntaxUID );
		strncpy_s( pBufferElement -> TransferSyntaxName, 64, pTransferSyntaxUID, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
		
		pBufferDescriptor -> BufferType = BUFTYPE_A_TRANSFER_SYNTAX_BUFFER;
		pBufferDescriptor -> MaxBufferLength = sizeof(A_TRANSFER_SYNTAX_BUFFER);
		pBufferDescriptor -> InsertedBufferLength = pBufferElement -> Length + 4;
		pBufferDescriptor -> bInsertedLengthIsFinalized = TRUE;
		pBufferDescriptor -> pBuffer = (void*)pBufferElement;

		AssociationSwapBytes( pAssociation, &pBufferElement -> Length, 2 );
		bNoError = PrefixToList( &pAssociation -> AssociationBufferList, (void*)pBufferDescriptor );
		}

	return bNoError;
}


BOOL PrepareUserInformationBuffer( DICOM_ASSOCIATION *pAssociation )
{
	BOOL							bNoError = TRUE;
	BUFFER_LIST_ELEMENT				*pBufferDescriptor;
	A_USER_INFO_ITEM_HEADER_BUFFER	*pBufferElement;

	pBufferDescriptor = (BUFFER_LIST_ELEMENT*)malloc( sizeof(BUFFER_LIST_ELEMENT) );
	if ( pBufferDescriptor == 0 )
		{
		bNoError = FALSE;
		RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_INSUFFICIENT_MEMORY );
		}
	if ( bNoError )
		{
		pBufferElement = (A_USER_INFO_ITEM_HEADER_BUFFER*)malloc( sizeof(A_USER_INFO_ITEM_HEADER_BUFFER) );
		if ( pBufferElement == 0 )
			{
			bNoError = FALSE;
			RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_INSUFFICIENT_MEMORY );
			free( pBufferDescriptor );							// *[1] Fix potential memory leak.
			}
		else
			{
			memset( (char*)pBufferElement, '\0', sizeof(A_USER_INFO_ITEM_HEADER_BUFFER) );
			pBufferElement -> PDU_Type = 0x50;
			pBufferElement -> Reserved1 = 0x00;
			}
		}
	if ( bNoError )
		{
		pBufferElement -> Length = 0L;
		pBufferDescriptor -> BufferType = BUFTYPE_A_USER_INFO_ITEM_HEADER_BUFFER;
		pBufferDescriptor -> MaxBufferLength = sizeof(A_USER_INFO_ITEM_HEADER_BUFFER);
		pBufferDescriptor -> InsertedBufferLength = 4;
		pBufferDescriptor -> bInsertedLengthIsFinalized = FALSE;
		pBufferDescriptor -> pBuffer = (void*)pBufferElement;

		bNoError = PrefixToList( &pAssociation -> AssociationBufferList, pBufferDescriptor );
		if ( !bNoError )									// *[1] Deallocate the buffers here in the event of an error.
			{
			if ( pBufferElement != 0 )
				free( pBufferElement );
			if ( pBufferDescriptor != 0 )
				free( pBufferDescriptor );
			}
		}
	// Prepare the subitem buffers to be appended to this buffer.
	if ( bNoError )
		bNoError = PrepareMaximumLengthBuffer( pAssociation );
	// Append the subitem buffer to this buffer.
	if ( bNoError )
		{
		bNoError = AppendSubitemBuffer( pAssociation, pBufferDescriptor );
		if ( bNoError )
			{
			pBufferElement = (A_USER_INFO_ITEM_HEADER_BUFFER*)pBufferDescriptor -> pBuffer;
			pBufferElement -> Length = (unsigned short)( pBufferDescriptor -> InsertedBufferLength - 4 );
			}
		}
	if ( bNoError )
		bNoError = PrepareImplementationClassUIDBuffer( pAssociation );
	// Append the subitem buffer to this buffer.
	if ( bNoError )
		{
		bNoError = AppendSubitemBuffer( pAssociation, pBufferDescriptor );
		if ( bNoError )
			{
			pBufferElement = (A_USER_INFO_ITEM_HEADER_BUFFER*)pBufferDescriptor -> pBuffer;
			pBufferElement -> Length = (unsigned short)( pBufferDescriptor -> InsertedBufferLength - 4 );
			}
		}
	if ( bNoError )
		bNoError = PrepareImplementationVersionNameBuffer( pAssociation );
	// Append the subitem buffer to this buffer.
	if ( bNoError )
		{
		bNoError = AppendSubitemBuffer( pAssociation, pBufferDescriptor );
		if ( bNoError )
			{
			pBufferElement = (A_USER_INFO_ITEM_HEADER_BUFFER*)pBufferDescriptor -> pBuffer;
			pBufferElement -> Length = (unsigned short)( pBufferDescriptor -> InsertedBufferLength - 4 );
			}
		}
	if ( bNoError )
		{
		AssociationSwapBytes( pAssociation, &pBufferElement -> Length, 2 );
		pBufferDescriptor -> bInsertedLengthIsFinalized = TRUE;
		}
		
	return bNoError;
}


BOOL PrepareMaximumLengthBuffer( DICOM_ASSOCIATION *pAssociation )
{
	BOOL							bNoError = TRUE;
	BUFFER_LIST_ELEMENT				*pBufferDescriptor;
	A_MAXIMUM_LENGTH_REQUEST_BUFFER	*pBufferElement;

	pBufferDescriptor = (BUFFER_LIST_ELEMENT*)malloc( sizeof(BUFFER_LIST_ELEMENT) );
	if ( pBufferDescriptor == 0 )
		{
		bNoError = FALSE;
		RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_INSUFFICIENT_MEMORY );
		}
	if ( bNoError )
		{
		pBufferElement = (A_MAXIMUM_LENGTH_REQUEST_BUFFER*)malloc( sizeof(A_MAXIMUM_LENGTH_REQUEST_BUFFER) );
		if ( pBufferElement == 0 )
			{
			bNoError = FALSE;
			RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_INSUFFICIENT_MEMORY );
			free( pBufferDescriptor );											// *[1] Fix potential memory leak.
			}
		else
			{
			memset( (char*)pBufferElement, '\0', sizeof(A_MAXIMUM_LENGTH_REQUEST_BUFFER) );
			pBufferElement -> PDU_Type = 0x51;
			pBufferElement -> Reserved1 = 0x00;
			pBufferElement -> Length = 0x0004;
			}
		}
	if ( bNoError )
		{
		pBufferElement -> MaximumLengthReceivable = MAX_ASSOCIATION_RECEIVED_BUFFER_SIZE;
		AssociationSwapBytes( pAssociation, &pBufferElement -> MaximumLengthReceivable, 4 );
		
		pBufferDescriptor -> BufferType = BUFTYPE_A_MAXIMUM_LENGTH_REQUEST_BUFFER;
		pBufferDescriptor -> MaxBufferLength = sizeof(A_MAXIMUM_LENGTH_REQUEST_BUFFER);
		pBufferDescriptor -> InsertedBufferLength = pBufferElement -> Length + 4;
		pBufferDescriptor -> bInsertedLengthIsFinalized = TRUE;
		pBufferDescriptor -> pBuffer = (void*)pBufferElement;

		AssociationSwapBytes( pAssociation, &pBufferElement -> Length, 2 );
		bNoError = PrefixToList( &pAssociation -> AssociationBufferList, (void*)pBufferDescriptor );
		}

	return bNoError;
}


BOOL PrepareImplementationClassUIDBuffer( DICOM_ASSOCIATION *pAssociation )
{
	BOOL								bNoError = TRUE;
	BUFFER_LIST_ELEMENT					*pBufferDescriptor;
	A_IMPLEMENTATION_CLASS_UID_BUFFER	*pBufferElement;

	pBufferDescriptor = (BUFFER_LIST_ELEMENT*)malloc( sizeof(BUFFER_LIST_ELEMENT) );
	if ( pBufferDescriptor == 0 )
		{
		bNoError = FALSE;
		RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_INSUFFICIENT_MEMORY );
		}
	if ( bNoError )
		{
		pBufferElement = (A_IMPLEMENTATION_CLASS_UID_BUFFER*)malloc( sizeof(A_IMPLEMENTATION_CLASS_UID_BUFFER) );
		if ( pBufferElement == 0 )
			{
			bNoError = FALSE;
			RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_INSUFFICIENT_MEMORY );
			free( pBufferDescriptor );							// *[1] Fix potential memory leak.
			}
		else
			{
			memset( (char*)pBufferElement, '\0', sizeof(A_IMPLEMENTATION_CLASS_UID_BUFFER) );
			pBufferElement -> PDU_Type = 0x52;
			pBufferElement -> Reserved1 = 0x00;
			}
		}
	if ( bNoError )
		{
		pBufferElement -> Length = (unsigned short)strlen( pTransferServiceImplementationClassUID );
		strncpy_s( pBufferElement -> ImplementationClassUID, 64, pTransferServiceImplementationClassUID, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
		
		pBufferDescriptor -> BufferType = BUFTYPE_A_IMPLEMENTATION_CLASS_UID_BUFFER;
		pBufferDescriptor -> MaxBufferLength = sizeof(A_IMPLEMENTATION_CLASS_UID_BUFFER);
		pBufferDescriptor -> InsertedBufferLength = pBufferElement -> Length + 4;
		pBufferDescriptor -> bInsertedLengthIsFinalized = TRUE;
		pBufferDescriptor -> pBuffer = (void*)pBufferElement;

		AssociationSwapBytes( pAssociation, &pBufferElement -> Length, 2 );
		bNoError = PrefixToList( &pAssociation -> AssociationBufferList, (void*)pBufferDescriptor );
		}

	return bNoError;
}


BOOL PrepareImplementationVersionNameBuffer( DICOM_ASSOCIATION *pAssociation )
{
	BOOL									bNoError = TRUE;
	BUFFER_LIST_ELEMENT						*pBufferDescriptor;
	A_IMPLEMENTATION_VERSION_NAME_BUFFER	*pBufferElement;

	pBufferDescriptor = (BUFFER_LIST_ELEMENT*)malloc( sizeof(BUFFER_LIST_ELEMENT) );
	if ( pBufferDescriptor == 0 )
		{
		bNoError = FALSE;
		RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_INSUFFICIENT_MEMORY );
		}
	if ( bNoError )
		{
		pBufferElement = (A_IMPLEMENTATION_VERSION_NAME_BUFFER*)malloc( sizeof(A_IMPLEMENTATION_VERSION_NAME_BUFFER) );
		if ( pBufferElement == 0 )
			{
			bNoError = FALSE;
			RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_INSUFFICIENT_MEMORY );
			}
		else
			{
			memset( (char*)pBufferElement, '\0', sizeof(A_IMPLEMENTATION_VERSION_NAME_BUFFER) );
			pBufferElement -> PDU_Type = 0x55;
			pBufferElement -> Reserved1 = 0x00;
			}
		}
	if ( bNoError )
		{
		pBufferElement -> Length = (unsigned short)strlen( pTransferServiceImplementationVersionName );
		memcpy( pBufferElement -> ImplementationVersionName, pTransferServiceImplementationVersionName, pBufferElement -> Length );
		
		pBufferDescriptor -> BufferType = BUFTYPE_A_IMPLEMENTATION_VERSION_NAME_BUFFER;
		pBufferDescriptor -> MaxBufferLength = sizeof(A_IMPLEMENTATION_VERSION_NAME_BUFFER);
		pBufferDescriptor -> InsertedBufferLength = pBufferElement -> Length + 4;
		pBufferDescriptor -> bInsertedLengthIsFinalized = TRUE;
		pBufferDescriptor -> pBuffer = (void*)pBufferElement;

		AssociationSwapBytes( pAssociation, &pBufferElement -> Length, 2 );
		bNoError = PrefixToList( &pAssociation -> AssociationBufferList, (void*)pBufferDescriptor );
		}
	if ( !bNoError )								// *[1] Deallocate the buffers here in the event of an error.
		{
		if ( pBufferElement != 0 )
			free( pBufferElement );
		if ( pBufferDescriptor != 0 )
			free( pBufferDescriptor );
		}

	return bNoError;
}

BOOL AppendSubitemBuffer( DICOM_ASSOCIATION *pAssociation, BUFFER_LIST_ELEMENT *pParentBufferDescriptor )
{
	BOOL							bNoError = TRUE;
	BUFFER_LIST_ELEMENT				*pSubitemBufferDescriptor;
	void							*pOriginalParentBuffer;
	void							*pReallocatedParentBuffer;
	void							*pSubitemBuffer;
	unsigned long					OriginalBufferLength;
	unsigned long					RevisedBufferLength;
	LIST_ELEMENT					*pListElement;

	// The subitem to be appended to the end of the parent buffer is the first one on the association's buffer list.
	pSubitemBufferDescriptor = (BUFFER_LIST_ELEMENT*)pAssociation -> AssociationBufferList -> pItem;
	pSubitemBuffer = pSubitemBufferDescriptor -> pBuffer;
	if ( pSubitemBufferDescriptor -> bInsertedLengthIsFinalized )
		{
		OriginalBufferLength = pParentBufferDescriptor -> InsertedBufferLength;
		RevisedBufferLength = pParentBufferDescriptor -> InsertedBufferLength + pSubitemBufferDescriptor -> InsertedBufferLength;
		pOriginalParentBuffer = pParentBufferDescriptor -> pBuffer;
		pReallocatedParentBuffer = realloc( pOriginalParentBuffer, RevisedBufferLength );
		if ( pReallocatedParentBuffer != 0 )
			{
			pParentBufferDescriptor -> pBuffer = pReallocatedParentBuffer;
			pParentBufferDescriptor -> InsertedBufferLength = RevisedBufferLength;
			// Other than this appending of the subitem, no changes are made here to the parent buffer.
			memcpy( (char*)pReallocatedParentBuffer + OriginalBufferLength,
						(char*)pSubitemBuffer, pSubitemBufferDescriptor -> InsertedBufferLength );
			// The subitem buffer element has been copied into the parent buffer.  The subitem buffer can be deleted:
			free( pSubitemBuffer );
			free( pSubitemBufferDescriptor );
			// The buffer list now starts with the parent buffer.
			pListElement = pAssociation -> AssociationBufferList;
			pAssociation -> AssociationBufferList = pAssociation -> AssociationBufferList -> pNextListElement;
			if ( pListElement != 0 )
				free( pListElement );
			// NOTE:  The calling function is responsible for setting the revised length into the parent buffer.
			}
		else
			{
			bNoError = FALSE;
			RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_INSUFFICIENT_MEMORY );
			}
		}
	else
		bNoError = FALSE;

	return bNoError;
}



//____________________________________________________________________________________________________________________________________________________________
// Received buffer parsing functions.
//

BOOL ParseAssociationReceivedDataSetBuffer( DICOM_ASSOCIATION *pAssociation, BOOL bFirstBufferInSeries,
												BOOL *pbNeedsMoreData, unsigned long *pPrevPDUBytesToBeRead )
{
	BOOL								bNoError = TRUE;
	DATA_TRANSFER_PDU_HEADER			MessagePacketHeader;
	PRESENTATION_DATA_VALUE_HEADER		CommandMessageHeader;
	char								*pBuffer;
	char								*pBufferReadPoint;
	unsigned long						RemainingBufferSize;
	unsigned long						RemainingPDULength;
	unsigned long						RemainingDataSetLength;
	BOOL								bNeedsMoreBuffer;
	unsigned char						PresentationContextID;
	TRANSFER_SYNTAX						TransferSyntax;
	BOOL								bTransferSyntaxFound;
	char								*pExamDepositDirectory;
	char								*pGroup2Buffer = 0;
	unsigned long						Group2BufferSize;
	char								LocalFileSpecification[ MAX_FILE_SPEC_LENGTH ];
	long								nBytesToBeWritten;
	long								nBytesWritten;
	char								Message[ 1096 ];
	PRESENTATION_CONTEXT_ITEM			*pPresentationContextItem;
	PRESENTATION_CONTEXT_ITEM			*pTrialPresentationContextItem;
	PRESENTATION_CONTEXT_ITEM			*pSelectedPresentationContextItem;
	unsigned long						TransferSyntaxValueLength;
	char								*pTransferSyntaxUID;
	LIST_ELEMENT						*pListElement;
	DECODING_PLAN						FileDecodingPlan;
	char								TextField[ MAX_LOGGING_STRING_LENGTH ];

	pBuffer = pAssociation -> pReceivedBuffer;
	pBufferReadPoint = pBuffer;
	RemainingBufferSize = pAssociation -> ReceivedBufferLength;
	while ( bNoError && RemainingBufferSize > 0 )
		{
		// Handle any leftover buffer from the previous parse.
		if ( *pPrevPDUBytesToBeRead > 0L )
			{
			if ( pAssociation -> pCurrentAssociatedImageInfo -> pImageDataFile != 0 )
				{
				nBytesToBeWritten = *pPrevPDUBytesToBeRead;
				nBytesWritten = (long)fwrite( pBufferReadPoint, 1,
							(long)nBytesToBeWritten, pAssociation -> pCurrentAssociatedImageInfo -> pImageDataFile );
				if ( nBytesWritten != nBytesToBeWritten )
					{
					bNoError = FALSE;
					RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_TEMP_IMAGE_FILE_WRITE );
					}
				*pPrevPDUBytesToBeRead = 0L;
				pBufferReadPoint += nBytesWritten;
				RemainingBufferSize -= nBytesWritten;
				}
			}
		// Read the message packet header.
		memcpy( (char*)&MessagePacketHeader, pBufferReadPoint, sizeof(DATA_TRANSFER_PDU_HEADER) );
		pBufferReadPoint += sizeof(DATA_TRANSFER_PDU_HEADER);
		RemainingBufferSize -= sizeof(DATA_TRANSFER_PDU_HEADER);
		if ( MessagePacketHeader.PDU_Type == 0x04 )
			{
			AssociationSwapBytes( pAssociation, &MessagePacketHeader.PDULength, 4 );

			_snprintf_s( Message, 1096, _TRUNCATE, "Received Data Set Buffer.  Length = %d", MessagePacketHeader.PDULength );	// *[1] Replaced sprintf() with _snprintf_s.
			LogMessage( Message, MESSAGE_TYPE_DETAILS );

			RemainingPDULength = MessagePacketHeader.PDULength;
			// Read the command message header.
			memcpy( (char*)&CommandMessageHeader, pBufferReadPoint, sizeof(PRESENTATION_DATA_VALUE_HEADER) );
			pBufferReadPoint += sizeof(PRESENTATION_DATA_VALUE_HEADER);
			RemainingBufferSize -= sizeof(PRESENTATION_DATA_VALUE_HEADER);
			pAssociation -> ReceivedCommandID = DICOM_CMD_UNSPECIFIED;
			if ( ( CommandMessageHeader.MessageControlHeader & CONTAINS_COMMAND_MESSAGE ) == 0 )
				pAssociation -> ReceivedCommandID = DICOM_CMD_DATA;
			bNeedsMoreBuffer = ( ( CommandMessageHeader.MessageControlHeader & LAST_MESSAGE_FRAGMENT ) == 0 );
			*pbNeedsMoreData = bNeedsMoreBuffer;
			AssociationSwapBytes( pAssociation, &CommandMessageHeader.PDVItemLength, 4 );
			RemainingDataSetLength = CommandMessageHeader.PDVItemLength - 2;

			PresentationContextID = CommandMessageHeader.PresentationContextID;
			_snprintf_s( Message, 1096, _TRUNCATE, "Presentation context:  %02X.", PresentationContextID );		// *[1] Replaced sprintf() with _snprintf_s.
			LogMessage( Message, MESSAGE_TYPE_DETAILS );

			if ( bNoError && bFirstBufferInSeries )
				{
				_snprintf_s( Message, 1096, _TRUNCATE, "Presentation context:  %02X.", PresentationContextID );	// *[1] Replaced sprintf() with _snprintf_s.
				LogMessage( Message, MESSAGE_TYPE_SUPPLEMENTARY );
				pAssociation -> ActivePresentationContextID = PresentationContextID;
				pPresentationContextItem = GetPresentationContextInfo( pAssociation, PresentationContextID );
				if ( pPresentationContextItem == 0 )
					{
					bNoError = FALSE;
					RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_NO_PRES_CONTEXT_FOUND );
					}
				else
					{
					TransferSyntax = GetTransferSyntaxForDicomElementParsing( (unsigned char)pPresentationContextItem -> AcceptedTransferSyntaxIndex );
					_snprintf_s( Message, 1096, _TRUNCATE,														// *[1] Replaced sprintf() with _snprintf_s.
									"Transfer syntax ID:  %d.", pPresentationContextItem -> AcceptedTransferSyntaxIndex );
					LogMessage( Message, MESSAGE_TYPE_SUPPLEMENTARY );
					}
				}

			if ( pAssociation -> ReceivedCommandID == DICOM_CMD_DATA )
				{
				if ( bNoError && bFirstBufferInSeries )
					{
					pGroup2Buffer = 0;
					Group2BufferSize = 0L;
					// Various Dicom sending programs in various circumstances do not identify the correct presentation
					// context for the current Dicom command.  Thus, it is necessary to examine the data and guess
					// the correct transfer syntax from among those offered during the association negotiation.
					if ( bNoError )
						{
						// Look at the beginning element in the dataset and guestimate its transfer syntax.
						FileDecodingPlan.DataSetTransferSyntax =
													GetConsistentTransferSyntaxFromBuffer( LITTLE_ENDIAN | EXPLICIT_VR, pBufferReadPoint );
						}
					if ( bNoError )
						{
						pSelectedPresentationContextItem = 0;
						pListElement = pAssociation -> ProposedPresentationContextList;
						bTransferSyntaxFound = FALSE;
						while ( bNoError && pListElement != 0 && !bTransferSyntaxFound )					// *[1] Added error check.
							{
							pTrialPresentationContextItem = (PRESENTATION_CONTEXT_ITEM*)pListElement -> pItem;
							bNoError = ( pTrialPresentationContextItem != 0 );								// *[1] Added error check to prevent potential NULL ptr dereference.
							if ( bNoError )																	// *[1]
								{
								// Default to selecting the first presentation context.  If there is only one,
								// it is "guaranteed" to match.
								if ( pSelectedPresentationContextItem == 0 )
									pSelectedPresentationContextItem = pTrialPresentationContextItem;
								// Check the current trial presentation context for image compression.
								pTransferSyntaxUID = TransferSyntaxLookupTable[ pSelectedPresentationContextItem -> AcceptedTransferSyntaxIndex ].pUIDString;
								_snprintf_s( Message, 1096, _TRUNCATE, "____Examining transfer syntax %s", pTransferSyntaxUID );		// *[1] Replaced sprintf() with _snprintf_s.
								LogMessage( Message, MESSAGE_TYPE_SUPPLEMENTARY );
								TransferSyntax = InterpretUniqueTransferSyntaxIdentifier( pTransferSyntaxUID );
								// If they are both compressed or else both uncompressed, they match.
								if ( ( ( TransferSyntax & UNCOMPRESSED ) != 0 &&
											( FileDecodingPlan.ImageDataTransferSyntax & UNCOMPRESSED ) != 0 ) ||
										( ( TransferSyntax & UNCOMPRESSED ) == 0 &&
											( FileDecodingPlan.ImageDataTransferSyntax & UNCOMPRESSED ) == 0 ) )
									{
									pSelectedPresentationContextItem = pTrialPresentationContextItem;
									_snprintf_s( Message, 1096, _TRUNCATE, "______Selecting transfer syntax %s", pTransferSyntaxUID );	// *[1] Replaced sprintf() with _snprintf_s.
									LogMessage( Message, MESSAGE_TYPE_SUPPLEMENTARY );
									bTransferSyntaxFound = TRUE;;		// Don't look any farther into the list.
									}
								}
							pListElement = pListElement -> pNextListElement;
							}
						// If the offered transfer syntax(es) disagree with the received data, reset to an
						// appropriate syntax, where possible.
						bNoError = ( pSelectedPresentationContextItem != 0 );								// *[1] Added error check to prevent potential NULL ptr dereference.
						if ( bNoError )																		// *[1]
							{
							if ( FileDecodingPlan.DataSetTransferSyntax != ( TransferSyntax & 0x00FF ) )
								{
								TransferSyntax &= ~0x00FF;
								TransferSyntax |= FileDecodingPlan.DataSetTransferSyntax & 0x00FF;
								if ( ( FileDecodingPlan.ImageDataTransferSyntax & UNCOMPRESSED ) != 0 )
									{
									if ( ( FileDecodingPlan.DataSetTransferSyntax & EXPLICIT_VR ) != 0 )
										pSelectedPresentationContextItem -> AcceptedTransferSyntaxIndex = LITTLE_ENDIAN_EXPLICIT_TRANSFER_SYNTAX;
									else
										pSelectedPresentationContextItem -> AcceptedTransferSyntaxIndex = LITTLE_ENDIAN_IMPLICIT_TRANSFER_SYNTAX;
									}
								}
							FileDecodingPlan.ImageDataTransferSyntax = TransferSyntax & 0xFF00;
							FileDecodingPlan.DataSetTransferSyntax = TransferSyntax & 0x00FF;
							pTransferSyntaxUID = TransferSyntaxLookupTable[ pSelectedPresentationContextItem -> AcceptedTransferSyntaxIndex ].pUIDString;
							TransferSyntaxValueLength = (unsigned long)strlen( pTransferSyntaxUID );
							FileDecodingPlan.nTransferSyntaxIndex = GetTransferSyntaxIndex( pTransferSyntaxUID,
																				(unsigned short)TransferSyntaxValueLength );
							}
						}
					if ( bNoError )
						bNoError = ComposeFileMetaInformation( pAssociation, pSelectedPresentationContextItem,
																			0, &pGroup2Buffer, &Group2BufferSize );
					if ( bNoError )
						{
						pExamDepositDirectory = pAssociation -> pProductOperation -> pOutputEndPoint -> Directory;
						// Begin the file copy with the newly-composed file meta information.
						LocalFileSpecification[ 0 ] = '\0';																// *[1] Eliminate call to strcpy.
						strncat_s( LocalFileSpecification, MAX_FILE_SPEC_LENGTH, pExamDepositDirectory, _TRUNCATE );	// *[1] Replaced strncat with strncat_s.
						if ( LocalFileSpecification[ strlen( LocalFileSpecification ) - 1 ] != '\\' )
							strncat_s( LocalFileSpecification, MAX_FILE_SPEC_LENGTH, "\\", _TRUNCATE );					// *[1] Replaced strcat with strncat_s.
						strncat_s( LocalFileSpecification, MAX_FILE_SPEC_LENGTH,
									pAssociation -> pCurrentAssociatedImageInfo -> CurrentDicomFileName, _TRUNCATE );	// *[1] Replaced strncat with strncat_s.
						strncat_s( LocalFileSpecification, MAX_FILE_SPEC_LENGTH, ".dcm", _TRUNCATE );					// *[1] Replaced strcat with strncat_s.
						_snprintf_s( Message, 1096, _TRUNCATE, "Receiving Dicom file %s", LocalFileSpecification );		// *[1] Replaced sprintf() with _snprintf_s.
						LogMessage( Message, MESSAGE_TYPE_SUPPLEMENTARY );
						pAssociation -> pCurrentAssociatedImageInfo -> pImageDataFile = OpenDicomFileForOutput( LocalFileSpecification );
						if ( pAssociation -> pCurrentAssociatedImageInfo -> pImageDataFile != 0 )
							{
							nBytesToBeWritten = Group2BufferSize;
							nBytesWritten = (long)fwrite( pGroup2Buffer, 1,
										(long)nBytesToBeWritten, pAssociation -> pCurrentAssociatedImageInfo -> pImageDataFile );
							if ( nBytesWritten != nBytesToBeWritten )
								{
								bNoError = FALSE;
								RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_TEMP_IMAGE_FILE_WRITE );
								}
							}
						else
							{
							bNoError = FALSE;
							RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_TEMP_IMAGE_FILE_OPEN );
							}
						if ( bNoError )
							{
							pAssociation -> pCurrentAssociatedImageInfo -> LocalImageFileSpecification[ 0 ] = '\0';		// *[1] Eliminate call to strcpy.
							strncat_s( pAssociation -> pCurrentAssociatedImageInfo -> LocalImageFileSpecification,
										MAX_FILE_SPEC_LENGTH, LocalFileSpecification, _TRUNCATE );						// *[1] Replaced strncat with strncat_s.
							}
						}
					if ( pGroup2Buffer != 0 )
						{
						free( pGroup2Buffer );
						pGroup2Buffer = 0;
						}
					}			// ... end if first buffer of image data.
				// Append the current buffer to the local image file.
				if ( bNoError )
					{
					if ( pAssociation -> pCurrentAssociatedImageInfo -> pImageDataFile != 0 )
						{
						nBytesToBeWritten = RemainingBufferSize;
						if ( (unsigned long)nBytesToBeWritten > RemainingPDULength - 6 )
							nBytesToBeWritten = RemainingPDULength - 6;
						nBytesWritten = (long)fwrite( pBufferReadPoint, 1,
									(long)nBytesToBeWritten, pAssociation -> pCurrentAssociatedImageInfo -> pImageDataFile );
						pBufferReadPoint += nBytesWritten;
						RemainingBufferSize -= nBytesWritten;
						if ( nBytesWritten != nBytesToBeWritten )
							{
							bNoError = FALSE;
							RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_TEMP_IMAGE_FILE_WRITE );
							}
						if ( bNoError )
							{
							*pPrevPDUBytesToBeRead = RemainingPDULength - 6 - nBytesWritten;
							_snprintf_s( Message, 1096, _TRUNCATE, "Leftover PDU = %d bytes.", *pPrevPDUBytesToBeRead );	// *[1] Replaced sprintf() with _snprintf_s.
							LogMessage( Message, MESSAGE_TYPE_DETAILS );
							}
						}
					else
						{
						bNoError = FALSE;
						RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_TEMP_IMAGE_FILE_CLOSED );
						}
					}
				if ( bNoError )
					{
					if ( !bNeedsMoreBuffer )
						{
						fclose( pAssociation -> pCurrentAssociatedImageInfo -> pImageDataFile );
						pAssociation -> pCurrentAssociatedImageInfo -> pImageDataFile = 0;
						if ( bNoError )
							{
							strncpy_s( TextField, MAX_LOGGING_STRING_LENGTH, pAssociation -> RemoteAE_Title, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
							TrimBlanks( TextField );
							LogMessage( "", MESSAGE_TYPE_NORMAL_LOG | MESSAGE_TYPE_NO_TIME_STAMP );
							_snprintf_s( Message, 1096, _TRUNCATE, "Successfully received from %s and stored:    %s", TextField,	// *[1] Replaced sprintf() with _snprintf_s.
										pAssociation -> pCurrentAssociatedImageInfo -> LocalImageFileSpecification );
							LogMessage( Message, MESSAGE_TYPE_NORMAL_LOG );
							MoveFileForAccess( pAssociation -> pCurrentAssociatedImageInfo -> LocalImageFileSpecification,
															pAssociation -> pCurrentAssociatedImageInfo -> CurrentDicomFileName );
							}
						}
					}
				else if ( pAssociation -> pCurrentAssociatedImageInfo -> pImageDataFile != 0 )
					{
					fclose( pAssociation -> pCurrentAssociatedImageInfo -> pImageDataFile );
					pAssociation -> pCurrentAssociatedImageInfo -> pImageDataFile = 0;
					}
				}
			else
				{
				// Parse the individual Dicom data elements associated with a received Dicom command.
				bNoError = ParseReceivedDataSetBuffer( pAssociation, pBufferReadPoint, RemainingDataSetLength );
				if ( bNoError && pAssociation -> ReceivedCommandID == DICOM_CMD_STORE )
					*pbNeedsMoreData = TRUE;
				RemainingBufferSize -= RemainingDataSetLength;
				pBufferReadPoint += RemainingDataSetLength;
				}
			}
		}			// ... end while unread buffer remains.
	if ( pAssociation -> pReceivedBuffer != 0 )
		{
		free( pAssociation -> pReceivedBuffer );
		pAssociation -> pReceivedBuffer = 0;
		pAssociation -> ReceivedBufferLength = 0L;
		}

	return bNoError;
}


// The bare file name does not include the extension.  This function moves the file over
// to the watch folder, after it has been written to the inbox and closed.  This move
// prevents contention for the file while it is still be recorded via the association.
void MoveFileForAccess( char *pExistingFilePath, char *pBareFileName )
{
	char			DestinationFileSpecification[ MAX_FILE_SPEC_LENGTH ];
	int				ResultCode;
	ENDPOINT		*pDestinationEndpoint;
	
	pDestinationEndpoint = &EndPointWatchFolder;
	if ( pDestinationEndpoint != 0 )
		{
		strncpy_s( DestinationFileSpecification, MAX_FILE_SPEC_LENGTH, pDestinationEndpoint -> Directory, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
		if ( DestinationFileSpecification[ strlen( DestinationFileSpecification ) - 1 ] != '\\' )
			strncat_s( DestinationFileSpecification, MAX_FILE_SPEC_LENGTH, "\\", _TRUNCATE );							// *[1] Replaced strcat with strncat_s.
		strncat_s( DestinationFileSpecification, MAX_FILE_SPEC_LENGTH, pBareFileName, _TRUNCATE );						// *[1] Replaced strncat with strncat_s.
		strncat_s( DestinationFileSpecification, MAX_FILE_SPEC_LENGTH, ".dcm", _TRUNCATE );								// *[1] Replaced strcat with strncat_s.
		ResultCode = rename( pExistingFilePath, DestinationFileSpecification );
		}
}


BOOL ParseAssociationAbortBuffer( DICOM_ASSOCIATION *pAssociation )
{
	BOOL										bNoError = TRUE;
	A_ABORT_BUFFER								*pAbortBuffer;
	long										RemainingBufferLength;
	char										TextMsg[ MAX_LOGGING_STRING_LENGTH ];

	RemainingBufferLength = pAssociation -> ReceivedBufferLength;
	pAbortBuffer = (A_ABORT_BUFFER*)pAssociation -> pReceivedBuffer;
	pAssociation -> ReceivedCommandID = DICOM_CMD_ABORT_ASSOCIATION;

	if ( pAssociation -> ReceivedBufferLength != sizeof(A_ABORT_BUFFER) )
		{
		_snprintf_s( TextMsg, MAX_LOGGING_STRING_LENGTH, _TRUNCATE,											// *[1] Replaced sprintf() with _snprintf_s.
						"%d bytes remained unread from the received association abort buffer.",
										pAssociation -> ReceivedBufferLength - sizeof(A_ABORT_BUFFER) );
		LogMessage( TextMsg, MESSAGE_TYPE_SUPPLEMENTARY );
		}
	else
		{
		if ( pAbortBuffer -> Source == ASSOC_ABORT_SOURCE_SERVICE_USER )
			LogMessage( "Association aborted by service user.", MESSAGE_TYPE_SUPPLEMENTARY );
		else if ( pAbortBuffer -> Source == ASSOC_ABORT_SOURCE_SERVICE_PROVIDER )
			switch ( pAbortBuffer -> Reason )
				{
				case ASSOC_ABORT_REASON2_NOT_GIVEN:					// No reason given.
					LogMessage( "Association aborted without a reason being given.", MESSAGE_TYPE_SUPPLEMENTARY );
					break;
				case ASSOC_ABORT_REASON2_UNRECOGNIZED_PDU:			// Unrecognized PDU.
					LogMessage( "Association aborted following receipt of an unrecognized PDU.", MESSAGE_TYPE_SUPPLEMENTARY );
					break;
				case ASSOC_ABORT_REASON2_UNEXPECTED_PDU:			// Unexpected PDU.
					LogMessage( "Association aborted following receipt of an unexpected PDU.", MESSAGE_TYPE_SUPPLEMENTARY );
					break;
				case ASSOC_ABORT_REASON2_UNRECOGNIZED_PDU_PARM:		// Unrecognized PDU parameter.
					LogMessage( "Association aborted following receipt of an unrecognized PDU parameter.", MESSAGE_TYPE_SUPPLEMENTARY );
					break;
				case ASSOC_ABORT_REASON2_UNEXPECTED_PDU_PARM:		// Unexpected PDU parameter.
					LogMessage( "Association aborted following receipt of an unexpected PDU parameter.", MESSAGE_TYPE_SUPPLEMENTARY );
					break;
				case ASSOC_ABORT_REASON2_INVALID_PARM_VALUE:		// Invalid PDU parameter value.
					LogMessage( "Association aborted following receipt of an invalid PDU parameter value.", MESSAGE_TYPE_SUPPLEMENTARY );
					break;
				}
		}
	if ( pAssociation -> pReceivedBuffer != 0 )
		{
		free( pAssociation -> pReceivedBuffer );
		pAssociation -> pReceivedBuffer = 0;
		pAssociation -> ReceivedBufferLength = 0L;
		}
	pAssociation -> EventIDReadyToBeProcessed = EVENT_RECEPTION_OF_ASSOCIATION_ABORT_REQUEST;
	pAssociation -> bSentMessageExpectsResponse = FALSE;

	return bNoError;
}





//____________________________________________________________________________________________________________________________________________________________
// Miscellaneous utility functions.
//

BOOL RegisterProposedAbstractSyntax( DICOM_ASSOCIATION *pAssociation, char *pAbstractSyntaxUID, unsigned short Length )
{
	BOOL							bAbstractSyntaxWasRecognized = TRUE;
	char							AbstractSyntaxUIDString[ 256 ];
	int								nAbstractSyntaxIndex;
	BOOL							bAbstractSyntaxFound;
	ABSTRACT_SYNTAX_TABLE_ENTRY		*pAbstractSyntaxTableEntry;

	memcpy( AbstractSyntaxUIDString,  pAbstractSyntaxUID, Length );
	AbstractSyntaxUIDString[ Length ] = '\0';
	TrimTrailingSpaces( AbstractSyntaxUIDString );
	nAbstractSyntaxIndex = 0;
	bAbstractSyntaxFound = FALSE;
	while ( !bAbstractSyntaxFound && nAbstractSyntaxIndex < NUMBER_OF_ABSTRACT_SYNTAX_IDS )
		{
		pAbstractSyntaxTableEntry = &AbstractSyntaxLookupTable[ nAbstractSyntaxIndex ];
		if ( strcmp( AbstractSyntaxUIDString, pAbstractSyntaxTableEntry -> pUIDString ) == 0 )
			bAbstractSyntaxFound = TRUE;
		else
			nAbstractSyntaxIndex++;
		}
	if ( bAbstractSyntaxFound && pAbstractSyntaxTableEntry -> bSupported )
		pAssociation -> PresentationContextSelector.pAbstractSyntaxDescriptor = pAbstractSyntaxTableEntry;
	else
		bAbstractSyntaxWasRecognized = FALSE;

	return bAbstractSyntaxWasRecognized;
}


// This function sequences through the list of acceptable transfer syntaxes associated with the currently registered
// abstract syntax.  If multiple transfer syntaxes are proposed for this abstract syntax, any previous acceptance
// is replaced with the current selection if it occurs earlier in the prioritized list.
BOOL RegisterProposedTransferSyntax( DICOM_ASSOCIATION *pAssociation, char *pTransferSyntaxUID, unsigned char PresentationContextID )
{
	BOOL							bTransferSyntaxWasAcceptable;
	BOOL							bTransferSyntaxWasFound;
	ABSTRACT_SYNTAX_TABLE_ENTRY		*pAbstractSyntaxTableEntry;
	unsigned long					nAvailableTransferSyntaxes;
	unsigned long					nTransferSyntax;
	unsigned long					TransferSyntaxIndex;
	unsigned long					SupportedTransferSyntaxIndex;

	bTransferSyntaxWasAcceptable = FALSE;
	pAbstractSyntaxTableEntry = pAssociation -> PresentationContextSelector.pAbstractSyntaxDescriptor;
	if (  pAbstractSyntaxTableEntry != 0 )
		{
		nAvailableTransferSyntaxes = pAbstractSyntaxTableEntry -> nSupportedTransferSyntaxCount;
		nTransferSyntax = 0;
		bTransferSyntaxWasFound = FALSE;
		TransferSyntaxIndex = GetTransferSyntaxIndex( pTransferSyntaxUID, (unsigned short)strlen( pTransferSyntaxUID ) );
		if ( TransferSyntaxIndex == NUMBER_OF_TRANSFER_SYNTAX_IDS )
			nTransferSyntax = nAvailableTransferSyntaxes;		// Terminate the loop.
		while ( !bTransferSyntaxWasFound && nTransferSyntax < nAvailableTransferSyntaxes )
			{
			SupportedTransferSyntaxIndex = (unsigned long)pAbstractSyntaxTableEntry -> pAcceptableTransferSyntaxes[ nTransferSyntax ];
			if ( SupportedTransferSyntaxIndex == pAssociation -> PresentationContextSelector.MostPreferableTransferSyntaxProposed )
				{
				bTransferSyntaxWasFound = TRUE;		// It doesn't matter that it hasn't been found yet,
													//  since a more preferred one has already be registered.
				}
			else if ( TransferSyntaxIndex == SupportedTransferSyntaxIndex )
				{
				pAssociation -> PresentationContextSelector.MostPreferableTransferSyntaxProposed = (unsigned char)TransferSyntaxIndex;
				pAssociation -> PresentationContextSelector.AcceptedPresentationContext = PresentationContextID;
				pAssociation -> PresentationContextSelector.AcceptedTransferSyntax = (unsigned char)TransferSyntaxIndex;
				bTransferSyntaxWasFound = TRUE;
				bTransferSyntaxWasAcceptable = TRUE;
				} 
			nTransferSyntax++;
			}
		}

	return bTransferSyntaxWasAcceptable;
}


char *GetSOPClassUID( int nSOPClassIndex )
{
	char				*pSOPClassUID;
	
	pSOPClassUID = AbstractSyntaxLookupTable[ nSOPClassIndex ].pUIDString;
	
	return pSOPClassUID;
}


unsigned short GetAbstractSyntaxIndex( char *pAbstractSyntaxUID, unsigned short Length )
{
	BOOL				bNoError = TRUE;
	int					nAbstractSyntaxIndex;
	BOOL				bAbstractSyntaxFound;
	unsigned long		nChars;
	char				AbstractSyntaxUIDString[ 256 ];

	memcpy( AbstractSyntaxUIDString,  pAbstractSyntaxUID, Length );
	AbstractSyntaxUIDString[ Length ] = '\0';
	TrimTrailingSpaces( AbstractSyntaxUIDString );
	nChars = (unsigned long)strlen( AbstractSyntaxUIDString );
	nAbstractSyntaxIndex = 0;
	bAbstractSyntaxFound = FALSE;
	while ( !bAbstractSyntaxFound && nAbstractSyntaxIndex < NUMBER_OF_ABSTRACT_SYNTAX_IDS )
		{
		if ( strncmp( AbstractSyntaxUIDString, AbstractSyntaxLookupTable[ nAbstractSyntaxIndex ].pUIDString, nChars ) == 0 )
			bAbstractSyntaxFound = TRUE;
		else
			nAbstractSyntaxIndex++;
		}
	if ( !bAbstractSyntaxFound )
		{
		nAbstractSyntaxIndex = NUMBER_OF_ABSTRACT_SYNTAX_IDS;	// Set to an impossibly large number.
		bNoError = FALSE;
		}

	return (unsigned short)nAbstractSyntaxIndex;
}


PRESENTATION_CONTEXT_ITEM *GetPresentationContextInfo( DICOM_ASSOCIATION *pAssociation, unsigned char PresentationContextID )
{
	LIST_ELEMENT					*pListElement;
	PRESENTATION_CONTEXT_ITEM		*pPresentationContextItem;
	BOOL							bMatchingPresContextFound;

	bMatchingPresContextFound = FALSE;
	pPresentationContextItem = 0;
	pListElement = pAssociation -> ProposedPresentationContextList;
	while ( pListElement != 0 && !bMatchingPresContextFound )
		{
		pPresentationContextItem = (PRESENTATION_CONTEXT_ITEM*)pListElement -> pItem;
		if ( pPresentationContextItem ->AcceptedPresentationContextID == PresentationContextID )
			bMatchingPresContextFound = TRUE;
		else
			pListElement = pListElement -> pNextListElement;
		}

	return pPresentationContextItem;
}


char *GetTransferSyntaxUID( int nTransferSyntaxIndex )
{
	char				*pTransferSyntaxUID;
	
	pTransferSyntaxUID = TransferSyntaxLookupTable[ nTransferSyntaxIndex ].pUIDString;
	
	return pTransferSyntaxUID;
}


unsigned long GetTransferSyntaxBitCode( int nTransferSyntaxIndex )
{
	unsigned long		TransferSyntaxBitCode;
	
	TransferSyntaxBitCode = TransferSyntaxLookupTable[ nTransferSyntaxIndex ].BitCode;
	
	return TransferSyntaxBitCode;
}


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


BOOL ComposeFileMetaInformation( DICOM_ASSOCIATION *pAssociation,
									PRESENTATION_CONTEXT_ITEM *pPresentationContextItem,
									DICOM_HEADER_SUMMARY *pDicomHeaderSummary, char **ppGroup2Buffer, unsigned long *pBufferSize )
{
	BOOL									bNoError = TRUE;
	FILE_META_INFO_GROUP_LENGTH				GroupLengthElement;
	FILE_META_INFO_VERSION					MetaInfoVersionElement;
	FILE_META_INFO_HEADER_EXPLICIT_VR		BufferElement;
	FILE_META_INFO_HEADER_EXPLICIT_VR_OB	BufferElement_OB;

	unsigned long					SOPClassValueLength;
	unsigned long					SOPInstanceValueLength;
	unsigned long					TransferSyntaxValueLength;
	unsigned long					ImplementationClassValueLength;
	unsigned long					ImplementationVersionValueLength;
	unsigned long					SourceApplicationValueLength;
	unsigned long					DestinationApplicationValueLength;

	BOOL							bSOPClassUIDLengthIsOdd;
	BOOL							bSOPInstanceUIDLengthIsOdd;
	BOOL							bTransferSyntaxLengthIsOdd;
	BOOL							bImplementationClassUIDLengthIsOdd;
	BOOL							bImplementationVersionLengthIsOdd;
	BOOL							bSourceApplicationLengthIsOdd;
	BOOL							bDestinationApplicationLengthIsOdd;

	unsigned long					TotalSOPClassElementSize;
	unsigned long					TotalSOPInstanceElementSize;
	unsigned long					TotalTransferSyntaxElementSize;
	unsigned long					TotalImplementationClassElementSize;
	unsigned long					TotalImplementationVersionElementSize;
	unsigned long					TotalSourceApplicationElementSize;
	unsigned long					TotalDestinationApplicationElementSize;
	unsigned long					TotalBufferSize;

	char							*pBuffer;
	char							*pBufferInsertPoint;
	char							*pSOPClassUID;
	char							*pSOPInstanceUID;
	char							*pTransferSyntaxUID;
	char							*pImplementationClassUID;
	char							*pImplementationVersionName;
	char							*pSourceApplication;
	char							*pDestinationApplication;
	
	char							Message[ MAX_LOGGING_STRING_LENGTH ];

	// Determine the overall buffer length.
	pSOPClassUID = GetSOPClassUID( pAssociation -> nAcceptedAbstractSyntax );
	SOPClassValueLength = (unsigned long)strlen( pSOPClassUID );
	bSOPClassUIDLengthIsOdd = ( ( SOPClassValueLength & 0x00000001 ) != 0 );
	if ( bSOPClassUIDLengthIsOdd )
		SOPClassValueLength++;					// Make the value length an even number of bytes.
	TotalSOPClassElementSize = SOPClassValueLength + sizeof(FILE_META_INFO_HEADER_EXPLICIT_VR);

	pSOPInstanceUID = pAssociation -> pCurrentAssociatedImageInfo -> CurrentDicomFileName;
	SOPInstanceValueLength = (unsigned long)strnlen_s( pSOPInstanceUID, MAX_FILE_SPEC_LENGTH );			// *[1] Ensure string is null terminated.
	bSOPInstanceUIDLengthIsOdd = ( ( SOPInstanceValueLength & 0x00000001 ) != 0 );
	if ( bSOPInstanceUIDLengthIsOdd )
		SOPInstanceValueLength++;				// Make the value length an even number of bytes.
	TotalSOPInstanceElementSize = SOPInstanceValueLength + sizeof(FILE_META_INFO_HEADER_EXPLICIT_VR);

	pTransferSyntaxUID = TransferSyntaxLookupTable[ pPresentationContextItem -> AcceptedTransferSyntaxIndex ].pUIDString;
	TransferSyntaxValueLength = (unsigned long)strlen( pTransferSyntaxUID );
	bTransferSyntaxLengthIsOdd = ( ( TransferSyntaxValueLength & 0x00000001 ) != 0 );
	if ( bTransferSyntaxLengthIsOdd )
		TransferSyntaxValueLength++;			// Make the value length an even number of bytes.
	TotalTransferSyntaxElementSize = TransferSyntaxValueLength + sizeof(FILE_META_INFO_HEADER_EXPLICIT_VR);

	_snprintf_s( Message, MAX_LOGGING_STRING_LENGTH, _TRUNCATE, "____Saving transfer syntax %s", pTransferSyntaxUID );	// *[1] Replaced sprintf() with _snprintf_s.
	LogMessage( Message, MESSAGE_TYPE_SUPPLEMENTARY );

	pImplementationClassUID = pAssociation -> pImplementationClassUID;
	ImplementationClassValueLength = (unsigned long)strlen( pImplementationClassUID );
	bImplementationClassUIDLengthIsOdd = ( ( ImplementationClassValueLength & 0x00000001 ) != 0 );
	if ( bImplementationClassUIDLengthIsOdd )
		ImplementationClassValueLength++;		// Make the value length an even number of bytes.
	TotalImplementationClassElementSize = ImplementationClassValueLength + sizeof(FILE_META_INFO_HEADER_EXPLICIT_VR);

	pImplementationVersionName = pAssociation -> pImplementationVersionName;
	if ( pImplementationVersionName == 0 || strlen( pImplementationVersionName ) == 0 )
		{
		ImplementationVersionValueLength = (unsigned long)strlen( pTransferServiceImplementationVersionName );
		pAssociation -> pImplementationVersionName  = (char*)malloc( ImplementationVersionValueLength + 1 );
		strncpy_s( pAssociation -> pImplementationVersionName,
					ImplementationVersionValueLength + 1, pTransferServiceImplementationVersionName, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
		pImplementationVersionName = pAssociation -> pImplementationVersionName;
		}
	ImplementationVersionValueLength = (unsigned long)strlen( pImplementationVersionName );
	bImplementationVersionLengthIsOdd = ( ( ImplementationVersionValueLength & 0x00000001 ) != 0 );
	if ( bImplementationVersionLengthIsOdd )
		ImplementationVersionValueLength++;		// Make the value length an even number of bytes.
	TotalImplementationVersionElementSize = ImplementationVersionValueLength + sizeof(FILE_META_INFO_HEADER_EXPLICIT_VR);

	pSourceApplication = pAssociation -> RemoteAE_Title;
	SourceApplicationValueLength = (unsigned long)strlen( pSourceApplication );
	bSourceApplicationLengthIsOdd = ( ( SourceApplicationValueLength & 0x00000001 ) != 0 );
	if ( bSourceApplicationLengthIsOdd )
		SourceApplicationValueLength++;			// Make the value length an even number of bytes.
	TotalSourceApplicationElementSize = SourceApplicationValueLength + sizeof(FILE_META_INFO_HEADER_EXPLICIT_VR);

	pDestinationApplication = pAssociation -> LocalAE_Title;
	DestinationApplicationValueLength = (unsigned long)strlen( pDestinationApplication );
	bDestinationApplicationLengthIsOdd = ( ( DestinationApplicationValueLength & 0x00000001 ) != 0 );
	if ( bDestinationApplicationLengthIsOdd )
		DestinationApplicationValueLength++;			// Make the value length an even number of bytes.
	TotalDestinationApplicationElementSize = DestinationApplicationValueLength + sizeof(FILE_META_INFO_HEADER_EXPLICIT_VR_OB);

	TotalBufferSize = sizeof(FILE_META_INFO_GROUP_LENGTH) + sizeof(FILE_META_INFO_VERSION) + TotalSOPClassElementSize +
						TotalSOPInstanceElementSize + TotalTransferSyntaxElementSize + TotalImplementationClassElementSize +
						TotalImplementationVersionElementSize + TotalSourceApplicationElementSize +
						TotalDestinationApplicationElementSize;

	// Allocate and fill the message buffer.
	pBuffer = (char*)malloc( TotalBufferSize );
	pBufferInsertPoint = pBuffer;
	if ( pBuffer != 0 )
		{
		pBufferInsertPoint = pBuffer;
		// Insert the Group Length dicom element into the buffer.
		GroupLengthElement.Group = 0x0002;
		GroupLengthElement.Element = 0x0000;
		memcpy( GroupLengthElement.ValueRepresentation, "UL", 2 );
		GroupLengthElement.ValueLength = 4;
		GroupLengthElement.Value = TotalBufferSize - sizeof(FILE_META_INFO_GROUP_LENGTH);
		memcpy( pBufferInsertPoint, (char*)&GroupLengthElement, sizeof(FILE_META_INFO_GROUP_LENGTH) );
		pBufferInsertPoint += sizeof(FILE_META_INFO_GROUP_LENGTH);
		if ( pDicomHeaderSummary != 0 )
			{
			pDicomHeaderSummary -> MetadataGroupLength = (unsigned long*)malloc( GroupLengthElement.ValueLength );
			if ( pDicomHeaderSummary -> MetadataGroupLength != 0 )
				*pDicomHeaderSummary -> MetadataGroupLength = GroupLengthElement.Value;
			}

		// Insert the file meta information dicom element into the buffer.
		MetaInfoVersionElement.Group = 0x0002;
		MetaInfoVersionElement.Element = 0x0001;
		memcpy( MetaInfoVersionElement.ValueRepresentation, "OB", 2 );
		MetaInfoVersionElement.Reserved = 0x0000;
		MetaInfoVersionElement.ValueLength = 4L;
		memcpy( (char*)&MetaInfoVersionElement.Value, "0.1 ", 4 );
		memcpy( pBufferInsertPoint, (char*)&MetaInfoVersionElement, sizeof(FILE_META_INFO_VERSION) );
		pBufferInsertPoint += sizeof(FILE_META_INFO_VERSION);
		if ( pDicomHeaderSummary != 0 )
			{
			pDicomHeaderSummary -> FileMetaInformationVersion = (unsigned char*)malloc( MetaInfoVersionElement.ValueLength + 1 );
			if ( pDicomHeaderSummary -> FileMetaInformationVersion != 0 )
				{
				pDicomHeaderSummary -> FileMetaInformationVersion[ 0 ] = '\0';											// *[1] Eliminate call to strcpy.
				strncat_s( (char*)pDicomHeaderSummary -> FileMetaInformationVersion,
							MetaInfoVersionElement.ValueLength + 1, (char*)&MetaInfoVersionElement.Value, _TRUNCATE );	// *[1] Replaced strncat with strncat_s.
				}
			}

		// Insert the media storage SOP class dicom element into the buffer.
		BufferElement.Group = 0x0002;
		BufferElement.Element = 0x0002;
		memcpy( BufferElement.ValueRepresentation, "UI", 2 );
		BufferElement.ValueLength = (unsigned short)SOPClassValueLength;
		memcpy( pBufferInsertPoint, (char*)&BufferElement, sizeof(FILE_META_INFO_HEADER_EXPLICIT_VR) );
		pBufferInsertPoint += sizeof(FILE_META_INFO_HEADER_EXPLICIT_VR);
		memcpy( pBufferInsertPoint, pSOPClassUID, SOPClassValueLength );
		if ( bSOPClassUIDLengthIsOdd )
			*( pBufferInsertPoint + SOPClassValueLength - 1 ) = ' ';	// Pad value with a space.
		pBufferInsertPoint += SOPClassValueLength;
		if ( pDicomHeaderSummary != 0 )
			{
			pDicomHeaderSummary -> MediaStorageSOPClassUID = (char*)malloc( SOPClassValueLength + 1 );
			if ( pDicomHeaderSummary -> MediaStorageSOPClassUID != 0 )
				strncpy_s( pDicomHeaderSummary -> MediaStorageSOPClassUID, SOPClassValueLength + 1, pSOPClassUID, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
			}

		// Insert the media storage SOP instance dicom element into the buffer.
		BufferElement.Group = 0x0002;
		BufferElement.Element = 0x0003;
		memcpy( BufferElement.ValueRepresentation, "UI", 2 );
		BufferElement.ValueLength = (unsigned short)SOPInstanceValueLength;
		memcpy( pBufferInsertPoint, (char*)&BufferElement, sizeof(FILE_META_INFO_HEADER_EXPLICIT_VR) );
		pBufferInsertPoint += sizeof(FILE_META_INFO_HEADER_EXPLICIT_VR);
		memcpy( pBufferInsertPoint, pSOPInstanceUID, SOPInstanceValueLength );
		if ( bSOPInstanceUIDLengthIsOdd )
			*( pBufferInsertPoint + SOPInstanceValueLength - 1 ) = ' ';	// Pad value with a space.
		pBufferInsertPoint += SOPInstanceValueLength;
		if ( pDicomHeaderSummary != 0 )
			{
			pDicomHeaderSummary -> MediaStorageSOPInstanceUID = (char*)malloc( SOPInstanceValueLength + 1 );
			if ( pDicomHeaderSummary -> MediaStorageSOPInstanceUID != 0 )
				strncpy_s( pDicomHeaderSummary -> MediaStorageSOPInstanceUID, SOPInstanceValueLength + 1, pSOPInstanceUID, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
			}

		// Insert the transfer syntax dicom element into the buffer.
		BufferElement.Group = 0x0002;
		BufferElement.Element = 0x0010;
		memcpy( BufferElement.ValueRepresentation, "UI", 2 );
		BufferElement.ValueLength = (unsigned short)TransferSyntaxValueLength;
		memcpy( pBufferInsertPoint, (char*)&BufferElement, sizeof(FILE_META_INFO_HEADER_EXPLICIT_VR) );
		pBufferInsertPoint += sizeof(FILE_META_INFO_HEADER_EXPLICIT_VR);
		memcpy( pBufferInsertPoint, pTransferSyntaxUID, TransferSyntaxValueLength );
		if ( bTransferSyntaxLengthIsOdd )
			*( pBufferInsertPoint + TransferSyntaxValueLength - 1 ) = ' ';	// Pad value with a space.
		pBufferInsertPoint += TransferSyntaxValueLength;
		if ( pDicomHeaderSummary != 0 && pDicomHeaderSummary -> TransferSyntaxUniqueIdentifier == 0 )
			{
			pDicomHeaderSummary -> TransferSyntaxUniqueIdentifier = (char*)malloc( TransferSyntaxValueLength + 1 );
			if ( pDicomHeaderSummary -> TransferSyntaxUniqueIdentifier != 0 )
				strncpy_s( pDicomHeaderSummary -> TransferSyntaxUniqueIdentifier, TransferSyntaxValueLength + 1, pTransferSyntaxUID, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
			}

		// Insert the implementation class UID dicom element into the buffer.
		BufferElement.Group = 0x0002;
		BufferElement.Element = 0x0012;
		memcpy( BufferElement.ValueRepresentation, "UI", 2 );
		BufferElement.ValueLength = (unsigned short)ImplementationClassValueLength;
		memcpy( pBufferInsertPoint, (char*)&BufferElement, sizeof(FILE_META_INFO_HEADER_EXPLICIT_VR) );
		pBufferInsertPoint += sizeof(FILE_META_INFO_HEADER_EXPLICIT_VR);
		memcpy( pBufferInsertPoint, pImplementationClassUID, ImplementationClassValueLength );
		if ( bImplementationClassUIDLengthIsOdd )
			*( pBufferInsertPoint + ImplementationClassValueLength - 1 ) = ' ';	// Pad value with a space.
		pBufferInsertPoint += ImplementationClassValueLength;
		if ( pDicomHeaderSummary != 0 )
			{
			pDicomHeaderSummary -> ImplementationClassUID = (char*)malloc( ImplementationClassValueLength + 1 );
			if ( pDicomHeaderSummary -> ImplementationClassUID != 0 )
				strncpy_s( pDicomHeaderSummary -> ImplementationClassUID, ImplementationClassValueLength + 1, pImplementationClassUID, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
			}

		// Insert the implementation version name dicom element into the buffer.
		BufferElement.Group = 0x0002;
		BufferElement.Element = 0x0013;
		memcpy( BufferElement.ValueRepresentation, "SH", 2 );
		BufferElement.ValueLength = (unsigned short)ImplementationVersionValueLength;
		memcpy( pBufferInsertPoint, (char*)&BufferElement, sizeof(FILE_META_INFO_HEADER_EXPLICIT_VR) );
		pBufferInsertPoint += sizeof(FILE_META_INFO_HEADER_EXPLICIT_VR);
		memcpy( pBufferInsertPoint, pImplementationVersionName, ImplementationVersionValueLength );
		if ( bImplementationVersionLengthIsOdd )
			*( pBufferInsertPoint + ImplementationVersionValueLength - 1 ) = ' ';	// Pad value with a space.
		pBufferInsertPoint += ImplementationVersionValueLength;
		if ( pDicomHeaderSummary != 0 )
			{
			pDicomHeaderSummary -> ImplementationVersionName = (char*)malloc( ImplementationVersionValueLength + 1 );
			if ( pDicomHeaderSummary -> ImplementationVersionName != 0 )
				strncpy_s( pDicomHeaderSummary -> ImplementationVersionName,
							ImplementationVersionValueLength + 1, pImplementationVersionName, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
			}

		// Insert the source application AE-title dicom element into the buffer.
		BufferElement.Group = 0x0002;
		BufferElement.Element = 0x0016;
		memcpy( BufferElement.ValueRepresentation, "AE", 2 );
		BufferElement.ValueLength = (unsigned short)SourceApplicationValueLength;
		memcpy( pBufferInsertPoint, (char*)&BufferElement, sizeof(FILE_META_INFO_HEADER_EXPLICIT_VR) );
		pBufferInsertPoint += sizeof(FILE_META_INFO_HEADER_EXPLICIT_VR);
		memcpy( pBufferInsertPoint, pSourceApplication, SourceApplicationValueLength );
		if ( bSourceApplicationLengthIsOdd )
			*( pBufferInsertPoint + SourceApplicationValueLength - 1 ) = ' ';	// Pad value with a space.
		pBufferInsertPoint += SourceApplicationValueLength;
		if ( pDicomHeaderSummary != 0 )
			{
			pDicomHeaderSummary -> SourceAE_TITLE = (char*)malloc( SourceApplicationValueLength + 1 );
			if ( pDicomHeaderSummary -> SourceAE_TITLE != 0 )
				strncpy_s( pDicomHeaderSummary -> SourceAE_TITLE, SourceApplicationValueLength + 1, pSourceApplication, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
			}

		// Insert the destination application AE-title dicom element into the buffer
		// as the PrivateInformation element.
		BufferElement_OB.Group = 0x0002;
		BufferElement_OB.Element = 0x0102;
		memcpy( BufferElement_OB.ValueRepresentation, "OB", 2 );
		BufferElement_OB.Reserved = 0x0000;
		BufferElement_OB.ValueLength = (unsigned short)DestinationApplicationValueLength;
		memcpy( pBufferInsertPoint, (char*)&BufferElement_OB, sizeof(FILE_META_INFO_HEADER_EXPLICIT_VR_OB) );
		pBufferInsertPoint += sizeof(FILE_META_INFO_HEADER_EXPLICIT_VR_OB);
		memcpy( pBufferInsertPoint, pDestinationApplication, DestinationApplicationValueLength );
		if ( bDestinationApplicationLengthIsOdd )
			*( pBufferInsertPoint + DestinationApplicationValueLength - 1 ) = ' ';	// Pad value with a space.
		pBufferInsertPoint += DestinationApplicationValueLength;
		if ( pDicomHeaderSummary != 0 )
			{
			pDicomHeaderSummary -> DestinationAE_TITLE = (char*)malloc( DestinationApplicationValueLength + 1 );
			if ( pDicomHeaderSummary -> DestinationAE_TITLE != 0 )
				strncpy_s( pDicomHeaderSummary -> DestinationAE_TITLE,
							DestinationApplicationValueLength + 1, pDestinationApplication, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
			}
		}
	else
		{
		bNoError = FALSE;
		RespondToError( MODULE_DICOMASSOC, DICOMASSOC_ERROR_INSUFFICIENT_MEMORY );
		}
	if ( bNoError )
		{
		*ppGroup2Buffer = pBuffer;
		*pBufferSize = TotalBufferSize;
		}
	else
		{
		*ppGroup2Buffer = 0;
		*pBufferSize = 0L;
		}

	return bNoError;
}


