// DicomDictionary.h - Defines the data structures and functions related to reading
// and parsing the Dicom dictionary text file.
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

#include <stdio.h>

#define DICTIONARY_ERROR_DICTIONARY_OPEN		1
#define DICTIONARY_ERROR_DICTIONARY_READ		2
#define DICTIONARY_ERROR_DICTIONARY_PARSE		3

#define DICTIONARY_ERROR_DICT_LENGTH			3



typedef long	FILE_STATUS;
	#define FILE_STATUS_OK				0
	#define FILE_STATUS_EOF				1
	#define FILE_STATUS_READ_ERROR		2


// Dicom data value representations (VR):
typedef enum
	{
	AE = ( 'A' << 8 ) | 'E',	// Application Entity
	AS = ( 'A' << 8 ) | 'S',	// Age String
	AT = ( 'A' << 8 ) | 'T',	// Attribute Tag, a data element tag
	CS = ( 'C' << 8 ) | 'S',	// Code String
	DA = ( 'D' << 8 ) | 'A',	// Date
	DS = ( 'D' << 8 ) | 'S',	// Decimal String
	DT = ( 'D' << 8 ) | 'T',	// Date Time
	FL = ( 'F' << 8 ) | 'L',	// Floating Point Single
	FD = ( 'F' << 8 ) | 'D',	// Floating Point Double
	IS = ( 'I' << 8 ) | 'S',	// Integer String
	LO = ( 'L' << 8 ) | 'O',	// Long String
	LT = ( 'L' << 8 ) | 'T',	// Long Text
	OB = ( 'O' << 8 ) | 'B',	// Other Byte String
	OF = ( 'O' << 8 ) | 'F',	// Other Float String
	OW = ( 'O' << 8 ) | 'W',	// Other Word String
	PN = ( 'P' << 8 ) | 'N',	// Person Name
	SH = ( 'S' << 8 ) | 'H',	// Short String
	SL = ( 'S' << 8 ) | 'L',	// Signed Long
	SQ = ( 'S' << 8 ) | 'Q',	// Sequence of Items
	SS = ( 'S' << 8 ) | 'S',	// Signed Short
	ST = ( 'S' << 8 ) | 'T',	// Short Text
	TM = ( 'T' << 8 ) | 'M',	// Time
	UI = ( 'U' << 8 ) | 'I',	// Unique Identifier
	UL = ( 'U' << 8 ) | 'L',	// Unsigned long
	UN = ( 'U' << 8 ) | 'N',	// Unknown
	US = ( 'U' << 8 ) | 'S',	// Unsigned Short
	UT = ( 'U' << 8 ) | 'T',	// Unlimited Text
	xs = ( 'x' << 8 ) | 's',	// Ambiguous Sign Short
	ox = ( 'o' << 8 ) | 'x',	// Ambiguous Sign Short
	} VR;


// A Dicom element tag has the following structure:
typedef struct
	{
	unsigned short		Group;
	unsigned short		Element;
	} TAG;


// Define flags for matching even and/or odd group or element tag numbers.
typedef enum
	{
	MATCH_EVEN,
	MATCH_ODD,
	MATCH_ANY
	} TAG_MATCH_CONSTRAINT;


// Each Dicom dictionary entry is loaded into an object with the following structure:
typedef struct
	{
	unsigned short			Group;
	unsigned short			EndOfGroupRange;
	TAG_MATCH_CONSTRAINT	GroupConstraint;
	unsigned short			Element;
	unsigned short			EndOfElementRange;
	TAG_MATCH_CONSTRAINT	ElementConstraint;
	VR						ValueRepresentation;
	char					*Description;
	} DICOM_DICTIONARY_ITEM;


typedef struct
	{
	char			PrivateDictionaryName[ 64 ];
	long			nDictionaryItems;
	long			nDictionaryFirstItem;
	} PRIVATE_DICTIONARY_INDEX;


// Function prototypes.
//
void					InitDictionaryModule();
void					CloseDictionaryModule();

BOOL					ReadDictionaryFiles( char *DicomDictionaryFileSpec, char *PrivateDicomDictionaryFileSpec );
BOOL					ReadDictionaryFile( char *DicomDictionaryFileSpec, BOOL bIsPrivateDictionary );
FILE_STATUS				ReadDictionaryItem( FILE *pDictFile, char *TextLine, long nMaxBytes );
void					IdentifyPrivateDicomDictionary( void *pDicomElement );
BOOL					ParseDictionaryItem( char DictionaryLine[], DICOM_DICTIONARY_ITEM* pDictItem, BOOL bIsPrivateDictionary, char PrivateDictionaryName[] );
DICOM_DICTIONARY_ITEM	*GetDicomElementFromDictionary( TAG DicomElementTag );
void					DeallocateDicomDictionary();
