// DicomAcceptor.cpp - Implements data structures and functions for Dicom association
// activities initiated by remote C-Echo and C-Store requestor.
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
//	*[2] 03/11/2024 by Tom Atwood
//		Convert windows headers byte packing to the Win32 default for compatibility
//		with Visual Studio 2022.
//	*[1] 03/07/2024 by Tom Atwood
//		Fixed security issues.
//
//
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <process.h>
#pragma pack(push, 8)		// *[2] Pack structure members on 8-byte boundaries.
#include <winsock2.h>
#pragma pack(pop)
#include "Module.h"
#include "ReportStatus.h"
#include "ServiceMain.h"
#include "Dicom.h"
#include "Configuration.h"
#include "Operation.h"
#include "ProductDispatcher.h"
#include "WinSocketsAPI.h"
#include "DicomAssoc.h"
#include "DicomAcceptor.h"
#include "DicomCommand.h"
#include "DicomCommunication.h"


//___________________________________________________________________________
//
// The module header for this module:
//
extern CONFIGURATION				ServiceConfiguration;
extern PRODUCT_OPERATION			OperationReceive;
extern BOOL							bProgramTerminationRequested;
extern unsigned long				BRetrieverStatus;

static MODULE_INFO		DicomAcceptorModuleInfo = { MODULE_DICOMACCEPT, "Dicom Association Acceptance Module", InitDicomAcceptorModule, CloseDicomAcceptorModule };


static ERROR_DICTIONARY_ENTRY	DicomAcceptorErrorCodes[] =
			{
				{ DICOMACCEPT_ERROR_INSUFFICIENT_MEMORY				, "There is not enough memory to allocate a data structure." },
				{ DICOMACCEPT_ERROR_CREATE_ASSOCIATION				, "An error occurred creating an association structure." },
				{ DICOMACCEPT_ERROR_SOCKET_ACCEPT_FAILED			, "A network failure occurred accepting a socket connection." },
				{ DICOMACCEPT_ERROR_SET_SOCKET_TIMEOUT				, "An error occurred setting the send or receive timeout value for the connecting socket." },
				{ DICOMACCEPT_ERROR_PARSE_EXPECT_APPL_CONTEXT		, "During response parsing, an application context was expected but was not found." },
				{ DICOMACCEPT_ERROR_EVEN_PRES_CONTEXT_ID			, "An even numbered presentation context ID was specified for the association." },
				{ DICOMACCEPT_ERROR_PARSE_EXPECT_ABSTRACT_SYNTAX	, "During response parsing, an abstract syntax was expected but was not found." },
				{ DICOMACCEPT_ERROR_PARSE_EXPECT_TRANSFER_SYNTAX	, "During response parsing, a transfer syntax was expected but was not found." },
				{ DICOMACCEPT_ERROR_PARSE_EXPECT_PRES_CONTEXT		, "During response parsing, a presentation context was expected but was not found." },
				{ DICOMACCEPT_ERROR_PARSE_EXPECT_USER_INFO			, "During response parsing, user information was expected but was not found." },
				{ DICOMACCEPT_ERROR_PARSE_EXPECT_MAX_LENGTH			, "During response parsing, maximum length information was expected but was not found." },
				{ DICOMACCEPT_ERROR_PARSE_EXPECT_IMPL_CLASS_UID		, "During response parsing, implementation class UID was expected but was not found." },
				{ DICOMACCEPT_ERROR_PARSE_EXPECT_IMPL_VER_NAME		, "During response parsing, the implementation version name was expected but was not found." },
				{ DICOMACCEPT_ERROR_START_OP_THREAD					, "An error occurred starting a Dicom acceptor operation thread." },
				{ DICOMACCEPT_ERROR_LISTEN_SOCKET_SHUTDOWN			, "Failure to accept connection request.  Shutting down listening operation." },
				{ 0													, NULL }
			};


static ERROR_DICTIONARY_MODULE		DicomAcceptorStatusErrorDictionary =
										{
										MODULE_DICOMACCEPT,
										DicomAcceptorErrorCodes,
										DICOMACCEPT_ERROR_DICT_LENGTH,
										0
										};

static BOOL					bSocketsEnabled = FALSE;
static BOOL					bListeningEnabled = FALSE;
static BOOL					bListeningTerminated = FALSE;
static unsigned long		TotalThreadCount = 0L;

#pragma pack(push, 8)			// *[2] Pack structure members on 8-byte boundaries.
	static SOCKET				ListeningSocket;
#pragma pack(pop)



// This function must be called before any other function in this module.
void InitDicomAcceptorModule()
{
	LinkModuleToList( &DicomAcceptorModuleInfo );
	RegisterErrorDictionary( &DicomAcceptorStatusErrorDictionary );
	bListeningEnabled = FALSE;
	bListeningTerminated = FALSE;
	TotalThreadCount = 0L;
}


void CloseDicomAcceptorModule()
{
	BOOL					bNoError = TRUE;

	if ( bSocketsEnabled )
		{
		LogMessage( "Shut down sending from the listening socket.", MESSAGE_TYPE_SUPPLEMENTARY );
		bNoError = WindowsSocketShutdown( ListeningSocket, SD_SEND );
		LogMessage( "Close the listening socket.", MESSAGE_TYPE_SUPPLEMENTARY );
		bNoError = WindowsCloseSocket( ListeningSocket );
		ListeningSocket = INVALID_SOCKET;
		bListeningEnabled = FALSE;
		TerminateWindowsSockets();
		}
	bSocketsEnabled = FALSE;
}


unsigned __stdcall ListenForExamThreadFunction( void *pOperationStruct )
{
	BOOL					bNoError = TRUE;
	BOOL					bTerminateOperation = FALSE;
	PRODUCT_OPERATION		*pProductOperation;
	char					TextLine[ 1096 ];
	char					ThisLocalNetworkAddress[ MAX_CFG_STRING_LENGTH ];

	pProductOperation = (PRODUCT_OPERATION*)pOperationStruct;
	_snprintf_s( TextLine, 1096, _TRUNCATE, "    Operation Thread: %s", pProductOperation -> OperationName );						// *[1] Replaced sprintf() with _snprintf_s.
	LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
	strcpy( ThisLocalNetworkAddress, "" );
	strncat( ThisLocalNetworkAddress, pProductOperation -> pInputEndPoint -> NetworkAddress, MAX_CFG_STRING_LENGTH - 1 );
	while ( !bTerminateOperation && !bListeningTerminated )
		{
		_snprintf_s( TextLine, 1096, _TRUNCATE, "  Listen thread:  Begin listening at port number %s.", ThisLocalNetworkAddress );	// *[1] Replaced sprintf() with _snprintf_s.
		LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
		bNoError = ListenForDicomAssociationRequests( pProductOperation, ThisLocalNetworkAddress );
		LogMessage( "  Listen thread:  End listening.", MESSAGE_TYPE_SUPPLEMENTARY );
		_snprintf_s( TextLine, 1096, _TRUNCATE, "  Listen thread:  status = %X.", pProductOperation -> OpnState.StatusCode );		// *[1] Replaced sprintf() with _snprintf_s.
		LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
		
		LogMessage( "  Listen thread:  Entering sleep period.", MESSAGE_TYPE_SUPPLEMENTARY );
		EnterOperationCycleWaitInterval( pProductOperation, FALSE, &bTerminateOperation );
		if ( ( BRetrieverStatus & BRETRIEVER_STATUS_PROCESSING ) == 0 )
			UpdateBRetrieverStatus( BRETRIEVER_STATUS_ACTIVE );
		}			// ...end while not bTerminateOperation.

	LogMessage( "  Listen thread:  End thread.", MESSAGE_TYPE_SUPPLEMENTARY );
	Sleep( 2000 );		// Sleep while winding down.
	CloseOperation( pProductOperation );

	return 0;
}

BOOL ListenForDicomAssociationRequests( PRODUCT_OPERATION *pProductOperation, char *pThisLocalNetworkAddress )
{
	BOOL					bNoError = TRUE;

	bSocketsEnabled = InitWindowsSockets();
	// Enable a socket to listen for TCP connection requests.
	if ( !bListeningEnabled && !bListeningTerminated )
		{
		bNoError = InitializeSocketForListening( pThisLocalNetworkAddress );
		if ( bNoError )
			bListeningEnabled = TRUE;
		// Check for any recent connection requests.
		bNoError = RespondToConnectionRequests( pProductOperation );
		}

	return bNoError;
}


