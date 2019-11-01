// Operation.h : Defines the data structures and functions related to
//	image file processing operations, such as receiving the file and
//	extracting the image.
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

#include <Wininet.h>


#define OPERATIONS_ERROR_CREATE_SEMAPHORE			1
#define OPERATIONS_ERROR_SEMAPHORE_WAIT				2
#define OPERATIONS_ERROR_OPERATION_DISABLED			3
#define OPERATIONS_ERROR_START_OP_THREAD			4

#define OPERATIONS_ERROR_DICT_LENGTH				4


// Function prototypes.
//
void						InitProductOperationsModule();
void						CloseProductOperationsModule();

PRODUCT_OPERATION			*CreateProductOperation();
void						DeleteProductOperation( PRODUCT_OPERATION *pProductOperation );
void						ControlProductOperations();
BOOL						LaunchOperation( PRODUCT_OPERATION *pProductOperation );
void						EnterOperationCycleWaitInterval( PRODUCT_OPERATION *pProductOperation,
																BOOL bEnableDependentOperations, BOOL *pbTerminateOperation );
BOOL						CheckForOperationTerminationRequest( PRODUCT_OPERATION *pProductOperation );
void						CloseOperation( PRODUCT_OPERATION *pProductOperation );
void						TerminateAllOperations();

