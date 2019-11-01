// Abstract.cpp - Implements the functions and data structures that marshal the 
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
#include "Module.h"
#include "ReportStatus.h"
#include "ServiceMain.h"
#include "DicomDictionary.h"
#include "Abstract.h"


//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		AbstractModuleInfo = { MODULE_ABSTRACT, "Abstract Module", InitAbstractModule, CloseAbstractModule };


static ERROR_DICTIONARY_ENTRY	AbstractErrorCodes[] =
			{
				{ ABSTRACT_ERROR_INSUFFICIENT_MEMORY	, "An error occurred allocating a memory block for data storage." },
				{ ABSTRACT_ERROR_FILE_OPEN				, "An error occurred attempting to open an abstract configuration file." },
				{ ABSTRACT_ERROR_FILE_READ				, "An error occurred attempting to read an abstract configuration file." },
				{ ABSTRACT_ERROR_FILE_PARSE				, "An error occurred attempting to parse an abstract configuration item." },
				{ ABSTRACT_ERROR_PARSE_ATTRIB_VALUE		, "An error occurred parsing an attribute value in an abstract configuration file." },
				{ ABSTRACT_ERROR_MULTIPLE_COPIES		, "An attribute was specified multiple times in a single abstract configuration file." },
				{ ABSTRACT_ERROR_ITEM_NOT_IN_DICTIONARY	, "A specified abstract field was not found in the Dicom dictionary." },
				{ ABSTRACT_ERROR_PATH_NOT_FOUND			, "The directory containing the abstract configuration files could not be located." },
				{ ABSTRACT_ERROR_ABSTRACT_FILE_OPEN		, "An error occurred attempting to open an abstract file for output." },
				{ 0										, NULL }
			};


static ERROR_DICTIONARY_MODULE		AbstractStatusErrorDictionary =
										{
										MODULE_ABSTRACT,
										AbstractErrorCodes,
										ABSTRACT_ERROR_DICT_LENGTH,
										0
										};

static unsigned long				ConfigurationState = CONFIG_STATE_UNSPECIFIED;
ABSTRACT_FILE_CONTENTS				*pAbstractFileList = 0;

extern CONFIGURATION				ServiceConfiguration;


// This function must be called before any other function in this module.
void InitAbstractModule()
{
	LinkModuleToList( &AbstractModuleInfo );
	RegisterErrorDictionary( &AbstractStatusErrorDictionary );
	pAbstractFileList = 0;
}


void CloseAbstractModule()
{
	DeallocateAbstractConfiguration();
}


void FormatCSVField( char *pInputString, char *pOutputString, unsigned short nOutputBufferChars )
{
	BOOL			bContainsSpecialCharacter;
	char			*pInputChar;
	char			*pOutputChar;
	int				nCharsRemaining;

	bContainsSpecialCharacter = ( strchr( pInputString, ',' ) != 0 ) ||
								( strchr( pInputString, '\"' ) != 0 ) ||
								( strchr( pInputString, 0x0A ) != 0 ) ||
								( strchr( pInputString, '\n' ) != 0 );
	pInputChar = pInputString;
	pOutputChar = pOutputString;
	nCharsRemaining = (int)strlen( pInputString );
	if ( bContainsSpecialCharacter )
		{
		*pOutputChar = '\"';
		pOutputChar++;
		nOutputBufferChars--;
		}
	while ( nCharsRemaining > 0 && nOutputBufferChars > 0 )
		{
		if ( *pInputChar == '\"' )		// If a double quote character is encountered...
			{
			*pOutputChar = '\"';		// add a second double quote.
			pOutputChar++;
			nOutputBufferChars--;
			}
		*pOutputChar = *pInputChar;
		pInputChar++;
		pOutputChar++;
		nCharsRemaining--;
		nOutputBufferChars--;
		if ( nOutputBufferChars == 0 )
			pOutputChar--;				// Back up one character.
		*pOutputChar = '\0';			// Preload the string terminator.
		}
	if ( bContainsSpecialCharacter )
		{
		if ( nOutputBufferChars <= 1 )
			pOutputChar--;				// Back up one additional character.
		if ( nOutputBufferChars == 0 )
			pOutputChar--;				// Back up one character.
		*pOutputChar = '\"';			// add a terminating double quote.
		pOutputChar++;
		*pOutputChar = '\0';			// Load the string terminator.
		}
}


