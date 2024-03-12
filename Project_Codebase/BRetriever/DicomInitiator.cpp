// DicomInitiator.cpp - Implements functions for Dicom association
// activities initiated by BRetriever.
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
#include "Configuration.h"
#include "Operation.h"
#include "WinSocketsAPI.h"
#include "DicomAssoc.h"
#include "DicomInitiator.h"


//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		DicomInitiatorModuleInfo = { MODULE_DICOMINITIATE, "Dicom Association Initiation Module", InitDicomInitiatorModule, CloseDicomInitiatorModule };


static ERROR_DICTIONARY_ENTRY	DicomInitiatorErrorCodes[] =
			{
				{ DICOMINITIATE_ERROR_INSUFFICIENT_MEMORY			, "There is not enough memory to allocate a data structure." },
				{ 0													, NULL }
			};


static ERROR_DICTIONARY_MODULE		DicomInitiatorStatusErrorDictionary =
										{
										MODULE_DICOMINITIATE,
										DicomInitiatorErrorCodes,
										DICOMINITIATE_ERROR_DICT_LENGTH,
										0
										};



// This function must be called before any other function in this module.
void InitDicomInitiatorModule()
{
	LinkModuleToList( &DicomInitiatorModuleInfo );
	RegisterErrorDictionary( &DicomInitiatorStatusErrorDictionary );
}


void CloseDicomInitiatorModule()
{
}


BOOL PrepareAssociationReleaseRequestBuffer( DICOM_ASSOCIATION *pAssociation )
{
	BOOL							bNoError = TRUE;
	A_RELEASE_RQ_BUFFER				*pBuffer;

	pAssociation -> bReleaseReplyReceived = FALSE;
	if ( bNoError )
		{
		pBuffer = (A_RELEASE_RQ_BUFFER*)malloc( sizeof(A_RELEASE_RQ_BUFFER) );
		if ( pBuffer == 0 )
			{
			bNoError = FALSE;
			RespondToError( MODULE_DICOMINITIATE, DICOMINITIATE_ERROR_INSUFFICIENT_MEMORY );
			}
		else
			{
			memset( (char*)pBuffer, '\0', sizeof(A_RELEASE_RQ_BUFFER) );
			pBuffer -> PDU_Type = 0x05;
			pBuffer -> Reserved1 = 0x00;
			pBuffer -> PDU_Length = 0x00000004;
			AssociationSwapBytes( pAssociation, &pBuffer -> PDU_Length, 4 );
			pBuffer -> Reserved2 = 0x00000000;
			}
		}
	if ( bNoError )
		{
		pAssociation -> pSendBuffer = (char*)pBuffer;
		pAssociation -> SendBufferLength = sizeof(A_RELEASE_RQ_BUFFER);
		}

	return bNoError;
}


BOOL ParseAssociationReleaseReplyBuffer( DICOM_ASSOCIATION *pAssociation )
{
	BOOL										bNoError = TRUE;
	A_RELEASE_RP_BUFFER							*pReplyBuffer;
	long										RemainingBufferLength;
	char										TextMsg[ MAX_LOGGING_STRING_LENGTH ];

	RemainingBufferLength = pAssociation -> ReceivedBufferLength;
	pReplyBuffer = (A_RELEASE_RP_BUFFER*)pAssociation -> pReceivedBuffer;
	pAssociation -> bReleaseReplyReceived = TRUE;

	if ( pAssociation -> ReceivedBufferLength != sizeof(A_RELEASE_RP_BUFFER) )
		{
		_snprintf_s( TextMsg, MAX_LOGGING_STRING_LENGTH, _TRUNCATE, "%d bytes remained unread from the received association release reply buffer.",	// *[1] Replaced sprintf() with _snprintf_s.
										pAssociation -> ReceivedBufferLength - sizeof(A_RELEASE_RP_BUFFER) );
		LogMessage( TextMsg, MESSAGE_TYPE_SUPPLEMENTARY );
		}
	if ( pAssociation -> pReceivedBuffer != 0 )
		{
		free( pAssociation -> pReceivedBuffer );
		pAssociation -> pReceivedBuffer = 0;
		pAssociation -> ReceivedBufferLength = 0L;
		}

	return bNoError;
}





