// ExamEdit.cpp - Implements the data structures and functions related to
//	the editing of individual Dicom elements in support of Dicom file
//  composition and output.
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
#include "ReportStatus.h"
#include "ServiceMain.h"
#include "Dicom.h"
#include "Configuration.h"
#include "Operation.h"
#include "ProductDispatcher.h"
#include "ExamEdit.h"


LIST_HEAD							ExamEditSpecificationList = 0;

//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO					ExamEditModuleInfo = { MODULE_EDIT_EXAM, "Exam Editing Module",
																		InitExamEditModule, CloseExamEditModule };


static ERROR_DICTIONARY_ENTRY	ExamEditErrorCodes[] =
			{
				{ EDIT_EXAM_ERROR_INSUFFICIENT_MEMORY		, "An error occurred allocating a memory block for data storage."					},
				{ EDIT_EXAM_ERROR_FILE_OPEN					, "An error occurred attempting to open an edit specification file." },
				{ EDIT_EXAM_ERROR_FILE_READ					, "An error occurred attempting to read an edit specification file." },
				{ EDIT_EXAM_ERROR_FILE_PARSE				, "An error occurred attempting to parse an edit specification item." },
				{ 0											, NULL }
			};


static ERROR_DICTIONARY_MODULE		ExamEditStatusErrorDictionary =
										{
										MODULE_EDIT_EXAM,
										ExamEditErrorCodes,
										EDIT_EXAM_ERROR_DICT_LENGTH,
										0
										};

extern CONFIGURATION		ServiceConfiguration;
extern TRANSFER_SERVICE		TransferService;


// This function must be called before any other function in this module.
void InitExamEditModule()
{
	LinkModuleToList( &ExamEditModuleInfo );
	RegisterErrorDictionary( &ExamEditStatusErrorDictionary );
}


void CloseExamEditModule()
{
}


// Each item in an exam edit specification file consists of the two-number tag of an item from
// the Dicom dictionary, followed by the new value to be edited into that Dicom field.  For
// example, to edit the patient name, one would include the following item on a separate line:
//
//		(0010,0010),	<Lastname^Firstname>
//
// This corresponds to the "PatientsName" item in the dictionary.  Each of the two numbers
// comprising a tag is a four-digit, hexadecimal integer.
//
// A comment line begins with a "#" character.
//
// Each edit specification is linked to the ...
BOOL ReadExamEditSpecificationFile( LIST_HEAD *pEditSpecificationList )
{
	BOOL						bNoError = TRUE;
	char						EditSpecificationDirectory[ MAX_FILE_SPEC_LENGTH ];
	char						EditFileSpec[ MAX_FILE_SPEC_LENGTH ];
	FILE						*pEditSpecificationFile;
	FILE_STATUS					FileStatus = FILE_STATUS_READ_ERROR;		// Error exit if file doesn't open.
	char						TextLine[ MAX_LOGGING_STRING_LENGTH ];
	char						EditSpecificationLine[ 1024 ];
	char						PrevEditSpecificationLine[ 1024 ];
	EDIT_SPECIFICATION			*pEditSpecification;

	strcpy( EditSpecificationDirectory, "" );
	strncat( EditSpecificationDirectory, TransferService.ConfigDirectory, MAX_FILE_SPEC_LENGTH );
	if ( EditSpecificationDirectory[ strlen( EditSpecificationDirectory ) - 1 ] != '\\' )
		strcat( EditSpecificationDirectory, "\\" );
	// Check existence of source path.
	bNoError = DirectoryExists( EditSpecificationDirectory );
	if ( bNoError )
		{
		strcpy( EditFileSpec, EditSpecificationDirectory );
		strcat( EditFileSpec, "DicomEdits.cfg" );
		pEditSpecificationFile = fopen( EditFileSpec, "rt" );
		}
	if ( bNoError && pEditSpecificationFile != 0 )
		{
		sprintf( TextLine, "Editing Dicom element(s) using %s", EditFileSpec );
		LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
		do
			{
			// Read an edit specification line and parse it into the appropriate member of the
			// EDIT_SPECIFICATION structure.
			FileStatus = ReadExamEditItem( pEditSpecificationFile, EditSpecificationLine, 1024 );
			if ( FileStatus == FILE_STATUS_OK && strlen( EditSpecificationLine ) > 0 )	// Skip comment lines.
				{
				pEditSpecification = (EDIT_SPECIFICATION*)malloc( sizeof(EDIT_SPECIFICATION) );
				if ( pEditSpecification != 0 )
					{
					strcpy( PrevEditSpecificationLine, EditSpecificationLine );
					bNoError = ParseExamEditItem( EditSpecificationLine, pEditSpecification );
					pEditSpecification -> bEditCompleted = FALSE;
					}
				if ( !bNoError )
					{
					RespondToError( MODULE_EDIT_EXAM, EDIT_EXAM_ERROR_FILE_PARSE );
					sprintf( TextLine, "Exam edit line being parsed was:\n      %s", PrevEditSpecificationLine );
					LogMessage( TextLine, MESSAGE_TYPE_ERROR );
					}
				else
					{
					// Link the new edit specification information to the list of edits to be created.
					bNoError = AppendToList( pEditSpecificationList, (void*)pEditSpecification );
					if ( !bNoError )
						RespondToError( MODULE_EDIT_EXAM, EDIT_EXAM_ERROR_INSUFFICIENT_MEMORY );
					}
				}
			}
		while ( FileStatus == FILE_STATUS_OK );

		if ( FileStatus & FILE_STATUS_READ_ERROR )
			{
			sprintf( TextLine, "Last good exam edit line read:\n      %s", PrevEditSpecificationLine );
			LogMessage( TextLine, MESSAGE_TYPE_ERROR );
			bNoError = FALSE;
			}
		fclose( pEditSpecificationFile );
		}

	return bNoError;
}


