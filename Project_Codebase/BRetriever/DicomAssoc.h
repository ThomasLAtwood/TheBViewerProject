// DicomAssoc.h - Defines data structures and functions that support the
// Dicom association interactions.  This includes the structure of the
// various association syntatic units.
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
//	*[1] 04/17/2024 by Tom Atwood
//		Restored association structure member byte packing from 8 to 1.
//
//
#pragma once


#define DICOMASSOC_ERROR_INSUFFICIENT_MEMORY			1
#define DICOMASSOC_ERROR_INVALID_CALLED_AE_TITLE		2
#define DICOMASSOC_ERROR_TEMP_IMAGE_FILE_WRITE			3
#define DICOMASSOC_ERROR_TEMP_IMAGE_FILE_OPEN			4
#define DICOMASSOC_ERROR_TEMP_IMAGE_FILE_CLOSED			5
#define DICOMASSOC_ERROR_NO_PRES_CONTEXT_FOUND			6

#define DICOMASSOC_ERROR_DICT_LENGTH					6


#define MAX_ASSOCIATION_RECEIVED_BUFFER_SIZE			0x00010000	// 64K

#pragma pack(push)
#pragma pack(1)		// Pack calibration structure members on 1-byte boundaries.


typedef struct _PresContextItem
	{
	unsigned char				AcceptedPresentationContextID;
	unsigned long				AcceptedAbstractSyntaxIndex;
	unsigned long				AcceptedTransferSyntaxes;
	unsigned long				AcceptedTransferSyntaxIndex;
	struct _PresContextItem		*pNextPresContextItem;
	} PRESENTATION_CONTEXT_ITEM;


typedef struct
	{
	unsigned short		BufferType;
							#define BUFTYPE_UNSPECIFIED										0
							#define BUFTYPE_A_ASSOCIATE_RQ_HEADER_BUFFER					1
							#define BUFTYPE_A_APPLICATION_CONTEXT_BUFFER					2
							#define BUFTYPE_A_PRESENTATION_CONTEXT_HEADER_BUFFER			3
							#define BUFTYPE_A_ABSTRACT_SYNTAX_BUFFER						4
							#define BUFTYPE_A_TRANSFER_SYNTAX_BUFFER						5
							#define BUFTYPE_A_USER_INFO_ITEM_HEADER_BUFFER					6
							#define BUFTYPE_A_MAXIMUM_LENGTH_REQUEST_BUFFER					7
							#define BUFTYPE_A_MAXIMUM_LENGTH_REPLY_BUFFER					8
							#define BUFTYPE_A_IMPLEMENTATION_CLASS_UID_BUFFER				9
							#define BUFTYPE_A_IMPLEMENTATION_VERSION_NAME_BUFFER			10
							#define BUFTYPE_A_ASSOCIATE_AC_HEADER_BUFFER					11
							#define BUFTYPE_A_PRESENTATION_CONTEXT_REPLY_HEADER_BUFFER		12
							#define BUFTYPE_A_ASSOCIATE_RJ_BUFFER							13
							#define BUFTYPE_A_PRESENTATION_DATA_HEADER_BUFFER				14
							#define BUFTYPE_A_PRESENTATION_DATA_VALUE_ITEM_HEADER_BUFFER	15
							#define BUFTYPE_MESSAGE_FRAGMENT_HEADER_BUFFER					16
							#define BUFTYPE_A_RELEASE_RQ_BUFFER								17
							#define BUFTYPE_A_RELEASE_RP_BUFFER								18
							#define BUFTYPE_A_ABORT_BUFFER									19
	unsigned long		InsertedBufferLength;
	unsigned long		MaxBufferLength;
	BOOL				bInsertedLengthIsFinalized;
	void				*pBuffer;
	} BUFFER_LIST_ELEMENT;

// DIMSE buffer structure definitions.