BOOL InitializeSocketForListening( char *pNetworkAddress )
{
	struct linger LingerRequirement;// The linger structure maintains information about a specific socket that specifies
									// how that socket should behave when data are queued to be sent and the closesocket
									// function is called on the socket.
	int						reuse = 1;
	BOOL					bNoError = TRUE;
	int						SocketNameLength;
#pragma pack(push, 8)		// *[2] Pack structure members on 8-byte boundaries.
	struct sockaddr_in		InternetAddr;	// 4-byte IP address.  In the Internet address family, the SOCKADDR_IN structure
											// is used by Windows Sockets to specify a local or remote endpoint address to
											// which to connect a socket.
											//
											//		struct sockaddr_in
											//			{
											//			short			sin_family;		// Address family (must be AF_INET).
											//			unsigned short	sin_port;		// IP port.
											//			struct in_addr	sin_addr;		// IP address.
											//			char			sin_zero[8];	// Padding to make structure the same
											//											//  size as SOCKADDR.
											//			};
											//
											// This is the form of the SOCKADDR structure specific to the Internet address family
											// and can be cast to SOCKADDR.  The IP address component of this structure is of type
											// IN_ADDR. The IN_ADDR structure is defined in Windows Sockets header file WINSOCK.H
											// as follows:
											//
											//		struct   in_addr
											//			{
											//			union
											//				{
											//				struct
											//					{
											//					unsigned char		s_b1,
											//										s_b2,
											//										s_b3,
											//										s_b4;
											//					}			s_un_b;
											//				struct
											//					{
											//					unsigned short		s_w1,
											//										s_w2;
											//					}			s_un_w;
											//				unsigned long	s_addr;
											//				} s_un;
											//			};
											//
#pragma pack(pop)
	char					TextString[ 256 ];
	char					*pStringPtr;
	int						nChars;
	unsigned short			nListenPort;
	
	memset( &LingerRequirement, 0, sizeof(LingerRequirement) );
	// Name socket using wildcards.
	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = INADDR_ANY;
	// The htons function takes a 16-bit number in host byte order and returns a 16-bit number in network byte
	// order used in TCP/IP networks:  Copy the port number.
	pStringPtr = strchr( pNetworkAddress, ':' );
	if ( pStringPtr != 0 )
		{
		nChars = (int)( pStringPtr - pNetworkAddress );
		strcpy( TextString, pStringPtr + 1 );
		nListenPort = (unsigned short)atoi( TextString );
		}
	else
		nListenPort = DEFAULT_DICOM_PORT_NUMBER;
	InternetAddr.sin_port = htons( nListenPort );

	LogMessage( "Creating listening socket.", MESSAGE_TYPE_SUPPLEMENTARY );
	ListeningSocket = CreateWindowsSocket();
	if ( ListeningSocket == INVALID_SOCKET )
		bNoError = FALSE;
	else
		{
		LogMessage( "Setting reuse listening socket option.", MESSAGE_TYPE_DETAILS );
		reuse = 1;		// Enable reusing the local socket address.
		bNoError = WindowsSetSocketOptions( ListeningSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse) );
		}
	if ( bNoError )
		{
		LogMessage( "Binding listening socket to IP address.", MESSAGE_TYPE_DETAILS );
		bNoError = WindowsSocketBind( ListeningSocket, (struct sockaddr*)&InternetAddr );
		}
	if ( bNoError )
		{
		LogMessage( "Retrieving socket address.", MESSAGE_TYPE_DETAILS );
		SocketNameLength = sizeof(InternetAddr);
		bNoError = WindowsGetSocketName( ListeningSocket, (struct sockaddr*)&InternetAddr );
		}
	if ( bNoError )
		{
		LogMessage( "Disable lingering after close.", MESSAGE_TYPE_DETAILS );
		LingerRequirement.l_onoff = 0;		// Disable lingering after a close request.
		bNoError = WindowsSetSocketOptions( ListeningSocket, SOL_SOCKET,
												SO_LINGER, (char*)&LingerRequirement, sizeof(LingerRequirement) );
		}
	if ( bNoError )
		{
		LogMessage( "Listen for socket connections.", MESSAGE_TYPE_SUPPLEMENTARY );
		bNoError = WindowsSocketListen( ListeningSocket, PRV_LISTENBACKLOG );
		}
	if ( !bNoError )
		bListeningEnabled = FALSE;

	return bNoError;
}


void TerminateListeningSocket()
{
	bListeningEnabled = FALSE;
	bListeningTerminated = TRUE;
	if ( ListeningSocket != INVALID_SOCKET )
		{
		CloseConnection( ListeningSocket );
		ListeningSocket = INVALID_SOCKET;
		}
	LogMessage( "Listening socket closed.", MESSAGE_TYPE_SUPPLEMENTARY );
}


BOOL RespondToConnectionRequests( PRODUCT_OPERATION *pListenOperation )
{
	BOOL						bNoError = TRUE;
	BOOL						bConnectionRequest = FALSE;
#pragma pack(push, 8)												// *[2] Pack structure members on 8-byte boundaries.
	struct linger				LingerRequirement;
	SOCKET						ConnectingSocket;
	struct sockaddr				ConnectingAddress;
#pragma pack(pop)
	char						TextLine[ MAX_LOGGING_STRING_LENGTH ];
	DWORD						SystemErrorCode;
	int							bReuseLocalSocketAddress;
	int							TimeoutInMilliseconds = 30000;		// Set the send and receive timeouts to 30 seconds.
    int							SocketBufferSize;
	int							bDisableNagleAlgorithm;
	DICOM_ASSOCIATION			*pAssociation;
	char						ClientIPAddress[ 20 ];
	struct hostent				*pRemoteHostEntity = NULL;
	BOOL						bTerminateOperation;
	PRODUCT_OPERATION			*pReceiveOperation;
	OPERATION_THREAD_FUNCTION	OpnThreadFunction;

	bTerminateOperation = FALSE;
	while ( !bTerminateOperation && !bListeningTerminated && bListeningEnabled && ListeningSocket != INVALID_SOCKET &&
				( pListenOperation -> OpnState.StatusCode & OPERATION_STATUS_TERMINATION_REQUESTED ) == 0 )
		{
		pReceiveOperation = 0;
		// Accept a connection from the listening socket.
		bNoError = WindowsSocketAccept( ListeningSocket, &ConnectingSocket, &ConnectingAddress );
		_snprintf_s( TextLine, MAX_LOGGING_STRING_LENGTH, _TRUNCATE, "   Accepting connection on socket ID %d", ConnectingSocket );	// *[1] Replaced sprintf() with _snprintf_s.
		LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
		bNoError = ( ConnectingSocket != INVALID_SOCKET );
		if ( !bNoError && ( pListenOperation -> OpnState.StatusCode & OPERATION_STATUS_TERMINATION_REQUESTED ) == 0  )
			{
			ConnectingSocket = 0;
			RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_SOCKET_ACCEPT_FAILED );
			SystemErrorCode = GetLastError();
			_snprintf_s( TextLine, MAX_LOGGING_STRING_LENGTH, _TRUNCATE, "    System error number = %d: ", SystemErrorCode );		// *[1] Replaced sprintf() with _snprintf_s.
			LogMessage( TextLine, MESSAGE_TYPE_ERROR );
			pListenOperation -> OpnState.StatusCode |= OPERATION_STATUS_TERMINATION_REQUESTED;
			RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_LISTEN_SOCKET_SHUTDOWN );
			}
		if ( bNoError )
			{
			// Create a string containing the client's numerical IP address.
			_snprintf_s( ClientIPAddress, 20, _TRUNCATE, "%-d.%-d.%-d.%-d",															// *[1] Replaced sprintf() with _snprintf_s.
						( (int) ConnectingAddress.sa_data[2] ) & 0xff,
						( (int) ConnectingAddress.sa_data[3] ) & 0xff,
						( (int) ConnectingAddress.sa_data[4] ) & 0xff,
						( (int) ConnectingAddress.sa_data[5] ) & 0xff );
			_snprintf_s( TextLine, MAX_LOGGING_STRING_LENGTH, _TRUNCATE,															// *[1] Replaced sprintf() with _snprintf_s.
							"  Connecting receiving socket to IP address %s.", ClientIPAddress );
			LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
			memset( &LingerRequirement, 0, sizeof(LingerRequirement) );
			LingerRequirement.l_onoff = 0;		// Disable lingering after a close request.
			bNoError = WindowsSetSocketOptions( ConnectingSocket, SOL_SOCKET, SO_LINGER, (char*)&LingerRequirement, sizeof(LingerRequirement) );
			}
		if ( bNoError )
			{
			// SO_REUSEADDR allows the socket to be bound to an address that is already in use. (Not applicable on ATM sockets.)
			// By default, a socket cannot be bound to a local address that is already in use. On occasion, however, it can be
			// necessary to reuse an address in this way. Since every connection is uniquely identified by the combination of local and
			// remote addresses, there is no problem with having two sockets bound to the same local address as long as the remote addresses
			// are different. To inform the Windows Sockets provider that a bind on a socket should not be disallowed because the desired
			// address is already in use by another socket, the application should set the SO_REUSEADDR socket option for the socket before
			// issuing the bind. The option is interpreted only at the time of the bind. It is therefore unnecessary and harmless to set
			// the option on a socket that is not to be bound to an existing address. Setting or resetting the option after the bind
			// has no effect on this or any other socket.
			bReuseLocalSocketAddress = 1;
			bNoError = WindowsSetSocketOptions( ConnectingSocket, SOL_SOCKET, SO_REUSEADDR,
												(char*)&bReuseLocalSocketAddress, sizeof(bReuseLocalSocketAddress) );
			}
		if ( bNoError )
			{
			// Use a 64K default socket buffer length.
			SocketBufferSize = 0x10000;
			bNoError = WindowsSetSocketOptions( ConnectingSocket, SOL_SOCKET, SO_SNDBUF, (char*)&SocketBufferSize, sizeof(SocketBufferSize) );
			if ( bNoError )
				bNoError = WindowsSetSocketOptions( ConnectingSocket, SOL_SOCKET, SO_RCVBUF, (char*)&SocketBufferSize, sizeof(SocketBufferSize) );
			}
		if ( bNoError )
			{
			// Set a 30-second timeout for the blocking send and receive operations.
			// You can set these options on any type of socket in any state. The default value for these options is zero,
			// which refers to an infinite time-out. Any other setting is the time-out, in milliseconds. It is valid to set the
			// time-out to any value, but values less than 500 milliseconds (half a second) are interpreted to be 500 milliseconds.
			bNoError = WindowsSetSocketOptions( ConnectingSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&TimeoutInMilliseconds, sizeof(TimeoutInMilliseconds) );
			if ( bNoError )
				bNoError = WindowsSetSocketOptions( ConnectingSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&TimeoutInMilliseconds, sizeof(TimeoutInMilliseconds) );
			if ( !bNoError )
				RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_SET_SOCKET_TIMEOUT );
			}
		if ( bNoError )
			{
			// Improve performance by disabling the Nagle algorithm.
			bDisableNagleAlgorithm = 1;			// Disable the Nagle algorithm.
			bNoError = WindowsSetSocketOptions( ConnectingSocket, IPPROTO_TCP, TCP_NODELAY,
												(char*)&bDisableNagleAlgorithm, sizeof(bDisableNagleAlgorithm) );
			}
		if ( bNoError )
			{
			// Launch the receiving operation on a separate thread.
			pReceiveOperation = CreateProductOperation();
			if ( pReceiveOperation == 0 )
				{
				bNoError = FALSE;
				RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_INSUFFICIENT_MEMORY );
				}
			else
				{
				// Clone the listening operation.
				memcpy( pReceiveOperation, &OperationReceive, sizeof( PRODUCT_OPERATION ) );
				pReceiveOperation -> OpnState.SocketDescriptor = ConnectingSocket;
				LogMessage( "New receive operation created.", MESSAGE_TYPE_SUPPLEMENTARY );
				}
			}
		if ( bNoError )
			{
			pReceiveOperation -> OpnState.StatusCode |= OPERATION_STATUS_CONNECTED;
			pAssociation = CreateAssociationStructure( pReceiveOperation );
			if ( pAssociation != 0 )
				{
				pReceiveOperation -> OpnState.pDicomAssociation = pAssociation;
				pAssociation -> DicomAssociationSocket = ConnectingSocket;
				strcpy( pAssociation -> RemoteIPAddress, "" );
				strncat( pAssociation -> RemoteIPAddress, ClientIPAddress, sizeof(pAssociation -> RemoteIPAddress) - 1 );
				// Attempt to use DNS to look up the client Internet URL.
				bNoError = GetWindowsHostByAddress( &ConnectingAddress.sa_data[2], 4, 2, &pRemoteHostEntity );
				if ( !bNoError )
					{
					bNoError = TRUE;
					pRemoteHostEntity = 0;
					}
				}
			else
				{
				bNoError = FALSE;
				RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_CREATE_ASSOCIATION );
				}
			}
		if ( bNoError )
			{
			if ( pRemoteHostEntity == 0 )
				{
				// Reverse DNS lookup disabled or host not found, so use the numerical address.
				strcpy( pAssociation -> RemoteNodeName, "" );
				strncat( pAssociation -> RemoteNodeName, ClientIPAddress, sizeof(pAssociation -> RemoteNodeName) - 1 );
				}
			else
				{
				strcpy( pAssociation -> RemoteNodeName, "" );
				strncat( pAssociation -> RemoteNodeName, pRemoteHostEntity -> h_name, sizeof(pAssociation -> RemoteNodeName) - 1 );
				_snprintf_s( TextLine, MAX_LOGGING_STRING_LENGTH, _TRUNCATE, "  Preparing to receive from %s.", pAssociation -> RemoteNodeName );	// *[1] Replaced sprintf() with _snprintf_s.
				LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
				}
			// Launch the acceptor association processing on a separate thread.
			pReceiveOperation -> bEnabled = TRUE;
			OpnThreadFunction = pReceiveOperation ->OpnState.ThreadFunction;
			if ( OpnThreadFunction != 0 )
				{
				pReceiveOperation -> OpnState.hOperationThreadHandle =
						(HANDLE)_beginthreadex(	NULL,							// No security issues for child processes.
												0,								// Use same stack size as parent process.
												OpnThreadFunction,				// Thread function.
												(void*)pReceiveOperation,		// Argument for thread function.
												0,								// Initialize thread state as running.
												&pReceiveOperation -> OpnState.OperationThreadID
												);
				if ( pReceiveOperation -> OpnState.hOperationThreadHandle == 0 )
					{
					RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_START_OP_THREAD );
					bNoError = FALSE;
					bProgramTerminationRequested = TRUE;
					}
				else
					{
					TotalThreadCount++;
					}
				}
			}
		if ( !bNoError )
			bListeningEnabled = FALSE;
		if ( pReceiveOperation != 0 )
			bTerminateOperation = CheckForOperationTerminationRequest( pReceiveOperation );
		else
			bTerminateOperation = TRUE;
		}			// ... end while not terminating operation.

	bListeningEnabled = FALSE;
	bNoError = CloseConnection( ListeningSocket );
	ListeningSocket = INVALID_SOCKET;
	LogMessage( "Closed listening connection (acceptance loop).", MESSAGE_TYPE_SUPPLEMENTARY );

	return bNoError;
}


