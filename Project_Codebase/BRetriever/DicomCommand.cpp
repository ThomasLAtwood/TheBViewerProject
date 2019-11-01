// DicomCommand.cpp - Implements functions that support the
// Dicom commands and responses:  C-Echo and C-Store.
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
#include "ReportStatus.h"
#include "Dicom.h"
#include "Configuration.h"
#include "Operation.h"
#include "ProductDispatcher.h"
#include "WinSocketsAPI.h"
#include "DicomAssoc.h"
#include "DicomCommand.h"
#include "DicomAcceptor.h"
#include "DicomInitiator.h"
#include "DicomCommunication.h"


//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		DicomCommandModuleInfo = { MODULE_DICOMCMD, "Dicom Command Module", InitDicomCommandModule, CloseDicomCommandModule };


static ERROR_DICTIONARY_ENTRY	DicomCommandErrorCodes[] =
			{
				{ DICOMCMD_ERROR_UNEXPECTED_MESSAGE			, "The response received does not match the message most recently sent."	},
				{ DICOMCMD_ERROR_MATCHING_SOP_CLASS			, "The response SOP class does not match the current association."			},
				{ DICOMCMD_ERROR_UID_EXCESSIVE_LENGTH		, "The length of a received UID exceeded the available buffer space."		},
				{ DICOMCMD_ERROR_UNEXPECTED_DICOM_ELEMENT	, "An unexpected Dicom group/element could not be parsed."					},
				{ 0											, NULL }
			};


static ERROR_DICTIONARY_MODULE		DicomCommandStatusErrorDictionary =
										{
										MODULE_DICOMCMD,
										DicomCommandErrorCodes,
										DICOMCMD_ERROR_DICT_LENGTH,
										0
										};

static BOOL					bSocketsEnabled = FALSE;


// This function must be called before any other function in this module.
void InitDicomCommandModule()
{
	LinkModuleToList( &DicomCommandModuleInfo );
	RegisterErrorDictionary( &DicomCommandStatusErrorDictionary );
}


void CloseDicomCommandModule()
{
	if ( bSocketsEnabled )
		TerminateWindowsSockets();
	bSocketsEnabled = FALSE;
}


BOOL RespondToReceivedDicomMessage( DICOM_ASSOCIATION *pAssociation )
{
	BOOL							bNoError = TRUE;

	if ( pAssociation -> bReleaseReplyReceived )
		{
		LogMessage( "Association release reply received.", MESSAGE_TYPE_SUPPLEMENTARY );
		pAssociation -> EventIDReadyToBeProcessed = EVENT_RECEPTION_OF_ASSOCIATION_RELEASE_REPLY;
		}
	else
		switch( pAssociation -> ReceivedCommandID )
			{
			case DICOM_CMD_DATA:
				break;
			case DICOM_CMD_ECHO:
				LogMessage( "Preparing C-Echo Response.", MESSAGE_TYPE_SUPPLEMENTARY );
				bNoError = PrepareCEchoResponseBuffer( pAssociation );
				if ( bNoError )
					{
					pAssociation -> EventIDReadyToBeProcessed = EVENT_THIS_NODE_REQUESTS_TO_SEND_MESSAGE;
					pAssociation -> bSentMessageExpectsResponse = TRUE;		// Expect a release request or another C-Echo command.
					LogMessage( "@@@ Responding to external verification request.", MESSAGE_TYPE_NORMAL_LOG );
					}
				break;
			case DICOM_CMD_ECHO_RESPONSE:
				LogMessage( "Preparing release request following C-Echo Response.", MESSAGE_TYPE_SUPPLEMENTARY );
				pAssociation -> EventIDReadyToBeProcessed = EVENT_THIS_NODE_REQUESTS_ASSOCIATION_RELEASE;
				break;
			case DICOM_CMD_STORE:
				LogMessage( "Received C-Store Command.", MESSAGE_TYPE_SUPPLEMENTARY );
				break;
			case DICOM_CMD_STORE_RESPONSE:
				break;
			case DICOM_CMD_RELEASE_ASSOCIATION:
				pAssociation -> EventIDReadyToBeProcessed = EVENT_RECEPTION_OF_ASSOCIATION_RELEASE_REQUEST;
				break;
			}

	return bNoError;
}


