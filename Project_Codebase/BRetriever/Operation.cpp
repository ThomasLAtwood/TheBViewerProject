//Operation.cpp : Implements the data structures and functions related to
//	image file processing operations, such as receiving the file and
//	extracting the image.
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
#include <process.h>
#include "ReportStatus.h"
#include "Configuration.h"
#include "Operation.h"
#include "ProductDispatcher.h"


BOOL								bProductOperationsLaunched = FALSE;	// If timed on and off are needed, logic will need to be added.

extern PRODUCT_OPERATION			*pPrimaryOperationList;
extern void							TerminateListeningSocket();

//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		OperationsModuleInfo = { MODULE_OPERATIONS, "Operations Module", InitProductOperationsModule, CloseProductOperationsModule };


static ERROR_DICTIONARY_ENTRY	ProductOperationsErrorCodes[] =
			{
				{ OPERATIONS_ERROR_CREATE_SEMAPHORE			, "An error occurred creating the remote control sleep interruption semaphore." },
				{ OPERATIONS_ERROR_SEMAPHORE_WAIT			, "An undiagnosed error occurred while waiting for the remote control sleep interruption semaphore." },
				{ OPERATIONS_ERROR_OPERATION_DISABLED		, "An attempt was made to launch a disabled operation." },
				{ OPERATIONS_ERROR_START_OP_THREAD			, "An error occurred starting an operation thread." },
				{ 0											, NULL }
			};


static ERROR_DICTIONARY_MODULE		ProductOperationsStatusErrorDictionary =
										{
										MODULE_OPERATIONS,
										ProductOperationsErrorCodes,
										OPERATIONS_ERROR_DICT_LENGTH,
										0
										};


// This function must be called before any other function in this module.
void InitProductOperationsModule()
{
	LinkModuleToList( &OperationsModuleInfo );
	RegisterErrorDictionary( &ProductOperationsStatusErrorDictionary );
}


void CloseProductOperationsModule()
{
}


PRODUCT_OPERATION *CreateProductOperation()
{
	PRODUCT_OPERATION	*pProductOperation;

	pProductOperation = (PRODUCT_OPERATION*)malloc(  sizeof( PRODUCT_OPERATION ) );
	if ( pProductOperation != 0 )
		{
		// Initialize all structure members to zero or equivalent.
		strcpy( pProductOperation -> OperationName, "" );
		pProductOperation -> OperationTimeInterval = 100000;
		pProductOperation -> pInputEndPoint = 0;
		pProductOperation -> bInputDeleteSourceOnCompletion = FALSE;
		pProductOperation -> pOutputEndPoint = 0;
		pProductOperation -> bEnabled = FALSE;
		memset( (char*)&pProductOperation -> OpnState, '\0', sizeof(OPERATION_STATE) );
		pProductOperation -> OpnState.StatusCode = OPERATION_STATUS_UNKNOWN;
		pProductOperation -> OpnState.hOperationThreadHandle = 0;
		pProductOperation -> OpnState.OperationThreadID = 0;
		pProductOperation -> OpnState.ThreadFunction = 0;
		pProductOperation -> OpnState.SocketDescriptor = INVALID_SOCKET;
		pProductOperation -> OpnState.hSleepSemaphore = 0;
		strcpy( pProductOperation -> OpnState.SleepSemaphoreName, "" );
		pProductOperation -> OpnState.bOKtoProcessThisStudy = FALSE;
		pProductOperation -> OpnState.DirectorySearchLevel = 0;
		pProductOperation -> OpnState.pProductItem = 0;
		pProductOperation -> OpnState.pDicomAssociation = 0;
		strcpy( pProductOperation -> DependentOperationName, "" );
		pProductOperation -> pDependentOperation = 0;
		pProductOperation -> pNextOperation = 0;
		}

	return pProductOperation;
}


void DeleteProductOperation( PRODUCT_OPERATION *pProductOperation )
{
	if ( pProductOperation != 0 )
		free( pProductOperation );
}


