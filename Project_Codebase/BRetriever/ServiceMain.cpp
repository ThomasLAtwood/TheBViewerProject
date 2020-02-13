// ServiceMain.cpp : Defines the entry point for the BRetriever service application.
//				Implements the Win32 service control functionality.
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
#include "Dicom.h"
#include "Abstract.h"
#include "ServiceMain.h"
#include "Configuration.h"
#include "Operation.h"
#include "ProductDispatcher.h"


// NOTE:  "SCM" refers to the Microsoft Windows "Service Control Manager" program,
// under the control of which this program operates.  This software implements the
// SCM applications interface specifications for this service.
//
// If you set
//					bRunAsService = FALSE;
//
// you can run this program directly as a console app, for example, in the debugger,
// without any of the constraints associated with running a service.
//

SERVICE_TABLE_ENTRY			ServiceTable[] =
								{
									{ "BRetriever", ServiceMain },
									{ NULL, NULL }
								};

SERVICE_STATUS				InitialServiceStatus =
								{
								SERVICE_WIN32_OWN_PROCESS,	// Not shared.
								SERVICE_START_PENDING,		// Current state, actual value must be set for
															//  each call to SetServiceStatus().
								SERVICE_ACCEPT_STOP,		// Can be any or all of the following:
															//  SERVICE_ACCEPT_STOP, SERVICE_ACCEPT_PAUSE_CONTINUE,
															//  SERVICE_ACCEPT_SHUTDOWN but probably not until successfully
															//  started.
								NO_ERROR,					// Current error status.  Alternative:  ERROR_SERVICE_SPECIFIC_ERROR.
								MAIN_ERROR_UNKNOWN,			// Replace with actual local error code.
								1,							// Counter for the number of consequtive PENDING status updates
															//  so SCM can tell that the service is still alive.
								5000						// Wait hint.
								};

SERVICE_STATUS				NormalServiceStatus =
								{
								SERVICE_WIN32_OWN_PROCESS,	// Not shared.
								SERVICE_RUNNING,			// Current state.
								SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN,
								NO_ERROR,					// Current error status.
								0,							// Replace with actual local error code.
								0,							// Counter for the number of consequtive PENDING status updates
															//  so SCM can tell that the service is still alive.
								0							// Wait hint.
								};

SERVICE_STATUS				MiscServiceStatus =
								{
								SERVICE_WIN32_OWN_PROCESS,	// Not shared.
								SERVICE_RUNNING,			// Current state.
								SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN,
								NO_ERROR,					// Current error status.
								0,							// Replace with actual local error code.
								0,							// Counter for the number of consequtive PENDING status updates
															//  so SCM can tell that the service is still alive.
								0							// Wait hint.
								};

SERVICE_STATUS				StoppedServiceStatus =
								{
								SERVICE_WIN32_OWN_PROCESS,	// Not shared.
								SERVICE_STOP_PENDING,		// Current state.
								SERVICE_ACCEPT_STOP,
								NO_ERROR,					// Current error status.
								0,							// Replace with actual local error code.
								0,							// Counter for the number of consequtive PENDING status updates
															//  so SCM can tell that the service is still alive.
								0							// Wait hint.
								};


extern CONFIGURATION		ServiceConfiguration;
extern HANDLE				hStatusSemaphore;

TRANSFER_SERVICE			TransferService;
// For debugging purposes, you can set the following flag to FALSE, then run
// BRetriever.exe as a normal program.  If you want it to run as a Windows
// service, this flag had better be TRUE.
//BOOL						bRunAsService = FALSE;
BOOL						bRunAsService = TRUE;
BOOL						bProgramTerminationRequested = FALSE;


