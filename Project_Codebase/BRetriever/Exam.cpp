// Exam.cpp - Implements the data structures and functions related to
//	navigation and processing of Studies/Series/Images.
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
#include <direct.h>
#include "ReportStatus.h"
#include "ServiceMain.h"
#include "Dicom.h"
#include "Abstract.h"
#include "Configuration.h"
#include "Operation.h"
#include "Exam.h"
#include "ProductDispatcher.h"
#include "ExamReformat.h"

extern TRANSFER_SERVICE				TransferService;
extern CONFIGURATION				ServiceConfiguration;
extern ENDPOINT						EndPointWatchFolder;
extern ENDPOINT						EndPointQueue;
extern unsigned long				BRetrieverStatus;


//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO					ExamModuleInfo = { MODULE_EXAM, "Exam Module", InitExamModule, CloseExamModule };


static ERROR_DICTIONARY_ENTRY	ExamErrorCodes[] =
			{
				{ EXAM_ERROR_SOURCE_DIRECTORY				, "The specified image file receiving directory was not found." },
				{ EXAM_ERROR_REMOVE_DIRECTORY				, "A cleanup error occurred removing a study folder." },
				{ EXAM_ERROR_NO_EXAM_INFO					, "No exam information was found for an otherwise processable product." },
				{ EXAM_ERROR_FILE_DELETE					, "An error occurred deleting a source file following a successful copy." },
				{ EXAM_ERROR_DICOM_PARSE					, "An error occurred decoding the study's Dicom image file format." },
				{ EXAM_ERROR_INSUFFICIENT_DISK_SPACE		, "There is insufficient unused storage space available to record any more image files." },
				{ 0											, NULL }
			};



static ERROR_DICTIONARY_MODULE		ExamStatusErrorDictionary =
										{
										MODULE_EXAM,
										ExamErrorCodes,
										EXAM_ERROR_DICT_LENGTH,
										0
										};



// This function must be called before any other function in this module.
void InitExamModule()
{
	LinkModuleToList( &ExamModuleInfo );
	RegisterErrorDictionary( &ExamStatusErrorDictionary );
}


void CloseExamModule()
{
}


void InitExamInfoStructure( EXAM_INFO *pExamInfo )
{
	pExamInfo -> pFirstName = 0;
	pExamInfo -> pLastName = 0;
	pExamInfo -> pExamID = 0;
	pExamInfo -> pAppointmentDate = 0;
	pExamInfo -> pAppointmentTime = 0;
	pExamInfo -> pSeriesNumber = 0;
	pExamInfo -> pSeriesDescription = 0;
	pExamInfo -> pDicomInfo = 0;
}


void DeallocateExamInfoAttributes( EXAM_INFO *pExamInfo )
{
	if ( pExamInfo != 0 )
		{
		if ( pExamInfo -> pFirstName != 0 )
			{
			free( pExamInfo -> pFirstName );
			pExamInfo -> pFirstName = 0;
			}
		if ( pExamInfo -> pLastName != 0 )
			{
			free( pExamInfo -> pLastName );
			pExamInfo -> pLastName = 0;
			}
		if ( pExamInfo -> pExamID != 0 )
			{
			free( pExamInfo -> pExamID );
			pExamInfo -> pExamID = 0;
			}
		if ( pExamInfo -> pAppointmentDate != 0 )
			{
			free( pExamInfo -> pAppointmentDate );
			pExamInfo -> pAppointmentDate = 0;
			}
		if ( pExamInfo -> pAppointmentTime != 0 )
			{
			free( pExamInfo -> pAppointmentTime );
			pExamInfo -> pAppointmentTime = 0;
			}
		if ( pExamInfo -> pSeriesNumber != 0 )
			{
			free( pExamInfo -> pSeriesNumber );
			pExamInfo -> pSeriesNumber = 0;
			}
		if ( pExamInfo -> pSeriesDescription != 0 )
			{
			free( pExamInfo -> pSeriesDescription );
			pExamInfo -> pSeriesDescription = 0;
			}
		}
}


void AppendDateAndTimeToString( char *pTextString )
{
	char					DateString[20];
	char					TimeString[20];
	int						nChar;
	int						mChar;

	_strdate( DateString );
	_strtime( TimeString );
	mChar = (int)strlen( pTextString );
	for ( nChar = 0; nChar < (int)strlen( DateString ); nChar++ )
		if ( DateString[ nChar ] != '/' )
			pTextString[ mChar++ ] = DateString[ nChar ];
	for ( nChar = 0; nChar < (int)strlen( TimeString ); nChar++ )
		if ( TimeString[ nChar ] != ':' )
			pTextString[ mChar++ ] = TimeString[ nChar ];
	pTextString[ mChar++ ] = '\0';
}


static char			*pTechSupportMsg = "Request technical support.";
static char			*pRetryMsg = "Try sending or importing the file again.";
static char			*pRestartMsg = "Try restarting BRetriever.";

