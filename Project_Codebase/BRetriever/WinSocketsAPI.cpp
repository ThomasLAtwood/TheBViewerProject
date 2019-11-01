// WinSocketsAPI.cpp - Implements the functions and data structures that support the 
// sockets communication interface used by BRetriever.  Much of the documentation
// of these functions was copied from the Microsoft Windows function documentation.
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

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif



#pragma pack(push, 16)		// Pack structure members on 16-byte boundaries to overcome 64-bit Microsoft errors.
#include <winsock2.h>
#pragma pack(pop)

#include "Module.h"
#include "ReportStatus.h"
#include "WinSocketsAPI.h"


static MODULE_INFO		WinSockModuleInfo = { MODULE_WINSOCKAPI, "Socket Module", InitWinSockAPIModule, 0 };


static ERROR_DICTIONARY_ENTRY	WinSockAPIErrorCodes[] =
			{
				{ WINSOCKAPI_ERROR_NETWORK_NOT_READY			, "The network subsystem is not ready for communication." },
				{ WINSOCKAPI_ERROR_SOCKETS_VERSION_MISMATCH		, "The requested Windows Sockets version is not available on this system." },
				{ WINSOCKAPI_ERROR_SOCKETS_CURRENTLY_BLOCKED	, "A blocking Windows Sockets 1.1 operation is currently in progress." },
				{ WINSOCKAPI_ERROR_SOCKETS_TASK_COUNT			, "The Windows Sockets task limit for this system has been reached." },
				{ WINSOCKAPI_ERROR_BAD_SOCKETS_STARTUP_DATA		, "The WinsockData returned by WSAStartup is not a valid pointer." },
				{ WINSOCKAPI_ERROR_MISSING_WINSOCK_DLL			, "A usable WinSock.DLL file could not be located." },
				{ WINSOCKAPI_ERROR_UNKNOWN						, "An unknown error has occurred." },
				{ WINSOCKAPI_ERROR_NO_WSASTARTUP				, "A successful WSAStartup call must occur before opening a socket." },
				{ WINSOCKAPI_ERROR_NETWORK_FAILED				, "The network subsystem or the associated service provider has failed." },
				{ WINSOCKAPI_ERROR_UNSUPPORTED_ADDRESS_FAMILY	, "The specified socket address family is not supported." },
				{ WINSOCKAPI_ERROR_CALLBACK_IN_PROGRESS			, "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function." },
				{ WINSOCKAPI_ERROR_MAX_SOCKET_DESCRIPTORS		, "No more socket descriptors are available." },
				{ WINSOCKAPI_ERROR_CANNOT_CREATE_SOCKET			, "No buffer space is available. The socket cannot be created." },
				{ WINSOCKAPI_ERROR_UNSUPPORTED_PROTOCOL			, "The specified sockets communication protocol is not supported." },
				{ WINSOCKAPI_ERROR_MISMATCHED_PROTOCOL			, "The specified sockets communication protocol is the wrong type for this socket." },
				{ WINSOCKAPI_ERROR_INVALID_SOCKET_TYPE			, "The specified socket type is not supported in the specified sockets address family." },
				{ WINSOCKAPI_ERROR_INVALID_SOCKETS_OPTION		, "optval is not in a valid part of the process address space or optlen parameter is too small." },
				{ WINSOCKAPI_ERROR_INVALID_SOCKETS_LEVEL		, "level is not valid, or the information in optval is not valid." },
				{ WINSOCKAPI_ERROR_KEEPALIVE_TIMEOUT			, "The sockets connection has timed out when SO_KEEPALIVE is set." },
				{ WINSOCKAPI_ERROR_UNSUPPORTED_OPTION			, "The sockets option is unknown or unsupported for the specified provider or socket." },
				{ WINSOCKAPI_ERROR_KEEPALIVE_FAILURE			, "The sockets connection has been reset when SO_KEEPALIVE is set." },
				{ WINSOCKAPI_ERROR_INVALID_SOCKET_DESCRIPTOR	, "The specified descriptor is not a socket." },
				{ WINSOCKAPI_ERROR_BROADCAST_DISABLED			, "Attempt to connect datagram socket to broadcast address failed because broadcast option was not enabled." },
				{ WINSOCKAPI_ERROR_SOCKET_IN_USE				, "A process is already bound to the same fully-qualified socket address and the REUSE option has not been set." },
				{ WINSOCKAPI_ERROR_INVALID_IPADDRESS			, "The specified address is not a valid IP address." },
				{ WINSOCKAPI_ERROR_INVALID_SOCKETS_NAME_FIELD	, "The binding name or namelen parameter is not valid." },
				{ WINSOCKAPI_ERROR_SOCKET_ALREADY_BOUND			, "The socket is already bound to an address." },
				{ WINSOCKAPI_ERROR_INSUF_BIND_BUFFERS			, "Not enough buffers available for binding, too many connections." },
				{ WINSOCKAPI_ERROR_SOCKET_NOT_BOUND				, "The socket has not been bound to an address." },
				{ WINSOCKAPI_ERROR_ALREADY_CONNECTED			, "The socket is already connected." },
				{ WINSOCKAPI_ERROR_NO_LISTEN_BUFFERS			, "No buffer space is available for listening." },
				{ WINSOCKAPI_ERROR_SOCKET_TYPE_FOR_LISTEN		, "The socket is not of a type that supports the listen operation." },
				{ WINSOCKAPI_ERROR_INCONSISTENT_SOCKET_TYPE		, "The requested shutdown mode is consistent with the socket type." },
				{ WINSOCKAPI_ERROR_SOCKET_NOT_CONNECTED			, "The socket is not connected." },
				{ WINSOCKAPI_ERROR_SOCKET_ALREADY_CANCELLED		, "The Windows Socket 1.1 call was canceled through a WSACancelBlockingCall." },
				{ WINSOCKAPI_ERROR_LINGER_TIMEOUT_VALUE			, "The socket is marked as nonblocking and SO_LINGER is set to a nonzero time-out value." },
				{ WINSOCKAPI_ERROR_AUTH_HOST_NOT_FOUND			, "Authoritative answer host not found." },
				{ WINSOCKAPI_ERROR_NONAUTH_HOST_NOT_FOUND		, "Nonauthoritative host not found, or server failure." },
				{ WINSOCKAPI_ERROR_NONRECOVERABLE_ERROR			, "A nonrecoverable error occurred getting a host name." },
				{ WINSOCKAPI_ERROR_VALID_HOST_NAME_NO_DATA		, "Unable to locate address information for designated host." },
				{ WINSOCKAPI_ERROR_SOCKETS_RESOURCE_ALLOCATION	, "Windows Sockets was unable to allocate needed resources for its internal operations." },
				{ WINSOCKAPI_ERROR_INVALID_TIMEOUT_OR_PARAMS	, "The time-out value is not valid, or all descriptor parameters are null." },
				{ WINSOCKAPI_ERROR_SOCKET_OP_NOT_SUPPORTED		, "A requested operation is not supported on this socket." },
				{ WINSOCKAPI_ERROR_INVALID_PARAMETER			, "A socket parameter is not completely contained in a valid part of the user address space." },
				{ WINSOCKAPI_ERROR_RECEIVE_AFTER_SHUTDOWN		, "The socket has been shut down; it is not possible to receive on a socket after shutdown." },
				{ WINSOCKAPI_ERROR_OPERATION_WOULD_BLOCK		, "The socket is marked as nonblocking and the requested operation would block." },
				{ WINSOCKAPI_ERROR_MESSAGE_TRUNCATED			, "The message was too large to fit into the specified buffer and was truncated." },
				{ WINSOCKAPI_ERROR_CONNECTION_ABORTED			, "The connection was abruptly terminated due to a time-out or other failure." },
				{ WINSOCKAPI_ERROR_CONNECTION_TIMEOUT			, "A socket transmission timeout occurred because of a network failure or because the peer system failed to respond." },
				{ WINSOCKAPI_ERROR_CONNECTION_RESET				, "The connection was reset by the remote side executing a hard or abortive close." },
				{ WINSOCKAPI_ERROR_SEND_AFTER_SHUTDOWN			, "The socket has been shut down; it is not possible to send on a socket after shutdown." },
				{ WINSOCKAPI_ERROR_BROADCAST_OPTION_NOT_SET		, "The requested address is a broadcast address, but the broadcast flag was not set." },
				{ WINSOCKAPI_ERROR_NO_SEND_BUFFERS				, "No buffer space is available for sending." },
				{ WINSOCKAPI_ERROR_REMOTE_HOST_UNREACHABLE		, "The remote host cannot be reached from this host at this time." },
				{ WINSOCKAPI_ERROR_MUST_LISTEN_BEFORE_ACCEPT	, "The listen function was not invoked prior to the accept function." },
				{ WINSOCKAPI_ERROR_RECEIVE_TIMEOUT				, "A socket timed out while waiting to receive data." },
				{ 0												, NULL }
			};


