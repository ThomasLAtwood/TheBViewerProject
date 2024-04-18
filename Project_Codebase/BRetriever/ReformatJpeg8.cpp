// ReformatJpeg8.cpp : Implements the data structures and functions related to
//	the conversion of an image from a 8-bit per pixel JPEG grayscale format into
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
#include "Dicom.h"
#include "Configuration.h"
#include "Operation.h"
#include "ProductDispatcher.h"
#include "ExamReformat.h"


#pragma pack(push)
#pragma pack(8)		// Pack structure members on 8-byte boundaries for faster access and library compatibility.

extern "C"
{
#include "jpeglib8.h"
#include "png.h"


struct LocalJpegErrorManager
	{
	struct jpeg_error_mgr		DefaultErrorManager;		// "public" fields
	jmp_buf						ReturnEnvironmentBuffer;	// for return to caller
	};

METHODDEF(void) JumpToErrorExit( j_common_ptr cinfo )
{
	LocalJpegErrorManager		*pErrorManager = (LocalJpegErrorManager*)cinfo -> err;

	// Always display the message.
	// We could postpone this until after returning, if we chose.
	( *cinfo -> err -> output_message )( cinfo );

	// Return control to the setjmp point
	longjmp( pErrorManager -> ReturnEnvironmentBuffer, 1 );
}

}	// end extern "C"

#pragma pack(pop)


#define MAX_READ_BUFFER_SIZE		0x20000