void InitializeTransferService( TRANSFER_SERVICE *pTransferService )
{
	char				DriveSpecification[ FILE_PATH_STRING_LENGTH ];
	char				*pChar;

	pTransferService -> hServiceHandle = 0;
	pTransferService -> CurrentServiceState = 0;
	if ( bRunAsService )
		{
		pTransferService -> hEvents[0] = CreateEvent( NULL, FALSE, FALSE, "BRetriever Stop Event" );
		pTransferService -> hEvents[1] = CreateEvent( NULL, FALSE, FALSE, "BRetriever Pause Event" );
		pTransferService -> hEvents[2] = CreateEvent( NULL, FALSE, FALSE, "BRetriever Continue Event" );
		}
	strcpy( pTransferService -> ServiceName, "BRetriever" );

	strcpy( DriveSpecification, TransferService.ProgramPath );
	pChar = strchr( DriveSpecification, ':' );
	if ( pChar != NULL )
		*(pChar + 1) = '\0';
	strcpy( TransferService.ProgramDataPath, DriveSpecification );
	strcat( TransferService.ProgramDataPath, "\\ProgramData\\BViewer\\" );

	strcpy( TransferService.StudyDataDirectory, TransferService.ProgramDataPath );
	strcat( TransferService.StudyDataDirectory, "Data\\" );
	
	strcat( TransferService.ProgramDataPath, "BRetriever\\" );

	strcpy( pTransferService -> ServiceDirectory, TransferService.ProgramDataPath );
	strcat( pTransferService -> ServiceDirectory, "Service\\" );
	LocateOrCreateDirectory( pTransferService -> ServiceDirectory );	// Ensure directory exists.

	strcpy( pTransferService -> ConfigDirectory, TransferService.ProgramDataPath );
	strcat( pTransferService -> ConfigDirectory, "Config\\" );
	LocateOrCreateDirectory( pTransferService -> ConfigDirectory );	// Ensure directory exists.
	strcpy( pTransferService -> CfgFile, pTransferService -> ProgramDataPath );
	strcat( pTransferService -> CfgFile, "BRetriever.cfg" );

	strcpy( pTransferService -> BackupCfgFile, pTransferService -> ProgramDataPath );
	strcat( pTransferService -> BackupCfgFile, "BRetrieverBackup.cfg" );

	strcpy( pTransferService -> LogDirectory, TransferService.ProgramDataPath );
	strcat( pTransferService -> LogDirectory, "Log\\" );
	LocateOrCreateDirectory( pTransferService -> LogDirectory );	// Ensure directory exists.
	strcpy( pTransferService -> ServiceLogFile, pTransferService -> LogDirectory );
	strcat( pTransferService -> ServiceLogFile, "BRetriever.log" );
	strcpy( pTransferService -> SupplementaryLogFile, pTransferService -> LogDirectory );
	strcat( pTransferService -> SupplementaryLogFile, "BRetrieverDetail.log" );

	pTransferService -> bPrintToConsole = TRUE;
}


void TerminateTransferService( TRANSFER_SERVICE *pTransferService )
{
	if ( bRunAsService )
		{
		CloseHandle( pTransferService -> hEvents[ 0 ] );
		CloseHandle( pTransferService -> hEvents[ 1 ] );
		CloseHandle( pTransferService -> hEvents[ 2 ] );
		}
}


// This function runs on the parent process's main thread.  It
// is called by the Service Control Manager (SCM) to change or
// enquire about the state of the service.  It signals events
// to be responded to by the ServiceMain() thread.
void WINAPI ServiceControlHandler( DWORD RequestedState )
{
	// Ignore an redundant control request if we're already handling it.
	if ( TransferService.CurrentServiceState == RequestedState )
		return;

	switch ( RequestedState )
		{
		case SERVICE_CONTROL_STOP:
			LogMessage( "The service control manager has requested a stop.", MESSAGE_TYPE_NORMAL_LOG );
			TransferService.CurrentServiceState = RequestedState;
			SetEvent( TransferService.hEvents[ 0 ] );				// Signal the stop request to the processing thread;
			break;
		case SERVICE_CONTROL_PAUSE:
			TransferService.CurrentServiceState = RequestedState;
			SetEvent( TransferService.hEvents[ 1 ] );				// Signal the pause request to the processing thread;
			break;
		case SERVICE_CONTROL_CONTINUE:
			TransferService.CurrentServiceState = RequestedState;
			SetEvent( TransferService.hEvents[ 2 ] );				// Signal the continue request to the processing thread;
			break;
		case SERVICE_CONTROL_SHUTDOWN:
			TransferService.CurrentServiceState = RequestedState;
			LogMessage( "Performing controlled shutdown", MESSAGE_TYPE_ERROR );
			// Do what it takes to shut down the service.
			TerminateAllOperations();
			LogMessage( "Terminating BRetriever service.", MESSAGE_TYPE_SERVICE_CONTROL );
			CloseSoftwareModules();
			TerminateTransferService( &TransferService );
			break;
		default:
			UpdateStatusForSCM( &NormalServiceStatus );
		}
}