static ERROR_DICTIONARY_MODULE		WinSockAPIStatusErrorDictionary =
										{
										MODULE_WINSOCKAPI,
										WinSockAPIErrorCodes,
										WINSOCKAPI_ERROR_DICT_LENGTH,
										0
										};

unsigned long		WindowsSocketSessionCount = 0;


// This function must be called before any other function in this module.
void InitWinSockAPIModule()
{
	LinkModuleToList( &WinSockModuleInfo );
	RegisterErrorDictionary( &WinSockAPIStatusErrorDictionary );
}


BOOL InitWindowsSockets()
{
	WORD		WinsockVersionRequested;
	WSADATA		WinsockData;
	int			WinsockErrorCode;
	BOOL		bWinsockInitializedOK = FALSE;
	 
	if ( WindowsSocketSessionCount++ == 0 )
		{
		// Require at least version 1.1.
		WinsockVersionRequested = MAKEWORD( 1, 1 );
		// Initialize the WS2_32.DLL sockets interface.
		LogMessage( "Initializing sockets WS2_32.DLL.", MESSAGE_TYPE_DETAILS );
		WinsockErrorCode = WSAStartup( WinsockVersionRequested, &WinsockData );
		if ( WinsockErrorCode != 0 )
			{
			switch ( WinsockErrorCode )
				{
				case WSASYSNOTREADY:
					// The underlying network subsystem is not ready for network communication.
					RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NETWORK_NOT_READY );
					break;
				case WSAVERNOTSUPPORTED:
					// The version of Windows Sockets support requested is not provided by this
					// particular Windows Sockets implementation.
					RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKETS_VERSION_MISMATCH );
					break;
				case WSAEINPROGRESS:
					// A blocking Windows Sockets 1.1 operation is in progress.
					RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKETS_CURRENTLY_BLOCKED );
					break;
				case WSAEPROCLIM:
					// The limit on the number of tasks supported by the Windows Sockets implementation
					// has been reached.
					RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKETS_TASK_COUNT );
					break;
				case WSAEFAULT:
					// The WinsockData is not a valid pointer.
					RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_BAD_SOCKETS_STARTUP_DATA );
					break;
				}
			}
		// Confirm that the WinSock DLL supports 1.1.  Note that if the DLL supports
		// versions greater than 1.1 in addition to 1.1, it will still return 1.1 in
		// wVersion since that is the version we requested.
		else if ( LOBYTE( WinsockData.wVersion ) != 1 || HIBYTE( WinsockData.wVersion ) != 1 )
			{
			// A usable WinSock.DLL file could not be located.
			RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_MISSING_WINSOCK_DLL );
			WSACleanup();
			}
		else
			{
			bWinsockInitializedOK = TRUE;
			LogMessage( "Sockets initialized successfully.", MESSAGE_TYPE_SUPPLEMENTARY );
			}
		if ( !bWinsockInitializedOK )
			WindowsSocketSessionCount = 0;
		}
	else
		bWinsockInitializedOK = TRUE;

	return bWinsockInitializedOK;
}

void TerminateWindowsSockets()
{
	if ( WindowsSocketSessionCount > 0 )
		{
		WindowsSocketSessionCount--;
		if ( WindowsSocketSessionCount == 0 )
			WSACleanup();
		}
}


SOCKET CreateWindowsSocket()
{
	// Create a socket for internet communication.  The socket function creates a socket that is bound to a specific
	// service provider.  When a socket is created, by default it is a blocking socket
	//
	//		SOCKET	  socket(
	//						int		af,			// [in] Address family specification.
	//						int		type,		// [in] Type specification for the new socket.  The following are the
	//											//		only two type specifications supported for Windows Sockets 1.1:
	//											//
	//											//		SOCK_STREAM Provides sequenced, reliable, two-way, connection-based
	//											//				byte streams with an OOB data transmission mechanism. Uses
	//											//				TCP for the Internet address family.
	//											//		SOCK_DGRAM Supports datagrams, which are connectionless, unreliable
	//											//				buffers of a fixed (typically small) maximum length. Uses
	//											//				UDP for the Internet address family. 
	//											//		In Windows Sockets 2, many new socket types were introduced and no
	//											//				longer need to be specified, since an application can
	//											//				dynamically discover the attributes of each available
	//											//				transport protocol through the WSAEnumProtocols() function.
	//											//				Socket type definitions appear in Winsock2.h, which is
	//											//				periodically updated as new socket types, address families,
	//											//				and protocols are defined.
	//						int		protocol	// [in] Protocol to be used with the socket that is specific to the indicated
	//											//		address family.
	//						);
	//
	// Return Values:	If no error occurs, socket() returns a descriptor referencing the new socket. Otherwise, a value of
	//					INVALID_SOCKET is returned, and a specific error code can be retrieved by calling WSAGetLastError().
	//
	SOCKET			SocketDescriptor;
	int				WinSockErrorCode;
	
	SocketDescriptor = socket( AF_INET, SOCK_STREAM, 0 );
	if ( SocketDescriptor == INVALID_SOCKET )
		{
		WinSockErrorCode = WSAGetLastError();
		switch ( WinSockErrorCode )
			{
			case WSANOTINITIALISED:
				// A successful WSAStartup call must occur before using this function.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NO_WSASTARTUP );
				break;
			case WSAENETDOWN:
				// The network subsystem or the associated service provider has failed.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NETWORK_FAILED );
				break; 
			case WSAEAFNOSUPPORT:
				// The specified address family is not supported.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_UNSUPPORTED_ADDRESS_FAMILY );
				break; 
			case WSAEINPROGRESS:
				// A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_CALLBACK_IN_PROGRESS );
				break;
			case WSAEMFILE:
				// No more socket descriptors are available.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_MAX_SOCKET_DESCRIPTORS );
				break;
			case WSAENOBUFS:
				// No buffer space is available. The socket cannot be created.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_CANNOT_CREATE_SOCKET );
				break;
			case WSAEPROTONOSUPPORT:
				// The specified protocol is not supported.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_UNSUPPORTED_PROTOCOL );
				break;
			case WSAEPROTOTYPE:
				// The specified protocol is the wrong type for this socket.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_MISMATCHED_PROTOCOL );
				break;
			case WSAESOCKTNOSUPPORT:
				// The specified socket type is not supported in this address family.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INVALID_SOCKET_TYPE );
				break;
			default:
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_UNKNOWN );
				break;
			}
		}
	return SocketDescriptor;
}


BOOL GetWindowsHostByAddress( char *pAddressBuffer, int AddressLength, int AddressType, hostent **ppHostInformation )
{
	// The gethostbyaddr function retrieves the host information corresponding to a network address.
	//		struct HOSTENT* FAR	   gethostbyaddr(
	//											const char* addr,	// [in] Pointer to an address in network byte order.
	//											int len,			// [in] Length of the address, in bytes.
	//											int type			// [in] Type of the address, such as the AF_INET address
	//																//		family type (defined as TCP, UDP, and other associated
	//																//		Internet protocols). Address family types and their
	//																//		corresponding values are defined in Winsock2.h.
	//											);
	//
	// Return Values:  If no error occurs, gethostbyaddr() returns a pointer to the hostent structure.
	//					Otherwise, it returns a null pointer, and a specific error code can be retrieved
	//					by calling WSAGetLastError().
	//
	// The gethostbyaddr() function returns a pointer to the hostent structure that contains the name and address corresponding
	// to the given network address.
	//
	// The hostent structure is used by functions to store information about a given host, such as host name,
	// IP address, and so forth. An application should never attempt to modify this structure or to free any
	// of its components. Furthermore, only one copy of the hostent structure is allocated per thread, and an
	// application should therefore copy any information that it needs before issuing any other Windows Sockets
	// API calls.
	//
	// typedef struct hostent
	//		{
	//		char FAR*			h_name;			// Official name of the host (PC). If using the DNS or
	//											// similar resolution system, it is the Fully Qualified
	//											// Domain Name (FQDN) that caused the server to return a
	//											// reply. If using a local hosts file, it is the first
	//											// entry after the IP address. 
	//		char FAR  FAR**		h_aliases;		// Null-terminated array of alternate names.
	//		short				h_addrtype;		// Type of address being returned.
	//		short				h_length;		// Length of each address, in bytes.
	//		char FAR  FAR**		h_addr_list;	// Null-terminated list of addresses for the host. Addresses
	//											// are returned in network byte order. The macro h_addr is
	//											// defined to be h_addr_list[0] for compatibility with older
	//											// software.
	//		} hostent;
	//
	struct hostent			*pHostEnt;
	BOOL					bNoError = TRUE;
	int						WinSockErrorCode;

	pHostEnt = gethostbyaddr( pAddressBuffer, AddressLength, AddressType );
	if ( pHostEnt == 0 )
		{
		bNoError = FALSE;
		WinSockErrorCode = WSAGetLastError();
		switch ( WinSockErrorCode )
			{
			case WSANOTINITIALISED:
				// A successful WSAStartup call must occur before using this function.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NO_WSASTARTUP );
				break;
			case WSAENETDOWN:
				// The network subsystem has failed.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NETWORK_FAILED );
				break; 
			case WSAHOST_NOT_FOUND:
				// Authoritative answer host not found. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_AUTH_HOST_NOT_FOUND );
				break;
			case WSATRY_AGAIN:
				// Nonauthoritative host not found, or server failure. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NONAUTH_HOST_NOT_FOUND );
				break;
			case WSANO_RECOVERY:
				// A nonrecoverable error occurred. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NONRECOVERABLE_ERROR );
				break;
			case WSANO_DATA:
				// Valid name, no data record of requested type. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_VALID_HOST_NAME_NO_DATA );
				break;
			case WSAEINPROGRESS:
				// A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_CALLBACK_IN_PROGRESS );
				break;
			case WSAEAFNOSUPPORT:
				// The type specified is not supported by the Windows Sockets implementation.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_UNSUPPORTED_ADDRESS_FAMILY );
				break; 
			case WSAEINTR:
				// The (blocking) Windows Socket 1.1 call was canceled through WSACancelBlockingCall.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKET_ALREADY_CANCELLED );
				break;
			case WSAEFAULT:
				// The addr parameter is not a valid part of the user address space. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INVALID_SOCKETS_NAME_FIELD );
				break;
			default:
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_UNKNOWN );
				break;
			}
		*ppHostInformation = 0;
		}
	else
		*ppHostInformation = pHostEnt;
	return bNoError;
}