BOOL ParseReceivedDataSetBuffer( DICOM_ASSOCIATION *pAssociation, char *pBuffer, unsigned long BufferSize )
{
	BOOL							bNoError = TRUE;
	ASSOCIATED_IMAGE_INFO			*pAssociatedImageInfo;
	DATA_ELEMENT_HEADER_IMPLICIT_VR	BufferElement;
	char							*pBufferReadPoint;
	char							*pEndOfBuffer;
	char							*pSOPClassUID;
	char							*pSOPInstanceUID;
	unsigned short					CommandCodeReceived;
	unsigned short					MessageIDReceived;
	unsigned short					PriorityReceived;
	unsigned short					DataSetTypeReceived;
	unsigned short					StatusReceived;
	unsigned long					ValueLength;
	char							Message[ 1096 ];

	pBufferReadPoint = pBuffer;
	pEndOfBuffer = pBuffer + BufferSize;

	while ( pBufferReadPoint < pEndOfBuffer )
		{
		memcpy( (char*)&BufferElement, pBufferReadPoint, sizeof(DATA_ELEMENT_HEADER_IMPLICIT_VR) );
		pBufferReadPoint += sizeof(DATA_ELEMENT_HEADER_IMPLICIT_VR);
		switch ( BufferElement.Group )
			{
			case 0x0000:
				switch ( BufferElement.Element )
					{
					case 0x0000:		// DATA_ELEMENT_GROUP_LENGTH
						break;
					case 0x0002:		// Affected SOP Class UID.
						pSOPClassUID = GetSOPClassUID( pAssociation -> nAcceptedAbstractSyntax );
						break;
					case 0x0100:		// DATA_ELEMENT_COMMAND_FIELD
						CommandCodeReceived = *((unsigned short*)pBufferReadPoint);
						if ( CommandCodeReceived == 0x0030 )
							{
							pAssociation -> ReceivedCommandID = DICOM_CMD_ECHO;
							LogMessage( "Received Dicom C-ECHO command", MESSAGE_TYPE_SUPPLEMENTARY );
							}
						else if ( CommandCodeReceived == 0x8030 )
							{
							pAssociation -> ReceivedCommandID = DICOM_CMD_ECHO_RESPONSE;
							LogMessage( "Received Dicom C-ECHO response", MESSAGE_TYPE_SUPPLEMENTARY );
							}
						else if ( CommandCodeReceived == 0x0001 )
							{
							pAssociation -> ReceivedCommandID = DICOM_CMD_STORE;

							LogMessage( "Received Dicom C-STORE command", MESSAGE_TYPE_SUPPLEMENTARY );
							pAssociatedImageInfo = CreateAssociatedImageInfo();
							if ( pAssociatedImageInfo == 0 )
								bNoError = FALSE;
							if ( bNoError )
								{
								pAssociation -> pCurrentAssociatedImageInfo = pAssociatedImageInfo;
								AppendToList( &pAssociation -> AssociatedImageList, (void*)pAssociatedImageInfo );
								}
							}
						else if ( CommandCodeReceived == 0x8001 )
							{
							pAssociation -> ReceivedCommandID = DICOM_CMD_STORE_RESPONSE;
							LogMessage( "Received Dicom C-STORE response", MESSAGE_TYPE_SUPPLEMENTARY );
							}
						break;
					case 0x0110:		// DATA_ELEMENT_MESSAGE_ID
						MessageIDReceived = *((unsigned short*)pBufferReadPoint);
						pAssociation -> CurrentMessageID = MessageIDReceived;
						break;
					case 0x0120:		// DATA_ELEMENT_MESSAGE_ID_RESPONDING
						MessageIDReceived = *((unsigned short*)pBufferReadPoint);
						if ( MessageIDReceived != pAssociation -> CurrentMessageID )
							{
							bNoError = FALSE;
							RespondToError( MODULE_DICOMCMD, DICOMCMD_ERROR_UNEXPECTED_MESSAGE );
							}
						break;
					case 0x0700:		// DATA_ELEMENT_PRIORITY
						PriorityReceived = *((unsigned short*)pBufferReadPoint);
						break;
					case 0x0800:		// DATA_ELEMENT_DATASET_TYPE
						DataSetTypeReceived = *((unsigned short*)pBufferReadPoint);
						break;
					case 0x0900:		// DATA_ELEMENT_STATUS
						StatusReceived = *((unsigned short*)pBufferReadPoint);
						break;
					case 0x1000:		// Affected SOP Instance UID.
						pSOPInstanceUID = pBufferReadPoint;
						ValueLength = BufferElement.ValueLength;
						if ( ValueLength < MAX_FILE_SPEC_LENGTH )
							{
							memcpy( pAssociation -> pCurrentAssociatedImageInfo -> CurrentDicomFileName, pSOPInstanceUID, ValueLength );
							pAssociation -> pCurrentAssociatedImageInfo -> CurrentDicomFileName[ ValueLength ] = '\0';
							sprintf( Message, "    Affected SOP Instance:  %s", pAssociation -> pCurrentAssociatedImageInfo -> CurrentDicomFileName );
							LogMessage( Message, MESSAGE_TYPE_SUPPLEMENTARY );
							}
						else
							{
							bNoError = FALSE;
							RespondToError( MODULE_DICOMCMD, DICOMCMD_ERROR_UID_EXCESSIVE_LENGTH );
							}
						break;
					default:
						bNoError = FALSE;
						RespondToError( MODULE_DICOMCMD, DICOMCMD_ERROR_UNEXPECTED_DICOM_ELEMENT );
						break;
					}
				break;
			}
		pBufferReadPoint += BufferElement.ValueLength;
		}

	return bNoError;
}