// This function needs to be called on a regular basis.  It starts and stops operations
// according to their scheduled times.
void ControlProductOperations()
{
	BOOL				bNoError = TRUE;
	PRODUCT_OPERATION	*pProductOperation;
	PRODUCT_OPERATION	*pDependentOperation;
	char				TextLine[ MAX_CFG_STRING_LENGTH ];
	HANDLE				hProcess;
	DWORD				ProcessPriorityClass;

	if ( !bProductOperationsLaunched )
		{
		hProcess = GetCurrentProcess();
		ProcessPriorityClass = GetPriorityClass( hProcess );
		if ( ProcessPriorityClass == NORMAL_PRIORITY_CLASS )
			LogMessage( "Process priority is normal.", MESSAGE_TYPE_SUPPLEMENTARY );
		else if ( ProcessPriorityClass == REALTIME_PRIORITY_CLASS )
			LogMessage( "Process priority is realtime.", MESSAGE_TYPE_SUPPLEMENTARY );
		else if ( ProcessPriorityClass == IDLE_PRIORITY_CLASS )
			LogMessage( "Process priority is idle.", MESSAGE_TYPE_SUPPLEMENTARY );
		else if ( ProcessPriorityClass == BELOW_NORMAL_PRIORITY_CLASS )
			LogMessage( "Process priority is below normal.", MESSAGE_TYPE_SUPPLEMENTARY );
		else if ( ProcessPriorityClass == ABOVE_NORMAL_PRIORITY_CLASS )
			LogMessage( "Process priority is above normal.", MESSAGE_TYPE_SUPPLEMENTARY );
		else if ( ProcessPriorityClass == HIGH_PRIORITY_CLASS )
			LogMessage( "Process priority is high priority.", MESSAGE_TYPE_SUPPLEMENTARY );

		bProductOperationsLaunched = TRUE;
		pProductOperation = pPrimaryOperationList;
		while ( pProductOperation != 0 )
			{
			if ( strlen( pProductOperation -> DependentOperationName ) > 0 )
				{
				// Locate the dependent operation and set a pointer to it.
				pDependentOperation = pPrimaryOperationList;
				while ( pDependentOperation != 0 )
					{
					if ( _stricmp( pDependentOperation -> OperationName, pProductOperation -> DependentOperationName ) == 0 )
						pProductOperation -> pDependentOperation = pDependentOperation;
					pDependentOperation = pDependentOperation -> pNextOperation;
					}	
				}
			if ( pProductOperation -> bEnabled )
				{
				bNoError = LaunchOperation( pProductOperation );
				if ( !bNoError )
					{
					_snprintf_s( TextLine, MAX_CFG_STRING_LENGTH, _TRUNCATE,								// *[1] Replaced sprintf() with _snprintf_s.
									"Error launching operation:  %s", pProductOperation -> OperationName );
					LogMessage( TextLine, MESSAGE_TYPE_ERROR );
					}
				}
			pProductOperation = pProductOperation -> pNextOperation;
			}
		}
}


