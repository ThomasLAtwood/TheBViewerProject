// ReportStatus.cpp : Implements the functions that handle the status and error reporting.
//
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
#include "stdafx.h"
#include "Module.h"
#include "ReportStatus.h"
#include "Configuration.h"
#include "ServiceController.h"


extern SERVICE_DESCRIPTOR			ServiceDescriptor;


//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		StatusModuleInfo = { MODULE_STATUS, "Status Module", InitStatusModule, CloseStatusModule };

static ERROR_DICTIONARY_ENTRY	StatusErrorCodes[] =
			{
				{ STATUS_ERROR_CREATE_SEMAPHORE			, "An error occurred creating the status reporting semaphore." },
				{ 0										, NULL }
			};

static ERROR_DICTIONARY_MODULE		ReportStatusErrorDictionary =
										{
										MODULE_STATUS,
										StatusErrorCodes,
										STATUS_ERROR_DICT_LENGTH,
										0
										};

static ERROR_DICTIONARY_MODULE		*pErrorDictionaryModuleList = 0;


HANDLE						hStatusSemaphore = 0;
static char					*pSemaphoreName = "StatusSemaphore";


// This function must be called before any other function in this module.
void InitStatusModule()
{
	DWORD					SystemErrorCode;
	char					TextLine[ 128 ];

	LinkModuleToList( &StatusModuleInfo );
	RegisterErrorDictionary( &ReportStatusErrorDictionary );
	// Create a semaphore for controlling access to the status module from
	// different (competing) threads.
	hStatusSemaphore = CreateSemaphore( NULL, 1L, 1L, pSemaphoreName );
	if ( hStatusSemaphore == NULL )
		{
		// Note:  The semaphore creation will fail if another copy of this program is already running.
		// The following error message won't be logged, because the semaphore won't grant access.
		RespondToError( MODULE_STATUS, STATUS_ERROR_CREATE_SEMAPHORE );
		SystemErrorCode = GetLastError();
		sprintf( TextLine, "Status semaphore creation: system error code = %d", SystemErrorCode );
		LogMessage( TextLine, MESSAGE_TYPE_ERROR );
		}
}


// This function must be called to deallocate memory and close this module.
void CloseStatusModule()
{
	CloseHandle( hStatusSemaphore );
}


unsigned GetLoggingDetail()
{
	return ServiceDescriptor.LoggingDetail;
}


// Add the local error message entry for each module to the list.  This way,
// ReportStatus.cpp doesn't have to know what they are until run time.  Make sure
// the ERROR_DICTIONARY_MODULE structure referenced by pNewErrorDictionaryModule is
// hard coded, since no entry deallocation logic is supplied.
void RegisterErrorDictionary( ERROR_DICTIONARY_MODULE *pNewErrorDictionaryModule )
{
	ERROR_DICTIONARY_MODULE		*pModuleItem;

	// Add the new module reference to the end of the list.
	if ( pErrorDictionaryModuleList == 0 )
		pErrorDictionaryModuleList = pNewErrorDictionaryModule;
	else
		{
		pModuleItem = pErrorDictionaryModuleList;
		// Sequence through the list to find the terminal node.
		while ( pModuleItem -> pNextModule != 0 )
			pModuleItem = pModuleItem -> pNextModule;
		// Add the new module reference to the list.
		pModuleItem -> pNextModule = pNewErrorDictionaryModule;
		}
	// Terminate the list.
	pNewErrorDictionaryModule -> pNextModule = 0;
}


void RespondToError( unsigned long nModuleIndex, unsigned ErrorCode )
{
	char						*pModuleName;
	char						*pMessageText;
	char						ErrorMessage[ 1096 ];
	char						TextMsg[256];
	ERROR_DICTIONARY_ENTRY		*pDictEntry;
	time_t						CurrentSystemTime;
	double						TimeDifferenceInSeconds;
	unsigned					LoggingDetailSelection;
	BOOL						bDisallowMessageRepetitionLimits;

	pModuleName = GetModuleName( nModuleIndex );
	pDictEntry = GetMessageFromDictionary( nModuleIndex, ErrorCode );
	time( &CurrentSystemTime );
	LoggingDetailSelection = GetLoggingDetail();
	// Only enable error message repetition limits if the logging mode is normal.
	// If we're looking for problems, we don't want to limit the messages.
	bDisallowMessageRepetitionLimits = ( LoggingDetailSelection != CONFIG_LOGGING_NORMAL );

	if ( pDictEntry -> LogRepetitionCount >= MAX_MESSAGE_REPETITIONS )
		{
		TimeDifferenceInSeconds = difftime( CurrentSystemTime, pDictEntry -> LastLogTime );
		if ( TimeDifferenceInSeconds > REPETITION_RESET_IN_SECONDS )		// If it has been awhile since the last message, reset the count.
			pDictEntry -> LogRepetitionCount = 0;
		}
	if ( bDisallowMessageRepetitionLimits || pDictEntry -> LogRepetitionCount < MAX_MESSAGE_REPETITIONS )
		{
		pMessageText = pDictEntry -> pErrorMessage;
		// Log the error message.
		sprintf( ErrorMessage, ">>> %s Error:   ", pModuleName );
		strcat( ErrorMessage, pMessageText );
		if ( !bDisallowMessageRepetitionLimits && pDictEntry -> LogRepetitionCount == MAX_MESSAGE_REPETITIONS - 1 )
			{
			sprintf( TextMsg, "\n                                 (Message repetition suspended for %d minutes.)",
																						(int)(0.5 + REPETITION_RESET_IN_SECONDS / 60.0 ) );
			strcat( ErrorMessage, TextMsg );
			}
		LogMessage( ErrorMessage, MESSAGE_TYPE_ERROR );
		pDictEntry -> LastLogTime = CurrentSystemTime;
		}
	pDictEntry -> LogRepetitionCount++;
}