BOOL WindowsSelect( fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timeval *timeout, int *pReadySocketCount )
{
	// The select function determines the status of one or more sockets, waiting if necessary, to perform synchronous I/O.
	//
	//			int   select(
	//						int						nfds,		// [in] Ignored. The nfds parameter is included only for
	//															//		compatibility with Berkeley sockets.
	//						fd_set					*readfds,	// [in, out] Optional pointer to a set of sockets to be
	//															//		checked for readability.
	//						fd_set					*writefds,	// [in, out] Optional pointer to a set of sockets to be
	//															//		checked for writability.
	//						fd_set					*exceptfds,	// [in, out] Optional pointer to a set of sockets to be
	//															//		checked for errors.
	//						const struct timeval	*timeout	// [in] Maximum time for select to wait, provided in the
	//															//		form of a TIMEVAL structure. Set the timeout parameter
	//															//		to null for blocking operations.
	//						);
	//
	// Return Values:  The select() function returns the total number of socket handles that are ready and contained in the
	//					fd_set structures, zero if the time limit expired, or SOCKET_ERROR if an error occurred. If the return
	//					value is SOCKET_ERROR, WSAGetLastError() can be used to retrieve a specific error code.
	//
	// The select() function is used to determine the status of one or more sockets. For each socket, the caller can request
	// information on read, write, or error status. The set of sockets for which a given status is requested is indicated by an
	// fd_set structure. The sockets contained within the fd_set structures must be associated with a single service provider.
	// For the purpose of this restriction, sockets are considered to be from the same service provider if the WSAPROTOCOL_INFO
	// structures describing their protocols have the same providerId value. Upon return, the structures are updated to reflect
	// the subset of these sockets that meet the specified condition. The select() function returns the number of sockets meeting
	// the conditions. A set of macros is provided for manipulating an fd_set structure. These macros are compatible with those
	// used in the Berkeley software, but the underlying representation is completely different.
	//
	// The parameter readfds identifies the sockets that are to be checked for readability. If the socket is currently in the
	// listen state, it will be marked as readable if an incoming connection request has been received such that an accept() is
	// guaranteed to complete without blocking. For other sockets, readability means that queued data is available for reading
	// such that a call to recv(), WSARecv(), WSARecvFrom(), or recvfrom() is guaranteed not to block.
	//
	// For connection-oriented sockets, readability can also indicate that a request to close the socket has been received from
	// the peer. If the virtual circuit was closed gracefully, and all data was received, then a recv() will return immediately
	// with zero bytes read. If the virtual circuit was reset, then a recv() will complete immediately with an error code such as
	// WSAECONNRESET. The presence of OOB data will be checked if the socket option SO_OOBINLINE has been enabled (see setsockopt).
	//
	// The parameter writefds identifies the sockets that are to be checked for writability. If a socket is processing a connect()
	// call (nonblocking), a socket is writeable if the connection establishment successfully completes. If the socket is not
	// processing a connect() call, writability means a send(), sendto(), or WSASendto() are guaranteed to succeed. However, they
	// can block on a blocking socket if the len parameter exceeds the amount of outgoing system buffer space available. It is not
	// specified how long these guarantees can be assumed to be valid, particularly in a multithreaded environment.
	//
	// The parameter exceptfds identifies the sockets that are to be checked for the presence of OOB data (see section DECnet
	// Out-of-band data for a discussion of this topic) or any exceptional error conditions.  Note:  Out-of-band data will only
	// be reported in this way if the option SO_OOBINLINE is FALSE. If a socket is processing a connect() call (nonblocking),
	// failure of the connect attempt is indicated in exceptfds (application must then call getsockopt SO_ERROR to determine the
	// error value to describe why the failure occurred). This document does not define which other errors will be included.
	//
	// Any two of the parameters, readfds, writefds, or exceptfds, can be given as null. At least one must be non-null, and any
	// non-null descriptor set must contain at least one handle to a socket.
	//
	// In summary, a socket will be identified in a particular set when select() returns if:
	//
	//		readfds:	If listen has been called and a connection is pending, accept will succeed.
	//					Data is available for reading (includes OOB data if SO_OOBINLINE is enabled).
	//					Connection has been closed/reset/terminated.
	//		writefds:	If processing a connect call (nonblocking), connection has succeeded.
	//					Data can be sent. 
	//		exceptfds:	If processing a connect call (nonblocking), connection attempt failed.
	//					OOB data is available for reading (only if SO_OOBINLINE is disabled).
	//
	// Four macros are defined in the header file Winsock2.h for manipulating and checking the descriptor sets. The variable
	// FD_SETSIZE determines the maximum number of descriptors in a set. (The default value of FD_SETSIZE is 64, which can be
	// modified by defining FD_SETSIZE to another value before including Winsock2.h.) Internally, socket handles in an fd_set
	// structure are not represented as bit flags as in Berkeley Unix. Their data representation is opaque. Use of these macros
	// will maintain software portability between different socket environments. The macros to manipulate and check fd_set
	// contents are:
	//
	//		FD_CLR(s, *set)				Removes the descriptor s from set. 
	//		FD_ISSET(s, *set)			Nonzero if s is a member of the set. Otherwise, zero. 
	//		FD_SET(s, *set)				Adds descriptor s to set.
	//		FD_ZERO(*set)				Initializes the set to the null set.
	//
	// The parameter time-out controls how long the select() can take to complete. If time-out is a null pointer, select() will block
	// indefinitely until at least one descriptor meets the specified criteria. Otherwise, time-out points to a TIMEVAL structure
	// that specifies the maximum time that select() should wait before returning. When select() returns, the contents of the TIMEVAL
	// structure are not altered. If TIMEVAL is initialized to {0, 0}, select() will return immediately; this is used to poll the
	// state of the selected sockets. If select() returns immediately, then the select() call is considered nonblocking and the standard
	// assumptions for nonblocking calls apply. For example, the blocking hook will not be called, and Windows Sockets will not yield.
	//
	// Note:  The select() function has no effect on the persistence of socket events registered with WSAAsyncSelect() or WSAEventSelect().
	//
	BOOL			bNoError = TRUE;
	int				WinSockErrorCode;

	*pReadySocketCount = select( 0, readfds, writefds, exceptfds, timeout );
	if ( *pReadySocketCount == SOCKET_ERROR )
		{
		bNoError = FALSE;
		WinSockErrorCode = WSAGetLastError();
		switch ( WinSockErrorCode )
			{
			case WSANOTINITIALISED:
				// A successful WSAStartup call must occur before using this function.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NO_WSASTARTUP );
				break;
			case WSAENETDOWN:
				// The network subsystem has failed.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NETWORK_FAILED );
				break;
			case WSAEINPROGRESS:
				// A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_CALLBACK_IN_PROGRESS );
				break;
			case WSAENOTSOCK:
				// The descriptor is not a socket.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INVALID_SOCKET_DESCRIPTOR );
				break;
			case WSAEINTR:
				// The (blocking) Windows Socket 1.1 call was canceled through WSACancelBlockingCall.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKET_ALREADY_CANCELLED );
				break;
			case WSAEFAULT:
				// The Windows Sockets implementation was unable to allocate needed resources for its internal operations,
				// or the readfds, writefds, exceptfds, or timeval parameters are not part of the user address space. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKETS_RESOURCE_ALLOCATION );
				break;
			case WSAEINVAL:
				// The time-out value is not valid, or all three descriptor parameters were null.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INVALID_TIMEOUT_OR_PARAMS );
				break;
			default:
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_UNKNOWN );
				break;
			}
		}

	return bNoError;
}