typedef struct
	{
	unsigned char		PDU_Type;					// = 0x40
	unsigned char		Reserved1;					// = 0x00
	unsigned long		PDULength;					// The length in bytes of what follows.
	} PDU_BUFFER_HEADER;


typedef struct
	{
	unsigned char		PDU_Type;					//  = 0x01
	unsigned char		Reserved1;					//  = 0x00
	unsigned long		PDULength;					// The length in bytes of what follows.
	unsigned short		ProtocolVersion;			//  = 0x0001.  This is version 1, so only test the zero bit.
	unsigned short		Reserved2;					//  = 0x0000
	char				CalledAETitle[ 16 ];
	char				CallingAETitle[ 16 ];
	unsigned char		Reserved3[ 32 ];
	// Following this header, the buffer shall contain the following items:
	// one Application Context Item, one or more Presentation Context Items and one User Information Item.
	} A_ASSOCIATE_RQ_HEADER_BUFFER;


typedef struct
	{
	unsigned char		PDU_Type;						// = 0x10
	unsigned char		Reserved1;						// = 0x00
	unsigned short		Length;							// = 22			// The length in bytes of what follows.
	char				ApplicationContextName[ 64 ];	// To be filled with "1.2.840.10008.3.1.1.1 ", without the null terminator.
	} A_APPLICATION_CONTEXT_BUFFER;


typedef struct
	{
	unsigned char		PDU_Type;					// = 0x20
	unsigned char		Reserved1;					// = 0x00
	unsigned short		Length;						// The length in bytes of what follows.
	unsigned char		PresentationContextID;		// Presentation context ID values shall be odd integers between 1 and 255.
	unsigned char		Reserved2;					// = 0x00
	unsigned char		Reserved3;					// = 0x00
	unsigned char		Reserved4;					// = 0x00
	// Following this header, the buffer shall contain the following sub items: one Abstract Syntax and one or more Transfer Syntax(es).
	} A_PRESENTATION_CONTEXT_HEADER_BUFFER;


typedef struct
	{
	unsigned char		PDU_Type;					// = 0x30
	unsigned char		Reserved1;					// = 0x00
	unsigned short		Length;						// The length in bytes of what follows.
	char				AbstractSyntaxName[ 64 ];	// The abstract syntax name related to the proposed presentation context.
	} A_ABSTRACT_SYNTAX_BUFFER;


typedef struct
	{
	unsigned char		PDU_Type;					// = 0x40
	unsigned char		Reserved1;					// = 0x00
	unsigned short		Length;						// The length in bytes of what follows.
	char				TransferSyntaxName[ 64 ];	// A transfer syntax name proposed for the presentation context.
	} A_TRANSFER_SYNTAX_BUFFER;


typedef struct
	{
	unsigned char		PDU_Type;					// = 0x50
	unsigned char		Reserved1;					// = 0x00
	unsigned short		Length;						// The length in bytes of what follows.
	// Following this header, the buffer shall contain one or more user data sub items.
	} A_USER_INFO_ITEM_HEADER_BUFFER;


typedef struct
	{
	unsigned char		PDU_Type;					// = 0x51
	unsigned char		Reserved1;					// = 0x00
	unsigned short		Length;						// = 0x0004
	unsigned long		MaximumLengthReceivable;	// This parameter allows the association requestor to restrict the
													// maximum length of the variable field of the P-DATA-TF PDUs sent
													// by the acceptor on the association once established. This length
													// value is indicated as a number of bytes encoded as an unsigned
													// binary number. The value of (0) indicates that no maximum length
													// is specified. This maximum length value shall never be exceeded
													// by the PDU length values used in the PDU length field of the
													// P-DATA-TF PDUs received by the association requestor. Otherwise,
													// it shall be a protocol error.
	} A_MAXIMUM_LENGTH_REQUEST_BUFFER;


