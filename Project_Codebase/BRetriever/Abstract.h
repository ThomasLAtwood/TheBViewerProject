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
#pragma once


#define ABSTRACT_ERROR_INSUFFICIENT_MEMORY			1
#define ABSTRACT_ERROR_FILE_OPEN					2
#define ABSTRACT_ERROR_FILE_READ					3
#define ABSTRACT_ERROR_FILE_PARSE					4
#define ABSTRACT_ERROR_PARSE_ATTRIB_VALUE			5
#define ABSTRACT_ERROR_MULTIPLE_COPIES				6
#define ABSTRACT_ERROR_ITEM_NOT_IN_DICTIONARY		7
#define ABSTRACT_ERROR_PATH_NOT_FOUND				8
#define ABSTRACT_ERROR_ABSTRACT_FILE_OPEN			9

#define ABSTRACT_ERROR_DICT_LENGTH					9


// Configuration file syntax status tracking.
typedef long	FILE_STATUS;
	#define FILE_STATUS_OK				0
	#define FILE_STATUS_EOF				1
	#define FILE_STATUS_READ_ERROR		2


// Configuration file semantics status tracking:
#define CONFIG_STATE_UNSPECIFIED		0
#define CONFIG_STATE_FILE_OPENED		0x01
#define CONFIG_STATE_OUTPUT_DESIGNATED	0x02
#define CONFIG_STATE_USAGE_DESIGNATED	0x04
#define CONFIG_STATE_FIELDS_FOUND		0x10


// Options for abstract usage.  This specifies what the particular abstract
// output is to be used for.  LOG and SELECTION require internal program
// semantics.
#define ABSTRACT_USAGE_IMPORT			1
#define ABSTRACT_USAGE_EXPORT			2


typedef struct
	{
	unsigned short					Group;
	unsigned short					Element;
	char							Description[ 64 ];
	char							TempTextValueStorage[ MAX_LOGGING_STRING_LENGTH ];
	} ABSTRACT_FIELD;


typedef struct _AbstractFileContents
	{
	char							OutputFileName[ MAX_FILE_SPEC_LENGTH ];
	unsigned long					Usage;
	unsigned long					FieldCount;
	ABSTRACT_FIELD					*pAbstractFieldArray;
	struct _AbstractFileContents	*pNextFileContents;
	} ABSTRACT_FILE_CONTENTS;


typedef struct _AbstractRecordTextLine
	{
	char							*pAbstractRecordText;
	ABSTRACT_FILE_CONTENTS			*pAbstractFileContents;
	struct _AbstractRecordTextLine	*pNextAbstractRecordTextLine;
	} ABSTRACT_RECORD_TEXT_LINE;

// Function prototypes:

void						InitAbstractModule();
void						CloseAbstractModule();

void						FormatCSVField( char *pInputString, char *pOutputString, unsigned short nOutputBufferChars );
BOOL						DecodeCSVField( char *pInputString, char *pOutputString, unsigned short nOutputBufferChars );

BOOL						ReadAllAbstractConfigFiles();
BOOL						ReadAbstractConfigFile( char *AbstractConfigurationFileSpec );
FILE_STATUS					ReadAbstractConfigItem( FILE *pCfgFile, char *TextLine, long nMaxBytes );
BOOL						ParseAbstractConfigItem( char ConfigLine[], ABSTRACT_FILE_CONTENTS *pAbstractFileContents );
BOOL						OpenNewAbstractRecord();
BOOL						AddNewAbstractDataElement( TAG DicomElementTag, char *pElementTextValue );
ABSTRACT_RECORD_TEXT_LINE	*CreateNewAbstractRecords();
BOOL						OutputAbstractRecords( char *pOutputFileName, ABSTRACT_RECORD_TEXT_LINE *pAbstractLineList );
void						DeallocateAbstractConfiguration();
BOOL						IsFileAStandardReferenceImage(  char *pDicomFileName );
