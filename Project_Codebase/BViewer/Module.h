// Module.h : Define data structures for general-purpose use by the BRetriever
//  software modules.
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

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// Detect memory leaks while debugging.
#define _CRTDBG_MAP_ALLOC 
#include <stdlib.h>
#include <stdio.h>
#include "crtdbg.h"
#include <windows.h>
#include <time.h>


#define MODULE_ERROR_INSUFFICIENT_MEMORY			1
#define MODULE_ERROR_CREATE_DIRECTORY				2

#define MODULE_ERROR_DICT_LENGTH					2


#define MAX_CFG_STRING_LENGTH			128
#define FILE_PATH_STRING_LENGTH			256
#define FULL_FILE_SPEC_STRING_LENGTH	256
#define MAX_LOGGING_STRING_LENGTH		256
#define MAX_EXTRA_LONG_STRING_LENGTH	512


typedef 	void (*MODULE_INIT_FUNCTION)();


typedef struct _Module_Info
	{
	unsigned long			ModuleIndex;
	char					*pModuleName;
	void					(*ModuleInitFunction)();
	void					(*ModuleCloseFunction)();
	struct _Module_Info		*pNextModuleInfo;
	} MODULE_INFO;


// Software module IDs.
#define MODULE_MAIN					1
#define MODULE_STATUS				2
#define MODULE_CONFIG				3
#define MODULE_DICTIONARY			4
#define MODULE_ABSTRACT				5
#define MODULE_MODULE				6
#define MODULE_IMAGE				7
#define MODULE_IMAGEVIEW			8
#define MODULE_HELP					9
#define MODULE_GRAPHICS				10
#define MODULE_DICOM				11
#define MODULE_IMPORT				12
#define MODULE_IMPORT_DICOMDIR		13
#define MODULE_INSTALL				14
#define MODULE_SIGNATURE			15
#define MODULE_CLIENT				16
#define MODULE_PRESET				17


typedef struct ListElement
	{
	void					*pItem;
	struct ListElement		*pNextListElement;
	} LIST_ELEMENT;

typedef LIST_ELEMENT		*LIST_HEAD;


typedef struct
	{
	int					nNumberOfBins;
	double				AverageBinValue;
	unsigned long		*pHistogramArray;
	unsigned long		ViewableHistogramArray[ 128 ];
	double				AverageViewableBinValue;
	} HISTOGRAM_DATA;


// Function prototypes.
//
void			InitModuleModule();

void			InitializeSoftwareModules();
void			LinkModuleToList( MODULE_INFO *pNewModuleInfo );
char			*GetModuleName( unsigned long RequestedModuleIndex );
void			CloseSoftwareModules();
void			InitMainModule();
void			CloseMainModule();

BOOL			AppendToList( LIST_HEAD *pListHead, void *pItemToAppend );
BOOL			PrefixToList( LIST_HEAD *pListHead, void *pItemToPrefix );
BOOL			RemoveFromList( LIST_HEAD *pListHead, void *pListItemToRemove );
BOOL			IsItemInList( LIST_HEAD *pListHead, void *pItemToMatch );
unsigned long	CountListItems( LIST_HEAD ListHead );
BOOL			DissolveList( LIST_HEAD *pListHead );
BOOL			EraseList( LIST_HEAD *pListHead );

BOOL			LocateOrCreateDirectory( char *pDirectorySpec );
void			GetDateAndTimeForFileName( char *pDateTimeString );
void			SubstituteCharacterInText( char *pTextString, char SearchForChar, char ReplacementChar );
__int64			GetFileSizeInBytes( char *pFullFileSpecification );
void			GetDriveLabel( char *pStorageDeviceSpecification, char *pStorageDeviceLabel );
void			IsolateFileName( char *pFilePath, char *pImageFileName );

// External references to module-specific functions:
//
void			InitDictionaryModule();
BOOL			ReadDictionaryFile( char *DicomDictionaryFileSpec );
void			InitAbstractModule();
void			InitImageModule();
void			InitImageViewModule();
void			InitHelpModule();
void			InitGraphicsAdapterModule();
void			InitDicomModule();
void			InitImportModule();
void			InitImportDicomdirModule();
void			InitInstallModule();
void			InitSignatureModule();
void			InitClientModule();
void			InitPresetModule();

