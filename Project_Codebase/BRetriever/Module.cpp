// Module.cpp:  Implements functions for general-purpose use by the BRetriever
// software modules.  Controls the initialization and closure of each of the
// software modules.
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
#include <sys/types.h>
#include <sys/stat.h>
#include "ReportStatus.h"
#include "ServiceMain.h"
#include "Dicom.h"
#include "Configuration.h"
#include "Operation.h"
#include "ProductDispatcher.h"
#include "WinSocketsAPI.h"
#include "DicomAssoc.h"
#include "DicomAcceptor.h"
#include "DicomInitiator.h"
#include "DicomCommand.h"
#include "DicomCommunication.h"
#include "Exam.h"
#include "Abstract.h"
#include "ExamReformat.h"
#include "Calibration.h"
#include "ExamEdit.h"


MODULE_INFO				*pModuleInfoList = 0;

MODULE_INIT_FUNCTION	ModuleInitializationFunctions[] =
								{
								InitMainModule,
								InitStatusModule,
								InitConfigurationModule,
								InitDicomModule,
								InitWinSockAPIModule,
								InitDicomAssocModule,
								InitProductDispatcherModule,
								InitProductOperationsModule,
								InitDicomCommandModule,
								InitDicomStateModule,
								InitDicomAcceptorModule,
								InitDicomInitiatorModule,
								InitExamModule,
								InitAbstractModule,
								InitDictionaryModule,
								InitImageCalibrateModule,
								InitExamReformatModule,
								InitExamEditModule,
								0
								};


// This function needs to be called during program initialization to individually
// initialize each of the software modules that make up the program.
void InitializeSoftwareModules()
{
	unsigned long			nModuleTableItem;
	MODULE_INIT_FUNCTION	ModuleInitFunction;
	
	nModuleTableItem = 0;
	do
		{
		ModuleInitFunction = ModuleInitializationFunctions[ nModuleTableItem++ ];
		// Call the next module initialization function.
		if ( ModuleInitFunction != 0 )
			( *ModuleInitFunction )();
		}
	while ( ModuleInitFunction != 0 );
}


void LinkModuleToList( MODULE_INFO *pNewModuleInfo )
{
	MODULE_INFO			*pModuleInfo;
	
	pModuleInfo = pModuleInfoList;
	// Move to the end of the list of currently linked modules.
	while ( pModuleInfo != 0 && pModuleInfo -> pNextModuleInfo != 0 )
		pModuleInfo = pModuleInfo -> pNextModuleInfo;
	// Link the new module to the end of the list.
	if ( pModuleInfo == 0 )
		pModuleInfoList = pNewModuleInfo;
	else
		pModuleInfo -> pNextModuleInfo = pNewModuleInfo;
	pNewModuleInfo -> pNextModuleInfo = 0;
}


char *GetModuleName( unsigned long RequestedModuleIndex )
{
	MODULE_INFO			*pModuleInfoItem;
	BOOL				bNotFound = TRUE;
	char				*pModuleName;
	
	pModuleName = "";
	pModuleInfoItem = pModuleInfoList;
	while ( bNotFound && pModuleInfoItem != 0 )
		{
		if ( pModuleInfoItem -> ModuleIndex == RequestedModuleIndex )
			{
			pModuleName = pModuleInfoItem -> pModuleName;
			bNotFound = FALSE;
			}
		pModuleInfoItem = pModuleInfoItem -> pNextModuleInfo;
		}
	return pModuleName;
}


// This function needs to be called during program termination to individually
// close each of the software modules that make up the program.
void CloseSoftwareModules()
{
	MODULE_INFO			*pModuleInfoItem;
	
	pModuleInfoItem = pModuleInfoList;;
	while ( pModuleInfoItem != 0 )
		{
		if ( pModuleInfoItem -> ModuleCloseFunction != 0 )
			( *pModuleInfoItem -> ModuleCloseFunction )();
		pModuleInfoItem = pModuleInfoItem -> pNextModuleInfo;
		}
}



