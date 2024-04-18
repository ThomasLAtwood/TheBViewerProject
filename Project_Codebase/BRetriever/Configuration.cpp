// Configuration.cpp : Implements the functions that handle the BRetriever service configuration.
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
//	*[1] 03/12/2024 by Tom Atwood
//		Fixed security issues.
//
//
#include "Module.h"
#include "ReportStatus.h"
#include "ServiceMain.h"
#include "Configuration.h"
#include "Operation.h"


extern TRANSFER_SERVICE			TransferService;
extern unsigned __stdcall ListenForExamThreadFunction( void *pOperationStruct );
extern unsigned __stdcall ReceiveDicomThreadFunction( VOID *pOperationStruct );
extern unsigned __stdcall WatchForExamThreadFunction( VOID *pOperationStruct );
extern unsigned __stdcall ProcessProductQueueThreadFunction( VOID *pOperationStruct );


PRODUCT_OPERATION				*pPrimaryOperationList = 0;
ENDPOINT						*pEndPointList = 0;
CONFIGURATION					ServiceConfiguration;


//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		ConfigModuleInfo = { MODULE_CONFIG, "Configuration Module", InitConfigurationModule, CloseConfigurationModule };


static ERROR_DICTIONARY_ENTRY	ConfigurationErrorCodes[] =
			{
				{ CONFIG_ERROR_INSUFFICIENT_MEMORY		, "There is not enough memory to allocate a data structure." },
				{ CONFIG_ERROR_OPEN_CFG_FILE			, "An error occurred attempting to open the configuration file." },
				{ CONFIG_ERROR_READ_CFG_FILE			, "An error occurred while reading the configuration file." },
				{ CONFIG_ERROR_PARSE_ALLOCATION			, "An error occurred allocating memory for reading data from the configuration file." },
				{ CONFIG_ERROR_PARSE_ATTRIB_VALUE		, "An error occurred parsing a configuration attribute value in the configuration file." },
				{ CONFIG_ERROR_PARSE_UNKNOWN_ATTR		, "An error occurred parsing a configuration attribute name in the configuration file." },
				{ CONFIG_ERROR_PARSE_TIME				, "An error occurred parsing a date/time specification in the configuration file." },
				{ CONFIG_ERROR_PARSE_ENDPOINT_TYPE		, "An error occurred parsing an endpoint type in the configuration file." },
				{ 0										, NULL }
			};


static ERROR_DICTIONARY_MODULE		ConfigurationStatusErrorDictionary =
										{
										MODULE_CONFIG,
										ConfigurationErrorCodes,
										CONFIG_ERROR_DICT_LENGTH,
										0
										};


// This function must be called before any other function in this module.
void InitConfigurationModule()
{
	LinkModuleToList( &ConfigModuleInfo );
	RegisterErrorDictionary( &ConfigurationStatusErrorDictionary );
	InitConfiguration();
}


void CloseConfigurationModule()
{
	EraseProductOperationList();
	EraseEndPointList( &pEndPointList );
}


ENDPOINT EndPointWatchFolder =
	{
	"Image Input Watch Folder",				// Name[ MAX_CFG_STRING_LENGTH ].
	ENDPOINT_TYPE_FILE,						// EndPointType.
	"localhost",							// NetworkAddress[ MAX_CFG_STRING_LENGTH ].
	"",										// AE_TITLE[ MAX_CFG_STRING_LENGTH ].
	"Watch Folder",							// Directory[ MAX_CFG_STRING_LENGTH ].
	TRUE,									// bTrustSpecifiedTransferSyntax.
	FALSE,									// bApplyManualDicomEdits.
	0										// *pNextEndPoint.
	};


ENDPOINT EndPointNetworkIn =
	{
	"Network Dicom Source",					// Name[ MAX_CFG_STRING_LENGTH ].
	ENDPOINT_TYPE_NETWORK,					// EndPointType.
	"",										// NetworkAddress[ MAX_CFG_STRING_LENGTH ].
	"BViewer",								// AE_TITLE[ MAX_CFG_STRING_LENGTH ].
	"",										// Directory[ MAX_CFG_STRING_LENGTH ].
	TRUE,									// bTrustSpecifiedTransferSyntax.
	FALSE,									// bApplyManualDicomEdits.
	0										// *pNextEndPoint.
	};


ENDPOINT EndPointInbox =
	{
	"Inbox",								// Name[ MAX_CFG_STRING_LENGTH ].
	ENDPOINT_TYPE_FILE,						// EndPointType.
	"localhost",							// NetworkAddress[ MAX_CFG_STRING_LENGTH ].
	"",										// AE_TITLE[ MAX_CFG_STRING_LENGTH ].
	"Inbox",								// Directory[ MAX_CFG_STRING_LENGTH ].
	TRUE,									// bTrustSpecifiedTransferSyntax.
	FALSE,									// bApplyManualDicomEdits.
	0										// *pNextEndPoint.
	};


ENDPOINT EndPointQueue =
	{
	"Queue",								// Name[ MAX_CFG_STRING_LENGTH ].
	ENDPOINT_TYPE_FILE,						// EndPointType.
	"",										// NetworkAddress[ MAX_CFG_STRING_LENGTH ].
	"",										// AE_TITLE[ MAX_CFG_STRING_LENGTH ].
	"Queued Files",							// Directory[ MAX_CFG_STRING_LENGTH ].
	TRUE,									// bTrustSpecifiedTransferSyntax.
	FALSE,									// bApplyManualDicomEdits.
	0										// *pNextEndPoint.
	};