// This function reads a Comma Separated Values (CSV) field value and reformats it as a text
// output string.  The function returns a pointer to the trailing delimiter of the field in the
// input string.
BOOL DecodeCSVField( char *pInputString, char *pOutputString, unsigned short nOutputBufferChars )
{
	BOOL			bNoError = TRUE;
	BOOL			bContainsSpecialCharacter;
	BOOL			bFieldTerminatorLocated;
	char			*pInputChar;
	char			*pOutputChar;

	bFieldTerminatorLocated = FALSE;
	pInputChar = pInputString;
	pOutputChar = pOutputString;
	bContainsSpecialCharacter = ( *pInputChar == '\"' );
	if ( bContainsSpecialCharacter )
		pInputChar++;
	while ( !bFieldTerminatorLocated && nOutputBufferChars > 0 )
		{
		if ( *pInputChar == '\"' )
			{
			if ( *( pInputChar + 1 ) == '\"' )	// If this begins a pair of double quotes...
				{
				pInputChar++;					// Skip the first double quote in a pair.
				*pOutputChar = *pInputChar;		// Copy the second.
				pOutputChar++;
				pInputChar++;
				nOutputBufferChars--;
				}
			else if ( bContainsSpecialCharacter )
				{
				pInputChar++;
				if ( *pInputChar == ',' )
					bFieldTerminatorLocated = TRUE;
				else
					bNoError = FALSE;
				}
			else
				bNoError = FALSE;
			}
		else
			{
			if ( !bContainsSpecialCharacter && *pInputChar == ',' )
				bFieldTerminatorLocated = TRUE;
			else
				{
				*pOutputChar = *pInputChar;
				pInputChar++;
				pOutputChar++;
				nOutputBufferChars--;
				}
			}
		if ( nOutputBufferChars == 0 )
			pOutputChar--;						// Back up one character.
		*pOutputChar = '\0';					// Preload the string terminator.
		}			// ...end while another input character remains.

	return bNoError;
}


