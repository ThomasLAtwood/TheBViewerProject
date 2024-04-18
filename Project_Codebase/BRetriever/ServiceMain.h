// ServiceMain.h : Define data structures for the BRetriever service application.
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
//	*[1] 03/11/2024 by Tom Atwood
//		Convert windows headers byte packing to the Win32 default for compatibility
//		with Visual Studio 2022.
//
#pragma once

#pragma pack(push, 8)		// *[1] Pack structure members on 8-byte boundaries.
#include <windows.h>
#include <time.h>
#include <WinSvc.h>
#pragma pack(pop)			// *[1]


#define MAIN_ERROR_UNKNOWN						1
#define MAIN_ERROR_SERVICE_DISPATCH				2
#define MAIN_ERROR_SERVICE_NAME					3
#define MAIN_ERROR_SERVICE_NOT_FOUND			4
#define MAIN_ERROR_SCM_SET_STATUS				5
#define MAIN_ERROR_CMD_LINE						6
#define MAIN_ERROR_OPEN_SCM						7
#define MAIN_ERROR_INSTALL_SERVICE				8
#define MAIN_ERROR_OPEN_SERVICE					9
#define MAIN_ERROR_DELETE_SERVICE				10
#define MAIN_ERROR_CFG_FILE						11
#define MAIN_ERROR_ENDPOINT_ASSIGNMENT			12
#define MAIN_ERROR_DICOM_DICTIONARY				13
#define MAIN_ERROR_ABSTRACT_CONFIGURATION		14
#define MAIN_ERROR_SERVICE_ALREADY_RUNNING		15

#define MAIN_ERROR_DICT_LENGTH					15


#define FILE_PATH_STRING_LENGTH			128
#define FULL_FILE_SPEC_STRING_LENGTH	256


typedef struct
	{
	char					ThisTransferNodeName[ MAX_CFG_STRING_LENGTH ];
	char					ConfigDirectory[ MAX_CFG_STRING_LENGTH ];
	char					QueuedFilesDirectory[ MAX_CFG_STRING_LENGTH ];
	char					ErroredFilesDirectory[ MAX_CFG_STRING_LENGTH ];
	char					AbstractsDirectory[ MAX_CFG_STRING_LENGTH ];
	char					ExportsDirectory[ MAX_CFG_STRING_LENGTH ];
	char					DicomImageArchiveDirectory[ MAX_CFG_STRING_LENGTH ];
	char					NetworkAddress[ MAX_CFG_STRING_LENGTH ];
	// If the following boolean members are TRUE, the transfer syntax is obtained from
	// the file metadata or, in the case of network transfers, from the accepted
	// presentation context.
	//
	// In cases where this information is not reliable, (which, given the absurd
	// complexity of the Dicom standard for network transfers, occasionaly happens) the
	// boolean member can be set to FALSE.  In this case, the transfer syntax is 
	// deduced from the actual file data, as follows:  If VR is present, the syntax is
	// EXPLICIT_VR, otherwise IMPLICIT_VR.  Almost every computer is LITTLE_ENDIAN,
	// anyway.  And the size of the encapsulated data, if uncompressed, will equal the
	// product of the raster length in bytes times the number of rasters, assuming this
	// information is included in the file and is valid.  If compressed, it is probably
	// JPEG, and the JPEG library will figure out which variety.
	BOOL					bTrustSpecifiedTransferSyntaxFromLocalStorage;
	BOOL					bTrustSpecifiedTransferSyntaxFromNetwork;
	unsigned long			MinimumFreeSpaceStorageRequirementInMegabytes;
	BOOL					bArchiveAXTOuputFiles;
	BOOL					bEnableSurvey;
	BOOL					bComposeDicomOutputFile;
	BOOL					bApplyManualDicomEdits;
	} CONFIGURATION;


typedef struct
	{
	SERVICE_STATUS_HANDLE		hServiceHandle;
	DWORD						CurrentServiceState;
	HANDLE						hEvents[3];
	char						ProgramPath[ FILE_PATH_STRING_LENGTH ];
	char						ProgramDataPath[ FILE_PATH_STRING_LENGTH ];
	char						StudyDataDirectory[ MAX_CFG_STRING_LENGTH ];
	char						ConfigDirectory[ MAX_CFG_STRING_LENGTH ];
	char						ServiceDirectory[ MAX_CFG_STRING_LENGTH ];
	char						LogDirectory[ MAX_CFG_STRING_LENGTH ];
	char						ExeFile[ FULL_FILE_SPEC_STRING_LENGTH ];
	char						ServiceName[ 64 ];
	char						CfgFile[ FULL_FILE_SPEC_STRING_LENGTH ];
	char						BackupCfgFile[ FULL_FILE_SPEC_STRING_LENGTH ];
	char						ServiceLogFile[ FULL_FILE_SPEC_STRING_LENGTH ];
	char						SupplementaryLogFile[ FULL_FILE_SPEC_STRING_LENGTH ];
	BOOL						bPrintToConsole;
	} TRANSFER_SERVICE;


// Function prototypes:
//
void			InitializeTransferService( TRANSFER_SERVICE *pTransferService );
void			TerminateTransferService( TRANSFER_SERVICE *pTransferService );
void WINAPI		ServiceMain( DWORD argc, LPTSTR *argv );
void WINAPI		ServiceControlHandler( DWORD dwControl );
BOOL			UpdateStatusForSCM( SERVICE_STATUS *pServiceStatus );
void			InstallService();
void			RemoveService();


