// ExamEdit.h : Defines the data structures and functions related to
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
#pragma once


#define EDIT_EXAM_ERROR_INSUFFICIENT_MEMORY			1
#define EDIT_EXAM_ERROR_FILE_OPEN					2
#define EDIT_EXAM_ERROR_FILE_READ					3
#define EDIT_EXAM_ERROR_FILE_PARSE					4

#define EDIT_EXAM_ERROR_DICT_LENGTH					4



typedef struct
	{
	TAG				DicomFieldIdentifier;
	char			EditedFieldValue[ MAX_CFG_STRING_LENGTH ];
	} EDIT_SPECIFICATION;


// Function prototypes.
//
void					InitExamEditModule();
void					CloseExamEditModule();

BOOL					ReadExamEditSpecificationFile( LIST_HEAD *pEditSpecificationList );
FILE_STATUS				ReadExamEditItem( FILE *pEditSpecificationFile, char *TextLine, long nMaxBytes );
BOOL					ParseExamEditItem( char EditSpecificationLine[], EDIT_SPECIFICATION *pEditSpecification );
void					DeallocateEditSpecifications( LIST_HEAD *pEditSpecificationList );
BOOL					EnscribeImageOverlay( DICOM_HEADER_SUMMARY *pDicomHeader, char *pDecompressedImageData,
												unsigned long ImageWidthInPixels, unsigned long ImageHeightInPixels,
												unsigned BytesPerPixel, unsigned long nOverlayImageX0, unsigned long nOverlayImageY0 );
BOOL					CropImage( DICOM_HEADER_SUMMARY *pDicomHeader, unsigned long nCroppedImageWidth, unsigned long nCroppedImageHeight,
												unsigned long nCroppedImageX0, unsigned long nCroppedImageY0 );



