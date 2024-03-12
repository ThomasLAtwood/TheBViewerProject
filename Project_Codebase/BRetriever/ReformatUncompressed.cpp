// ReformatUncompressed.cpp : Implements the data structures and functions related to
//	the conversion of an image from uncompressed grayscale bitmap format into
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
// #include <stdio.h>
// #include <stdlib.h>
#include "Module.h"
#include "ReportStatus.h"
#include "Dicom.h"
#include "Configuration.h"
#include "Operation.h"
#include "ProductDispatcher.h"
#include "ExamReformat.h"

#pragma pack(push)
#pragma pack(8)		// Pack structure members on 16-byte boundaries for faster access.

extern "C"
{
#include "png.h"
}

#pragma pack(pop)


#define MAX_READ_BUFFER_SIZE		0x20000

BOOL ConvertUncompressedImageToPNGFile( DICOM_HEADER_SUMMARY *pDicomHeader, FILE *pOutputImageFile, BOOL bIncludesCalibrationData )
{
	BOOL					bNoError = TRUE;
	BOOL					bEndOfFile;
	long					nImageBitsAllocatedPerPixel;
	long					nImageOutputBitDepth;
	long					nImageBitDepth;
	long					nImagePixelsPerRow;
	long					nImageBytesPerRow;
	long					nImageRows;
	long					nRow;
	long					nImageRowsPerBuffer;
	long					nImageBytesPerBuffer;
	long					nBytesToBeWritten;
	long					nBytesWritten;
	long					nImageRowsRemaining;
	int						PNGColorType;
	long					nSamplesPerPixel;

	png_struct				*pPngConfig;
	png_info				*pPngImageInfo;
	png_color_8				PngSignificantBits;

	unsigned char			**pRows;

	bEndOfFile = FALSE;
	nImageRowsRemaining = 0L;
	nImageBitsAllocatedPerPixel = (long)( *pDicomHeader -> BitsAllocated );
	nImageBitDepth = (long)( *pDicomHeader -> BitsStored );
	nImagePixelsPerRow = (long)( *pDicomHeader -> ImageColumns );
	nSamplesPerPixel = (long)( *pDicomHeader -> SamplesPerPixel );
	if ( nSamplesPerPixel == 0 )
		nSamplesPerPixel = 1;
	if ( nImageBitsAllocatedPerPixel <= 8 )
		{
		nImageBytesPerRow = nImagePixelsPerRow * nSamplesPerPixel;
		nImageOutputBitDepth = 8;
		}
	else if ( nImageBitsAllocatedPerPixel > 8 )
		{
		nImageBytesPerRow = nImagePixelsPerRow * nSamplesPerPixel * 2;
		nImageOutputBitDepth = 16;
		}
	nImageRowsPerBuffer = pDicomHeader -> ImageLengthInBytes / nImageBytesPerRow;
	nImageBytesPerBuffer = nImageRowsPerBuffer * nImageBytesPerRow;

	if ( !bIncludesCalibrationData )
		{
		unsigned long		nPixel;
		size_t				nPixelCount;
		unsigned short		*p16BitPixel;
		unsigned short		PixelValue;

		nPixelCount = nImagePixelsPerRow * nImageRowsPerBuffer;
		p16BitPixel = (unsigned short*)pDicomHeader -> pImageData;
		for ( nPixel = 0; nPixel < nPixelCount; nPixel++ )
			{
			PixelValue = ( *p16BitPixel ) << ( 16 - nImageBitDepth );
			( *p16BitPixel ) = PixelValue;
			p16BitPixel++;
			}
		}

	// Create an array of pointers to the image pixel rows to be input by the PNG conversion.
	pRows = (unsigned char**)malloc( nImageRowsPerBuffer * sizeof(char*) );
	if ( pRows == 0 )
		{
		bNoError = FALSE;
		RespondToError( MODULE_REFORMAT, REFORMAT_ERROR_INSUFFICIENT_MEMORY );
		}
	else
		{
		for( nRow = 0; nRow < nImageRowsPerBuffer; nRow++ )
			pRows[ nRow ] = (unsigned char*)( pDicomHeader -> pImageData + ( nRow * nImageBytesPerRow ) );
		}
	nImageRows = (long)( *pDicomHeader -> ImageRows );
	// Create and initialize the png_struct with the desired error handler
	// functions.  If you want to use the default stderr and longjump method,
	// you can supply NULL for the last three parameters.
	pPngConfig = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	if ( pPngConfig == NULL )
		{
		RespondToError( MODULE_REFORMAT, REFORMAT_ERROR_PNG_WRITE );
		fclose( pOutputImageFile );
		pOutputImageFile = 0;
		bNoError = FALSE;
		}
	if ( bNoError )
		{
		// Allocate/initialize the image information data.
		pPngImageInfo = png_create_info_struct( pPngConfig );
		if ( pPngImageInfo == NULL )
			{
			RespondToError( MODULE_REFORMAT, REFORMAT_ERROR_PNG_WRITE );
			fclose( pOutputImageFile );
			pOutputImageFile = 0;
			png_destroy_write_struct( &pPngConfig,  png_infopp_NULL );
			bNoError = FALSE;
			}
		}
	if ( bNoError )
		{
		// Set error handling if you aren't supplying your own
		// error handling functions in the png_create_write_struct() call.
		//
		if ( setjmp( png_jmpbuf( pPngConfig ) ) )
			{
			// If we get here, we had a problem writing the file.
			RespondToError( MODULE_REFORMAT, REFORMAT_ERROR_PNG_WRITE );
			fclose( pOutputImageFile );
			pOutputImageFile = 0;
			png_destroy_write_struct( &pPngConfig, &pPngImageInfo );
			bNoError = FALSE;
			}
		}
	if ( bNoError )
		{
		// Set up the output control for using standard C streams.
		png_init_io( pPngConfig, pOutputImageFile );
		// Set the image information here.  Width and height are up to 2^31,
		// bit_depth is one of 1, 2, 4, 8, or 16.
		PNGColorType = PNG_COLOR_TYPE_GRAY;
		PngSignificantBits.alpha = 0;
		PngSignificantBits.blue = 0;
		PngSignificantBits.green = 0;
		PngSignificantBits.red = 0;
		PngSignificantBits.gray = (unsigned char)16;	//nImageBitDepth;
		if ( _stricmp( pDicomHeader -> PhotometricInterpretation, "RGB" ) == 0 )
			{
			PNGColorType = PNG_COLOR_TYPE_RGB;
			PngSignificantBits.blue = 8;
			PngSignificantBits.green = 8;
			PngSignificantBits.red = 8;
			PngSignificantBits.gray = 0;
			}
		png_set_IHDR( pPngConfig, pPngImageInfo, nImagePixelsPerRow, nImageRows, nImageOutputBitDepth,
						PNGColorType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE );
		// Write the file header information.
		png_write_info( pPngConfig, pPngImageInfo );

		// Create an output chunk to indicate the original image grayscale bit depth.
		png_set_sBIT( pPngConfig, pPngImageInfo, &PngSignificantBits );

		if ( nImageBitsAllocatedPerPixel > 8 )
			{
			if ( ( pDicomHeader -> FileDecodingPlan.nTransferSyntaxIndex != BIG_ENDIAN_EXPLICIT_TRANSFER_SYNTAX ) &&
						_stricmp( pDicomHeader -> Modality, "NM" ) != 0 )
				png_set_swap( pPngConfig );
			}
		}

	if ( bNoError && !bEndOfFile )
		{
		nBytesToBeWritten = nImageBytesPerBuffer;
		nImageRowsPerBuffer = nBytesToBeWritten / nImageBytesPerRow;
		if ( nImageRowsPerBuffer > 0 )
			{
			// Write the image scan lines to the output file.
			png_write_rows( pPngConfig, (png_bytepp)pRows, nImageRowsPerBuffer );
			nBytesWritten = nImageRowsPerBuffer * nImageBytesPerRow;
			if ( nBytesWritten != nBytesToBeWritten )
				{
				RespondToError( MODULE_REFORMAT, REFORMAT_ERROR_PNG_WRITE );
				fclose( pOutputImageFile );
				pOutputImageFile = 0;
				bNoError = FALSE;
				}
			}
		}			// ... end while more image data remains to be written.
	if ( pOutputImageFile != 0 )
		png_write_end( pPngConfig, pPngImageInfo );
	png_destroy_write_struct( &pPngConfig, &pPngImageInfo );

	if ( pRows != 0 )
		free( pRows );

	return bNoError;
}

