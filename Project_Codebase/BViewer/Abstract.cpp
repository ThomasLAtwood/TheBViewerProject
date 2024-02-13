// Abstract.cpp - Implements the functions and data structures that marshal the 
// Dicom data elements from the image file and package it as comma-separated
// values in "abstracted" text files.
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
//	*[2] 01/30/2024 by Tom Atwood
//		Tidied up call to fgets() so it conforms exactly to the Windows prototype.
//		Fixed a race condition that occurred in deleting the abstract file.
//	*[1] 07/17/2022 by Tom Atwood
//		Fixed code security issues.
//
//
#include "Module.h"
#include <process.h>
#include "ReportStatus.h"
#include "Configuration.h"
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
				{ 0										, NULL }
			};


static ERROR_DICTIONARY_MODULE		AbstractStatusErrorDictionary =
										{
										MODULE_ABSTRACT,
										AbstractErrorCodes,
										ABSTRACT_ERROR_DICT_LENGTH,
										0
										};

unsigned long						MaxRowIndex = 0;

static	BOOL						bTimerHasBeenStarted = FALSE;
static	BOOL						bTerminateTimer = FALSE;
static	BOOL						bTimerHasTerminated = FALSE;

extern CONFIGURATION				BViewerConfiguration;


// This function must be called before any other function in this module.
void InitAbstractModule()
{
	LinkModuleToList( &AbstractModuleInfo );
	RegisterErrorDictionary( &AbstractStatusErrorDictionary );
}


void CloseAbstractModule()
{
	bTerminateTimer = TRUE;
	while ( bTimerHasBeenStarted && !bTimerHasTerminated )
		Sleep( 500 );
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
								( strchr( pInputString, 0x0D ) != 0 ) ||
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
		if ( *pInputChar == '\"' )			// If a double quote character is encountered...
			{
			*pOutputChar = '\"';			// add a second double quote as an excape character.
			pOutputChar++;
			nOutputBufferChars--;
			}
		// For export, replace end of line characters with spaces.
		else if ( *pInputChar == 0x0A || *pInputChar == 0x0D || *pInputChar == '\n' )
			*pInputChar = ' ';
		// Copy the current character from the input to the output and advance to the next character.
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
char *DecodeCSVField( char *pInputString, char *pOutputString, unsigned short nOutputBufferChars )
{
	BOOL			bNoError = TRUE;
	BOOL			bEntireFieldIsQuoted;
	BOOL			bFieldTerminatorLocated;
	char			*pInputChar;
	char			*pOutputChar;

	bFieldTerminatorLocated = FALSE;
	pInputChar = pInputString;
	pOutputChar = pOutputString;
	bEntireFieldIsQuoted = ( *pInputChar == '\"' );
	if ( bEntireFieldIsQuoted )
		pInputChar++;			// Skip the opening quote.
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
			else if ( bEntireFieldIsQuoted )
				{
				pInputChar++;					// Skip the quote mark.
				if ( *pInputChar == ',' || *pInputChar == 0 )
					bFieldTerminatorLocated = TRUE;
				else
					bNoError = FALSE;
				}
			else
				bNoError = FALSE;
			}
		else
			{
			if ( !bEntireFieldIsQuoted && ( *pInputChar == ',' || *pInputChar == 0 ) )
				bFieldTerminatorLocated = TRUE;
			else
				{
				*pOutputChar = *pInputChar;			// Copy input character to the output.
				pInputChar++;
				pOutputChar++;
				nOutputBufferChars--;
				}
			}
		if ( nOutputBufferChars == 0 )
			pOutputChar--;				// Back up one character.
		*pOutputChar = '\0';			// Preload the string terminator.
		}			// ...end while another input character remains.
	if ( !bNoError )
		pInputChar = 0;

	return pInputChar;
}