void NotifyUserOfImageFileError( unsigned long ErrorCode, PRODUCT_QUEUE_ITEM *pProductItem )
{
	USER_NOTIFICATION		UserNoticeDescriptor;

	RespondToError( MODULE_EXAM, ErrorCode );
	strcpy( UserNoticeDescriptor.Source, TransferService.ServiceName );
	UserNoticeDescriptor.ModuleCode = MODULE_EXAM;
	UserNoticeDescriptor.ErrorCode = ErrorCode;
	UserNoticeDescriptor.TypeOfUserResponseSupported = USER_RESPONSE_TYPE_ERROR | USER_RESPONSE_TYPE_CONTINUE;
	UserNoticeDescriptor.UserNotificationCause = USER_NOTIFICATION_CAUSE_PRODUCT_PROCESSING_ERROR;
	UserNoticeDescriptor.UserResponseCode = 0L;
	if ( pProductItem != 0 && strlen( pProductItem -> Description ) > 0 )
		sprintf( UserNoticeDescriptor.NoticeText, "A BRetriever error occurred while processing\n%s\n%s\n\n",
													pProductItem -> Description, pProductItem -> SourceFileName );
	else
		strcpy( UserNoticeDescriptor.NoticeText, "The BRetriever service encountered an error:\n\n" );
	
	switch ( ErrorCode )
		{
		case EXAM_ERROR_SOURCE_DIRECTORY:
			strcat( UserNoticeDescriptor.NoticeText, "The required image file receiving directory was not found." );
			strcpy( UserNoticeDescriptor.SuggestedActionText, pTechSupportMsg );
			break;
		case EXAM_ERROR_NO_EXAM_INFO:
			strcat( UserNoticeDescriptor.NoticeText, "Memory corruption was detected\nwhile processing a study." );
			strcpy( UserNoticeDescriptor.SuggestedActionText, pRestartMsg );
			break;
		case EXAM_ERROR_FILE_DELETE:
			strcat( UserNoticeDescriptor.NoticeText, "Unable to delete a study file\nduring post-processing cleanup." );
			strcpy( UserNoticeDescriptor.SuggestedActionText, pTechSupportMsg );
			break;
		case EXAM_ERROR_DICOM_PARSE:
			strcat( UserNoticeDescriptor.NoticeText, "Unable to decode the study file's\nDicom information." );
			strcpy( UserNoticeDescriptor.SuggestedActionText, pRetryMsg );
			break;
		case EXAM_ERROR_INSUFFICIENT_DISK_SPACE:
			strcat( UserNoticeDescriptor.NoticeText, "There is insufficient unused\nstorage space available\nto record any more image files." );
			strcpy( UserNoticeDescriptor.SuggestedActionText, pTechSupportMsg );
			break;
		}
	UserNoticeDescriptor.TextLinesRequired = 8;
	SubmitUserNotification( &UserNoticeDescriptor );
}


BOOL StorageCapacityIsAdequate()
{
	BOOL					bStorageCapacityIsAdequate = TRUE;
	int						DriveIndex;
	unsigned				ErrorCode;
	struct _diskfree_t		DiskCapacityInfo = { 0 };
	double					AvailableBytesForStorage;
	char					Msg[ MAX_LOGGING_STRING_LENGTH ];
	char					DriveName[ 20 ];
	double					FreeMegabytes;
	
	DriveIndex = _getdrive();
	if ( DriveIndex != 0 )
		{
		DriveName[ 0 ] = DriveIndex + 'A' - 1;
		DriveName[ 1 ] = ':';
		DriveName[ 2 ] = '\0';
		ErrorCode = _getdiskfree( (unsigned)DriveIndex, &DiskCapacityInfo );
		if ( ErrorCode == 0 )
			{
			AvailableBytesForStorage = (double)DiskCapacityInfo.avail_clusters *
							(double)DiskCapacityInfo.sectors_per_cluster * (double)DiskCapacityInfo.bytes_per_sector;
			FreeMegabytes = AvailableBytesForStorage / 1000000.0;
			sprintf( Msg, "      Drive %s availabale free space = %16.1f megabytes.", DriveName, FreeMegabytes );
			LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
			bStorageCapacityIsAdequate = ( FreeMegabytes > (double)ServiceConfiguration.MinimumFreeSpaceStorageRequirementInMegabytes );
			if ( !bStorageCapacityIsAdequate )
				NotifyUserOfImageFileError( EXAM_ERROR_INSUFFICIENT_DISK_SPACE, 0 );
			}
		}

	return bStorageCapacityIsAdequate;
}


unsigned __stdcall WatchForExamThreadFunction( VOID *pOperationStruct )
{
	BOOL						bNoError = TRUE;
	BOOL						bTerminateOperation = FALSE;
	PRODUCT_OPERATION			*pProductOperation;
	STUDY_PROCESSING_TASK		QueueExams;
	char						TextLine[ 1096 ];

	pProductOperation = (PRODUCT_OPERATION*)pOperationStruct;

	QueueExams.InitStudyFunction = InitNewStudy;			// Create a product item to represent the entire study.
	QueueExams.ProcessFilesFunction = QueueImage;
	QueueExams.ProcessStudyFunction = CloseStudy;

	sprintf( TextLine, "    Operation Thread: %s", pProductOperation -> OperationName );
	LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
	while ( !bTerminateOperation )
		{
		pProductOperation -> OpnState.DirectorySearchLevel = 0;
		pProductOperation -> OpnState.bOKtoProcessThisStudy = TRUE;
		pProductOperation -> OpnState.pProductItem = 0;
		// Find any new exams and add them to the product queue.
		bTerminateOperation = NavigateExamDirectory( pProductOperation -> pInputEndPoint -> Directory,
														pProductOperation, &QueueExams, "" );
		ProcessProductQueueItems();		// Handle file deletions and queue cleanup.
		if ( ( BRetrieverStatus & BRETRIEVER_STATUS_PROCESSING ) == 0 )
			UpdateBRetrieverStatus( BRETRIEVER_STATUS_ACTIVE );
		EnterOperationCycleWaitInterval( pProductOperation, TRUE, &bTerminateOperation );
		}			// ...end while not bTerminateOperation.
	CloseOperation( pProductOperation );

	return 0;
}