// Before calling this function, the entries in the ServiceStatus structure
// above have to be populated for this call to SetServiceStatus().
BOOL UpdateStatusForSCM( SERVICE_STATUS *pServiceStatus )
{
	BOOL					bNoError = TRUE;
	DWORD					SystemErrorCode;
	char					TextLine[ 1096 ];

	bNoError = SetServiceStatus( TransferService.hServiceHandle, pServiceStatus );
	if ( !bNoError )
		{
		RespondToError( MODULE_MAIN, MAIN_ERROR_SCM_SET_STATUS );
		SystemErrorCode = GetLastError();
		sprintf( TextLine, "Set service status system error code %d", SystemErrorCode );
		LogMessage( TextLine, MESSAGE_TYPE_ERROR );
		}
	return bNoError;
}


static char			*pTechSupportMsg = "Request technical support.";

void NotifyUserOfBRetrieverStartupError( unsigned long ErrorCode )
{
	USER_NOTIFICATION		UserNoticeDescriptor;

	RespondToError( MODULE_MAIN, ErrorCode );

	strcpy( UserNoticeDescriptor.Source, TransferService.ServiceName );
	UserNoticeDescriptor.ModuleCode = MODULE_MAIN;
	UserNoticeDescriptor.ErrorCode = ErrorCode;
	UserNoticeDescriptor.TypeOfUserResponseSupported = USER_RESPONSE_TYPE_ERROR | USER_RESPONSE_TYPE_CONTINUE;
	UserNoticeDescriptor.UserNotificationCause = USER_NOTIFICATION_CAUSE_RETRIEVAL_STARTUP_ERROR;
	UserNoticeDescriptor.UserResponseCode = 0L;
	strcpy( UserNoticeDescriptor.NoticeText, "The BRetriever service failed to start.\nReason:\n\n" );
	
	switch ( ErrorCode )
		{
		case MAIN_ERROR_SERVICE_DISPATCH:
			strcat( UserNoticeDescriptor.NoticeText, "No Windows service registration response." );
			strcpy( UserNoticeDescriptor.SuggestedActionText, pTechSupportMsg );
			break;
		case MAIN_ERROR_CMD_LINE:
			strcat( UserNoticeDescriptor.NoticeText, "There were errors in the command line." );
			strcpy( UserNoticeDescriptor.SuggestedActionText, pTechSupportMsg );
			break;
		case MAIN_ERROR_CFG_FILE:
			strcat( UserNoticeDescriptor.NoticeText, "Missing or corrupt configuration file." );
			strcpy( UserNoticeDescriptor.SuggestedActionText, pTechSupportMsg );
			break;
		case MAIN_ERROR_ENDPOINT_ASSIGNMENT:
			strcat( UserNoticeDescriptor.NoticeText, "Corrupt image routing assignments." );
			strcpy( UserNoticeDescriptor.SuggestedActionText, pTechSupportMsg );
			break;
		case MAIN_ERROR_DICOM_DICTIONARY:
			strcat( UserNoticeDescriptor.NoticeText, "Missing or corrupt Dicom Dictionary file." );
			strcpy( UserNoticeDescriptor.SuggestedActionText, pTechSupportMsg );
			break;
		case MAIN_ERROR_ABSTRACT_CONFIGURATION:
			strcpy( UserNoticeDescriptor.NoticeText, "BRetriever is not fully operational.\nReason:\n\n" );
			strcat( UserNoticeDescriptor.NoticeText, "Missing or corrupt abstract configuration files." );
			strcpy( UserNoticeDescriptor.SuggestedActionText, pTechSupportMsg );
			break;
		case MAIN_ERROR_SERVICE_NAME:
		case MAIN_ERROR_SERVICE_NOT_FOUND:
			strcat( UserNoticeDescriptor.NoticeText, "The system doesn't recognize the BRetriever service." );
			strcpy( UserNoticeDescriptor.SuggestedActionText, pTechSupportMsg );
			break;
		case MAIN_ERROR_UNKNOWN:
			strcat( UserNoticeDescriptor.NoticeText, "Windows doesn't know the reason." );
			strcpy( UserNoticeDescriptor.SuggestedActionText, pTechSupportMsg );
			break;
		case MAIN_ERROR_SCM_SET_STATUS:
			strcat( UserNoticeDescriptor.NoticeText, "Unable to set/change service status." );
			strcpy( UserNoticeDescriptor.SuggestedActionText, pTechSupportMsg );
			break;
		case MAIN_ERROR_SERVICE_ALREADY_RUNNING:
			strcat( UserNoticeDescriptor.NoticeText, "The service is already running." );
			strcpy( UserNoticeDescriptor.SuggestedActionText, "Request technical support." );
			break;
		}
	UserNoticeDescriptor.TextLinesRequired = 7;
	SubmitUserNotification( &UserNoticeDescriptor );
}