//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		ModuleModuleInfo = { MODULE_MODULE, "Module Module", InitModuleModule, 0 };


static ERROR_DICTIONARY_ENTRY	ModuleErrorCodes[] =
			{
				{ MODULE_ERROR_INSUFFICIENT_MEMORY			, "There is not enough memory to allocate a data structure." },
				{ MODULE_ERROR_CREATE_DIRECTORY				, "An error occurred while attempting to create a destination directory." },
				{ 0											, NULL }
			};


static ERROR_DICTIONARY_MODULE		ModuleStatusErrorDictionary =
										{
										MODULE_MODULE,
										ModuleErrorCodes,
										MODULE_ERROR_DICT_LENGTH,
										0
										};


// This function must be called before any other function in this module.
void InitModuleModule()
{
	LinkModuleToList( &ModuleModuleInfo );
	RegisterErrorDictionary( &ModuleStatusErrorDictionary );
}


//___________________________________________________________________________
//
// General-purpose functions for managing lists:
//

BOOL AppendToList( LIST_HEAD *pListHead, void *pItemToAppend )
{
	BOOL			bNoError = TRUE;
	LIST_ELEMENT	*pNewListElement;
	LIST_ELEMENT	*pListElement;
	LIST_ELEMENT	*pPrevListElement;
	BOOL			bAlreadyInList;
	
	pNewListElement = (LIST_ELEMENT*)malloc( sizeof(LIST_ELEMENT) );
	if ( pNewListElement == 0 )
		{
		bNoError = FALSE;
		RespondToError( MODULE_MODULE, MODULE_ERROR_INSUFFICIENT_MEMORY );
		}
	else
		{
		// Populate the new list element structure.
		pNewListElement -> pItem = pItemToAppend;
		pNewListElement -> pNextListElement = 0;
		// Append it to the specified list.
		pListElement = *pListHead;
		bAlreadyInList = FALSE;
		pPrevListElement = 0;
		if ( pListElement == 0 )
			// If the list is empty, insert the new item at the beginning of the list.
			*pListHead = pNewListElement;
		else
			{
			// Append the new item at the end of the list.
			while ( !bAlreadyInList && pListElement != 0 )
				{
				if ( pListElement -> pItem == pItemToAppend )
					bAlreadyInList = TRUE;
				pPrevListElement = pListElement;
				pListElement = pListElement -> pNextListElement;
				}
			if ( bAlreadyInList )
				free( pNewListElement );
			else
				pPrevListElement -> pNextListElement = pNewListElement;
			}
		}

	return bNoError;
}


BOOL PrefixToList( LIST_HEAD *pListHead, void *pItemToPrefix )
{
	BOOL			bNoError = TRUE;
	LIST_ELEMENT	*pNewListElement;
	
	pNewListElement = (LIST_ELEMENT*)malloc( sizeof(LIST_ELEMENT) );
	if ( pNewListElement == 0 )
		{
		bNoError = FALSE;
		RespondToError( MODULE_MODULE, MODULE_ERROR_INSUFFICIENT_MEMORY );
		}
	else
		{
		// Populate the new list element structure.
		pNewListElement -> pItem = pItemToPrefix;
		// Unhook the first item in the list (if any) and link it to the new item..
		pNewListElement -> pNextListElement = *pListHead;
		// Insert the new item at the beginning of the list.
		*pListHead = pNewListElement;
		}

	return bNoError;
}


// This function removes the indicated item from the list and frees
// the memory used by the link.  The item itsself is not deleted.
BOOL RemoveFromList( LIST_HEAD *pListHead, void *pListItemToRemove )
{
	LIST_ELEMENT	*pPrevListElement;
	LIST_ELEMENT	*pListElement;
	BOOL			bMatchingListElementFound = FALSE;
	
	pListElement = *pListHead;
	pPrevListElement = 0;
	while ( pListElement != 0 && !bMatchingListElementFound )
		{
		if ( pListElement -> pItem == pListItemToRemove )
			{
			bMatchingListElementFound = TRUE;
			if ( pPrevListElement == 0 )
				*pListHead = pListElement -> pNextListElement;
			else
				pPrevListElement -> pNextListElement = pListElement -> pNextListElement;
			free( pListElement );
			}
		else
			{
			pPrevListElement = pListElement;
			pListElement = pListElement -> pNextListElement;
			}
		}

	return bMatchingListElementFound;
}