FILE_STATUS ReadExamEditItem( FILE *pEditSpecificationFile, char *TextLine, long nMaxBytes )
{
	FILE_STATUS		FileStatus = FILE_STATUS_OK;
	int				SystemErrorNumber;
	char			*pChar;

	// Read the next Group and Element Tag.
	clearerr( pEditSpecificationFile );
	if ( fgets( TextLine, nMaxBytes - 1, pEditSpecificationFile ) == NULL )
		{
		if ( feof( pEditSpecificationFile ) )
			FileStatus |= FILE_STATUS_EOF;
		SystemErrorNumber = ferror( pEditSpecificationFile );
		if ( SystemErrorNumber != 0 )
			{
			FileStatus |= FILE_STATUS_READ_ERROR;
			RespondToError( MODULE_EDIT_EXAM, EDIT_EXAM_ERROR_FILE_READ );
			LogMessage( strerror( SystemErrorNumber ), MESSAGE_TYPE_ERROR );
			}
		}
	else
		{
		pChar = strchr( TextLine, '#' );		// Look for a comment delimiter in this line.
		if ( pChar != 0 )
			*pChar = '\0';						// If found, drop that portion of the string.
		TrimBlanks( TextLine );
		}

	return FileStatus;
}


BOOL ParseExamEditItem( char EditSpecificationLine[], EDIT_SPECIFICATION *pEditSpecification )
{
	BOOL					bNoError = TRUE;
	char					*pChar;
	char					TextLine[ MAX_FILE_SPEC_LENGTH ];

	strcpy( TextLine, EditSpecificationLine );
	pChar = &EditSpecificationLine[ 0 ];
	if ( *pChar == '+' )
		{
		pEditSpecification -> EditOperation = EDIT_ADD_ELEMENT;
		pChar++;
		}
	else if ( *pChar == '-' )
		{
		// The element value for the delete operation is the number of successive elements
		// following the current one that are to be deleted.
		pEditSpecification -> EditOperation = EDIT_DELETE_ELEMENT;
		pChar++;
		}
	else if ( *pChar == 'O' )
		{
		// Merge a jpeg overlay into the image.
		pEditSpecification -> EditOperation = EDIT_ADD_IMAGE_OVERLAY;
		pChar++;
		}
	else if ( *pChar == 'r' )
		{
		// Replace the image.  This was needed because some viewers can't handle
		// images with VOI_LUT data elements.  Two standard images were extracted
		// from BViewer after processing, replacing the original image.  The
		// VOI_LUT elements were removed.
		pEditSpecification -> EditOperation = EDIT_REPLACE_IMAGE;
		pChar++;
		}
	else if ( *pChar == 'c' )
		{
		// Crop the image.
		pEditSpecification -> EditOperation = EDIT_CROP_IMAGE;
		pChar++;
		}
	else
		pEditSpecification -> EditOperation = EDIT_VALUE;
	if ( *pChar++ == '(' )
		{
		// Attempt to convert the first numeric field to a Group Tag number, base-16.
		// Advance the character pointer to the terminating (non-numeric) character.
		pEditSpecification -> DicomFieldIdentifier.Group = (unsigned short)strtol( pChar, &pChar, 16 );
		// Next is expected a comma, separating the group and element tag fields.
		if ( *pChar++ == ',' )
			{
			// Attempt to convert the next numeric field to an Element Tag number, base-16.
			// Advance the character pointer to the terminating (non-numeric) character.
			pEditSpecification -> DicomFieldIdentifier.Element = (unsigned short)strtol( pChar, &pChar, 16 );
			}
		if ( *pChar++ == ')' && *pChar++ == ',' )
			{
			strcpy( pEditSpecification -> EditedFieldValue, "" );
			strcat( pEditSpecification -> EditedFieldValue, pChar );
			TrimBlanks( pEditSpecification -> EditedFieldValue );
			}
		}

	return bNoError;
}


