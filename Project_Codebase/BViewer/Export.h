// Export.h : Defines the functions and data structures that handle data
//  exporting via the Abstracts.
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


typedef struct
	{
	char				*pColumnTitle;
	unsigned short		DataStructureID;
							// Options for abstract information source.  This information
							// is used to indicate how each abstract data contribution
							// is to be gathered.
							#define ABSTRACT_SOURCE_UNDEFINED		0
							#define ABSTRACT_SOURCE_PATIENT			1
							#define ABSTRACT_SOURCE_STUDY			2
							#define ABSTRACT_SOURCE_SERIES			3
							#define ABSTRACT_SOURCE_IMAGE			4
							#define ABSTRACT_SOURCE_READER			5
							#define ABSTRACT_SOURCE_ANALYSIS		6
							#define ABSTRACT_SOURCE_REPORT			7
							#define ABSTRACT_SOURCE_CLIENT			8
	unsigned short		DataType;
							#define TRANSLATE_AS_UNDEFINED			0
							#define TRANSLATE_AS_TEXT				1
							#define TRANSLATE_AS_BOOLEAN			2
							#define TRANSLATE_AS_BITFIELD_32		3
							#define TRANSLATE_AS_BITFIELD_16		4
							#define TRANSLATE_AS_BITFIELD_8			5
							#define TRANSLATE_AS_BITFIELD_4			6
							#define TRANSLATE_AS_DATE				7
							#define TRANSLATE_AS_FLOAT				8
							#define TRANSLATE_AS_TEXT_STRING		9
	long				DataStructureOffset;
								
	} EXPORT_COLUMN_FORMAT;



// Function prototypes:
//
BOOL				CreateAbstractExportFile( CStudy *pCurrentStudy );
void				ImportAbstractStudyRow( char *pTitleRow, char *pDataRow );