// This function visits each file in a three-level directory tree and calls the function
// pointed to by the ProcessFile argument for each file encountered.  It calls the function
// pointed to by the ProcessStudy argument after each study surveyed.
//
// This function may be called recursively as it navigates through each successive
// subdirectory level.
BOOL NavigateExamDirectory( char *pSourcePath, PRODUCT_OPERATION *pProductOperation,
							STUDY_PROCESSING_TASK *pStudyTransferTask, char *pSpecificProductFileName )
{
	BOOL						bNoError = TRUE;
	char						SearchFileSpec[ MAX_FILE_SPEC_LENGTH ];
	WIN32_FIND_DATA				FindFileInfo;
	HANDLE						hFindFile;
	BOOL						bFileFound;
	BOOL						bTerminationRequested = FALSE;
	BOOL						bQueueSpecificProduct;
	BOOL						bNewStudyEncountered;
	char						SubdirectoryPath[ MAX_FILE_SPEC_LENGTH ];
	char						FoundFileSpec[ MAX_FILE_SPEC_LENGTH ];
	PRODUCT_QUEUE_ITEM			*pStudyProductItem;
	char						TextLine[ 1096 ];

	bQueueSpecificProduct = ( strlen( pSpecificProductFileName ) > 0 );
	bNoError = DirectoryExists( pSourcePath );		// Check existence of source path.
	if ( bNoError )
		{
		strcpy( SearchFileSpec, pSourcePath );
		if ( SearchFileSpec[ strlen( SearchFileSpec ) - 1 ] != '\\' )
			strcat( SearchFileSpec, "\\" );
		if ( bQueueSpecificProduct )
			{
			strcat( SearchFileSpec, pSpecificProductFileName );
			strcat( SearchFileSpec, ".*" );
			}
		else
			strcat( SearchFileSpec, "*.*" );
		// Locate the first file or directory member in the current search directory.
		hFindFile = FindFirstFile( SearchFileSpec, &FindFileInfo );
		bFileFound = ( hFindFile != INVALID_HANDLE_VALUE );
		while ( bFileFound && !bTerminationRequested )
			{
			// If the entry found in the search folder is a subdirectory...
			if ( ( FindFileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )  )
				{
				// Skip the file system's directory entries for the current and parent directory.
				if ( strcmp( FindFileInfo.cFileName, "." ) != 0 && strcmp( FindFileInfo.cFileName, ".." ) != 0 )
					{
					// For each subdirectory encountered, prepare for a recursive call to this function.
					strcpy( SubdirectoryPath, pSourcePath );
					if ( SubdirectoryPath[ strlen( SubdirectoryPath ) - 1 ] != '\\' )
						strcat( SubdirectoryPath, "\\" );
					strcat( SubdirectoryPath, FindFileInfo.cFileName );
					// Have we found a potential new study?  A new study would be represented by a
					// separate folder (not a file) encountered in the highest level search directory.
					bNewStudyEncountered = ( pProductOperation -> OpnState.DirectorySearchLevel == 0 );
					if ( bNewStudyEncountered )
						{
						// This function call will return a pointer to a study product item, which is
						// now associated with the current operation for the rest of the navigation,
						// unless an error occurs.
						pProductOperation -> OpnState.bOKtoProcessThisStudy =
									pStudyTransferTask -> InitStudyFunction( pProductOperation, SubdirectoryPath,
												FindFileInfo.cFileName, FALSE, &pProductOperation -> OpnState.pProductItem );
						}
					if ( pProductOperation -> OpnState.pProductItem != 0 )
						{
						// If a study has been assigned, descend to the next level in the directory tree.
						pProductOperation -> OpnState.DirectorySearchLevel++;
						bTerminationRequested = NavigateExamDirectory( SubdirectoryPath, pProductOperation, pStudyTransferTask, "" );
						// Watch out!  During this nested call, an error could have resulted in the product being deallocated.
						}
					}
				}
			else	// ...else this is a file at the current search level: call the file processing function.
				{
				strcpy( FoundFileSpec, pSourcePath );
				if ( FoundFileSpec[ strlen( FoundFileSpec ) - 1 ] != '\\' )
					strcat( FoundFileSpec, "\\" );
				strcat( FoundFileSpec, FindFileInfo.cFileName );
				pStudyProductItem = pProductOperation -> OpnState.pProductItem;
				// If a study has not yet been assigned (because this is an isolated file and not
				// nested in a study directory tree)...
				if ( pStudyProductItem == 0 )
					{
					// This function call will return a pointer to a study product item, which is
					// now associated with the current operation for the rest of the navigation,
					// unless an error occurs.
					pProductOperation -> OpnState.bOKtoProcessThisStudy =
								pStudyTransferTask -> InitStudyFunction( pProductOperation, FoundFileSpec,
											FindFileInfo.cFileName, TRUE, &pProductOperation -> OpnState.pProductItem );
					}
				pStudyProductItem = pProductOperation -> OpnState.pProductItem;
				if ( pStudyProductItem != 0 )
					{
					if ( pProductOperation -> OpnState.bOKtoProcessThisStudy && pStudyTransferTask -> ProcessFilesFunction != 0 )
						bNoError = pStudyTransferTask -> ProcessFilesFunction( pProductOperation, FoundFileSpec, &FindFileInfo );
					if ( !bNoError )
						{
						sprintf( TextLine, "An error occurred calling the file processing function for %s", FindFileInfo.cFileName );
						LogMessage( TextLine, MESSAGE_TYPE_ERROR );
						pProductOperation -> OpnState.bOKtoProcessThisStudy = FALSE;
						}
					else if ( pProductOperation -> OpnState.DirectorySearchLevel != 3 )
						pProductOperation -> OpnState.DirectorySearchLevel = 1;
					}
				}
			// Look for another file in the source directory.
			bFileFound = FindNextFile( hFindFile, &FindFileInfo );
			bTerminationRequested = CheckForOperationTerminationRequest( pProductOperation );
			}			// ...end while another file found.
		if ( hFindFile != INVALID_HANDLE_VALUE )
			FindClose( hFindFile );
		if ( pProductOperation -> OpnState.bOKtoProcessThisStudy && pStudyTransferTask -> ProcessStudyFunction != 0 &&
					pProductOperation -> OpnState.pProductItem != 0 )
			{
			// Call the study completion function for this study transfer task.  The operation references the
			// study level product.
			bNoError = pStudyTransferTask -> ProcessStudyFunction( pProductOperation, pSourcePath, &pProductOperation -> OpnState.pProductItem );
			if ( pProductOperation -> OpnState.DirectorySearchLevel == 1 )
				{
				pProductOperation -> OpnState.bOKtoProcessThisStudy = FALSE;
				if ( !bNoError )
					{
					sprintf( TextLine, ">>>An error occurred calling the process study function for operation %s", pProductOperation -> OperationName );
					LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
					}
				}
			}
		// Complete processing on this directory level for the current subdirectory.
		if ( pProductOperation -> OpnState.DirectorySearchLevel > 0 )
			pProductOperation -> OpnState.DirectorySearchLevel--;
		}
	else if ( _strnicmp( pSourcePath, "C:", 2 ) == 0 )
		NotifyUserOfImageFileError( EXAM_ERROR_SOURCE_DIRECTORY, 0 );

	return bTerminationRequested;
}


