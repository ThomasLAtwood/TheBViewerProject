// DicomCommunication.h - Defines data structures and functions that support the
// communications state machine.  All Dicom communications are handled through
// the state machine.
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

#define DICOMSTATE_ERROR_INSUFFICIENT_MEMORY			1
#define DICOMSTATE_ERROR_INVALID_FSM_EVENT				2
#define DICOMSTATE_ERROR_INVALID_FSM_STATE				3
#define DICOMSTATE_ERROR_NO_FSM_ACTION_DEFINED			4
#define DICOMSTATE_ERROR_PARSE_EXPECT_ASSOC_REQUEST		5
#define DICOMSTATE_ERROR_ASSOCIATION_ABORT				6

#define DICOMSTATE_ERROR_DICT_LENGTH					6


#define DEFAULT_DICOM_PORT_NUMBER						204

//
// State Machine definitions:
//

// Definition of communication events.
#define EVENT_NO_EVENT_ID_SPECIFIED						0xffff
	// Association establishment events.
#define EVENT_RECEPTION_OF_TRANSPORT_CONNECTION_REQUEST	0
#define EVENT_RECEPTION_OF_ASSOCIATION_REQUEST			1
#define EVENT_THIS_NODE_DECIDES_TO_ACCEPT_ASSOCIATION	2
#define EVENT_THIS_NODE_DECIDES_TO_REJECT_ASSOCIATION	3
	// Data transfer events.
#define EVENT_THIS_NODE_REQUESTS_TO_SEND_MESSAGE		4	// "Messages" are sent and received in the form of "PDU"s.
#define EVENT_RECEPTION_OF_MESSAGE						5	// "Messages" are sent and received in the form of "PDU"s.
	// Association release events.
#define EVENT_THIS_NODE_REQUESTS_ASSOCIATION_RELEASE	6
#define EVENT_RECEPTION_OF_ASSOCIATION_RELEASE_REQUEST	7
#define EVENT_RECEPTION_OF_ASSOCIATION_RELEASE_REPLY	8
#define EVENT_THIS_NODE_RESPONDS_TO_RELEASE_REQUEST		9
	// Association abort events.
#define EVENT_THIS_NODE_REQUESTS_ASSOCIATION_ABORT		10
#define EVENT_RECEPTION_OF_ASSOCIATION_ABORT_REQUEST	11
#define EVENT_THIS_NODE_TRANSPORT_CONNECTION_CLOSED		12
#define EVENT_ASSOCIATION_EVENT_TIMER_HAS_EXPIRED		13
#define EVENT_INVALID_MESSAGE_FORMAT_RECEIVED			14

#define NUMBER_OF_EVENT_IDS								15


// Definition of communication states.
#define STATE_NO_STATE_SPECIFIED						0xffffu

#define STATE1_IDLE										0u
#define STATE2_TRANSPORT_CONNECTION_LISTENING			1u		// Actually, a connection request has been accepted and TCP is connected.
#define STATE3_AWAITING_THIS_NODE_ASSOCIATION_RESPONSE	2u
#define STATE4_AWAITING_THIS_NODE_TRANSPORT_CONNECTION	3u
#define STATE6_READY_FOR_DATA_TRANSFER					4u
#define STATE7_AWAITING_REPLY_TO_ASSOC_RELEASE_REQUEST	5u
#define STATE8_AWAITING_THIS_NODE_RELEASE_RESPONSE		6u
#define STATE9_REL_COLLIDE_REQ_AWAIT_THIS_NODE_REPLY	7u
#define STATE10_REL_COLLIDE_ACC_AWAIT_REPLY_RECEPTION	8u
#define STATE11_REL_COLLIDE_REQ_AWAIT_REPLY_RECEPTION	9u
#define STATE12_REL_COLLIDE_ACC_AWAIT_THIS_NODE_RESPONSE 10u
#define STATE13_AWAITING_THIS_NODE_TRANSPORT_CLOSE		11u		// The association is already closed.

#define NUMBER_OF_STATE_IDS								12									


// Definition of communication actions.
#define ACTION_NO_ACTION_SPECIFIED						0xffffu

#define ACTION_AE_1_INFORM_THIS_NODE_CONNECTION_REQUESTED	0u
#define ACTION_AE_2_RESPOND_TO_ASSOCIATION_REQUEST			1u
#define ACTION_AE_3_ACCEPT_ASSOCIATION_REQUEST				2u
#define ACTION_AE_4_REJECT_ASSOCIATION_REQUEST				3u

#define ACTION_DT_1_SEND_MESSAGE							4u
#define ACTION_DT_2_PROCESS_RECEIVED_MESSAGE				5u

#define ACTION_AR_1_SEND_ASSOCIATION_RELEASE_REQUEST		6u
#define ACTION_AR_2_INFORM_THIS_NODE_RELEASE_REQUESTED		7u
#define ACTION_AR_3_INFORM_THIS_NODE_RELEASED_CLOSE_CONNECT	8u
#define ACTION_AR_4_SEND_TIMED_REPLY_TO_RELEASE_REQUEST		9u
#define ACTION_AR_5_STOP_TIMER_AND_GO_IDLE					10u
#define ACTION_AR_6_INFORM_THIS_NODE_MESSAGE_RECEIVED		11u	// A data packet was received during association release processing.
#define ACTION_AR_7_SEND_MESSAGE_DURING_RELEASE_PROCESSING	12u
#define ACTION_AR_8_INFORM_THIS_NODE_COLLISION_RELEASE_REQ	13u
#define ACTION_AR_9_SEND_REPLY_TO_RELEASE_REQUEST			14u
#define ACTION_AR_10_INFORM_THIS_NODE_ASSOCIATION_RELEASED	15u