// This function can be launched to handle each new association established for
// receiving image files.  It should not enter a sleep loop.  The thread should
// terminate when the association is terminated.
unsigned __stdcall ReceiveDicomThreadFunction( void *pOperationStruct )
{
	BOOL						bNoError = TRUE;
	DICOM_ASSOCIATION			*pAssociation;
	PRODUCT_OPERATION			*pReceiveOperation;
	SOCKET						SocketDescriptor;
	char						TextString[ MAX_LOGGING_STRING_LENGTH ];

 	pAssociation = 0;
	pReceiveOperation = (PRODUCT_OPERATION*)pOperationStruct;
	if ( pReceiveOperation != 0 )
		pAssociation = (DICOM_ASSOCIATION*)pReceiveOperation -> OpnState.pDicomAssociation;
	if ( pAssociation != 0 )
		{
		UpdateBRetrieverStatus( BRETRIEVER_STATUS_PROCESSING );
		pAssociation -> pProductOperation = pReceiveOperation;
		_snprintf_s( TextString, MAX_LOGGING_STRING_LENGTH, _TRUNCATE,									// *[1] Replaced sprintf() with _snprintf_s.
						"Launching association acceptor operation:  %s    Thread ID:  %X  Total threads so far:  %d.",
							pReceiveOperation -> OperationName, pReceiveOperation -> OpnState.OperationThreadID, TotalThreadCount );
		LogMessage( TextString, MESSAGE_TYPE_SUPPLEMENTARY );
		// Load the association parameters with information related to accepting an association request
		// by setting the initial state for the association state macchine.
		pAssociation -> EventIDReadyToBeProcessed = EVENT_RECEPTION_OF_TRANSPORT_CONNECTION_REQUEST;
		// The following function call will result in the processing of the association
		// through its final termination.
		bNoError = StartNewAssociation( pAssociation );	// Let the Dicom state machine take over processing on this thread.

		// The state machine was in control of sequencing the association activities
		// until this point is reached:
		pAssociation -> bAssociationClosed = TRUE;
		if ( bNoError && pReceiveOperation -> pDependentOperation != 0 )
			{
			// Enable any dependent operation to cycle.
			ReleaseSemaphore( pReceiveOperation -> pDependentOperation -> OpnState.hSleepSemaphore, 1L, NULL );
			}
		// Shut down the socket for this receiving operation, if the association didn't already shut it down.
		SocketDescriptor = pAssociation -> DicomAssociationSocket;
		if ( SocketDescriptor != INVALID_SOCKET )
			{
			CloseConnection( SocketDescriptor );
			LogMessage( "Closed receiving connection socket.", MESSAGE_TYPE_SUPPLEMENTARY );
			pReceiveOperation -> OpnState.SocketDescriptor = INVALID_SOCKET;
			pReceiveOperation -> OpnState.StatusCode &= ~OPERATION_STATUS_CONNECTED;
			}
		// Deallocate the Dicom association, which is no longer active.
		pReceiveOperation ->OpnState.pDicomAssociation = 0;
		pAssociation -> pProductOperation = 0;
		DeleteAssociationStructure( pAssociation );
		if ( ( BRetrieverStatus & BRETRIEVER_STATUS_PROCESSING ) == 0 )
			UpdateBRetrieverStatus( BRETRIEVER_STATUS_ACTIVE );
		}

	CloseOperation( pReceiveOperation );
	DeleteProductOperation( pReceiveOperation );
	pReceiveOperation = 0;

	return 0;
}


BOOL PrepareCEchoResponseBuffer( DICOM_ASSOCIATION *pAssociation )
{
	BOOL							bNoError = TRUE;
	DATA_TRANSFER_PDU_HEADER		MessagePacketHeader;
	PRESENTATION_DATA_VALUE_HEADER	CommandMessageHeader;
	char							*pBuffer;
	char							*pDataElementBuffer;
	unsigned long					DataElementBufferSize;
	unsigned long					TotalBufferSize;
	char							*pBufferInsertPoint;

	bNoError = PrepareCEchoCommandResponseBuffer( pAssociation, &pDataElementBuffer, &DataElementBufferSize );
	if ( bNoError )
		{
		TotalBufferSize = sizeof(DATA_TRANSFER_PDU_HEADER) + sizeof(PRESENTATION_DATA_VALUE_HEADER) + DataElementBufferSize;
		// Allocate and fill the message buffer.
		pBuffer = (char*)malloc( TotalBufferSize );
		if ( pBuffer != 0 )
			{
			pBufferInsertPoint = pBuffer;
			// Insert the message packet header.
			MessagePacketHeader.PDU_Type = 0x04;
			MessagePacketHeader.Reserved1 = 0x00;
			MessagePacketHeader.PDULength = TotalBufferSize - sizeof(DATA_TRANSFER_PDU_HEADER);
			AssociationSwapBytes( pAssociation, &MessagePacketHeader.PDULength, 4 );
			memcpy( pBufferInsertPoint, (char*)&MessagePacketHeader, sizeof(DATA_TRANSFER_PDU_HEADER) );
			pBufferInsertPoint += sizeof(DATA_TRANSFER_PDU_HEADER);
			// Insert the command message (PDV Item) header.
			CommandMessageHeader.PDVItemLength = TotalBufferSize - sizeof(DATA_TRANSFER_PDU_HEADER) - 4;
			AssociationSwapBytes( pAssociation, &CommandMessageHeader.PDVItemLength, 4 );
			CommandMessageHeader.PresentationContextID =
						pAssociation -> PresentationContextSelector.AcceptedPresentationContext;
			CommandMessageHeader.MessageControlHeader = CONTAINS_COMMAND_MESSAGE | LAST_MESSAGE_FRAGMENT;
			memcpy( pBufferInsertPoint, (char*)&CommandMessageHeader, sizeof(PRESENTATION_DATA_VALUE_HEADER) );
			pBufferInsertPoint += sizeof(PRESENTATION_DATA_VALUE_HEADER);
			// Append the message content buffer.
			memcpy( pBufferInsertPoint, pDataElementBuffer, DataElementBufferSize );
			free( pDataElementBuffer );

			pAssociation -> pSendBuffer = pBuffer;
			pAssociation -> SendBufferLength = TotalBufferSize;
			pAssociation -> SentCommandID = DICOM_CMD_ECHO_RESPONSE;
			}
		else
			{
			bNoError = FALSE;
			RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_INSUFFICIENT_MEMORY );
			}
		}

	return bNoError;
}


