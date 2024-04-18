// DicomDictionary.cpp - Implements the functionality related to reading
// and parsing the Dicom dictionary text file and creating a lookup capability
// in memory for each defined Dicom data element.
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


//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		DictionaryModuleInfo = { MODULE_DICTIONARY, "Dictionary Module", InitDictionaryModule, CloseDictionaryModule };


static ERROR_DICTIONARY_ENTRY	DictionaryErrorCodes[] =
			{
				{ DICTIONARY_ERROR_DICTIONARY_OPEN		, "An error occurred attempting to open the Dicom dictionary file." },
				{ DICTIONARY_ERROR_DICTIONARY_READ		, "An error occurred attempting to read the Dicom dictionary file." },
				{ DICTIONARY_ERROR_DICTIONARY_PARSE		, "An error occurred attempting to parse a Dicom dictionary item." },
				{ 0										, NULL }
			};

static ERROR_DICTIONARY_MODULE		DictionaryStatusErrorDictionary =
										{
										MODULE_DICTIONARY,
										DictionaryErrorCodes,
										DICOM_ERROR_DICT_LENGTH,
										0
										};

// This function must be called before any other function in this module.
void InitDictionaryModule()
{
	LinkModuleToList( &DictionaryModuleInfo );
	RegisterErrorDictionary( &DictionaryStatusErrorDictionary );
}


void CloseDictionaryModule()
{
	DeallocateDicomDictionary();
}



static DICOM_DICTIONARY_ITEM		*pDicomDictionary = 0;				// Pointer to allocated array buffer.
static DICOM_DICTIONARY_ITEM		*pPrivateDicomDictionary = 0;		// Pointer to allocated array buffer for private dictionaries.
static long							nTotalDictionaryItemCount = 0;
static long							nTotalPrivateDictionaryItemCount = 0;
static LIST_HEAD					ListOfPrivateDictionaries = 0;
char								ActivePrivateDictionaryName[ 64 ];	// The name of the private dictionary to be used for this Dicom image file.
unsigned short						ActivePrivateDataElementPrefix;




BOOL ReadDictionaryFiles( char *DicomDictionaryFileSpec, char *PrivateDicomDictionaryFileSpec )
{
	BOOL			bNoError = TRUE;

	bNoError = ReadDictionaryFile( DicomDictionaryFileSpec, FALSE );
	if ( bNoError )
		bNoError = ReadDictionaryFile( PrivateDicomDictionaryFileSpec, TRUE );

	return bNoError;
}


