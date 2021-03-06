// ReportStatus.h : Defines the functions and data structures that handle status and error reporting.
//
//	Written by Thomas L. Atwood
//	P.O. Box 1089
//	West Fork, Arkansas 72774
//	(479)445-4690
//	TomAtwood@Earthlink.net
//
//	Copyright � 2010 CDC
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

#define STATUS_ERROR_CREATE_SEMAPHORE			1

#define STATUS_ERROR_DICT_LENGTH				1


// An error message that shows up more than MAX_MESSAGE_REPETITIONS times over a short
// time interval will be silenced until REPETITION_RESET_IN_SECONDS of time passes.
// This keeps the log from filling up with duplicate messages.
#define MAX_MESSAGE_REPETITIONS			3
#define REPETITION_RESET_IN_SECONDS		900		// 15 minutes.


// Each module declares its local error dictionary.  Each possible error message
// that can be generated by that module is encoded using the following data
// structure.
typedef struct
	{
	unsigned long		ErrorCode;
	char				*pErrorMessage;
	BOOL				LogRepetitionCount;
	time_t				LastLogTime;
	} ERROR_DICTIONARY_ENTRY;


// Each module's error dictionary is encoded using the following data structure.
typedef struct ErrorDictionaryModule
	{
	unsigned long					nModuleIndex;
	ERROR_DICTIONARY_ENTRY			*pFirstDictionaryEntry;		// Pointer to the array of
																// possible error messages.
	unsigned long					nEntryCount;
	struct ErrorDictionaryModule	*pNextModule;
	} ERROR_DICTIONARY_MODULE;


// This USER_NOTIFICATION structure encodes messages for the user to be stored in
// a local file, where they can be picked up for display by the BViewer program.
// The BRetriever program is a typical Windows service, and as such, does not
// support its own user interface.
typedef struct
	{
	char				Source[ 16 ];
	unsigned long		ModuleCode;
	unsigned long		ErrorCode;
	unsigned long		UserNotificationCause;
							#define USER_NOTIFICATION_CAUSE_REPORT_FIELDS				1
							#define USER_NOTIFICATION_CAUSE_PRODUCT_RECEIVE_ERROR		2
							#define USER_NOTIFICATION_CAUSE_RETRIEVAL_STARTUP_ERROR		3
							#define USER_NOTIFICATION_CAUSE_PRODUCT_PROCESSING_ERROR	4
							#define USER_NOTIFICATION_CAUSE_IMAGE_PROCESSING_ERROR		5
							#define USER_NOTIFICATION_CAUSE_IMPORT_PROCESSING_ERROR		6
							#define USER_NOTIFICATION_CAUSE_CONFIRM_USER_REQUEST		7
							#define USER_NOTIFICATION_CAUSE_NEEDS_ACKNOWLEDGMENT		8
							#define USER_NOTIFICATION_CAUSE_INCOMPLETE_INTERPRETATION	9
	unsigned long		TypeOfUserResponseSupported;
							#define USER_RESPONSE_TYPE_CONTINUE				0x00000001
							#define USER_RESPONSE_TYPE_YESNO				0x00000002
							#define USER_RESPONSE_TYPE_YESNO_NO_CANCEL		0x00000004
							#define USER_RESPONSE_TYPE_SUSPEND				0x00000008
							#define USER_RESPONSE_TYPE_ERROR				0x00010000
	unsigned long		UserResponseCode;
							#define USER_RESPONSE_CODE_YES					0x00000001
							#define USER_RESPONSE_CODE_NO					0x00000002
							#define USER_RESPONSE_CODE_CONTINUE				0x00000004
							#define USER_RESPONSE_CODE_SUSPEND				0x00000008
	char				NoticeText[ 512 ];
	char				SuggestedActionText[ 128 ];
	int					TextLinesRequired;
	} USER_NOTIFICATION;


// Function prototypes.
//
void					InitStatusModule();
void					CloseStatusModule();

ERROR_DICTIONARY_ENTRY	*GetMessageFromDictionary( unsigned long nModuleIndex, unsigned MessageCode );
void					RegisterErrorDictionary( ERROR_DICTIONARY_MODULE *pNewErrorDictionaryModule );
void					RespondToErrorAt( char *pSourceFile, int SourceLineNumber, unsigned long nModuleIndex, unsigned ErrorCode );
void					RespondToError( unsigned long nModuleIndex, unsigned ErrorCode );
void					LogMessageAt( char *pSourceFile, int SourceLineNumber, char *pMessage, long MessageType );
void					LogMessage( char *pMessage, long MessageType );
							#define MESSAGE_TYPE_NORMAL_LOG			0x0000		// Normal log message.
							#define MESSAGE_TYPE_SUPPLEMENTARY		0x0001		// Supplementary information, not normally viewed.
							#define MESSAGE_TYPE_DETAILS			0x0002		// Detailed debugging information.
							#define MESSAGE_TYPE_ERROR				0x0004
							#define MESSAGE_TYPE_NO_TIME_STAMP		0x0100
void					PrintEvent( const char *Message );
BOOL					CheckForUserNotification();
DWORD					GetLastSystemErrorMessage( char *TextBuffer, DWORD BufferSize );
void					TrimBlanks( char *pTextString );
void					TrimTrailingSpaces( char *pTextString );
void					PruneEmbeddedSpaceAndPunctuation( char *pTextString );
void					PruneQuotationMarks( char *pTextString );
void					PruneEmbeddedWhiteSpace( char *pTextString );