BOOL PrepareCEchoCommandResponseBuffer( DICOM_ASSOCIATION *pAssociation, char **ppBuffer, unsigned long *pBufferSize )
{
	BOOL							bNoError = TRUE;
	DATA_ELEMENT_GROUP_LENGTH		GroupLengthElement;
	DATA_ELEMENT_HEADER_IMPLICIT_VR	BufferElement;
	DATA_ELEMENT_COMMAND_FIELD		CommandFieldElement;
	DATA_ELEMENT_MESSAGE_ID			MessageIDElement;
	DATA_ELEMENT_DATASET_TYPE		DataSetTypeElement;
	DATA_ELEMENT_STATUS				StatusElement;
	char							*pBuffer;
	char							*pBufferInsertPoint;
	char							*pSOPClassUID;
	unsigned long					ValueLength;
	unsigned long					TotalElementSize;
	unsigned long					TotalBufferSize;
	BOOL							bUIDLengthIsOdd;

	// Determine the overall buffer length.  The only variable part is the affected SOP class UID element,
	// so start with it.
	pSOPClassUID = GetSOPClassUID( SOP_CLASS_VERIFICATION );	// Already padded with a space to make the character count even.
	ValueLength = (unsigned long)strlen( pSOPClassUID );
	bUIDLengthIsOdd = ( ( ValueLength & 0x00000001 ) != 0 );
	if ( bUIDLengthIsOdd )
		ValueLength++;				// Make the value length an even number of bytes.
	TotalElementSize = ValueLength + sizeof(DATA_ELEMENT_HEADER_IMPLICIT_VR);
	TotalBufferSize = sizeof(DATA_ELEMENT_GROUP_LENGTH) + TotalElementSize + sizeof(DATA_ELEMENT_COMMAND_FIELD) + 
				sizeof(DATA_ELEMENT_MESSAGE_ID) + sizeof(DATA_ELEMENT_DATASET_TYPE) + sizeof( DATA_ELEMENT_STATUS );
	// Allocate and fill the message buffer.
	pBuffer = (char*)malloc( TotalBufferSize );
	pBufferInsertPoint = pBuffer;
	if ( pBuffer != 0 )
		{
		pBufferInsertPoint = pBuffer;
		// Insert the Group Length dicom element into the buffer.
		GroupLengthElement.Group = 0x0000;
		GroupLengthElement.Element = 0x0000;
		GroupLengthElement.ValueLength = 4L;
		GroupLengthElement.Value = TotalBufferSize - sizeof(DATA_ELEMENT_GROUP_LENGTH);
		memcpy( pBufferInsertPoint, (char*)&GroupLengthElement, sizeof(DATA_ELEMENT_GROUP_LENGTH) );
		pBufferInsertPoint += sizeof(DATA_ELEMENT_GROUP_LENGTH);
		// Insert the affected SOP class dicom element into the buffer.
		BufferElement.Group = 0x0000;
		BufferElement.Element = 0x0002;
		BufferElement.ValueLength = ValueLength;
		memcpy( pBufferInsertPoint, (char*)&BufferElement, sizeof(DATA_ELEMENT_HEADER_IMPLICIT_VR) );
		pBufferInsertPoint += sizeof(DATA_ELEMENT_HEADER_IMPLICIT_VR);
		memcpy( pBufferInsertPoint, pSOPClassUID, ValueLength );
		if ( bUIDLengthIsOdd )
			*( pBufferInsertPoint + ValueLength - 1 ) = ' ';	// Pad value with a space.
		pBufferInsertPoint += ValueLength;
		// Insert the Command Field dicom element into the buffer.
		CommandFieldElement.Group = 0x0000;
		CommandFieldElement.Element = 0x0100;
		CommandFieldElement.ValueLength = 2L;
		CommandFieldElement.Value = DICOM_CMD_ECHO_RESPONSE;	// Designates the C-Echo request command.
		memcpy( pBufferInsertPoint, (char*)&CommandFieldElement, sizeof(DATA_ELEMENT_COMMAND_FIELD) );
		pBufferInsertPoint += sizeof(DATA_ELEMENT_COMMAND_FIELD);
		// Insert the Message ID dicom element into the buffer.
		MessageIDElement.Group = 0x0000;
		MessageIDElement.Element = 0x0120;
		MessageIDElement.ValueLength = 2L;
		memcpy( pBufferInsertPoint, (char*)&MessageIDElement, sizeof(DATA_ELEMENT_MESSAGE_ID) );
		pBufferInsertPoint += sizeof(DATA_ELEMENT_MESSAGE_ID);
		// Insert the Data Set Type dicom element into the buffer.
		DataSetTypeElement.Group = 0x0000;
		DataSetTypeElement.Element = 0x0800;
		DataSetTypeElement.ValueLength = 2L;
		DataSetTypeElement.Value = 0x0101;						// Indicate no data set present.
		memcpy( pBufferInsertPoint, (char*)&DataSetTypeElement, sizeof(DATA_ELEMENT_DATASET_TYPE) );
		pBufferInsertPoint += sizeof(DATA_ELEMENT_DATASET_TYPE);
		// Insert the Status dicom element into the buffer.
		StatusElement.Group = 0x0000;
		StatusElement.Element = 0x0900;
		StatusElement.ValueLength = 2L;
		StatusElement.Value = 0x0000;							// Indicate success.
		memcpy( pBufferInsertPoint, (char*)&StatusElement, sizeof(DATA_ELEMENT_STATUS) );
		pBufferInsertPoint += sizeof(DATA_ELEMENT_STATUS);
		}
	else
		{
		bNoError = FALSE;
		RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_INSUFFICIENT_MEMORY );
		}
	if ( bNoError )
		{
		*ppBuffer = pBuffer;
		*pBufferSize = TotalBufferSize;
		}
	else
		{
		*ppBuffer = 0;
		*pBufferSize = 0L;
		}

		
	return bNoError;
}


BOOL PrepareCStoreResponseBuffer( DICOM_ASSOCIATION *pAssociation, BOOL bNoError )
{
	DATA_TRANSFER_PDU_HEADER		MessagePacketHeader;
	PRESENTATION_DATA_VALUE_HEADER	CommandMessageHeader;
	char							*pBuffer;
	char							*pDataElementBuffer;
	unsigned long					DataElementBufferSize;
	unsigned long					TotalBufferSize;
	char							*pBufferInsertPoint;

	bNoError = PrepareCStoreCommandResponseBuffer( pAssociation, &pDataElementBuffer, &DataElementBufferSize, bNoError );
	if ( bNoError )
		{
		TotalBufferSize = sizeof(DATA_TRANSFER_PDU_HEADER) + sizeof(PRESENTATION_DATA_VALUE_HEADER) + DataElementBufferSize;
		// Allocate and fill the message buffer.
		pBuffer = (char*)malloc( TotalBufferSize );
		if ( pBuffer != 0 )
			{
			pBufferInsertPoint = pBuffer;
			// Insert the message packet header.
			MessagePacketHeader.PDU_Type = 0x04;
			MessagePacketHeader.Reserved1 = 0x00;
			MessagePacketHeader.PDULength = TotalBufferSize - sizeof(DATA_TRANSFER_PDU_HEADER);
			AssociationSwapBytes( pAssociation, &MessagePacketHeader.PDULength, 4 );
			memcpy( pBufferInsertPoint, (char*)&MessagePacketHeader, sizeof(DATA_TRANSFER_PDU_HEADER) );
			pBufferInsertPoint += sizeof(DATA_TRANSFER_PDU_HEADER);
			// Insert the command message (PDV Item) header.
			CommandMessageHeader.PDVItemLength = TotalBufferSize - sizeof(DATA_TRANSFER_PDU_HEADER) - 4;
			AssociationSwapBytes( pAssociation, &CommandMessageHeader.PDVItemLength, 4 );
			CommandMessageHeader.PresentationContextID =
						pAssociation -> PresentationContextSelector.AcceptedPresentationContext;
			CommandMessageHeader.MessageControlHeader = CONTAINS_COMMAND_MESSAGE | LAST_MESSAGE_FRAGMENT;
			memcpy( pBufferInsertPoint, (char*)&CommandMessageHeader, sizeof(PRESENTATION_DATA_VALUE_HEADER) );
			pBufferInsertPoint += sizeof(PRESENTATION_DATA_VALUE_HEADER);
			// Append the message content buffer.
			memcpy( pBufferInsertPoint, pDataElementBuffer, DataElementBufferSize );
			free( pDataElementBuffer );
			pAssociation -> pSendBuffer = pBuffer;
			pAssociation -> SendBufferLength = TotalBufferSize;
			pAssociation -> SentCommandID = DICOM_CMD_STORE_RESPONSE;
			pAssociation -> ReceivedCommandID = DICOM_CMD_UNSPECIFIED;
			}
		else
			{
			bNoError = FALSE;
			RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_INSUFFICIENT_MEMORY );
			}
		}

	return bNoError;
}


