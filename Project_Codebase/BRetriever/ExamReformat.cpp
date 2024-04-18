// ExamReformat.cpp : Implements the data structures and functions related to
//	the conversion of an image from one of the Dicom image formats into
//  the PNG format readable by BViewer.
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
//	*[1] 03/07/2024 by Tom Atwood
//		Fixed security issues.
//
//
#include "Module.h"
#include "ReportStatus.h"
#include "ServiceMain.h"
#include "Dicom.h"
#include "Configuration.h"
#include "Operation.h"
#include "ProductDispatcher.h"
#include "ExamReformat.h"
#include "Exam.h"


extern TRANSFER_SERVICE				TransferService;


//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO					ExamReformatModuleInfo = { MODULE_REFORMAT, "Exam Reformat Module", InitExamReformatModule, CloseExamReformatModule };


static ERROR_DICTIONARY_ENTRY	ExamReformatErrorCodes[] =
			{
				{ REFORMAT_ERROR_INSUFFICIENT_MEMORY		, "An error occurred allocating a memory block for data storage."					},
				{ REFORMAT_ERROR_EXTRACTION					, "An error occurred while extracting the image from the Dicom file."				},
				{ REFORMAT_ERROR_JPEG_2000					, "A JPEG-2000 image was received.  JPEG-2000 is not currently supported."			},
				{ REFORMAT_ERROR_JPEG_CORRUPTION			, "A potentially corrupt JPEG image was received.  An error occurred decoding it."	},
				{ REFORMAT_ERROR_IMAGE_CONVERT_SEEK			, "The pixel data for the image to be converted could not be located." },
				{ REFORMAT_ERROR_PNG_WRITE					, "An error occurred writing image data to the Dicom file." },
				{ 0											, NULL }
			};


static ERROR_DICTIONARY_MODULE		ExamReformatStatusErrorDictionary =
										{
										MODULE_REFORMAT,
										ExamReformatErrorCodes,
										REFORMAT_ERROR_DICT_LENGTH,
										0
										};


// This function must be called before any other function in this module.
void InitExamReformatModule()
{
	LinkModuleToList( &ExamReformatModuleInfo );
	RegisterErrorDictionary( &ExamReformatStatusErrorDictionary );
}


void CloseExamReformatModule()
{
}



static char			*pRetryMsg = "Try sending or importing the file again.";