// NOTE:  Avoid output from a timer thread function.
BOOL NewAbstractDataAreAvailable()
{
	char						AbstractsDirectory[ FULL_FILE_SPEC_STRING_LENGTH ];
	char						BRetrieverAbstractFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	WIN32_FIND_DATA				FindFileInfo;
	HANDLE						hFindFile;
	BOOL						bFileFound;

	strncpy_s( AbstractsDirectory, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.AbstractsDirectory, _TRUNCATE );		// *[1] Replaced strncat with strncpy_s.
	if ( AbstractsDirectory[ strlen( AbstractsDirectory ) - 1 ] != '\\' )
		strncat_s( AbstractsDirectory, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );										// *[1] Replaced strcat with strncat_s.
	strncpy_s( BRetrieverAbstractFileSpec, FULL_FILE_SPEC_STRING_LENGTH, AbstractsDirectory, _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
	strncat_s( BRetrieverAbstractFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "*.axt", _TRUNCATE );								// *[1] Replaced strcat with strncat_s.

	hFindFile = FindFirstFile( BRetrieverAbstractFileSpec, &FindFileInfo );
	bFileFound = ( hFindFile != INVALID_HANDLE_VALUE );
	if ( hFindFile != INVALID_HANDLE_VALUE )
		FindClose( hFindFile );

	return bFileFound;
	}


// Called from BViewer.cpp::ReadNewAbstractData(), from the TimerThreadFunction().
// NOTE:  Avoid printing or displaying from a timer thread function.
void ImportNewAbstractData( ABSTRACT_EXTRACTION_FUNCTION ProcessingFunction )												// *[2] Changed unused BOOL return typo to void.
{
	BOOL						bNoError = TRUE;
	char						AbstractsDirectory[ FULL_FILE_SPEC_STRING_LENGTH ];
	char						BViewerAbstractFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char						SearchAbstractFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	WIN32_FIND_DATA				FindFileInfo;
	HANDLE						hFindFile;
	BOOL						bFileFound;
	char						Msg[ FILE_PATH_STRING_LENGTH ];

	strncpy_s( AbstractsDirectory, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.AbstractsDirectory, _TRUNCATE );						// *[1] Replaced strncat with strncpy_s.
	if ( AbstractsDirectory[ strlen( AbstractsDirectory ) - 1 ] != '\\' )
		strncat_s( AbstractsDirectory, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );														// *[1] Replaced strcat with strncat_s.
	strncpy_s( SearchAbstractFileSpec, FULL_FILE_SPEC_STRING_LENGTH, AbstractsDirectory, _TRUNCATE );										// *[1] Replaced strcpy with strncpy_s.
	strncat_s( SearchAbstractFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "*.axt", _TRUNCATE );													// *[1] Replaced strcat with strncat_s.

	// Remove the previously processed file, if it exists.
	DeleteFile( BViewerAbstractFileSpec );

	hFindFile = FindFirstFile( SearchAbstractFileSpec, &FindFileInfo );
	bFileFound = ( hFindFile != INVALID_HANDLE_VALUE );
	while ( bFileFound )
		{
		strncpy_s( BViewerAbstractFileSpec, FULL_FILE_SPEC_STRING_LENGTH, AbstractsDirectory, _TRUNCATE );									// *[1] Replaced strcpy with strncpy_s.
		strncat_s( BViewerAbstractFileSpec, FULL_FILE_SPEC_STRING_LENGTH, FindFileInfo.cFileName, strlen( FindFileInfo.cFileName ) );		// *[1] Replaced strcat with strncat_s.
		sprintf_s( Msg, FILE_PATH_STRING_LENGTH, "    Reading abstract file %s.", BViewerAbstractFileSpec );								// *[1] Replaced sprintf with sprintf_s.
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );

		// Process the selection list data from the newly found .axt file.
		bNoError = ReadAbstractDataFile( BViewerAbstractFileSpec, ProcessingFunction );
		if ( !bNoError )																													// *[2] Moved the file deletion into ReadAbstractDataFile().
			LogMessage( ">>> Error reading or subsequently deleting this abstract file.", MESSAGE_TYPE_SUPPLEMENTARY );						// *[2] Included deletion errors in this message.
		// Look for another file in the .axt import directory.
		bFileFound = FindNextFile( hFindFile, &FindFileInfo );
		}
	if ( hFindFile != INVALID_HANDLE_VALUE )
		FindClose( hFindFile );

	return;
}


