// Signature.cpp : Implements the functions that handle reading
//  the signature bitmap file.
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
//	*[1] 01/04/2023 by Tom Atwood
//		Fixed code security issues.
//
//
#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include "Module.h"
#include "ReportStatus.h"
#include "Configuration.h"


extern CONFIGURATION			BViewerConfiguration;

//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		SignatureModuleInfo = { MODULE_SIGNATURE, "Signature Module", InitSignatureModule, CloseSignatureModule };


static ERROR_DICTIONARY_ENTRY	SignatureErrorCodes[] =
			{
				{ SIGNATURE_ERROR_INSUFFICIENT_MEMORY	, "There is not enough memory to allocate a data structure." },
				{ SIGNATURE_ERROR_OPEN_CFG_FILE			, "An error occurred attempting to open the signature file." },
				{ SIGNATURE_ERROR_READ_CFG_FILE			, "An error occurred while reading the signature file." },
				{ SIGNATURE_ERROR_WRONG_FILE_TYPE		, "The signature file is not a bitmap file." },
				{ SIGNATURE_ERROR_FORMAT_NOT_SUPPORTED	, "This variant of the bitmap file format is not supported." },
				{ SIGNATURE_ERROR_READ_COLOR_TABLE		, "An error occurred while reading the signature file color table." },
				{ 0										, NULL }
			};


static ERROR_DICTIONARY_MODULE		SignatureStatusErrorDictionary =
										{
										MODULE_SIGNATURE,
										SignatureErrorCodes,
										SIGNATURE_ERROR_DICT_LENGTH,
										0
										};


// This function must be called before any other function in this module.
void InitSignatureModule()
{
	LinkModuleToList( &SignatureModuleInfo );
	RegisterErrorDictionary( &SignatureStatusErrorDictionary );
}


void CloseSignatureModule()
{
}