int main( int argc, char *argv[] )
{
	char		*pCmdLine = GetCommandLine();
	char		*pText;
	char		FileSpec[ MAX_CFG_STRING_LENGTH ];
	char		PrivateDictionaryFileSpec[ MAX_CFG_STRING_LENGTH ];
	char		Msg[ MAX_CFG_STRING_LENGTH ];
	
	// Keep track of memory leaks.
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	
	strcpy( TransferService.ExeFile, "" );
	strncat( TransferService.ExeFile, argv[ 0 ], FULL_FILE_SPEC_STRING_LENGTH - 1 );
	pText = strrchr( TransferService.ExeFile, '\\' );		// Point to the last backslash in the string.
	if ( pText != 0 )
		*( ++pText ) = '\0';		// Terminate the ProgramPath string following the trailing backslash.
	strcpy( TransferService.ProgramPath, "" );
	strncat( TransferService.ProgramPath, TransferService.ExeFile, FILE_PATH_STRING_LENGTH - 1 );
	strcpy( TransferService.ExeFile, "" );
	strncat( TransferService.ExeFile, argv[ 0 ], FULL_FILE_SPEC_STRING_LENGTH - 1 );
	InitializeTransferService( &TransferService );

	InitializeSoftwareModules();
	if ( hStatusSemaphore == 0 )
		{
		NotifyUserOfBRetrieverStartupError( MAIN_ERROR_SERVICE_ALREADY_RUNNING );
		return 0;
		}
	
	if ( bRunAsService )
		{
		if ( argc > 1 )
			_strlwr( argv[1] );		// Convert command line argument to lower case.
		if ( argc > 1 && strstr( pCmdLine, "-install" ) != 0 )
			{
			InstallService();
			}
		else if ( argc > 1 && strstr( pCmdLine, "-remove" ) != 0 )
			{
			RemoveService();
			}
		else if ( argc == 1 )
			{
			// Notify the SCM about the location of the ServiceTable.
			// This will initiate the sequence of events needed to start the service.
			TransferService.bPrintToConsole = FALSE;
			// Attempt to switch control to ServiceMain(), below.
			if ( !StartServiceCtrlDispatcher( ServiceTable ) )
				NotifyUserOfBRetrieverStartupError( MAIN_ERROR_SERVICE_DISPATCH );
			}
		else
			NotifyUserOfBRetrieverStartupError( MAIN_ERROR_CMD_LINE );
		}
	else			// if running a debugging session
		{
		TransferService.bPrintToConsole = TRUE;
		LogMessage( "\n\nBRetriever (version 1.2n) started for debugging.  ****************************************\n", MESSAGE_TYPE_SERVICE_CONTROL );
		// Do what it takes to perform the service initialization.
		InitializeOperationConfiguration();
		if ( !ReadConfigurationFile( TransferService.ConfigDirectory, "BRetriever.cfg" ) )
			{
			NotifyUserOfBRetrieverStartupError( MAIN_ERROR_CFG_FILE );
			LogMessage( "Aborting BRetriever Service:  Configuration file read failure.", MESSAGE_TYPE_SERVICE_CONTROL );
			return 0;
			}
		ReadConfigurationFile( TransferService.ServiceDirectory, "Shared.cfg" );
		sprintf( Msg, "    Using Network Address:  %s", ServiceConfiguration.NetworkAddress );
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );

		strcpy( ServiceConfiguration.ConfigDirectory, TransferService.ConfigDirectory );
		strcpy( FileSpec, TransferService.ConfigDirectory );
		if ( FileSpec[ strlen( FileSpec ) - 1 ] != '\\' )
			strcat( FileSpec, "\\" );
		strcpy( PrivateDictionaryFileSpec, FileSpec );
		strcat( FileSpec, "DicomDictionary.txt" );
		strcat( PrivateDictionaryFileSpec, "DicomDictionaryPrivate.txt" );
		if ( !ReadDictionaryFiles( FileSpec, PrivateDictionaryFileSpec ) )
			{
			NotifyUserOfBRetrieverStartupError( MAIN_ERROR_DICOM_DICTIONARY );
			return 0;
			}
		if ( !ReadAllAbstractConfigFiles() )
			{
			NotifyUserOfBRetrieverStartupError( MAIN_ERROR_ABSTRACT_CONFIGURATION );
			return 0;
			}
		LogMessage( "Controlling BRetriever operations.\n", MESSAGE_TYPE_SUPPLEMENTARY );
		ListImageFolderContents();

		ControlProductOperations();	// Start/stop scheduled BRetriever operations.

		Sleep( 3000000 );		// Sleep for 3000 seconds (50 min) while debugging.
		// The "debug" program terminates when execution reaches this point.
		TerminateAllOperations();
		LogMessage( "BRetriever debugging exited successfully.", MESSAGE_TYPE_SERVICE_CONTROL );
		CloseSoftwareModules();
		}
		
	return 0;
}