BOOL ReadAbstractDataFile( char *AbstractConfigurationFileSpec, ABSTRACT_EXTRACTION_FUNCTION ProcessingFunction )
{
	BOOL						bNoError = TRUE;
	FILE						*pAbstractFile;
	FILE_STATUS					FileStatus = FILE_STATUS_READ_ERROR;
	char						TextLine[ MAX_SUPER_EXTRA_LONG_STRING_LENGTH ];
	char						NewAbstractTitleLine[ MAX_AXT_LINE_LENGTH ];
	char						NewAbstractDataLine[ MAX_AXT_LINE_LENGTH ];
	char						PrevNewAbstractDataLine[ MAX_AXT_LINE_LENGTH ];
	char						Msg[ FILE_PATH_STRING_LENGTH ];
	int							Result;
	DWORD						SystemErrorCode;
	
	pAbstractFile = fopen( AbstractConfigurationFileSpec, "rtD" );																			// *[2] Added "D" to eliminate a race condition involving file deletion.
	if ( pAbstractFile != 0 )
		{
		sprintf_s( Msg, FILE_PATH_STRING_LENGTH, "    %s opened successfully.", AbstractConfigurationFileSpec );							// *[1] Replaced sprintf with sprintf_s.
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
		// Read the first line, which is the list of column titles.
		sprintf_s( Msg, FILE_PATH_STRING_LENGTH, "    %s:  Reading column titles.", AbstractConfigurationFileSpec );						// *[1] Replaced sprintf with sprintf_s.
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
		FileStatus = ReadAbstractDataLine( pAbstractFile, NewAbstractTitleLine, MAX_AXT_LINE_LENGTH );
		// Count the number of items in the abstract configuration file.
		if ( FileStatus == FILE_STATUS_OK )
			do
				{
				sprintf_s( Msg, FILE_PATH_STRING_LENGTH, "    %s:  Reading new study info.", AbstractConfigurationFileSpec );				// *[1] Replaced sprintf with sprintf_s.
				LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
				FileStatus = ReadAbstractDataLine( pAbstractFile, NewAbstractDataLine, MAX_AXT_LINE_LENGTH );
				if ( FileStatus == FILE_STATUS_OK && strlen( NewAbstractDataLine ) > 0 )
					{
					strncpy_s( PrevNewAbstractDataLine, MAX_AXT_LINE_LENGTH, NewAbstractDataLine, _TRUNCATE );								// *[1] Replaced strcpy with strncpy_s.
					sprintf_s( Msg, FILE_PATH_STRING_LENGTH, "    %s:  Processing new study data row.", AbstractConfigurationFileSpec );	// *[1] Replaced sprintf with sprintf_s.
					LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );

					ProcessingFunction( NewAbstractTitleLine, NewAbstractDataLine );
					}
				}
			while ( FileStatus == FILE_STATUS_OK );																							// *[2] Removed spurious test of bNoError.
		bNoError = ( ( FileStatus & FILE_STATUS_READ_ERROR ) != 0 );																		// *[2] Added explicit error condition.
		if ( !bNoError )																													// *[2] Removed test that is now redundant.
			{
			sprintf_s( TextLine, MAX_SUPER_EXTRA_LONG_STRING_LENGTH, "Last good abstract data line read:\n      %s", PrevNewAbstractDataLine );	// *[1] Replaced sprintf with sprintf_s.
			LogMessage( TextLine, MESSAGE_TYPE_ERROR );
			}
		sprintf_s( Msg, FILE_PATH_STRING_LENGTH, "    Closing abstract file %s.", AbstractConfigurationFileSpec );							// *[1] Replaced sprintf with sprintf_s.
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );

		// *[2] Delete the abstract file and check for any deletion error.
		Result = fclose( pAbstractFile );
		if ( Result != 0 )
			{
			sprintf_s( TextLine, MAX_SUPER_EXTRA_LONG_STRING_LENGTH, "Error deleting %s", AbstractConfigurationFileSpec );
			LogMessage( TextLine, MESSAGE_TYPE_ERROR );
			SystemErrorCode = GetLastSystemErrorMessage( TextLine, MAX_SUPER_EXTRA_LONG_STRING_LENGTH );
			sprintf_s( TextLine, MAX_SUPER_EXTRA_LONG_STRING_LENGTH, "System error code %d:  %s", SystemErrorCode, TextLine );
			LogMessage( TextLine, MESSAGE_TYPE_ERROR );
			}
		}
	else
		{
		RespondToError( MODULE_ABSTRACT, ABSTRACT_ERROR_FILE_OPEN );
		bNoError = FALSE;
		}

	return bNoError;
}


