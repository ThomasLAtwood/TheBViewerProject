// ProductDispatcher.cpp : Implements the data structures and functions related to
//	the product queue activities.
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
#include "ReportStatus.h"
#include "Dicom.h"
#include "ServiceMain.h"
#include "Configuration.h"
#include "Operation.h"
#include "Exam.h"
#include "ProductDispatcher.h"
#include "ExamReformat.h"


//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		DispatcherModuleInfo = { MODULE_DISPATCH, "Product Dispatcher Module", InitProductDispatcherModule, CloseProductDispatcherModule };


static ERROR_DICTIONARY_ENTRY	ProductDispatcherErrorCodes[] =
			{
				{ DISPATCH_ERROR_INSUFFICIENT_MEMORY		, "There is not enough memory to allocate a data structure." },
				{ DISPATCH_ERROR_CREATE_PRODUCT_SEMAPHORE	, "An error occurred creating the product queue semaphore." },
				{ DISPATCH_ERROR_PRODUCT_SEMAPHORE_TIMEOUT	, "A timeout occurred waiting for the product queue semaphore." },
				{ DISPATCH_ERROR_PRODUCT_SEMAPHORE_WAIT		, "An undiagnosed error occurred while waiting for the product queue semaphore." },
				{ DISPATCH_ERROR_PRODUCT_SEMAPHORE_RELEASE	, "An error occurred releasing the product queue semaphore." },
				{ 0											, NULL }
			};

static ERROR_DICTIONARY_MODULE			ProductDispatcherStatusErrorDictionary =
											{
											MODULE_DISPATCH,
											ProductDispatcherErrorCodes,
											DISPATCH_ERROR_DICT_LENGTH,
											0
											};

extern TRANSFER_SERVICE					TransferService;

LIST_HEAD								ProductQueue;
HANDLE									hProductQueueSemaphore = 0;
static char								*pProductSemaphoreName = "BRetrieverProductQueueSemaphore";
static PRODUCT_OPERATION				*pProductDeletionOperation = 0;
static unsigned long					LocalProductID = 0;


// This function must be called before any other function in this module.
void InitProductDispatcherModule()
{
	LinkModuleToList( &DispatcherModuleInfo );
	RegisterErrorDictionary( &ProductDispatcherStatusErrorDictionary );
	ProductQueue = 0;
	// Create a semaphore for controlling access to the product queue from
	// different (competing) threads.
	hProductQueueSemaphore = CreateSemaphore( NULL, 1L, 1L, pProductSemaphoreName );
	if ( hProductQueueSemaphore == NULL )
		RespondToError( MODULE_DISPATCH, DISPATCH_ERROR_CREATE_PRODUCT_SEMAPHORE );
	CreateProductDeletionOperation();
}


// This function must be called to deallocate memory and close this module.
void CloseProductDispatcherModule()
{
	DeleteProductOperation( pProductDeletionOperation );
	pProductDeletionOperation = 0;
	CloseHandle( hProductQueueSemaphore );
}


void InitProductInfoStructure( PRODUCT_QUEUE_ITEM *pProductItem )
{
	pProductItem -> ProcessingStatus = PRODUCT_STATUS_NOT_SET;
	pProductItem -> ModuleWhereErrorOccurred = 0L;
	pProductItem -> FirstErrorCode = 0L;
	pProductItem -> SourceFileName[ 0 ] = '\0';					// *[1] Eliminate call to strcpy.
	pProductItem -> SourceFileSpec[ 0 ] = '\0';					// *[1] Eliminate call to strcpy.
	pProductItem -> DestinationFileName[ 0 ] = '\0';			// *[1] Eliminate call to strcpy.
	pProductItem -> Description[ 0 ] = '\0';					// *[1] Eliminate call to strcpy.
	pProductItem -> LocalProductIndex = 0L;
	pProductItem -> ComponentCount = 0L;
	pProductItem -> pParentProduct = 0;
	pProductItem -> ArrivalTime = 0L;
	pProductItem -> LatestActivityTime = 0L;
	pProductItem -> pProductOperation = 0;
	pProductItem -> pProductInfo = 0;
}