BOOL LaunchOperation( PRODUCT_OPERATION *pProductOperation )
{
	BOOL						bNoError = TRUE;
	char						TextLine[ MAX_FILE_SPEC_LENGTH ];
	OPERATION_THREAD_FUNCTION	OpnThreadFunction;
	int							ThreadPriority;

	if ( pProductOperation -> bEnabled )
		{
		// Create a semaphore to allow interruption of the sleep intervals of the
		// transfer operation in between normal cycle times.  Initial count
		// is zero, so that it normally blocks.  (Max count is 1.)  Transfer operation
		// can release this semaphore to cancel any remainder of a sleep interval.
		strcpy( pProductOperation -> OpnState.SleepSemaphoreName, pProductOperation -> OperationName );
		strcat( pProductOperation -> OpnState.SleepSemaphoreName, "BRetrieverSleepSemaphore" );
		pProductOperation -> OpnState.hSleepSemaphore = CreateSemaphore( NULL, 0L, 1L,
															pProductOperation -> OpnState.SleepSemaphoreName );
		if ( pProductOperation -> OpnState.hSleepSemaphore == NULL )
			RespondToError( MODULE_OPERATIONS, OPERATIONS_ERROR_CREATE_SEMAPHORE );
		_snprintf_s( TextLine, MAX_FILE_SPEC_LENGTH, _TRUNCATE, "Starting thread for %s.", pProductOperation -> OperationName );			// *[1] Replaced sprintf() with _snprintf_s.
		LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
		if ( pProductOperation -> OpnState.ThreadFunction != 0 )
			{
			OpnThreadFunction = pProductOperation -> OpnState.ThreadFunction;
			pProductOperation -> OpnState.hOperationThreadHandle =
					(HANDLE)_beginthreadex(	NULL,						// No security issues for child processes.
											0,							// Use same stack size as parent process.
											OpnThreadFunction,			// Thread function.
											(void*)pProductOperation,	// Argument for thread function.
											0,							// Initialize thread state as running.
											&pProductOperation -> OpnState.OperationThreadID );
			if ( strcmp( pProductOperation -> OperationName, "Receive From Network" ) == 0 )
				SetThreadPriority( pProductOperation -> OpnState.hOperationThreadHandle, THREAD_PRIORITY_HIGHEST );
			else if ( strcmp( pProductOperation -> OperationName, "Receive From Folder" ) == 0 )
				SetThreadPriority( pProductOperation -> OpnState.hOperationThreadHandle, THREAD_PRIORITY_ABOVE_NORMAL );

			ThreadPriority = GetThreadPriority( pProductOperation -> OpnState.hOperationThreadHandle );
			if ( ThreadPriority == THREAD_PRIORITY_NORMAL )
				LogMessage( "  Thread started at normal priority.", MESSAGE_TYPE_SUPPLEMENTARY );
			else if ( ThreadPriority == THREAD_PRIORITY_TIME_CRITICAL )
				LogMessage( "  Thread started at realtime priority.", MESSAGE_TYPE_SUPPLEMENTARY );
			else if ( ThreadPriority == THREAD_PRIORITY_IDLE )
				LogMessage( "  Thread started at idle priority.", MESSAGE_TYPE_SUPPLEMENTARY );
			else if ( ThreadPriority == THREAD_PRIORITY_BELOW_NORMAL )
				LogMessage( "  Thread started at below normal priority.", MESSAGE_TYPE_SUPPLEMENTARY );
			else if ( ThreadPriority == THREAD_PRIORITY_ABOVE_NORMAL )
				LogMessage( "  Thread started at above normal priority.", MESSAGE_TYPE_SUPPLEMENTARY );
			else if ( ThreadPriority == THREAD_PRIORITY_HIGHEST )
				LogMessage( "  Thread started at high priority.", MESSAGE_TYPE_SUPPLEMENTARY );
			else if ( ThreadPriority == THREAD_PRIORITY_ERROR_RETURN )
				LogMessage( "  Thread priority is not accessible.", MESSAGE_TYPE_SUPPLEMENTARY );
			}
		else
			LogMessage( "    No thread function.", MESSAGE_TYPE_SUPPLEMENTARY );
		if ( pProductOperation -> OpnState.hOperationThreadHandle == 0 )
			{
			RespondToError( MODULE_OPERATIONS, OPERATIONS_ERROR_START_OP_THREAD );
			bNoError = FALSE;
			}
		else
			{
			// Mark the operation in the running state.
			pProductOperation -> OpnState.StatusCode |= OPERATION_STATUS_RUNNING;
			Sleep( 100 );		// Pause for a tenth second to give the new thread time to run.
			}
		}
	else
		{
		RespondToError( MODULE_OPERATIONS, OPERATIONS_ERROR_OPERATION_DISABLED );
		_snprintf_s( TextLine, MAX_FILE_SPEC_LENGTH, _TRUNCATE, "    Disabled operation:  %s.", pProductOperation -> OperationName );			// *[1] Replaced sprintf() with _snprintf_s.
		LogMessage( TextLine, MESSAGE_TYPE_NORMAL_LOG );
		bNoError = FALSE;
		}

	return bNoError;
}