BOOL InitNewStudy( PRODUCT_OPERATION *pProductOperation, char *pSourcePath, char *pSourceName,
											BOOL bIsSeparateFile, PRODUCT_QUEUE_ITEM **ppProductItem )
{
	BOOL					bNoError = TRUE;
	PRODUCT_QUEUE_ITEM		*pStudyProductItem;

	UpdateBRetrieverStatus( BRETRIEVER_STATUS_PROCESSING );
	// Create a product item to represent the entire study.
	bNoError = InitNewProductQueueItem( pProductOperation, &pStudyProductItem );
	if ( bNoError )
		{
		pStudyProductItem -> ProcessingStatus |= PRODUCT_STATUS_STUDY | PRODUCT_STATUS_ITEM_BEING_PROCESSED;
		if ( bIsSeparateFile )
			{
			pStudyProductItem -> ProcessingStatus |= PRODUCT_STATUS_IMAGE_EXTRACT_SINGLE;
			strcpy( pStudyProductItem -> SourceFileName, "" );
			strncat( pStudyProductItem -> SourceFileName, pSourceName, MAX_FILE_SPEC_LENGTH - 1 );
			// Load the full file specification for the source file.
			strcpy( pStudyProductItem -> SourceFileSpec, "" );
			strncat( pStudyProductItem -> SourceFileSpec, pSourcePath, MAX_FILE_SPEC_LENGTH - 1 );
			}
		else
			{
			// Load the name of the subdirectory corresponding to the study's high-level folder.
			strcpy( pStudyProductItem -> SourceFileName, "" );
			strncat( pStudyProductItem -> SourceFileName, pSourceName, MAX_FILE_SPEC_LENGTH - 1 );
			// Load the subdirectory path corresponding to the study's high-level folder.
			strcpy( pStudyProductItem -> SourceFileSpec, "" );
			strncat( pStudyProductItem -> SourceFileSpec, pSourcePath, MAX_FILE_SPEC_LENGTH - 1 );
			}
		pProductOperation -> OpnState.pProductItem = pStudyProductItem;
		}
	*ppProductItem = pStudyProductItem;

	return bNoError;
}


// The bare file name does not include the extension.  This function moves the file over
// to the queued files folder, after it has been found in the watch folder.  This move
// prevents attempts to re-read the file while it is queued or being processed.
int CaptureDicomFileFromWatchFolder( PRODUCT_QUEUE_ITEM *pProductItem )
{
	
	char			DestinationFileSpecification[ MAX_FILE_SPEC_LENGTH ];
	int				ResultCode;

	strcpy( DestinationFileSpecification, "" );
	strncat( DestinationFileSpecification, ServiceConfiguration.QueuedFilesDirectory, FULL_FILE_SPEC_STRING_LENGTH - 1 );
	if ( LocateOrCreateDirectory( DestinationFileSpecification ) )	// Ensure directory exists.
		{
		if ( DestinationFileSpecification[ strlen( DestinationFileSpecification ) - 1 ] != '\\' )
			strcat( DestinationFileSpecification, "\\" );
		strncat( DestinationFileSpecification, pProductItem -> SourceFileName,
					FULL_FILE_SPEC_STRING_LENGTH - 1 - strlen( DestinationFileSpecification ) );
		// Move the file to the "Queued Files" folder.
		ResultCode = rename( pProductItem -> SourceFileSpec, DestinationFileSpecification );
		if ( ResultCode != 0 )
			{
			LogMessage( "Dicom file capture failed:  attempting to delete previous file.", MESSAGE_TYPE_SUPPLEMENTARY );
			if ( DeleteFile( DestinationFileSpecification ) )
				{
				LogMessage( "     Previous version of file deleted from Queued Files folder.", MESSAGE_TYPE_SUPPLEMENTARY );
				ResultCode = rename( pProductItem -> SourceFileSpec, DestinationFileSpecification );
				if ( ResultCode == 0 )
					LogMessage( "     Dicom file capture succeeded on 2nd try.", MESSAGE_TYPE_SUPPLEMENTARY );
				else
					LogMessage( " >>>> Dicom file capture failed on 2nd try.", MESSAGE_TYPE_SUPPLEMENTARY );
				}
			}
		if ( ResultCode == 0 )
			// Post the new file location in the product item.
			strcpy( pProductItem -> SourceFileSpec, DestinationFileSpecification );
		}

	return ResultCode;
}