typedef struct
	{
	unsigned char		PDU_Type;					// = 0x51
	unsigned char		Reserved1;					// = 0x00
	unsigned short		Length;						// = 0x0004
	unsigned long		MaximumLengthReceivable;	// This parameter allows the association acceptor to restrict the
													// maximum length of the variable field of the P-DATA-TF PDUs sent
													// by the requestor on the association once established. This length
													// value is indicated as a number of bytes encoded as an unsigned
													// binary number. The value of (0) indicates that no maximum length
													// is specified. This maximum length value shall never be exceeded
													// by the PDU length values used in the PDU length field of the
													// P-DATA-TF PDUs received by the association acceptor. Otherwise,
													// it shall be a protocol error.
	} A_MAXIMUM_LENGTH_REPLY_BUFFER;


typedef struct
	{
	unsigned char		PDU_Type;					// = 0x52
	unsigned char		Reserved1;					// = 0x00
	unsigned short		Length;						// The length in bytes of what follows.
	char				ImplementationClassUID[ 64 ];	// The implementation class that identifies this software application.
	} A_IMPLEMENTATION_CLASS_UID_BUFFER;


typedef struct
	{
	unsigned char		PDU_Type;					// = 0x54
	unsigned char		Reserved1;					// = 0x00
	unsigned short		Length;						// The length in bytes of what follows.
	unsigned short		UIDLength;					// The length in bytes of the SOP-class-uid field.
	char				SOPClassUID[ 66 ];			// The SOP class that identifies the abstract syntax to which this item applies.
													// ... followed by a byte indicating support of the SCU role,
													// ... followed by a byte indicating support of the SCP role.
	} A_ROLE_SELECTION_BUFFER;


typedef struct
	{
	unsigned char		PDU_Type;					// = 0x55
	unsigned char		Reserved1;					// = 0x00
	unsigned short		Length;						// The length in bytes of what follows.
	char				ImplementationVersionName[ 16 ]; // The implementation class that identifies this software application.
	} A_IMPLEMENTATION_VERSION_NAME_BUFFER;


typedef struct
	{
	unsigned char		PDU_Type;					// = 0x02
	unsigned char		Reserved1;					// = 0x00
	unsigned long		PDULength;					// The length in bytes of what follows.
	unsigned short		ProtocolVersion;			// = 0x0001.  This is version 1, so only test the zero bit.
	unsigned short		Reserved2;					// = 0x0000
	char				CalledAETitle[ 16 ];		// Copied from the same field of the request, but not to be tested.
	char				CallingAETitle[ 16 ];		// Copied from the same field of the request, but not to be tested.
	unsigned char		Reserved3[ 32 ];			// Copied from the same field of the request, but not to be tested.
	// Following this header, the buffer shall contain the following items:
	// one Application Context Item, one or more Presentation Context Items and one User Information Item.
	} A_ASSOCIATE_AC_HEADER_BUFFER;


typedef struct
	{
	unsigned char		PDU_Type;					// = 0x21
	unsigned char		Reserved1;					// = 0x00
	unsigned short		Length;						// The length in bytes of what follows.
	unsigned char		PresentationContextID;		// Presentation context ID values shall be odd integers between 1 and 255.
	unsigned char		Reserved2;					// = 0x00
	unsigned char		Result;
			#define PRES_CONTEXT_RESULT_ACCEPTED					0
			#define PRES_CONTEXT_RESULT_USER_REJECTED				1
			#define PRES_CONTEXT_RESULT_REJECTED_WITHOUT_CAUSE		2
			#define PRES_CONTEXT_RESULT_ABSTRACT_SYNTAX_REJECTED	3
			#define PRES_CONTEXT_RESULT_TRANSFER_SYNTAX_REJECTED	4
	unsigned char		Reserved3;					// = 0x00
	// Following this header, the buffer shall contain the single selected Transfer Syntax.
	} A_PRESENTATION_CONTEXT_REPLY_HEADER_BUFFER;


