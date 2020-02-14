// Module.h : Define data structures for general-purpose use by the program's
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
#pragma once

// Detect memory leaks while debugging.
#define _CRTDBG_MAP_ALLOC 
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <stdio.h>
#include "crtdbg.h"
#include <time.h>


#define MODULE_ERROR_INSUFFICIENT_MEMORY			1
#define MODULE_ERROR_CREATE_DIRECTORY				2

#define MODULE_ERROR_DICT_LENGTH					2

#define FILE_PATH_STRING_LENGTH					128
#define FULL_FILE_SPEC_STRING_LENGTH			256


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
#define MODULE_CONFIG				1
#define MODULE_STATUS				2
#define MODULE_SERVICE_INTERFACE	3
#define MODULE_MAIN					4
#define MODULE_MODULE				5

typedef struct ListElement
	{
	void					*pItem;
	struct ListElement		*pNextListElement;
	} LIST_ELEMENT;

typedef LIST_ELEMENT		*LIST_HEAD;



// Function prototypes.
//
void			InitModuleModule();

void			InitializeSoftwareModules();
BOOL			LocateOrCreateDirectory( char *pDirectorySpec );
void			LinkModuleToList( MODULE_INFO *pNewModuleInfo );
char			*GetModuleName( unsigned long RequestedModuleIndex );
void			CloseSoftwareModules();

BOOL			AppendToList( LIST_HEAD *pListHead, void *pItemToAppend );
BOOL			PrefixToList( LIST_HEAD *pListHead, void *pItemToPrefix );
BOOL			RemoveFromList( LIST_HEAD *pListHead, void *pListItemToRemove );
BOOL			IsItemInList( LIST_HEAD *pListHead, void *pItemToMatch );


