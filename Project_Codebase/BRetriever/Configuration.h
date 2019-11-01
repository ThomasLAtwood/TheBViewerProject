// Configuration.h : Defines the functions and data structures that handle 
// the BRetriever service configuration.
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

#define CONFIG_ERROR_INSUFFICIENT_MEMORY			1
#define CONFIG_ERROR_OPEN_CFG_FILE					2
#define CONFIG_ERROR_READ_CFG_FILE					3
#define CONFIG_ERROR_PARSE_ALLOCATION				4
#define CONFIG_ERROR_PARSE_ATTRIB_VALUE				5
#define CONFIG_ERROR_PARSE_UNKNOWN_ATTR				6
#define CONFIG_ERROR_PARSE_TIME						7
#define CONFIG_ERROR_PARSE_ENDPOINT_TYPE			8

#define CONFIG_ERROR_DICT_LENGTH					8


typedef BOOL ( *PRODUCT_DELETE_FUNCTION )( VOID *pProductItem );


typedef struct
	{
	unsigned long				ProcessingStatus;
									#define PRODUCT_STATUS_NOT_SET					0x00000000
									#define PRODUCT_STATUS_INITIALIZATION_COMPLETE	0x00000001	// The product is in the queue.
									#define PRODUCT_STATUS_ITEM_QUEUED				0x00000002
									#define PRODUCT_STATUS_ITEM_BEING_PROCESSED		0x00000004
									#define PRODUCT_STATUS_ABSTRACTS_COMPLETED		0x00000008	// The extraction of data abstracts has completed.
									#define PRODUCT_STATUS_EXTRACTION_COMPLETED		0x00000010	// The images have been successfully extracted.
									#define PRODUCT_STATUS_SOURCE_DELETABLE			0x00000020	// The product source file is ready for deletion.
									#define PRODUCT_STATUS_SOURCE_DELETED			0x00000040	// The product source file was deleted.
									#define PRODUCT_STATUS_IMAGE_EXTRACT_SINGLE		0x00000080	// The product is a single file and not necessarily a full study.
									#define PRODUCT_STATUS_STUDY					0x00000100	// The product is a study with one or more associated images.  Each
																								//  image will also have its own PRODUCT_QUEUE_ITEM without this
																								//  bit set.
									#define PRODUCT_STATUS_RECEIVE_ERROR			0x00000200
									#define PRODUCT_STATUS_IMAGE_EXTRACTION_ERROR	0x00000400
									#define PRODUCT_STATUS_ERROR_NOTIFY_USER		0x00000800
									#define PRODUCT_STATUS_USER_HAS_BEEN_NOTIFIED	0x00001000

	unsigned long				ModuleWhereErrorOccurred;
	unsigned long				FirstErrorCode;
	char						SourceFileName[ MAX_FILE_SPEC_LENGTH ];
	char						SourceFileSpec[ MAX_FILE_SPEC_LENGTH ];
	char						DestinationFileName[ MAX_FILE_SPEC_LENGTH ];
	char						Description[ MAX_FILE_SPEC_LENGTH ];
	unsigned long				LocalProductIndex;
	unsigned long				ComponentCount;							// For studies, this is the number of images currently stored for this study.
																		//	Only valid if PRODUCT_STATUS_STUDY is set.
	void						*pParentProduct;						// Only valid if PRODUCT_STATUS_STUDY is NOT set.
	time_t						ArrivalTime;
	time_t						LatestActivityTime;
	void						*pProductOperation;
	void						*pProductInfo;							// Pointer to structure of type EXAM_INFO, DICTATION_INFO, etc.
	} PRODUCT_QUEUE_ITEM;