typedef struct
	{
	unsigned char		PDU_Type;					// = 0x03
	unsigned char		Reserved1;					// = 0x00
	unsigned long		PDU_Length;					// = 0x00000004
	unsigned char		Reserved2;					// = 0x00
	unsigned char		Result;
			#define ASSOC_REJECTION_RESULT_PERMANENT			1
			#define ASSOC_REJECTION_RESULT_TRANSIENT			2
	unsigned char		Source;
			#define ASSOC_REJECTION_SOURCE_SERVICE_USER			1
			#define ASSOC_REJECTION_SOURCE_ASSOCIATION			2		// ACSE related function.
			#define ASSOC_REJECTION_SOURCE_PRESENTATION			3		// Presentation related function.
	unsigned char		Reason;
			#define ASSOC_REJECTION_REASON1_NOT_GIVEN			1		// For Source = 1, no reason given.
			#define ASSOC_REJECTION_REASON1_APP_CONTEXT			2		// For Source = 1, application context name not supported.
			#define ASSOC_REJECTION_REASON1_CALLING_AE_TITLE	3		// For Source = 1, calling AE title not recognized.
			#define ASSOC_REJECTION_REASON1_CALLED_AE_TITLE		7		// For Source = 1, called AE title not recognized.
			#define ASSOC_REJECTION_REASON2_NOT_GIVEN			1		// For Source = 2, no reason given.
			#define ASSOC_REJECTION_REASON2_PROTOCOL_VERSION	2		// For Source = 2, protocol version not supported.
			#define ASSOC_REJECTION_REASON3_RESERVED			0		// For Source = 3, reserved.
			#define ASSOC_REJECTION_REASON3_TEMP_CONGESTION		1		// For Source = 3, temporary congestion.
			#define ASSOC_REJECTION_REASON3_LOCAL_LIMIT			2		// For Source = 3, local limit exceeded.
	} A_ASSOCIATE_RJ_BUFFER;


typedef struct
	{
	unsigned char		PDU_Type;					// = 0x04
	unsigned char		Reserved1;					// = 0x00
	unsigned long		Length;						// The length in bytes of what follows.
	// Following this header, the buffer shall contain one or more Presentation data value Items(s).
	} A_PRESENTATION_DATA_HEADER_BUFFER;


typedef struct
	{
	unsigned long					Length;							// The length in bytes of what follows.
	unsigned char					PresentationContextID;			// Presentation context ID values shall be odd integers between 1 and 255.
	// Following this header, the buffer shall contain DICOM message information (command and/or data set) with a message control header.
	} A_PRESENTATION_DATA_VALUE_ITEM_HEADER_BUFFER;


typedef struct
	{
	unsigned char					MessageControlHeader;
			#define MESSAGE_CONTROL_COMMAND_INFO	0x01// If bit 0 is set to 1, the following fragment shall contain Message Command information.
														// If bit 0 is set to 0, the following fragment shall contain Message Data Set information.
			#define MESSAGE_CONTROL_LAST_FRAGMENT	0x02// If bit 1 is set to 1, the following fragment shall contain the last fragment of a
														//  Message Data Set or of a Message Command.
														// If bit 1 is set to 0, the following fragment does not contain the last fragment of a
														// Message Data Set or of a Message Command.
	// The Message Control Header, in the Transport data flow, is the 1st byte in each PDV. The Transfer Syntax, negotiated at association
	// establishment, defines the encoding for the Command/Data fragment.
	} MESSAGE_FRAGMENT_HEADER_BUFFER;


typedef struct
	{
	unsigned char		PDU_Type;					// = 0x05
	unsigned char		Reserved1;					// = 0x00
	unsigned long		PDU_Length;					// = 0x00000004
	unsigned long		Reserved2;					// = 0x00000000
	} A_RELEASE_RQ_BUFFER;


typedef struct
	{
	unsigned char		PDU_Type;					// = 0x06
	unsigned char		Reserved1;					// = 0x00
	unsigned long		PDU_Length;					// = 0x00000004
	unsigned long		Reserved2;					// = 0x00000000
	} A_RELEASE_RP_BUFFER;