ENDPOINT EndPointImageDeposit =
	{
	"Image Deposit",						// Name[ MAX_CFG_STRING_LENGTH ].
	ENDPOINT_TYPE_FILE,						// EndPointType.
	"",										// NetworkAddress[ MAX_CFG_STRING_LENGTH ].
	"",										// AE_TITLE[ MAX_CFG_STRING_LENGTH ].
	"Images",								// Directory[ MAX_CFG_STRING_LENGTH ].
	TRUE,									// bTrustSpecifiedTransferSyntax.
	FALSE,									// bApplyManualDicomEdits.
	0										// *pNextEndPoint.
	};


ENDPOINT	*pEndPointPrototypeArray[] =
	{
	&EndPointWatchFolder,
	&EndPointNetworkIn,
	&EndPointInbox,
	&EndPointQueue,
	&EndPointImageDeposit,
	0
	};



PRODUCT_OPERATION OperationWatch =
	{
	"Receive From Folder",					// Operation Name[ MAX_FILE_SPEC_LENGTH ].
	OPERATION_TYPE_RECEIVE_FROM_FOLDER,		// OperationType.
	30,										// OperationTimeInterval.
	&EndPointWatchFolder,					// *pInputEndPoint.
	TRUE,									// bInputDeleteSourceOnCompletion.
	&EndPointQueue,							// *pOutputEndPoint.
	TRUE,									// bEnabled.
	"",										// DependentOperationName[ MAX_CFG_STRING_LENGTH ].
	0,										// *pDependentOperation.
	0,										// *pNextOperation.
		{									// OpnState.
		OPERATION_STATUS_UNKNOWN,				// StatusCode.
		0,										// DirectorySearchLevel.
		0,										// *pProductItem.
		TRUE,									// bOKtoProcessThisStudy.
		0,										// hOperationThreadHandle.
		0,										// OperationThreadID.
		WatchForExamThreadFunction,				// ThreadFunction.
		0,										// hSleepSemaphore.
		""										// SleepSemaphoreName[ MAX_CFG_STRING_LENGTH ].
		}
	};


PRODUCT_OPERATION OperationListen =
	{
	"Listen",								// Operation Name[ MAX_FILE_SPEC_LENGTH ].
	OPERATION_TYPE_LISTEN_TO_NETWORK,		// OperationType.
	40,										// OperationTimeInterval.
	&EndPointNetworkIn,						// *pInputEndPoint.
	FALSE,									// bInputDeleteSourceOnCompletion.
	&EndPointQueue,							// *pOutputEndPoint.
	TRUE,									// bEnabled.
	"",										// DependentOperationName[ MAX_CFG_STRING_LENGTH ].
	0,										// *pDependentOperation.
	0,										// *pNextOperation.
		{									// OpnState.
		OPERATION_STATUS_UNKNOWN,				// StatusCode.
		0,										// DirectorySearchLevel.
		0,										// *pProductItem.
		TRUE,									// bOKtoProcessThisStudy.
		0,										// hOperationThreadHandle.
		0,										// OperationThreadID.
		ListenForExamThreadFunction,			// ThreadFunction.
		0,										// hSleepSemaphore.
		""										// SleepSemaphoreName[ MAX_CFG_STRING_LENGTH ].
		}
	};


PRODUCT_OPERATION OperationReceive =
	{
	"Receive Dicom From Network",			// Operation Name[ MAX_FILE_SPEC_LENGTH ].
	OPERATION_TYPE_RECEIVE_FROM_NETWORK,	// OperationType.
	40,										// OperationTimeInterval.
	&EndPointNetworkIn,						// *pInputEndPoint.
	FALSE,									// bInputDeleteSourceOnCompletion.
	&EndPointInbox,							// *pOutputEndPoint.
	TRUE,									// bEnabled.
	"",										// DependentOperationName[ MAX_CFG_STRING_LENGTH ].
	0,										// *pDependentOperation.
	0,										// *pNextOperation.
		{									// OpnState.
		OPERATION_STATUS_UNKNOWN,				// StatusCode.
		0,										// DirectorySearchLevel.
		0,										// *pProductItem.
		TRUE,									// bOKtoProcessThisStudy.
		0,										// hOperationThreadHandle.
		0,										// OperationThreadID.
		ReceiveDicomThreadFunction,				// ThreadFunction.
		0,										// hSleepSemaphore.
		""										// SleepSemaphoreName[ MAX_CFG_STRING_LENGTH ].
		}
	};


PRODUCT_OPERATION OperationProcessImage =
	{
	"Process Image",						// Operation Name[ MAX_FILE_SPEC_LENGTH ].
	OPERATION_TYPE_DISPATCH_FROM_QUEUE,		// OperationType.
	5,										// OperationTimeInterval.
	&EndPointQueue,							// *pInputEndPoint.
	TRUE,									// bInputDeleteSourceOnCompletion.
	&EndPointImageDeposit,					// *pOutputEndPoint.
	TRUE,									// bEnabled.
	"",										// DependentOperationName[ MAX_CFG_STRING_LENGTH ].
	0,										// *pDependentOperation.
	0,										// *pNextOperation.
		{									// OpnState.
		OPERATION_STATUS_UNKNOWN,				// StatusCode.
		0,										// DirectorySearchLevel.
		0,										// *pProductItem.
		TRUE,									// bOKtoProcessThisStudy.
		0,										// hOperationThreadHandle.
		0,										// OperationThreadID.
		ProcessProductQueueThreadFunction,		// ThreadFunction.
		0,										// hSleepSemaphore.
		""										// SleepSemaphoreName[ MAX_CFG_STRING_LENGTH ].
		}
	};