// When this function is called the pDicomFile input file must not only be opened, but it must also
// have its read cursor positioned to the beginning of the image "pixel data", namely the beginning
// of the JPEG image header within the Dicom file.
BOOL Convert8BitJpegImageToPNGFile( DICOM_HEADER_SUMMARY *pDicomHeader, FILE *pOutputImageFile )
{
	BOOL							bNoError = TRUE;
	BOOL							bEndOfFile;
	BOOL							bBufferNotFull;
	long							nImageBitsAllocatedPerPixel;
	long							nImageOutputBitDepth;
	long							nImageBitDepth;
	long							nImagePixelsPerRow;
	long							nImageBytesPerRow;
	long							nImageRows;
	int								JPEGColorType;
	int								PNGColorType;
	int								JPEGColorComponents;
	long							nRow;
	long							nBytesRemainingInBuffer;
	long							nImageRowsPerBuffer;
	long							nImageBytesPerBuffer;
	long							nBytesWritten;
	long							nImageRowsInserted;
	char							*pBuffer;
	double							Gamma;

	// This struct contains the JPEG decompression parameters and pointers to
	// working space (which is allocated as needed by the Jpeg8 library).
	struct jpeg_decompress_struct	JpegDecompressInfo;			// Interface to the Jpeg library.
	struct LocalJpegErrorManager	LocalErrorManager;

	png_struct						*pPngConfig;				// Interface to the PNG library.
	png_info						*pPngImageInfo;				// Interface to the PNG library.
	png_color_8						PngSignificantBits;			// Significant bits in each available channel;

	unsigned char					**pRows;					// Array of pointers to image pixel rows.
	
	bEndOfFile = FALSE;
	pPngConfig = 0;
	pPngImageInfo = 0;
	pRows = 0;
	pBuffer = (char*)malloc( MAX_READ_BUFFER_SIZE );		// Allocate a buffer for decoding the JPEG source image.
	if ( pBuffer == 0 )
		{
		bNoError = FALSE;
		RespondToError( MODULE_REFORMAT, REFORMAT_ERROR_INSUFFICIENT_MEMORY );
		}
	if ( bNoError )
		{
		// Set up the local error handler.
		JpegDecompressInfo.err = jpeg_std_error( &LocalErrorManager.DefaultErrorManager );
		LocalErrorManager.DefaultErrorManager.error_exit = JumpToErrorExit;
		// Establish the setjmp return context for LocalErrorManager to use. */
		if ( setjmp( LocalErrorManager.ReturnEnvironmentBuffer ) )
			{
			// If we get here, the JPEG code has signaled an error.
			RespondToError( MODULE_REFORMAT, REFORMAT_ERROR_JPEG_CORRUPTION );
			jpeg_destroy_decompress( &JpegDecompressInfo );
			bNoError = FALSE;
			if ( pRows != 0 )						// *[1] Clean up before error exit.
				free( pRows );
			if ( pBuffer != 0 )
				free( pBuffer );
			}
		}
	if ( bNoError )
		{
		// Initialize the JPEG decompression object.
		jpeg_create_decompress( &JpegDecompressInfo );
		// Specify the data source.
		jpeg_memory_src( &JpegDecompressInfo, pDicomHeader -> pImageData, pDicomHeader -> ImageLengthInBytes );
		// Read the JPEG file parameters.
		jpeg_read_header( &JpegDecompressInfo, TRUE );
		Gamma = JpegDecompressInfo.output_gamma;
		// Extract the image parameters from the JPEG header information.
 		nImageBitDepth = (long)JpegDecompressInfo.data_precision;
		nImagePixelsPerRow = (long)JpegDecompressInfo.image_width;
		nImageRows = (long)JpegDecompressInfo.image_height;
		*pDicomHeader -> ImageRows = (unsigned short)nImageRows;
		*pDicomHeader -> ImageColumns = (unsigned short)nImagePixelsPerRow;
		if ( nImageBitDepth <= 8 )
			{
			nImageBitsAllocatedPerPixel = 8;
			nImageBytesPerRow = nImagePixelsPerRow;
			nImageOutputBitDepth = 8;
			}
		else if ( nImageBitDepth > 8 )
			{
			nImageBitsAllocatedPerPixel = 16;
			nImageBytesPerRow = nImagePixelsPerRow * 2;
			nImageOutputBitDepth = 16;
			}
		JPEGColorType = JpegDecompressInfo.out_color_space;
		JPEGColorComponents = JpegDecompressInfo.out_color_components;

		nImageRowsPerBuffer = MAX_READ_BUFFER_SIZE / nImageBytesPerRow;
		nImageBytesPerBuffer = nImageRowsPerBuffer * nImageBytesPerRow;
		// Create an array of pointers to the image pixel rows to be output by the JPEG
		// conversion and input by the PNG conversion.
		pRows = (unsigned char**)malloc( nImageRowsPerBuffer * sizeof(char*) );
		if ( pRows == 0 )
			{
			bNoError = FALSE;
			RespondToError( MODULE_REFORMAT, REFORMAT_ERROR_INSUFFICIENT_MEMORY );
			}
		else
			{
			for( nRow = 0; nRow < nImageRowsPerBuffer; nRow++ )
				pRows[ nRow ] = (unsigned char*)( pBuffer + ( nRow * nImageBytesPerRow ) );
			}
		}
	if ( bNoError )
		{
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
		}
	if ( bNoError )
		{
		/// Allocate/initialize the image information data.
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
		// Convert the image color information.  Default to grayscale.
		PngSignificantBits.alpha = 0;
		PngSignificantBits.blue = 0;
		PngSignificantBits.green = 0;
		PngSignificantBits.red = 0;
		PngSignificantBits.gray = (png_byte)nImageBitDepth;					// *[1] Recast to eliminate data type mismatch.
		switch( JPEGColorType )
			{
			case JCS_UNKNOWN:		// Error, unspecified.
				PNGColorType = PNG_COLOR_TYPE_GRAY;
				break;
			case JCS_GRAYSCALE:		// Monochrome.
				PNGColorType = PNG_COLOR_TYPE_GRAY;
				break;
			case JCS_RGB:			// Red/green/blue.
			case JCS_YCbCr:			// Y/Cb/Cr (also known as YUV).
				PNGColorType = PNG_COLOR_TYPE_RGB;
				PngSignificantBits.blue = 8;
				PngSignificantBits.green = 8;
				PngSignificantBits.red = 8;
				PngSignificantBits.gray = 0;
				break;
			case JCS_CMYK:			// C/M/Y/K.
			case JCS_YCCK:			// Y/Cb/Cr/K.
				PNGColorType = PNG_COLOR_TYPE_RGB_ALPHA;
				PngSignificantBits.alpha = 8;
				PngSignificantBits.blue = 8;
				PngSignificantBits.green = 8;
				PngSignificantBits.red = 8;
				PngSignificantBits.gray = 0;
				break;
			default:
				PNGColorType = PNG_COLOR_TYPE_GRAY;
				break;
			};
		// Set the image information here.  Width and height are up to 2^31,
		// bit_depth is one of 1, 2, 4, 8, or 16.
		png_set_IHDR( pPngConfig, pPngImageInfo, nImagePixelsPerRow, nImageRows, nImageOutputBitDepth,
						PNGColorType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE );
		// Optional gamma chunk is strongly suggested if you have any guess
		// as to the correct gamma of the image.
		png_set_gAMA( pPngConfig, pPngImageInfo, Gamma );
		// Write the file header information.
		png_write_info( pPngConfig, pPngImageInfo );

		// Create an output chunk to indicate the original image grayscale bit depth.
		png_set_sBIT( pPngConfig, pPngImageInfo, &PngSignificantBits );
		}
	if ( bNoError )
		{
		// Initialize for decompression.
		jpeg_start_decompress( &JpegDecompressInfo );
		}
	while ( bNoError && !bEndOfFile )
		{
		// Load the image buffer with as many image rows as it will hold of decompressed
		// JPEG image rows.
		bBufferNotFull = TRUE;
		nImageRowsInserted = 0;
		while (  bNoError && bBufferNotFull )
			{
			nImageRowsInserted += jpeg_read_scanlines ( &JpegDecompressInfo, (JSAMPARRAY)&pRows[ nImageRowsInserted ],
																	nImageRowsPerBuffer - nImageRowsInserted );
			bEndOfFile = ( JpegDecompressInfo.output_scanline == JpegDecompressInfo.output_height );
			bBufferNotFull = ( nImageRowsPerBuffer > nImageRowsInserted && !bEndOfFile );
			}
		nBytesRemainingInBuffer = nImageRowsInserted * nImageBytesPerRow;
		// Convert the decompressed rows into PNG output.
		png_write_rows( pPngConfig, (png_bytepp)pRows, nImageRowsInserted );
		nBytesWritten = nImageRowsInserted * nImageBytesPerRow;
		if ( nBytesWritten != nBytesRemainingInBuffer )
			{
			RespondToError( MODULE_REFORMAT, REFORMAT_ERROR_PNG_WRITE );
			fclose( pOutputImageFile );
			pOutputImageFile = 0;
			bNoError = FALSE;
			}
		nBytesRemainingInBuffer = 0L;
		}			// ... end while more image data remains to be written.
	if ( bNoError )
		{
		(void)jpeg_finish_decompress( &JpegDecompressInfo );
		jpeg_destroy_decompress( &JpegDecompressInfo );
		}
	if ( pOutputImageFile != 0 && pPngConfig != 0 && pPngImageInfo != 0 )
		{
		png_write_end( pPngConfig, pPngImageInfo );
		png_destroy_write_struct( &pPngConfig, &pPngImageInfo );
		}

	if ( pRows != 0 )
		free( pRows );
	if ( pBuffer != 0 )
		free( pBuffer );

	return bNoError;
}