typedef struct
	{
	unsigned char		PDU_Type;					//  = 0x04
	unsigned char		Reserved1;					//  = 0x00
	unsigned long		PDULength;					// The length in bytes of what follows.
	} DATA_TRANSFER_PDU_HEADER;

typedef struct
	{
	unsigned long		PDVItemLength;
	unsigned char		PresentationContextID;		// Presentation context ID values shall be odd integers between 1 and 255.
	unsigned char		MessageControlHeader;
							#define CONTAINS_COMMAND_MESSAGE		0x01
							#define LAST_MESSAGE_FRAGMENT			0x02
	} PRESENTATION_DATA_VALUE_HEADER;


typedef struct
	{
	unsigned char		PDU_Type;					// = 0x07
	unsigned char		Reserved1;					// = 0x00
	unsigned long		PDU_Length;					// = 0x00000004
	unsigned char		Reserved2;					// = 0x00
	unsigned char		Reserved3;					// = 0x00
	unsigned char		Source;
			#define ASSOC_ABORT_SOURCE_SERVICE_USER				0		// Service user initiated the abort.
			#define ASSOC_ABORT_SOURCE_SERVICE_PROVIDER			2		// Service provider initiated the abort.
	unsigned char		Reason;						// Should be zero for Source = 0;
			#define ASSOC_ABORT_REASON2_NOT_GIVEN				0		// No reason given.
			#define ASSOC_ABORT_REASON2_UNRECOGNIZED_PDU		1		// Unrecognized PDU.
			#define ASSOC_ABORT_REASON2_UNEXPECTED_PDU			2		// Unexpected PDU.
			#define ASSOC_ABORT_REASON2_UNRECOGNIZED_PDU_PARM	4		// Unrecognized PDU parameter.
			#define ASSOC_ABORT_REASON2_UNEXPECTED_PDU_PARM		5		// Unexpected PDU parameter.
			#define ASSOC_ABORT_REASON2_INVALID_PARM_VALUE		6		// Invalid PDU parameter value.
	} A_ABORT_BUFFER;



typedef struct
	{
	unsigned long		BitCode;		// Used to specify one or more transfer syntaxes to be
										// included in a presentation context.
	char				*pUIDString;	// Pointer to the transfer syntax UID.
	} TRANSFER_SYNTAX_TABLE_ENTRY;



typedef struct
	{
	unsigned long					SOP_ClassIndex;	// Internal binary symbol for the abstract syntax (SOP class) to be
													// included in a presentation context.
										// Definition of indices for SOP class UIDs:
										#define SOP_CLASS_VERIFICATION							0
										#define SOP_CLASS_COMPUTED_RADIOGRAPHY_IMAGE_STORAGE	1
										#define SOP_CLASS_COMPUTED_TOMOGRAPHY_IMAGE_STORAGE		2
										#define SOP_CLASS_MAGNETIC_RESONANCE_IMAGE_STORAGE		3
										#define SOP_CLASS_NUCLEAR_MEDICINE_IMAGE_STORAGE		4
										#define SOP_CLASS_ULTRASOUND_IMAGE_STORAGE				5
										#define SOP_CLASS_SECONDARY_CAPTURE_IMAGE_STORAGE		6
										#define SOP_CLASS_GRAYSCALE_SOFTCOPY_PRES_STORAGE		7
										#define SOP_CLASS_DIGITAL_XRAY_FOR_PRES_STORAGE			8

										#define NUMBER_OF_SOP_CLASS_IDS							9
										#define NUMBER_OF_ABSTRACT_SYNTAX_IDS					9

	BOOL							bSupported;		// Supported by the current application?
	char							*pUIDString;	// Pointer to the abstract syntax UID.
	char							*pExternalName;	// Pointer to a character string identifying this class to a human.
	char							ModalityCode[ 8 ];
	unsigned long					nSupportedTransferSyntaxCount;
	// The following transfer syntaxes are ordered by preference.  The first one in the list
	// is the most preferable and the last one is the least preferable.  If a transfer syntax
	// is proposed, but is not in this list, it is rejected.
	unsigned char					*pAcceptableTransferSyntaxes;
	} ABSTRACT_SYNTAX_TABLE_ENTRY;



