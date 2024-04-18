// Exam.h : Defines the data structures and functions related to
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
#pragma once

#define EXAM_ERROR_SOURCE_DIRECTORY				1
#define EXAM_ERROR_REMOVE_DIRECTORY				2
#define EXAM_ERROR_NO_EXAM_INFO					3
#define EXAM_ERROR_FILE_DELETE					4
#define EXAM_ERROR_DICOM_PARSE					5
#define EXAM_ERROR_INSUFFICIENT_DISK_SPACE		6

#define EXAM_ERROR_DICT_LENGTH					6



typedef BOOL		(*STUDY_OPERATION)( PRODUCT_OPERATION *pProductOperation, char *pSourcePath, char *pSourceName,
																		BOOL bIsSeparateFile, PRODUCT_QUEUE_ITEM **ppProductItem );
typedef BOOL		(*FILE_OPERATION)( PRODUCT_OPERATION *pProductOperation, char *pFileSpec, WIN32_FIND_DATA *pFindFileInfo );
typedef BOOL		(*STUDY_COMPLETION)( PRODUCT_OPERATION *pProductOperation, char *pFileSpec, PRODUCT_QUEUE_ITEM **ppProductItem );


typedef struct
	{
	STUDY_OPERATION		InitStudyFunction;
	FILE_OPERATION		ProcessFilesFunction;
	STUDY_COMPLETION	ProcessStudyFunction;
	} STUDY_PROCESSING_TASK;



// Function prototypes.
//
void				InitExamModule();
void				CloseExamModule();

void				InitExamInfoStructure( EXAM_INFO *pExamInfo );
void				DeallocateExamInfoAttributes( EXAM_INFO *pExamInfo );
BOOL				InitNewStudy( PRODUCT_OPERATION *pProductOperation, char *pSourcePath, char *pSourceName,
														BOOL bIsSeparateFile, PRODUCT_QUEUE_ITEM **ppProductItem );
BOOL				QueueImage( PRODUCT_OPERATION *pProductOperation, char *pFileSpec, WIN32_FIND_DATA *pFindFileInfo );
BOOL				CloseStudy( PRODUCT_OPERATION *pProductOperation, char *pFileSpec, PRODUCT_QUEUE_ITEM **ppStudyProductItem );
BOOL				CheckOkToDeleteSourceExam( PRODUCT_OPERATION *pProductOperation, char *pSourcePath, char *pSourceName,
																BOOL bIsSeparateFile, PRODUCT_QUEUE_ITEM **ppProductItem );
BOOL				DeleteSeriesFile( PRODUCT_OPERATION *pProductOperation, char *pSourceFileSpec, WIN32_FIND_DATA *pFindFileInfo );
BOOL				DeleteStudyFolders( PRODUCT_OPERATION *pProductOperation, char *pFileSpec, PRODUCT_QUEUE_ITEM **ppProductItem );
void				AppendDateAndTimeToString( char *pTextString );
void				NotifyUserOfImageFileError( unsigned long ErrorCode, PRODUCT_QUEUE_ITEM *pProductItem );
BOOL				StorageCapacityIsAdequate();
BOOL				NavigateExamDirectory( char *pSourcePath, PRODUCT_OPERATION *pProductOperation,
														STUDY_PROCESSING_TASK *pStudyTransferTask, char *pSendProductID );
//void				ComposePatientLevelFolderName( EXAM_INFO *pExamInfo, char *pTextString );
void				ComposeStudyLevelFolderName( EXAM_INFO *pExamInfo, char *pTextString );
//void				ComposeSeriesLevelFolderName( EXAM_INFO *pExamInfo, char *pTextString );
BOOL				DeleteExamFolders( VOID *pProductItemStruct );