// The bare file name does not include the extension.  This function moves the file over
// to the errored files folder following an error occurrence.
int MoveDicomFileToErrorFolder( PRODUCT_QUEUE_ITEM *pProductItem )
{
	
	char			DestinationFileSpecification[ MAX_FILE_SPEC_LENGTH ];
	int				ResultCode;

	strcpy( DestinationFileSpecification, "" );
	strncat( DestinationFileSpecification, ServiceConfiguration.ErroredFilesDirectory, FULL_FILE_SPEC_STRING_LENGTH - 1 );
	if ( LocateOrCreateDirectory( DestinationFileSpecification ) )	// Ensure directory exists.
		{
		if ( DestinationFileSpecification[ strlen( DestinationFileSpecification ) - 1 ] != '\\' )
			strcat( DestinationFileSpecification, "\\" );
		strncat( DestinationFileSpecification, pProductItem -> SourceFileName,
					FULL_FILE_SPEC_STRING_LENGTH - 1 - strlen( DestinationFileSpecification ) );
		// Move the file to the "Errored Files" folder.
		ResultCode = rename( pProductItem -> SourceFileSpec, DestinationFileSpecification );
		if ( ResultCode == 0 )
			// Post the new file location in the product item.
			strcpy( pProductItem -> SourceFileSpec, DestinationFileSpecification );
		}

	return ResultCode;
}


