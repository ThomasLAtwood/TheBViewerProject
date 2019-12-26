// ReportStatus.cpp : Implements the functions that handle the status and error reporting.
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
#include "StdAfx.h"
#include "BViewer.h"
#include "Module.h"
#include "ReportStatus.h"
#include "Configuration.h"


extern CONFIGURATION			BViewerConfiguration;
extern CBViewerApp				ThisBViewerApp;


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
static char					*pSemaphoreName = "BViewerStatusSemaphore";
LIST_HEAD					UserNoticeList = 0;


// This function must be called before any other function in this module.
void InitStatusModule()
{
	LinkModuleToList( &StatusModuleInfo );
	RegisterErrorDictionary( &ReportStatusErrorDictionary );
	// Create a semaphore for controlling access to the status module from
	// different (competing) threads.
	hStatusSemaphore = CreateSemaphore( NULL, 1L, 1L, pSemaphoreName );
	// Note:  The semaphore creation will fail if another copy of BViewer is already running.
	// The following error message won't be logged, because the semaphore won't grant access.
	// This condition is tested and responded to in the CBViewerApp::InitInstance() method
	// and will result in aborting the program.
	if ( GetLastError() == ERROR_ALREADY_EXISTS )
		hStatusSemaphore = NULL;
	if ( hStatusSemaphore == NULL )
		RespondToError( MODULE_STATUS, STATUS_ERROR_CREATE_SEMAPHORE );
}