void WINAPI ServiceMain( DWORD argc, LPTSTR *argv )
{
	BOOL		bNoError = TRUE;
	DWORD		SystemErrorCode;
	char		FileSpec[ MAX_CFG_STRING_LENGTH ];
	char		PrivateDictionaryFileSpec[ MAX_CFG_STRING_LENGTH ];
	char		Msg[ MAX_CFG_STRING_LENGTH ];

	TransferService.hServiceHandle = RegisterServiceCtrlHandler( "BRetriever", ServiceControlHandler );
	if ( TransferService.hServiceHandle == 0 )
		{
		SystemErrorCode = GetLastError();
		switch( SystemErrorCode )
			{
			case ERROR_INVALID_NAME:
				NotifyUserOfBRetrieverStartupError( MAIN_ERROR_SERVICE_NAME );
				break;
			case ERROR_SERVICE_DOES_NOT_EXIST:
				NotifyUserOfBRetrieverStartupError( MAIN_ERROR_SERVICE_NOT_FOUND );
				break;
			default:
				NotifyUserOfBRetrieverStartupError( MAIN_ERROR_UNKNOWN );
				break;
			}
		}
	else
		{
		// Send initial service status report to the SCM.
		if ( !UpdateStatusForSCM( &InitialServiceStatus ) )
			{
			NotifyUserOfBRetrieverStartupError( MAIN_ERROR_SCM_SET_STATUS );
			return;
			}
		}
	LogMessage( "\n\nBRetriever (version 1.2n) started.  ****************************************\n", MESSAGE_TYPE_SERVICE_CONTROL );
	// Do what it takes to perform the service initialization.
	InitializeOperationConfiguration();
	if ( !ReadConfigurationFile( TransferService.ConfigDirectory, "BRetriever.cfg" ) )
		{
		NotifyUserOfBRetrieverStartupError( MAIN_ERROR_CFG_FILE );
		LogMessage( "Aborting BRetriever Service:  Configuration file read failure.", MESSAGE_TYPE_SERVICE_CONTROL );
		bNoError = FALSE;
		}
	ReadConfigurationFile( TransferService.ServiceDirectory, "Shared.cfg" );
	sprintf( Msg, "    Using Network Address:  %s", ServiceConfiguration.NetworkAddress );
	LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );

	strcpy( ServiceConfiguration.ConfigDirectory, TransferService.ConfigDirectory );
	strcpy( FileSpec, TransferService.ConfigDirectory );
	if ( FileSpec[ strlen( FileSpec ) - 1 ] != '\\' )
		strcat( FileSpec, "\\" );
	strcpy( PrivateDictionaryFileSpec, FileSpec );
	strcat( FileSpec, "DicomDictionary.txt" );
	strcat( PrivateDictionaryFileSpec, "DicomDictionaryPrivate.txt" );
	if ( bNoError && !ReadDictionaryFiles( FileSpec, PrivateDictionaryFileSpec ) )
		{
		LogMessage( "Aborting BRetriever Service:  Dicom dictionary file read failure.", MESSAGE_TYPE_SERVICE_CONTROL );
		NotifyUserOfBRetrieverStartupError( MAIN_ERROR_DICOM_DICTIONARY );
		bNoError = FALSE;
		}
	if ( !ReadAllAbstractConfigFiles() )
		{
		LogMessage( "Aborting BRetriever Service:  Abstract configuration files read failure.", MESSAGE_TYPE_SERVICE_CONTROL );
		NotifyUserOfBRetrieverStartupError( MAIN_ERROR_ABSTRACT_CONFIGURATION );
		LogMessage( "Continuing service without abstract files.", MESSAGE_TYPE_SERVICE_CONTROL );
		}
	if ( !bNoError )
		{
		StoppedServiceStatus.dwCurrentState = SERVICE_STOPPED;
		StoppedServiceStatus.dwCheckPoint = 0;
		StoppedServiceStatus.dwWaitHint = 30000;	// 30 seconds.
		UpdateStatusForSCM( &StoppedServiceStatus );
		return;
		}

	BOOL		bPaused = FALSE;

	// Notify SCM we're up and running.
	UpdateStatusForSCM( &NormalServiceStatus );

	// Enter main processing loop.
	DWORD		dwWait;
	
	LogMessage( "Controlling BRetriever operations.\n", MESSAGE_TYPE_SUPPLEMENTARY );
	ListImageFolderContents();
	while ( !bProgramTerminationRequested )
		{
		if ( !bPaused )
			{
			ControlProductOperations();	// Start/stop scheduled BRetriever operations.
			Sleep( 2000 );				// Sleep for 2 seconds.
			}
		// Wait for a signal to stop/pause/continue or a timeout.
		dwWait = WaitForMultipleObjects( 3, TransferService.hEvents, FALSE, 0 );
		if ( dwWait == WAIT_OBJECT_0 )			// If the stop event was signalled...
			{
			LogMessage( "BRetriever stop request received.", MESSAGE_TYPE_SERVICE_CONTROL );
			StoppedServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
			StoppedServiceStatus.dwCheckPoint = 1;
			StoppedServiceStatus.dwWaitHint = 30000;	// 30 seconds.
			UpdateStatusForSCM( &StoppedServiceStatus );
			TerminateAllOperations();
			break;										// Exit while loop.
			}
		else if ( dwWait == WAIT_OBJECT_0 + 1 )			// If the pause event was signalled...  (currently disabled)
			{
			LogMessage( "BRetriever pause request received.", MESSAGE_TYPE_SERVICE_CONTROL );
			bPaused = TRUE;
			MiscServiceStatus.dwCurrentState = SERVICE_PAUSE_PENDING;
			UpdateStatusForSCM( &MiscServiceStatus );
			TerminateAllOperations();		// Operations will automatically be restarted
												//  when ControlProductOperations() is called again
												//  after CONTINUE.
			MiscServiceStatus.dwCurrentState = SERVICE_PAUSED;
			UpdateStatusForSCM( &MiscServiceStatus );
			ResetEvent( TransferService.hEvents[ 1 ] );
			}
		else if ( dwWait == WAIT_OBJECT_0 + 2 )		// If the continue event was signalled...  (currently disabled)
			{
			LogMessage( "BRetriever continue request received.", MESSAGE_TYPE_SERVICE_CONTROL );
			bPaused = FALSE;
			UpdateStatusForSCM( &NormalServiceStatus );
			ResetEvent( TransferService.hEvents[ 2 ] );
			}
		}			// ... end while.
	LogMessage( "Terminating BRetriever service.", MESSAGE_TYPE_SERVICE_CONTROL );
	CloseSoftwareModules();

	StoppedServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
	StoppedServiceStatus.dwCheckPoint = 2;
	StoppedServiceStatus.dwWaitHint = 30000;	// 30 seconds.
	UpdateStatusForSCM( &StoppedServiceStatus );
	// Do what it takes to stop the service.
	TerminateTransferService( &TransferService );
	
	StoppedServiceStatus.dwCurrentState = SERVICE_STOPPED;
	StoppedServiceStatus.dwCheckPoint = 0;
	StoppedServiceStatus.dwWaitHint = 30000;	// 30 seconds.
	UpdateStatusForSCM( &StoppedServiceStatus );
	}


