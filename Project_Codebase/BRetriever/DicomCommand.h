// DicomCommand.h - Defines data structures and functions that support the
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
#pragma once


#define DICOMCMD_ERROR_UNEXPECTED_MESSAGE			1
#define	DICOMCMD_ERROR_MATCHING_SOP_CLASS			2
#define DICOMCMD_ERROR_UID_EXCESSIVE_LENGTH			3
#define DICOMCMD_ERROR_UNEXPECTED_DICOM_ELEMENT		4

#define DICOMCMD_ERROR_DICT_LENGTH					4


#define DICOM_CMD_UNSPECIFIED						0xFFFF		// The data set type has not been specified.
#define DICOM_CMD_DATA								0x0000		// This is a data message, not a command message.
#define DICOM_CMD_RELEASE_ASSOCIATION				0x0005		// This is not a command message, but needs to be responded to as if it were.
#define DICOM_CMD_RELEASE_ASSOCIATION_REPLY			0x0006		// This is not a command response, but needs to be responded to as if it were.
#define DICOM_CMD_ABORT_ASSOCIATION					0x0007		// This is not a command message, but needs to be responded to as if it were.
#define DICOM_CMD_STORE								0x0001
#define DICOM_CMD_STORE_RESPONSE					0x8001
#define DICOM_CMD_ECHO								0x0030
#define DICOM_CMD_ECHO_RESPONSE						0x8030


// The command set uses little endian, implicit VR data element encoding.
// The value consists of ValueLength bytes, always an even number, appended
// immediately after this header.
typedef struct
	{
	unsigned short		Group;
	unsigned short		Element;
	unsigned long		ValueLength;
	} DATA_ELEMENT_HEADER_IMPLICIT_VR;


typedef struct
	{
	unsigned short		Group;				// = 0x0000
	unsigned short		Element;			// = 0x0000
	unsigned long		ValueLength;		// = 4L
	unsigned long		Value;
	} DATA_ELEMENT_GROUP_LENGTH;


typedef struct
	{
	unsigned short		Group;				// = 0x0000
	unsigned short		Element;			// = 0x0100
	unsigned long		ValueLength;		// = 2L
	unsigned short		Value;
	} DATA_ELEMENT_COMMAND_FIELD;


typedef struct
	{
	unsigned short		Group;				// = 0x0000
	unsigned short		Element;			// = 0x0110
	unsigned long		ValueLength;		// = 2L
	unsigned short		Value;
	} DATA_ELEMENT_MESSAGE_ID;


typedef struct
	{
	unsigned short		Group;				// = 0x0000
	unsigned short		Element;			// = 0x0120
	unsigned long		ValueLength;		// = 2L
	unsigned short		Value;
	} DATA_ELEMENT_MESSAGE_ID_RESPONDING;


typedef struct
	{
	unsigned short		Group;				// = 0x0000
	unsigned short		Element;			// = 0x0700
	unsigned long		ValueLength;		// = 2L
	unsigned short		Value;
							#define DATA_ELEMENT_PRIORITY_LOW			0x0002
							#define DATA_ELEMENT_PRIORITY_MEDIUM		0x0000
							#define DATA_ELEMENT_PRIORITY_HIGH			0x0001
	} DATA_ELEMENT_PRIORITY;


typedef struct
	{
	unsigned short		Group;				// = 0x0000
	unsigned short		Element;			// = 0x0800
	unsigned long		ValueLength;		// = 2L
	unsigned short		Value;
	} DATA_ELEMENT_DATASET_TYPE;


typedef struct
	{
	unsigned short		Group;				// = 0x0000
	unsigned short		Element;			// = 0x0900
	unsigned long		ValueLength;		// = 2L
	unsigned short		Value;
	} DATA_ELEMENT_STATUS;




// Function prototypes.
//
void				InitDicomCommandModule();
void				CloseDicomCommandModule();

BOOL				RespondToReceivedDicomMessage( DICOM_ASSOCIATION *pAssociation );
BOOL				ParseReceivedDataSetBuffer( DICOM_ASSOCIATION *pAssociation, char *pBuffer, unsigned long BufferLength );