void CreateProductDeletionOperation()
{
	pProductDeletionOperation = CreateProductOperation();
	if ( pProductDeletionOperation == 0 )
		RespondToError( MODULE_DISPATCH, DISPATCH_ERROR_INSUFFICIENT_MEMORY );
}


BOOL InitNewProductQueueItem( PRODUCT_OPERATION *pProductOperation, PRODUCT_QUEUE_ITEM **ppProductItem )
{
	BOOL					bNoError = TRUE;
	PRODUCT_QUEUE_ITEM		*pProductItem;
	EXAM_INFO				*pExamInfo;

	pProductItem = (PRODUCT_QUEUE_ITEM*)malloc( sizeof(PRODUCT_QUEUE_ITEM) );
	if ( pProductItem == 0 )
		{
		bNoError = FALSE;
		RespondToError( MODULE_DISPATCH, DISPATCH_ERROR_INSUFFICIENT_MEMORY );
		}
	if ( bNoError )
		{
		InitProductInfoStructure( pProductItem );
		pProductItem -> pProductOperation = (void*)pProductOperation;
		pExamInfo = (EXAM_INFO*)malloc( sizeof(EXAM_INFO) );
		if ( pExamInfo == 0 )
			{
			bNoError = FALSE;
			RespondToError( MODULE_DISPATCH, DISPATCH_ERROR_INSUFFICIENT_MEMORY );
			free( pProductItem );					// *[1] Fix potential memory leak.
			}
		else
			{
			InitExamInfoStructure( pExamInfo );
			pProductItem -> pProductInfo = (void*)pExamInfo;
			}
		}
	if ( bNoError )
		{
		pProductItem -> ProcessingStatus = PRODUCT_STATUS_INITIALIZATION_COMPLETE;
		*ppProductItem = pProductItem;
		}

	return bNoError;
}


void DeallocateProductInfo( PRODUCT_QUEUE_ITEM *pProductItem )
{
	BOOL						bThisIsAQueuedInputProduct;
	BOOL						bThisIsAStudyProduct;
	EXAM_INFO					*pExamInfo;
	DICOM_HEADER_SUMMARY		*pDicomHeader;

	if ( pProductItem != 0 )
		{
		pExamInfo = (EXAM_INFO*)pProductItem -> pProductInfo;
		bThisIsAStudyProduct = ( ( pProductItem -> ProcessingStatus & PRODUCT_STATUS_STUDY ) != 0 );
		bThisIsAQueuedInputProduct = !bThisIsAStudyProduct;
		if ( bThisIsAQueuedInputProduct )
			{
			if ( pExamInfo != 0 )
				{
				// This product represents a Dicom image.
				pDicomHeader = pExamInfo -> pDicomInfo;
				if ( pDicomHeader != 0 )
					{
					// Deallocate everything.
					if ( pDicomHeader -> pImageData != 0 )
						{
						free( pDicomHeader -> pImageData );
						pDicomHeader -> pImageData = 0;
						}
					if ( pDicomHeader -> CalibrationInfo.pModalityLUTData != 0 )		// *[1] Fixed memory leak by adding this.
						free( pDicomHeader -> CalibrationInfo.pModalityLUTData );
					if ( pDicomHeader -> CalibrationInfo.pVOI_LUTData != 0 )			// *[1] Fixed memory leak by adding this.
						free( pDicomHeader -> CalibrationInfo.pVOI_LUTData );
					DeallocateInputBuffers( pDicomHeader );
					DeallocateListOfDicomElements( pDicomHeader );
					free( pDicomHeader );
					pDicomHeader = 0;
					pExamInfo -> pDicomInfo = 0;
					DeallocateExamInfoAttributes( pExamInfo );
					free( pExamInfo );
					pExamInfo = 0;
					}
				}
			}
		else if ( bThisIsAStudyProduct )
			{
			if ( pExamInfo != 0 )
				{
				pExamInfo -> pDicomInfo = 0;
				DeallocateExamInfoAttributes( pExamInfo );
				free( pExamInfo );
				pExamInfo = 0;
				}
			}
		free( pProductItem );
		}
}


