// Module.cpp:  Implements functions for general-purpose use by the BViewer
//  software modules.  Controls the initialization and closure of each of the
//  software modules, and provides list management functions.
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
//#define _WIN32_WINNT 0x0501

//#include <windows.h>

#include "Module.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "ReportStatus.h"
#include "Configuration.h"


MODULE_INFO				*pModuleInfoList = 0;

MODULE_INIT_FUNCTION	ModuleInitializationFunctions[] =
								{
								InitStatusModule,
								InitConfigurationModule,
								InitDictionaryModule,
								InitAbstractModule,
								InitImageModule,
								InitImageViewModule,
								InitHelpModule,
								InitModuleModule,
								InitGraphicsAdapterModule,
								InitDicomModule,
								InitImportModule,
								InitImportDicomdirModule,
								InitInstallModule,
								InitSignatureModule,
								InitClientModule,
								InitPresetModule,
								0
								};

extern CONFIGURATION	BViewerConfiguration;


// This function needs to be called during program initialization to individually
// initialize each of the software modules that make up the program.
void InitializeSoftwareModules()
{
	unsigned long			nModuleTableItem;
	MODULE_INIT_FUNCTION	ModuleInitFunction;
	char					FileSpec[ MAX_CFG_STRING_LENGTH ];

	nModuleTableItem = 0;
	do
		{
		ModuleInitFunction = ModuleInitializationFunctions[ nModuleTableItem++ ];
		// Call the next module initialization function.
		if ( ModuleInitFunction != 0 )
			( *ModuleInitFunction )();
		}
	while ( ModuleInitFunction != 0 );

	LogMessage( "\n\nBViewer (version 1.2l) started.  ****************************************", MESSAGE_TYPE_NORMAL_LOG );
	if ( !ReadConfigurationFile( BViewerConfiguration.ConfigDirectory, "BViewer.cfg" ) )
		{
		LogMessage( "Aborting BViewer without configuration file.", MESSAGE_TYPE_ERROR );
		exit( 0 );
		}
	if ( !ReadConfigurationFile( BViewerConfiguration.BRetrieverServiceDirectory, "Shared.cfg" ) )
		{
		LogMessage( "Aborting BViewer without shared configuration file.", MESSAGE_TYPE_ERROR );
		exit( 0 );
		}
	strcpy( FileSpec, BViewerConfiguration.ConfigDirectory );
	if ( FileSpec[ strlen( FileSpec ) - 1 ] != '\\' )
		strcat( FileSpec, "\\" );
	strcat( FileSpec, "DicomDictionary.txt" );
	if ( !ReadDictionaryFile( FileSpec ) )
		{
		LogMessage( "Aborting without Dicom dictionary.", MESSAGE_TYPE_ERROR );
		exit( 0 );
		}
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


unsigned long CountListItems( LIST_HEAD ListHead )
{
	LIST_ELEMENT	*pListElement;
	unsigned long	nListElement = 0;
	
	pListElement = ListHead;
	while ( pListElement != 0 )
		{
		pListElement = pListElement -> pNextListElement;
		nListElement++;
		}

	return nListElement;
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


void GetDateAndTimeForFileName( char *pDateTimeString )
{
	time_t						CurrentSystemTime;
	struct tm					*pDaTim;

	time( &CurrentSystemTime );
	pDaTim = localtime( &CurrentSystemTime );   // Convert time to struct tm form.

	sprintf( pDateTimeString, "%04d%02d%02d_%02d%02d%02d_",
				pDaTim -> tm_year +1900, pDaTim -> tm_mon + 1, pDaTim -> tm_mday, pDaTim -> tm_hour, pDaTim -> tm_min, pDaTim -> tm_sec );
}


void SubstituteCharacterInText( char *pTextString, char SearchForChar, char ReplacementChar )
{
	size_t			nChars;
	size_t			nChar;

	nChars = strlen( pTextString );
	for ( nChar = 0; nChar < nChars; nChar++ )
		if ( pTextString[ nChar ] == SearchForChar )
			pTextString[ nChar ] = ReplacementChar;
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


// pStorageDeviceSpecification points to a drive specification string of the form "A:".
void GetDriveLabel( char *pStorageDeviceSpecification, char *pStorageDeviceLabel )
{
	BOOL			bNoError;
	BOOL			bRemovableMedia;
	char			StorageDeviceRootDirectory[ 256 ];
	char			VolumeName[ 256 ];
	char			RemovableVolumeName[ 256 ];
	char			FileSystemName[ 256 ];
	unsigned		StorageDeviceType;
	DWORD			VolumeSerialNumber;
	DWORD			MaximumComponentLength;
	DWORD			FileSystemFlags;

	strcpy( StorageDeviceRootDirectory, pStorageDeviceSpecification );
	strcat( StorageDeviceRootDirectory, "\\" );
	StorageDeviceType = GetDriveType( StorageDeviceRootDirectory );
	bRemovableMedia = FALSE;
	switch ( StorageDeviceType )
		{
		case DRIVE_UNKNOWN:
			strcpy( VolumeName, pStorageDeviceSpecification );
			break;
		case DRIVE_NO_ROOT_DIR:
			strcpy( VolumeName, "No Drive Mounted (" );
			bRemovableMedia = TRUE;
			break;
		case DRIVE_REMOVABLE:
			bRemovableMedia = TRUE;
			if ( pStorageDeviceSpecification[ 0 ] >= 'C' )
				strcpy( VolumeName, "Removable Media (" );
			else
				strcpy( VolumeName, pStorageDeviceSpecification );
			break;
		case DRIVE_FIXED:
			strcpy( VolumeName, "Local Disk (" );
			bNoError = GetVolumeInformation( StorageDeviceRootDirectory, RemovableVolumeName, 255,
												&VolumeSerialNumber, &MaximumComponentLength, &FileSystemFlags, FileSystemName, 255 );
			if ( bNoError && strlen( RemovableVolumeName ) > 0 )
				{
				strcpy( VolumeName, RemovableVolumeName );
				strcat( VolumeName, " (" );
				}
			break;
		case DRIVE_REMOTE:
			strcpy( VolumeName, "Network Drive (" );
			break;
		case DRIVE_CDROM:
			bRemovableMedia = TRUE;
			strcpy( VolumeName, "DVD/CD Drive (" );
			break;
		case DRIVE_RAMDISK:
			strcpy( VolumeName, "RAM Drive (" );
			bNoError = GetVolumeInformation( StorageDeviceRootDirectory, RemovableVolumeName, 255,
												&VolumeSerialNumber, &MaximumComponentLength, &FileSystemFlags, FileSystemName, 255 );
			if ( bNoError && strlen( RemovableVolumeName ) > 0 )
				{
				strcpy( VolumeName, RemovableVolumeName );
				strcat( VolumeName, " (" );
				}
			break;
		}
	if ( VolumeName[ strlen( VolumeName ) - 1 ] == '(' )
		{
		strcat( VolumeName, pStorageDeviceSpecification );
		strcat( VolumeName, ")" );
		}
	if ( bRemovableMedia )
		{
		bNoError = GetVolumeInformation( StorageDeviceRootDirectory, RemovableVolumeName, 255,
											&VolumeSerialNumber, &MaximumComponentLength, &FileSystemFlags, FileSystemName, 255 );
		if ( bNoError && strlen( RemovableVolumeName ) > 0 )
			{
			strcpy( VolumeName, RemovableVolumeName );
			strcat( VolumeName, " (" );
			strcat( VolumeName, pStorageDeviceSpecification );
			strcat( VolumeName, ")" );
			}
		}
	strcpy( pStorageDeviceLabel, VolumeName );
}