SIGNATURE_BITMAP *ReadSignatureFile( char *pSignatureFileName )
{
	BOOL				bNoError = TRUE;
	FILE				*pSignatureFile;
	char				SignatureFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	BITMAPFILEHEADER	BitmapFileHeader;
	BITMAPINFOHEADER	BitmapInfoHeader;
	BITMAPV5HEADER		BitmapV5Header;
	DWORD				BMIHeaderSize;
	size_t				nBytesRead;
	size_t				BufferSize;
	DWORD				FileSizeInBytes;
	DWORD				nBytesInHeaders;
	unsigned long		nBitmapColors;
	RGBQUAD				*pColorTable;
	SIGNATURE_BITMAP	*pSignatureBitmap;
	int					nByte;

	pSignatureBitmap = (SIGNATURE_BITMAP*)calloc( 1, sizeof(SIGNATURE_BITMAP) );
	if ( pSignatureBitmap == 0 )
		{
		RespondToError( MODULE_SIGNATURE, SIGNATURE_ERROR_INSUFFICIENT_MEMORY );
		bNoError = FALSE;
		}
	else
		{
		strncpy_s( SignatureFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "", _TRUNCATE );
		strncat_s( SignatureFileSpec, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.ProgramDataPath, _TRUNCATE );
		strncat_s( SignatureFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "Signatures", _TRUNCATE );
		LocateOrCreateDirectory( SignatureFileSpec );	// Ensure directory exists.
		if ( SignatureFileSpec[ strlen( SignatureFileSpec ) - 1 ] != '\\' )
			strncat_s( SignatureFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );
		strncat_s( SignatureFileSpec, FULL_FILE_SPEC_STRING_LENGTH, pSignatureFileName, _TRUNCATE );
		strncat_s( SignatureFileSpec, FULL_FILE_SPEC_STRING_LENGTH, ".bmp", _TRUNCATE );

		pSignatureFile = fopen( SignatureFileSpec, "rb" );
		if ( pSignatureFile == NULL )
			{
			RespondToError( MODULE_SIGNATURE, SIGNATURE_ERROR_OPEN_CFG_FILE );
			bNoError = FALSE;
			}
		}
	if ( bNoError )
		{
		nBytesRead = fread_s( &BitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, sizeof(BITMAPFILEHEADER), pSignatureFile );		// *[1] Replaced fread with fread_s.
		if ( nBytesRead != sizeof(BITMAPFILEHEADER) )
			{
			RespondToError( MODULE_SIGNATURE, SIGNATURE_ERROR_READ_CFG_FILE );
			bNoError = FALSE;
			}
		if ( bNoError )
			{
			if ( BitmapFileHeader.bfType != 0x4d42 )	// "BM"
				{
				RespondToError( MODULE_SIGNATURE, SIGNATURE_ERROR_WRONG_FILE_TYPE );
				bNoError = FALSE;
				}
			}
		if ( bNoError )
			{
			FileSizeInBytes = BitmapFileHeader.bfSize;
			nBytesInHeaders = BitmapFileHeader.bfOffBits / 8;
			nBytesRead = fread_s( &BMIHeaderSize, sizeof(DWORD), 1, sizeof(DWORD), pSignatureFile );		// *[1] Replaced fread with fread_s.
			if ( nBytesRead != sizeof(DWORD) )
				{
				RespondToError( MODULE_SIGNATURE, SIGNATURE_ERROR_READ_CFG_FILE );
				bNoError = FALSE;
				}
			}
		if ( bNoError && BMIHeaderSize == sizeof(BITMAPV5HEADER) )
			{
			nBytesRead = fread_s( &BitmapV5Header.bV5Width, sizeof(BITMAPV5HEADER) - sizeof(DWORD), 1, sizeof(BITMAPV5HEADER) - sizeof(DWORD), pSignatureFile );		// *[1] Replaced fread with fread_s.
			if ( nBytesRead != sizeof(BITMAPV5HEADER) - sizeof(DWORD) )
				{
				RespondToError( MODULE_SIGNATURE, SIGNATURE_ERROR_READ_CFG_FILE );
				bNoError = FALSE;
				}
			if ( bNoError && BitmapV5Header.bV5Compression != BI_RGB )
				{
				RespondToError( MODULE_SIGNATURE, SIGNATURE_ERROR_FORMAT_NOT_SUPPORTED );
				bNoError = FALSE;
				}
			if ( bNoError )
				{
				pSignatureBitmap -> WidthInPixels = BitmapV5Header.bV5Width;
				pSignatureBitmap -> HeightInPixels = BitmapV5Header.bV5Height;
				pSignatureBitmap -> BitsPerPixel = BitmapV5Header.bV5BitCount;
				}
			}
		else if ( bNoError && BMIHeaderSize == sizeof(BITMAPINFOHEADER) )
			{
			nBytesRead = fread_s( &BitmapInfoHeader.biWidth, sizeof(BITMAPINFOHEADER) - sizeof(DWORD), 1, sizeof(BITMAPINFOHEADER) - sizeof(DWORD), pSignatureFile );		// *[1] Replaced fread with fread_s.
			if ( nBytesRead != sizeof(BITMAPINFOHEADER) - sizeof(DWORD) )
				{
				RespondToError( MODULE_SIGNATURE, SIGNATURE_ERROR_READ_CFG_FILE );
				bNoError = FALSE;
				}
			if ( bNoError && BitmapInfoHeader.biCompression != BI_RGB )
				{
				RespondToError( MODULE_SIGNATURE, SIGNATURE_ERROR_FORMAT_NOT_SUPPORTED );
				bNoError = FALSE;
				}
			if ( bNoError )
				{
				pSignatureBitmap -> WidthInPixels = BitmapInfoHeader.biWidth;
				pSignatureBitmap -> HeightInPixels = BitmapInfoHeader.biHeight;
				pSignatureBitmap -> BitsPerPixel = BitmapInfoHeader.biBitCount;
				}
			}

		if ( bNoError )
			{
			// Bitmap images need to be a multiple of 16 bits wide.  If they aren't
			// created that way, this will introduce a black bar on the right side:
			if ( pSignatureBitmap -> BitsPerPixel == 1 )
				while ( pSignatureBitmap -> WidthInPixels % 16 != 0 )
					pSignatureBitmap -> WidthInPixels++;
			nBitmapColors = ( 1 << ( pSignatureBitmap -> BitsPerPixel - 1 ) );
			if ( pSignatureBitmap -> BitsPerPixel < 24 )
				{
				pColorTable = new RGBQUAD[ nBitmapColors ];
				if ( pColorTable != 0 )
					{
					nBytesRead = fread_s( pColorTable, sizeof(RGBQUAD) * nBitmapColors, nBitmapColors, sizeof(RGBQUAD), pSignatureFile );		// *[1] Replaced fread with fread_s.
					if ( nBytesRead != nBitmapColors * sizeof(RGBQUAD) )
						{
						RespondToError( MODULE_SIGNATURE, SIGNATURE_ERROR_READ_COLOR_TABLE );
						bNoError = FALSE;
						}
					delete [] pColorTable;			// *[1] Fixed memory leak by adding array specifier.
					}
				else
					{
					RespondToError( MODULE_SIGNATURE, SIGNATURE_ERROR_INSUFFICIENT_MEMORY );
					bNoError = FALSE;
					}
				}
			}
		if ( bNoError )
			{
			pSignatureBitmap -> ImageSizeInBytes = 0;									// *[1] Set limits on the size of the memory allocation
			BufferSize = FileSizeInBytes - nBytesInHeaders;								// *[1]  and use a size_t data type for the allocation
			if ( BufferSize > 0 && BufferSize < 4000000 )								// *[1]  to ensure no erroneous allocation due to integer
				{																		// *[1]  overflow.
				pSignatureBitmap -> ImageSizeInBytes = BufferSize;						// *[1]
				pSignatureBitmap -> pImageData = (unsigned char*)malloc( BufferSize );	// *[1]
				}
			if ( pSignatureBitmap -> pImageData != 0 )
				{
				nBytesRead = fread_s( pSignatureBitmap -> pImageData, pSignatureBitmap -> ImageSizeInBytes, 1, pSignatureBitmap -> ImageSizeInBytes, pSignatureFile );		// *[1] Replaced fread with fread_s.
				if ( pSignatureBitmap -> BitsPerPixel == 1 )
					for ( nByte = 0; nByte < (int)nBytesRead; nByte++ )
						pSignatureBitmap -> pImageData[ nByte ] = ~pSignatureBitmap -> pImageData[ nByte ];
				}
			else
				{
				RespondToError( MODULE_SIGNATURE, SIGNATURE_ERROR_INSUFFICIENT_MEMORY );
				bNoError = FALSE;
				}
			}
		fclose( pSignatureFile );
		}
	if ( !bNoError )
		{
		if ( pSignatureBitmap != 0 )
			free( pSignatureBitmap );
		pSignatureBitmap = 0;
		}

	return pSignatureBitmap;
}


