// WinSocketsAPI.h - Defines the functions and data structures that support the 
// sockets communication interface used by BRetriever.
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
#pragma once

#define WINSOCKAPI_ERROR_NETWORK_NOT_READY				1
#define WINSOCKAPI_ERROR_SOCKETS_VERSION_MISMATCH		2
#define WINSOCKAPI_ERROR_SOCKETS_CURRENTLY_BLOCKED		3
#define WINSOCKAPI_ERROR_SOCKETS_TASK_COUNT				4
#define WINSOCKAPI_ERROR_BAD_SOCKETS_STARTUP_DATA		5
#define WINSOCKAPI_ERROR_MISSING_WINSOCK_DLL			6
#define WINSOCKAPI_ERROR_UNKNOWN						7
#define WINSOCKAPI_ERROR_NO_WSASTARTUP					8
#define WINSOCKAPI_ERROR_NETWORK_FAILED					9
#define WINSOCKAPI_ERROR_UNSUPPORTED_ADDRESS_FAMILY		10
#define WINSOCKAPI_ERROR_CALLBACK_IN_PROGRESS			11
#define WINSOCKAPI_ERROR_MAX_SOCKET_DESCRIPTORS			12
#define WINSOCKAPI_ERROR_CANNOT_CREATE_SOCKET			13
#define WINSOCKAPI_ERROR_UNSUPPORTED_PROTOCOL			14
#define WINSOCKAPI_ERROR_MISMATCHED_PROTOCOL			15
#define WINSOCKAPI_ERROR_INVALID_SOCKET_TYPE			16
#define WINSOCKAPI_ERROR_INVALID_SOCKETS_OPTION			17
#define WINSOCKAPI_ERROR_INVALID_SOCKETS_LEVEL			18
#define WINSOCKAPI_ERROR_KEEPALIVE_TIMEOUT				19
#define WINSOCKAPI_ERROR_UNSUPPORTED_OPTION				20
#define WINSOCKAPI_ERROR_KEEPALIVE_FAILURE				21
#define WINSOCKAPI_ERROR_INVALID_SOCKET_DESCRIPTOR		22
#define WINSOCKAPI_ERROR_BROADCAST_DISABLED				23
#define WINSOCKAPI_ERROR_SOCKET_IN_USE					24
#define WINSOCKAPI_ERROR_INVALID_IPADDRESS				25
#define WINSOCKAPI_ERROR_INVALID_SOCKETS_NAME_FIELD		26
#define WINSOCKAPI_ERROR_SOCKET_ALREADY_BOUND			27
#define WINSOCKAPI_ERROR_INSUF_BIND_BUFFERS				28
#define WINSOCKAPI_ERROR_SOCKET_NOT_BOUND				29
#define WINSOCKAPI_ERROR_ALREADY_CONNECTED				30
#define WINSOCKAPI_ERROR_NO_LISTEN_BUFFERS				31
#define WINSOCKAPI_ERROR_SOCKET_TYPE_FOR_LISTEN			32
#define WINSOCKAPI_ERROR_INCONSISTENT_SOCKET_TYPE		33
#define WINSOCKAPI_ERROR_SOCKET_NOT_CONNECTED			34
#define WINSOCKAPI_ERROR_SOCKET_ALREADY_CANCELLED		35
#define WINSOCKAPI_ERROR_LINGER_TIMEOUT_VALUE			36
#define WINSOCKAPI_ERROR_AUTH_HOST_NOT_FOUND			37
#define WINSOCKAPI_ERROR_NONAUTH_HOST_NOT_FOUND			38
#define WINSOCKAPI_ERROR_NONRECOVERABLE_ERROR			39
#define WINSOCKAPI_ERROR_VALID_HOST_NAME_NO_DATA		40
#define WINSOCKAPI_ERROR_SOCKETS_RESOURCE_ALLOCATION	41
#define WINSOCKAPI_ERROR_INVALID_TIMEOUT_OR_PARAMS		42
#define WINSOCKAPI_ERROR_SOCKET_OP_NOT_SUPPORTED		43
#define WINSOCKAPI_ERROR_INVALID_PARAMETER				44
#define WINSOCKAPI_ERROR_RECEIVE_AFTER_SHUTDOWN			45
#define WINSOCKAPI_ERROR_OPERATION_WOULD_BLOCK			46
#define WINSOCKAPI_ERROR_MESSAGE_TRUNCATED				47
#define WINSOCKAPI_ERROR_CONNECTION_ABORTED				48
#define WINSOCKAPI_ERROR_CONNECTION_TIMEOUT				49
#define WINSOCKAPI_ERROR_CONNECTION_RESET				50
#define WINSOCKAPI_ERROR_SEND_AFTER_SHUTDOWN			51
#define WINSOCKAPI_ERROR_BROADCAST_OPTION_NOT_SET		52
#define WINSOCKAPI_ERROR_NO_SEND_BUFFERS				53
#define WINSOCKAPI_ERROR_REMOTE_HOST_UNREACHABLE		54
#define WINSOCKAPI_ERROR_MUST_LISTEN_BEFORE_ACCEPT		55
#define WINSOCKAPI_ERROR_RECEIVE_TIMEOUT				56

#define WINSOCKAPI_ERROR_DICT_LENGTH					56




// Function prototypes.
//
void		InitWinSockAPIModule();
BOOL		InitWindowsSockets();
void		TerminateWindowsSockets();
SOCKET		CreateWindowsSocket();
BOOL		WindowsSelect( fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timeval *timeout, int *pReadySocketCount );
BOOL		WindowsSetSocketOptions( SOCKET SocketDescriptor, int ProtocolLevel, int OptionName, const char *OptionValue, int nValueBytes );
BOOL		WindowsSocketBind( SOCKET SocketDescriptor, struct sockaddr *pInternetAddr );
BOOL		WindowsGetSocketName( SOCKET SocketDescriptor, struct sockaddr *pInternetAddr );
BOOL		WindowsSocketListen( SOCKET SocketDescriptor, int nMaxPendingConnectionBacklog );
BOOL		WindowsSocketAccept( SOCKET ListeningSocket, SOCKET *pConnectingSocket, struct sockaddr *pInternetAddr );
BOOL		GetWindowsHostByAddress( char *pAddressBuffer, int AddressLength, int AddressType, hostent **ppHostInformation );
BOOL		WindowsSocketReceive( SOCKET SocketDescriptor, char *pDataBuffer, int BufferLength, int Flags, int *pnBytesRead, BOOL bErrorOnDisconnect );
BOOL		WindowsSocketSend( SOCKET SocketDescriptor, char *pDataBuffer, int BufferLength, int Flags, int *pnBytesSent );
BOOL		WindowsSocketShutdown( SOCKET SocketDescriptor, int TypeOfShutdown );
BOOL		WindowsCloseSocket( SOCKET SocketDescriptor );
BOOL		CloseConnection( SOCKET SocketDescriptor );