// Process a Dicom image file found in a source directory.
BOOL QueueImage( PRODUCT_OPERATION *pProductOperation, char *pFileSpec, WIN32_FIND_DATA *pFindFileInfo )
{
	BOOL					bNoError = TRUE;
	BOOL					bDeleteDuplicateSourceProduct;
	PRODUCT_QUEUE_ITEM		*pStudyProductItem;
	PRODUCT_QUEUE_ITEM		*pProductItem;
	PRODUCT_QUEUE_ITEM		*pDuplicateProductItem;
	EXAM_INFO				*pStudyExamInfo;
	EXAM_INFO				*pExamInfo;
	char					TextLine[ 1096 ];
	int						ResultCode;
	DICOM_HEADER_SUMMARY	*pDicomHeader;
	char					*pFileName;
	time_t					CurrentSystemTime;

	pProductItem = 0;
	pDuplicateProductItem = 0;
	bDeleteDuplicateSourceProduct = FALSE;
	pStudyProductItem = pProductOperation -> OpnState.pProductItem;
	if ( pStudyProductItem != 0 )
		pStudyExamInfo = (EXAM_INFO*)pStudyProductItem -> pProductInfo;
	// A product item representing the study should already have been created by InitNewStudy().
	if ( pStudyProductItem == 0 || pStudyExamInfo == 0 )
		{
		pStudyProductItem -> ModuleWhereErrorOccurred = MODULE_EXAM;
		pStudyProductItem -> FirstErrorCode = EXAM_ERROR_NO_EXAM_INFO;
		NotifyUserOfImageFileError( EXAM_ERROR_NO_EXAM_INFO, pStudyProductItem );
		bNoError = FALSE;
		}
	else
		{
		bNoError = InitNewProductQueueItem( pProductOperation, &pProductItem );
		if ( bNoError )
			{
			// The product item exists from this point on.
			pProductItem -> pParentProduct = pStudyProductItem;
			pStudyProductItem -> ComponentCount++;
			// Set up the product file specifications.
			strcpy( pProductItem -> SourceFileSpec, "" );
			strncat( pProductItem -> SourceFileSpec, pFileSpec, MAX_FILE_SPEC_LENGTH - 1 );
			pFileName = strrchr( pFileSpec, '\\' );
			pFileName++;
			strcpy( pProductItem -> SourceFileName, "" );
			strncat( pProductItem -> SourceFileName, pFileName, MAX_FILE_SPEC_LENGTH - 1 );

			// Move the file out of the watch folder, into the queued files folder.  If an
			// error occurs, the product source specification is unchanged.
			ResultCode = CaptureDicomFileFromWatchFolder( pProductItem );
			// If it's already there, delete the (presumed) duplicate in the watch folder.
			if ( ResultCode != 0 )
				{
				bDeleteDuplicateSourceProduct = TRUE;
				bNoError = FALSE;
				}
			else
				{
				pExamInfo = (EXAM_INFO*)pProductItem -> pProductInfo;
				bNoError = ( pExamInfo != 0 );
				}
			}
		if ( bNoError )
			{
			// Decode and parse the Dicom information from the file.  From this information
			// generate the abstract information output for this image.  The Dicom file contents
			// are retained in a series of memory buffers in the list, pDicomHeader -> ListOfInputBuffers.
			// The copied image buffer is at pDicomHeader -> pImageData.
			bNoError = ReadDicomHeaderInfo( pProductItem -> SourceFileSpec, pExamInfo, &pDicomHeader, TRUE );
//			if ( ServiceConfiguration.bEnableSurvey )
//				{
//				CopyImageFileToSortTreeDirectory( pDicomHeader, pProductItem -> SourceFileSpec, pDicomHeader -> Manufacturer, pDicomHeader -> Modality );
//				}
			if ( bNoError )
				{
				if ( ServiceConfiguration.bEnableSurvey )
					{
					AddImageToSurvey( pDicomHeader, pFileName );
//					CopyImageFileToSortTreeDirectory( pDicomHeader, pProductItem -> SourceFileSpec, pDicomHeader -> Manufacturer, pDicomHeader -> Modality );
					}
				LoadExamInfoFromDicomHeader( pExamInfo, pDicomHeader );
				strcpy( pProductItem -> DestinationFileName, pDicomHeader -> SOPInstanceUniqueIdentifier );
				strncat( pProductItem -> DestinationFileName, ".png", MAX_FILE_SPEC_LENGTH - 1 - strlen(pProductItem -> DestinationFileName) );
				}
			else
				{
				pProductItem -> ModuleWhereErrorOccurred = MODULE_DICOM;
				pProductItem -> FirstErrorCode = DICOM_ERROR_FILE_READ;
				}
			if ( pExamInfo != 0 && pExamInfo -> pLastName != 0 && strlen( pExamInfo -> pLastName ) > 0 )
				{
				strcpy( pProductItem -> Description, "[" );
				strcat( pProductItem -> Description, pExamInfo -> pLastName );
				strcat( pProductItem -> Description, ", " );
				strcat( pProductItem -> Description, pExamInfo -> pFirstName );
				if ( pDicomHeader -> AcquisitionDate != 0 )
					{
					strcat( pProductItem -> Description, " " );
					strcat( pProductItem -> Description, pDicomHeader -> AcquisitionDate );
					}
				if ( pDicomHeader -> AcquisitionTime != 0 )
					{
					strcat( pProductItem -> Description, " " );
					strcat( pProductItem -> Description, pDicomHeader -> AcquisitionTime );
					}
				strcat( pProductItem -> Description, "]" );
				}
			else
				strcpy( pProductItem -> Description, pProductItem -> SourceFileName );
			// Free up all the Dicom input buffers except those up front containing the Dicom header info.
			DeallocateInputImageBuffers( pDicomHeader );
			if ( !bNoError )
				{
				pProductItem -> ProcessingStatus |= PRODUCT_STATUS_RECEIVE_ERROR;
				NotifyUserOfImageFileError( EXAM_ERROR_DICOM_PARSE, pProductItem );
				}
			}
		if ( bNoError )
			{
			pProductItem -> ProcessingStatus |= PRODUCT_STATUS_ABSTRACTS_COMPLETED;
			sprintf( TextLine, "Importing Dicom exam %s from %s:  %s",
						pProductItem -> Description, pProductOperation -> pInputEndPoint -> Name, pProductItem -> SourceFileName );
			LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
			// If this is the first Dicom image file in the current study, load the study parameters
			// from the current file.
			if ( pStudyExamInfo -> pLastName == 0 && pStudyExamInfo -> pFirstName == 0 && pStudyProductItem -> ComponentCount == 1 )
				{
				LoadExamInfoFromDicomHeader( pStudyExamInfo, pDicomHeader );
				strcpy( pStudyProductItem -> Description, pProductItem -> Description );
				}
			}
		if ( bNoError )
			bNoError = QueueProductForTransfer( pProductOperation, &pProductItem, &pDuplicateProductItem );
		if ( pDuplicateProductItem != 0 )
			bDeleteDuplicateSourceProduct = TRUE;
		else if ( bNoError )		// If not a duplicate item...
			{
			bNoError = ( pProductItem -> LocalProductIndex != 0L );
			if ( bNoError )
				{
				time( &CurrentSystemTime );
				pStudyProductItem -> LatestActivityTime = CurrentSystemTime;
				pProductItem -> LatestActivityTime = CurrentSystemTime;
				}
			}
		}
	if ( !bNoError )
		{
		if ( pProductItem != 0 )
			pProductItem -> ProcessingStatus |= PRODUCT_STATUS_IMAGE_EXTRACTION_ERROR;
		sprintf( TextLine, "Possible Error:  File found but not queued:  %s", pFileSpec );
		LogMessage( TextLine, MESSAGE_TYPE_ERROR );
		// List contents of image folders.
		ListImageFolderContents();
		// The source file at this point may be in the Watch Folder or in the Queued Files
		// folder, depending upon when the error was detected.  If an error occurs moving
		// the file, the product source specification is unchanged:
		ResultCode = MoveDicomFileToErrorFolder( pProductItem );
		// If it's already there, delete the (presumed) duplicate.
		if ( ResultCode != 0 )
			{
			bDeleteDuplicateSourceProduct = TRUE;
			bNoError = FALSE;
			}
		}
	if ( bDeleteDuplicateSourceProduct )
		DeleteSourceProduct( pProductOperation, &pProductItem );			// Caution:  no product info remains after this deletion.
	if ( ( BRetrieverStatus & BRETRIEVER_STATUS_PROCESSING ) == 0 )
		UpdateBRetrieverStatus( BRETRIEVER_STATUS_ACTIVE );

	Sleep( 3000 );		// Allow other programs to run.

	return bNoError;
}