BOOL PrepareCStoreCommandResponseBuffer( DICOM_ASSOCIATION *pAssociation, char **ppBuffer, unsigned long *pBufferSize, BOOL bNoError )
{
	DATA_ELEMENT_GROUP_LENGTH		GroupLengthElement;
	DATA_ELEMENT_HEADER_IMPLICIT_VR	BufferElement;
	DATA_ELEMENT_COMMAND_FIELD		CommandFieldElement;
	DATA_ELEMENT_MESSAGE_ID			MessageIDElement;
	DATA_ELEMENT_DATASET_TYPE		DataSetTypeElement;
	DATA_ELEMENT_STATUS				StatusElement;
	char							*pBuffer;
	char							*pBufferInsertPoint;
	char							*pSOPClassUID;
	char							*pSOPInstanceUID;
	unsigned long					SOPClassValueLength;
	unsigned long					SOPInstanceValueLength;
	unsigned long					TotalSOPClassElementSize;
	unsigned long					TotalSOPInstanceElementSize;
	unsigned long					TotalBufferSize;
	BOOL							bSOPClassUIDLengthIsOdd;
	BOOL							bSOPInstanceUIDLengthIsOdd;

	// Determine the overall buffer length.  The only variable parts are the affected SOP class and instance UID elements,
	// so start with them.
	pSOPClassUID = GetSOPClassUID( SOP_CLASS_COMPUTED_RADIOGRAPHY_IMAGE_STORAGE );
	SOPClassValueLength = (unsigned long)strlen( pSOPClassUID );
	bSOPClassUIDLengthIsOdd = ( ( SOPClassValueLength & 0x00000001 ) != 0 );
	if ( bSOPClassUIDLengthIsOdd )
		SOPClassValueLength++;					// Make the value length an even number of bytes.
	TotalSOPClassElementSize = SOPClassValueLength + sizeof(DATA_ELEMENT_HEADER_IMPLICIT_VR);

	pSOPInstanceUID = pAssociation -> pCurrentAssociatedImageInfo -> CurrentDicomFileName;
	SOPInstanceValueLength = (unsigned long)strlen( pSOPInstanceUID );
	bSOPInstanceUIDLengthIsOdd = ( ( SOPInstanceValueLength & 0x00000001 ) != 0 );
	if ( bSOPInstanceUIDLengthIsOdd )
		SOPInstanceValueLength++;				// Make the value length an even number of bytes.
	TotalSOPInstanceElementSize = SOPInstanceValueLength + sizeof(DATA_ELEMENT_HEADER_IMPLICIT_VR);
	TotalBufferSize = sizeof(DATA_ELEMENT_GROUP_LENGTH) + TotalSOPClassElementSize + sizeof(DATA_ELEMENT_COMMAND_FIELD) + 
						sizeof(DATA_ELEMENT_MESSAGE_ID) + sizeof(DATA_ELEMENT_DATASET_TYPE) + sizeof( DATA_ELEMENT_STATUS ) +
							TotalSOPInstanceElementSize;
	// Allocate and fill the message buffer.
	pBuffer = (char*)malloc( TotalBufferSize );
	pBufferInsertPoint = pBuffer;
	if ( pBuffer != 0 )
		{
		pBufferInsertPoint = pBuffer;
		// Insert the Group Length dicom element into the buffer.
		GroupLengthElement.Group = 0x0000;
		GroupLengthElement.Element = 0x0000;
		GroupLengthElement.ValueLength = 4L;
		GroupLengthElement.Value = TotalBufferSize - sizeof(DATA_ELEMENT_GROUP_LENGTH);
		memcpy( pBufferInsertPoint, (char*)&GroupLengthElement, sizeof(DATA_ELEMENT_GROUP_LENGTH) );
		pBufferInsertPoint += sizeof(DATA_ELEMENT_GROUP_LENGTH);
		// Insert the affected SOP class dicom element into the buffer.
		BufferElement.Group = 0x0000;
		BufferElement.Element = 0x0002;
		BufferElement.ValueLength = SOPClassValueLength;
		memcpy( pBufferInsertPoint, (char*)&BufferElement, sizeof(DATA_ELEMENT_HEADER_IMPLICIT_VR) );
		pBufferInsertPoint += sizeof(DATA_ELEMENT_HEADER_IMPLICIT_VR);
		memcpy( pBufferInsertPoint, pSOPClassUID, SOPClassValueLength );
		if ( bSOPClassUIDLengthIsOdd )
			*( pBufferInsertPoint + SOPClassValueLength - 1 ) = ' ';	// Pad value with a space.
		pBufferInsertPoint += SOPClassValueLength;
		// Insert the Command Field dicom element into the buffer.
		CommandFieldElement.Group = 0x0000;
		CommandFieldElement.Element = 0x0100;
		CommandFieldElement.ValueLength = 2L;
		CommandFieldElement.Value = DICOM_CMD_STORE_RESPONSE;			// Designates the C-Store request command.
		memcpy( pBufferInsertPoint, (char*)&CommandFieldElement, sizeof(DATA_ELEMENT_COMMAND_FIELD) );
		pBufferInsertPoint += sizeof(DATA_ELEMENT_COMMAND_FIELD);
		// Insert the Message ID dicom element into the buffer.
		MessageIDElement.Group = 0x0000;
		MessageIDElement.Element = 0x0120;
		MessageIDElement.ValueLength = 2L;
		memcpy( pBufferInsertPoint, (char*)&MessageIDElement, sizeof(DATA_ELEMENT_MESSAGE_ID) );
		pBufferInsertPoint += sizeof(DATA_ELEMENT_MESSAGE_ID);
		// Insert the Data Set Type dicom element into the buffer.
		DataSetTypeElement.Group = 0x0000;
		DataSetTypeElement.Element = 0x0800;
		DataSetTypeElement.ValueLength = 2L;
		DataSetTypeElement.Value = 0x0101;								// Indicate no data set present.
		memcpy( pBufferInsertPoint, (char*)&DataSetTypeElement, sizeof(DATA_ELEMENT_DATASET_TYPE) );
		pBufferInsertPoint += sizeof(DATA_ELEMENT_DATASET_TYPE);
		// Insert the Status dicom element into the buffer.
		StatusElement.Group = 0x0000;
		StatusElement.Element = 0x0900;
		StatusElement.ValueLength = 2L;
		if ( bNoError )
			StatusElement.Value = 0x0000;								// Indicate success.
		else
			StatusElement.Value = 0xFE00;								// Indicate failure ==> cancel c-store.
		memcpy( pBufferInsertPoint, (char*)&StatusElement, sizeof(DATA_ELEMENT_STATUS) );
		pBufferInsertPoint += sizeof(DATA_ELEMENT_STATUS);
		// Insert the affected SOP instance (file name) dicom element into the buffer.
		BufferElement.Group = 0x0000;
		BufferElement.Element = 0x1000;
		BufferElement.ValueLength = SOPInstanceValueLength;
		memcpy( pBufferInsertPoint, (char*)&BufferElement, sizeof(DATA_ELEMENT_HEADER_IMPLICIT_VR) );
		pBufferInsertPoint += sizeof(DATA_ELEMENT_HEADER_IMPLICIT_VR);
		memcpy( pBufferInsertPoint, pSOPInstanceUID, SOPInstanceValueLength );
		if ( bSOPInstanceUIDLengthIsOdd )
			*( pBufferInsertPoint + SOPInstanceValueLength - 1 ) = ' ';	// Pad value with a space.
		pBufferInsertPoint += SOPInstanceValueLength;
		}
	else
		{
		bNoError = FALSE;
		RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_INSUFFICIENT_MEMORY );
		}
	if ( bNoError )
		{
		*ppBuffer = pBuffer;
		*pBufferSize = TotalBufferSize;
		}
	else
		{
		*ppBuffer = 0;
		*pBufferSize = 0L;
		}
		
	return bNoError;
}


BOOL PrepareAssociationAcceptanceBuffer( DICOM_ASSOCIATION *pAssociation )
{
	BOOL							bNoError = TRUE;
	LIST_ELEMENT					*pListElement;
	BUFFER_LIST_ELEMENT				*pBufferDescriptor;
	A_ASSOCIATE_AC_HEADER_BUFFER	*pBufferElement;
	PRESENTATION_CONTEXT_ITEM		*pPresentationContextItem;

	pBufferDescriptor = (BUFFER_LIST_ELEMENT*)malloc( sizeof(BUFFER_LIST_ELEMENT) );
	if ( pBufferDescriptor == 0 )
		{
		bNoError = FALSE;
		RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_INSUFFICIENT_MEMORY );
		}
	if ( bNoError )
		{
		pBufferElement = (A_ASSOCIATE_AC_HEADER_BUFFER*)malloc( sizeof(A_ASSOCIATE_AC_HEADER_BUFFER) );
		if ( pBufferElement == 0 )
			{
			bNoError = FALSE;
			RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_INSUFFICIENT_MEMORY );
			}
		else
			{
			memset( (char*)pBufferElement, '\0', sizeof(A_ASSOCIATE_AC_HEADER_BUFFER) );
			pBufferElement -> PDU_Type = 0x02;
			pBufferElement -> Reserved1 = 0x00;
			pBufferElement -> ProtocolVersion = 0x0001;
			AssociationSwapBytes( pAssociation, &pBufferElement -> ProtocolVersion, 2 );
			pBufferElement -> Reserved2 = 0x0000;
			}
		}
	if ( bNoError )
		{
		pBufferDescriptor -> BufferType = BUFTYPE_A_ASSOCIATE_AC_HEADER_BUFFER;
		pBufferDescriptor -> MaxBufferLength = sizeof(A_ASSOCIATE_AC_HEADER_BUFFER);
		pBufferDescriptor -> InsertedBufferLength = sizeof(A_ASSOCIATE_AC_HEADER_BUFFER);
		pBufferDescriptor -> bInsertedLengthIsFinalized = FALSE;
		pBufferDescriptor -> pBuffer = (void*)pBufferElement;
		// Set the preliminary buffer length, before variable-length items are added.
		// As each subitem is added, this length should be incremented.
		pBufferElement -> PDULength = sizeof(A_ASSOCIATE_AC_HEADER_BUFFER);
		// The AE_TITLEs should be padded with spaces, without a null terminator.
		memset( pBufferElement -> CallingAETitle, ' ', 16 );
		memcpy( pBufferElement -> CallingAETitle, pAssociation -> RemoteAE_Title, 16 );
		memset( pBufferElement -> CalledAETitle, ' ', 16 );
		memcpy( pBufferElement -> CalledAETitle, pAssociation -> LocalAE_Title, 16 );
		
		bNoError = PrefixToList( &pAssociation -> AssociationBufferList, (void*)pBufferDescriptor );
		}
	// Following this header, the buffer shall contain the following items:
	// one Application Context Item, one or more Presentation Context Items and one User Information Item.
	if ( bNoError )
		bNoError = PrepareApplicationContextBuffer( pAssociation );
	// Append the subitem buffer to this buffer.
	if ( bNoError )
		{
		bNoError = AppendSubitemBuffer( pAssociation, pBufferDescriptor );
		if ( bNoError )
			{
			pBufferElement = (A_ASSOCIATE_AC_HEADER_BUFFER*)pBufferDescriptor -> pBuffer;
			pBufferElement -> PDULength = pBufferDescriptor -> InsertedBufferLength;
			}
		}
	if ( bNoError )
		{
		// The key points in Presentation Context Negotiation are as follows:
		// a.	The Association-requester may offer multiple Presentation Contexts per Association.
		// b.	Each Presentation Context supports one Abstract Syntax (related to a SOP Class or Meta SOP Class) and one or more Transfer Syntaxes.
		// c.	The Association-acceptor may accept or reject each Presentation Context individually.
		// d.	The Association-acceptor selects a suitable Transfer Syntax for each Presentation Context accepted.
		pListElement = pAssociation -> ProposedPresentationContextList;
		while ( bNoError && pListElement != 0 )
			{
			pPresentationContextItem = (PRESENTATION_CONTEXT_ITEM*)pListElement -> pItem;
			bNoError = PreparePresentationContextReplyBuffer( pAssociation, pPresentationContextItem );
			// Append the subitem buffer to this buffer.
			bNoError = AppendSubitemBuffer( pAssociation, pBufferDescriptor );
			if ( bNoError )
				{
				pBufferElement = (A_ASSOCIATE_AC_HEADER_BUFFER*)pBufferDescriptor -> pBuffer;
				pBufferElement -> PDULength = pBufferDescriptor -> InsertedBufferLength;
				}
			pListElement = pListElement -> pNextListElement;
			}
		}
	if ( bNoError )
		bNoError = PrepareUserInformationBuffer( pAssociation );
	// Append the subitem buffer to this buffer.
	if ( bNoError )
		{
		bNoError = AppendSubitemBuffer( pAssociation, pBufferDescriptor );
		if ( bNoError )
			{
			pBufferElement = (A_ASSOCIATE_AC_HEADER_BUFFER*)pBufferDescriptor -> pBuffer;
			pBufferElement -> PDULength = pBufferDescriptor -> InsertedBufferLength;
			}
		}
	// The pAssociation -> AssociationBufferList points to the resulting consolidated buffer,
	// ready for the transport layer to send.
	pAssociation -> SendBufferLength = pBufferElement -> PDULength;
	pBufferElement -> PDULength -= 6;
	AssociationSwapBytes( pAssociation, &pBufferElement -> PDULength, 4 );
	pBufferDescriptor = (BUFFER_LIST_ELEMENT*)pAssociation -> AssociationBufferList -> pItem;;
	pAssociation -> pSendBuffer = (char*)pBufferDescriptor -> pBuffer;
	if ( pBufferDescriptor != 0 )
		free( pBufferDescriptor );

	return bNoError;
}