// Any function which calls this one must access the product queue through
// the semaphore.  The call to this function must be protected by
// semaphore gated access.
PRODUCT_QUEUE_ITEM *GetDuplicateProductQueueEntry( PRODUCT_QUEUE_ITEM *pNewProductItem )
{
	LIST_ELEMENT		*pListElement;
	PRODUCT_QUEUE_ITEM	*pProductItem;
	BOOL				bDuplicateFound;
	
	bDuplicateFound = FALSE;
	pListElement = ProductQueue;
	while ( !bDuplicateFound && pListElement != 0 )
		{
		pProductItem = (PRODUCT_QUEUE_ITEM*)pListElement -> pItem;
		if ( _stricmp( pProductItem -> SourceFileName, pNewProductItem -> SourceFileName ) == 0 &&
					( pProductItem -> ProcessingStatus & PRODUCT_STATUS_STUDY ) == ( pNewProductItem -> ProcessingStatus & PRODUCT_STATUS_STUDY ) )
			bDuplicateFound = TRUE;
		else
			pListElement = pListElement -> pNextListElement;
		}
	if ( !bDuplicateFound )
		pProductItem = 0;

	return pProductItem;
}


// Called from the thread dedicated to the product Operation making the queue request.
BOOL QueueProductForTransfer( PRODUCT_OPERATION *pProductOperation, PRODUCT_QUEUE_ITEM **ppProductItem, PRODUCT_QUEUE_ITEM **ppDuplicateProductItem )
{
	BOOL				bNoError = TRUE;
	time_t				CurrentSystemTime;
	DWORD				WaitResponse;
	PRODUCT_QUEUE_ITEM	*pProductItem;
	PRODUCT_QUEUE_ITEM	*pDuplicateProductItem;
	char				TextLine[ MAX_FILE_SPEC_LENGTH ];

	pProductItem = *ppProductItem;
	time( &CurrentSystemTime );
	pProductItem -> ArrivalTime = CurrentSystemTime;
	// Queue the PRODUCT_QUEUE_ITEM.  Remember to deallocate it when finished with it.
	// Access the product queue with semaphore protection.
	WaitResponse = WaitForSingleObject( hProductQueueSemaphore, PRODUCT_QUEUE_ACCESS_TIMEOUT );
	if ( WaitResponse == WAIT_OBJECT_0 )
		{
		pDuplicateProductItem = GetDuplicateProductQueueEntry( pProductItem );
		// If this product is not already in the queue, add it.
		if ( pDuplicateProductItem == 0 )
			{
			LocalProductID++;					// Establish a temporary local identifier for the exam
			if ( LocalProductID == 0 )			// to assist with queue management.
				LocalProductID++;
			pProductItem -> LocalProductIndex = LocalProductID;
			pProductItem -> ProcessingStatus |= PRODUCT_STATUS_ITEM_QUEUED;
			bNoError = AppendToList( &ProductQueue, (void*)pProductItem );
			if ( pProductItem != 0 )
				{
				_snprintf_s( TextLine, MAX_FILE_SPEC_LENGTH, _TRUNCATE,											// *[1] Replaced sprintf() with _snprintf_s.
								"Queue new product: ID = %d, status = %X   %s", pProductItem -> LocalProductIndex,
															pProductItem -> ProcessingStatus, pProductItem -> SourceFileName );
				LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
				}
			*ppDuplicateProductItem = 0;
			}
		else
			{
			LogMessage( "Duplicate entry already queued.", MESSAGE_TYPE_SUPPLEMENTARY );
			*ppDuplicateProductItem = pDuplicateProductItem;
			pProductOperation -> OpnState.pProductItem = 0;
			}
		}
	else if ( WaitResponse == WAIT_TIMEOUT )
		{
		bNoError = FALSE;
		RespondToError( MODULE_DISPATCH, DISPATCH_ERROR_PRODUCT_SEMAPHORE_TIMEOUT );
		}
	else
		{
		bNoError = FALSE;
		RespondToError( MODULE_DISPATCH, DISPATCH_ERROR_PRODUCT_SEMAPHORE_WAIT );
		}
	if ( ReleaseSemaphore( hProductQueueSemaphore, 1L, NULL ) == FALSE )
		{
		bNoError = FALSE;
		RespondToError( MODULE_DISPATCH, DISPATCH_ERROR_PRODUCT_SEMAPHORE_RELEASE );
		}
		
	return bNoError;
}