BOOL CloseStudy( PRODUCT_OPERATION *pProductOperation, char *pFileSpec, PRODUCT_QUEUE_ITEM **ppStudyProductItem )
{
	BOOL						bNoError = TRUE;
	PRODUCT_QUEUE_ITEM			*pStudyProductItem;
	char						TextLine[ 1096 ];

	pStudyProductItem = *ppStudyProductItem;
	if ( pStudyProductItem != 0 )
		{
		pStudyProductItem -> ProcessingStatus &= ~PRODUCT_STATUS_ITEM_BEING_PROCESSED;
		// This is delicate, since DeleteExamFolders() will lead to a recursive call into
		//  NavigateExamDirectory().
		pProductOperation -> OpnState.DirectorySearchLevel = 0;
		}
	// Delete the associated study folders.
	pStudyProductItem -> ProcessingStatus |= PRODUCT_STATUS_SOURCE_DELETABLE;
	DeleteExamFolders( (void*)pStudyProductItem );
	// Even if there is a folder deletion error (Windows thinks it's not empty.), go
	// ahead and delete the study:  A new one will be created if any undeleted
	// folders are rediscovered.
	pStudyProductItem -> ProcessingStatus |= PRODUCT_STATUS_SOURCE_DELETED;
	// If finished image processing, go ahead and deallocate the study.  Otherwise,
	// image processing will take care of it.
	if ( ( pStudyProductItem -> ProcessingStatus & PRODUCT_STATUS_STUDY ) != 0 &&
				pStudyProductItem -> ComponentCount == 0 )
		{
		sprintf( TextLine, "Deallocating study: ID = %d, %s",
								pStudyProductItem -> LocalProductIndex, pStudyProductItem -> SourceFileName );
		LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
		DeallocateProductInfo( pStudyProductItem );
		}
	
	return bNoError;
}


BOOL CheckOkToDeleteSourceExam( PRODUCT_OPERATION *pProductOperation, char *pSourcePath, char *pSourceName,
																BOOL bIsSeparateFile, PRODUCT_QUEUE_ITEM **ppProductItem )
{
	BOOL					bNoError = TRUE;
	BOOL					bOkToDeleteProductSource;
	BOOL					bExamDeleteAuthorized;
	PRODUCT_QUEUE_ITEM		*pProductItem;
	char					TextLine[ 1096 ];

	pProductItem = *ppProductItem;
	bExamDeleteAuthorized = FALSE;
	if ( pProductItem != 0 )
		{
		if ( pProductItem -> ProcessingStatus & PRODUCT_STATUS_RECEIVE_ERROR )
			pProductItem -> ProcessingStatus |= PRODUCT_STATUS_ERROR_NOTIFY_USER;
		// Determine which, if any, of the product files the CURRENT OPERATION is authorized to delete.
		bOkToDeleteProductSource =	( ( pProductItem -> ProcessingStatus & PRODUCT_STATUS_SOURCE_DELETABLE ) != 0 &&
										( pProductItem -> ProcessingStatus & PRODUCT_STATUS_SOURCE_DELETED ) == 0 &&
											( pProductOperation -> pInputEndPoint == &EndPointWatchFolder ||
												pProductOperation -> pInputEndPoint == &EndPointQueue ||
											( pProductItem -> ProcessingStatus & PRODUCT_STATUS_STUDY ) != 0 ) );
		bExamDeleteAuthorized = bOkToDeleteProductSource;
		}
	if ( bExamDeleteAuthorized )
		{
		sprintf( TextLine, "Deletion authorized for:  %s", pProductItem -> SourceFileName );
		LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
		pProductItem -> ProcessingStatus |= PRODUCT_STATUS_ITEM_BEING_PROCESSED;
		}
	else if ( pProductItem != 0 )
		{
		sprintf( TextLine, "No deletion authorized for:  %s at this time.", pProductItem -> SourceFileName );
		LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
		}

	return bExamDeleteAuthorized;
}


BOOL DeleteSeriesFile( PRODUCT_OPERATION *pProductOperation, char *pSourceFileSpec, WIN32_FIND_DATA *pFindFileInfo )
{
	BOOL					bNoError = TRUE;
	char					TextLine[ 1096 ];
	PRODUCT_QUEUE_ITEM		*pProductItem;

	pProductItem = pProductOperation -> OpnState.pProductItem;
	sprintf( TextLine, "Deleting image file:  %s", pSourceFileSpec );
	LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
	if ( !DeleteFile( pSourceFileSpec ) )
		{
		sprintf( TextLine, "Unable to delete: %s", pSourceFileSpec );
		LogMessage( TextLine, MESSAGE_TYPE_ERROR );
		}

	return bNoError;
}


BOOL DeleteStudyFolders( PRODUCT_OPERATION *pProductOperation, char *pFileSpec, PRODUCT_QUEUE_ITEM **ppProductItem )
{
	BOOL					bNoError = TRUE;
	PRODUCT_QUEUE_ITEM		*pProductItem;
	BOOL					bDirectoryRemoved;
	DWORD					SystemErrorCode;
	char					TextLine[ 1096 ];

	pProductItem = *ppProductItem;
	if ( pProductOperation -> OpnState.DirectorySearchLevel >= 0 &&
				pProductItem != 0 && ( pProductItem -> ProcessingStatus & PRODUCT_STATUS_IMAGE_EXTRACT_SINGLE ) == 0 )
		{
		// Since the calls to this function start from the deepest subdirectories and bubble up, it can
		// be used to delete the directory tree from the bottom up.
		sprintf( TextLine, "Deleting source directory = %s", pFileSpec );
		LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
		// Unset this local directory as the current working directory.
		SetCurrentDirectory( TransferService.ProgramDataPath );
		bDirectoryRemoved = RemoveDirectory( pFileSpec );
		if ( !bDirectoryRemoved )
			{
			SystemErrorCode = GetLastError();
			sprintf( TextLine, "   >>>Delete study folders system error code %d", SystemErrorCode );
			LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
			bNoError = FALSE;
			}
		}
	// If this is the top level study folder, mark the job complete.
	if ( bNoError && pProductOperation -> OpnState.DirectorySearchLevel == 0 ||
				( pProductItem != 0 && ( pProductItem -> ProcessingStatus & PRODUCT_STATUS_IMAGE_EXTRACT_SINGLE ) != 0 ) )
		{
		if ( bNoError && pProductItem != 0 )
			{
			if ( ( pProductItem -> ProcessingStatus & PRODUCT_STATUS_SOURCE_DELETABLE ) != 0 &&
						( pProductItem -> ProcessingStatus & PRODUCT_STATUS_SOURCE_DELETED ) == 0 &&
							pProductOperation -> pInputEndPoint == &EndPointWatchFolder )
				{
				pProductItem -> ProcessingStatus |= PRODUCT_STATUS_SOURCE_DELETED;
				pProductItem -> ProcessingStatus &= ~PRODUCT_STATUS_ITEM_BEING_PROCESSED;
				}
			}
		}

	return bNoError;
}