// This function must be called to deallocate memory and close this module.
void CloseStatusModule()
{
	CloseHandle( hStatusSemaphore );
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


void RespondToErrorAt( char *pSourceFile, int SourceLineNumber, unsigned long nModuleIndex, unsigned ErrorCode )
{
	char						*pModuleName;
	char						*pMessageText;
	char						ErrorMessage[ 1096 ];
	char						TextMsg[256];
	ERROR_DICTIONARY_ENTRY		*pDictEntry;
	time_t						CurrentSystemTime;
	double						TimeDifferenceInSeconds;
	BOOL						bDisallowMessageRepetitionLimits;

	pModuleName = GetModuleName( nModuleIndex );
	pDictEntry = GetMessageFromDictionary( nModuleIndex, ErrorCode );
	time( &CurrentSystemTime );
	bDisallowMessageRepetitionLimits = FALSE;

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
		LogMessageAt( pSourceFile, SourceLineNumber, ErrorMessage, MESSAGE_TYPE_ERROR );
		pDictEntry -> LastLogTime = CurrentSystemTime;
		}
	pDictEntry -> LogRepetitionCount++;
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
	BOOL						bDisallowMessageRepetitionLimits;

	pModuleName = GetModuleName( nModuleIndex );
	pDictEntry = GetMessageFromDictionary( nModuleIndex, ErrorCode );
	time( &CurrentSystemTime );
	bDisallowMessageRepetitionLimits = FALSE;

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


void CheckForLogFileRotation( char *pFullLogFileSpecification )
{
	unsigned long			nFileSizeInBytes;
	char					OldLogFileSpec[ 256 ];
	char					NewLogFileSpec[ 256 ];

	nFileSizeInBytes = (unsigned long)GetFileSizeInBytes( pFullLogFileSpecification );
	if ( nFileSizeInBytes > 2000000 )		// Limit log file size to 2 megabytes.
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


void LogMessageAt( char *pSourceFile, int SourceLineNumber, char *pMessage, long MessageType )
{
	size_t			MessageLength;
	char			*pAugmentedMsg;
	
	MessageLength = strlen( pMessage ) + 1;
	pAugmentedMsg = (char*)malloc( MessageLength + 128 );
	if ( pAugmentedMsg != 0 )
		{
		sprintf( pAugmentedMsg, "%s(%d):  %s", pSourceFile, SourceLineNumber, pMessage );
		LogMessage( pAugmentedMsg, MessageType );
		}
	free( pAugmentedMsg );
}


void LogMessage( char *pMessage, long MessageType )
{
	BOOL		bNoError = TRUE;
	FILE		*pLogFile;
	char		DateString[20];
	char		TimeString[20];
	DWORD		WaitResponse;
	BOOL		bOkToLog;
	BOOL		bOkForSupplementaryLog;

	_strdate( DateString );
	_strtime( TimeString );

	// Access the status resources with semaphore protection.  Time out after half a second.
	WaitResponse = WaitForSingleObject( hStatusSemaphore, 500 );
	if ( WaitResponse == WAIT_OBJECT_0 )
		{
		bOkToLog = FALSE;
		bOkForSupplementaryLog = TRUE;
		switch ( MessageType & 0x00FF )
			{
			case MESSAGE_TYPE_NORMAL_LOG:
				bOkToLog = TRUE;
				bOkForSupplementaryLog = TRUE;
				break;
			case MESSAGE_TYPE_SUPPLEMENTARY:
				bOkToLog = FALSE;
				bOkForSupplementaryLog = TRUE;
				break;
			case MESSAGE_TYPE_DETAILS:
				bOkToLog = FALSE;
				bOkForSupplementaryLog = FALSE;		// Set this to TRUE and recompile to get these messages.
				break;
			case MESSAGE_TYPE_ERROR:
				bOkToLog = TRUE;
				bOkForSupplementaryLog = TRUE;
				break;
			}
		if ( bOkToLog )
			{ 
			if ( BViewerConfiguration.bPrintToConsole )
				printf( "%s %s:  %s\n", DateString, TimeString, pMessage );

			pLogFile = fopen( BViewerConfiguration.BViewerLogFile, "at" );
			if ( pLogFile != NULL )
				{
				if ( ( MessageType & MESSAGE_TYPE_NO_TIME_STAMP ) != 0 )
					fprintf( pLogFile, "%s\n", pMessage );
				else
					fprintf( pLogFile, "%s %s:  %s\n", DateString, TimeString, pMessage );
				fclose( pLogFile );
				}
			if ( MessageType == MESSAGE_TYPE_ERROR )
				PrintEvent( pMessage );

			CheckForLogFileRotation( BViewerConfiguration.BViewerLogFile );
			}
		if ( bOkForSupplementaryLog )
			{ 
			pLogFile = fopen( BViewerConfiguration.BViewerSupplementaryLogFile, "at" );
			if ( pLogFile != NULL )
				{
				if ( ( MessageType & MESSAGE_TYPE_NO_TIME_STAMP ) != 0 )
					fprintf( pLogFile, "%s\n", pMessage );
				else
					fprintf( pLogFile, "%s %s:  %s\n", DateString, TimeString, pMessage );
				fclose( pLogFile );
				}
			CheckForLogFileRotation( BViewerConfiguration.BViewerSupplementaryLogFile );
			}
		if ( ReleaseSemaphore( hStatusSemaphore, 1L, NULL ) == FALSE )
			bNoError = FALSE;		// An error in the report module?  Who ya gonna call?
									// Just keep on trucking.
		}
	else if ( WaitResponse == WAIT_TIMEOUT )
		bNoError = FALSE;
	else
		{
		bNoError = FALSE;
		ReleaseSemaphore( hStatusSemaphore, 1L, NULL );
		}
}


void PrintEvent( const char *Message )
{
	const char	*MessageArray[] = { Message };
	HANDLE		hEventSource;
	
	hEventSource = RegisterEventSource( 0, BViewerConfiguration.ProgramName );
	if ( hEventSource != 0 )
		{
		ReportEvent( hEventSource, EVENTLOG_INFORMATION_TYPE, 0, 0, 0, 1, 0, MessageArray, 0 );
		DeregisterEventSource( hEventSource );
		}
}


// Read any externally generated user notification messages and append them to the
// list of pending notices.
BOOL CheckForUserNotification()
{
	char				UserNoticeFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	FILE				*pUserNoticeFile;
	USER_NOTIFICATION	UserNoticeDescriptor;
	USER_NOTIFICATION	*pNewUserNoticeDescriptor;
	size_t				nBytesRead;
	BOOL				bFileHasBeenCompletelyRead;
	BOOL				bNoticesHaveBeenPosted;
	unsigned long		BRetrieverStatus;

	strcpy( UserNoticeFileSpec, "" );
	strncat( UserNoticeFileSpec, BViewerConfiguration.BRetrieverServiceDirectory, FULL_FILE_SPEC_STRING_LENGTH - 1 );
	LocateOrCreateDirectory( UserNoticeFileSpec );	// Ensure directory exists.
	if ( UserNoticeFileSpec[ strlen( UserNoticeFileSpec ) - 1 ] != '\\' )
		strcat( UserNoticeFileSpec, "\\" );
	strncat( UserNoticeFileSpec, "UserNotices.dat",
				FULL_FILE_SPEC_STRING_LENGTH - 1 - strlen( UserNoticeFileSpec ) );
	pUserNoticeFile = fopen( UserNoticeFileSpec, "rb" );
	if ( pUserNoticeFile != 0 )
		{
		bFileHasBeenCompletelyRead = FALSE;
		while( !bFileHasBeenCompletelyRead )
			{
			nBytesRead = fread( &UserNoticeDescriptor, 1, sizeof( USER_NOTIFICATION ), pUserNoticeFile );
			bFileHasBeenCompletelyRead = ( nBytesRead < sizeof( USER_NOTIFICATION ) );
			if ( !bFileHasBeenCompletelyRead )
				{
				pNewUserNoticeDescriptor = (USER_NOTIFICATION*)malloc( sizeof(USER_NOTIFICATION) );
				if ( pNewUserNoticeDescriptor != 0 )
					{
					memcpy( pNewUserNoticeDescriptor, &UserNoticeDescriptor, sizeof(USER_NOTIFICATION) );
					AppendToList( &UserNoticeList, (void*)pNewUserNoticeDescriptor );
					}
				}
			}
		fclose( pUserNoticeFile );
		DeleteFile( UserNoticeFileSpec );
		}
	bNoticesHaveBeenPosted = ( UserNoticeList != 0 );
	//
	// Check for a BRetriever status report.
	strcpy( UserNoticeFileSpec, "" );
	strncat( UserNoticeFileSpec, BViewerConfiguration.BRetrieverServiceDirectory, FULL_FILE_SPEC_STRING_LENGTH - 1 );
	LocateOrCreateDirectory( UserNoticeFileSpec );	// Ensure directory exists.
	if ( UserNoticeFileSpec[ strlen( UserNoticeFileSpec ) - 1 ] != '\\' )
		strcat( UserNoticeFileSpec, "\\" );
	strncat( UserNoticeFileSpec, "BRetrieverStatus.dat",
				FULL_FILE_SPEC_STRING_LENGTH - 1 - strlen( UserNoticeFileSpec ) );
	pUserNoticeFile = fopen( UserNoticeFileSpec, "rb" );
	if ( pUserNoticeFile != 0 )
		{
		nBytesRead = fread( &BRetrieverStatus, 1, sizeof( unsigned long ), pUserNoticeFile );
		if ( nBytesRead == sizeof( unsigned long ) )
			{
			ThisBViewerApp.m_BRetrieverStatus = BRetrieverStatus;
			ThisBViewerApp.m_nSecondsSinceHearingFromBRetriever = 0L;
			bNoticesHaveBeenPosted = TRUE;
			}
		fclose( pUserNoticeFile );
		DeleteFile( UserNoticeFileSpec );
		}

	return bNoticesHaveBeenPosted;
}


DWORD GetLastSystemErrorMessage( char *TextBuffer, DWORD BufferSize )
{
	DWORD			SystemErrorCode;

	SystemErrorCode = GetLastError();
	if ( SystemErrorCode == 0 )
		strcpy( TextBuffer, "" );
	else
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 0, SystemErrorCode, 0, TextBuffer, BufferSize / sizeof(TCHAR) - 1, 0 );
	
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


void TrimTrailingSpaces( char *pTextString )
{
	long			nChars;

	// Trim trailing blanks.
	nChars = (long)strlen( pTextString );
	while ( nChars > 0 && pTextString[ --nChars ] == ' ' )
		pTextString[ nChars ] = '\0';
}


// Remove blanks, tabs and end-of-line characters.
void PruneEmbeddedSpaceAndPunctuation( char *pTextString )
{
	long			nChars;
	long			nChar;
	long			nCharOut;
	char			*pTrimmedText = pTextString;

	// Convert any tabs or other special characters to spaces.
	nChars = (long)strlen( pTrimmedText );
	nCharOut = 0;
	for ( nChar = 0; nChar < nChars; nChar++ )
		{
		if ( ( pTrimmedText[ nChar ] >= '0' &&  pTrimmedText[ nChar ] <= '9' )
					|| ( pTrimmedText[ nChar ] >= 'A' && pTrimmedText[ nChar ] <= 'Z' )
					|| ( pTrimmedText[ nChar ] >= 'a' && pTrimmedText[ nChar ] <= 'z' )
					|| pTrimmedText[ nChar ] == '_' )
			{
			pTextString[ nCharOut ] = pTrimmedText[ nChar ];
			nCharOut++;
			}
		}

	pTextString[ nCharOut ] = '\0';
}


void PruneQuotationMarks( char *pTextString )
{
	long			nChars;
	long			nChar;
	long			nCharOut;
	char			*pTrimmedText = pTextString;

	nChars = (long)strlen( pTrimmedText );
	nCharOut = 0;
	for ( nChar = 0; nChar < nChars; nChar++ )
		{
		if ( pTrimmedText[ nChar ] != '\"' )
			{
			pTextString[ nCharOut ] = pTrimmedText[ nChar ];
			nCharOut++;
			}
		}

	pTextString[ nCharOut ] = '\0';
}


void PruneEmbeddedWhiteSpace( char *pTextString )
{
	long			nChars;
	long			nChar;
	long			nCharOut;
	char			*pTrimmedText = pTextString;

	nChars = (long)strlen( pTrimmedText );
	nCharOut = 0;
	for ( nChar = 0; nChar < nChars; nChar++ )
		{
		if ( pTrimmedText[ nChar ] != ' ' )
			{
			pTextString[ nCharOut ] = pTrimmedText[ nChar ];
			nCharOut++;
			}
		}

	pTextString[ nCharOut ] = '\0';
}