ERROR_DICTIONARY_ENTRY *GetMessageFromDictionary( unsigned long nModuleIndex, unsigned MessageCode )
{
	long						nDictEntry;
	BOOL						bEntryFound;
	BOOL						bEndOfList;
	ERROR_DICTIONARY_MODULE		*pDictionaryModule;
	ERROR_DICTIONARY_ENTRY		*pDictEntry = 0;

	// Locate the required error message dictionary module.
	bEntryFound = FALSE;
	pDictionaryModule = pErrorDictionaryModuleList;
	while ( !bEntryFound && pDictionaryModule != 0 )
		{
		if ( pDictionaryModule -> nModuleIndex == nModuleIndex )
			bEntryFound = TRUE;
		else
			pDictionaryModule = pDictionaryModule -> pNextModule;
		}
	
	if ( bEntryFound )
		{
		nDictEntry = 0;
		bEntryFound = FALSE;
		bEndOfList = FALSE;
		do
			{
			pDictEntry = &( pDictionaryModule -> pFirstDictionaryEntry[ nDictEntry ] );
			bEndOfList = ( pDictEntry -> ErrorCode == 0 );
			if ( pDictEntry -> ErrorCode == MessageCode )
				bEntryFound = TRUE;
			nDictEntry++;
			}
		while ( !bEntryFound && !bEndOfList );
		}

	return pDictEntry;
}


#define MAX_LOG_FILE_SIZE_IN_BYTES			2000000		// Limit log file size to 2 megabytes.

// When the log file size starts getting ponderous, rename and save it and start a new one.
void CheckForLogFileRotation( char *pFullLogFileSpecification )
{
	WIN32_FIND_DATA			FindFileInfo;
	HANDLE					hFindFile;
	BOOL					bFileFound;
	unsigned long			nFileSizeInBytes;
	char					OldLogFileSpec[ 256 ];
	char					NewLogFileSpec[ 256 ];

	hFindFile = FindFirstFile( pFullLogFileSpecification, &FindFileInfo );
	bFileFound = ( hFindFile != INVALID_HANDLE_VALUE );
	if ( bFileFound && FindFileInfo.nFileSizeLow )
		{
		nFileSizeInBytes = FindFileInfo.nFileSizeLow;
		if ( nFileSizeInBytes > MAX_LOG_FILE_SIZE_IN_BYTES )
			{
			strcpy( NewLogFileSpec, pFullLogFileSpecification );
			strcat( NewLogFileSpec, ".5" );
			remove( NewLogFileSpec );
			strcpy( OldLogFileSpec, pFullLogFileSpecification );
			strcat( OldLogFileSpec, ".4" );
			rename( OldLogFileSpec, NewLogFileSpec );

			strcpy( NewLogFileSpec, OldLogFileSpec );
			strcpy( OldLogFileSpec, pFullLogFileSpecification );
			strcat( OldLogFileSpec, ".3" );
			rename( OldLogFileSpec, NewLogFileSpec );

			strcpy( NewLogFileSpec, OldLogFileSpec );
			strcpy( OldLogFileSpec, pFullLogFileSpecification );
			strcat( OldLogFileSpec, ".2" );
			rename( OldLogFileSpec, NewLogFileSpec );

			strcpy( NewLogFileSpec, OldLogFileSpec );
			strcpy( OldLogFileSpec, pFullLogFileSpecification );
			strcat( OldLogFileSpec, ".1" );
			rename( OldLogFileSpec, NewLogFileSpec );

			strcpy( NewLogFileSpec, OldLogFileSpec );
			strcpy( OldLogFileSpec, pFullLogFileSpecification );
			rename( OldLogFileSpec, NewLogFileSpec );
			}
		}
	if ( hFindFile != INVALID_HANDLE_VALUE )
		FindClose( hFindFile );
}


