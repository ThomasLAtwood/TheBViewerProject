// Configuration.h : Defines the functions and data structures that handle program
//  configuration and user identification.
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
//	*[3] 05/14/2024 by Tom Atwood
//		Removed obsolete film standard reference images.
//	*[2] 01/23/2024 by Tom Atwood
//		Moved IsDefaultReader past pSignatureBitmap in READER_PERSONAL_INFO to
//		simplify backward compatability.
//	*[1] 09/05/2023 by Tom Atwood
//		Added the the default user flag to the READER_PERSONAL_INFO structure.
//		Relocated COUNTRY_INFO definition here from Customization.h.
//
#pragma once

#include "Signature.h"


#define CONFIG_ERROR_INSUFFICIENT_MEMORY			1
#define CONFIG_ERROR_OPEN_CFG_FILE					2
#define CONFIG_ERROR_READ_CFG_FILE					3
#define CONFIG_ERROR_PARSE_ATTRIB_VALUE				4
#define CONFIG_ERROR_PARSE_UNKNOWN_ATTR				5
#define CONFIG_ERROR_WRITE_CFG_FILE					6
#define EVENT_ERROR_PROGRAM_PATH_NOT_FOUND			7
#define EVENT_ERROR_FILE_OPEN_FOR_READ				8
#define EVENT_ERROR_FILE_READ						9

#define CONFIG_ERROR_DICT_LENGTH					9


#define MAX_USER_INFO_LENGTH		32
#define MAX_NAME_LENGTH				64

#define MAX_EVENT_PROGRAM_ARGUMENTS		16

typedef struct
	{
	unsigned long			Event;
								#define EVENT_NOT_SPECIFIED						0
								#define EVENT_REPORT_APPROVAL					1

	unsigned char			ExternalProgramArgumentSequence[ MAX_EVENT_PROGRAM_ARGUMENTS ];
								#define ARG_NONE								0
								#define ARG_PNG_REPORT_FILE_PATH				1
								#define ARG_PDF_OUTPUT_FILE_PATH				2
								#define ARG_DICOM_IMAGE_FILE_PATH				3
								#define ARG_SDY_FILE_PATH						4
								#define ARG_AXT_FILE_PATH						5
								#define ARG_PATIENT_NAME						6
								#define ARG_PATIENT_FIRST_NAME					7
								#define ARG_PATIENT_LAST_NAME					8
								#define ARG_SOP_INSTANCE_UID					9
								#define ARG_STUDY_INSTANCE_UID					10
								#define ARG_READER_LAST_NAME					11
								#define ARG_READER_SIGNED_NAME					12
								#define ARG_PATIENT_ID							13

	unsigned short			ExternalProgramArgumentCount;
	char					ExternalProgramPath[ MAX_CFG_STRING_LENGTH ];
	} EVENT_SUBSCRIPTION_INFO;