// Any function which calls this one must access the exam queue through
// the semaphore.  The call to this function must be protected by
// semaphore gated access.
PRODUCT_QUEUE_ITEM *GetFirstProductQueueEntry( unsigned long Status, unsigned long NotStatus )
{
	LIST_ELEMENT		*pListElement;
	PRODUCT_QUEUE_ITEM	*pProductItem;
	BOOL				bSpecifiedProductFound;
	
	bSpecifiedProductFound = FALSE;
	pListElement = ProductQueue;
	while ( !bSpecifiedProductFound && pListElement != 0 )
		{
		pProductItem = (PRODUCT_QUEUE_ITEM*)pListElement -> pItem;
		if ( ( pProductItem -> ProcessingStatus & Status ) != 0 && ( pProductItem -> ProcessingStatus & NotStatus ) == 0 )
			bSpecifiedProductFound = TRUE;
		pListElement = pListElement -> pNextListElement;
		}
	if ( !bSpecifiedProductFound )
		pProductItem = 0;

	return pProductItem;
}


PRODUCT_QUEUE_ITEM *GetFirstQueuedProductByStatus( unsigned long Status, unsigned long NotStatus )
{
	BOOL				bNoError = TRUE;
	PRODUCT_QUEUE_ITEM	*pProductItem;
	DWORD				WaitResponse;
	
	pProductItem = 0;
	// Access the exam queue with semaphore protection.
	WaitResponse = WaitForSingleObject( hProductQueueSemaphore, PRODUCT_QUEUE_ACCESS_TIMEOUT );
	if ( WaitResponse == WAIT_OBJECT_0 )
		{
		// Get first exam with this precise status.
		pProductItem = GetFirstProductQueueEntry( Status, NotStatus );
		}
	else if ( WaitResponse == WAIT_TIMEOUT )
		{
		bNoError = FALSE;
		RespondToError( MODULE_DISPATCH, DISPATCH_ERROR_PRODUCT_SEMAPHORE_TIMEOUT );
		}
	else
		{
		bNoError = FALSE;
		RespondToError( MODULE_DISPATCH, DISPATCH_ERROR_PRODUCT_SEMAPHORE_WAIT );
		}
	if ( ReleaseSemaphore( hProductQueueSemaphore, 1L, NULL ) == FALSE )
		{
		bNoError = FALSE;
		RespondToError( MODULE_DISPATCH, DISPATCH_ERROR_PRODUCT_SEMAPHORE_RELEASE );
		}

	return pProductItem;
}


