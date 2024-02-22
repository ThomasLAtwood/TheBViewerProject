// Routing.h
//
#pragma once

typedef BOOL ( *PRODUCT_DELETE_FUNCTION )( VOID *pProductItem );


#define OPERATION_RECEIVE		0x00000001
#define OPERATION_COPY			0x00000002
#define OPERATION_SEND			0x00000004
#define PRODUCT_SELECTION		0x00000008
#define OPERATION_LAUNCH		0x00000010
#define DISCR_PRODUCT_EXAM		0x00000040
#define DISCR_PRODUCT_IMAGE		0x00000080
#define DISCR_PRODUCT_DICTATION	0x00000100
#define DISCR_FORMAT_DICOM		0x00000200
#define DISCR_FORMAT_JPEG		0x00000400
#define DISCR_FORMAT_ZIP		0x00000800
#define DISCR_FORMAT_TEXT		0x00001000
#define DISCR_MODALITY_CR		0x00002000
#define DISCR_MODALITY_CT		0x00004000
#define DISCR_MODALITY_MR		0x00008000
#define DISCR_MODALITY_NM		0x00010000
#define DISCR_MODALITY_US		0x00020000
#define DISCR_COMPRESSED		0x00040000
#define DISCR_UNCOMPRESSED		0x00080000
#define INTERVAL				0x00100000
#define CONVERSION_FORMAT		0x00200000
#define MODIFIER_REPORT			0x00400000
#define MODIFIER_AUTHORIZE		0x00800000

#define PRODUCT_DISCR_MASK		0x000fffc0
#define PRODUCT_TYPE_MASK		0x000001c0
#define COMMAND_FIELD_MASK		0x0000001f


typedef struct
	{
	unsigned long				SelectionRuleSpecification;						// Product discrimination bits defined in Parse.h.
	unsigned long				ProcessingStatus;
									#define PRODUCT_STATUS_NOT_SET					0x00000000
									// Receive phase operations.
									#define PRODUCT_STATUS_INITIALIZATION_COMPLETE	0x00000001	// The product is in the input queue.
									#define PRODUCT_STATUS_STUDY					0x00000002	// The product is a study with one or more associated images.  Each
																								//  image will also have its own PRODUCT_QUEUE_ITEM without this
																								//  bit set.
									#define PRODUCT_STATUS_UNFOLDERED_LOCAL_IMAGE	0x00000004	// The product is a single, isolated file without a folder structure.
									#define PRODUCT_STATUS_RECEIVE_ERROR			0x00000008
									#define PRODUCT_STATUS_SOURCE_DELETABLE			0x00000010	// The product source file is ready for deletion.
									#define PRODUCT_STATUS_SOURCE_DELETED			0x00000020	// The product source file was deleted.
									// Processing operations.
									#define PRODUCT_STATUS_PRODUCT_IN_INBOX			0x00000040	// The product has been successfully received.
									#define PRODUCT_STATUS_EXTRACTION_BEGUN			0x00000080	// The images have been successfully extracted.
									#define PRODUCT_STATUS_EXTRACTION_COMPLETED		0x00000100	// The images have been successfully extracted.
									#define PRODUCT_STATUS_IMAGE_EXTRACT_SINGLE		0x00000200	// The product is a single file and not necessarily a full study.
									#define PRODUCT_STATUS_IMAGE_EXTRACTION_ERROR	0x00000400
									#define PRODUCT_STATUS_INBOX_DELETABLE			0x00008000	// The product Inbox file is ready for deletion.
									#define PRODUCT_STATUS_INBOX_DELETED			0x00010000	// The product Inbox file was deleted.
									// Sending operations.
									#define PRODUCT_STATUS_ABSTRACTS_STARTED		0x00020000	// The extraction of data abstracts has begun.
									#define PRODUCT_STATUS_ABSTRACTS_COMPLETED		0x00040000	// The extraction of data abstracts has completed.
									#define PRODUCT_STATUS_ERROR_NOTIFY_USER		0x00100000
									#define PRODUCT_STATUS_USER_HAS_BEEN_NOTIFIED	0x00200000
									// Queue management flags.
									#define PRODUCT_STATUS_DICOM_RECEIVE_ITEM		0x08000000
									#define PRODUCT_STATUS_ITEM_BEING_PROCESSED		0x10000000
									#define PRODUCT_STATUS_SELECTED_ITEM			0x20000000
									#define PRODUCT_STATUS_DUPLICATE_ITEM			0x40000000
									#define PRODUCT_STATUS_LOCKED_ITEM				0x80000000

	unsigned long				ModuleWhereErrorOccurred;
	unsigned long				FirstErrorCode;
	char						SourceFileName[ MAX_FILE_SPEC_LENGTH ];
	char						OriginalSourceFileName[ MAX_FILE_SPEC_LENGTH ];
	char						SourceFileSpec[ MAX_FILE_SPEC_LENGTH ];
	char						DestinationFileName[ MAX_FILE_SPEC_LENGTH ];
	char						Description[ MAX_FILE_SPEC_LENGTH ];
	char						Addressee[ MAX_CFG_STRING_LENGTH ];		// For Dicom receptions this is the AE_TITLE.
	unsigned long				LocalProductIndex;		// Used as a temporary product identifier until ExternalProductIndex has been assigned.
	unsigned long				ExternalProductIndex;
	unsigned long				ComponentCount;			// For studies, this is the number of images currently stored for this study.
														// Only valid if PRODUCT_STATUS_STUDY is set.
	void						*pParentProduct;		// Only valid if PRODUCT_STATUS_STUDY is NOT set.
	time_t						ArrivalTime;
	time_t						ErrorRetryTime;
	unsigned long				ProductType;
	void						*pProductOperation;
	void						*pProductInfo;			// Pointer to structure of type EXAM_INFO, DICTATION_INFO, etc.
	void						*pDicomHeaderSummary;
	LIST_HEAD					AssociationList;		// CAUTION!  Referencing association parameters outside the state machine environment
														//			must be done without assumptions about the current state of the association.
    PRODUCT_DELETE_FUNCTION		ProductDeleteFunction;
	} PRODUCT_QUEUE_ITEM;


