// ServiceInterface.cpp : Implementation file for interfacing with the operating
//	system's Service Control Manager (SCM).
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
#include "ServiceController.h"
#include "Module.h"
#include "ReportStatus.h"
#include "ServiceInterface.h"


extern SERVICE_DESCRIPTOR			ServiceDescriptor;


//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		ServiceInterfaceModuleInfo = { MODULE_SERVICE_INTERFACE,
														"Service Interface Module", InitServiceInterfaceModule, CloseServiceInterfaceModule };

static ERROR_DICTIONARY_ENTRY	ServiceInterfaceErrorCodes[] =
			{
				{ SERVICE_INTERFACE_ERROR_OPEN_SCM					, "An error occurred attempting to open the Service Control Manager (SCM)." },
				{ SERVICE_INTERFACE_ERROR_INSTALL_SERVICE			, "An error occurred attempting to install the service." },
				{ SERVICE_INTERFACE_ERROR_OPEN_SERVICE				, "The service was not found." },
				{ SERVICE_INTERFACE_ERROR_START_SERVICE				, "An error occurred attempting to start the service." },
				{ SERVICE_INTERFACE_ERROR_STOP_SERVICE				, "An error occurred attempting to stop the service." },
				{ SERVICE_INTERFACE_ERROR_CHECK_SERVICE				, "An error occurred attempting to check the service status." },
				{ SERVICE_INTERFACE_ERROR_DELETE_SERVICE			, "An error occurred attempting to remove the service." },
				{ SERVICE_INTERFACE_ERROR_SECURITY					, "An error occurred attempting to interrogate the service's security settings." },
				{ 0													, NULL }
			};

static ERROR_DICTIONARY_MODULE		ServiceInterfaceErrorDictionary =
										{
										MODULE_SERVICE_INTERFACE,
										ServiceInterfaceErrorCodes,
										SERVICE_INTERFACE_ERROR_DICT_LENGTH,
										0
										};


// This function must be called before any other function in this module.
void InitServiceInterfaceModule()
{
	LinkModuleToList( &ServiceInterfaceModuleInfo );
	RegisterErrorDictionary( &ServiceInterfaceErrorDictionary );
}


// This function must be called to deallocate memory and close this module.
void CloseServiceInterfaceModule()
{
}


static void LogSystemError( unsigned int ERROR_CODE )
{
	DWORD				SystemErrorCode = 0;
	char				Msg[ 256 ];

	SystemErrorCode = GetLastSystemErrorMessage( Msg, 256 );
	if ( SystemErrorCode == 0 )
		LogMessage( "  (No system error detected.)", MESSAGE_TYPE_NORMAL_LOG );
	else
		LogMessage( Msg, MESSAGE_TYPE_NORMAL_LOG );
	RespondToError( MODULE_SERVICE_INTERFACE, ERROR_CODE );
}


BOOL CheckServiceSecurity( SC_HANDLE hService )
{
	SECURITY_INFORMATION		SecurityControlBitmask;
	SECURITY_DESCRIPTOR			*pSecurityDescriptor;
	DWORD						BufferSize;
	DWORD						BytesNeeded;
	BOOL						bOK;
	PSID						pOwnerSecurityIdentifier;
	BOOL						bOwnerDefaulted;
	PACL						pDacl;
	BOOL						bDACLIsPresent;
	BOOL						bDACLDefaulted;
	ACL_SIZE_INFORMATION		ACListSizeInfo;
	unsigned long				nACEntry;
	PVOID						pACEntry;
	ACE_HEADER					ACEntryHeader;
	ACCESS_MASK					AccessMask;
	DWORD						SIDStart;
	char						Msg[ 256 ];

	SecurityControlBitmask = DACL_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION;
	BufferSize = 256;
	pSecurityDescriptor = (SECURITY_DESCRIPTOR*)malloc( BufferSize );
	if ( pSecurityDescriptor != 0 )
		{
		bOK = QueryServiceObjectSecurity( hService, SecurityControlBitmask, pSecurityDescriptor, BufferSize, &BytesNeeded );
		if ( bOK )
			bOK = GetSecurityDescriptorOwner( pSecurityDescriptor, &pOwnerSecurityIdentifier, &bOwnerDefaulted );
		if ( bOK )
			bOK = GetSecurityDescriptorDacl( pSecurityDescriptor, &bDACLIsPresent, &pDacl, &bDACLDefaulted );
		if ( bOK && bDACLIsPresent )
			bOK = GetAclInformation( pDacl, &ACListSizeInfo, sizeof(ACL_SIZE_INFORMATION), AclSizeInformation );
		if ( bOK )
			for ( nACEntry = 0; nACEntry < ACListSizeInfo.AceCount && bOK; nACEntry++ )
				{
				bOK = GetAce( pDacl, nACEntry, &pACEntry );
				if ( bOK )
					{
					ACEntryHeader = ( (ACCESS_ALLOWED_ACE*)pACEntry ) -> Header;
					AccessMask = ( (ACCESS_ALLOWED_ACE*)pACEntry ) -> Mask;
					SIDStart = ( (ACCESS_ALLOWED_ACE*)pACEntry ) -> SidStart;
					sprintf( Msg, "   Access Control List (ACL) item #%d is type %X, flags: %X, size = %d,   Access = %X   for SID %X", nACEntry, ACEntryHeader.AceType, ACEntryHeader.AceFlags, ACEntryHeader.AceSize, AccessMask, SIDStart );
					LogMessage( Msg, MESSAGE_TYPE_NORMAL_LOG );
					}
				}

		free( pSecurityDescriptor );
		if ( !bOK )
			{
			LogSystemError( SERVICE_INTERFACE_ERROR_SECURITY );
			}
		}

	return bOK;
}