// Any function which calls this one must access the dictation queue through
// the semaphore.  The call to this function must be protected by
// semaphore gated access.
PRODUCT_QUEUE_ITEM *GetMatchingProductEntry( unsigned long LocalProductIndex )
{
	LIST_ELEMENT		*pListElement;
	PRODUCT_QUEUE_ITEM	*pProductItem;
	BOOL				bSpecifiedDictationFound;
	
	bSpecifiedDictationFound = FALSE;
	pListElement = ProductQueue;
	while ( !bSpecifiedDictationFound && pListElement != 0 )
		{
		pProductItem = (PRODUCT_QUEUE_ITEM*)pListElement -> pItem;
		// Look for the pending flag set and no others.
		if ( pProductItem -> LocalProductIndex == LocalProductIndex )
			bSpecifiedDictationFound = TRUE;
		pListElement = pListElement -> pNextListElement;
		}
	if ( !bSpecifiedDictationFound )
		pProductItem = 0;

	return pProductItem;
}


// Called from the thread dedicated to the product Operation signalling completion of the transfer.
void RemoveProductFromQueue( unsigned long LocalProductIndex )
{
	BOOL					bNoError = TRUE;
	PRODUCT_QUEUE_ITEM		*pProductItem;
	DWORD					WaitResponse;

	// Access the product queue with semaphore protection.
	WaitResponse = WaitForSingleObject( hProductQueueSemaphore, PRODUCT_QUEUE_ACCESS_TIMEOUT );
	if ( WaitResponse == WAIT_OBJECT_0 )
		{
		pProductItem = GetMatchingProductEntry( LocalProductIndex );
		if ( pProductItem != 0 )
			{
			RemoveFromList( &ProductQueue, (void*)pProductItem );
			DeallocateProductInfo( pProductItem );
			}
		}
	else if ( WaitResponse == WAIT_TIMEOUT )
		{
		bNoError = FALSE;
		RespondToError( MODULE_DISPATCH, DISPATCH_ERROR_PRODUCT_SEMAPHORE_TIMEOUT );
		}
	else
		{
		bNoError = FALSE;
		RespondToError( MODULE_DISPATCH, DISPATCH_ERROR_PRODUCT_SEMAPHORE_WAIT );
		}
	if ( ReleaseSemaphore( hProductQueueSemaphore, 1L, NULL ) == FALSE )
		{
		bNoError = FALSE;
		RespondToError( MODULE_DISPATCH, DISPATCH_ERROR_PRODUCT_SEMAPHORE_RELEASE );
		}

	return;
}