typedef struct
	{
	ABSTRACT_SYNTAX_TABLE_ENTRY		*pAbstractSyntaxDescriptor;
	unsigned char					MostPreferableTransferSyntaxProposed;	// In the end, this is the one that is accepted.
	unsigned char					AcceptedTransferSyntax;
	unsigned char					AcceptedPresentationContext;
	} PRESENTATION_CONTEXT_SELECTOR;


typedef struct _ProposedPresentationContext
	{
	unsigned char							PresentationContextID;
	unsigned char							TransferSyntaxID;
	BOOL									bProposalAccepted;
	struct _ProposedPresentationContext		*pNextProposedPresentationContext;
	} PROPOSED_PRESENTATION_CONTEXT;


typedef struct
	{
	PRODUCT_OPERATION				*pProductOperation;
	char							RemoteNodeName[ MAX_CFG_STRING_LENGTH ];
	char							RemoteIPAddress[ MAX_CFG_STRING_LENGTH ];
	unsigned short					nRemotePortNumber;
	char							LocalAE_Title[ 18 ];
	char							RemoteAE_Title[ 18 ];

	// TCP communications.
	SOCKET							DicomAssociationSocket;
	char							*pSendBuffer;
	unsigned long					SendBufferLength;
	char							*pReceivedBuffer;
	unsigned long					ReceivedBufferLength;

	// Dicom association control parameters.
	unsigned short					CurrentStateID;
	unsigned short					EventIDReadyToBeProcessed;
	BOOL							bAssociationIsActive;
	BOOL							bReturnAfterAssociationEstablished;
	BOOL							bReadyToProceedWithAssociation;
	LIST_HEAD						AssociationBufferList;
	PRESENTATION_CONTEXT_SELECTOR	PresentationContextSelector;
	unsigned char					ActivePresentationContextID;
	unsigned short					AbstractSyntax;
	LIST_HEAD						ProposedPresentationContextList;
	BOOL							bAssociationSyntaxIsBigEndian;
	BOOL							bCurrentSyntaxIsLittleEndian;
	BOOL							bAssociationAccepted;
	// The following three entries are valid only if bAssociationAccepted is FALSE.
	BOOL							bRejectionIsTransient;
	unsigned char					RejectionSource;
	unsigned char					RejectionReason;

	BOOL							bReleaseReplyReceived;
	BOOL							bAssociationClosed;
	unsigned short					nAcceptedAbstractSyntax;
	char							*pImplementationClassUID;
	char							*pImplementationVersionName;

	// Dicom P-Data service:  CEcho and CStore command handling.
	unsigned short					SentCommandID;
	unsigned short					CurrentMessageID;	// ID assigned to most recent message sent.
	unsigned short					ReceivedCommandID;
	BOOL							bSentMessageExpectsResponse;
	LIST_HEAD						AssociatedImageList;
	ASSOCIATED_IMAGE_INFO			*pCurrentAssociatedImageInfo;
	} DICOM_ASSOCIATION;

#pragma pack(pop)					// *[1]

typedef struct
	{
	unsigned short		Group;					// = 0x0002
	unsigned short		Element;				// = 0x0000
	char				ValueRepresentation[2];	// = "UL"
	unsigned short		ValueLength;			// = 4L
	unsigned long		Value;
	} FILE_META_INFO_GROUP_LENGTH;


typedef struct
	{
	unsigned short		Group;				// = 0x0002
	unsigned short		Element;			// = 0x0001
	char				ValueRepresentation[2];	// = "OB"
	unsigned short		Reserved;			//
	unsigned long		ValueLength;		// = 4
	unsigned long		Value;				// = 0.1	VR = "OB"
	} FILE_META_INFO_VERSION;