BOOL PreparePresentationContextReplyBuffer( DICOM_ASSOCIATION *pAssociation, PRESENTATION_CONTEXT_ITEM *pPresentationContextItem )
{
	BOOL										bNoError = TRUE;
	BUFFER_LIST_ELEMENT							*pBufferDescriptor;
	A_PRESENTATION_CONTEXT_REPLY_HEADER_BUFFER	*pBufferElement;
	char										TextMsg[ MAX_LOGGING_STRING_LENGTH ];

	pBufferDescriptor = (BUFFER_LIST_ELEMENT*)malloc( sizeof(BUFFER_LIST_ELEMENT) );
	if ( pBufferDescriptor == 0 )
		{
		bNoError = FALSE;
		RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_INSUFFICIENT_MEMORY );
		}
	if ( bNoError )
		{
		pBufferElement = (A_PRESENTATION_CONTEXT_REPLY_HEADER_BUFFER*)malloc( sizeof(A_PRESENTATION_CONTEXT_REPLY_HEADER_BUFFER) );
		if ( pBufferElement == 0 )
			{
			bNoError = FALSE;
			RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_INSUFFICIENT_MEMORY );
			}
		else
			{
			memset( (char*)pBufferElement, '\0', sizeof(A_PRESENTATION_CONTEXT_REPLY_HEADER_BUFFER) );
			pBufferElement -> PDU_Type = 0x21;
			pBufferElement -> Reserved1 = 0x00;
			pBufferElement -> PresentationContextID = pPresentationContextItem -> AcceptedPresentationContextID;
			pBufferElement -> Reserved2 = 0x00;
			pBufferElement -> Result = PRES_CONTEXT_RESULT_ACCEPTED;			// Acceptance.
			_snprintf_s( TextMsg, MAX_LOGGING_STRING_LENGTH, _TRUNCATE,									// *[1] Replaced sprintf() with _snprintf_s.
							"Accepted presentation context ID = %02X", pBufferElement -> PresentationContextID );
			pBufferElement -> Reserved3 = 0x00;
			LogMessage( TextMsg, MESSAGE_TYPE_SUPPLEMENTARY );
			}
		}
	if ( bNoError )
		{
		pBufferDescriptor -> BufferType = BUFTYPE_A_PRESENTATION_CONTEXT_REPLY_HEADER_BUFFER;
		pBufferDescriptor -> MaxBufferLength = sizeof(A_PRESENTATION_CONTEXT_REPLY_HEADER_BUFFER);
		pBufferDescriptor -> InsertedBufferLength = sizeof(A_PRESENTATION_CONTEXT_REPLY_HEADER_BUFFER);
		pBufferDescriptor -> bInsertedLengthIsFinalized = FALSE;
		pBufferDescriptor -> pBuffer = (void*)pBufferElement;
		// Set the preliminary buffer length, before variable-length items are added.
		// As each subitem is added, this length should be incremented.
		pBufferElement -> Length = sizeof(A_PRESENTATION_CONTEXT_REPLY_HEADER_BUFFER);

		bNoError = PrefixToList( &pAssociation -> AssociationBufferList, (void*)pBufferDescriptor );
		// Following this header, the buffer shall contain the following sub items: one accepted Transfer Syntax.
		}
	if ( bNoError )
		{
		// Prepare a transfer syntax buffer for each flag set in pAssociation -> ProposedTransferSyntaxes.
		bNoError = PrepareTransferSyntaxBuffer( pAssociation, pPresentationContextItem -> AcceptedTransferSyntaxIndex );
		_snprintf_s( TextMsg, MAX_LOGGING_STRING_LENGTH, _TRUNCATE,								// *[1] Replaced sprintf() with _snprintf_s.
						"      Transfer syntax = %s", GetTransferSyntaxUID( pPresentationContextItem -> AcceptedTransferSyntaxIndex ) );
		LogMessage( TextMsg, MESSAGE_TYPE_SUPPLEMENTARY );
		// Append the subitem buffer to this buffer.
		if ( bNoError )
			{
			bNoError = AppendSubitemBuffer( pAssociation, pBufferDescriptor );
			if ( bNoError )
				{
				pBufferElement = (A_PRESENTATION_CONTEXT_REPLY_HEADER_BUFFER*)pBufferDescriptor -> pBuffer;
				pBufferElement -> Length = (unsigned short)( pBufferDescriptor -> InsertedBufferLength - 4 );
				}
			}
		}
	if ( bNoError )
		{
		AssociationSwapBytes( pAssociation, &pBufferElement -> Length, 2 );
		pBufferDescriptor -> bInsertedLengthIsFinalized = TRUE;
		}

	return bNoError;
}


BOOL  PrepareAssociationRejectionBuffer( DICOM_ASSOCIATION *pAssociation )
{
	BOOL							bNoError = TRUE;
	A_ASSOCIATE_RJ_BUFFER			*pBuffer;

	if ( bNoError )
		{
		pBuffer = (A_ASSOCIATE_RJ_BUFFER*)malloc( sizeof(A_ASSOCIATE_RJ_BUFFER) );
		if ( pBuffer == 0 )
			{
			bNoError = FALSE;
			RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_INSUFFICIENT_MEMORY );
			}
		else
			{
			memset( (char*)pBuffer, '\0', sizeof(A_ASSOCIATE_RJ_BUFFER) );
			pBuffer -> PDU_Type = 0x03;
			pBuffer -> Reserved1 = 0x00;
			pBuffer -> PDU_Length = 0x00000004;
			AssociationSwapBytes( pAssociation, &pBuffer -> PDU_Length, 4 );
			pBuffer -> Reserved2 = 0x00;
			pBuffer -> Result = ASSOC_REJECTION_RESULT_PERMANENT;	// Handle transient rejections due to temporary resource
																	// limitations after exam queueing is in place.
			pBuffer -> Source = ASSOC_REJECTION_SOURCE_SERVICE_USER;
			pBuffer -> Reason = ASSOC_REJECTION_REASON1_NOT_GIVEN;
			}
		}
	if ( bNoError )
		{
		pAssociation -> pSendBuffer = (char*)pBuffer;
		pAssociation -> SendBufferLength = sizeof(A_ASSOCIATE_RJ_BUFFER);
		}

	return bNoError;
}


BOOL  PrepareReleaseRequestReplyBuffer( DICOM_ASSOCIATION *pAssociation )
{
	BOOL							bNoError = TRUE;
	A_RELEASE_RP_BUFFER				*pBuffer;

	LogMessage( "Preparing reply to release request.", MESSAGE_TYPE_SUPPLEMENTARY );
	pBuffer = (A_RELEASE_RP_BUFFER*)malloc( sizeof(A_RELEASE_RP_BUFFER) );
	if ( pBuffer == 0 )
		{
		bNoError = FALSE;
		RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_INSUFFICIENT_MEMORY );
		}
	else
		{
		memset( (char*)pBuffer, '\0', sizeof(A_RELEASE_RP_BUFFER) );
		pBuffer -> PDU_Type = 0x06;
		pBuffer -> Reserved1 = 0x00;
		pBuffer -> PDU_Length = 0x00000004;
		AssociationSwapBytes( pAssociation, &pBuffer -> PDU_Length, 4 );
		pBuffer -> Reserved2 = 0x00000000;
		}
	if ( bNoError )
		{
		pAssociation -> pSendBuffer = (char*)pBuffer;
		pAssociation -> SendBufferLength = sizeof(A_RELEASE_RP_BUFFER);
		}

	return bNoError;
}


BOOL PrepareAssociationProviderAbortBuffer( DICOM_ASSOCIATION *pAssociation )
{
	BOOL							bNoError = TRUE;
	A_ABORT_BUFFER					*pBuffer;

	if ( bNoError )
		{
		pBuffer = (A_ABORT_BUFFER*)malloc( sizeof(A_ABORT_BUFFER) );
		if ( pBuffer == 0 )
			{
			bNoError = FALSE;
			RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_INSUFFICIENT_MEMORY );
			}
		else
			{
			memset( (char*)pBuffer, '\0', sizeof(A_ABORT_BUFFER) );
			pBuffer -> PDU_Type = 0x07;
			pBuffer -> Reserved1 = 0x00;
			pBuffer -> PDU_Length = 0x00000004;
			AssociationSwapBytes( pAssociation, &pBuffer -> PDU_Length, 4 );
			pBuffer -> Reserved2 = 0x00;
			pBuffer -> Reserved3 = 0x00;
			pBuffer -> Source = ASSOC_ABORT_SOURCE_SERVICE_PROVIDER;
			pBuffer -> Reason = ASSOC_ABORT_REASON2_UNRECOGNIZED_PDU_PARM;
			}
		}
	if ( bNoError )
		{
		pAssociation -> pSendBuffer = (char*)pBuffer;
		pAssociation -> SendBufferLength = sizeof(A_ABORT_BUFFER);
		}

	return bNoError;
}