FILE_STATUS ReadAbstractDataLine( FILE *pAbstractFile, char *TextLine, long nMaxBytes )
{
	FILE_STATUS		FileStatus = FILE_STATUS_OK;
	int				SystemErrorNumber;

	if ( nMaxBytes > 0 )
		{
		if ( fgets( TextLine, (int)nMaxBytes, pAbstractFile ) == NULL )			// *[2] Add cast to the buffer size and extend it one byte.
			{
			if ( feof( pAbstractFile ) )
				FileStatus |= FILE_STATUS_EOF;
			SystemErrorNumber = ferror( pAbstractFile );
			if ( SystemErrorNumber != 0 )
				{
				FileStatus |= FILE_STATUS_READ_ERROR;
				RespondToError( MODULE_ABSTRACT, ABSTRACT_ERROR_FILE_READ );
				LogMessage( strerror( SystemErrorNumber ), MESSAGE_TYPE_ERROR );
				}
			}
		else
			TrimBlanks( TextLine );
		}
	else
		TextLine[ 0 ] = '\0';

	return FileStatus;
}


// *[1] Added unsigned int nValueChars argument to specify the buffer size for the returned character string.
BOOL GetAbstractColumnValueForSpecifiedField( char *pDesiredFieldDescription,
										char *pNewAbstractTitleLine, char *pNewAbstractDataLine, char *pValue, unsigned int nValueChars )
{
	char					FieldDescription[ MAX_CFG_STRING_LENGTH ];
	char					FieldValue[ 2048 ];
	char					*pTitleField;
	char					*pValueField;
	char					*pTitleComma;
	int						nTitleChars;
	BOOL					bMatchingFieldFound;

	bMatchingFieldFound = FALSE;
	pTitleField = pNewAbstractTitleLine;
	pValueField = pNewAbstractDataLine;
	do
		{
		pTitleComma = strchr( pTitleField, ',' );
		if ( pTitleComma != 0 )
			nTitleChars = (int)( pTitleComma - pTitleField );
		else
			nTitleChars = (int)strlen( pTitleField );
		if ( nTitleChars > 127 )
			nTitleChars = 127;
		if ( nTitleChars > 0 )
			strncpy_s( FieldDescription, MAX_CFG_STRING_LENGTH, pTitleField, nTitleChars );		// *[1] Replaced strncat with strncpy_s.

		// Decode the value field and return a pointer to the trailing delimiter in the
		// input string.
		pValueField = DecodeCSVField( pValueField, FieldValue, 2047 );
		if ( pValueField != 0 )
			{
			if ( _stricmp( FieldDescription, pDesiredFieldDescription ) == 0 )
				{
				bMatchingFieldFound = TRUE;
				strncpy_s( pValue, nValueChars, FieldValue, _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.
				}
			pTitleField = pTitleComma + 1;
			pValueField += 1;			// Point past the trailing delimiter.
			}
		else
			pTitleComma = 0;			// Exit the loop after a decoding error.
		}
	while ( !bMatchingFieldFound && pTitleComma != 0 );

	return bMatchingFieldFound;
}