typedef struct _EndPoint
	{
	char					Name[ MAX_CFG_STRING_LENGTH ];
	unsigned long			EndPointType;
								#define ENDPOINT_TYPE_UNSPECIFIED		0
								#define ENDPOINT_TYPE_FILE				1
								#define ENDPOINT_TYPE_NETWORK			2
	char					NetworkAddress[ MAX_CFG_STRING_LENGTH ];
	char					AE_TITLE[ MAX_CFG_STRING_LENGTH ];
	char					Directory[ MAX_CFG_STRING_LENGTH ];
	// If the following boolean member is TRUE, the transfer syntax is obtained from
	// the file metadata or, in the case of network transfers, from the accepted
	// presentation context.
	//
	// In cases where this information is not reliable, (which, given the absurd
	// complexity of the Dicom standard for network transfers, occasionaly happens) the
	// boolean member can be set to FALSE.  In this case, the transfer syntax is 
	// deduced from the actual file data, as follows:  If VR is present, the syntax is
	// EXPLICIT_VR, otherwise IMPLICIT_VR.  Almost every computer is LITTLE_ENDIAN,
	// anyway.  And the size of the encapsulated data, if uncompressed, will equal the
	// product of the raster length in bytes times the number of rasters, assuming this
	// information is included in the file and is valid.  If compressed, it is probably
	// JPEG, and the JPEG library will figure out which variety.
	BOOL					bTrustSpecifiedTransferSyntax;
	BOOL					bApplyManualDicomEdits;
	struct _EndPoint		*pNextEndPoint;
	} ENDPOINT;


typedef unsigned ( __stdcall *OPERATION_THREAD_FUNCTION )( VOID *pProductOperation );


typedef struct
	{
	unsigned long				StatusCode;
									#define OPERATION_STATUS_UNKNOWN					0x00000000
									#define	OPERATION_STATUS_RUNNING					0x00000001
									#define	OPERATION_STATUS_CONNECTED					0x00000002
									#define OPERATION_STATUS_TERMINATION_REQUESTED		0x00000004
									#define OPERATION_STATUS_QUEUE_ACCESS_IN_PROGRESS	0x00000008
									#define OPERATION_STATUS_DUPLICATE_PRODUCT_FOUND	0x00000010
	unsigned long				DirectorySearchLevel;	// Increments by one for each encountered subdirectory in depth.
	PRODUCT_QUEUE_ITEM			*pProductItem;
	BOOL						bOKtoProcessThisStudy;
	HANDLE						hOperationThreadHandle;
    unsigned					OperationThreadID;
	OPERATION_THREAD_FUNCTION	ThreadFunction;
	SOCKET						SocketDescriptor;
	void						*pDicomAssociation;	// Pointer to the DICOM_ASSOCIATION structure, if one is established
													//  for this operation.  CAUTION!  Referencing association parameters
													//  outside the state machine environment must be done without
													//  assumptions about the current state of the association.
	HANDLE						hSleepSemaphore;
	char						SleepSemaphoreName[ MAX_CFG_STRING_LENGTH ];
	} OPERATION_STATE;


typedef struct _ProductOperation
	{
	char						OperationName[ MAX_FILE_SPEC_LENGTH ];
	unsigned short				OperationType;
									#define OPERATION_TYPE_UNSPECIFIED				0x0000
									#define OPERATION_TYPE_RECEIVE_FROM_FOLDER		0x0001
									#define OPERATION_TYPE_LISTEN_TO_NETWORK		0x0002
									#define OPERATION_TYPE_RECEIVE_FROM_NETWORK		0x0003
									#define OPERATION_TYPE_DISPATCH_FROM_QUEUE		0x0004
									#define OPERATION_TYPE_STORE_IN_LOCAL_FOLDER	0x0005
									#define OPERATION_TYPE_SEND_OVER_NETWORK		0x0006
	time_t						OperationTimeInterval;
	ENDPOINT					*pInputEndPoint;
	BOOL						bInputDeleteSourceOnCompletion;
	ENDPOINT					*pOutputEndPoint;
	BOOL						bEnabled;
	char						DependentOperationName[ MAX_CFG_STRING_LENGTH ];
	struct _ProductOperation	*pDependentOperation;
	struct _ProductOperation	*pNextOperation;
	OPERATION_STATE				OpnState;
	} PRODUCT_OPERATION;




// Function prototypes.
//
void				InitConfigurationModule();
void				CloseConfigurationModule();
ENDPOINT			*CreateEndPoint();
BOOL				ReadConfigurationFile( char *pConfigurationDirectory, char *pConfigurationFileName );
void				InitConfiguration();
void				InitializeOperationConfiguration();
BOOL				ParseEndPointConfigurationLine( char *pTextLine, ENDPOINT *pEndPoint );
BOOL				ParseConfigurationLine( char *pTextLine );
BOOL				ParseOperationConfigurationLine( char *pTextLine, PRODUCT_OPERATION *pProductOperation );
ENDPOINT			*LookUpEndPointAddress( char *pEndPointName );
void				EraseProductOperationList();
void				EraseEndPointList( ENDPOINT **ppEndPointList );