void InstallService()
{
	SC_HANDLE			hSCM = NULL;
	SC_HANDLE			hService = NULL;
	
	hSCM = OpenSCManager( NULL, NULL, SC_MANAGER_CREATE_SERVICE );
	if ( hSCM == 0 )
		{
		RespondToError( MODULE_MAIN, MAIN_ERROR_OPEN_SCM );
		return;
		}
	hService = CreateService(	hSCM,
								"BRetriever",
								"BRetriever Service",
								GENERIC_READ,
								SERVICE_WIN32_OWN_PROCESS,
								SERVICE_DEMAND_START,
								SERVICE_ERROR_NORMAL,
								TransferService.ExeFile,
								NULL,
								NULL,
								NULL,
								NULL,
								NULL );
	if ( hService == 0 )
		{
		RespondToError( MODULE_MAIN, MAIN_ERROR_INSTALL_SERVICE );
		CloseServiceHandle( hSCM );
		return;
		}
	else
		{
		LogMessage( "BRetriever Service Successfully Installed", MESSAGE_TYPE_SERVICE_CONTROL );
		CloseServiceHandle( hService );
		}
	CloseServiceHandle( hSCM );
	return;
}


void RemoveService()
{
	SC_HANDLE			hSCM = NULL;
	SC_HANDLE			hService = NULL;
	
	hSCM = OpenSCManager( NULL, NULL, SC_MANAGER_CONNECT );
	if ( hSCM == 0 )
		{
		RespondToError( MODULE_MAIN, MAIN_ERROR_OPEN_SCM );
		return;
		}
	hService = OpenService(	hSCM, "BRetriever", DELETE );
	if ( hService == 0 )
		{
		RespondToError( MODULE_MAIN, MAIN_ERROR_OPEN_SERVICE );
		CloseServiceHandle( hSCM );
		return;
		}
	else if ( DeleteService( hService ) )
		LogMessage( "BRetriever Service Successfully Removed", MESSAGE_TYPE_SERVICE_CONTROL );
	else
		RespondToError( MODULE_MAIN, MAIN_ERROR_DELETE_SERVICE );
	CloseServiceHandle( hService );
	CloseServiceHandle( hSCM );
	return;
}