BOOL ParseAssociationRequestBuffer( DICOM_ASSOCIATION *pAssociation )
{
	BOOL										bNoError = TRUE;
	unsigned char								PDU_Type;
	A_ASSOCIATE_RQ_HEADER_BUFFER				*pRequestBufferHeader;
	A_APPLICATION_CONTEXT_BUFFER				*pApplicationContextBuffer;
	A_PRESENTATION_CONTEXT_HEADER_BUFFER		*pPresentationContextBuffer;
	A_ABSTRACT_SYNTAX_BUFFER					*pAbstractSyntaxBuffer;
	A_TRANSFER_SYNTAX_BUFFER					*pTransferSyntaxBuffer;
	A_USER_INFO_ITEM_HEADER_BUFFER				*pUserInfoBuffer;
	A_MAXIMUM_LENGTH_REPLY_BUFFER				*pMaximumLengthBuffer;
	A_IMPLEMENTATION_CLASS_UID_BUFFER			*pImplementationClassUIDBuffer;
	A_ROLE_SELECTION_BUFFER						*pRoleSelectionBuffer;
	A_IMPLEMENTATION_VERSION_NAME_BUFFER		*pImplementationVersionNameBuffer;
	long										RemainingBufferLength;
	unsigned long								SizeOfSubItem;
	char										*pBufferReadPoint;
	char										*pAbstractSyntaxUID;
	char										*pTransferSyntaxUID;
	unsigned short								nPresentationContextsFound;
	unsigned short								nTransferSyntaxesFound;
	char										TextMsg[ MAX_LOGGING_STRING_LENGTH ];
	char										AbstractSyntaxUID[ 256 ];
	char										TransferSyntaxUID[ 256 ];
	unsigned long								ValueLength;
	TRANSFER_SYNTAX								TransferSyntax;
	PRESENTATION_CONTEXT_ITEM					*pPresentationContextItem;
	BOOL										bAbstractSyntaxWasRecognized;
	BOOL										bTransferSyntaxWasAcceptable;
	unsigned char								PresentationContextID;
	long										nByte;
	char										TempText[ 64 ];

	pBufferReadPoint = pAssociation -> pReceivedBuffer;
	RemainingBufferLength = pAssociation -> ReceivedBufferLength;
	pRequestBufferHeader = (A_ASSOCIATE_RQ_HEADER_BUFFER*)pBufferReadPoint;
	pAssociation -> bAssociationAccepted = FALSE;	// To be set to TRUE on acceptance.
	AssociationSwapBytes( pAssociation, &pRequestBufferHeader -> PDULength, 4 );
	RemainingBufferLength -= sizeof(A_ASSOCIATE_RQ_HEADER_BUFFER);
	memcpy( pAssociation -> RemoteAE_Title, pRequestBufferHeader -> CallingAETitle, 16 );
	pAssociation -> RemoteAE_Title[ 16 ] = '\0';
	memcpy( pAssociation -> LocalAE_Title, pRequestBufferHeader -> CalledAETitle, 16 );
	pAssociation -> LocalAE_Title[ 16 ] = '\0';

	// Following this header, the buffer shall contain the following items:
	// one Application Context Item, one or more Presentation Context Items and one User Information Item.
	if ( RemainingBufferLength > 0 )
		{
		// Locate the application context subitem.
		pBufferReadPoint += sizeof(A_ASSOCIATE_RQ_HEADER_BUFFER);
		pApplicationContextBuffer = (A_APPLICATION_CONTEXT_BUFFER*)pBufferReadPoint;
		PDU_Type = pApplicationContextBuffer -> PDU_Type;
		AssociationSwapBytes( pAssociation, &pApplicationContextBuffer -> Length, 2 );
		SizeOfSubItem = (unsigned long)pApplicationContextBuffer -> Length + 4;
		if ( PDU_Type == 0x10 )
			{
			LogMessage( "    Parsing application context subitem.", MESSAGE_TYPE_SUPPLEMENTARY );
			RemainingBufferLength -= SizeOfSubItem;
			}
		else
			{
			bNoError = FALSE;
			RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_PARSE_EXPECT_APPL_CONTEXT );
			}
		}
	else
		LogMessage( "No presentation context was sent.", MESSAGE_TYPE_SUPPLEMENTARY );

	if ( bNoError && RemainingBufferLength > 0 )
		{
		pBufferReadPoint += SizeOfSubItem;
		nPresentationContextsFound = 0;
		// Loop through the proposed presentation contexts.
		do
			{
			PresentationContextID = 0;
			// Locate the first (next) presentation context subitem.
			pPresentationContextBuffer = (A_PRESENTATION_CONTEXT_HEADER_BUFFER*)pBufferReadPoint;
			PDU_Type = pPresentationContextBuffer -> PDU_Type;
			if ( PDU_Type == 0x20 )
				{
				AssociationSwapBytes( pAssociation, &pPresentationContextBuffer -> Length, 2 );
				SizeOfSubItem = (unsigned long)pPresentationContextBuffer -> Length + 4;
				pBufferReadPoint += sizeof(A_PRESENTATION_CONTEXT_HEADER_BUFFER);
				RemainingBufferLength -= sizeof(A_PRESENTATION_CONTEXT_HEADER_BUFFER);
				nPresentationContextsFound++;
				LogMessage( "    Parsing presentation context subitem.", MESSAGE_TYPE_SUPPLEMENTARY );
				if ( ( pPresentationContextBuffer -> PresentationContextID & 0x01 ) == 0 )
					{
					bNoError = FALSE;
					RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_EVEN_PRES_CONTEXT_ID );
					}
				else
					{
					PresentationContextID = pPresentationContextBuffer -> PresentationContextID;
					_snprintf_s( TextMsg, MAX_LOGGING_STRING_LENGTH, _TRUNCATE,									// *[1] Replaced sprintf() with _snprintf_s.
									"Proposed presentation context ID = %02X", pPresentationContextBuffer -> PresentationContextID );
					LogMessage( TextMsg, MESSAGE_TYPE_SUPPLEMENTARY );
					pPresentationContextItem = CreatePresentationContextItem();
					bNoError = ( pPresentationContextItem != 0 );
					if ( bNoError )
						{
						pPresentationContextItem -> AcceptedPresentationContextID = pPresentationContextBuffer -> PresentationContextID;
						bNoError = AppendToList( &pAssociation -> ProposedPresentationContextList, (void*)pPresentationContextItem );
						}
					}
				// Following this header, the buffer shall contain the following sub items: one Abstract Syntax and one or more Transfer Syntax(es).
				if ( bNoError )
					{
					pAbstractSyntaxBuffer = (A_ABSTRACT_SYNTAX_BUFFER*)pBufferReadPoint;
					if ( pAbstractSyntaxBuffer -> PDU_Type == 0x30 )
						{
						LogMessage( "    Parsing abstract syntax subitem.", MESSAGE_TYPE_SUPPLEMENTARY );
						pAbstractSyntaxUID = (char*)pAbstractSyntaxBuffer + 4;
						AssociationSwapBytes( pAssociation, &pAbstractSyntaxBuffer -> Length, 2 );
						pAssociation -> nAcceptedAbstractSyntax = GetAbstractSyntaxIndex( pAbstractSyntaxUID, pAbstractSyntaxBuffer -> Length );

						pPresentationContextItem -> AcceptedAbstractSyntaxIndex = GetAbstractSyntaxIndex( pAbstractSyntaxUID, pAbstractSyntaxBuffer -> Length );
						strcpy( AbstractSyntaxUID, "" );
						strncat( AbstractSyntaxUID, pAbstractSyntaxUID, pAbstractSyntaxBuffer -> Length );
						AbstractSyntaxUID[ pAbstractSyntaxBuffer -> Length ] = '\0';
						_snprintf_s( TextMsg, MAX_LOGGING_STRING_LENGTH, _TRUNCATE,									// *[1] Replaced sprintf() with _snprintf_s.
										"      Proposed abstract syntax = %s", AbstractSyntaxUID );
						LogMessage( TextMsg, MESSAGE_TYPE_SUPPLEMENTARY );

						bAbstractSyntaxWasRecognized = RegisterProposedAbstractSyntax( pAssociation, pAbstractSyntaxUID, pAbstractSyntaxBuffer -> Length );
						if ( !bAbstractSyntaxWasRecognized )
							pAssociation -> nAcceptedAbstractSyntax = 0;
						pBufferReadPoint += (unsigned long)pAbstractSyntaxBuffer -> Length + 4;
						RemainingBufferLength -= (unsigned long)pAbstractSyntaxBuffer -> Length + 4;
						}
					else
						{
						bNoError = FALSE;
						RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_PARSE_EXPECT_ABSTRACT_SYNTAX );
						}

					}
				if ( bNoError )
					{
					nTransferSyntaxesFound = 0;
					do
						{
						// Locate the proposed transfer syntax subitems.
						pTransferSyntaxBuffer = (A_TRANSFER_SYNTAX_BUFFER*)pBufferReadPoint;
						if ( pTransferSyntaxBuffer -> PDU_Type == 0x40 )
							{
							pTransferSyntaxUID = (char*)pTransferSyntaxBuffer + 4;
							AssociationSwapBytes( pAssociation, &pTransferSyntaxBuffer -> Length, 2 );
							nTransferSyntaxesFound++;
							LogMessage( "    Parsing transfer syntax subitem.", MESSAGE_TYPE_SUPPLEMENTARY );

							if ( pTransferSyntaxBuffer -> Length > 255 )
								pTransferSyntaxBuffer -> Length = 128;
							memcpy( TransferSyntaxUID, pTransferSyntaxUID, pTransferSyntaxBuffer -> Length );
							TransferSyntaxUID[ pTransferSyntaxBuffer -> Length ] = '\0';
							_snprintf_s( TextMsg, MAX_LOGGING_STRING_LENGTH, _TRUNCATE,								// *[1] Replaced sprintf() with _snprintf_s.
											"      Proposed transfer syntax = %s", TransferSyntaxUID );
							LogMessage( TextMsg, MESSAGE_TYPE_SUPPLEMENTARY );

							pPresentationContextItem -> AcceptedTransferSyntaxIndex = GetTransferSyntaxIndex( pTransferSyntaxUID,
																										pTransferSyntaxBuffer -> Length );
							pPresentationContextItem -> AcceptedTransferSyntaxes |=
													GetTransferSyntaxBitCode( pPresentationContextItem -> AcceptedTransferSyntaxIndex );

							bTransferSyntaxWasAcceptable = RegisterProposedTransferSyntax( pAssociation, TransferSyntaxUID, PresentationContextID );
							pBufferReadPoint += (unsigned long)pTransferSyntaxBuffer -> Length + 4;
							RemainingBufferLength -= (unsigned long)pTransferSyntaxBuffer -> Length + 4;
							}
						}
					while ( pTransferSyntaxBuffer -> PDU_Type == 0x40 );

					if ( nTransferSyntaxesFound == 0 )
						{
						bNoError = FALSE;
						RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_PARSE_EXPECT_TRANSFER_SYNTAX );
						}
					}
				}
			}
		while ( PDU_Type == 0x20 );		// ...end loop through presentation contexts.
		if ( nPresentationContextsFound == 0 )
			{
			bNoError = FALSE;
			RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_PARSE_EXPECT_PRES_CONTEXT );
			}
		}
	if ( bNoError && RemainingBufferLength > 0 )
		{
		// Locate the user information subitem.  It should be at the previously
		// read attempt for a presentation context buffer.
		pUserInfoBuffer = (A_USER_INFO_ITEM_HEADER_BUFFER*)pBufferReadPoint;
		PDU_Type = pUserInfoBuffer -> PDU_Type;
		AssociationSwapBytes( pAssociation, &pUserInfoBuffer -> Length, 2 );
		SizeOfSubItem = (unsigned long)pUserInfoBuffer -> Length + 4;
		pBufferReadPoint += sizeof(A_USER_INFO_ITEM_HEADER_BUFFER);
		if ( PDU_Type == 0x50 )
			{
			LogMessage( "    Parsing user information subitem.", MESSAGE_TYPE_SUPPLEMENTARY );
			RemainingBufferLength -= sizeof(A_USER_INFO_ITEM_HEADER_BUFFER);
			}
		else
			{
			bNoError = FALSE;
			RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_PARSE_EXPECT_USER_INFO );
			}

		if ( bNoError )
			{
			// Locate the maximum length subitem.
			pMaximumLengthBuffer = (A_MAXIMUM_LENGTH_REPLY_BUFFER*)pBufferReadPoint;
			PDU_Type = pMaximumLengthBuffer -> PDU_Type;
			AssociationSwapBytes( pAssociation, &pMaximumLengthBuffer -> Length, 2 );
			SizeOfSubItem = (unsigned long)pMaximumLengthBuffer -> Length + 4;
			pBufferReadPoint += SizeOfSubItem;
			if ( PDU_Type == 0x51 )
				{
				LogMessage( "    Parsing maximum length subitem.", MESSAGE_TYPE_SUPPLEMENTARY );
				RemainingBufferLength -= SizeOfSubItem;
				AssociationSwapBytes( pAssociation, &pMaximumLengthBuffer -> MaximumLengthReceivable, 4 );
				}
			else
				{
				bNoError = FALSE;
				RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_PARSE_EXPECT_MAX_LENGTH );
				}
			}
		if ( bNoError )
			{
			// Locate the Implementation Class UID subitem.
			pImplementationClassUIDBuffer = (A_IMPLEMENTATION_CLASS_UID_BUFFER*)pBufferReadPoint;
			PDU_Type = pImplementationClassUIDBuffer -> PDU_Type;
			AssociationSwapBytes( pAssociation, &pImplementationClassUIDBuffer -> Length, 2 );
			ValueLength = (unsigned long)pImplementationClassUIDBuffer -> Length;
			if ( PDU_Type == 0x52 )
				{
				LogMessage( "    Parsing implementation class UID subitem.", MESSAGE_TYPE_SUPPLEMENTARY );
				pAssociation -> pImplementationClassUID  = (char*)malloc( ValueLength + 1 );
				if ( pAssociation -> pImplementationClassUID != 0 )
					{
					memcpy( pAssociation -> pImplementationClassUID, pBufferReadPoint + 4, ValueLength );
					pAssociation -> pImplementationClassUID[ ValueLength ] = '\0';
					}
				else
					{
					bNoError = FALSE;
					RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_INSUFFICIENT_MEMORY );
					}
				}
			else
				{
				bNoError = FALSE;
				RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_PARSE_EXPECT_IMPL_CLASS_UID );
				}
			SizeOfSubItem = ValueLength + 4;
			pBufferReadPoint += SizeOfSubItem;
			RemainingBufferLength -= SizeOfSubItem;
			}
		if ( bNoError )
			{
			// Locate the (optional) SCU/SCP Role Selection subitem.
			pRoleSelectionBuffer = (A_ROLE_SELECTION_BUFFER*)pBufferReadPoint;
			PDU_Type = pRoleSelectionBuffer -> PDU_Type;
			if ( PDU_Type == 0x54 )
				{
				AssociationSwapBytes( pAssociation, &pRoleSelectionBuffer -> Length, 2 );
				ValueLength = (unsigned long)pRoleSelectionBuffer -> Length;
				LogMessage( "    Parsing SCU/SCP role selection subitem.", MESSAGE_TYPE_SUPPLEMENTARY );
				// Ignore this item, for the time being.  We probably know what the roles are.
				SizeOfSubItem = ValueLength + 4;
				pBufferReadPoint += SizeOfSubItem;
				RemainingBufferLength -= SizeOfSubItem;
				}
			}
		if ( bNoError )
			{
			// Locate the Implementation Version Name subitem.
			pImplementationVersionNameBuffer = (A_IMPLEMENTATION_VERSION_NAME_BUFFER*)pBufferReadPoint;
			PDU_Type = pImplementationVersionNameBuffer -> PDU_Type;
			AssociationSwapBytes( pAssociation, &pImplementationVersionNameBuffer -> Length, 2 );
			ValueLength = (unsigned long)pImplementationVersionNameBuffer -> Length;
			if ( PDU_Type == 0x55 )
				{
				LogMessage( "    Parsing implementation version name subitem.", MESSAGE_TYPE_SUPPLEMENTARY );
				pAssociation -> pImplementationVersionName  = (char*)malloc( ValueLength + 1 );
				SizeOfSubItem = ValueLength + 4;
				if ( pAssociation -> pImplementationVersionName != 0 )
					{
					memcpy( pAssociation -> pImplementationVersionName, pBufferReadPoint + 4, ValueLength );
					pAssociation -> pImplementationVersionName[ ValueLength ] = '\0';
					}
				else
					{
					bNoError = FALSE;
					RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_INSUFFICIENT_MEMORY );
					pAssociation -> pImplementationVersionName = 0;
					}
				}
			else
				{
				RespondToError( MODULE_DICOMACCEPT, DICOMACCEPT_ERROR_PARSE_EXPECT_IMPL_VER_NAME );
				pAssociation -> pImplementationVersionName = 0;
				_snprintf_s( TextMsg, MAX_LOGGING_STRING_LENGTH, _TRUNCATE, "    Instead, encountered PDU Type = %03X", PDU_Type );	// *[1] Replaced sprintf() with _snprintf_s.
				LogMessage( TextMsg, MESSAGE_TYPE_SUPPLEMENTARY );
				SizeOfSubItem = 0;
				}
			pBufferReadPoint += SizeOfSubItem;
			RemainingBufferLength -= SizeOfSubItem;
			}
		}
	if ( bNoError )
		{
		// Follow up on the decision to accept a particular presentation context and transfer syntax.
		TransferSyntax = GetTransferSyntaxForDicomElementParsing( pAssociation -> PresentationContextSelector.AcceptedTransferSyntax );
		pAssociation -> bCurrentSyntaxIsLittleEndian = ( TransferSyntax & LITTLE_ENDIAN );
		}
	if ( RemainingBufferLength != 0 )
		{
		_snprintf_s( TextMsg, MAX_LOGGING_STRING_LENGTH, _TRUNCATE,								// *[1] Replaced sprintf() with _snprintf_s.
						"%d bytes remained unread from the received association acceptance buffer.", RemainingBufferLength );
		LogMessage( TextMsg, MESSAGE_TYPE_SUPPLEMENTARY );
		strcpy( TextMsg, "" );
		for ( nByte = 0; nByte < RemainingBufferLength; nByte++ )
			{
			_snprintf_s( TempText, 64, _TRUNCATE, "%02X ", *pBufferReadPoint++ );				// *[1] Replaced sprintf() with _snprintf_s.
			strcat( TextMsg, TempText );
			if ( strlen( TextMsg ) > 128 )
				{
				LogMessage( TextMsg, MESSAGE_TYPE_SUPPLEMENTARY );
				strcpy( TextMsg, "" );
				}
			}
		if ( strlen( TextMsg ) > 0 )
			LogMessage( TextMsg, MESSAGE_TYPE_SUPPLEMENTARY );
		}
	pAssociation -> EventIDReadyToBeProcessed = EVENT_RECEPTION_OF_ASSOCIATION_REQUEST;
	if ( pAssociation -> pReceivedBuffer != 0 )
		{
		free( pAssociation -> pReceivedBuffer );
		pAssociation -> pReceivedBuffer = 0;
		pAssociation -> ReceivedBufferLength = 0L;
		}

	return bNoError;
}


