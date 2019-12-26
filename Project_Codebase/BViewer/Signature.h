// Signature.h : Defines the functions and data structures that handle reading
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

#define SIGNATURE_ERROR_INSUFFICIENT_MEMORY			1
#define SIGNATURE_ERROR_OPEN_CFG_FILE				2
#define SIGNATURE_ERROR_READ_CFG_FILE				3
#define SIGNATURE_ERROR_WRONG_FILE_TYPE				4
#define SIGNATURE_ERROR_FORMAT_NOT_SUPPORTED		5
#define SIGNATURE_ERROR_READ_COLOR_TABLE			6

#define SIGNATURE_ERROR_DICT_LENGTH					6



typedef struct
	{
	long				WidthInPixels;
	long				HeightInPixels;
	WORD				BitsPerPixel;
	size_t				ImageSizeInBytes;
	unsigned char		*pImageData;
	} SIGNATURE_BITMAP;



// Function prototypes.
//
void				InitSignatureModule();
void				CloseSignatureModule();
SIGNATURE_BITMAP	*ReadSignatureFile( char *pSignatureFileName );