PRODUCT_OPERATION	*pProductOperationPrototypeArray[] =
	{
	&OperationWatch,
	&OperationListen,
	&OperationReceive,
	&OperationProcessImage,
	0
	};


void ResolveEndPointAddresses()
{
	ENDPOINT			*pEndPoint;
	char				EndPointAbsoluteDirectory[ MAX_FILE_SPEC_LENGTH ];

	pEndPoint = pEndPointList;
	while( pEndPoint != 0 )
		{
		if ( pEndPoint -> Directory != 0 && strlen( pEndPoint -> Directory ) > 0 )
			{
			// If the directory specification is relative, and not absolute, make it absolute.
			if ( strchr( pEndPoint -> Directory, ':' ) == 0 && pEndPoint -> Directory[ 0 ] != '\\' )
				{
				EndPointAbsoluteDirectory[0] = '\0';					// *[1] Eliminate call to strcpy.
				strncat_s( EndPointAbsoluteDirectory, MAX_FILE_SPEC_LENGTH, TransferService.ProgramDataPath, _TRUNCATE );	// *[1] Replaced strncat with strncat_s.
				if ( EndPointAbsoluteDirectory[ strlen( EndPointAbsoluteDirectory ) - 1 ] != '\\' )
					strncat_s( EndPointAbsoluteDirectory, MAX_FILE_SPEC_LENGTH, "\\", _TRUNCATE );							// *[1] Replaced strcat with strncat_s.
				strncat_s( EndPointAbsoluteDirectory, MAX_FILE_SPEC_LENGTH, pEndPoint -> Directory, _TRUNCATE );			// *[1] Replaced strcat with strncat_s.
				strncpy_s( pEndPoint -> Directory, MAX_CFG_STRING_LENGTH, EndPointAbsoluteDirectory, _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
				}
			}
		pEndPoint = pEndPoint -> pNextEndPoint;
		}
}


void LinkEndPointsToList()
{
	ENDPOINT			*pEndPoint;
	ENDPOINT			*pPrevEndPoint;
	int					nEndPoint;
	BOOL				bEndOfList;

	pPrevEndPoint = 0;
	nEndPoint = 0;
	pEndPointList = 0;					// *[1] Added redundant pointer init to make it locally clear that no NULL pointer dereference.
	bEndOfList = FALSE;
	while( !bEndOfList )
		{
		pEndPoint = pEndPointPrototypeArray[ nEndPoint ];
		bEndOfList = ( pEndPoint == 0 );
		if ( !bEndOfList )
			{
			pEndPoint -> pNextEndPoint = 0;
			if ( pEndPointList == 0 )
				pEndPointList = pEndPoint;
			else
				pPrevEndPoint -> pNextEndPoint = pEndPoint;
			pPrevEndPoint = pEndPoint;
			nEndPoint++;
			}
		}
	ResolveEndPointAddresses();
}


void LinkOperationsToList()
{
	PRODUCT_OPERATION			*pOperation;
	PRODUCT_OPERATION			*pPrevOperation;
	int							nOperation;
	BOOL						bEndOfList;

	pPrevOperation = 0;
	nOperation = 0;
	pPrimaryOperationList = 0;					// *[1] Added redundant pointer init to make it locally clear that no NULL pointer dereference.
	bEndOfList = FALSE;
	while( !bEndOfList )
		{
		pOperation = pProductOperationPrototypeArray[ nOperation ];
		bEndOfList = ( pOperation == 0 );
		if ( !bEndOfList )
			{
			pOperation -> pNextOperation = 0;
			if ( pPrimaryOperationList == 0 )
				pPrimaryOperationList = pOperation;
			else
				pPrevOperation -> pNextOperation = pOperation;
			pPrevOperation = pOperation;
			nOperation++;
			}
		}
	LinkEndPointsToList();
}


void AppendOperationToList( PRODUCT_OPERATION *pOperationToBeAppended )
{
	PRODUCT_OPERATION			*pListOperation;

	if ( pPrimaryOperationList == 0 )
		pPrimaryOperationList = pOperationToBeAppended;
	else
		{
		pListOperation = pPrimaryOperationList;
		while ( pListOperation -> pNextOperation != 0 )
			pListOperation = pListOperation -> pNextOperation;
		pListOperation -> pNextOperation = pOperationToBeAppended;
		}
	pOperationToBeAppended -> pNextOperation = 0;
}


PRODUCT_OPERATION *FindMatchingOperation( char *pOperationName )
{
	PRODUCT_OPERATION			*pOperation;
	BOOL						bEndOfList;
	BOOL						bMatchingOperationFound;

	bEndOfList = FALSE;
	bMatchingOperationFound = FALSE;
	pOperation = pPrimaryOperationList;
	while ( !bEndOfList && !bMatchingOperationFound )
		{
		bEndOfList = ( pOperation == 0 );
		if ( !bEndOfList )
			bMatchingOperationFound = ( _stricmp( pOperation -> OperationName, pOperationName ) == 0 );
		if ( !bEndOfList && !bMatchingOperationFound )
			pOperation = pOperation -> pNextOperation;
		}

	return pOperation;
}



//
// The following are the functions used to configure BRetriever for operation.
//


BOOL ReadConfigurationFile( char *pConfigurationDirectory, char *pConfigurationFileName )
{
	BOOL				bNoError = TRUE;
	char				CfgFileSpec[ MAX_CFG_STRING_LENGTH ];
	FILE				*pCfgFile;
	char				TextLine[ MAX_CFG_STRING_LENGTH ];
	BOOL				bEndOfFile;
	BOOL				bFileReadError;
	int					ParseState;
							#define PARSE_STATE_UNSPECIFIED		0
							#define PARSE_STATE_OPERATION		1
							#define PARSE_STATE_CONFIGURATION	2
							#define PARSE_STATE_ENDPOINT		3
	PRODUCT_OPERATION	*pProductOperation = 0;
	ENDPOINT			*pEndPoint = 0;
	ENDPOINT			*pTempEndPoint;
	BOOL				bSkipLine;
	char				*pAttributeName;
	char				*pAttributeValue;
	BOOL				bOpenBracketEncountered;
	
	strncpy_s( CfgFileSpec, MAX_CFG_STRING_LENGTH, pConfigurationDirectory, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
	if ( CfgFileSpec[ strlen( CfgFileSpec ) - 1 ] != '\\' )
		strncat_s( CfgFileSpec, MAX_CFG_STRING_LENGTH, "\\", _TRUNCATE );						// *[1] Replaced strcat with strncat_s.
	strncat_s( CfgFileSpec, MAX_CFG_STRING_LENGTH, pConfigurationFileName, _TRUNCATE );			// *[1] Replaced strcat with strncat_s.
	pCfgFile = fopen( CfgFileSpec, "rt" );
	if ( pCfgFile != 0 )
		{
		bEndOfFile = FALSE;
		bFileReadError = FALSE;
		ParseState = PARSE_STATE_UNSPECIFIED;
		bOpenBracketEncountered = FALSE;
		do
			{
			if ( fgets( TextLine, MAX_CFG_STRING_LENGTH - 1, pCfgFile ) == NULL )
				{
				if ( feof( pCfgFile ) )
					bEndOfFile = TRUE;
				else if ( ferror( pCfgFile ) )
					{
					bFileReadError = TRUE;
					RespondToError( MODULE_CONFIG, CONFIG_ERROR_READ_CFG_FILE );
					}
				}
			if ( !bEndOfFile && !bFileReadError )
				{
				bSkipLine = FALSE;
				TrimBlanks( TextLine );
				if ( ParseState == PARSE_STATE_UNSPECIFIED )
					{
					// Look for validly formatted attribute name and value.  Find a colon or an end-of-line.
					pAttributeName = strtok( TextLine, ":\n" );
					if ( pAttributeName == NULL )
						bSkipLine = TRUE;			// If neither found, skip this line.
					}
				if ( TextLine[ 0 ] == '{' )
					{
					bOpenBracketEncountered = TRUE;
					bSkipLine = TRUE;
					}
				if ( TextLine[0] == '#' || strlen( TextLine ) == 0 )
					bSkipLine = TRUE;
				if ( !bSkipLine && ParseState == PARSE_STATE_UNSPECIFIED )
					{
					pAttributeValue = strtok( NULL, "\n" );  // Point to the value following the colon.
					if ( pAttributeValue == NULL )
						{
						RespondToError( MODULE_CONFIG, CONFIG_ERROR_PARSE_ATTRIB_VALUE );
						bNoError = FALSE;
						}
					else
						TrimBlanks( pAttributeValue );

					if ( bNoError )														// *[1] Test for value presence.
						{
						if ( _stricmp( pAttributeName, "OPERATION" ) == 0 )
							{
							ParseState = PARSE_STATE_OPERATION;
							pProductOperation = FindMatchingOperation( pAttributeValue );
							if ( pProductOperation == 0 )
								{
								RespondToError( MODULE_CONFIG, CONFIG_ERROR_PARSE_ALLOCATION );
								bNoError = FALSE;
								}
							}
						else if ( _stricmp( pAttributeName, "CONFIGURATION" ) == 0 )
							{
							ParseState = PARSE_STATE_CONFIGURATION;
							strncpy_s( ServiceConfiguration.ThisTransferNodeName, MAX_CFG_STRING_LENGTH, pAttributeValue, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
							}
						else if ( _stricmp( pAttributeName, "ENDPOINT" ) == 0 )
							{
							ParseState = PARSE_STATE_ENDPOINT;
							pEndPoint = LookUpEndPointAddress( pAttributeValue );
							if ( pEndPoint == 0 )
								{
								pEndPoint = CreateEndPoint();
								if ( pEndPoint != 0 )																						// *[1] Added test for successful allocation.
									{
									strncpy_s( pEndPoint -> Name, MAX_CFG_STRING_LENGTH, pAttributeValue, _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.
									// Append created end point structure to the list.
									pEndPoint -> pNextEndPoint = 0;		// Set list terminator.
									if ( pEndPointList == 0 )
										pEndPointList = pEndPoint;
									else
										{
										// Append to end of list.
										pTempEndPoint = pEndPointList;
										while ( pTempEndPoint != 0 && pTempEndPoint -> pNextEndPoint != 0 )
											pTempEndPoint = pTempEndPoint -> pNextEndPoint;
										pTempEndPoint -> pNextEndPoint = pEndPoint;
										}
									}
								}
							if ( pEndPoint == 0 )
								{
								RespondToError( MODULE_CONFIG, CONFIG_ERROR_PARSE_ALLOCATION );
								bNoError = FALSE;
								}
							}
						}
					}
				if ( !bSkipLine )
					{
					if ( TextLine[ 0 ] == '}' )
						{
						ParseState = PARSE_STATE_UNSPECIFIED;
						bOpenBracketEncountered = FALSE;
						}
					else if ( ParseState != PARSE_STATE_UNSPECIFIED && bOpenBracketEncountered )
						{
						switch ( ParseState )
							{
							case PARSE_STATE_OPERATION:
								bNoError = ParseOperationConfigurationLine( TextLine, pProductOperation );
								break;
							case PARSE_STATE_CONFIGURATION:
								bNoError = ParseConfigurationLine( TextLine );
								break;
							case PARSE_STATE_ENDPOINT:
								bNoError = ParseEndPointConfigurationLine( TextLine, pEndPoint );
								break;
							}
						}
					}
				}
			}
		while ( bNoError && !bEndOfFile && !bFileReadError );
		fclose( pCfgFile );
		}
	else
		RespondToError( MODULE_CONFIG, CONFIG_ERROR_OPEN_CFG_FILE );

	return ( bNoError && !bFileReadError );
}


ENDPOINT *CreateEndPoint()
{
	ENDPOINT	*pEndPoint;

	pEndPoint = (ENDPOINT*)malloc(  sizeof( ENDPOINT ) );
	if ( pEndPoint != 0 )
		{
		// Initialize all structure members to zero or equivalent.
		pEndPoint -> Name[0] = '\0';						// *[1] Eliminate call to strcpy.
		pEndPoint -> AE_TITLE[0] = '\0';					// *[1] Eliminate call to strcpy.
		pEndPoint -> EndPointType = ENDPOINT_TYPE_UNSPECIFIED;
		pEndPoint -> NetworkAddress[0] = '\0';				// *[1] Eliminate call to strcpy.
		pEndPoint -> Directory[0] = '\0';					// *[1] Eliminate call to strcpy.
		pEndPoint -> pNextEndPoint = 0;
		}
	else
		RespondToError( MODULE_CONFIG, CONFIG_ERROR_INSUFFICIENT_MEMORY );

	return pEndPoint;
}


void InitConfiguration()
{
	ServiceConfiguration.ThisTransferNodeName[ 0 ] = '\0';					// *[1] Eliminate call to strcpy.
	ServiceConfiguration.ConfigDirectory[0] = '\0';							// *[1] Eliminate call to strcpy.
	strncpy_s( ServiceConfiguration.QueuedFilesDirectory, MAX_CFG_STRING_LENGTH, TransferService.ProgramDataPath, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
	strncat_s( ServiceConfiguration.QueuedFilesDirectory, MAX_CFG_STRING_LENGTH, "Queued Files\\", _TRUNCATE );						// *[1] Replaced strcat with strncat_s.
	strncpy_s( ServiceConfiguration.ErroredFilesDirectory, MAX_CFG_STRING_LENGTH, TransferService.ProgramDataPath, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
	strncat_s( ServiceConfiguration.ErroredFilesDirectory, MAX_CFG_STRING_LENGTH, "Errored Files\\", _TRUNCATE );					// *[1] Replaced strcat with strncat_s.
	// Preset the abstracts local directory to the default so that user messages
	// will be sent to the (default) location expected by BViewer.
	strncpy_s( ServiceConfiguration.AbstractsDirectory, MAX_CFG_STRING_LENGTH, TransferService.ProgramDataPath, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
	strncat_s( ServiceConfiguration.AbstractsDirectory, MAX_CFG_STRING_LENGTH, "Abstracts\\Local\\", _TRUNCATE );					// *[1] Replaced strcat with strncat_s.
	strncpy_s( ServiceConfiguration.ExportsDirectory, MAX_CFG_STRING_LENGTH, TransferService.ProgramDataPath, _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
	strncat_s( ServiceConfiguration.ExportsDirectory, MAX_CFG_STRING_LENGTH, "Abstracts\\Export\\", _TRUNCATE );					// *[1] Replaced strcat with strncat_s.
	ServiceConfiguration.NetworkAddress[0] = '\0';							// *[1] Eliminate call to strcpy.
	ServiceConfiguration.DicomImageArchiveDirectory[0] = '\0';				// *[1] Eliminate call to strcpy.
	ServiceConfiguration.ExportsDirectory[0] = '\0';						// *[1] Eliminate call to strcpy.
	ServiceConfiguration.bTrustSpecifiedTransferSyntaxFromLocalStorage = TRUE;
	ServiceConfiguration.bTrustSpecifiedTransferSyntaxFromNetwork = TRUE;
	ServiceConfiguration.MinimumFreeSpaceStorageRequirementInMegabytes = 100L;
	ServiceConfiguration.bArchiveAXTOuputFiles = FALSE;
	ServiceConfiguration.bEnableSurvey = FALSE;
	ServiceConfiguration.bComposeDicomOutputFile = FALSE;
	ServiceConfiguration.bApplyManualDicomEdits = FALSE;
}


void InitializeOperationConfiguration( BOOL bProgramExit )
{
	if ( pPrimaryOperationList != 0 )
		EraseProductOperationList();
	if ( pEndPointList != 0 )
		EraseEndPointList( &pEndPointList );
	// Link the operations to the primary list.
	if ( !bProgramExit )												// *[1] Add test to avoid memory leak on program exit.
		LinkOperationsToList();
}


BOOL ParseConfigurationLine( char *pTextLine )
{
	BOOL			bNoError = TRUE;
	char			TextLine[ MAX_CFG_STRING_LENGTH ];
	char			*pAttributeName;
	char			*pAttributeValue;
	BOOL			bSkipLine = FALSE;

	strncpy_s( TextLine, MAX_CFG_STRING_LENGTH, pTextLine, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
	// Look for validly formatted attribute name and value.  Find a colon or an end-of-line.
	pAttributeName = strtok( TextLine, ":\n" );
	if ( pAttributeName == NULL )
		bSkipLine = TRUE;			// If neither found, skip this line.
	if ( !bSkipLine )
		{
		pAttributeValue = strtok( NULL, "\n" );  // Point to the value following the colon.
		if ( pAttributeValue == NULL )
			{
			RespondToError( MODULE_CONFIG, CONFIG_ERROR_PARSE_ATTRIB_VALUE );
			bNoError = FALSE;
			}
		else
			TrimBlanks( pAttributeValue );
		if ( bNoError )
			{
			if ( _stricmp( pAttributeName, "ABSTRACTS DIRECTORY" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )																			// If this is an absolute address.
					ServiceConfiguration.AbstractsDirectory[0] = '\0';																// *[1] Eliminate call to strcpy.
				else
					strncpy_s( ServiceConfiguration.AbstractsDirectory,
								MAX_CFG_STRING_LENGTH, TransferService.ProgramDataPath, _TRUNCATE );								// *[1] Replaced strcpy with strncpy_s.
				strncat_s( ServiceConfiguration.AbstractsDirectory, MAX_CFG_STRING_LENGTH, pAttributeValue, _TRUNCATE );			// *[1] Replaced strncat with strncat_s.
				}
			else if ( _stricmp( pAttributeName, "ABSTRACT EXPORT DIRECTORY" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )																			// If this is an absolute address.
					ServiceConfiguration.ExportsDirectory[0] = '\0';																// *[1] Eliminate call to strcpy.
				else
					strncpy_s( ServiceConfiguration.ExportsDirectory,
								MAX_CFG_STRING_LENGTH, TransferService.ProgramDataPath, _TRUNCATE );								// *[1] Replaced strcpy with strncpy_s.
				strncat_s( ServiceConfiguration.ExportsDirectory, MAX_CFG_STRING_LENGTH, pAttributeValue, _TRUNCATE );				// *[1] Replaced strncat with strncat_s.
				}
			else if ( _stricmp( pAttributeName, "DICOM IMAGE FILE ARCHIVE" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )																			// If this is an absolute address.
					ServiceConfiguration.DicomImageArchiveDirectory[0] = '\0';														// *[1] Eliminate call to strcpy.
				else
					strncpy_s( ServiceConfiguration.DicomImageArchiveDirectory,
								MAX_CFG_STRING_LENGTH, TransferService.ProgramDataPath, _TRUNCATE );								// *[1] Replaced strcpy with strncpy_s.
				strncat_s( ServiceConfiguration.DicomImageArchiveDirectory, MAX_CFG_STRING_LENGTH, pAttributeValue, _TRUNCATE );	// *[1] Replaced strncat with strncat_s.
				}
			else if ( _stricmp( pAttributeName, "ADDRESS" ) == 0 )
				{
				ServiceConfiguration.NetworkAddress[ 0 ] = '\0';																	// *[1] Eliminate call to strcpy.
				strncat_s( ServiceConfiguration.NetworkAddress, MAX_CFG_STRING_LENGTH, pAttributeValue, _TRUNCATE );				// *[1] Replaced strncat with strncat_s.
				EndPointNetworkIn.NetworkAddress[ 0 ] = '\0';																		// *[1] Eliminate call to strcpy.
				strncat_s( EndPointNetworkIn.NetworkAddress, MAX_CFG_STRING_LENGTH, pAttributeValue, _TRUNCATE );					// *[1] Replaced strncat with strncat_s.
				}
			else if ( _stricmp( pAttributeName, "TRUST NETWORK SYNTAX" ) == 0 )
				{
				if ( _stricmp( pAttributeValue, "YES" ) == 0 )
					ServiceConfiguration.bTrustSpecifiedTransferSyntaxFromNetwork = TRUE;
				else if ( _stricmp( pAttributeValue, "NO" ) == 0 )
					ServiceConfiguration.bTrustSpecifiedTransferSyntaxFromNetwork = FALSE;
				}
			else if ( _stricmp( pAttributeName, "TRUST STORED SYNTAX" ) == 0 )
				{
				if ( _stricmp( pAttributeValue, "YES" ) == 0 )
					ServiceConfiguration.bTrustSpecifiedTransferSyntaxFromLocalStorage = TRUE;
				else if ( _stricmp( pAttributeValue, "NO" ) == 0 )
					ServiceConfiguration.bTrustSpecifiedTransferSyntaxFromLocalStorage = FALSE;
				}
			else if ( _stricmp( pAttributeName, "LOGGING DETAIL" ) == 0 )
				{
				// This attribute is obsolete, but is retained here for backward
				// compatibility with older configuration files.
				}
			else if ( _stricmp( pAttributeName, "MINIMUM FREE STORAGE (MB)" ) == 0 )
				{
				ServiceConfiguration.MinimumFreeSpaceStorageRequirementInMegabytes = atol( pAttributeValue );
				if ( ServiceConfiguration.MinimumFreeSpaceStorageRequirementInMegabytes < 1 )
					ServiceConfiguration.MinimumFreeSpaceStorageRequirementInMegabytes = 100;
				}

			else if ( _stricmp( pAttributeName, "ARCHIVE AXT OUTPUT" ) == 0 )
				{
				if ( _stricmp( pAttributeValue, "YES" ) == 0 )
					ServiceConfiguration.bArchiveAXTOuputFiles = TRUE;
				else
					ServiceConfiguration.bArchiveAXTOuputFiles = FALSE;
				}
			else if ( _stricmp( pAttributeName, "COMPOSE DICOM OUTPUT" ) == 0 )
				{
				if ( _stricmp( pAttributeValue, "YES" ) == 0 )
					ServiceConfiguration.bComposeDicomOutputFile = TRUE;
				else
					ServiceConfiguration.bComposeDicomOutputFile = FALSE;
				}
			else if ( _stricmp( pAttributeName, "APPLY MANUAL DICOM EDITS" ) == 0 )
				{
				if ( _stricmp( pAttributeValue, "YES" ) == 0 )
					ServiceConfiguration.bApplyManualDicomEdits = TRUE;
				else
					ServiceConfiguration.bApplyManualDicomEdits = FALSE;
				}
			else if ( _stricmp( pAttributeName, "ENABLE SURVEY" ) == 0 )
				{
				if ( _stricmp( pAttributeValue, "YES" ) == 0 )
					ServiceConfiguration.bEnableSurvey = TRUE;
				else
					ServiceConfiguration.bEnableSurvey = FALSE;
				}

			else
				{
				RespondToError( MODULE_CONFIG, CONFIG_ERROR_PARSE_UNKNOWN_ATTR );
				bNoError = FALSE;
				}
			}
		}
	if ( !bNoError )
		{
		strncpy_s( TextLine, MAX_CFG_STRING_LENGTH, "Error in configuration line:  ", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
		strncat_s( TextLine, MAX_CFG_STRING_LENGTH, pTextLine, _TRUNCATE );								// *[1] Replaced strncat with strncat_s.
		LogMessage( TextLine, MESSAGE_TYPE_ERROR );
		}

	return bNoError;
}


BOOL ParseOperationConfigurationLine( char *pTextLine, PRODUCT_OPERATION *pProductOperation )
{
	BOOL			bNoError = TRUE;
	char			TextLine[ MAX_CFG_STRING_LENGTH ];
	char			*pAttributeName;
	char			*pAttributeValue;
	BOOL			bSkipLine = FALSE;
	struct tm		TimeSpec;
	int				nArgs;

	strncpy_s( TextLine, MAX_CFG_STRING_LENGTH, pTextLine, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
	// Look for validly formatted attribute name and value.  Find a colon or an end-of-line.
	pAttributeName = strtok( TextLine, ":\n" );
	if ( pAttributeName == NULL )
		bSkipLine = TRUE;			// If neither found, skip this line.
	if ( !bSkipLine )
		{
		pAttributeValue = strtok( NULL, "\n" );  // Point to the value following the colon.
		if ( pAttributeValue == NULL )
			{
			RespondToError( MODULE_CONFIG, CONFIG_ERROR_PARSE_ATTRIB_VALUE );
			bNoError = FALSE;
			}
		else
			TrimBlanks( pAttributeValue );

		if ( bNoError && pProductOperation != 0 )
			{
			if ( _stricmp( pAttributeName, "WATCH FREQUENCY" ) == 0 )
				{
				nArgs = sscanf( pAttributeValue, "%2d:%2d:%2d", &TimeSpec.tm_hour, &TimeSpec.tm_min, &TimeSpec.tm_sec );
				if ( nArgs != 3 )
					{
					RespondToError( MODULE_CONFIG, CONFIG_ERROR_PARSE_TIME );
					bNoError = FALSE;
					}
				else
					{
					pProductOperation -> OperationTimeInterval =
								TimeSpec.tm_hour * 3600 + TimeSpec.tm_min * 60 + TimeSpec.tm_sec;
					}
				}
			else if ( _stricmp( pAttributeName, "SOURCE DELETE ON COMPLETION" ) == 0 )
				pProductOperation -> bInputDeleteSourceOnCompletion =
								( pAttributeValue[0] == 'Y' || pAttributeValue[0] == 'y' );
			else if ( _stricmp( pAttributeName, "DEPENDENT OPERATION" ) == 0 )
				strncpy_s( pProductOperation -> DependentOperationName, MAX_CFG_STRING_LENGTH, pAttributeValue, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
			else if ( _stricmp( pAttributeName, "ENABLED" ) == 0 )
				if ( pProductOperation != &OperationReceive )
					pProductOperation -> bEnabled =
								( pAttributeValue[0] == 'Y' || pAttributeValue[0] == 'y' );
				else
					// The network receive operation should never be manually enabled.  Each
					// cloned copy is enabled automatically, when created.
					pProductOperation -> bEnabled = FALSE;
			else
				{
				RespondToError( MODULE_CONFIG, CONFIG_ERROR_PARSE_UNKNOWN_ATTR );
				bNoError = FALSE;
				}
			}		// ... end if pProductOperation != 0
		}
	if ( !bNoError )
		{
		strncpy_s( TextLine, MAX_CFG_STRING_LENGTH, "Error in operation line:  ", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
		strncat_s( TextLine, MAX_CFG_STRING_LENGTH, pTextLine, _TRUNCATE );							// *[1] Replaced strncat with strncat_s.
		LogMessage( TextLine, MESSAGE_TYPE_ERROR );
		}

	return bNoError;
}


BOOL ParseEndPointConfigurationLine( char *pTextLine, ENDPOINT *pEndPoint )
{
	BOOL			bNoError = TRUE;
	char			TextLine[ MAX_CFG_STRING_LENGTH ];
	char			*pAttributeName;
	char			*pAttributeValue;
	BOOL			bSkipLine = FALSE;

	strncpy_s( TextLine, MAX_CFG_STRING_LENGTH, pTextLine, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
	// Look for validly formatted attribute name and value.  Find a colon or an end-of-line.
	pAttributeName = strtok( TextLine, ":\n" );
	if ( pAttributeName == NULL )
		bSkipLine = TRUE;			// If neither found, skip this line.
	if ( !bSkipLine )
		{
		pAttributeValue = strtok( NULL, "\n" );  // Point to the value following the colon.
		if ( pAttributeValue == NULL )
			{
			RespondToError( MODULE_CONFIG, CONFIG_ERROR_PARSE_ATTRIB_VALUE );
			bNoError = FALSE;
			}
		else
			TrimBlanks( pAttributeValue );
		if ( bNoError && pEndPoint != 0 )
			{
			if ( _stricmp( pAttributeName, "AE_TITLE" ) == 0 )
				strncpy_s( pEndPoint -> AE_TITLE, MAX_CFG_STRING_LENGTH, pAttributeValue, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
			else if ( _stricmp( pAttributeName, "TYPE" ) == 0 )
				{
				if ( _stricmp( pAttributeValue, "FILE" ) == 0 )
					pEndPoint -> EndPointType = ENDPOINT_TYPE_FILE;
				else if ( _stricmp( pAttributeValue, "NETWORK" ) == 0 )
					pEndPoint -> EndPointType = ENDPOINT_TYPE_NETWORK;
				else
					{
					pEndPoint -> EndPointType = ENDPOINT_TYPE_UNSPECIFIED;
					RespondToError( MODULE_CONFIG, CONFIG_ERROR_PARSE_ENDPOINT_TYPE );
					bNoError = FALSE;
					}
				}
			else if ( _stricmp( pAttributeName, "ADDRESS" ) == 0 )
				strncpy_s( pEndPoint -> NetworkAddress, MAX_CFG_STRING_LENGTH, pAttributeValue, _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
			else if ( _stricmp( pAttributeName, "DIRECTORY" ) == 0 )
				{
				if ( strchr( pAttributeValue, ':' ) != 0 )		// If this is an absolute address.
					strncpy_s( pEndPoint -> Directory, MAX_CFG_STRING_LENGTH, pAttributeValue, _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.
				else
					{
					strncpy_s( pEndPoint -> Directory, MAX_CFG_STRING_LENGTH, TransferService.ProgramDataPath, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
					strncat_s( pEndPoint -> Directory, MAX_CFG_STRING_LENGTH, pAttributeValue, _TRUNCATE );						// *[1] Replaced strcat with strncat_s.
					}
				}
			}
		}
	if ( !bNoError )
		{
		strncpy_s( TextLine, MAX_CFG_STRING_LENGTH, "Error in end point line:  ", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
		strncat_s( TextLine, MAX_CFG_STRING_LENGTH, pTextLine, _TRUNCATE );							// *[1] Replaced strncat with strncat_s.
		LogMessage( TextLine, MESSAGE_TYPE_ERROR );
		}

	return bNoError;
}




ENDPOINT *LookUpEndPointAddress( char *pEndPointName )
{
	BOOL				bNoError = TRUE;
	ENDPOINT			*pEndPoint;
	BOOL				bMatchingEndPointFound;

	bMatchingEndPointFound = FALSE;
	if ( strlen( pEndPointName ) > 0 )
		{
		pEndPoint = pEndPointList;
		while( pEndPoint != 0 && !bMatchingEndPointFound )
			{
			if ( _stricmp( pEndPointName, pEndPoint -> Name ) == 0 )
				bMatchingEndPointFound = TRUE;
			else
				pEndPoint = pEndPoint -> pNextEndPoint;
			}
		}
	else
		pEndPoint = 0;

	return pEndPoint;
}


void EraseProductOperationList()
{
	PRODUCT_OPERATION		*pProductOperation;
	PRODUCT_OPERATION		*pPrevProductOperation;

	pProductOperation = pPrimaryOperationList;
	while ( pProductOperation != 0 )
		{
		pPrevProductOperation = pProductOperation;
		pProductOperation = pProductOperation -> pNextOperation;
		// Don't deallocate the hard-coded operation structures.
		if ( pPrevProductOperation != &OperationWatch && pPrevProductOperation != &OperationListen && pPrevProductOperation != &OperationReceive )
			free( pPrevProductOperation );
		}
	pPrimaryOperationList = 0;
}


void EraseEndPointList( ENDPOINT **ppEndPointList )
{
	ENDPOINT			*pEndPoint;
	ENDPOINT			*pPrevEndPoint;

	pEndPoint = *ppEndPointList;
	while ( pEndPoint != 0 )
		{
		pPrevEndPoint = pEndPoint;
		pEndPoint = pEndPoint -> pNextEndPoint;
		free( pPrevEndPoint );
		}
	*ppEndPointList = 0;
}