BOOL IsItemInList( LIST_HEAD *pListHead, void *pItemToMatch )
{
	LIST_ELEMENT	*pListElement;
	BOOL			bMatchingListElementFound = FALSE;
	
	pListElement = *pListHead;
	while ( pListElement != 0 && !bMatchingListElementFound )
		{
		if ( pListElement -> pItem == pItemToMatch )
			bMatchingListElementFound = TRUE;
		pListElement = pListElement -> pNextListElement;
		}

	return bMatchingListElementFound;
}


// Delete the list, but not the referenced items.
BOOL DissolveList( LIST_HEAD *pListHead )
{
	BOOL			bNoError = TRUE;
	LIST_ELEMENT	*pPrevListElement;
	LIST_ELEMENT	*pListElement;

	if ( pListHead != 0 )
		{
		pListElement = *pListHead;
		while ( pListElement != 0 )
			{
			pPrevListElement = pListElement;
			pListElement = pListElement -> pNextListElement;
			free( pPrevListElement );
			}
		*pListHead = 0;
		}

	return bNoError;
}


// Delete the list and the referenced items.
BOOL EraseList( LIST_HEAD *pListHead )
{
	BOOL			bNoError = TRUE;
	LIST_ELEMENT	*pPrevListElement;
	LIST_ELEMENT	*pListElement;
	void			*pListItem;

	if ( pListHead != 0 )
		{
		pListElement = *pListHead;
		while ( pListElement != 0 )
			{
			pPrevListElement = pListElement;
			pListElement = pListElement -> pNextListElement;
			pListItem = pPrevListElement -> pItem;
			if ( pListItem != 0 )
				free( pListItem );
			free( pPrevListElement );
			}
		*pListHead = 0;
		}

	return bNoError;
}


//___________________________________________________________________________
//
// Miscellaneous general-purpose functions:
//

BOOL LocateOrCreateDirectory( char *pDirectorySpec )
{
	BOOL					bNoError = TRUE;
	BOOL					bDirectoryCreated;
	BOOL					bDirectoryExists;

	bDirectoryExists = SetCurrentDirectory( pDirectorySpec );
	if ( !bDirectoryExists )
		{
		bDirectoryCreated = CreateDirectory( pDirectorySpec, NULL );
		if ( !bDirectoryCreated )
			{
			bNoError = FALSE;
			RespondToError( MODULE_MODULE, MODULE_ERROR_CREATE_DIRECTORY );
			}
		}

	return bNoError;
}


__int64 GetFileSizeInBytes( char *pFullFileSpecification )
{
	struct __stat64			FileStatisticsBuffer;
	int						Result;
	FILE					*pFile;
	__int64					nFileSizeInBytes;

	nFileSizeInBytes = 0;
	pFile = fopen( pFullFileSpecification, "rt" );
	if ( pFile != NULL )
		{
		Result = _fstat64( _fileno( pFile), &FileStatisticsBuffer );
		if ( Result == 0 )
			nFileSizeInBytes = (unsigned long)FileStatisticsBuffer.st_size;
		fclose( pFile );
		}
	return nFileSizeInBytes;
}


BOOL DirectoryExists( char *pFullDirectorySpecification )
{
	BOOL			bDirectoryExists;
	char			SavedCurrentDirectory[ MAX_FILE_SPEC_LENGTH ];

	GetCurrentDirectory( MAX_FILE_SPEC_LENGTH, SavedCurrentDirectory );
	bDirectoryExists = SetCurrentDirectory( pFullDirectorySpecification );
	SetCurrentDirectory( SavedCurrentDirectory );
	
	return bDirectoryExists;
}


