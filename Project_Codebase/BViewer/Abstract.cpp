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

	strcpy( AbstractsDirectory, "" );
	strncat( AbstractsDirectory, BViewerConfiguration.AbstractsDirectory, FULL_FILE_SPEC_STRING_LENGTH );
	if ( AbstractsDirectory[ strlen( AbstractsDirectory ) - 1 ] != '\\' )
		strcat( AbstractsDirectory, "\\" );
	strcpy( BRetrieverAbstractFileSpec, AbstractsDirectory );
	strcat( BRetrieverAbstractFileSpec, "*.axt" );

	hFindFile = FindFirstFile( BRetrieverAbstractFileSpec, &FindFileInfo );
	bFileFound = ( hFindFile != INVALID_HANDLE_VALUE );
	if ( hFindFile != INVALID_HANDLE_VALUE )
		FindClose( hFindFile );

	return bFileFound;
	}


// Called from BViewer.cpp::ReadNewAbstractData(), from the TimerThreadFunction().
// NOTE:  Avoid printing or displaying from a timer thread function.
BOOL ImportNewAbstractData( ABSTRACT_EXTRACTION_FUNCTION ProcessingFunction )
{
	BOOL						bNoError = TRUE;
	char						AbstractsDirectory[ FULL_FILE_SPEC_STRING_LENGTH ];
	char						BViewerAbstractFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char						SearchAbstractFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char						TextLine[ 1096 ];
	DWORD						SystemErrorCode;
	WIN32_FIND_DATA				FindFileInfo;
	HANDLE						hFindFile;
	BOOL						bFileFound;
	char						Msg[ 256 ];

	strcpy( AbstractsDirectory, "" );
	strncat( AbstractsDirectory, BViewerConfiguration.AbstractsDirectory, FULL_FILE_SPEC_STRING_LENGTH );
	if ( AbstractsDirectory[ strlen( AbstractsDirectory ) - 1 ] != '\\' )
		strcat( AbstractsDirectory, "\\" );
	strcpy( SearchAbstractFileSpec, AbstractsDirectory );
	strcat( SearchAbstractFileSpec, "*.axt" );

	// Remove the previously processed file, if it exists.
	DeleteFile( BViewerAbstractFileSpec );

	hFindFile = FindFirstFile( SearchAbstractFileSpec, &FindFileInfo );
	bFileFound = ( hFindFile != INVALID_HANDLE_VALUE );
	while ( bFileFound )
		{
		strcpy( BViewerAbstractFileSpec, AbstractsDirectory );
		strcat( BViewerAbstractFileSpec, FindFileInfo.cFileName );
		sprintf( Msg, "    Reading abstract file %s.", BViewerAbstractFileSpec );
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );

		// Process the selection list data from the newly found .axt file.
		bNoError = ReadAbstractDataFile( BViewerAbstractFileSpec, ProcessingFunction );

		if ( bNoError )
			{
			bNoError = DeleteFile( BViewerAbstractFileSpec );
			if ( !bNoError )
				{
				sprintf( TextLine, "Error deleting %s", BViewerAbstractFileSpec );
				LogMessage( TextLine, MESSAGE_TYPE_ERROR );
				SystemErrorCode = GetLastError();
				if ( SystemErrorCode == 997 )
					strcpy( TextLine, "The source file is still volatile or the destination doesn't have write access." );
				else
					sprintf( TextLine, "System error code %d", SystemErrorCode );
				LogMessage( TextLine, MESSAGE_TYPE_ERROR );
				}
			}
		else
			LogMessage( ">>> Error reading this abstract file.", MESSAGE_TYPE_SUPPLEMENTARY );
		// Look for another file in the .axt import directory.
		bFileFound = FindNextFile( hFindFile, &FindFileInfo );
		}
	if ( hFindFile != INVALID_HANDLE_VALUE )
		FindClose( hFindFile );

	return bNoError;
}


BOOL ReadAbstractDataFile( char *AbstractConfigurationFileSpec, ABSTRACT_EXTRACTION_FUNCTION ProcessingFunction )
{
	BOOL						bNoError = TRUE;
	FILE						*pAbstractFile;
	FILE_STATUS					FileStatus = FILE_STATUS_READ_ERROR;
	char						TextLine[ MAX_LOGGING_STRING_LENGTH ];
	char						NewAbstractTitleLine[ MAX_AXT_LINE_LENGTH ];
	char						NewAbstractDataLine[ MAX_AXT_LINE_LENGTH ];
	char						PrevNewAbstractDataLine[ MAX_AXT_LINE_LENGTH ];
	long						nNewAbstractItems;
	char						Msg[ 256 ];
	
	pAbstractFile = fopen( AbstractConfigurationFileSpec, "rt" );
	if ( pAbstractFile != 0 )
		{
		sprintf( Msg, "    %s opened successfully.", AbstractConfigurationFileSpec );
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
		nNewAbstractItems = 0L;
		// Read the first line, which is the list of column titles.
		sprintf( Msg, "    %s:  Reading column titles.", AbstractConfigurationFileSpec );
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
		FileStatus = ReadAbstractDataLine( pAbstractFile, NewAbstractTitleLine, MAX_AXT_LINE_LENGTH );
		// Count the number of items in the abstract configuration file.
		if ( FileStatus == FILE_STATUS_OK )
			do
				{
				sprintf( Msg, "    %s:  Reading new study info.", AbstractConfigurationFileSpec );
				LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
				FileStatus = ReadAbstractDataLine( pAbstractFile, NewAbstractDataLine, MAX_AXT_LINE_LENGTH );
				if ( FileStatus == FILE_STATUS_OK && strlen( NewAbstractDataLine ) > 0 )
					{
					strcpy( PrevNewAbstractDataLine, NewAbstractDataLine );
					sprintf( Msg, "    %s:  Processing new study data row.", AbstractConfigurationFileSpec );
					LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );

					ProcessingFunction( NewAbstractTitleLine, NewAbstractDataLine );
					}
				}
			while ( bNoError && FileStatus == FILE_STATUS_OK );
		if ( !bNoError || ( FileStatus & FILE_STATUS_READ_ERROR ) )
			{
			sprintf( TextLine, "Last good abstract data line read:\n      %s", PrevNewAbstractDataLine );
			LogMessage( TextLine, MESSAGE_TYPE_ERROR );
			}
		sprintf( Msg, "    Closing abstract file %s.", AbstractConfigurationFileSpec );
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
		fclose( pAbstractFile );
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

	if ( fgets( TextLine, nMaxBytes - 1, pAbstractFile ) == NULL )
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

	return FileStatus;
}


BOOL GetAbstractColumnValueForSpecifiedField( char *pDesiredFieldDescription,
										char *pNewAbstractTitleLine, char *pNewAbstractDataLine, char *pValue )
{
	char					FieldDescription[ 128 ];
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
		strcpy( FieldDescription, "" );
		if ( nTitleChars > 0 )
			strncat( FieldDescription, pTitleField, nTitleChars );

		// Decode the value field and return a pointer to the trailing delimiter in the
		// input string.
		pValueField = DecodeCSVField( pValueField, FieldValue, 2047 );
		if ( pValueField != 0 )
			{
			if ( _stricmp( FieldDescription, pDesiredFieldDescription ) == 0 )
				{
				bMatchingFieldFound = TRUE;
				strcpy( pValue, FieldValue );
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