void ComposePatientLevelFolderName( EXAM_INFO *pExamInfo, char *pTextString )
{
	if ( pExamInfo -> pFirstName == 0 && pExamInfo -> pLastName == 0 )
		strcpy( pTextString, "Nameless" );
	else
		strcpy( pTextString, "" );
	if ( pExamInfo -> pLastName != 0 )
		{
		strcat( pTextString, pExamInfo -> pLastName );
		strcat( pTextString, "_" );
		}
	if ( pExamInfo -> pFirstName != 0 )
		{
		strcat( pTextString, pExamInfo -> pFirstName );
		_strupr( pTextString );			// Convert patient name part to upper case.
		}
	strcat( pTextString, "_" );
	if ( pExamInfo -> pExamID != 0 )
		strcat( pTextString, pExamInfo -> pExamID );
	else
		// If no exam ID, use the current date and time.
		AppendDateAndTimeToString( pTextString );
}


void ComposeStudyLevelFolderName( EXAM_INFO *pExamInfo, char *pTextString )
{
	char		TempString[ 256 ];
	int			nChar;
	int			nOutChar;
	int			nChars;

	strcpy( TempString, "" );
	if ( pExamInfo -> pAppointmentDate == 0 && pExamInfo -> pAppointmentTime == 0 )
		// If no appointment time, use the current date and time.
		AppendDateAndTimeToString( TempString );
	else
		{
		if ( pExamInfo -> pAppointmentDate != 0 )
			{
			strcat( TempString, pExamInfo -> pAppointmentDate );
			strcat( TempString, "_" );
			}
		if ( pExamInfo -> pAppointmentTime != 0 )
			strcat( TempString, pExamInfo -> pAppointmentTime );
		}
	nChars = (int)strlen( TempString );
	nOutChar = 0;
	for ( nChar = 0; nChar < nChars; nChar++ )
		if ( TempString[ nChar ] != '-' && TempString[ nChar ] != ':' )
			pTextString[ nOutChar++ ] = TempString[ nChar ];
	pTextString[ nOutChar++ ] = '\0';
}


void ComposeSeriesLevelFolderName( EXAM_INFO *pExamInfo, char *pTextString )
{
	strcpy( pTextString, "Series_" );
	if ( pExamInfo -> pSeriesNumber != 0 )
		{
		strcat( pTextString, pExamInfo -> pSeriesNumber );
		strcat( pTextString, "_" );
		}
	if ( pExamInfo -> pSeriesDescription != 0 )
		strcat( pTextString, pExamInfo -> pSeriesDescription );
}


BOOL DeleteExamFolders( VOID *pProductItemStruct )
{
	BOOL					bNoError = TRUE;
	PRODUCT_QUEUE_ITEM		*pProductItem;
	STUDY_PROCESSING_TASK	DeleteTransferredExams;
	PRODUCT_OPERATION		*pProductOperation;
	ENDPOINT				*pEndpoint;
	char					FolderSpec[ MAX_FILE_SPEC_LENGTH ];
	BOOL					bDirectoryRemovalEnabled;
	char					TextLine[ 1096 ];

	DeleteTransferredExams.InitStudyFunction = CheckOkToDeleteSourceExam;
	DeleteTransferredExams.ProcessFilesFunction = DeleteSeriesFile;
	DeleteTransferredExams.ProcessStudyFunction = DeleteStudyFolders;

	pProductItem = ( PRODUCT_QUEUE_ITEM* )pProductItemStruct;
	if ( pProductItem != 0 )
		{
		pProductOperation = (PRODUCT_OPERATION*)pProductItem -> pProductOperation;
		sprintf( TextLine, "DeleteExamFolders called for:  %s", pProductItem -> SourceFileName );
		LogMessage( TextLine, MESSAGE_TYPE_SUPPLEMENTARY );
		}

	bDirectoryRemovalEnabled = FALSE;
	if ( ( pProductItem -> ProcessingStatus & PRODUCT_STATUS_SOURCE_DELETABLE ) != 0 &&
						( pProductItem -> ProcessingStatus & PRODUCT_STATUS_SOURCE_DELETED ) == 0  )
		{
		pEndpoint = &EndPointWatchFolder;
		strcpy( FolderSpec, pEndpoint -> Directory );
		if ( FolderSpec[ strlen( FolderSpec ) - 1 ] != '\\' )
			strcat( FolderSpec, "\\" );
		bDirectoryRemovalEnabled = TRUE;
		}

	if ( pProductOperation != 0 )
		{
		pProductOperation -> OpnState.pProductItem = pProductItem;
		// Delete any exams that have been successfully transferred.
		pProductOperation -> OpnState.DirectorySearchLevel = 0;
		pProductOperation -> OpnState.bOKtoProcessThisStudy = FALSE;
		bNoError = !NavigateExamDirectory( FolderSpec, pProductOperation, &DeleteTransferredExams, pProductItem -> SourceFileName );
		}

	return bNoError;
}