typedef struct
	{
	unsigned short		Group;
	unsigned short		Element;
	unsigned long		ValueLength;
	} FILE_META_INFO_HEADER_IMPLICIT_VR;


typedef struct
	{
	unsigned short		Group;
	unsigned short		Element;
	char				ValueRepresentation[2];
	unsigned short		ValueLength;
	} FILE_META_INFO_HEADER_EXPLICIT_VR;


typedef struct
	{
	unsigned short		Group;
	unsigned short		Element;
	char				ValueRepresentation[2];	// = "OB"
	unsigned short		Reserved;
	unsigned long		ValueLength;
	} FILE_META_INFO_HEADER_EXPLICIT_VR_OB;





// Function prototypes.
//

void InitDicomAssocModule();

DICOM_ASSOCIATION			*CreateAssociationStructure( PRODUCT_OPERATION *pProductOperation );
void						DeleteAssociationStructure( DICOM_ASSOCIATION *pAssociation );
PRESENTATION_CONTEXT_ITEM	*CreatePresentationContextItem();
void						AssociationSwapBytes( DICOM_ASSOCIATION *pAssociation, void *pData, long nValueSize );
BOOL						PrepareApplicationContextBuffer( DICOM_ASSOCIATION *pAssociation );
BOOL						PrepareTransferSyntaxBuffer( DICOM_ASSOCIATION *pAssociation, int TransferSyntaxIndex );
BOOL						PrepareUserInformationBuffer( DICOM_ASSOCIATION *pAssociation );
BOOL						PrepareMaximumLengthBuffer( DICOM_ASSOCIATION *pAssociation );
BOOL						PrepareImplementationClassUIDBuffer( DICOM_ASSOCIATION *pAssociation );
BOOL						PrepareImplementationVersionNameBuffer( DICOM_ASSOCIATION *pAssociation );
BOOL						AppendSubitemBuffer( DICOM_ASSOCIATION *pAssociation, BUFFER_LIST_ELEMENT *pParentBufferDescriptor );
BOOL						ParseReceivedDicomBuffer( DICOM_ASSOCIATION *pAssociation );
BOOL						ParseAssociationReceivedDataSetBuffer( DICOM_ASSOCIATION *pAssociation,
													BOOL bFirstBufferInSeries, BOOL *pbNeedsMoreData, unsigned long *pPrevPDUBytesToBeRead );
void						MoveFileForAccess( char *pExistingFilePath, char *pBareFileName );
BOOL						ParseAssociationAbortBuffer( DICOM_ASSOCIATION *pAssociation );
BOOL						RegisterProposedAbstractSyntax( DICOM_ASSOCIATION *pAssociation, char *pAbstractSyntaxUID, unsigned short Length );
BOOL						RegisterProposedTransferSyntax( DICOM_ASSOCIATION *pAssociation, char *pTransferSyntaxUID, unsigned char PresentationContextID );
char						*GetSOPClassUID( int nSOPClassIndex );
unsigned short				GetAbstractSyntaxIndex( char *pAbstractSyntaxUID, unsigned short Length );
PRESENTATION_CONTEXT_ITEM	*GetPresentationContextInfo( DICOM_ASSOCIATION *pAssociation, unsigned char PresentationContextID );
char						*GetTransferSyntaxUID( int nTransferSyntaxIndex );
unsigned long				GetTransferSyntaxBitCode( int nTransferSyntaxIndex );
unsigned short				GetTransferSyntaxIndex( char *pTransferSyntaxUID, unsigned short Length );
BOOL						ComposeFileMetaInformation( DICOM_ASSOCIATION *pAssociation,
												PRESENTATION_CONTEXT_ITEM *pPresentationContextItem,
												DICOM_HEADER_SUMMARY *pDicomHeaderSummary,
												char **ppGroup2Buffer, unsigned long *pBufferSize );