BOOL WindowsSetSocketOptions( SOCKET SocketDescriptor, int ProtocolLevel, int OptionName, const char *OptionValue, int nValueBytes )
{
	// The setsockopt() function sets a socket option.
	//
	//		  int setsockopt(
	//						SOCKET		s,			// [in] Descriptor identifying a socket.
	//						int			level,		// [in] Level at which the option is defined; the supported levels include
	//												//		SOL_SOCKET and IPPROTO_TCP. See Windows Sockets 2 Protocol-Specific
	//												//		Annex for more information on protocol-specific levels. 
	//						int			optname,	// [in] Socket option for which the value is to be set.
	//						const char	*optval,	// [in] Pointer to the buffer in which the value for the requested option
	//												//		is specified.
	//						int			optlen		// [in] Size of the optval buffer, in bytes.
	//						);
	//
	// Return Values:  If no error occurs, setsockopt() returns zero. Otherwise, a value of SOCKET_ERROR is returned, and a
	//					specific error code can be retrieved by calling WSAGetLastError().
	//
	// There are two types of socket options: Boolean options that enable or disable a feature or behavior, and options that
	// require an integer value or structure. To enable a Boolean option, optval points to a nonzero integer. To disable the
	// option optval points to an integer equal to zero. The optlen parameter should be equal to sizeof(int) for Boolean
	// options. For other options, optval points to the an integer or structure that contains the desired value for the option,
	// and optlen is the length of the integer or structure.
	//
	// The following options are supported for setsockopt. For default values of these options, see the description. The Type
	// identifies the type of data addressed by optval.
	//
	//			level = SOL_SOCKET
	//
	//	SO_BROADCAST			BOOL	Allows transmission of broadcast messages on the socket. 
	//	SO_CONDITIONAL_ACCEPT	BOOL	Enables sockets to delay the acknowledgment of a connection until after the WSAAccept
	//										condition function is called.  Setting this socket option to TRUE delays the
	//										acknowledgment of a connection until after the WSAAccept condition function is
	//										called. If FALSE, the connection may be accepted before the condition function
	//										is called, but the connection will be disconnected if the condition function
	//										rejects the call. This option must be set before calling the listen function,
	//										otherwise WSAEINVAL is returned. SO_CONDITIONAL_ACCEPT is only supported for TCP
	//										and ATM.  TCP sets SO_CONDITIONAL_ACCEPT to FALSE by default, and therefore by
	//										default the connection will be accepted before the WSAAccept condition function
	//										is called. When set to TRUE, the conditional decision must be made within the TCP
	//										connection time-out. CF_DEFER connections are still subject to the time-out.
	//										ATM sets SO_CONDITIONAL_ACCEPT to TRUE by default.
	//	SO_DEBUG				BOOL	Records debugging information. 
	//	SO_DONTLINGER			BOOL	Does not block close waiting for unsent data to be sent. Setting this option is
	//										equivalent to setting SO_LINGER with l_onoff set to zero. 
	//	SO_DONTROUTE			BOOL	Does not route: sends directly to interface. Succeeds but is ignored on AF_INET sockets;
	//										fails on AF_INET6 sockets with WSAENOPROTOOPT. Not supported on ATM sockets
	//										(results in an error). 
	//	SO_GROUP_PRIORITY		int		Reserved. 
	//	SO_KEEPALIVE			BOOL	Sends keep-alives. Not supported on ATM sockets (results in an error).  An application
	//										can request that a TCP/IP provider enable the use of keep-alive packets on TCP
	//										connections by turning on the SO_KEEPALIVE socket option. A Windows Sockets
	//										provider need not support the use of keep-alives. If it does, the precise semantics
	//										are implementation-specific but should conform to section 4.2.3.6 of RFC 1122:
	//										Requirements for Internet Hosts—Communication Layers. If a connection is dropped
	//										as the result of keep-alives the error code WSAENETRESET is returned to any calls
	//										in progress on the socket, and any subsequent calls will fail with WSAENOTCONN. 
	//	SO_LINGER				LINGER	Lingers on close if unsent data is present.  The SO_LINGER option controls the action
	//										taken when unsent data is queued on a socket and a closesocket is performed. See
	//										closesocket for a description of the way in which the SO_LINGER settings affect
	//										the semantics of closesocket. The application sets the desired behavior by creating
	//										a LINGER structure (pointed to by the optval parameter) with these members l_onoff
	//										and l_linger set appropriately. 
	//	SO_OOBINLINE			BOOL	Receives OOB data in the normal data stream. (See section Protocol Independent
	//										Out-Of-band Data for a discussion of this topic.) 
	//	SO_RCVBUF				int		Specifies the total per-socket buffer space reserved for receives. This is unrelated
	//										to SO_MAX_MSG_SIZE or the size of a TCP window.  When a Windows Sockets implementation
	//										supports the SO_RCVBUF and SO_SNDBUF options, an application can request different
	//										buffer sizes (larger or smaller). The call to setsockopt can succeed even when the
	//										implementation did not provide the whole amount requested. An application must call
	//										getsockopt with the same option to check the buffer size actually provided. 
	//	SO_REUSEADDR			BOOL	Allows the socket to be bound to an address that is already in use. (See bind.) Not
	//										applicable on ATM sockets.  By default, a socket cannot be bound (see bind) to a
	//										local address that is already in use. On occasion, however, it can be necessary to
	//										reuse an address in this way. Since every connection is uniquely identified by the
	//										combination of local and remote addresses, there is no problem with having two
	//										sockets bound to the same local address as long as the remote addresses are
	//										different. To inform the Windows Sockets provider that a bind on a socket should
	//										not be disallowed because the desired address is already in use by another socket,
	//										the application should set the SO_REUSEADDR socket option for the socket before
	//										issuing the bind. The option is interpreted only at the time of the bind. It is
	//										therefore unnecessary and harmless to set the option on a socket that is not to
	//										be bound to an existing address. Setting or resetting the option after the bind
	//										has no effect on this or any other socket.
	//	SO_EXCLUSIVEADDRUSE		BOOL	Enables a socket to be bound for exclusive access. Does not require administrative privilege.  
	//	SO_SNDBUF				int		Specifies the total per-socket buffer space reserved for sends. This is unrelated to
	//										SO_MAX_MSG_SIZE or the size of a TCP window.  When a Windows Sockets implementation
	//										supports the SO_RCVBUF and SO_SNDBUF options, an application can request different
	//										buffer sizes (larger or smaller). The call to setsockopt can succeed even when the
	//										implementation did not provide the whole amount requested. An application must call
	//										getsockopt with the same option to check the buffer size actually provided.
	//	PVD_CONFIG				???		This object stores the configuration information for the service provider associated with
	//										socket s. The exact format of this data structure is service provider specific. 
	//
	//			level = IPPROTO_TCP1, included for backward compatibility with Windows Sockets 1.1.
	//
	//	TCP_NODELAY				BOOL	Disables the Nagle algorithm for send coalescing.  Application writers should not set
	//										TCP_NODELAY unless the impact of doing so is well-understood and desired because setting
	//										TCP_NODELAY can have a significant negative impact on network and application performance. 
	//
	//
	//			level = NSPROTO_IPX
	//
	// IPX_PTYPE				int		Sets the IPX packet type. 
	// IPX_FILTERPTYPE			int		Sets the receive filter packet type 
	// IPX_STOPFILTERPTYPE		int		Stops filtering the filter type set with IPX_FILTERTYPE 
	// IPX_DSTYPE				int		Sets the value of the data stream field in the SPX header on every packet sent. 
	// IPX_EXTENDED_ADDRESS		BOOL	Sets whether extended addressing is enabled. 
	// IPX_RECVHDR				BOOL	Sets whether the protocol header is sent up on all receive headers. 
	// IPX_RECEIVE_BROADCAST	BOOL	Indicates broadcast packets are likely on the socket. Set to TRUE by default.  Applications
	//										that do not use broadcasts should set this to FALSE for better system performance. 
	// IPX_IMMEDIATESPXACK		BOOL	Directs SPX connections not to delay before sending an ACK. Applications without
	//										back-and-forth traffic should set this to TRUE to increase performance.
	//
	BOOL			bNoError = TRUE;
	int				WinSockErrorCode;

	if ( setsockopt( SocketDescriptor, ProtocolLevel, OptionName, OptionValue, nValueBytes ) == SOCKET_ERROR )
		{
		bNoError = FALSE;
		WinSockErrorCode = WSAGetLastError();
		switch ( WinSockErrorCode )
			{
			case WSANOTINITIALISED:
				// A successful WSAStartup call must occur before using this function.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NO_WSASTARTUP );
				break;
			case WSAENETDOWN:
				// The network subsystem has failed.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NETWORK_FAILED );
				break;
			case WSAEFAULT:
				// optval is not in a valid part of the process address space or optlen parameter is too small.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INVALID_SOCKETS_OPTION );
				break;
			case WSAEINPROGRESS:
				// A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_CALLBACK_IN_PROGRESS );
				break;
			case WSAEINVAL:
				// level is not valid, or the information in optval is not valid.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INVALID_SOCKETS_LEVEL );
				break;
			case WSAENETRESET:
				// Connection has timed out when SO_KEEPALIVE is set.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_KEEPALIVE_TIMEOUT );
				break;
			case WSAENOPROTOOPT:
				// The option is unknown or unsupported for the specified provider or socket (see SO_GROUP_PRIORITY limitations).
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_UNSUPPORTED_OPTION );
				break;
			case WSAENOTCONN:
				// Connection has been reset when SO_KEEPALIVE is set.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_KEEPALIVE_FAILURE );
				break;
			case WSAENOTSOCK:
				// The descriptor is not a socket.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INVALID_SOCKET_DESCRIPTOR );
				break;
			default:
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_UNKNOWN );
				break;
			}
		}
	return bNoError;
}


