// ExamReformat.h : Defines the data structures and functions related to
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
#pragma once


#define REFORMAT_ERROR_INSUFFICIENT_MEMORY			1
#define REFORMAT_ERROR_EXTRACTION					2
#define REFORMAT_ERROR_JPEG_2000					3
#define REFORMAT_ERROR_JPEG_CORRUPTION				4
#define REFORMAT_ERROR_IMAGE_CONVERT_SEEK			5
#define REFORMAT_ERROR_PNG_WRITE					6

#define REFORMAT_ERROR_DICT_LENGTH					6



// Function prototypes.
//
void					InitExamReformatModule();
void					CloseExamReformatModule();

BOOL					PerformLocalFileReformat( PRODUCT_QUEUE_ITEM *pProductItem, PRODUCT_OPERATION *pProductOperation );
BOOL					OutputPNGImage( char* pDestFileSpec, DICOM_HEADER_SUMMARY *pDicomHeader );
BOOL					ConvertUncompressedImageToPNGFile( DICOM_HEADER_SUMMARY *pDicomHeader, FILE *pOutputImageFile, BOOL bIncludesCalibrationData );
BOOL					Convert8BitJpegImageToPNGFile( DICOM_HEADER_SUMMARY *pDicomHeader, FILE *pOutputImageFile );
BOOL					Convert12BitJpegImageToPNGFile( DICOM_HEADER_SUMMARY *pDicomHeader, FILE *pOutputImageFile );
BOOL					Convert16BitJpegImageToPNGFile( DICOM_HEADER_SUMMARY *pDicomHeader, FILE *pOutputImageFile );
BOOL					Decompress8BitJpegImage( char *pJpegSourceImageBuffer, unsigned long JpegSourceImageSizeInBytes,
													unsigned long *pImageWidthInPixels, unsigned long *pImageHeightInPixels,
													char **ppDecompressedImageData, unsigned long *pDecompressedImageSizeInBytes );

