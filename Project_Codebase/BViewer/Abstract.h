// Abstract.h - Defines the functions and data structures that marshal the 
// Dicom data elements from the image file and package it for reading by
// BViewer as comma-separated values in "abstracted" text files.
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
//	*[1] 02/08/2024 by Tom Atwood
//		Fixed code security issues.
//
//
#pragma once

#include "ReportStatus.h"


#define ABSTRACT_ERROR_INSUFFICIENT_MEMORY			1
#define ABSTRACT_ERROR_FILE_OPEN					2
#define ABSTRACT_ERROR_FILE_READ					3

#define ABSTRACT_ERROR_DICT_LENGTH					3


#define DICOM_ATTRIBUTE_STRING_LENGTH				32
#define DICOM_ATTRIBUTE_UI_STRING_LENGTH			128
#define DICOM_ATTRIBUTE_DESCRIPTIVE_STRING_LENGTH	1024

#define MAX_AXT_LINE_LENGTH							8192


// Configuration file syntax status tracking.
typedef long	FILE_STATUS;
					#define FILE_STATUS_OK				0
					#define FILE_STATUS_EOF				1
					#define FILE_STATUS_READ_ERROR		2


// Options for abstract level.  This specifies how often the particular abstract
// output is to be updated.
#define ABSTRACT_LEVEL_UNDEFINED		0
#define ABSTRACT_LEVEL_PATIENT			1
#define ABSTRACT_LEVEL_STUDY			2
#define ABSTRACT_LEVEL_SERIES			4
#define ABSTRACT_LEVEL_IMAGE			8



typedef 	void (*ABSTRACT_EXTRACTION_FUNCTION)( char *pTitleRow, char *pDataRow );


// Function prototypes:
//
void				InitAbstractModule();
void				CloseAbstractModule();

void				FormatCSVField( char *pInputString, char *pOutputString, unsigned short nOutputBufferChars );
char				*DecodeCSVField( char *pInputString, char *pOutputString, unsigned short nOutputBufferChars );
BOOL				NewAbstractDataAreAvailable();
void				ImportNewAbstractData( ABSTRACT_EXTRACTION_FUNCTION ProcessAbstractDataRow );			// *[1] Changed return type from BOOL to void.
BOOL				ReadAbstractDataFile( char *AbstractConfigurationFileSpec, ABSTRACT_EXTRACTION_FUNCTION ProcessAbstractDataRow );
FILE_STATUS			ReadAbstractDataLine( FILE *pAbstractFile, char *TextLine, long nMaxBytes );
BOOL				GetAbstractColumnValueForSpecifiedField( char *pDesiredFieldDescription,
														char *pNewAbstractTitleLine, char *pNewAbstractDataLine, char *pValue, unsigned int nValueChars );

