// ProductDispatcher.h : Defines the data structures and functions related to
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
#pragma once

#define DISPATCH_ERROR_INSUFFICIENT_MEMORY			1
#define DISPATCH_ERROR_CREATE_PRODUCT_SEMAPHORE		2
#define DISPATCH_ERROR_PRODUCT_SEMAPHORE_TIMEOUT	3
#define DISPATCH_ERROR_PRODUCT_SEMAPHORE_WAIT		4
#define DISPATCH_ERROR_PRODUCT_SEMAPHORE_RELEASE	5

#define DISPATCH_ERROR_DICT_LENGTH					5


#define PRODUCT_QUEUE_ACCESS_TIMEOUT		60000



// Function prototypes.
//
void					InitProductDispatcherModule();
void					CloseProductDispatcherModule();

void					CreateProductDeletionOperation();
void					InitProductInfoStructure( PRODUCT_QUEUE_ITEM *pProductItem );
BOOL					InitNewProductQueueItem( PRODUCT_OPERATION *pProductOperation, PRODUCT_QUEUE_ITEM **ppProductItem );
void					DeallocateProductInfo( PRODUCT_QUEUE_ITEM *pProductItem );
PRODUCT_QUEUE_ITEM		*GetDuplicateProductQueueEntry( PRODUCT_QUEUE_ITEM *pNewProductItem );
BOOL					QueueProductForTransfer( PRODUCT_OPERATION *pProductOperation, PRODUCT_QUEUE_ITEM **ppProductItem,
																						PRODUCT_QUEUE_ITEM **ppDuplicateProductItem );
PRODUCT_QUEUE_ITEM		*GetMatchingProductEntry( unsigned long LocalProductIndex );
void					RemoveProductFromQueue( unsigned long LocalProductIndex );
void					NotifyUserOfProductError( PRODUCT_QUEUE_ITEM *pProductItem );
void					ProcessProductQueueItems();
BOOL					DeleteSourceProduct( PRODUCT_OPERATION *pProductOperation, PRODUCT_QUEUE_ITEM **ppProductItem );