// The contents of the Dicom dictionary are read into memory during program initialization
// and saved there for the duration of the program execution.  These entries are referenced
// in order to supply information that may not be included in the recorded data.
BOOL ReadDictionaryFile( char *DicomDictionaryFileSpec, BOOL bIsPrivateDictionary )
{
	BOOL						bNoError = TRUE;
	FILE						*pDictFile;
	FILE_STATUS					FileStatus = FILE_STATUS_READ_ERROR;		// Error exit if file doesn't open.
	char						TextLine[ MAX_LOGGING_STRING_LENGTH ];
	char						DictionaryLine[ 1024 ];
	char						PrevDictionaryLine[ 1024 ];
	char						PrivateDictionaryName[ 64 ];
	char						PrevPrivateDictionaryName[ 64 ];
	long						nDictionaryItems;
	long						nDictionaryItem;
	long						*pnDictionaryItemCount;
	BOOL						bAllocationError;
	BOOL						bItemParsedOK;
	DICOM_DICTIONARY_ITEM		*pDictionaryArray;
	PRIVATE_DICTIONARY_INDEX	*pPrivateDictionaryIndex;
	
	PrivateDictionaryName[ 0 ] = '\0';						// *[1] Eliminate call to strcpy.
	PrevPrivateDictionaryName[ 0 ] = '\0';					// *[1] Eliminate call to strcpy.
	bAllocationError = FALSE;
	pDictFile = fopen( DicomDictionaryFileSpec, "rt" );
	if ( pDictFile != 0 )
		{
		nDictionaryItems = 0L;
		// Count the number of items in the dictionary.
		do
			{
			FileStatus = ReadDictionaryItem( pDictFile, DictionaryLine, 1024 );
			if ( FileStatus == FILE_STATUS_OK && DictionaryLine[ 0 ] != '#' && strlen( DictionaryLine ) > 0 )	// Skip comment lines.
				{
				strncpy_s( PrevDictionaryLine, 1024, DictionaryLine, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
				nDictionaryItems++;
				}
			}
		while ( FileStatus == FILE_STATUS_OK );

		if ( FileStatus & FILE_STATUS_READ_ERROR )
			{
			_snprintf_s( TextLine, MAX_LOGGING_STRING_LENGTH, _TRUNCATE, "Last good dictionary line read:\n      %s", PrevDictionaryLine );	// *[1] Replaced sprintf() with _snprintf_s.
			LogMessage( TextLine, MESSAGE_TYPE_ERROR );
			}
		else
			{
			if ( bIsPrivateDictionary )
				{
				pDictionaryArray = pPrivateDicomDictionary;
				EraseList( &ListOfPrivateDictionaries );
				pnDictionaryItemCount = &nTotalPrivateDictionaryItemCount;
				}
			else
				{
				pDictionaryArray = pDicomDictionary;
				pnDictionaryItemCount = &nTotalDictionaryItemCount;
				}
			if ( pDictionaryArray != 0 )
				{
				// Erase the previous dictionary (if any ).  First, delete the allocated Dicom element description strings.
				for ( nDictionaryItem = 0; nDictionaryItem < *pnDictionaryItemCount; nDictionaryItem++ )
					if ( pDictionaryArray[ nDictionaryItem ].Description != 0 )
						free( pDictionaryArray[ nDictionaryItem ].Description );
				free( pDictionaryArray );
				pDictionaryArray = 0;
				}
			*pnDictionaryItemCount = 0;
			// Now allocate the required memory and read the dictionary entries into memory.
			pDictionaryArray = (DICOM_DICTIONARY_ITEM*)malloc( ( nDictionaryItems + 1 ) * sizeof( DICOM_DICTIONARY_ITEM ) );
			if ( bIsPrivateDictionary )
				pPrivateDicomDictionary = pDictionaryArray;
			else
				pDicomDictionary = pDictionaryArray;
			if ( pDictionaryArray == 0 )
				bAllocationError = TRUE;
			else
				{
				rewind( pDictFile );
				nDictionaryItem = 0;
				do
					{
					FileStatus = ReadDictionaryItem( pDictFile, DictionaryLine, 1024 );
					if ( FileStatus == FILE_STATUS_OK && DictionaryLine[ 0 ] != '#'  && DictionaryLine[ 0 ] != '\n' && strlen( DictionaryLine ) > 0 )	// Skip comment lines.
						{
						strncpy_s( PrevDictionaryLine, 1024, DictionaryLine, _TRUNCATE );							// *[1] Replaced strcpy with strncpy_s.
						bItemParsedOK = ParseDictionaryItem( DictionaryLine, &pDictionaryArray[ nDictionaryItem ], bIsPrivateDictionary, PrivateDictionaryName );
						if ( !bItemParsedOK )
							{
							RespondToError( MODULE_DICTIONARY, DICTIONARY_ERROR_DICTIONARY_PARSE );
							_snprintf_s( TextLine, MAX_LOGGING_STRING_LENGTH, _TRUNCATE,							// *[1] Replaced sprintf() with _snprintf_s.
											"Dictionary line being parsed was:\n      %s", PrevDictionaryLine );
							LogMessage( TextLine, MESSAGE_TYPE_ERROR );
							}
						else
							{
							if ( bIsPrivateDictionary )
								{
								// If the private dictionary name has changed (or if this is the first one), allocate a new
								// private dictionary index and link it to the list of private dictionaries.
								if ( _stricmp( PrivateDictionaryName, PrevPrivateDictionaryName ) != 0 )
									{
									pPrivateDictionaryIndex = (PRIVATE_DICTIONARY_INDEX*)malloc( sizeof(PRIVATE_DICTIONARY_INDEX) );
									if ( pPrivateDictionaryIndex != 0 )
										{
										strncpy_s( pPrivateDictionaryIndex -> PrivateDictionaryName, 64, PrivateDictionaryName, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
										strncpy_s( PrevPrivateDictionaryName, 64, PrivateDictionaryName, _TRUNCATE );							// *[1] Replaced strcpy with strncpy_s.
										pPrivateDictionaryIndex -> nDictionaryFirstItem = nDictionaryItem;
										pPrivateDictionaryIndex -> nDictionaryItems = 1;
										AppendToList( &ListOfPrivateDictionaries, (void*)pPrivateDictionaryIndex );
										}
									}
								else
									pPrivateDictionaryIndex -> nDictionaryItems++;
								}
							nDictionaryItem++;
							}
						}
					}
				while ( FileStatus == FILE_STATUS_OK );
				*pnDictionaryItemCount = nDictionaryItem;

				if ( FileStatus & FILE_STATUS_READ_ERROR )
					{
					_snprintf_s( TextLine, MAX_LOGGING_STRING_LENGTH, _TRUNCATE,							// *[1] Replaced sprintf() with _snprintf_s.
									"Last good dictionary line read:\n      %s", PrevDictionaryLine );
					LogMessage( TextLine, MESSAGE_TYPE_ERROR );
					}
				}
			}
		fclose( pDictFile );
		}
	else
		RespondToError( MODULE_DICTIONARY, DICTIONARY_ERROR_DICTIONARY_OPEN );

	bNoError = ( ( FileStatus & FILE_STATUS_READ_ERROR ) == 0 && !bAllocationError );

	return bNoError;
}


FILE_STATUS ReadDictionaryItem( FILE *pDictFile, char *TextLine, long nMaxBytes )
{
	FILE_STATUS		FileStatus = FILE_STATUS_OK;
	int				SystemErrorNumber;

	// Read the next Group and Element Tag.
	clearerr( pDictFile );		// Reset the error indicator for this file.
	// Read a line of text from the file and append to it a null character.
	if ( fgets( TextLine, nMaxBytes - 1, pDictFile ) == NULL )
		{
		if ( feof( pDictFile ) )
			FileStatus |= FILE_STATUS_EOF;
		SystemErrorNumber = ferror( pDictFile );
		if ( SystemErrorNumber != 0 )
			{
			FileStatus |= FILE_STATUS_READ_ERROR;
			RespondToError( MODULE_DICTIONARY, DICTIONARY_ERROR_DICTIONARY_READ );
			LogMessage( strerror( SystemErrorNumber ), MESSAGE_TYPE_ERROR );
			}
		}

	return FileStatus;
}

BOOL ParseDictionaryItem( char DictionaryLine[], DICOM_DICTIONARY_ITEM* pDictItem, BOOL bIsPrivateDictionary, char PrivateDictionaryName[] )
{
	BOOL				bParseOK = TRUE;
	BOOL				bEndOfSearch;
	char				*pChar;
	char				*pDescription;
	long				nChar;
	long				mChar;
	long				nTextLineLength;
	BOOL				bBreak = FALSE;
	
	pChar = &DictionaryLine[ 0 ];
	nTextLineLength = strlen( DictionaryLine );
	if ( *pChar++ == '(' )
		{
		// Attempt to convert the first numeric field to a Group Tag number, base-16.
		// Advance the character pointer to the terminating (non-numeric) character.
		pDictItem -> Group = (unsigned short)strtol( pChar, &pChar, 16 );
		pDictItem -> EndOfGroupRange = pDictItem -> Group;	// Default to a single value, not a range of values.
		pDictItem -> GroupConstraint = MATCH_ANY;			// Default to matching both even and odd group tag numbers.
		// Check for a Group range specification.
		if ( *pChar == '-' )
			{
			switch( *++pChar )
				{
				case 'u' :
					pChar += 2;								// Advance pointer to the group tag end-of-range value.
					break;
				case 'o' :
					pDictItem -> GroupConstraint = MATCH_ODD;
					pChar += 2;								// Advance pointer to the group tag end-of-range value.
				default :
					pDictItem -> GroupConstraint = MATCH_EVEN;
					break;
				}
			// Attempt to convert the next numeric field to a Group Tag number, base-16.
			// With the presence of the '-' delimiter, this marks the end of a range of Group tag values.
			// Advance the character pointer to the terminating (non-numeric) character.
			pDictItem -> EndOfGroupRange = (unsigned short)strtol( pChar, &pChar, 16 );
			}
		if ( bParseOK && bIsPrivateDictionary )
			{
			bEndOfSearch = FALSE;
			// Next is expected a comma, separating the group and element tag fields.
			if ( *pChar++ == ',' )
				{
				// For a private dictionary the next field is a quoted string.  This is the private dictionary name.
				if ( *pChar++ == '\"' )
					{
					mChar = 0;
					while ( !bEndOfSearch )
						{
						nChar = (long)( pChar - &DictionaryLine[ 0 ] );
						if (*pChar == '\"' )
							bEndOfSearch = TRUE;
						else if (  nChar >= nTextLineLength || *pChar == '\t' )
							{
							bEndOfSearch = TRUE;
							bParseOK = FALSE;
							}
						else
							PrivateDictionaryName[ mChar++ ] = *pChar;
						pChar++;
						}
					PrivateDictionaryName[ mChar++ ] = '\0';
					}
				}
			}
		// Next is expected a comma, separating the group and element tag fields.
		if (  bParseOK && *pChar++ == ',' )
			{
			// Attempt to convert the next numeric field to an Element Tag number, base-16.
			// Advance the character pointer to the terminating (non-numeric) character.
			pDictItem -> Element = (unsigned short)strtol( pChar, &pChar, 16 );
			pDictItem -> EndOfElementRange = pDictItem -> Element;		// Default to a single value, not a range of values.
			pDictItem -> ElementConstraint = MATCH_ANY;	// Default to matching both even and odd Element tag numbers.
			// Check for an Element range specification.
			if ( *pChar == '-' )
				{
				switch( *++pChar )
					{
					case 'u' :
						pChar += 2;								// Advance pointer to the Element tag end-of-range value.
						break;
					case 'o' :
						pDictItem -> ElementConstraint = MATCH_ODD;
						pChar += 2;								// Advance pointer to the Element tag end-of-range value.
					default :
						pDictItem -> ElementConstraint = MATCH_EVEN;
						break;
					}
				// Attempt to convert the next numeric field to an Element Tag number, base-16.
				// With the presence of the '-' delimiter, this marks the end of a range of Element tag values.
				// Advance the character pointer to the terminating (non-numeric) character.
				pDictItem -> EndOfElementRange = (unsigned short)strtol( pChar, &pChar, 16 );
				}
			if ( *pChar++ == ')' && *pChar++ == '\t' )
				{
				*((unsigned long*)( &pDictItem -> ValueRepresentation )) = 0L;
				*((unsigned short*)( &pDictItem -> ValueRepresentation )) = (unsigned short)( *pChar++ << 8 );
				*((unsigned short*)( &pDictItem -> ValueRepresentation )) |= (unsigned short)( *pChar++ );
				// Mark the value representation as Unknown if not one of the recognized ones.
				switch( pDictItem -> ValueRepresentation )
					{
					case AE:
					case AS:
					case AT:
					case CS:
					case DA:
					case DS:
					case DT:
					case FL:
					case FD:
					case IS:
					case LO:
					case LT:
					case OB:
					case OF:
					case OW:
					case PN:
					case SH:
					case SL:
					case SQ:
					case SS:
					case ST:
					case TM:
					case UI:
					case UL:
					case UN:
					case US:
					case UT:
					case xs:
					case ox:
						break;
					default:
						pDictItem -> ValueRepresentation = UN;
					}
				if ( *pChar++ == '\t' )
					{
					pDescription = pChar;
					for ( nChar = 0; *pChar != '\t' && nChar < 128L; pChar++ )
						nChar++;
					if ( nChar > 0 )
						{
						pDictItem -> Description = (char*)malloc( nChar + 1 );
						if ( pDictItem -> Description != 0 )
							{
							pDictItem -> Description[ 0 ] = '\0';										// *[1] Eliminate call to strcpy.
							strncat_s( pDictItem -> Description, nChar + 1, pDescription, _TRUNCATE );	// *[1] Replaced strncat with strncat_s.
							}
						}
					else
						pDictItem -> Description = 0;
					}
				}
			else
				 bParseOK = FALSE;
			}
		}

	return bParseOK;
}


void IdentifyPrivateDicomDictionary( void *pDicomElementPtr )
{
	DICOM_ELEMENT			*pDicomElement;

	pDicomElement = (DICOM_ELEMENT*)pDicomElementPtr;
	ActivePrivateDictionaryName[ 0 ] = '\0';													// *[1] Eliminate call to strcpy.
	strncat_s( ActivePrivateDictionaryName, 64, pDicomElement -> pConvertedValue, _TRUNCATE );	// *[1] Replaced strncat with strncat_s.
	ActivePrivateDataElementPrefix = pDicomElement -> Tag.Element * 0x100;
}


static DICOM_DICTIONARY_ITEM *SearchDictionary( TAG DicomElementTag,  DICOM_DICTIONARY_ITEM *pFirstDicomDictionaryItem, long nDictionaryItems )
{
	DICOM_DICTIONARY_ITEM		*pDictItem;
	BOOL						bMatchingItemFound;
	long						nDictionaryItem;
	BOOL						bNoMatch;

	// Read the value representation and the value length.
	bMatchingItemFound = FALSE;
	for ( nDictionaryItem = 0; nDictionaryItem < nDictionaryItems && !bMatchingItemFound; nDictionaryItem++ )
		{
		pDictItem = &pFirstDicomDictionaryItem[ nDictionaryItem ];
		if ( DicomElementTag.Group >= pDictItem -> Group &&
						DicomElementTag.Group <= pDictItem -> EndOfGroupRange )
			{
			// At this point the element Group matches the dictionary Group (or range of Groups).
			// Check whether the element Group matches any Even or Odd constraint for this group range.
			bNoMatch = FALSE;
			switch( pDictItem -> GroupConstraint )
				{
				case MATCH_ANY:
					break;
				case MATCH_EVEN:
					if ( DicomElementTag.Group & 1 )
						bNoMatch = TRUE;
					break;
				case MATCH_ODD:
					if ( !( DicomElementTag.Group & 1 ) )
						bNoMatch = TRUE;
					break;
				}
			if ( !bNoMatch && DicomElementTag.Element >= pDictItem -> Element &&
						DicomElementTag.Element <= pDictItem -> EndOfElementRange )
				{
				// At this point the element tag matches the dictionary Element (or range of Elements).
				// Check whether the element tag matches any Even or Odd constraint for this Element range.
				bNoMatch = FALSE;
				switch( pDictItem -> ElementConstraint )
					{
					case MATCH_ANY:
						break;
					case MATCH_EVEN:
						if ( DicomElementTag.Element & 1 )
							bNoMatch = TRUE;
						break;
					case MATCH_ODD:
						if ( !( DicomElementTag.Element & 1 ) )
							bNoMatch = TRUE;
						break;
					}
				if ( !bNoMatch )
					{
					bMatchingItemFound = TRUE;
					}
				}
			}
		}			// ... end for next dictionary item.
	if ( !bMatchingItemFound )
		pDictItem = 0;
	
	return pDictItem;
}


DICOM_DICTIONARY_ITEM *GetDicomElementFromDictionary( TAG DicomElementTag )
{
	DICOM_DICTIONARY_ITEM		*pFirstDictItem;
	DICOM_DICTIONARY_ITEM		*pDictItem;
	BOOL						bPrivateData;
	LIST_ELEMENT				*pPrivateDictionaryListElement;
	PRIVATE_DICTIONARY_INDEX	*pPrivateDictionaryIndex;

	pDictItem = 0;
	// If the group number is odd, this is a private data sequence.  Also, treat the IconImageSequence as private data.
	bPrivateData = ( ( DicomElementTag.Group & 0x0001 ) != 0 );
	// Read the value representation and the value length.
	if ( !bPrivateData )
		{
		pFirstDictItem = &pDicomDictionary[ 0 ];
		pDictItem = SearchDictionary( DicomElementTag,  pFirstDictItem, nTotalDictionaryItemCount );
		}
	else			// This is a private data element.
		{
		pPrivateDictionaryListElement = ListOfPrivateDictionaries;
		while( pPrivateDictionaryListElement != 0 )
			{
			pPrivateDictionaryIndex = (PRIVATE_DICTIONARY_INDEX*)pPrivateDictionaryListElement -> pItem;
			if ( pPrivateDictionaryIndex != 0 )
				{
				if ( _stricmp( pPrivateDictionaryIndex -> PrivateDictionaryName, ActivePrivateDictionaryName ) == 0 )
					{
					// The matching private dictionary has been located.
					pFirstDictItem = &pPrivateDicomDictionary[ pPrivateDictionaryIndex -> nDictionaryFirstItem ];
					DicomElementTag.Element -= ActivePrivateDataElementPrefix;
					pDictItem = SearchDictionary( DicomElementTag,  pFirstDictItem, pPrivateDictionaryIndex -> nDictionaryItems );
					}
				}
			pPrivateDictionaryListElement = pPrivateDictionaryListElement -> pNextListElement;
			}
		}
	
	return pDictItem;
}


void DeallocateDicomDictionary()
{
	long					nDictionaryItem;
	DICOM_DICTIONARY_ITEM	*pDictItem = 0;

	if ( pDicomDictionary != 0 )
		{
		for ( nDictionaryItem = 0; nDictionaryItem < nTotalDictionaryItemCount; nDictionaryItem++ )
			{
			pDictItem = &pDicomDictionary[ nDictionaryItem ];
			if ( pDictItem -> Description != 0 )
				{
				free( pDictItem -> Description );
				pDictItem -> Description = 0;
				}
			}
		free( pDicomDictionary );
		pDicomDictionary = 0;
		}

	if ( pPrivateDicomDictionary != 0 )
		{
		for ( nDictionaryItem = 0; nDictionaryItem < nTotalPrivateDictionaryItemCount; nDictionaryItem++ )
			{
			pDictItem = &pPrivateDicomDictionary[ nDictionaryItem ];
			if ( pDictItem -> Description != 0 )
				{
				free( pDictItem -> Description );
				pDictItem -> Description = 0;
				}
			}
		free( pPrivateDicomDictionary );
		pPrivateDicomDictionary = 0;
		}
	if ( ListOfPrivateDictionaries != 0 )
		EraseList( &ListOfPrivateDictionaries );
}