#define ACTION_AA_1_SEND_TIMED_ASSOCIATION_ABORT			16u
#define ACTION_AA_2_STOP_TIMER_CLOSE_TRANSPORT_CONNECTION	17u
#define ACTION_AA_3_INFORM_THIS_NODE_ABORTED_CLOSE_CONNECT	18u
#define ACTION_AA_4_SEND_PROVIDER_ASSOCIATION_ABORT			19u
#define ACTION_AA_5_STOP_TIMER_AND_GO_IDLE					20u
#define ACTION_AA_6_IGNORE_RECEIVED_MESSAGE					21u
#define ACTION_AA_7_SEND_ASSOCIATION_ABORT					22u
#define ACTION_AA_8_SEND_PROVIDER_ABORT_INFORM_THIS_NODE	23u

#define NUMBER_OF_ACTION_IDS								24									


typedef struct
	{
	unsigned short		CurrentStateID;
	unsigned short		ActionID;
	unsigned short		NextStateID;
	} STATE_TRANSITION;


typedef		BOOL ( *FSM_ACTION_FUNCTION )( DICOM_ASSOCIATION *pAssociation );


typedef struct
	{
	FSM_ACTION_FUNCTION		ActionFunction;
	char					*pActionFunctionName;
	} ACTION_TABLE_ENTRY;




// Function prototypes.
//
void			InitDicomStateModule();
void			CloseDicomStateModule();

BOOL			StartNewAssociation( DICOM_ASSOCIATION *pAssociation );
BOOL			ExecuteStateTransition( DICOM_ASSOCIATION *pAssociation );
BOOL			ACTION_AE_1_InformThisNodeAssociationRequested( DICOM_ASSOCIATION *pAssociation );
BOOL			ACTION_AE_2_RespondToAssociationRequest( DICOM_ASSOCIATION *pAssociation );
BOOL			ACTION_AE_3_AcceptAssociationRequest( DICOM_ASSOCIATION *pAssociation );
BOOL			ACTION_AE_4_RejectAssociationRequest( DICOM_ASSOCIATION *pAssociation );

BOOL			ACTION_DT_1_SendMessage( DICOM_ASSOCIATION *pAssociation );
BOOL			ACTION_DT_2_ProcessReceivedMessage( DICOM_ASSOCIATION *pAssociation );

BOOL			ACTION_AR_1_SendAssociationReleaseRequest( DICOM_ASSOCIATION *pAssociation );
BOOL			ACTION_AR_2_InformThisNodeReleaseRequested( DICOM_ASSOCIATION *pAssociation );
BOOL			ACTION_AR_3_InformThisNodeReleasedAndCloseConnection( DICOM_ASSOCIATION *pAssociation );
BOOL			ACTION_AR_4_SendTimedReplyToReleaseRequest( DICOM_ASSOCIATION *pAssociation );
BOOL			ACTION_AR_5_StopTimerAndGoIdle( DICOM_ASSOCIATION *pAssociation );
BOOL			ACTION_AR_6_InformThisNodeMessageReceived( DICOM_ASSOCIATION *pAssociation );
BOOL			ACTION_AR_7_SendMessageDuringReleaseProcessing( DICOM_ASSOCIATION *pAssociation );
BOOL			ACTION_AR_8_InformThisNodeCollisionReleaseRequested( DICOM_ASSOCIATION *pAssociation );
BOOL			ACTION_AR_9_SendReplyToReleaseRequest( DICOM_ASSOCIATION *pAssociation );
BOOL			ACTION_AR_10_InformThisNodeAssociationReleased( DICOM_ASSOCIATION *pAssociation );

BOOL			ACTION_AA_1_SendTimedAssociationAbort( DICOM_ASSOCIATION *pAssociation );
BOOL			ACTION_AA_2_StopTimerAndCloseTransportConnection( DICOM_ASSOCIATION *pAssociation );
BOOL			ACTION_AA_3_InformThisNodeAssociationAbortedAndCloseConnection( DICOM_ASSOCIATION *pAssociation );
BOOL			ACTION_AA_4_SendProviderAssociationAbort( DICOM_ASSOCIATION *pAssociation );
BOOL			ACTION_AA_5_StopTimerAndGoIdle( DICOM_ASSOCIATION *pAssociation );
BOOL			ACTION_AA_6_IgnoreReceivedMessage( DICOM_ASSOCIATION *pAssociation );
BOOL			ACTION_AA_7_SendAssociationAbort( DICOM_ASSOCIATION *pAssociation );
BOOL			ACTION_AA_8_SendProviderAbortAndInformThisNode( DICOM_ASSOCIATION *pAssociation );

BOOL			SendDicomBuffer( DICOM_ASSOCIATION *pAssociation, BOOL bDeallocateAfterSending );
BOOL			ReceiveDicomBuffer( DICOM_ASSOCIATION *pAssociation );

