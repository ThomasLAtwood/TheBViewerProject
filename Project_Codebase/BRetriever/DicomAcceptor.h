// DicomAcceptor.h - Defines data structures and functions for Dicom association
// activities initiated by remote C-Echo and C-Store requestor.
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

#define DICOMACCEPT_ERROR_INSUFFICIENT_MEMORY			1
#define DICOMACCEPT_ERROR_CREATE_ASSOCIATION			2
#define DICOMACCEPT_ERROR_SOCKET_ACCEPT_FAILED			3
#define DICOMACCEPT_ERROR_SET_SOCKET_TIMEOUT			4
#define DICOMACCEPT_ERROR_PARSE_EXPECT_APPL_CONTEXT		5
#define DICOMACCEPT_ERROR_EVEN_PRES_CONTEXT_ID			6
#define DICOMACCEPT_ERROR_PARSE_EXPECT_ABSTRACT_SYNTAX	7
#define DICOMACCEPT_ERROR_PARSE_EXPECT_TRANSFER_SYNTAX	8
#define DICOMACCEPT_ERROR_PARSE_EXPECT_PRES_CONTEXT		9
#define DICOMACCEPT_ERROR_PARSE_EXPECT_USER_INFO		10
#define DICOMACCEPT_ERROR_PARSE_EXPECT_MAX_LENGTH		11
#define DICOMACCEPT_ERROR_PARSE_EXPECT_IMPL_CLASS_UID	12
#define DICOMACCEPT_ERROR_PARSE_EXPECT_IMPL_VER_NAME	13
#define DICOMACCEPT_ERROR_START_OP_THREAD				14
#define DICOMACCEPT_ERROR_LISTEN_SOCKET_SHUTDOWN		15

#define DICOMACCEPT_ERROR_DICT_LENGTH					15


#define	PRV_LISTENBACKLOG		50



// Function prototypes.
//
void				InitDicomAcceptorModule();
void				CloseDicomAcceptorModule();

BOOL				ListenForDicomAssociationRequests( PRODUCT_OPERATION *pProductOperation, char *pThisLocalNetworkAddress );
BOOL				InitializeSocketForListening( char *pNetworkAddress );
void				TerminateListeningSocket();
BOOL				RespondToConnectionRequests( PRODUCT_OPERATION *pProductOperation );
BOOL				PrepareCEchoResponseBuffer( DICOM_ASSOCIATION *pAssociation );
BOOL				PrepareCEchoCommandResponseBuffer( DICOM_ASSOCIATION *pAssociation, char **ppBuffer, unsigned long *pBufferSize );
BOOL				PrepareCStoreResponseBuffer( DICOM_ASSOCIATION *pAssociation, BOOL bNoError );
BOOL				PrepareCStoreCommandResponseBuffer( DICOM_ASSOCIATION *pAssociation, char **ppBuffer, unsigned long *pBufferSize, BOOL bNoError );
BOOL				PrepareAssociationAcceptanceBuffer( DICOM_ASSOCIATION *pAssociation );
BOOL				PreparePresentationContextReplyBuffer( DICOM_ASSOCIATION *pAssociation,
										PRESENTATION_CONTEXT_ITEM *pPresentationContextItem );
BOOL				PrepareAssociationRejectionBuffer( DICOM_ASSOCIATION *pAssociation );
BOOL				PrepareReleaseRequestReplyBuffer( DICOM_ASSOCIATION *pAssociation );
BOOL				PrepareAssociationProviderAbortBuffer( DICOM_ASSOCIATION *pAssociation );
BOOL				ParseAssociationRequestBuffer( DICOM_ASSOCIATION *pAssociation );
BOOL				ParseAssociationReleaseRequestBuffer( DICOM_ASSOCIATION *pAssociation );