BOOL WindowsSocketBind( SOCKET SocketDescriptor, struct sockaddr *pInternetAddr )
{
	// The bind function associates a local address with a socket.
	//
	//		int bind(
	//				SOCKET					s,			// [in] Descriptor identifying an unbound socket.
	//				const struct sockaddr*	name,		// [in] Address to assign to the socket from the SOCKADDR structure.
	//				int						namelen		// Length of the value in the name parameter, in bytes.
	//				);
	//
	// Return Value:  If no error occurs, bind() returns zero. Otherwise, it returns SOCKET_ERROR, and a specific error code
	//					can be retrieved by calling WSAGetLastError().
	//
	BOOL			bNoError = TRUE;
	int				WinSockErrorCode;

	if ( bind( SocketDescriptor, pInternetAddr, sizeof(*pInternetAddr) ) == SOCKET_ERROR )
		{
		bNoError = FALSE;
		WinSockErrorCode = WSAGetLastError();
		switch ( WinSockErrorCode )
			{
			case WSANOTINITIALISED:
				// A successful WSAStartup call must occur before using this function. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NO_WSASTARTUP );
				break;
			case WSAENETDOWN:
				// The network subsystem has failed. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NETWORK_FAILED );
				break;
			case WSAEACCES:
				// Attempt to connect datagram socket to broadcast address failed because setsockopt option SO_BROADCAST is
				// not enabled. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_BROADCAST_DISABLED );
				break;
			case WSAEADDRINUSE:
				// A process on the computer is already bound to the same fully-qualified address and the socket has not
				// been marked to allow address reuse with SO_REUSEADDR. For example, the IP address and port are bound in
				// the af_inet case). (See the SO_REUSEADDR socket option under setsockopt.) 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKET_IN_USE );
				break;
			case WSAEADDRNOTAVAIL:
				// The specified address is not a valid address for this computer. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INVALID_IPADDRESS );
				break;
			case WSAEFAULT:
				// The name or namelen parameter is not a valid part of the user address space, the namelen parameter is too
				// small, the name parameter contains an incorrect address format for the associated address family, or the
				// first two bytes of the memory block specified by name does not match the address family associated with
				// the socket descriptor s. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INVALID_SOCKETS_NAME_FIELD );
				break;
			case WSAEINPROGRESS:
				// A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_CALLBACK_IN_PROGRESS );
				break;
			case WSAEINVAL:
				// The socket is already bound to an address. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKET_ALREADY_BOUND );
				break;
			case WSAENOBUFS:
				// Not enough buffers available, too many connections. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INSUF_BIND_BUFFERS );
				break;
			case WSAENOTSOCK:
				// The descriptor is not a socket. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INVALID_SOCKET_DESCRIPTOR );
				break;
			default:
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_UNKNOWN );
				break;
			}
		}
	return bNoError;
}


BOOL WindowsGetSocketName( SOCKET SocketDescriptor, struct sockaddr *pInternetAddr )
{
	// The getsockname() function retrieves the local name for a socket.
	//
	//		int  getsockname(
	//						SOCKET				s,			// [in] Descriptor identifying a socket.
	//						struct sockaddr		*name,		// [out] Pointer to a SOCKADDR structure that receives
	//														//		 the address (name) of the socket.
	//						int					*namelen	// [in, out] Size of the name buffer, in bytes.
	//						);
	//
	// Return Values:  If no error occurs, getsockname returns zero. Otherwise, a value of SOCKET_ERROR is returned, and
	//					a specific error code can be retrieved by calling WSAGetLastError().
	//
	// The getsockname function retrieves the current name for the specified socket descriptor in name. It is used on
	// the bound or connected socket specified by the s parameter. The local association is returned. This call is
	// especially useful when a connect call has been made without doing a bind first; the getsockname function provides
	// the only way to determine the local association that has been set by the system.
	//
	// On call, the namelen parameter contains the size of the name buffer, in bytes. On return, the namelen parameter
	// contains the actual size in bytes of the name parameter.
	//
	// The getsockname function does not always return information about the host address when the socket has been bound
	// to an unspecified address, unless the socket has been connected with connect or accept (for example, using ADDR_ANY).
	// A Windows Sockets application must not assume that the address will be specified unless the socket is connected.
	// The address that will be used for the socket is unknown unless the socket is connected when used in a multihomed host.
	// If the socket is using a connectionless protocol, the address may not be available until I/O occurs on the socket.
	//
	BOOL			bNoError = TRUE;
	int				WinSockErrorCode;
	int				SocketNameLength;

	SocketNameLength = sizeof(*pInternetAddr);
	if ( getsockname( SocketDescriptor, pInternetAddr, &SocketNameLength ) == SOCKET_ERROR )
		{
		bNoError = FALSE;
		WinSockErrorCode = WSAGetLastError();
		switch ( WinSockErrorCode )
			{
			case WSANOTINITIALISED:
				// A successful WSAStartup call must occur before using this function. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NO_WSASTARTUP );
				break;
			case WSAENETDOWN:
				// The network subsystem has failed. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NETWORK_FAILED );
				break;
			case WSAEFAULT:
				// The name or namelen parameter is not a valid part of the user address space, the namelen parameter is too
				// small. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INVALID_SOCKETS_NAME_FIELD );
				break;
			case WSAEINPROGRESS:
				// A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_CALLBACK_IN_PROGRESS );
				break;
			case WSAENOTSOCK:
				// The descriptor is not a socket. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INVALID_SOCKET_DESCRIPTOR );
				break;
			case WSAEINVAL:
				// The socket has not been bound to an address with bind, or ADDR_ANY is specified in bind but connection
				// has not yet occured.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKET_NOT_BOUND );
				break;
			default:
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_UNKNOWN );
				break;
			}
		}
	return bNoError;
}


BOOL WindowsSocketListen( SOCKET SocketDescriptor, int nMaxPendingConnectionBacklog )
{
	// The listen() function places a socket in a state in which it is listening for an incoming connection.
	//
	//		int   listen(
	//					SOCKET		s,			// [in] Descriptor identifying a bound, unconnected socket.
	//					int			backlog		// [in] Maximum length of the queue of pending connections.
	//											//		If set to SOMAXCONN, the underlying service provider
	//											//		responsible for socket s will set the backlog to a
	//											//		maximum reasonable value. There is no standard provision
	//											//		to obtain the actual backlog value. 
	//					);
	//
	// Return Values:  If no error occurs, listen() returns zero. Otherwise, a value of SOCKET_ERROR is returned, and a
	//					specific error code can be retrieved by calling WSAGetLastError().
	//
	// To accept connections, a socket is first created with the socket() function and bound to a local address with the
	// bind() function, a backlog for incoming connections is specified with listen(), and then the connections are accepted
	// with the accept() function. Sockets that are connection-oriented, those of type SOCK_STREAM for example, are used
	// with listen(). The socket s is put into passive mode where incoming connection requests are acknowledged and queued
	// pending acceptance by the process.
	//
	// The listen() function is typically used by servers that can have more than one connection request at a time. If a
	// connection request arrives and the queue is full, the client will receive an error with an indication of WSAECONNREFUSED.
	//
	// If there are no available socket descriptors, listen() attempts to continue to function. If descriptors become available,
	// a later call to listen() or accept() will refill the queue to the current or most recent backlog, if possible, and
	// resume listening for incoming connections.
	//
	// An application can call listen() more than once on the same socket. This has the effect of updating the current backlog
	// for the listening socket. Should there be more pending connections than the new backlog value, the excess pending
	// connections will be reset and dropped.
	//
	BOOL			bNoError = TRUE;
	int				WinSockErrorCode;

	if ( listen( SocketDescriptor, nMaxPendingConnectionBacklog ) == SOCKET_ERROR )
		{
		bNoError = FALSE;
		WinSockErrorCode = WSAGetLastError();
		switch ( WinSockErrorCode )
			{
			case WSANOTINITIALISED:
				// A successful WSAStartup call must occur before using this function.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NO_WSASTARTUP );
				break;
			case WSAENETDOWN:
				// The network subsystem has failed.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NETWORK_FAILED );
				break;
			case WSAEADDRINUSE:
				// A process on the computer is already bound to the same fully-qualified address and the socket has not
				// been marked to allow address reuse with SO_REUSEADDR. For example, the IP address and port are bound in
				// the af_inet case). (See the SO_REUSEADDR socket option under setsockopt.) 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKET_IN_USE );
				break;
			case WSAEINPROGRESS:
				// A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_CALLBACK_IN_PROGRESS );
				break;
			case WSAEINVAL:
				// level is not valid, or the information in optval is not valid.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INVALID_SOCKETS_LEVEL );
				break;
			case WSAEMFILE:
				// No more socket descriptors are available.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_MAX_SOCKET_DESCRIPTORS );
				break;
			case WSAENOTSOCK:
				// The descriptor is not a socket.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INVALID_SOCKET_DESCRIPTOR );
				break;
			case WSAEISCONN:
				// The socket is already connected.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_ALREADY_CONNECTED );
				break;
			case WSAENOBUFS:
				// No buffer space is available for listening.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NO_LISTEN_BUFFERS );
				break;
			case WSAEOPNOTSUPP:
				// The referenced socket is not of a type that supports the listen operation.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKET_TYPE_FOR_LISTEN );
				break;
			default:
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_UNKNOWN );
				break;
			}
		}
	return bNoError;
}