BOOL ParseAssociationReleaseRequestBuffer( DICOM_ASSOCIATION *pAssociation )
{
	BOOL										bNoError = TRUE;
	A_RELEASE_RQ_BUFFER							*pRequestBuffer;
	long										RemainingBufferLength;
	char										TextMsg[ MAX_LOGGING_STRING_LENGTH ];

	RemainingBufferLength = pAssociation -> ReceivedBufferLength;
	pRequestBuffer = (A_RELEASE_RQ_BUFFER*)pAssociation -> pReceivedBuffer;
	pAssociation -> ReceivedCommandID = DICOM_CMD_RELEASE_ASSOCIATION;

	if ( pAssociation -> ReceivedBufferLength != sizeof(A_RELEASE_RQ_BUFFER) )
		{
		_snprintf_s( TextMsg, MAX_LOGGING_STRING_LENGTH, _TRUNCATE,									// *[1] Replaced sprintf() with _snprintf_s.
						"%d bytes remained unread from the received association release request buffer.",
										pAssociation -> ReceivedBufferLength - sizeof(A_RELEASE_RQ_BUFFER) );
		LogMessage( TextMsg, MESSAGE_TYPE_SUPPLEMENTARY );
		}
	if ( pAssociation -> pReceivedBuffer != 0 )
		{
		free( pAssociation -> pReceivedBuffer );
		pAssociation -> pReceivedBuffer = 0;
		pAssociation -> ReceivedBufferLength = 0L;
		}

	return bNoError;
}