BOOL InstallTheService()
{
	BOOL				bNoError = TRUE;
	SC_HANDLE			hSCM = NULL;
	SC_HANDLE			hService = NULL;
	char				ServiceExeFileSpecification[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				Msg[ 256 ];
	
	sprintf( Msg, " Attempting to install service:  %s", ServiceDescriptor.ServiceExeFileSpecification );
	LogMessage( Msg, MESSAGE_TYPE_NORMAL_LOG );
	hSCM = OpenSCManager( NULL, NULL, SC_MANAGER_CREATE_SERVICE );
	if ( hSCM == 0 )
		{
		bNoError = FALSE;
		LogSystemError( SERVICE_INTERFACE_ERROR_OPEN_SCM );
		}
	if ( bNoError )
		{
		// Put the service path in quotes to eliminate a possible security problem (Nessus flags it as a problem).
		strcpy( ServiceExeFileSpecification, "\"" );
		strcat( ServiceExeFileSpecification, ServiceDescriptor.ServiceExeFileSpecification );
		strcat( ServiceExeFileSpecification, "\"" );
		// Install the service via the Microsoft Service Manager.
		hService = CreateService(	hSCM,
									ServiceDescriptor.ShortServiceName,
									ServiceDescriptor.DisplayedServiceName,
									GENERIC_READ,
									SERVICE_WIN32_OWN_PROCESS,
									SERVICE_AUTO_START,
									SERVICE_ERROR_NORMAL,
									ServiceExeFileSpecification,
									NULL,
									NULL,
									NULL,
									NULL,
									NULL );
		if ( hService == 0 )
			{
			bNoError = FALSE;
			LogSystemError( SERVICE_INTERFACE_ERROR_INSTALL_SERVICE );
			}
		}
	if ( bNoError )
		{
		strcpy( Msg, ServiceDescriptor.ShortServiceName );
		strcat( Msg, " Service Successfully Installed from folder " );
		strcat( Msg, ServiceDescriptor.ServicePathSpecification );
		LogMessage( Msg, MESSAGE_TYPE_NORMAL_LOG );
		CloseServiceHandle( hService );
		}
	if ( hSCM != 0 )
		CloseServiceHandle( hSCM );

	return bNoError;
}


BOOL StartTheService()
{
	BOOL				bNoError = TRUE;
	SC_HANDLE			hSCM = NULL;
	SC_HANDLE			hService = NULL;
	char				Msg[ 128 ];
	CWaitCursor			DisplaysHourglass;
	SERVICE_STATUS		ServiceStatus;
	DWORD				ElapsedMilliseconds;
	BOOL				bGoodServiceResponseReceived;
	
	sprintf( Msg, " Attempting to start service:  %s", ServiceDescriptor.ShortServiceName );
	LogMessage( Msg, MESSAGE_TYPE_NORMAL_LOG );
	hSCM = OpenSCManager( NULL, NULL, GENERIC_READ );
	if ( hSCM == 0 )
		{
		bNoError = FALSE;
		LogSystemError( SERVICE_INTERFACE_ERROR_OPEN_SCM );
		}
	if ( bNoError )
		{
		hService = OpenService(	hSCM, ServiceDescriptor.ShortServiceName, SERVICE_START | READ_CONTROL );
		if ( hService == 0 )
			{
			bNoError = FALSE;
			LogSystemError( SERVICE_INTERFACE_ERROR_OPEN_SERVICE );
			}
		}
	if ( bNoError )
		{
		if ( StartService( hService, 0, NULL ) )
			{
			CloseServiceHandle( hService );
			strcpy( Msg, ServiceDescriptor.ShortServiceName );
			strcat( Msg, " Service Start request was issued by the Service Controler" );
			LogMessage( Msg, MESSAGE_TYPE_NORMAL_LOG );
			Sleep( 1000 );

			hService = OpenService(	hSCM, ServiceDescriptor.ShortServiceName, SERVICE_INTERROGATE );
			if ( hService == 0 )
				{
				bNoError = FALSE;
				LogSystemError( SERVICE_INTERFACE_ERROR_OPEN_SERVICE );
				}
			else
				{
				ElapsedMilliseconds = 0;
				bGoodServiceResponseReceived = FALSE;
				while ( ElapsedMilliseconds < 10000 && !bGoodServiceResponseReceived )
					{
					Sleep( 500 );
					ElapsedMilliseconds += 500;
					ControlService( hService, SERVICE_CONTROL_INTERROGATE, &ServiceStatus );
					if ( ServiceStatus.dwCurrentState == SERVICE_RUNNING )
						bGoodServiceResponseReceived = TRUE;
					}
				if ( bGoodServiceResponseReceived )
					strcat( Msg, " Service Start completed" );
				else
					strcat( Msg, " Service Start still waiting for confirmation." );
				LogMessage( Msg, MESSAGE_TYPE_NORMAL_LOG );
				}
			}
		else
			{
			bNoError = FALSE;
			LogSystemError( SERVICE_INTERFACE_ERROR_START_SERVICE );
			}
		CloseServiceHandle( hService );
		}
	if ( hSCM != 0 )
		CloseServiceHandle( hSCM );

	return bNoError;
}


BOOL CheckTheService()
{
	BOOL						bNoError = TRUE;
	SC_HANDLE					hSCM = NULL;
	SC_HANDLE					hService = NULL;
	SERVICE_STATUS_PROCESS		ServiceStatus;
	DWORD						StatusBytesNeeded;
	DWORD						SystemErrorCode = 0;
	char						Msg[ 256 ];
	CWaitCursor					DisplaysHourglass;
	
	sprintf( Msg, " Attempting to check the %s service", ServiceDescriptor.ShortServiceName );
	LogMessage( Msg, MESSAGE_TYPE_NORMAL_LOG );
	hSCM = OpenSCManager( NULL, NULL, GENERIC_READ );
	if ( hSCM == 0 )
		{
		bNoError = FALSE;
		LogSystemError( SERVICE_INTERFACE_ERROR_OPEN_SCM );
		}
	if ( bNoError )
		{
		hService = OpenService(	hSCM, ServiceDescriptor.ShortServiceName, SERVICE_QUERY_STATUS | READ_CONTROL );
		if ( hService == 0 )
			{
			bNoError = FALSE;
			RespondToError( MODULE_SERVICE_INTERFACE, SERVICE_INTERFACE_ERROR_OPEN_SERVICE );
			}
		}
	if ( bNoError )
		{
		Sleep( 1000 );
		bNoError = QueryServiceStatusEx( hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ServiceStatus, sizeof(SERVICE_STATUS_PROCESS), &StatusBytesNeeded );
		if ( bNoError )
			{
			switch ( ServiceStatus.dwCurrentState )
				{
				case SERVICE_STOPPED:
					sprintf( Msg, "Status Check:  %s is stopped", ServiceDescriptor.ShortServiceName );
					break;
				case SERVICE_START_PENDING:
					sprintf( Msg, "Status Check:  %s has a start pending", ServiceDescriptor.ShortServiceName );
					break;
				case SERVICE_STOP_PENDING:
					sprintf( Msg, "Status Check:  %s has a stop pending", ServiceDescriptor.ShortServiceName );
					break;
				case SERVICE_RUNNING:
					sprintf( Msg, "Status Check:  %s is running", ServiceDescriptor.ShortServiceName );
					break;
				case SERVICE_CONTINUE_PENDING:
					sprintf( Msg, "Status Check:  %s has a continue pending", ServiceDescriptor.ShortServiceName );
					break;
				case SERVICE_PAUSE_PENDING:
					sprintf( Msg, "Status Check:  %s has a pause pending", ServiceDescriptor.ShortServiceName );
					break;
				case SERVICE_PAUSED:
					sprintf( Msg, "Status Check:  %s is paused", ServiceDescriptor.ShortServiceName );
					break;
				default:
					sprintf( Msg, "Status Check:  %s status is indeterminate", ServiceDescriptor.ShortServiceName );
					break;
				}
			LogMessage( Msg, MESSAGE_TYPE_NORMAL_LOG );
			}
		else
			{
			bNoError = FALSE;
			LogSystemError( SERVICE_INTERFACE_ERROR_CHECK_SERVICE );
			}
		}
	if ( hService != 0 )
		CloseServiceHandle( hService );
	if ( hSCM != 0 )
		CloseServiceHandle( hSCM );

	return bNoError;
}


BOOL StopTheService()
{
	BOOL				bNoError = TRUE;
	SC_HANDLE			hSCM = NULL;
	SC_HANDLE			hService = NULL;
	char				Msg[ 128 ];
	SERVICE_STATUS		ServiceStatus;
	CWaitCursor			DisplaysHourglass;
	DWORD				ElapsedMilliseconds;
	BOOL				bGoodServiceResponseReceived;
	
	sprintf( Msg, " Attempting to stop the %s service", ServiceDescriptor.ShortServiceName );
	LogMessage( Msg, MESSAGE_TYPE_NORMAL_LOG );
	hSCM = OpenSCManager( NULL, NULL, GENERIC_READ );
	if ( hSCM == 0 )
		{
		bNoError = FALSE;
		LogSystemError( SERVICE_INTERFACE_ERROR_OPEN_SCM );
		}
	if ( bNoError )
		{
		hService = OpenService(	hSCM, ServiceDescriptor.ShortServiceName, SERVICE_STOP );
		if ( hService == 0 )
			{
			bNoError = FALSE;
			LogSystemError( SERVICE_INTERFACE_ERROR_OPEN_SERVICE );
			}
		}
	if ( bNoError )
		{
		if ( ControlService( hService, SERVICE_CONTROL_STOP, &ServiceStatus ) )
			{
			CloseServiceHandle( hService );
			strcpy( Msg, ServiceDescriptor.ShortServiceName );
			strcat( Msg, " Service Stop has been initiated by the Service Control Manager" );
			LogMessage( Msg, MESSAGE_TYPE_NORMAL_LOG );

			hService = OpenService(	hSCM, ServiceDescriptor.ShortServiceName, SERVICE_INTERROGATE );
			if ( hService == 0 )
				{
				bNoError = FALSE;
				LogSystemError( SERVICE_INTERFACE_ERROR_OPEN_SERVICE );
				}
			else
				{
				ElapsedMilliseconds = 0;
				bGoodServiceResponseReceived = FALSE;
				while ( ElapsedMilliseconds < 10000 && !bGoodServiceResponseReceived )
					{
					Sleep( 500 );
					ElapsedMilliseconds += 500;
					ControlService( hService, SERVICE_CONTROL_INTERROGATE, &ServiceStatus );
					if ( ServiceStatus.dwCurrentState == SERVICE_STOPPED )
						bGoodServiceResponseReceived = TRUE;
					}
				strcpy( Msg, ServiceDescriptor.ShortServiceName );
				if ( bGoodServiceResponseReceived )
					strcat( Msg, " Service Stop completed" );
				else
					strcat( Msg, " Service Stop still waiting for confirmation." );
				LogMessage( Msg, MESSAGE_TYPE_NORMAL_LOG );
				}
			}
		else
			{
			bNoError = FALSE;
			LogSystemError( SERVICE_INTERFACE_ERROR_STOP_SERVICE );
			}
		CloseServiceHandle( hService );
		}
	if ( hSCM != 0 )
		CloseServiceHandle( hSCM );

	return bNoError;
}


BOOL RemoveTheService()
{
	BOOL				bNoError = TRUE;
	SC_HANDLE			hSCM = NULL;
	SC_HANDLE			hService = NULL;
	char				Msg[ 128 ];
	
	sprintf( Msg, " Attempting to uninstall the %s service", ServiceDescriptor.ShortServiceName );
	LogMessage( Msg, MESSAGE_TYPE_NORMAL_LOG );
	hSCM = OpenSCManager( NULL, NULL, SC_MANAGER_CONNECT );
	if ( hSCM == 0 )
		{
		bNoError = FALSE;
		LogSystemError( SERVICE_INTERFACE_ERROR_OPEN_SCM );
		}
	if ( bNoError )
		{
		hService = OpenService(	hSCM, ServiceDescriptor.ShortServiceName, DELETE );
		if ( hService == 0 )
			{
			bNoError = FALSE;
			LogSystemError( SERVICE_INTERFACE_ERROR_OPEN_SERVICE );
			}
		}
	if ( bNoError )
		{
		if ( DeleteService( hService ) )
			{
			strcpy( Msg, ServiceDescriptor.ShortServiceName );
			strcat( Msg, " service uninstall request was issued by the Service Controller" );
			LogMessage( Msg, MESSAGE_TYPE_NORMAL_LOG );
			}
		else
			{
			bNoError = FALSE;
			LogSystemError( SERVICE_INTERFACE_ERROR_DELETE_SERVICE );
			}
		CloseServiceHandle( hService );
		}
	if ( hSCM != 0 )
		CloseServiceHandle( hSCM );

	return bNoError;
}