BOOL WindowsSocketAccept( SOCKET ListeningSocket, SOCKET *pConnectingSocket, struct sockaddr *pInternetAddr )
{
	// The getsockname() function permits an incoming connection attempt on a socket.
	//
	//		int  getsockname(
	//						SOCKET				s,			// [in] Descriptor identifying a socket that has been placed
	//														//		in a listening state with the listen() function. The
	//														//		connection is actually made with the socket that is
	//														//		returned by accept(). 
	//						struct sockaddr		*addr,		// [out] Optional pointer to a buffer that receives the address
	//														//		of the connecting entity, as known to the communications
	//														//		layer. The exact format of the addr parameter is determined
	//														//		by the address family that was established when the socket
	//														//		from the SOCKADDR structure was created. 
	//						int					*addrlen	// [out] Optional pointer to an integer that contains the length of addr.
	//						);
	//
	// Return Values:  If no error occurs, accept() returns a value of type SOCKET that is a descriptor for the new socket.
	//					This returned value is a handle for the socket on which the actual connection is made.  Otherwise, a
	//					value of INVALID_SOCKET is returned, and a specific error code can be retrieved by calling WSAGetLastError().
	//
	//					The integer referred to by addrlen initially contains the amount of space pointed to by addr. On return it
	//					will contain the actual length in bytes of the address returned.
	//
	// The accept() function extracts the first connection on the queue of pending connections on socket s. It then creates and returns
	// a handle to the new socket. The newly created socket is the socket that will handle the actual connection; it has the same
	// properties as socket s, including the asynchronous events registered with the WSAAsyncSelect() or WSAEventSelect() functions.
	//
	// The accept() function can block the caller until a connection is present if no pending connections are present on the queue,
	// and the socket is marked as blocking. If the socket is marked as nonblocking and no pending connections are present on the queue,
	// accept() returns an error as described in the following. After a successful completion, accept() returns a new socket handle,
	// the accepted socket cannot be used to accept more connections. The original socket remains open and listens for new connection
	// requests.
	//
	// The parameter addr is a result parameter that is filled in with the address of the connecting entity, as known to the
	// communications layer. The exact format of the addr parameter is determined by the address family in which the communication
	// is occurring. The addrlen is a value-result parameter; it should initially contain the amount of space pointed to by addr; on
	// return it will contain the actual length (in bytes) of the address returned.
	//
	// The accept() function is used with connection-oriented socket types such as SOCK_STREAM. If addr and/or addrlen are equal to
	// NULL, then no information about the remote address of the accepted socket is returned.
	//
	BOOL			bNoError = TRUE;
	int				WinSockErrorCode;
	int				SocketNameLength;

	SocketNameLength = sizeof(*pInternetAddr);
	*pConnectingSocket = accept( ListeningSocket, pInternetAddr, &SocketNameLength );
	if ( *pConnectingSocket == INVALID_SOCKET )
		{
		bNoError = FALSE;
		WinSockErrorCode = WSAGetLastError();
		switch ( WinSockErrorCode )
			{
			case WSANOTINITIALISED:
				// A successful WSAStartup call must occur before using this function. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NO_WSASTARTUP );
				break;
			case WSAENETDOWN:
				// The network subsystem has failed. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NETWORK_FAILED );
				break;
			case WSAEFAULT:
				// The addrlen parameter is too small or addr is not a valid part of the user address space.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INVALID_SOCKETS_NAME_FIELD );
				break;
			case WSAEINTR:
				// The (blocking) Windows Sockets 1.1 call was canceled through WSACancelBlockingCall.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKET_ALREADY_CANCELLED );
				break;
			case WSAEINPROGRESS:
				// A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_CALLBACK_IN_PROGRESS );
				break;
			case WSAEMFILE:
				// The queue is nonempty upon entry to accept and there are no descriptors available.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_MAX_SOCKET_DESCRIPTORS );
				break;
			case WSAENOBUFS:
				// No buffer space is available. The socket cannot be created.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_CANNOT_CREATE_SOCKET );
				break;
			case WSAENOTSOCK:
				// The descriptor is not a socket. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INVALID_SOCKET_DESCRIPTOR );
				break;
			case WSAEINVAL:
				// The listen function was not invoked prior to accept.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_MUST_LISTEN_BEFORE_ACCEPT );
				break;
			case WSAEOPNOTSUPP:
				// The referenced socket is not of a type that supports connection-oriented service.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKET_TYPE_FOR_LISTEN );
				break;
			case WSAEWOULDBLOCK:
				// The socket is marked as nonblocking and no connections are present to be accepted.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_OPERATION_WOULD_BLOCK );
				break;
			default:
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_UNKNOWN );
				break;
			}
		}
	return bNoError;
}


