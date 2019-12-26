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
		strcpy( SignatureFileSpec, "" );
		strncat( SignatureFileSpec, BViewerConfiguration.ProgramDataPath, FULL_FILE_SPEC_STRING_LENGTH - 1 );
		strncat( SignatureFileSpec, "Signatures", FULL_FILE_SPEC_STRING_LENGTH - 2 - strlen( SignatureFileSpec ) );
		LocateOrCreateDirectory( SignatureFileSpec );	// Ensure directory exists.
		if ( SignatureFileSpec[ strlen( SignatureFileSpec ) - 1 ] != '\\' )
			strcat( SignatureFileSpec, "\\" );
		strncat( SignatureFileSpec, pSignatureFileName, FULL_FILE_SPEC_STRING_LENGTH - 1 - strlen( SignatureFileSpec ) );
		strncat( SignatureFileSpec, ".bmp", FULL_FILE_SPEC_STRING_LENGTH - 1 - strlen( SignatureFileSpec ) );

		pSignatureFile = fopen( SignatureFileSpec, "rb" );
		if ( pSignatureFile == NULL )
			{
			RespondToError( MODULE_SIGNATURE, SIGNATURE_ERROR_OPEN_CFG_FILE );
			bNoError = FALSE;
			}
		}
	if ( bNoError )
		{
		nBytesRead = fread( &BitmapFileHeader, 1, sizeof(BITMAPFILEHEADER), pSignatureFile );
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
			nBytesRead = fread( &BMIHeaderSize, 1, sizeof(DWORD), pSignatureFile );
			if ( nBytesRead != sizeof(DWORD) )
				{
				RespondToError( MODULE_SIGNATURE, SIGNATURE_ERROR_READ_CFG_FILE );
				bNoError = FALSE;
				}
			}
		if ( bNoError && BMIHeaderSize == sizeof(BITMAPV5HEADER) )
			{
			nBytesRead = fread( &BitmapV5Header.bV5Width, 1, sizeof(BITMAPV5HEADER) - sizeof(DWORD), pSignatureFile );
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
			nBytesRead = fread( &BitmapInfoHeader.biWidth, 1, sizeof(BITMAPINFOHEADER) - sizeof(DWORD), pSignatureFile );
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
					nBytesRead = fread( pColorTable, nBitmapColors, sizeof(RGBQUAD), pSignatureFile );
					if ( nBytesRead != nBitmapColors * sizeof(RGBQUAD) )
						{
						RespondToError( MODULE_SIGNATURE, SIGNATURE_ERROR_READ_COLOR_TABLE );
						bNoError = FALSE;
						}
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
			pSignatureBitmap -> ImageSizeInBytes = FileSizeInBytes - nBytesInHeaders;
			pSignatureBitmap -> pImageData = (unsigned char*)malloc( pSignatureBitmap -> ImageSizeInBytes );
			if ( pSignatureBitmap -> pImageData != 0 )
				{
				nBytesRead = fread( pSignatureBitmap -> pImageData, 1, pSignatureBitmap -> ImageSizeInBytes, pSignatureFile );
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