void LogMessage( char *pMessage, long MessageType )
{
	BOOL		bNoError = TRUE;
	FILE		*pLogFile;
	char		DateString[20];
	char		TimeString[20];
	DWORD		WaitResponse;
	BOOL		bOkToLog;
	unsigned	LoggingDetailSelection;

	_strdate( DateString );
	_strtime( TimeString );

	// Access the status resources with semaphore protection.  Time out after half a second.
	WaitResponse = WaitForSingleObject( hStatusSemaphore, 500 );
	if ( WaitResponse == WAIT_OBJECT_0 )
		{
		bOkToLog = FALSE;
		LoggingDetailSelection = GetLoggingDetail();
		switch ( MessageType & 0x00FF )
			{
			case MESSAGE_TYPE_NORMAL_LOG:
				bOkToLog = TRUE;
				break;
			case MESSAGE_TYPE_SUPPLEMENTARY:
				if ( LoggingDetailSelection == CONFIG_LOGGING_SUPPLEMENTED || LoggingDetailSelection == CONFIG_LOGGING_DEBUG )
					bOkToLog = TRUE;
				break;
			case MESSAGE_TYPE_DETAILS:
				if ( LoggingDetailSelection == CONFIG_LOGGING_DEBUG )
					bOkToLog = TRUE;
				break;
			case MESSAGE_TYPE_ERROR:
				bOkToLog = TRUE;
				break;
			case MESSAGE_TYPE_SERVICE_CONTROL:
				bOkToLog = TRUE;
				break;
			}
		if ( bOkToLog )
			{ 
			if ( ServiceDescriptor.bPrintToConsole )
				printf( "%s %s:  ....  %s\n", DateString, TimeString, pMessage );

			pLogFile = fopen( ServiceDescriptor.LogFileSpecification, "at" );
			if ( pLogFile != NULL )
				{
				if ( ( MessageType & MESSAGE_TYPE_NO_TIME_STAMP ) != 0 )
					fprintf( pLogFile, "  ....  %s\n", pMessage );
				else
					fprintf( pLogFile, "%s %s:  ....  %s\n", DateString, TimeString, pMessage );
				fclose( pLogFile );
				}
			if ( MessageType == MESSAGE_TYPE_SERVICE_CONTROL || MessageType == MESSAGE_TYPE_ERROR )
				PrintEvent( pMessage );

			CheckForLogFileRotation( ServiceDescriptor.LogFileSpecification );
			CheckForLogFileRotation( ServiceDescriptor.SupplementaryLogFileSpecification );
			}
		}
	else if ( WaitResponse == WAIT_TIMEOUT )
		{
		bNoError = FALSE;
		}
	else
		{
		bNoError = FALSE;
		}
	if ( ReleaseSemaphore( hStatusSemaphore, 1L, NULL ) == FALSE )
		{
		bNoError = FALSE;
		}
}


void PrintEvent( const char *Message )
{
	const char	*MessageArray[] = { Message };
	HANDLE		hEventSource;
	
	hEventSource = RegisterEventSource( 0, ServiceDescriptor.DisplayedServiceName );
	if ( hEventSource != 0 )
		{
		ReportEvent( hEventSource, EVENTLOG_INFORMATION_TYPE, 0, 0, 0, 1, 0, MessageArray, 0 );
		DeregisterEventSource( hEventSource );
		}
}


DWORD GetLastSystemErrorMessage( char *TextBuffer, DWORD BufferSize )
{
	DWORD			SystemErrorCode;
	int				PrefixLength;

	SystemErrorCode = GetLastError();
	if ( SystemErrorCode == 0 )
		strcpy( TextBuffer, "" );
	else
		{
		strcpy( TextBuffer, "System Error:  " );
		PrefixLength = strlen( TextBuffer );
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 0, SystemErrorCode, 0, &TextBuffer[ PrefixLength ], BufferSize - PrefixLength / sizeof(TCHAR) - 1, 0 );
		}
	
	return SystemErrorCode;
}


// Remove blanks, tabs and end-of-line characters.
void TrimBlanks( char *pTextString )
{
	long			nChars;
	long			nChar;
	char			*pTrimmedText = pTextString;
	BOOL			bLeadingBlanksWereFound;

	// Convert any tabs or other special characters to spaces.
	nChars = (long)strlen( pTrimmedText );
	for ( nChar = 0; nChar < nChars; nChar++ )
		if ( pTrimmedText[ nChar ] < ' ' )
			pTrimmedText[ nChar ] = ' ';
	// Trim leading blanks.
	bLeadingBlanksWereFound = FALSE;
	while ( pTrimmedText[0] == ' ' || pTrimmedText[0] == '\n' )
		{
		pTrimmedText++;
		bLeadingBlanksWereFound = TRUE;
		}
	// Trim trailing blanks.
	nChars = (long)strlen( pTrimmedText );
	while ( pTrimmedText[ --nChars ] == ' ' || pTrimmedText[ nChars ] == '\n' )
		pTrimmedText[ nChars ] = '\0';

	if ( bLeadingBlanksWereFound )
		strcpy( pTextString, pTrimmedText );
}