BOOL WindowsSocketReceive( SOCKET SocketDescriptor, char *pDataBuffer, int BufferLength, int Flags, int *pnBytesRead, BOOL bErrorOnDisconnect )
{
	// The recv function receives data from a connected or bound socket.
	//
	//		int recv(
	//				SOCKET		s,			// [in] Descriptor identifying a connected socket.
	//				char		*buf,		// [out] Buffer for the incoming data.
	//				int			len,		// [in] Length of buf, in bytes.
	//				int			flags		// [in] Flag specifying the way in which the call is made.
	//				);
	//
	// Return Values:  If no error occurs, recv() returns the number of bytes received. If the connection
	// has been gracefully closed, the return value is zero. Otherwise, a value of SOCKET_ERROR is
	// returned, and a specific error code can be retrieved by calling WSAGetLastError().
	//
	// The recv() function is used to read incoming data on connection-oriented sockets, or
	// connectionless sockets. When using a connection-oriented protocol, the sockets must be
	// connected before calling recv. When using a connectionless protocol, the sockets must be
	// bound before calling recv.
	//
	// The local address of the socket must be known. For server applications, use an explicit bind()
	// function or an implicit accept() or WSAAccept() function. Explicit binding is discouraged for
	// client applications. For client applications, the socket can become bound implicitly to a
	// local address using connect(), WSAConnect(), sendto(), WSASendTo(), or WSAJoinLeaf().
	//
	// For connected or connectionless sockets, the recv() function restricts the addresses from which
	// received messages are accepted. The function only returns messages from the remote address
	// specified in the connection. Messages from other addresses are (silently) discarded.
	//
	// For connection-oriented sockets (type SOCK_STREAM for example), calling recv() will return as
	// much information as is currently available—up to the size of the buffer specified. If the
	// socket has been configured for in-line reception of OOB data (socket option SO_OOBINLINE)
	// and OOB data is yet unread, only OOB data will be returned. The application can use the
	// ioctlsocket() or WSAIoctl() SIOCATMARK command to determine whether any more OOB data remains
	// to be read.
	//
	// For connectionless sockets (type SOCK_DGRAM or other message-oriented sockets), data is
	// extracted from the first enqueued datagram (message) from the destination address specified
	// by the connect function.  If the datagram or message is larger than the buffer specified,
	//  the buffer is filled with the first part of the datagram, and recv generates the error
	// WSAEMSGSIZE. For unreliable protocols (for example, UDP) the excess data is lost; for
	// reliable protocols, the data is retained by the service provider until it is successfully
	// read by calling recv with a large enough buffer.
	//
	// If no incoming data is available at the socket, the recv() call blocks and waits for data to
	// arrive according to the blocking rules defined for WSARecv with the MSG_PARTIAL flag not
	// set unless the socket is nonblocking. In this case, a value of SOCKET_ERROR is returned
	// with the error code set to WSAEWOULDBLOCK. The select(), WSAAsyncSelect(), or WSAEventSelect()
	// functions can be used to determine when more data arrives.
	//
	// If the socket is connection oriented and the remote side has shut down the connection
	// gracefully, and all data has been received, a recv() will complete immediately with zero
	// bytes received. If the connection has been reset, a recv() will fail with the error WSAECONNRESET.
	//
	// The flags parameter can be used to influence the behavior of the function invocation beyond
	// the options specified for the associated socket. The semantics of this function are
	// determined by the socket options and the flags parameter. The latter is constructed by
	// using the bitwise OR operator with any of the following values.
	//
	//		MSG_PEEK		Peeks at the incoming data. The data is copied into the buffer but is not
	//						removed from the input queue. The function subsequently returns the amount
	//						of data that can be read in a single call to the recv (or recvfrom)
	//						function, which may not be the same as the total amount of data queued on
	//						the socket. The amount of data that can actually be read in a single call
	//						to the recv (or recvfrom) function is limited to the data size written in
	//						the send or sendto function call. 
	//		MSG_OOB			Processes OOB data. (See DECnet Out-of-band data for a discussion of this
	//						topic.) 
	//
	BOOL			bNoError = TRUE;
	int				WinSockErrorCode;

	*pnBytesRead = recv( SocketDescriptor, pDataBuffer, BufferLength, Flags );
	if ( *pnBytesRead == SOCKET_ERROR )
		{
		bNoError = FALSE;
		WinSockErrorCode = WSAGetLastError();
		switch ( WinSockErrorCode )
			{
			case WSANOTINITIALISED:
				// A successful WSAStartup call must occur before using this function.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NO_WSASTARTUP );
				break;
			case WSAENETDOWN:
				// The network subsystem has failed.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NETWORK_FAILED );
				break;
			case WSAEINPROGRESS:
				// A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_CALLBACK_IN_PROGRESS );
				break;
			case WSAEINVAL:
				// The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a
				// socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKET_NOT_BOUND );
				break;
			case WSAENOTSOCK:
				// The descriptor is not a socket.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INVALID_SOCKET_DESCRIPTOR );
				break;
			case WSAEOPNOTSUPP:
				// MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not
				// supported in the communication domain associated with this socket, or the socket is unidirectional
				// and supports only send operations.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKET_OP_NOT_SUPPORTED );
				break;
			case WSAEFAULT:
				// The buf parameter is not completely contained in a valid part of the user address space. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INVALID_PARAMETER );
				break;
			case WSAENOTCONN:
				// The socket is not connected (connection-oriented sockets only).
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKET_NOT_CONNECTED );
				break;
			case WSAEINTR:
				// The (blocking) call was canceled through WSACancelBlockingCall.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKET_ALREADY_CANCELLED );
				break;
			case WSAENETRESET:
				// The connection has been broken due to the keep-alive activity detecting a failure while the
				// operation was in progress.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_KEEPALIVE_TIMEOUT );
				break;
			case WSAESHUTDOWN:
				// The socket has been shut down; it is not possible to receive on a socket after shutdown has
				// been invoked with how set to SD_RECEIVE or SD_BOTH.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_RECEIVE_AFTER_SHUTDOWN );
				break;
			case WSAEWOULDBLOCK:
				// The socket is marked as nonblocking and the requested operation would block.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_OPERATION_WOULD_BLOCK );
				break;
			case WSAEMSGSIZE:
				// The message was too large to fit into the specified buffer and was truncated.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_MESSAGE_TRUNCATED );
				break;
			case WSAECONNABORTED:
				// The virtual circuit was terminated due to a time-out or other failure. The application should
				// close the socket as it is no longer usable.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_CONNECTION_ABORTED );
				break;
			case WSAETIMEDOUT:
				// The connection has been dropped because of a network failure or because the peer system failed to respond.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_RECEIVE_TIMEOUT );
				break;
			case WSAECONNRESET:
				// The virtual circuit was reset by the remote side executing a hard or abortive close. The application
				// should close the socket as it is no longer usable. On a UPD-datagram socket this error would indicate
				// that a previous send operation resulted in an ICMP "Port Unreachable" message. 
				if ( bErrorOnDisconnect )
	 				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_CONNECTION_RESET );
				break;
			default:
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_UNKNOWN );
				break;
			}
		}
	return bNoError;
}


BOOL WindowsSocketSend( SOCKET SocketDescriptor, char *pDataBuffer, int BufferLength, int Flags, int *pnBytesSent )
{
	// The send function sends data on a connected socket.
	//
	//		int send(
	//				SOCKET			s,		// [in] Descriptor identifying a connected socket.
	//				const char*		buf,	// [in] Buffer containing the data to be transmitted.
	//				int				len,	// [in] Length of the data in buf, in bytes.
	//				int				flags	// [in] Indicator specifying the way in which the call is made.
	//				);
	//
	// Return Values:  If no error occurs, send() returns the total number of bytes sent, which can be less than the number
	// indicated by len.  Otherwise, a value of SOCKET_ERROR is returned, and a specific error code can be retrieved by
	// calling WSAGetLastError().
	//
	// 
	BOOL			bNoError = TRUE;
	int				WinSockErrorCode;

	*pnBytesSent = send( SocketDescriptor, pDataBuffer, BufferLength, Flags );
	if ( *pnBytesSent == SOCKET_ERROR )
		{
		bNoError = FALSE;
		WinSockErrorCode = WSAGetLastError();
		switch ( WinSockErrorCode )
			{
			case WSANOTINITIALISED:
				// A successful WSAStartup call must occur before using this function.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NO_WSASTARTUP );
				break;
			case WSAENETDOWN:
				// The network subsystem has failed.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NETWORK_FAILED );
				break;
			case WSAEINPROGRESS:
				// A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_CALLBACK_IN_PROGRESS );
				break;
			case WSAEINVAL:
				// The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a
				// socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKET_NOT_BOUND );
				break;
			case WSAENOTSOCK:
				// The descriptor is not a socket.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INVALID_SOCKET_DESCRIPTOR );
				break;
			case WSAEOPNOTSUPP:
				// MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not
				// supported in the communication domain associated with this socket, or the socket is unidirectional
				// and supports only receive operations.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKET_OP_NOT_SUPPORTED );
				break;
			case WSAEFAULT:
				// The buf parameter is not completely contained in a valid part of the user address space. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INVALID_PARAMETER );
				break;
			case WSAENOTCONN:
				// The socket is not connected (connection-oriented sockets only).
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKET_NOT_CONNECTED );
				break;
			case WSAEINTR:
				// A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKET_ALREADY_CANCELLED );
				break;
			case WSAENETRESET:
				// The connection has been broken due to the keep-alive activity detecting a failure while the
				// operation was in progress.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_KEEPALIVE_TIMEOUT );
				break;
			case WSAESHUTDOWN:
				// The socket has been shut down; it is not possible to send on a socket after shutdown has been
				// invoked with how set to SD_SEND or SD_BOTH.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SEND_AFTER_SHUTDOWN );
				break;
			case WSAEWOULDBLOCK:
				// The socket is marked as nonblocking and the requested operation would block.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_OPERATION_WOULD_BLOCK );
				break;
			case WSAEMSGSIZE:
				// The message was too large to fit into the specified buffer and was truncated.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_MESSAGE_TRUNCATED );
				break;
			case WSAECONNABORTED:
				// The virtual circuit was terminated due to a time-out or other failure. The application should
				// close the socket as it is no longer usable. 
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_CONNECTION_ABORTED );
				break;
			case WSAETIMEDOUT:
				// The connection has been dropped, because of a network failure or because the system on the other
				// end went down without notice.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_CONNECTION_TIMEOUT );
				break;
			case WSAECONNRESET:
				// The virtual circuit was reset by the remote side executing a hard or abortive close. The application
				// should close the socket as it is no longer usable. On a UPD-datagram socket this error would indicate
				// that a previous send operation resulted in an ICMP "Port Unreachable" message. 
 				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_CONNECTION_RESET );
				break;
			case WSAEACCES:
				// The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt
				// with the SO_BROADCAST parameter to allow the use of the broadcast address.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_BROADCAST_OPTION_NOT_SET );
				break;
			case WSAENOBUFS:
				// No buffer space is available for sending.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NO_SEND_BUFFERS );
				break;
			case WSAEHOSTUNREACH:
				// The remote host cannot be reached from this host at this time.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_REMOTE_HOST_UNREACHABLE );
				break;
			default:
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_UNKNOWN );
				break;
			}
		}
	return bNoError;
}