//___________________________________________________________________________
//
// The module header for this module:
//


static MODULE_INFO		MainModuleInfo = { MODULE_MAIN, "Main Module", InitMainModule, CloseMainModule };


static ERROR_DICTIONARY_ENTRY	ErrorCodes[] =
			{
				{ MAIN_ERROR_UNKNOWN					, "An unknown error has occurred." },
				{ MAIN_ERROR_SERVICE_DISPATCH			, "The call to StartServiceCtrlDispatcher() failed." },
				{ MAIN_ERROR_SERVICE_NAME				, "The call to RegisterServiceCtrlHandler() failed due to an invalid service name." },
				{ MAIN_ERROR_SERVICE_NOT_FOUND			, "The call to RegisterServiceCtrlHandler() failed by referencing a nonexistent service." },
				{ MAIN_ERROR_SCM_SET_STATUS				, "An error occurred in the call to SetServiceStatus()." },
				{ MAIN_ERROR_CMD_LINE					, "Unrecognized program arguments were specified in the command line." },
				{ MAIN_ERROR_OPEN_SCM					, "An error occurred attempting to open the Service Control Manager (SCM)." },
				{ MAIN_ERROR_INSTALL_SERVICE			, "An error occurred attempting to install the service." },
				{ MAIN_ERROR_OPEN_SERVICE				, "An error occurred attempting to open the service." },
				{ MAIN_ERROR_DELETE_SERVICE				, "An error occurred attempting to remove the service." },
				{ MAIN_ERROR_CFG_FILE					, "An error occurred attempting to read the configuration file." },
				{ MAIN_ERROR_ENDPOINT_ASSIGNMENT		, "An error occurred attempting to configure the operation end points." },
				{ MAIN_ERROR_DICOM_DICTIONARY			, "An error occurred attempting to read the Dicom Dictionary file." },
				{ MAIN_ERROR_ABSTRACT_CONFIGURATION		, "An error occurred attempting to read the abstract configuration files." },
				{ MAIN_ERROR_SERVICE_ALREADY_RUNNING	, "Another instance of this service is already running." },
				{ 0										, NULL }
			};

static ERROR_DICTIONARY_MODULE		BRetrieverStatusErrorDictionary =
										{
										MODULE_MAIN,
										ErrorCodes,
										MAIN_ERROR_DICT_LENGTH,
										0
										};


// This function must be called before any other function in this module.
void InitMainModule()
{
	bProgramTerminationRequested = FALSE;
	LinkModuleToList( &MainModuleInfo );
	RegisterErrorDictionary( &BRetrieverStatusErrorDictionary );
}


void CloseMainModule()
{
	// Deallocate the operation structures.
	InitializeOperationConfiguration();
}