typedef struct
	{
	unsigned long			EventID;
	char					ReportPNGFilePath[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					ReportPDFFilePath[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					DicomImageFilePath[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					SDYFilePath[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					AXTFilePath[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					PatientName[ MAX_CFG_STRING_LENGTH ];
	char					PatientFirstName[ MAX_USER_INFO_LENGTH ];
	char					PatientLastName[ MAX_USER_INFO_LENGTH ];
	char					PatientID[ MAX_USER_INFO_LENGTH ];
	char					SOPInstanceUID[ MAX_CFG_STRING_LENGTH ];
	char					StudyInstanceUID[ MAX_CFG_STRING_LENGTH ];
	char					ReaderLastName[ MAX_USER_INFO_LENGTH ];
	char					ReaderSignedName[ MAX_CFG_STRING_LENGTH ];
	} EVENT_PARAMETERS;


typedef struct
	{
	char				CountryName[ MAX_NAME_LENGTH ];
	int					DateFormat;
							#define		DATE_FORMAT_UNSPECIFIED		0
							#define		DATE_FORMAT_YMD				1
							#define		DATE_FORMAT_DMY				2
							#define		DATE_FORMAT_MDY				3
	} COUNTRY_INFO;			// *[1]


typedef struct
	{
	char				LastName[ MAX_USER_INFO_LENGTH ];
	char				ID[ 12 ];
	char				Initials[ 4 ];
	char				StreetAddress[ 64 ];
	char				City[ MAX_USER_INFO_LENGTH ];
	char				State[ 4 ];
	char				ZipCode[ 12 ];
	char				LoginName[ MAX_USER_INFO_LENGTH ];
	char				EncodedPassword[ 2 * MAX_USER_INFO_LENGTH ];
	BOOL				bLoginNameEntered;
	BOOL				bPasswordEntered;
	char				AE_TITLE[ 20 ];
	char				ReportSignatureName[ 64 ];
	SIGNATURE_BITMAP	*pSignatureBitmap;
	BOOL				IsDefaultReader;					// *[1] *[2] Added parameter.
	COUNTRY_INFO		m_CountryInfo;						// *[1] Added parameter.
	char				pwLength;							// *[1] Added parameter.
	} READER_PERSONAL_INFO;


typedef struct
	{
	char			Name[ MAX_CFG_STRING_LENGTH ];
	char			StreetAddress[ MAX_CFG_STRING_LENGTH ];
	char			City[ MAX_CFG_STRING_LENGTH ];
	char			State[ MAX_CFG_STRING_LENGTH ];
	char			ZipCode[ MAX_CFG_STRING_LENGTH ];
	char			Phone[ MAX_CFG_STRING_LENGTH ];
	char			OtherContactInfo[ MAX_CFG_STRING_LENGTH ];
	} CLIENT_INFO;


typedef struct
	{
	char					ProgramPath[ FILE_PATH_STRING_LENGTH ];
	char					ProgramDataPath[ FILE_PATH_STRING_LENGTH ];
	char					ExeFile[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					ProgramName[ 64 ];
	char					CfgFile[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					BackupCfgFile[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					SharedCfgFile[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					BackupSharedCfgFile[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					BViewerAboutFile[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					BViewerTechnicalRequirementsFile[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					BViewerLogFile[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					BViewerSupplementaryLogFile[ FULL_FILE_SPEC_STRING_LENGTH ];
	BOOL					bPrintToConsole;
	char					ThisTransferNodeName[ MAX_CFG_STRING_LENGTH ];
	char					ConfigDirectory[ MAX_CFG_STRING_LENGTH ];
	char					BRetrieverDataDirectory[ MAX_CFG_STRING_LENGTH ];
	char					BRetrieverServiceDirectory[ MAX_CFG_STRING_LENGTH ];
	char					AbstractsDirectory[ MAX_CFG_STRING_LENGTH ];
	char					ExportsDirectory[ MAX_CFG_STRING_LENGTH ];
	char					ClientDirectory[ MAX_CFG_STRING_LENGTH ];
	char					ImageDirectory[ MAX_CFG_STRING_LENGTH ];
	char					ImageArchiveDirectory[ MAX_CFG_STRING_LENGTH ];
	char					InboxDirectory[ MAX_CFG_STRING_LENGTH ];
	char					WatchDirectory[ MAX_CFG_STRING_LENGTH ];
	char					DataDirectory[ MAX_CFG_STRING_LENGTH ];
	char					DataArchiveDirectory[ MAX_CFG_STRING_LENGTH ];
	char					StandardDirectory[ MAX_CFG_STRING_LENGTH ];
	char					ReportDirectory[ MAX_CFG_STRING_LENGTH ];
	char					ReportArchiveDirectory[ MAX_CFG_STRING_LENGTH ];
	char					DicomImageArchiveDirectory[ MAX_CFG_STRING_LENGTH ];
	char					NetworkAddress[ MAX_CFG_STRING_LENGTH ];
	BOOL					bEnableAutoAdvanceToInterpretation;
	BOOL					bPromptForStudyDeletion;
	BOOL					bArchiveSDYFiles;
	BOOL					bArchiveReportFiles;
	BOOL					bArchiveImageFiles;
	BOOL					bAutoGeneratePDFReportsFromAXTFiles;
	unsigned				InterpretationEnvironment;
								#define INTERP_ENVIRONMENT_GENERAL		0
								#define INTERP_ENVIRONMENT_NIOSH		1
								#define INTERP_ENVIRONMENT_TEST			2
								#define INTERP_ENVIRONMENT_STANDARDS	3
	unsigned				TypeOfReadingDefault;
								#define READING_TYPE_UNSPECIFIED	0x0000
								#define READING_TYPE_A_READER		0x0001
								#define READING_TYPE_B_READER		0x0002
								#define READING_TYPE_FACILITY		0x0004
								#define READING_TYPE_OTHER			0x0008

	BOOL					bEnableHistogram;
	CLIENT_INFO				m_ClientInfo;
	BOOL					bMakeDateOfReadingEditable;
	} CONFIGURATION;



// Function prototypes.
//
void				InitConfigurationModule();
void				CloseConfigurationModule();
BOOL				ReadConfigurationFile( char *pConfigurationDirectory, char *pConfigurationFileName );
void				InitConfiguration();
BOOL				ParseConfigurationLine( char *pTextLine );
BOOL				RewriteConfigurationFile( char *pConfigurationDirectory, char *pConfigurationFileName );

BOOL				ReadAllEventSubscriptionFiles();
BOOL				ProcessEventSubscription( EVENT_PARAMETERS *pEventParameters );