BOOL WindowsSocketShutdown( SOCKET SocketDescriptor, int TypeOfShutdown )
{
	// The shutdown() function disables sends or receives on a socket.
	//
	//		int shutdown(
	//					SOCKET		s,		// [in] Descriptor identifying a socket.
	//					int			how		// [in] Flag that describes what types of operation will no longer be allowed.
	//					);
	//
	// Return Values:  If no error occurs, shutdown returns zero. Otherwise, a value of SOCKET_ERROR is returned, and
	// a specific error code can be retrieved by calling WSAGetLastError().
	//
	// The shutdown function is used on all types of sockets to disable reception, transmission, or both.  If the how
	// parameter is SD_RECEIVE, subsequent calls to the recv function on the socket will be disallowed. This has no effect
	// on the lower protocol layers. For TCP sockets, if there is still data queued on the socket waiting to be received,
	// or data arrives subsequently, the connection is reset, since the data cannot be delivered to the user. For UDP
	// sockets, incoming datagrams are accepted and queued. In no case will an ICMP error packet be generated.
	//
	// If the how parameter is SD_SEND, subsequent calls to the send function are disallowed. For TCP sockets, a FIN will
	// be sent after all data is sent and acknowledged by the receiver.
	//
	// Setting how to SD_BOTH disables both sends and receives as described above.
	// The shutdown function does not close the socket. Any resources attached to the socket will not be freed until
	// closesocket is invoked.  To assure that all data is sent and received on a connected socket before it is closed,
	// an application should use shutdown to close the connection before calling closesocket. For example, to initiate
	// a graceful disconnect:
	//
	// Call WSAAsyncSelect() to register for FD_CLOSE notification.
	// Call shutdown() with how = SD_SEND.
	// When FD_CLOSE received, call recv() until zero returned, or SOCKET_ERROR.
	// Call closesocket(). 
	//
	// Note  The shutdown function does not block regardless of the SO_LINGER setting on the socket.  An application
	// should not rely on being able to reuse a socket after it has been shut down. In particular, a Windows Sockets
	// provider is not required to support the use of connect on a socket that has been shut down.
	//
	BOOL			bNoError = TRUE;
	int				WinSockErrorCode;

	if ( shutdown( SocketDescriptor, TypeOfShutdown ) == SOCKET_ERROR )
		{
		bNoError = FALSE;
		WinSockErrorCode = WSAGetLastError();
		switch ( WinSockErrorCode )
			{
			case WSANOTINITIALISED:
				// A successful WSAStartup call must occur before using this function.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NO_WSASTARTUP );
				break;
			case WSAENETDOWN:
				// The network subsystem has failed.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NETWORK_FAILED );
				break;
			case WSAEINPROGRESS:
				// A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_CALLBACK_IN_PROGRESS );
				break;
			case WSAEINVAL:
				// The how parameter is not valid, or is not consistent with the socket type. For example, SD_SEND is
				// used with a UNI_RECV socket type.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INCONSISTENT_SOCKET_TYPE );
				break;
			case WSAENOTSOCK:
				// The descriptor is not a socket.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INVALID_SOCKET_DESCRIPTOR );
				break;
			case WSAENOTCONN:
				// The socket is not connected (connection-oriented sockets only).
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKET_NOT_CONNECTED );
				break;
			default:
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_UNKNOWN );
				break;
			}
		}
	return bNoError;
}


BOOL WindowsCloseSocket( SOCKET SocketDescriptor )
{
	// The closesocket() function closes an existing socket.
	//		int  closesocket(
	//						SOCKET		s		// [in] Descriptor identifying the socket to close.
	//						);
	//
	// Return Values:  If no error occurs, closesocket() returns zero. Otherwise, a value of SOCKET_ERROR is returned,
	//					and a specific error code can be retrieved by calling WSAGetLastError().
	//
	// Use closesocket() to release the socket descriptor s so that further references to s fail with the error WSAENOTSOCK.
	// If this is the last reference to an underlying socket, the associated naming information and queued data are discarded
	// Any pending blocking, asynchronous calls issued by any thread in this process are canceled without posting any
	// notification messages.
	//
	// Any pending overlapped send and receive operations ( WSASend/ WSASendTo/ WSARecv/ WSARecvFrom with an overlapped
	// socket) issued by any thread in this process are also canceled. Any event, completion routine, or completion port
	// action specified for these overlapped operations is performed. The pending overlapped operations fail with the error
	// status WSA_OPERATION_ABORTED.
	//
	// An application should always have a matching call to closesocket() for each successful call to socket() to return any
	// socket resources to the system.  The semantics of closesocket() are affected by the socket options SO_LINGER and
	// SO_DONTLINGER as follows (SO_DONTLINGER is enabled by default; SO_LINGER is disabled).
	//
	//		Option			Interval		Type of close	Wait for close? 
	//
	//		SO_DONTLINGER	Do not care		Graceful		No 
	//		SO_LINGER		Zero			Hard			No 
	//		SO_LINGER		Nonzero			Graceful		Yes
	//
	// If SO_LINGER is set with a zero time-out interval (that is, the linger structure members l_onoff is not zero and
	// l_linger is zero), closesocket is not blocked even if queued data has not yet been sent or acknowledged. This is
	// called a hard or abortive close, because the socket's virtual circuit is reset immediately, and any unsent data
	// is lost. Any recv call on the remote side of the circuit will fail with WSAECONNRESET.
	//
	// If SO_LINGER is set with a nonzero time-out interval on a blocking socket, the closesocket call blocks on a blocking
	// socket until the remaining data has been sent or until the time-out expires. This is called a graceful disconnect.
	// If the time-out expires before all data has been sent, the Windows Sockets implementation terminates the connection
	// before closesocket returns.
	//
	// Enabling SO_LINGER with a nonzero time-out interval on a nonblocking socket is not recommended. In this case, the
	// call to closesocket will fail with an error of WSAEWOULDBLOCK if the close operation cannot be completed immediately.
	// If closesocket fails with WSAEWOULDBLOCK the socket handle is still valid, and a disconnect is not initiated. The
	// application must call closesocket again to close the socket. If SO_DONTLINGER is set on a stream socket by setting
	// the l_onoff member of the LINGER structure to zero, the closesocket call will return immediately and does not receive
	// WSAEWOULDBLOCK whether the socket is blocking or nonblocking. However, any data queued for transmission will be sent,
	// if possible, before the underlying socket is closed. This is also called a graceful disconnect. In this case, the
	// Windows Sockets provider cannot release the socket and other resources for an arbitrary period, thus affecting
	// applications that expect to use all available sockets. This is the default behavior (SO_DONTLINGER is set by default).
	//
	// Note:  To assure that all data is sent and received on a connection, an application should call shutdown before
	// calling closesocket (see Graceful shutdown, linger options, and socket closure for more information). Also note,
	// an FD_CLOSE network event is not posted after closesocket is called.
	//
	BOOL			bNoError = TRUE;
	int				WinSockErrorCode;

	if ( closesocket( SocketDescriptor ) == SOCKET_ERROR )
		{
		bNoError = FALSE;
		WinSockErrorCode = WSAGetLastError();
		switch ( WinSockErrorCode )
			{
			case WSANOTINITIALISED:
				// A successful WSAStartup call must occur before using this function.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NO_WSASTARTUP );
				break;
			case WSAENETDOWN:
				// The network subsystem has failed.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_NETWORK_FAILED );
				break;
			case WSAEINPROGRESS:
				// A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_CALLBACK_IN_PROGRESS );
				break;
			case WSAENOTSOCK:
				// The descriptor is not a socket.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_INVALID_SOCKET_DESCRIPTOR );
				break;
			case WSAEINTR:
				// The (blocking) Windows Socket 1.1 call was canceled through WSACancelBlockingCall.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_SOCKET_ALREADY_CANCELLED );
				break;
			case WSAEWOULDBLOCK:
				// The socket is marked as nonblocking and SO_LINGER is set to a nonzero time-out value.
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_LINGER_TIMEOUT_VALUE );
				break;
			default:
				RespondToError( MODULE_WINSOCKAPI, WINSOCKAPI_ERROR_UNKNOWN );
				break;
			}
		}
	return bNoError;
}


// Gracefully close the connection on SocketDescriptor.
//
BOOL CloseConnection( SOCKET SocketDescriptor )
{
	BOOL			bNoError = TRUE;
	int				ReadBufferLength = 256;
	char			ReadBuffer[ 256 ];
	char			TextMessage[ 256 ];
	int				nBytesRead;
	int				nTotalBytesRead;
	BOOL			bShutdownAcknowledgeReceived;

	// Disallow any further data sends.  This informs the remote host that we want
	// to terminate.  If this step is skipped, the shutdown is not done nicely.
	bNoError = WindowsSocketShutdown( SocketDescriptor, SD_SEND );

	if ( bNoError )
		{
		// Receive any extra data still sitting on the socket.  After all data are received, this call will block
		// until the remote host acknowledges the TCP control packet sent by the shutdown above.  Then recv will
		// return 0, signalling that the remote host has closed its side of the connection.
		bShutdownAcknowledgeReceived = FALSE;
		nTotalBytesRead = 0;
		while ( bNoError && !bShutdownAcknowledgeReceived )
			{
			nBytesRead = 0;
			bNoError = WindowsSocketReceive( SocketDescriptor, ReadBuffer, ReadBufferLength, 0, &nBytesRead, FALSE );
			if ( bNoError )
				{
				if ( nBytesRead > 0 )
					nTotalBytesRead += nBytesRead;
				else
					bShutdownAcknowledgeReceived = TRUE;
				}
			else
				bShutdownAcknowledgeReceived = TRUE;
			}
		if ( nTotalBytesRead > 0 )
			{
			sprintf( TextMessage, "  %d unexpected bytes received during socket shutdown.", nTotalBytesRead );
			LogMessage( TextMessage, MESSAGE_TYPE_NORMAL_LOG );
			}
		}

	// Close the socket.
	bNoError = WindowsCloseSocket( SocketDescriptor );

	return bNoError;
}