void DeallocateEditSpecifications( LIST_HEAD *pEditSpecificationList )
{
	LIST_ELEMENT			*pEditSpecificationListElement;
	LIST_ELEMENT			*pPrevEditSpecificationListElement;
	EDIT_SPECIFICATION		*pEditSpecification;

	pEditSpecificationListElement = *pEditSpecificationList;
	while ( pEditSpecificationListElement != 0 )
		{
		pPrevEditSpecificationListElement = pEditSpecificationListElement;
		pEditSpecification = (EDIT_SPECIFICATION*)pEditSpecificationListElement -> pItem;
		if ( pEditSpecification != 0 )
			free( pEditSpecification );
		pEditSpecificationListElement = pEditSpecificationListElement -> pNextListElement;
		free( pPrevEditSpecificationListElement );
		}
	*pEditSpecificationList = 0;
}


BOOL ReadRawImageFile( DICOM_HEADER_SUMMARY *pDicomHeader, char *pFileSpec )
{
	BOOL					bNoError = TRUE;
	char					FileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	FILE					*pInputRawImageFile;
	char					Msg[ FULL_FILE_SPEC_STRING_LENGTH ];
	unsigned long			ImageSizeInBytes;
	unsigned long			ImageWidthInPixels;
	unsigned long			ImageHeightInPixels;
	unsigned short			*pRawImageBuffer;
	unsigned short			*pConvertedImageBuffer;
	size_t					nBytesToRead;
	size_t					nBytesRead;
	unsigned long			nRow;
	unsigned long			nColumn;
	DWORD					SystemErrorCode;

	strcpy( FileSpec, pFileSpec );
	pInputRawImageFile = fopen( FileSpec, "rb" );
	if ( pInputRawImageFile == 0 )
		{
		sprintf( Msg, ">>> Unable to open %s raw image file.", FileSpec );
		LogMessage( Msg, MESSAGE_TYPE_ERROR );
		}
	else
		{
		nBytesToRead = sizeof( unsigned long);
		nBytesRead = fread( &ImageSizeInBytes, 1, nBytesToRead, pInputRawImageFile );
		bNoError = ( nBytesRead == nBytesToRead );
		if ( bNoError )
			{
			nBytesRead = fread( &ImageWidthInPixels, 1, nBytesToRead, pInputRawImageFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			nBytesRead = fread( &ImageHeightInPixels, 1, nBytesToRead, pInputRawImageFile );
			bNoError = ( nBytesRead == nBytesToRead );
			}
		if ( bNoError )
			{
			pRawImageBuffer = (unsigned short*)malloc( ImageSizeInBytes );
			pConvertedImageBuffer = (unsigned short*)malloc( ImageSizeInBytes );
			bNoError = ( pRawImageBuffer != 0 && pConvertedImageBuffer != 0 );
			}
		if ( bNoError )
			{
			nBytesToRead = ImageSizeInBytes;
			nBytesRead = fread( pRawImageBuffer, 1, nBytesToRead, pInputRawImageFile );
			bNoError = ( nBytesRead == nBytesToRead );
			if ( bNoError )
				{
				for ( nRow = 0; nRow < ImageHeightInPixels; nRow++ )
					{
					for ( nColumn = 0; nColumn < ImageWidthInPixels; nColumn++ )
						{
						// Invert the image vertically.
						pConvertedImageBuffer[ ( ( ImageHeightInPixels - nRow - 1 ) * ImageWidthInPixels ) + nColumn ] =
													pRawImageBuffer[ nRow * ImageWidthInPixels + nColumn ];
						}
					}
				free( pRawImageBuffer );
				}
			else
				{
				SystemErrorCode = GetLastError();
				sprintf( Msg, "   Write Dicom File:  system error code %d", SystemErrorCode );
				LogMessage( Msg, MESSAGE_TYPE_ERROR );
				}
			}
		fclose( pInputRawImageFile );
		}
	if ( bNoError )
		{
		free( pDicomHeader -> pImageData );
		pDicomHeader -> pImageData = (char*)pConvertedImageBuffer;
		}
	else
		{
		sprintf( Msg, ">>> Error reading from raw image input file %s.", FileSpec );
		LogMessage( Msg, MESSAGE_TYPE_ERROR );
		}

	return bNoError;


}


// Apply an overlay image to the original Dicom image by reading pixel-by-pixel from the overlay
// and writing into the Dicom image starting at the specified row and column.
BOOL EnscribeImageOverlay( DICOM_HEADER_SUMMARY *pDicomHeader, char *pDecompressedImageData,
							unsigned long ImageWidthInPixels, unsigned long ImageHeightInPixels,
							unsigned BytesPerPixel, unsigned long nOverlayImageX0, unsigned long nOverlayImageY0 )
{
	BOOL					bNoError = TRUE;
	char					*pDicomImageData;
	long					nImageBitsAllocatedPerPixel;
	long					nImageRowsRemaining;
	long					nDicomImagePixelsPerRow;
	long					nDicomImageBytesPerPixel;
	long					nDicomImageBytesPerRow;
	long					nDicomImageRows;
	long					nOverlayImagePixelsPerRow;
	long					nOverlayImageBytesPerRow;
	long					nOverlayImageRows;
	char					*pInputReadPoint;
	char					*pOutputWritePoint;
	long					nOverlayRow;
	char					InputPixelValue;
	long					nOutputRow;
	unsigned short			OutputPixelValue;
	long					nPixel;

	pDicomImageData = pDicomHeader -> pImageData;
	if ( pDicomImageData != 0 )
		{
		// Load Dicom image parameters.
		nImageBitsAllocatedPerPixel = (long)( *pDicomHeader -> BitsAllocated );
		nDicomImagePixelsPerRow = (long)( *pDicomHeader -> ImageColumns );
		if ( nImageBitsAllocatedPerPixel <= 8 )
			nDicomImageBytesPerPixel = 1;
		else if ( nImageBitsAllocatedPerPixel > 8 )
			nDicomImageBytesPerPixel = 2;
		nDicomImageBytesPerRow = nDicomImagePixelsPerRow * nDicomImageBytesPerPixel;
		nDicomImageRows = (long)pDicomHeader -> ImageRows;
		// Load Overlay image parameters.
		nOverlayImagePixelsPerRow = ImageWidthInPixels;
		nOverlayImageBytesPerRow = ImageWidthInPixels * BytesPerPixel;
		nOverlayImageRows = ImageHeightInPixels;
		nImageRowsRemaining = nOverlayImageRows;

		pInputReadPoint = pDecompressedImageData + ( nOverlayImageRows - 1 ) * nOverlayImageBytesPerRow;
		nOutputRow = nOverlayImageY0;
		// If near the end of the image, limit the number of rasters process to what is actually there.
		if ( nImageRowsRemaining > nDicomImageRows - nOutputRow )
			nImageRowsRemaining = nDicomImageRows - nOutputRow;
		pOutputWritePoint = pDicomImageData + ( nOutputRow * nDicomImageBytesPerRow ) + ( nOverlayImageX0 * nDicomImageBytesPerPixel );
		while ( bNoError && nImageRowsRemaining > 0L )
			{
			for ( nOverlayRow = 0; nOverlayRow < nOverlayImageRows; nOverlayRow++ )
				{
				// Process the row in the output buffer.
				for ( nPixel = 0; nPixel < nOverlayImagePixelsPerRow; nPixel++ )
					{
					InputPixelValue = pInputReadPoint[ nPixel ];
					if ( _stricmp( pDicomHeader -> PhotometricInterpretation, "MONOCHROME1" ) == 0 )
						InputPixelValue = ~InputPixelValue;
					// Copy the overlay value to the Dicom image.
					if ( nDicomImageBytesPerPixel == 2 )
						{
						OutputPixelValue = (unsigned short)InputPixelValue;
						( (unsigned short*)pOutputWritePoint )[ nPixel ] = OutputPixelValue;
						}
					else
						pOutputWritePoint[ nPixel ] = InputPixelValue;
				}
				pInputReadPoint -= nOverlayImageBytesPerRow;
				pOutputWritePoint += nDicomImageBytesPerRow;
				nImageRowsRemaining--;
				}
			}			// ... end while more image data remains to be written.
		}

	return bNoError;
}


// Crop the image by returning a pointer into the image data corresponding to the upper
// left corner of the cropped image.
BOOL CropImage( DICOM_HEADER_SUMMARY *pDicomHeader, unsigned long nCroppedImageWidth, unsigned long nCroppedImageHeight,
													unsigned long nCroppedImageX0, unsigned long nCroppedImageY0 )
{
	BOOL					bNoError = TRUE;
	char					*pDicomImageData;
	char					*pCroppedImageData;
	unsigned long			nCroppedImageBytes;
	long					nCroppedImageBytesPerRow;
	long					nImageBitsAllocatedPerPixel;
	long					nDicomImagePixelsPerRow;
	long					nDicomImageBytesPerPixel;
	long					nDicomImageBytesPerRow;
	long					nDicomImageRows;
	unsigned long			nRow;
	unsigned long			nPixel;
	char					*pInputReadPoint;
	char					*pOutputWritePoint;

	pDicomImageData = pDicomHeader -> pImageData;
	if ( pDicomImageData != 0 )
		{
		// Load Dicom image parameters.
		nImageBitsAllocatedPerPixel = (long)( *pDicomHeader -> BitsAllocated );
		nDicomImagePixelsPerRow = (long)( *pDicomHeader -> ImageColumns );
		if ( nImageBitsAllocatedPerPixel <= 8 )
			nDicomImageBytesPerPixel = 1;
		else if ( nImageBitsAllocatedPerPixel > 8 )
			nDicomImageBytesPerPixel = 2;
		nDicomImageBytesPerRow = nDicomImagePixelsPerRow * nDicomImageBytesPerPixel;
		nDicomImageRows = (long)pDicomHeader -> ImageRows;

		nCroppedImageBytesPerRow = nCroppedImageWidth * nDicomImageBytesPerPixel;
		nCroppedImageBytes = nCroppedImageBytesPerRow * nCroppedImageHeight;
		pCroppedImageData = (char*)malloc( nCroppedImageBytes );
		bNoError =  ( pCroppedImageData > 0 );
		if ( bNoError )
			{
			pInputReadPoint = pDicomImageData + ( nCroppedImageY0 * nDicomImageBytesPerRow ) + ( nCroppedImageX0 * nDicomImageBytesPerPixel );
			pOutputWritePoint = pCroppedImageData;
			for ( nRow = 0; nRow < nCroppedImageHeight; nRow++ )
				{
				for ( nPixel = 0; nPixel < nCroppedImageWidth; nPixel++ )
					{
					if ( nDicomImageBytesPerPixel == 2 )
						( (unsigned short*)pOutputWritePoint )[ nPixel ] = ( (unsigned short*)pInputReadPoint )[ nPixel ];
					else
						pOutputWritePoint[ nPixel ] = pInputReadPoint[ nPixel ];
					}
				pInputReadPoint += nDicomImageBytesPerRow;
				pOutputWritePoint += nCroppedImageBytesPerRow;
				}
			free( pDicomHeader -> pImageData );
			pDicomHeader -> pImageData = pCroppedImageData;
			pDicomHeader -> ImageLengthInBytes = nCroppedImageBytes;
			}
		}

	return bNoError;
}