// When this function is called, the compressed image pixel data is preceded by
// the JPEG image header within the file.  This function retrieves the compression
// details from this JPEG header and produces an uncompressed image in a memory
// buffer.
BOOL Decompress8BitJpegImage( char *pJpegSourceImageBuffer, unsigned long JpegSourceImageSizeInBytes,
								unsigned long *pImageWidthInPixels, unsigned long *pImageHeightInPixels,
								char **ppDecompressedImageData, unsigned long *pDecompressedImageSizeInBytes )
{
	BOOL							bNoError = TRUE;
	BOOL							bEndOfFile;
	BOOL							bBufferNotFull;
	long							nImageBitsAllocatedPerPixel;
	long							nImageBytesAllocatedPerPixel;
	long							nImageBitDepth;
	long							nImagePixelsPerRow;
	long							nImageBytesPerRow;
	long							nImageRows;
	long							nRow;
	long							nBytesRemainingInBuffer;
	long							nImageRowsPerBuffer;
	long							nImageBytesPerBuffer;
	long							nBytesWritten;
	long							nImageRowsInserted;
	size_t							UncompressedImageSizeInBytes;
	char							*pBuffer;
	double							Gamma;
	char							Msg[ MAX_FILE_SPEC_LENGTH ];

	// This struct contains the JPEG decompression parameters and pointers to
	// working space (which is allocated as needed by the Jpeg8 library).
	struct jpeg_decompress_struct	JpegDecompressInfo;			// Interface to the Jpeg library.
	struct LocalJpegErrorManager	LocalErrorManager;
	unsigned char					**pRows;					// Array of pointers to image pixel rows.
	
	pBuffer = 0;
	bEndOfFile = FALSE;
	pRows = 0;
	if ( bNoError )
		{
		// Set up the local error handler.
		JpegDecompressInfo.err = jpeg_std_error( &LocalErrorManager.DefaultErrorManager );
		LocalErrorManager.DefaultErrorManager.error_exit = JumpToErrorExit;
		// Establish the setjmp return context for LocalErrorManager to use. */
		if ( setjmp( LocalErrorManager.ReturnEnvironmentBuffer ) )
			{
			// If we get here, the JPEG code has signaled an error.
			RespondToError( MODULE_REFORMAT, REFORMAT_ERROR_JPEG_CORRUPTION );
			jpeg_destroy_decompress( &JpegDecompressInfo );
			bNoError = FALSE;
			if ( pRows != 0 )						// *[1] Clean up before error exit.
				free( pRows );
			if ( pBuffer != 0 )
				free( pBuffer );
			}
		}
	if ( bNoError )
		{
		// Initialize the JPEG decompression object.
		jpeg_create_decompress( &JpegDecompressInfo );
		// Specify the data source.
		jpeg_memory_src( &JpegDecompressInfo, pJpegSourceImageBuffer, JpegSourceImageSizeInBytes );
		// Read the JPEG file parameters.
		jpeg_read_header( &JpegDecompressInfo, TRUE );
		Gamma = JpegDecompressInfo.output_gamma;
		// Extract the image parameters from the JPEG header information.
 		nImageBitDepth = (long)JpegDecompressInfo.data_precision;
		nImagePixelsPerRow = (long)JpegDecompressInfo.image_width;
		*pImageWidthInPixels = (unsigned long)nImagePixelsPerRow;					// *[1] Recast to eliminate data type mismatch.
		nImageRows = (long)JpegDecompressInfo.image_height;
		*pImageHeightInPixels = (unsigned long)nImageRows;							// *[1] Recast to eliminate data type mismatch.
		if ( nImagePixelsPerRow > 20000 || nImagePixelsPerRow < 0 || nImageRows > 20000 || nImageRows < 0 )
			{
			RespondToError( MODULE_REFORMAT, REFORMAT_ERROR_JPEG_CORRUPTION );
			bNoError = FALSE;
			}
		if ( nImageBitDepth <= 8 )
			{
			nImageBitsAllocatedPerPixel = 8;
			nImageBytesAllocatedPerPixel = 1;
			nImageBytesPerRow = nImagePixelsPerRow;
			}
		else if ( nImageBitDepth > 8 )
			{
			nImageBitsAllocatedPerPixel = 16;
			nImageBytesAllocatedPerPixel = 2;
			nImageBytesPerRow = nImagePixelsPerRow * 2;
			}
		// Allocate a buffer large enough to hold the uncompressed image.  Allocate 16 bits per pixel.
		UncompressedImageSizeInBytes = nImageRows * nImageBytesPerRow;
		pBuffer = (char*)malloc( UncompressedImageSizeInBytes );
		if ( pBuffer == 0 )
			{
			bNoError = FALSE;
			RespondToError( MODULE_REFORMAT, REFORMAT_ERROR_INSUFFICIENT_MEMORY );
			}

		nImageRowsPerBuffer = UncompressedImageSizeInBytes / nImageBytesPerRow;
		nImageBytesPerBuffer = nImageRowsPerBuffer * nImageBytesPerRow;
		// Create an array of pointers to the image pixel rows to be output by the JPEG
		// conversion.
		pRows = (unsigned char**)malloc( nImageRowsPerBuffer * sizeof(char*) );
		if ( pRows == 0 )
			{
			bNoError = FALSE;
			RespondToError( MODULE_REFORMAT, REFORMAT_ERROR_INSUFFICIENT_MEMORY );
			}
		else
			{
			for( nRow = 0; nRow < nImageRowsPerBuffer; nRow++ )
				pRows[ nRow ] = (unsigned char*)( pBuffer + nImageBytesPerBuffer - ( ( nRow + 1 ) * nImageBytesPerRow ) );
			}
		}
	if ( bNoError )
		{
		// Initialize for decompression.
		jpeg_start_decompress( &JpegDecompressInfo );
		}
	while ( bNoError && !bEndOfFile )
		{
		// Load the image buffer with as many image rows as it will hold of decompressed
		// JPEG image rows.
		bBufferNotFull = TRUE;
		nImageRowsInserted = 0;
		while (  bNoError && bBufferNotFull )
			{
			nImageRowsInserted += jpeg_read_scanlines ( &JpegDecompressInfo, (JSAMPARRAY)&pRows[ nImageRowsInserted ],
																	nImageRowsPerBuffer - nImageRowsInserted );
			bEndOfFile = ( JpegDecompressInfo.output_scanline == JpegDecompressInfo.output_height );
			bBufferNotFull = ( nImageRowsPerBuffer > nImageRowsInserted && !bEndOfFile );
			}
		nBytesRemainingInBuffer = nImageRowsInserted * nImageBytesPerRow;
		nBytesWritten = nImageRowsInserted * nImageBytesPerRow;
		if ( nBytesWritten != nBytesRemainingInBuffer )
			{
			RespondToError( MODULE_REFORMAT, REFORMAT_ERROR_JPEG_CORRUPTION );
			bNoError = FALSE;
			}
		nBytesRemainingInBuffer = 0L;
		}			// ... end while more image data remains to be written.
	if ( bNoError )
		{
		(void)jpeg_finish_decompress( &JpegDecompressInfo );
		*ppDecompressedImageData = pBuffer;
		*pDecompressedImageSizeInBytes = nImageRows * nImagePixelsPerRow * nImageBytesAllocatedPerPixel;
		jpeg_destroy_decompress( &JpegDecompressInfo );
		_snprintf_s( Msg, MAX_FILE_SPEC_LENGTH, _TRUNCATE,										// *[1] Replaced sprintf() with _snprintf_s.
						"JPEG image was decompressed successfully:  Width = %d,  Height = %d,  Image Size (bytes) = %d", nImagePixelsPerRow, nImageRows, *pDecompressedImageSizeInBytes );
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
		}
	if ( pBuffer != 0 )																			// *[1] Removed the else condition on this deallocation.
		free( pBuffer );

	if ( pRows != 0 )
		free( pRows );

	return bNoError;
}