void NotifyUserOfProductError( PRODUCT_QUEUE_ITEM *pProductItem )
{
	USER_NOTIFICATION		UserNoticeDescriptor;

	strncpy_s( UserNoticeDescriptor.Source, 16, TransferService.ServiceName, _TRUNCATE );							// *[1] Replaced strcpy with strncpy_s.
	UserNoticeDescriptor.ModuleCode = pProductItem -> ModuleWhereErrorOccurred;
	UserNoticeDescriptor.ErrorCode = pProductItem -> FirstErrorCode;
	UserNoticeDescriptor.TypeOfUserResponseSupported = USER_RESPONSE_TYPE_ERROR | USER_RESPONSE_TYPE_CONTINUE;
	UserNoticeDescriptor.UserNotificationCause = USER_NOTIFICATION_CAUSE_PRODUCT_RECEIVE_ERROR;
	UserNoticeDescriptor.UserResponseCode = 0L;
	_snprintf_s( UserNoticeDescriptor.NoticeText, MAX_FILE_SPEC_LENGTH, _TRUNCATE,									// *[1] Replaced sprintf() with _snprintf_s.
					"The image file\n\n%s\n\nwas not received successfully.", pProductItem -> Description );
	strncpy_s( UserNoticeDescriptor.SuggestedActionText,
				MAX_CFG_STRING_LENGTH, "Try having it resent\nafter waiting a few minutes.\n", _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
	strncat_s( UserNoticeDescriptor.SuggestedActionText,
				MAX_CFG_STRING_LENGTH, "If that doesn't work, request\ntechnical support.", _TRUNCATE );			// *[1] Replaced strcat with strncat_s.
	UserNoticeDescriptor.TextLinesRequired = 9;
	SubmitUserNotification( &UserNoticeDescriptor );
}


unsigned __stdcall ProcessProductQueueThreadFunction( VOID *pOperationStruct )
{
	BOOL						bNoError = TRUE;
	BOOL						bTerminateOperation = FALSE;
	PRODUCT_QUEUE_ITEM			*pProductItem;
	PRODUCT_OPERATION			*pProductOperation;
	EXAM_INFO					*pExamInfo;
	ABSTRACT_RECORD_TEXT_LINE	*pAbstractLineList;
	char						TextLine[ 1096 ];

	pProductOperation = (PRODUCT_OPERATION*)pOperationStruct;

	_snprintf_s( TextLine, 1096, _TRUNCATE, "    Operation Thread: %s", pProductOperation -> OperationName );		// *[1] Replaced sprintf() with _snprintf_s.
	LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
	while ( !bTerminateOperation )
		{
		pProductOperation -> OpnState.DirectorySearchLevel = 0;
		pProductOperation -> OpnState.bOKtoProcessThisStudy = TRUE;
		pProductOperation -> OpnState.pProductItem = 0;
		// For the time being, just process the images one at a time as they
		// are encountered in the queue.
		pProductItem = GetFirstQueuedProductByStatus( PRODUCT_STATUS_ITEM_QUEUED, PRODUCT_STATUS_ITEM_BEING_PROCESSED | PRODUCT_STATUS_STUDY );
		if ( pProductItem != 0 )
			{
			UpdateBRetrieverStatus( BRETRIEVER_STATUS_PROCESSING );
			pProductItem -> ProcessingStatus |= PRODUCT_STATUS_ITEM_BEING_PROCESSED;
			_snprintf_s( TextLine, 1096, _TRUNCATE, "Retrieving %x from the image extraction queue.", (unsigned int)pProductItem );		// *[1] Replaced sprintf() with _snprintf_s.
			LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
			// Extract and reformat the Dicom image contained in the file, so that BViewer
			// can read it.
			bNoError = PerformLocalFileReformat( pProductItem, pProductOperation );
			if ( !bNoError )
				pProductItem -> ProcessingStatus |= PRODUCT_STATUS_IMAGE_EXTRACTION_ERROR;
			if ( ( pProductItem -> ProcessingStatus & PRODUCT_STATUS_IMAGE_EXTRACTION_ERROR ) == 0 )
				pProductItem -> ProcessingStatus |= PRODUCT_STATUS_EXTRACTION_COMPLETED;
			pExamInfo = (EXAM_INFO*)pProductItem -> pProductInfo;
			if ( pExamInfo != 0 && pExamInfo -> pDicomInfo != 0 )
				{
				pAbstractLineList = pExamInfo -> pDicomInfo -> pAbstractDataLineList;
				bNoError = OutputAbstractRecords( pProductItem -> DestinationFileName, pAbstractLineList );
				if ( bNoError )
					{
					// If image file archiving is requested from the configuration file, name the archived file
					// the same as the corresponding .PNG and .AXT files.
					bNoError = ArchiveDicomImageFile( pProductItem -> SourceFileSpec, pProductItem -> DestinationFileName );
					}
				if ( bNoError )
					{
					// If Dicom image output file composition is enabled, compose and save a Dicom output file.
					bNoError = ComposeDicomFileOutput( pProductItem -> SourceFileSpec, pProductItem -> DestinationFileName, pExamInfo );
					}
				}
			bNoError = DeleteSourceProduct( pProductOperation, &pProductItem );
			}
		UpdateBRetrieverStatus( BRETRIEVER_STATUS_ACTIVE );
		EnterOperationCycleWaitInterval( pProductOperation, TRUE, &bTerminateOperation );
		}			// ...end while not bTerminateOperation.
	CloseOperation( pProductOperation );

	return 0;
}


void ProcessProductQueueItems()
{
	BOOL					bNoError = TRUE;
	DWORD					WaitResponse;
	LIST_ELEMENT			*pListElement;
	LIST_ELEMENT			*pPrevListElement;
	PRODUCT_QUEUE_ITEM		*pProductItem;
	PRODUCT_QUEUE_ITEM		*pParentProductItem;
	char					TextLine[ 1096 ];
	PRODUCT_OPERATION		*pProductOperation;
	DWORD					SystemErrorCode;
	time_t					CurrentSystemTime;
	double					ElapsedTimeInSeconds;
	
	#define					QUEUE_TIMEOUT_IN_SECONDS		60.0

	// Access the exam queue with semaphore protection.
	WaitResponse = WaitForSingleObject( hProductQueueSemaphore, PRODUCT_QUEUE_ACCESS_TIMEOUT );
	if ( WaitResponse == WAIT_OBJECT_0 )
		{
		pListElement = ProductQueue;
		pPrevListElement = 0;
		while ( pListElement != 0 )
			{
			pProductItem = (PRODUCT_QUEUE_ITEM*)pListElement -> pItem;
			if ( pProductItem != 0 )
				{
				pProductOperation = 0;
				time( &CurrentSystemTime );
				ElapsedTimeInSeconds = difftime( CurrentSystemTime, pProductItem -> ArrivalTime );
				_snprintf_s( TextLine, 1096, _TRUNCATE,											// *[1] Replaced sprintf() with _snprintf_s.
								"Queue check: ID = %d, status = %X   %s    %5.0f sec.", pProductItem -> LocalProductIndex,
											pProductItem -> ProcessingStatus, pProductItem -> SourceFileName, ElapsedTimeInSeconds );
				LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
				pProductOperation = (PRODUCT_OPERATION*)pProductItem -> pProductOperation;
				//
				// If an error has occurred, notify the user.
				//
				if ( ( pProductItem -> ProcessingStatus & PRODUCT_STATUS_STUDY ) == 0 )
					{
					pParentProductItem = (PRODUCT_QUEUE_ITEM*)pProductItem -> pParentProduct;
					if ( pParentProductItem != 0 )
						pParentProductItem -> pProductOperation = (void*)pProductOperation;
					// If the user needs to be notified of an error with this image file and notification
					// has not yet been initiated, initiate it.
					if ( ( pProductItem -> ProcessingStatus & PRODUCT_STATUS_ERROR_NOTIFY_USER ) != 0 &&
								( pProductItem -> ProcessingStatus & PRODUCT_STATUS_USER_HAS_BEEN_NOTIFIED ) == 0 )
						{
						NotifyUserOfProductError( pProductItem );
						pProductItem -> ProcessingStatus &= ~( PRODUCT_STATUS_RECEIVE_ERROR | PRODUCT_STATUS_ITEM_BEING_PROCESSED );
						pProductItem -> ProcessingStatus |= PRODUCT_STATUS_USER_HAS_BEEN_NOTIFIED;
						if ( pParentProductItem != 0 )
							pParentProductItem -> ProcessingStatus |= PRODUCT_STATUS_USER_HAS_BEEN_NOTIFIED;
						}
					}
				pPrevListElement = pListElement;
				if ( pListElement != 0 )
					pListElement = pListElement -> pNextListElement;
				}
			}			// ... end while
		}
	else if ( WaitResponse == WAIT_TIMEOUT )
		{
		bNoError = FALSE;
		RespondToError( MODULE_DISPATCH, DISPATCH_ERROR_PRODUCT_SEMAPHORE_TIMEOUT );
		}
	else
		{
		bNoError = FALSE;
		RespondToError( MODULE_DISPATCH, DISPATCH_ERROR_PRODUCT_SEMAPHORE_WAIT );
		}
	// Unlock access to the product queue to let others access it during file processing.
	if ( ReleaseSemaphore( hProductQueueSemaphore, 1L, NULL ) == FALSE )
		{
		bNoError = FALSE;
		RespondToError( MODULE_DISPATCH, DISPATCH_ERROR_PRODUCT_SEMAPHORE_RELEASE );
		SystemErrorCode = GetLastError();
		_snprintf_s( TextLine, 1096, _TRUNCATE, "ProcessProductQueueItems semaphore release system error code %d", SystemErrorCode );			// *[1] Replaced sprintf() with _snprintf_s.
		LogMessage( TextLine, MESSAGE_TYPE_ERROR );
		}

	return;
}


BOOL DeleteSourceProduct( PRODUCT_OPERATION *pProductOperation, PRODUCT_QUEUE_ITEM **ppProductItem )
{
	BOOL						bNoError = TRUE;
	PRODUCT_QUEUE_ITEM			*pProductItem;
	PRODUCT_QUEUE_ITEM			*pStudyProductItem;
	char						TextLine[ 1096 ];
	time_t						CurrentSystemTime;
	DWORD						SystemErrorCode;
	
	pProductItem = *ppProductItem;
	SystemErrorCode = 0;
	if ( pProductOperation -> bInputDeleteSourceOnCompletion )
		{
		// Delete the product source file.
		_snprintf_s( TextLine, 1096, _TRUNCATE, "Deleting file:  %s", pProductItem -> SourceFileSpec );				// *[1] Replaced sprintf() with _snprintf_s.
		LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
		if ( !DeleteFile( pProductItem -> SourceFileSpec ) )
			{
			SystemErrorCode = GetLastError();
			_snprintf_s( TextLine, 1096, _TRUNCATE, "Delete image file system error code %d", SystemErrorCode );	// *[1] Replaced sprintf() with _snprintf_s.
			LogMessage( TextLine, MESSAGE_TYPE_ERROR );
			_snprintf_s( TextLine, 1096, _TRUNCATE, "Unable to delete: %s", pProductItem -> SourceFileSpec );		// *[1] Replaced sprintf() with _snprintf_s.
			LogMessage( TextLine, MESSAGE_TYPE_ERROR );
			}
		}
	// Remove the current image product from its parent study.
	pStudyProductItem = (PRODUCT_QUEUE_ITEM*)pProductItem -> pParentProduct;
	pStudyProductItem -> ComponentCount--;
	time( &CurrentSystemTime );
	pStudyProductItem -> LatestActivityTime = CurrentSystemTime;
	// Remove the current image product from the queue.  This will also delete the product
	//  and all its associated structures.
	_snprintf_s( TextLine, 1096, _TRUNCATE, "Removed image file from queue: ID = %d, %s",							// *[1] Replaced sprintf() with _snprintf_s.
							pProductItem -> LocalProductIndex, pProductItem -> SourceFileName );
	LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
	RemoveProductFromQueue( pProductItem -> LocalProductIndex );
	pProductItem = 0;	// The product has was deallocated by the previous function call.
	// If any source folders have been deleted, go ahead and deallocate the study.
	if ( ( pStudyProductItem -> ProcessingStatus & PRODUCT_STATUS_STUDY ) != 0 &&
				pStudyProductItem -> ComponentCount == 0 &&
				( pStudyProductItem -> ProcessingStatus & PRODUCT_STATUS_SOURCE_DELETED  ) != 0 )
		{
		// Remove the associated study product from the queue.  This will delete the product
		//  and all its associated structures.
		_snprintf_s( TextLine, 1096, _TRUNCATE,																		// *[1] Replaced sprintf() with _snprintf_s.
						"Deallocating study at completion of image processing: ID = %d, %s",
								pStudyProductItem -> LocalProductIndex, pStudyProductItem -> SourceFileName );
		LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
		DeallocateProductInfo( pStudyProductItem );
		}
	*ppProductItem = 0;

	return bNoError;
}