void NotifyUserOfImageExtractionError( unsigned long ErrorCode, PRODUCT_QUEUE_ITEM *pProductItem )
{
	USER_NOTIFICATION		UserNoticeDescriptor;

	RespondToError( MODULE_REFORMAT, ErrorCode );
	strncpy_s( UserNoticeDescriptor.Source, 16, TransferService.ServiceName, _TRUNCATE );								// *[1] Replaced strcpy with strncpy_s.
	UserNoticeDescriptor.ModuleCode = MODULE_REFORMAT;
	UserNoticeDescriptor.ErrorCode = ErrorCode;
	UserNoticeDescriptor.TypeOfUserResponseSupported = USER_RESPONSE_TYPE_ERROR | USER_RESPONSE_TYPE_CONTINUE;
	UserNoticeDescriptor.UserNotificationCause = USER_NOTIFICATION_CAUSE_PRODUCT_PROCESSING_ERROR;
	UserNoticeDescriptor.UserResponseCode = 0L;
	if ( pProductItem != 0 && strlen( pProductItem -> Description ) > 0 )
		_snprintf_s( UserNoticeDescriptor.NoticeText, MAX_FILE_SPEC_LENGTH, _TRUNCATE,									// *[1] Replaced sprintf() with _snprintf_s.
						"Image extraction failed for\n%s\n%s\n\n",
													pProductItem -> Description, pProductItem -> SourceFileName );
	else
		strncpy_s( UserNoticeDescriptor.NoticeText,
					MAX_FILE_SPEC_LENGTH, "The BRetriever service encountered an error:\n\n", _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
	
	switch ( ErrorCode )
		{
		case REFORMAT_ERROR_EXTRACTION:
			strncat_s( UserNoticeDescriptor.NoticeText,
						MAX_FILE_SPEC_LENGTH, "The image information could not be decoded.", _TRUNCATE );				// *[1] Replaced strcat with strncat_s.
			strncpy_s( UserNoticeDescriptor.SuggestedActionText, MAX_CFG_STRING_LENGTH, pRetryMsg, _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
			break;
		case REFORMAT_ERROR_JPEG_2000:
			strncat_s( UserNoticeDescriptor.NoticeText,
						MAX_FILE_SPEC_LENGTH, "JPEG-2000 image decoding is not\ncurrently supported.", _TRUNCATE );		// *[1] Replaced strcat with strncat_s. );
			strncpy_s( UserNoticeDescriptor.SuggestedActionText,
						MAX_CFG_STRING_LENGTH, "Ask the source to send it uncompressed.", _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
			break;
		}
	UserNoticeDescriptor.TextLinesRequired = 8;
	SubmitUserNotification( &UserNoticeDescriptor );
}


BOOL PerformLocalFileReformat( PRODUCT_QUEUE_ITEM *pProductItem, PRODUCT_OPERATION *pProductOperation )
{
	BOOL					bNoError = TRUE;
	char					*pDestFileName;
	char					DestFileSpec[ MAX_FILE_SPEC_LENGTH ];
	char					*pChar;
	char					TextLine[ 1096 ];
	char					*pTransferSyntaxUID;
	EXAM_INFO				*pExamInfo;
	DICOM_HEADER_SUMMARY	*pDicomHeader;
	char					*pExamDepositDirectory;

	pExamInfo = (EXAM_INFO*)pProductItem -> pProductInfo;
	pDestFileName = pProductItem -> DestinationFileName;
	pExamDepositDirectory = pProductOperation -> pOutputEndPoint -> Directory;
	pDicomHeader = pExamInfo -> pDicomInfo;
	strncpy_s( DestFileSpec, MAX_FILE_SPEC_LENGTH, pExamDepositDirectory, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
	if ( DestFileSpec[ strlen( DestFileSpec ) - 1 ] != '\\' )
		strncat_s( DestFileSpec, MAX_FILE_SPEC_LENGTH, "\\", _TRUNCATE );							// *[1] Replaced strcat with strncat_s.
	strncat_s( DestFileSpec, MAX_FILE_SPEC_LENGTH, pDestFileName, _TRUNCATE );						// *[1] Replaced strcat with strncat_s.
	pChar = strrchr( DestFileSpec, '.' );
	if ( pChar != 0 )
		strncpy_s( pChar, 5, ".png", _TRUNCATE );													// *[1] Replaced strcpy with strncpy_s.
	_snprintf_s( TextLine, 1096, _TRUNCATE, "Extracting Dicom image from %s", pProductItem -> SourceFileName );	// *[1] Replaced sprintf() with _snprintf_s.
	LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
	if ( pDicomHeader != 0 )
		{
		pTransferSyntaxUID = pDicomHeader -> TransferSyntaxUniqueIdentifier;
		if ( strcmp( pTransferSyntaxUID, "1.2.840.10008.1.2.4.90" ) == 0 )
			{
			bNoError = FALSE;
			pProductItem -> ModuleWhereErrorOccurred = MODULE_REFORMAT;
			pProductItem -> FirstErrorCode = REFORMAT_ERROR_JPEG_2000;
			NotifyUserOfImageExtractionError( REFORMAT_ERROR_JPEG_2000, pProductItem );
			}
		}
	if ( bNoError )
		{
		bNoError = OutputPNGImage( DestFileSpec, pDicomHeader );
		if ( bNoError )
			{
			_snprintf_s( TextLine, 1096, _TRUNCATE, "Dicom image extracted as %s", DestFileSpec );			// *[1] Replaced sprintf() with _snprintf_s.
			LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
			}
		else
			{
			pProductItem -> ModuleWhereErrorOccurred = MODULE_REFORMAT;
			pProductItem -> FirstErrorCode = REFORMAT_ERROR_EXTRACTION;
			NotifyUserOfImageExtractionError( REFORMAT_ERROR_EXTRACTION, pProductItem );
			_snprintf_s( TextLine, 1096, _TRUNCATE, "Error extracting Dicom image as %s", DestFileSpec );	// *[1] Replaced sprintf() with _snprintf_s.
			LogMessage( TextLine, MESSAGE_TYPE_ERROR );
			}
		}

	return bNoError;
}

static BOOL			bAppendCalibrationData = TRUE;

// Before calling this function, the Dicom Header structure must be loaded with the buffer address
// of the beginning of the pixel data, which is the address of element (7fe0, 10).
BOOL OutputPNGImage( char* pDestFileSpec, DICOM_HEADER_SUMMARY *pDicomHeader )
{
	BOOL					bNoError = TRUE;
	FILE					*pOutputImageFile;
	FILE_STATUS				FileStatus = FILE_STATUS_OK;
	long					nBytesRemainingInImageItem;
	unsigned short			nImageBitDepth;
	char					*IndentationString = "";
	long					nBytesWritten;

	#define MAX_READ_BUFFER_SIZE		0x10000

	pOutputImageFile = 0;
	bNoError = StorageCapacityIsAdequate();
	if ( bNoError )
		{
		// Open the Dicom file from which the image is to be extracted.
		// Open the extracted image output file.
		pOutputImageFile = fopen( pDestFileSpec, "wb" );
		bNoError = ( pOutputImageFile != 0 );
		}

	if ( bNoError )
		{
		// Force compliance with Dicom doc 3, C.8.4.7, but don't change nImageBitDepth = *pDicomHeader -> BitsStored,
		// which is used below to encode the image into PNG.  The CalibrationInfo.BitsStored must be forced to the
		// bits allocated in order for the modality and VOI lut logic to work in BViewer.  What a mess is this
		// Dicom "standard".
		// Evidently a 16-bit NM image can be encoded as lossy JPEG 12-bit by selecting the most significant 12 bits
		// of each pixel.  Bits Stored has to be set to 12, even though this is illegal for NM, in order to provide
		// for the correct decoding of the JPEG image.  Following this decoding, you have to treat the image as if
		// it still retained 16 bits per pixel.  Otherwise the LUTs don't work.
		if ( _stricmp( pDicomHeader -> Modality, "NM" ) == 0 )
			pDicomHeader -> CalibrationInfo.BitsStored = pDicomHeader -> CalibrationInfo.BitsAllocated;
		// For any modality expressed by the X-Ray image IOD, the Pixel Representation is required to be 0 (unsigned).
		if ( _stricmp( pDicomHeader -> Modality, "CR" ) == 0 || _stricmp( pDicomHeader -> Modality, "DX" ) == 0 ||
					_stricmp( pDicomHeader -> Modality, "SC" ) == 0 )
			pDicomHeader -> CalibrationInfo.bPixelValuesAreSigned = FALSE;
		// Set the label to indicate that the "PNG" file starts with calibration data.
		memcpy( pDicomHeader -> CalibrationInfo.Label, "BVCALIB2", 8 );
		// Record the image calibration information.
		if ( bAppendCalibrationData )
			{
			nBytesWritten = (long)fwrite( &pDicomHeader -> CalibrationInfo, 1,
											sizeof( IMAGE_CALIBRATION_INFO ), pOutputImageFile );
			bNoError = ( nBytesWritten == sizeof( IMAGE_CALIBRATION_INFO ) );
			}
		}

	if ( bNoError )
		{
		// If there is a modality LUT, record it.
		if ( pDicomHeader -> CalibrationInfo.ModalityLUTDataBufferSize > 0 )
			{
			nBytesWritten = (long)fwrite( pDicomHeader -> CalibrationInfo.pModalityLUTData, 1,
										pDicomHeader -> CalibrationInfo.ModalityLUTDataBufferSize, pOutputImageFile );
			bNoError = ( nBytesWritten == pDicomHeader -> CalibrationInfo.ModalityLUTDataBufferSize );
			}
		}
	if ( bNoError )
		{
		// If there is a VOI (value of interest) LUT, record it.
		if ( pDicomHeader -> CalibrationInfo.VOI_LUTDataBufferSize > 0 )
			{
			nBytesWritten = (long)fwrite( pDicomHeader -> CalibrationInfo.pVOI_LUTData, 1,
										pDicomHeader -> CalibrationInfo.VOI_LUTDataBufferSize, pOutputImageFile );
			bNoError = ( nBytesWritten == pDicomHeader -> CalibrationInfo.VOI_LUTDataBufferSize );
			}
		}
	if ( bNoError )
		{
		// Read from the beginning of the image buffer.
		if ( bNoError && pDicomHeader -> pImageData != 0 )
			{
			nBytesRemainingInImageItem = pDicomHeader -> ImageLengthInBytes;
			// Process the image pixel data.
			if ( pDicomHeader -> FileDecodingPlan.ImageDataTransferSyntax & UNCOMPRESSED )
				{
				LogMessage( "Converting uncompressed image.", MESSAGE_TYPE_SUPPLEMENTARY );
				bNoError = ConvertUncompressedImageToPNGFile( pDicomHeader, pOutputImageFile, bAppendCalibrationData );
				}
			else
				{
				LogMessage( "Converting default JPEG image.", MESSAGE_TYPE_SUPPLEMENTARY );
				nImageBitDepth = *pDicomHeader -> BitsStored;
				if ( nImageBitDepth > 0 && nImageBitDepth <= 16 )
					{
					if ( nImageBitDepth <= 8 )
						bNoError = Convert8BitJpegImageToPNGFile( pDicomHeader, pOutputImageFile );
					else if ( nImageBitDepth <= 12 )
						bNoError = Convert12BitJpegImageToPNGFile( pDicomHeader, pOutputImageFile );
					else if ( nImageBitDepth <= 16 )
						bNoError = Convert16BitJpegImageToPNGFile( pDicomHeader, pOutputImageFile );
					}
				}
			}
		else
			{
			bNoError = FALSE;
			RespondToError( MODULE_REFORMAT, REFORMAT_ERROR_IMAGE_CONVERT_SEEK );
			}
		}
	if ( pOutputImageFile != 0 )					// *[1] Move outside of limited scope.
		fclose( pOutputImageFile );

	return bNoError;
}