// From the configuration directory read all the files of the form "Abstract*.cfg".  These
// files contain the instructions for which abstract files are to be generated and which
// fields are to be included in each.  An ABSTRACT_FILE_CONTENTS structure is allocated
// and populated for each file and linked to the pAbstractFileList.  Navigating this list
// provides the full description of what abstract information is to be generated.
BOOL ReadAllAbstractConfigFiles()
{
	BOOL						bNoError = TRUE;
	char						ConfigurationDirectory[ MAX_FILE_SPEC_LENGTH ];
	char						SearchFileSpec[ MAX_FILE_SPEC_LENGTH ];
	char						FoundFileSpec[ MAX_FILE_SPEC_LENGTH ];
	WIN32_FIND_DATA				FindFileInfo;
	HANDLE						hFindFile;
	BOOL						bFileFound;

	strcpy( ConfigurationDirectory, "" );
	strncat( ConfigurationDirectory, ServiceConfiguration.ConfigDirectory, MAX_FILE_SPEC_LENGTH );
	if ( ConfigurationDirectory[ strlen( ConfigurationDirectory ) - 1 ] != '\\' )
		strcat( ConfigurationDirectory, "\\" );
	// Check existence of source path.
	bNoError = DirectoryExists( ConfigurationDirectory );
	if ( bNoError )
		{
		strcpy( SearchFileSpec, ConfigurationDirectory );
		strcat( SearchFileSpec, "Abstract*.cfg" );
		hFindFile = FindFirstFile( SearchFileSpec, &FindFileInfo );
		bFileFound = ( hFindFile != INVALID_HANDLE_VALUE );
		while ( bFileFound )
			{
			if ( ( FindFileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
				{
				strcpy( FoundFileSpec, ConfigurationDirectory );
				strncat( FoundFileSpec, FindFileInfo.cFileName,
								MAX_FILE_SPEC_LENGTH - strlen( ConfigurationDirectory ) - 1 );
				bNoError = ReadAbstractConfigFile( FoundFileSpec );
				}
			// Look for another file in the source directory.
			bFileFound = FindNextFile( hFindFile, &FindFileInfo );
			}
		if ( hFindFile != INVALID_HANDLE_VALUE )
			FindClose( hFindFile );
		}
	else
		{
		RespondToError( MODULE_ABSTRACT, ABSTRACT_ERROR_PATH_NOT_FOUND );
		bNoError = FALSE;
		}

	return bNoError;
}


// Each item in an abstract configuration file specifies the two-number tag of an item from
// the Dicom dictionary.  For example, to abstract the patient name, one would include the
// following item on a separate line:
//
//		(0010,0010)
//
// This corresponds to the "PatientName" item in the dictionary.  Each of the two numbers
// comprising a tag is a four-digit, hexadecimal integer.
//
// A comment line begins with a "#" character.
//
// This function first reads the abstract configuration file in order to count the number
// of entries in the file.  Using this information it allocates an ABSTRACT_FILE_CONTENTS
// structure, then parses the information from the file into the ABSTRACT_FILE_CONTENTS
// structure.  This structure is then linked to the pAbstractFileList list of abstract
// files to be generated.
BOOL ReadAbstractConfigFile( char *AbstractConfigurationFileSpec )
{
	BOOL						bNoError = TRUE;
	FILE						*pCfgFile;
	ABSTRACT_FILE_CONTENTS		*pAbstractFileContents;
	ABSTRACT_FILE_CONTENTS		*pFileContents;
	FILE_STATUS					FileStatus = FILE_STATUS_READ_ERROR;		// Error exit if file doesn't open.
	char						TextLine[ MAX_LOGGING_STRING_LENGTH ];
	char						ConfigLine[ 1024 ];
	char						PrevConfigLine[ 1024 ];
	long						nConfigItems;
	BOOL						bItemParsedOK;
	
	sprintf( TextLine, "Reading abstract configuration file:   %s", AbstractConfigurationFileSpec );
	LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );

	ConfigurationState = CONFIG_STATE_UNSPECIFIED;
	pCfgFile = fopen( AbstractConfigurationFileSpec, "rt" );
	if ( pCfgFile != 0 )
		{
		ConfigurationState = CONFIG_STATE_FILE_OPENED;
		nConfigItems = 0L;
		// Count the number of items in the abstract configuration file.
		do
			{
			FileStatus = ReadAbstractConfigItem( pCfgFile, ConfigLine, 1024 );
			// Count the number of potential abstract field items in this file.
			if ( FileStatus == FILE_STATUS_OK && strlen( ConfigLine ) > 0 && ConfigLine[ 0 ] == '(' )
				{
				strcpy( PrevConfigLine, ConfigLine );
				nConfigItems++;
				}
			}
		while ( FileStatus == FILE_STATUS_OK );

		if ( FileStatus & FILE_STATUS_READ_ERROR )
			{
			sprintf( TextLine, "Last good abstract configuration line read:\n      %s", PrevConfigLine );
			LogMessage( TextLine, MESSAGE_TYPE_ERROR );
			}
		else
			{
			pAbstractFileContents = (ABSTRACT_FILE_CONTENTS*)malloc( sizeof( ABSTRACT_FILE_CONTENTS ) );
			if ( pAbstractFileContents == 0 )
				{
				RespondToError( MODULE_ABSTRACT, ABSTRACT_ERROR_INSUFFICIENT_MEMORY );
				bNoError = FALSE;
				}
			else
				{
				pAbstractFileContents -> FieldCount = 0;
				pAbstractFileContents -> pAbstractFieldArray = (ABSTRACT_FIELD*)malloc( ( nConfigItems + 1 ) * sizeof( ABSTRACT_FIELD ) );
				if ( pAbstractFileContents -> pAbstractFieldArray == 0 )
					{
					RespondToError( MODULE_ABSTRACT, ABSTRACT_ERROR_INSUFFICIENT_MEMORY );
					bNoError = FALSE;
					free( pAbstractFileContents );
					pAbstractFileContents = 0;
					}
				}
			if ( bNoError )
				{
				rewind( pCfgFile );
				do
					{
					// Read a configuration line and parse it into the appropriate member of the
					// pAbstractFileContents structure.
					FileStatus = ReadAbstractConfigItem( pCfgFile, ConfigLine, 1024 );
					if ( FileStatus == FILE_STATUS_OK && strlen( ConfigLine ) > 0 )	// Skip comment lines.
						{
						strcpy( PrevConfigLine, ConfigLine );
						bItemParsedOK = ParseAbstractConfigItem( ConfigLine, pAbstractFileContents );
						if ( !bItemParsedOK )
							{
							RespondToError( MODULE_ABSTRACT, ABSTRACT_ERROR_FILE_PARSE );
							bNoError = FALSE;
							sprintf( TextLine, "Abstract configuration line being parsed was:\n      %s", PrevConfigLine );
							LogMessage( TextLine, MESSAGE_TYPE_ERROR );
							}
						}
					}
				while ( FileStatus == FILE_STATUS_OK );

				if ( FileStatus & FILE_STATUS_READ_ERROR )
					{
					sprintf( TextLine, "Last good abstract configuration line read:\n      %s", PrevConfigLine );
					LogMessage( TextLine, MESSAGE_TYPE_ERROR );
					bNoError = FALSE;
					}
				}
			}
		fclose( pCfgFile );
		}
	else
		{
		RespondToError( MODULE_ABSTRACT, ABSTRACT_ERROR_FILE_OPEN );
		bNoError = FALSE;
		}
	if ( bNoError )
		{
		// Link the new configuration information to the list of abstract files to be created.
		pAbstractFileContents -> pNextFileContents = 0;
		if ( pAbstractFileList == 0 )
			pAbstractFileList = pAbstractFileContents;
		else
			{
			pFileContents = pAbstractFileList;
			while ( pFileContents != 0 && pFileContents -> pNextFileContents != 0 )
				pFileContents = pFileContents -> pNextFileContents;
			pFileContents -> pNextFileContents = pAbstractFileContents;
			}
		}

	return bNoError;
}


FILE_STATUS ReadAbstractConfigItem( FILE *pCfgFile, char *TextLine, long nMaxBytes )
{
	FILE_STATUS		FileStatus = FILE_STATUS_OK;
	int				SystemErrorNumber;
	char			*pChar;

	// Read the next Group and Element Tag.
	clearerr( pCfgFile );
	if ( fgets( TextLine, nMaxBytes - 1, pCfgFile ) == NULL )
		{
		if ( feof( pCfgFile ) )
			FileStatus |= FILE_STATUS_EOF;
		SystemErrorNumber = ferror( pCfgFile );
		if ( SystemErrorNumber != 0 )
			{
			FileStatus |= FILE_STATUS_READ_ERROR;
			RespondToError( MODULE_ABSTRACT, ABSTRACT_ERROR_FILE_READ );
			LogMessage( strerror( SystemErrorNumber ), MESSAGE_TYPE_ERROR );
			}
		}
	else
		{
		pChar = strchr( TextLine, '#' );		// Look for a comment delimiter in this line.
		if ( pChar != 0 )
			*pChar = '\0';						// If found, drop that portion of the string.
		TrimBlanks( TextLine );
		}

	return FileStatus;
}


BOOL ParseAbstractConfigItem( char ConfigLine[], ABSTRACT_FILE_CONTENTS *pAbstractFileContents )
{
	BOOL					bNoError = TRUE;
	char					*pChar;
	char					TextLine[ MAX_CFG_STRING_LENGTH ];
	char					*pAttributeName;
	char					*pAttributeValue;
	BOOL					bSkipLine = FALSE;
	ABSTRACT_FIELD			*pAbstractItem;
	DICOM_DICTIONARY_ITEM	*pDictItem = 0;
	TAG						DicomElementTag;

	strcpy( TextLine, ConfigLine );
	// First look for file attributes.
	if ( ( ConfigurationState & CONFIG_STATE_OUTPUT_DESIGNATED ) == 0 ||
				( ConfigurationState & CONFIG_STATE_USAGE_DESIGNATED ) == 0 )
		{
		// Look for validly formatted attribute name and value.  Find a colon or an end-of-line.
		pAttributeName = strtok( TextLine, ":\n" );
		if ( pAttributeName == NULL )
			bSkipLine = TRUE;			// If neither found, skip this line.
		if ( !bSkipLine )
			{
			pAttributeValue = strtok( NULL, "\n" );  // Point to the value following the colon.
			if ( pAttributeValue == NULL )
				{
				RespondToError( MODULE_ABSTRACT, ABSTRACT_ERROR_PARSE_ATTRIB_VALUE );
				bNoError = FALSE;
				}
			else
				TrimBlanks( pAttributeValue );
			if ( bNoError )
				{
				TrimBlanks( pAttributeName );
				if ( _stricmp( pAttributeName, "Output File" ) == 0 )
					{
					if ( ( ConfigurationState & CONFIG_STATE_OUTPUT_DESIGNATED ) == 0 )
						{
						strcpy( pAbstractFileContents -> OutputFileName, "" );
						strncat( pAbstractFileContents -> OutputFileName, pAttributeValue, MAX_FILE_SPEC_LENGTH - 1 );
						ConfigurationState |= CONFIG_STATE_OUTPUT_DESIGNATED;
						}
					else
						{
						RespondToError( MODULE_ABSTRACT, ABSTRACT_ERROR_MULTIPLE_COPIES );
						bNoError = FALSE;
						}
					}
				else if ( _stricmp( pAttributeName, "Usage" ) == 0 )
					{
					if ( ( ConfigurationState & CONFIG_STATE_USAGE_DESIGNATED ) == 0 )
						{
						if ( _stricmp( pAttributeValue, "Export" ) == 0 )
							pAbstractFileContents -> Usage = ABSTRACT_USAGE_EXPORT;
						else if ( _stricmp( pAttributeValue, "Import" ) == 0 )
							pAbstractFileContents -> Usage = ABSTRACT_USAGE_IMPORT;
						else
							{
							RespondToError( MODULE_ABSTRACT, ABSTRACT_ERROR_PARSE_ATTRIB_VALUE );
							bNoError = FALSE;
							}
						if ( bNoError )
							ConfigurationState |= CONFIG_STATE_USAGE_DESIGNATED;
						}
					else
						{
						RespondToError( MODULE_ABSTRACT, ABSTRACT_ERROR_MULTIPLE_COPIES );
						bNoError = FALSE;
						}
					}
				}
			}			// ... end if not bSkipLine.
		}			// ... end if looking for initial attributes.
	else
		{
		pChar = &ConfigLine[ 0 ];
		if ( *pChar++ == '(' )
			{
			pAbstractItem = &pAbstractFileContents -> pAbstractFieldArray[ pAbstractFileContents -> FieldCount ];
			// Attempt to convert the first numeric field to a Group Tag number, base-16.
			// Advance the character pointer to the terminating (non-numeric) character.
			pAbstractItem -> Group = (unsigned short)strtol( pChar, &pChar, 16 );
			// Next is expected a comma, separating the group and element tag fields.
			if ( *pChar++ == ',' )
				{
				// Attempt to convert the next numeric field to an Element Tag number, base-16.
				// Advance the character pointer to the terminating (non-numeric) character.
				pAbstractItem -> Element = (unsigned short)strtol( pChar, &pChar, 16 );
				}
			DicomElementTag.Group = pAbstractItem -> Group;
			DicomElementTag.Element = pAbstractItem -> Element;
			pDictItem = GetDicomElementFromDictionary( DicomElementTag );
			if ( pDictItem != 0 )
				{
				strcpy( pAbstractItem -> Description, "" );
				strncat( pAbstractItem -> Description, pDictItem ->Description, 63 );
				pAbstractFileContents -> FieldCount++;
				}
			else
				{
				RespondToError( MODULE_ABSTRACT, ABSTRACT_ERROR_ITEM_NOT_IN_DICTIONARY );
				bNoError = FALSE;
				}
			}
		}

	return bNoError;
}


BOOL OpenNewAbstractRecord()
{
	BOOL						bNoError = TRUE;
	ABSTRACT_FILE_CONTENTS		*pFileContents;
	ABSTRACT_FIELD				*pAbstractField;
	unsigned long				nAbstractField;

	// Clear the temporary text value storage for all abstract fields.
	if ( pAbstractFileList != 0 )
		{
		pFileContents = pAbstractFileList;
		// Loop through the list of abstract files to be generated.
		while ( pFileContents != 0 )
			{
			// Clear each abstract value.
			for ( nAbstractField = 0; nAbstractField < pFileContents -> FieldCount; nAbstractField++ )
				{
				pAbstractField = &pFileContents -> pAbstractFieldArray[ nAbstractField ];
				strcpy( pAbstractField -> TempTextValueStorage, "" );
				}
			pFileContents = pFileContents -> pNextFileContents;
			}
		}

	return bNoError;
}


BOOL AddNewAbstractDataElement( TAG DicomElementTag, char *pElementTextValue )
{
	BOOL						bNoError = TRUE;
	BOOL						bMatchingElementFound;
	ABSTRACT_FILE_CONTENTS		*pFileContents;
	ABSTRACT_FIELD				*pAbstractField;
	unsigned long				nAbstractField;

	if ( pAbstractFileList != 0 )
		{
		pFileContents = pAbstractFileList;
		// Loop through the list of abstract files to be generated.
		while ( pFileContents != 0 )
			{
			bMatchingElementFound = FALSE;
			for ( nAbstractField = 0; nAbstractField < pFileContents -> FieldCount && !bMatchingElementFound; nAbstractField++ )
				{
				pAbstractField = &pFileContents -> pAbstractFieldArray[ nAbstractField ];
				// If this Dicom element tag is present for the current abstract file, format it and store it.
				if ( pAbstractField -> Group == DicomElementTag.Group &&  pAbstractField -> Element == DicomElementTag.Element )
					{
					FormatCSVField( pElementTextValue, pAbstractField -> TempTextValueStorage, MAX_LOGGING_STRING_LENGTH - 1 );
					bMatchingElementFound = TRUE;
					}
				}
			pFileContents = pFileContents -> pNextFileContents;
			}
		}

	return bNoError;
}


// Create a list of output lines of text, with each line associated with the
// file into which it is to be recorded (later, after image processing).
// This list will be stored with the product info for later recording.
ABSTRACT_RECORD_TEXT_LINE *CreateNewAbstractRecords()
{
	BOOL						bNoError = TRUE;
	ABSTRACT_RECORD_TEXT_LINE	*pNewAbstractLine;
	ABSTRACT_RECORD_TEXT_LINE	*pNewAbstractLineList;
	ABSTRACT_RECORD_TEXT_LINE	*pLastListElement;
	ABSTRACT_FILE_CONTENTS		*pFileContents;
	ABSTRACT_FIELD				*pAbstractField;
	unsigned long				nAbstractField;
	unsigned long				nTotalTextLength;
	char						*pOutputTextLine;
	char						*pChar;

	pNewAbstractLineList = 0;
	pLastListElement = 0;
	// Collect all the abstract records at this level.
	if ( pAbstractFileList != 0 )
		{
		pFileContents = pAbstractFileList;
		// Loop through the list of abstract files to be generated.
		while ( pFileContents != 0 )
			{
			pNewAbstractLine = (ABSTRACT_RECORD_TEXT_LINE*)malloc( sizeof(ABSTRACT_RECORD_TEXT_LINE) );
			if ( pLastListElement == 0 )
				pNewAbstractLineList = pNewAbstractLine;
			else
				pLastListElement -> pNextAbstractRecordTextLine = pNewAbstractLine;
			pLastListElement = pNewAbstractLine;
			// Calculate the abstract line length and allocate a buffer.
			nTotalTextLength = 0L;
			for ( nAbstractField = 0; nAbstractField < pFileContents -> FieldCount; nAbstractField++ )
				{
				pAbstractField = &pFileContents -> pAbstractFieldArray[ nAbstractField ];
				nTotalTextLength += (unsigned long)strlen( pAbstractField -> TempTextValueStorage ) + 1;
				}
			pOutputTextLine = (char*)malloc( nTotalTextLength + 10 );
			if ( pNewAbstractLine != 0 && pOutputTextLine != 0 )
				{
				// Save the info about which output file this output line is to be associated with.
				pNewAbstractLine -> pAbstractFileContents = pFileContents;
				pNewAbstractLine -> pNextAbstractRecordTextLine = 0;
				pNewAbstractLine -> pAbstractRecordText = pOutputTextLine;
				// Load the buffer with the comma-separated abstract field values.
				strcpy( pOutputTextLine, "" );
				for ( nAbstractField = 0; nAbstractField < pFileContents -> FieldCount; nAbstractField++ )
					{
					pAbstractField = &pFileContents -> pAbstractFieldArray[ nAbstractField ];
					strcat( pOutputTextLine, pAbstractField -> TempTextValueStorage );
					if ( nAbstractField < pFileContents -> FieldCount - 1 )
						strcat( pOutputTextLine, "," );
					}

				// Replace any embedded end-of-line characters with "^".
				do
					{
					pChar = strchr( pOutputTextLine, '\n' );
					if ( pChar != 0 )
						*pChar = '^';
					}
				while ( pChar != 0 );
				strcat( pOutputTextLine, "\n" );
				}
			else
				{
				RespondToError( MODULE_ABSTRACT, ABSTRACT_ERROR_INSUFFICIENT_MEMORY );
				bNoError = FALSE;
				}
			pFileContents = pFileContents -> pNextFileContents;
			}
		}

	return pNewAbstractLineList;
}


BOOL OutputAbstractRecords( char *pOutputFileName, ABSTRACT_RECORD_TEXT_LINE *pAbstractLineList )
{
	BOOL						bNoError = TRUE;
	BOOL						bFileIsAStandardReferenceImage;
	ABSTRACT_RECORD_TEXT_LINE	*pAbstractLine;
	ABSTRACT_RECORD_TEXT_LINE	*pPrevAbstractLine;
	ABSTRACT_FILE_CONTENTS		*pFileContents;
	ABSTRACT_FIELD				*pAbstractField;
	unsigned long				nAbstractField;
	unsigned long				nTotalTextLength;
	char						*pTitleTextLine;
	char						*pOutputTextLine;
	char						AbstractFileSpec[ MAX_FILE_SPEC_LENGTH ];
	char						AbstractArchiveFileSpec[ MAX_FILE_SPEC_LENGTH ];
	FILE						*pAbstractFile;
	DWORD						AbstractFileSize;
	DWORD						SystemErrorCode;
	char						Msg[ 256 ];

	// Write out all the abstract records at this level.
	pAbstractLine = pAbstractLineList;
	while ( pAbstractLine != 0 )
		{
		pAbstractLine = pAbstractLineList;
		pFileContents = pAbstractLine -> pAbstractFileContents;
		pOutputTextLine = pAbstractLine -> pAbstractRecordText;
		if ( pOutputTextLine != 0 )
			{
			// Set up the abstract file specification to receive this abstract record.
			strcpy( AbstractFileSpec, "" );

			if ( pFileContents -> Usage == ABSTRACT_USAGE_EXPORT )
				{
				strncat( AbstractFileSpec, ServiceConfiguration.ExportsDirectory, MAX_FILE_SPEC_LENGTH - 1 );
				LocateOrCreateDirectory( AbstractFileSpec );	// Ensure directory exists.
				if ( AbstractFileSpec[ strlen( AbstractFileSpec ) - 1 ] != '\\' )
					strcat( AbstractFileSpec, "\\" );
				strncat( AbstractFileSpec, pFileContents -> OutputFileName,
							MAX_FILE_SPEC_LENGTH - 1 - strlen( ServiceConfiguration.AbstractsDirectory ) );
				}
			else
				{
				strncat( AbstractFileSpec, ServiceConfiguration.AbstractsDirectory, MAX_FILE_SPEC_LENGTH - 1 );
				LocateOrCreateDirectory( AbstractFileSpec );	// Ensure directory exists.
				if ( AbstractFileSpec[ strlen( AbstractFileSpec ) - 1 ] != '\\' )
					strcat( AbstractFileSpec, "\\" );
				strcpy( AbstractArchiveFileSpec, AbstractFileSpec );
				strcat( AbstractArchiveFileSpec, "Archive\\" );
				strncat( AbstractFileSpec, pOutputFileName,
							MAX_FILE_SPEC_LENGTH - 1 - strlen( AbstractFileSpec ) );
				strcpy( &AbstractFileSpec[ strlen( AbstractFileSpec ) - 4 ], ".axt" );
				}
			AbstractFileSize = GetCompressedFileSize( AbstractFileSpec, NULL );

			sprintf( Msg, "    Opening abstract file:  %s", AbstractFileSpec );
			LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
			pAbstractFile = fopen( AbstractFileSpec, "at" );
			if ( pAbstractFile != 0 )
				{
				// Prefix with a line of column headings, if this is a new file.
				if ( AbstractFileSize == 0 || AbstractFileSize == INVALID_FILE_SIZE )
					{
					nTotalTextLength = 0L;
					for ( nAbstractField = 0; nAbstractField < pFileContents -> FieldCount; nAbstractField++ )
						{
						pAbstractField = &pFileContents -> pAbstractFieldArray[ nAbstractField ];
						nTotalTextLength += (unsigned long)strlen( pAbstractField -> Description ) + 1;
						}
					pTitleTextLine = (char*)malloc( nTotalTextLength + 10 );
					if ( pTitleTextLine != 0 )
						{
						strcpy( pTitleTextLine, "" );
						for ( nAbstractField = 0; nAbstractField < pFileContents -> FieldCount; nAbstractField++ )
							{
							pAbstractField = &pFileContents -> pAbstractFieldArray[ nAbstractField ];
							strcat( pTitleTextLine, pAbstractField -> Description );
							if ( nAbstractField < pFileContents -> FieldCount - 1 )
								strcat( pTitleTextLine, "," );
							}
						strcat( pTitleTextLine, "\n" );
						fputs( pTitleTextLine, pAbstractFile );
						}
					}
				// Output the abstract value sequence line.
				fputs( pOutputTextLine, pAbstractFile );
				fclose( pAbstractFile );
				sprintf( Msg, "    Closed abstract file:  %s", AbstractFileSpec );
				LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
				bFileIsAStandardReferenceImage = IsFileAStandardReferenceImage( pOutputFileName );
				if ( ServiceConfiguration.bArchiveAXTOuputFiles && !bFileIsAStandardReferenceImage )
					{
					LocateOrCreateDirectory( AbstractArchiveFileSpec );	// Ensure directory exists.
					strncat( AbstractArchiveFileSpec, pOutputFileName,
							MAX_FILE_SPEC_LENGTH - 1 - strlen( AbstractArchiveFileSpec ) );
					strcpy( &AbstractArchiveFileSpec[ strlen( AbstractArchiveFileSpec ) - 4 ], ".axt" );
				
					sprintf( Msg, "    Copying abstract file:  %s to the archive folder", AbstractFileSpec );
					LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
					bNoError = CopyFile( AbstractFileSpec, AbstractArchiveFileSpec, FALSE );
					if ( !bNoError )
						{
						SystemErrorCode = GetLastError();
						sprintf( Msg, "   >>> Copy to AXT archive system error code %d", SystemErrorCode );
						LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
						}
					}
				}
			else
				{
				RespondToError( MODULE_ABSTRACT, ABSTRACT_ERROR_ABSTRACT_FILE_OPEN );
				bNoError = FALSE;
				}
			free( pOutputTextLine );
			}
		else
			{
			RespondToError( MODULE_ABSTRACT, ABSTRACT_ERROR_INSUFFICIENT_MEMORY );
			bNoError = FALSE;
			}
		pPrevAbstractLine = pAbstractLine;
		pAbstractLine = pAbstractLine -> pNextAbstractRecordTextLine;
		free( pPrevAbstractLine );
		}

	return bNoError;
}


void DeallocateAbstractConfiguration()
{
	ABSTRACT_FILE_CONTENTS		*pFileContents;
	ABSTRACT_FILE_CONTENTS		*pPrevFileContents;
	
	if ( pAbstractFileList != 0 )
		{
		pFileContents = pAbstractFileList;
		while ( pFileContents != 0 )
			{
			if ( pFileContents -> pAbstractFieldArray != 0 )
				free( pFileContents -> pAbstractFieldArray );
			pPrevFileContents = pFileContents;
			pFileContents = pFileContents -> pNextFileContents;
			free( pPrevFileContents );
			}
		pAbstractFileList = 0;
		}
}


typedef struct
	{
	char			*pStandardDisplayName;
	char			*pStandardName;
	char			*pDicomName;
	} STANDARD_FILE_NAME;

STANDARD_FILE_NAME	StandardFileNameTable[] =
	{
	{ "Ex1", "0example1", "1.2.804.114118.2.20100209.174742.3502332353.1.1.1" },
	{ "Ex2", "0example2", "1.2.804.114118.2.20100209.174742.3502327970.1.1.1" },
	{ "1p ", "1p", "1.2.804.114118.2.20100209.174742.3502336264.1.1.1" },
	{ "1q ", "1q", "1.2.804.114118.2.20100209.174742.3502352654.1.1.1" },
	{ "1r ", "1r", "1.2.804.114118.2.20100209.174742.3502370719.1.1.1" },
	{ "1s ", "1s", "1.2.804.114118.2.20100209.174742.3502392181.1.1.1" },
	{ "1t ", "1t", "1.2.804.114118.2.20100209.174742.3502418113.1.1.1" },
	{ "2p ", "2p", "1.2.804.114118.2.20100209.174742.3502341143.1.1.1" },
	{ "2q ", "2q", "1.2.804.114118.2.20100209.174742.3502358083.1.1.1" },
	{ "2r ", "2r", "1.2.804.114118.2.20100209.174742.3502377247.1.1.1" },
	{ "2s ", "2s", "1.2.804.114118.2.20100209.174742.3502400586.1.1.1" },
	{ "2t ", "2t", "1.2.804.114118.2.20100209.174742.3502428272.1.1.1" },
	{ "3p ", "3p", "1.2.804.114118.2.20100209.174742.3502346423.1.1.1" },
	{ "3q ", "3q", "1.2.804.114118.2.20100209.174742.3502364494.1.1.1" },
	{ "3r ", "3r", "1.2.804.114118.2.20100209.174742.3502384231.1.1.1" },
	{ "3s ", "3s", "1.2.804.114118.2.20100209.174742.3502409397.1.1.1" },
	{ "3t ", "3t", "1.2.804.114118.2.20100209.174742.3502437834.1.1.1" },
	{ "A  ", "A", "1.2.804.114118.2.20100209.174742.3502450791.1.1.1" },
	{ "B  ", "B", "1.2.804.114118.2.20100209.174742.3502462440.1.1.1" },
	{ "C  ", "C", "1.2.804.114118.2.20100209.174742.3502474306.1.1.1" },
	{ "plu", "quad_calcification_thickening", "1.2.124.113532.172.22.102.206.20100907.112906.476870" },
	{ "u", "quad_u", "1.2.804.114118.2.20100309.162411.3030936179.1.1.1" },
	{ 0, 0, 0 }
	};


STANDARD_FILE_NAME *GetStandardFileName( char *pFoundDicomFileName )
{
	STANDARD_FILE_NAME		*pFileRenameEntry;
	int						nEntry;
	BOOL					bMatchFound;
	BOOL					bEndOfTable;
	
	nEntry = 0;
	bMatchFound = FALSE;
	bEndOfTable = FALSE;

	while ( !bMatchFound && !bEndOfTable )
		{
		pFileRenameEntry = &StandardFileNameTable[ nEntry ];
		bEndOfTable = ( pFileRenameEntry -> pDicomName == 0 );
		if ( !bEndOfTable )
			{
			bMatchFound = ( strncmp( pFileRenameEntry -> pDicomName, pFoundDicomFileName, strlen( pFileRenameEntry -> pDicomName ) ) == 0 );
			nEntry++;
			}
		}
	if ( !bMatchFound )
		pFileRenameEntry = 0;

	return pFileRenameEntry;
}


BOOL IsFileAStandardReferenceImage(  char *pDicomFileName )
{
	STANDARD_FILE_NAME			*pFileRenameEntry;
	BOOL						bFileIsAStandardReferenceImage;

	pFileRenameEntry = GetStandardFileName( pDicomFileName );
	bFileIsAStandardReferenceImage = ( pFileRenameEntry != 0 );

	return bFileIsAStandardReferenceImage;
}