typedef struct _EndPoint
	{
	unsigned long			UniqueSequenceNumber;
	char					Name[ MAX_CFG_STRING_LENGTH ];
	char					Title[ MAX_CFG_STRING_LENGTH ];
	unsigned long			TransmissionType;
								#define ENDPOINT_TYPE_UNSPECIFIED		0
								#define ENDPOINT_TYPE_LOCAL				1
								#define ENDPOINT_TYPE_DICOM				2
								#define ENDPOINT_TYPE_FTP				3
								#define ENDPOINT_TYPE_CONTROL			4
	unsigned long			Usage;
								#define ENDPOINT_USAGE_UNSPECIFIED		0
//								#define ENDPOINT_USAGE_RADIOLOGIST		1
								#define ENDPOINT_USAGE_INBOX			2
//								#define ENDPOINT_USAGE_OUTBOX			3
								#define ENDPOINT_USAGE_PRODUCT_SOURCE	4
								#define ENDPOINT_USAGE_LOCAL_ARCHIVE	5
//								#define ENDPOINT_USAGE_CENTRAL_ARCHIVE	6
//								#define ENDPOINT_USAGE_READING_STATION	7
//								#define ENDPOINT_USAGE_REMOTE_CONTROL	8
//								#define ENDPOINT_USAGE_MISCELLANEOUS	9
	unsigned long			StorageStructure;
								#define ENDPOINT_STRUCTURE_UNSPECIFIED			0
								#define ENDPOINT_STRUCTURE_FOLDER_HIERARCHY		1
								#define ENDPOINT_STRUCTURE_FILE_SEQUENCE		2
	unsigned long			ProductSelection;						// Product discrimination bits defined in Parse.h.
	char					NetworkAddress[ MAX_CFG_STRING_LENGTH ];
	char					Directory[ MAX_CFG_STRING_LENGTH ];
	char					SecurityIdentification[ MAX_CFG_STRING_LENGTH ];
	char					SecurityAccessCode[ MAX_CFG_STRING_LENGTH ];
	struct _EndPoint		*pNextEndPoint;
	} ENDPOINT;