void EnterOperationCycleWaitInterval( PRODUCT_OPERATION *pProductOperation, BOOL bEnableDependentOperations, BOOL *pbTerminateOperation )
{
	DWORD					SleepIntervalMSec;
	char					TextLine[ 1096 ];
	DWORD					OperationSleepWaitResponse;

	if ( !*pbTerminateOperation )
		*pbTerminateOperation = CheckForOperationTerminationRequest( pProductOperation );
	if ( !*pbTerminateOperation )
		{
		if ( bEnableDependentOperations && pProductOperation -> pDependentOperation != 0 )
			{
			// Enable any dependent operation to cycle.
			ReleaseSemaphore( pProductOperation -> pDependentOperation -> OpnState.hSleepSemaphore, 1L, NULL );
			}
		SleepIntervalMSec = (DWORD)pProductOperation -> OperationTimeInterval * 1000L;
		strcpy( TextLine, pProductOperation -> OperationName );
		OperationSleepWaitResponse = WaitForSingleObject( pProductOperation -> OpnState.hSleepSemaphore, SleepIntervalMSec );
		if ( OperationSleepWaitResponse == WAIT_OBJECT_0 )
			{
			strcat( TextLine, " operation activated." );
			LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
			}
		else if ( OperationSleepWaitResponse != WAIT_TIMEOUT )
			{
			LogMessage( TextLine, MESSAGE_TYPE_ERROR );
			RespondToError( MODULE_OPERATIONS, OPERATIONS_ERROR_SEMAPHORE_WAIT );
			}
		}
}


BOOL CheckForOperationTerminationRequest( PRODUCT_OPERATION *pProductOperation )
{
	BOOL			bTerminationRequested;
	
	bTerminationRequested = (( pProductOperation -> OpnState.StatusCode & OPERATION_STATUS_TERMINATION_REQUESTED ) != 0 );
	return bTerminationRequested;
}


void CloseOperation( PRODUCT_OPERATION *pProductOperation )
{
	char					TextLine[ 1096 ];

	// The operation thread is about to terminate.  Indicate that the operation is
	// no longer running.  It may be restarted the next time ControlProductOperations()
	// is called.
	pProductOperation -> OpnState.StatusCode &= ~( OPERATION_STATUS_RUNNING | OPERATION_STATUS_TERMINATION_REQUESTED );
	_snprintf_s( TextLine, 1096, _TRUNCATE, "Terminating Operation: %s", pProductOperation -> OperationName );			// *[1] Replaced sprintf() with _snprintf_s.
	LogMessage( TextLine, MESSAGE_TYPE_NORMAL_LOG );
	if ( pProductOperation -> OpnState.hSleepSemaphore != 0 )
		CloseHandle( pProductOperation -> OpnState.hSleepSemaphore );
	// When this function returns to the caller, the next expected instruction is to return 0,
	// thus terminating the thread.
}


void TerminateAllOperations()
{
	PRODUCT_OPERATION	*pProductOperation;
	char				TextLine[ MAX_CFG_STRING_LENGTH ];
	BOOL				bOperationsAreStillRunning;
	BOOL				bRunningOperationFound;
	
	// Loop through the operations and terminate any that are running.
	pProductOperation = pPrimaryOperationList;
	while ( pProductOperation != 0 )
		{
		if ( ( pProductOperation -> OpnState.StatusCode & OPERATION_STATUS_RUNNING ) != 0 )
			{
			// Request the termination of this operation.
			pProductOperation -> OpnState.StatusCode |= OPERATION_STATUS_TERMINATION_REQUESTED;
			_snprintf_s( TextLine, MAX_CFG_STRING_LENGTH, _TRUNCATE,													// *[1] Replaced sprintf() with _snprintf_s.
							"Requested termination of operation:  %s", pProductOperation -> OperationName );
			LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
			ReleaseSemaphore( pProductOperation -> OpnState.hSleepSemaphore, 1L, NULL );
			}
		pProductOperation = pProductOperation -> pNextOperation;
		}
	TerminateListeningSocket();
	// Enter a wait loop until all running operations have been successfully
	// shut down.
	bOperationsAreStillRunning = TRUE;
	while ( bOperationsAreStillRunning )
		{
		Sleep( 1000 );
		bRunningOperationFound = FALSE;
		pProductOperation = pPrimaryOperationList;
		// Check whether each operation is still running.
		while ( pProductOperation != 0 )
			{
			if ( ( pProductOperation -> OpnState.StatusCode & OPERATION_STATUS_RUNNING ) != 0 )
				bRunningOperationFound = TRUE;
			pProductOperation = pProductOperation -> pNextOperation;
			}
		if ( !bRunningOperationFound )
			bOperationsAreStillRunning = FALSE;
		}
	LogMessage( "All operations have terminated.", MESSAGE_TYPE_NORMAL_LOG );
	bProductOperationsLaunched = FALSE;
}



