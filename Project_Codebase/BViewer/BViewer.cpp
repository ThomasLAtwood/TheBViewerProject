// BViewer.cpp :Implementation file for the application class.
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
//	*[5] 02/01/2024 by Tom Atwood
//		Fixed code security issues.
//	*[4] 01/27/2024 by Tom Atwood
//		Consolidated the user selection by moving it into a login name
//		Combo Box instead of a pre-selected user Edit Box.  Removed SelectCurrentUser.h.
//	*[3] 10/09/2023 by Tom Atwood
//		Overhauled the login procedures to support multiple useres and handle
//		exceptional cases.
//	*[2] 07/17/2023 by Tom Atwood
//		Fixed code security issues.
//	*[1] 01/03/2023 by Tom Atwood
//		Fixed code security issues.
//
//
#include "stdafx.h"
#include <stdio.h>
#include <process.h>
#include "Module.h"
#include "ReportStatus.h"
#include "BViewer.h"
#include "Configuration.h"
#include "Access.h"
#include "DiagnosticImage.h"
#include "Mouse.h"
#include "ImageView.h"
#include "MainFrm.h"
#include "DicomDictionary.h"
#include "Abstract.h"
#include "ReaderInfoScreen.h"
#include "Client.h"
#include "Export.h"
#include "SelectUser.h"					// *[3] Added include file.


// CBViewerApp

extern CONFIGURATION				BViewerConfiguration;
extern LIST_HEAD					UserNoticeList;
extern HANDLE						hStatusSemaphore;


// The one and only CBViewerApp object:
CBViewerApp					ThisBViewerApp;

CCustomization				BViewerCustomization;
CCustomization				*pBViewerCustomization = 0;
LIST_HEAD					RegisteredUserList = 0;
READER_PERSONAL_INFO		LoggedInReaderInfo;			// Saved reader info, used for restoring overwrites from imported studies.
CString						ChildFrameWindowClass = "";
CString						PopupWindowClass = "";
CString						ExplorerWindowClass = "";
CString						ControlTipWindowClass = "";
BOOL						bLoadingStandards = FALSE;
BOOL						bOKToSaveReaderInfo = TRUE;
BOOL						bABatchStudyIsBeingProcessed = FALSE;


// Certification exams were previously taken by filling out paper report forms.
// Setting bMakeDumbButtons to TRUE will remove most of the intelligence from
// the analysis tab, simulating what one would get with a blank paper report form.
BOOL						bMakeDumbButtons = FALSE;

static	BOOL						bTimerHasBeenStarted = FALSE;
static	BOOL						bTerminateTimer = FALSE;
static	BOOL						bTimerHasTerminated = FALSE;


//BEGIN_MESSAGE_MAP(CBViewerApp, CWinApp)
//END_MESSAGE_MAP()


// CBViewerApp construction

CBViewerApp::CBViewerApp()
{
	m_ActiveStudyList = 0;
	m_AvailableStudyList = 0;
	m_NewlyArrivedStudyList = 0;
	m_pCurrentStudy = 0;
	m_bNewAbstractsAreAvailable = FALSE;
	m_nNewStudiesImported = 0;
	EnableHtmlHelp();
	m_BRetrieverStatus = BRETRIEVER_STATUS_STOPPED;
	m_nSecondsSinceHearingFromBRetriever = 0L;
	m_NumberOfSpawnedThreads = 0;
	m_bAutoViewStudyReceived = FALSE;
	m_bAutoViewStudyInProgress = FALSE;
}


// CBViewerApp initialization

BOOL CBViewerApp::InitInstance()
{
	BOOL					bOK;								// *[2] Added for error check.
	BOOL					bSuccessfulLogin;
	char					CmdLineArguments[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					AutoOpenFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					AutoOutputImageFileSpec[ FILE_PATH_STRING_LENGTH ];
	char					Msg[ MAX_EXTRA_LONG_STRING_LENGTH ];
	char					*pChar;
	READER_PERSONAL_INFO	*pReaderInfo;						// *[3] Added variable.
	LIST_ELEMENT			*pReaderListElement;				// *[3] Added variable.
	BOOL					bCountryWasPreviouslySelected;		// *[3] Added variable.
	
	bOK = CWinApp::InitInstance();								// *[2] Added error check.
	if ( bOK )													// *[2]
		{
		// Initialize OLE libraries
		if ( !AfxOleInit() )
			{
			AfxMessageBox( IDP_OLE_INIT_FAILED );
			return FALSE;
			}

		InitializeSoftwareModules();
		
		ReadBViewerConfiguration();			// Read the BViewer configuration from the CriticalData2.sav file.
		// If the customization is corrupted, reset it.
		if ( !BViewerCustomization.CustomizationLooksReasonable() || pBViewerCustomization == 0 )
			{
			BViewerCustomization.ResetToDefaultValues();
			pBViewerCustomization = &BViewerCustomization;
			}
		// For backward compatibility force the default country setting if not set.
		bCountryWasPreviouslySelected = ( strlen( BViewerCustomization.m_CountryInfo.CountryName ) != 0 );							// *[3] Added this section.
		if ( !bCountryWasPreviouslySelected )																						// *[3] Added this section.
			{
			strncpy_s( BViewerCustomization.m_CountryInfo.CountryName, MAX_NAME_LENGTH, "United States of America", _TRUNCATE );	// *[3] Added this section.
			BViewerCustomization.m_CountryInfo.DateFormat = DATE_FORMAT_MDY;														// *[3] Added this section.
			}

		strncpy_s( CmdLineArguments, FULL_FILE_SPEC_STRING_LENGTH, ThisBViewerApp.m_lpCmdLine, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
		if ( _stricmp( CmdLineArguments, "Standards" ) == 0 )
			{
			BViewerConfiguration.InterpretationEnvironment = INTERP_ENVIRONMENT_STANDARDS;
			ThisBViewerApp.m_lpCmdLine[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
			m_hApplicationIcon = (HICON)::LoadImage( m_hInstance, MAKEINTRESOURCE(IDI_BSTANDARDS_ICON), IMAGE_ICON, 16, 16, LR_SHARED );
			}
		else
			m_hApplicationIcon = (HICON)::LoadImage( m_hInstance, MAKEINTRESOURCE(IDI_BVIEWER_ICON), IMAGE_ICON, 16, 16, LR_SHARED );

		// If BViewer is already running, check for a command line file specification, then exit.
		if ( hStatusSemaphore == 0 )
			{
			if ( ThisBViewerApp.m_lpCmdLine[0] != _T('\0') )
				{
				// Select/check and view a file passed as the first command line parameter.
				strncpy_s( AutoOpenFileSpec, FULL_FILE_SPEC_STRING_LENGTH, ThisBViewerApp.m_lpCmdLine, _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
				PruneQuotationMarks( AutoOpenFileSpec );
				sprintf_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "Initializing for automatically viewing cmd line file specification: %s", AutoOpenFileSpec );	// *[1] Replaced sprintf with sprintf_s.
				LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
				// Create an output file spec for copying to the watch folder.
				pChar = strrchr( AutoOpenFileSpec, '\\' );
				pChar++;
				strncpy_s( AutoOutputImageFileSpec, FILE_PATH_STRING_LENGTH, BViewerConfiguration.WatchDirectory, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
				if ( AutoOutputImageFileSpec[ strlen( AutoOutputImageFileSpec ) - 1 ] != '\\' )
					strncat_s( AutoOutputImageFileSpec, FILE_PATH_STRING_LENGTH, "\\", _TRUNCATE );									// *[2] Replaced strcat with strncat_s.
				strncat_s( AutoOutputImageFileSpec, FILE_PATH_STRING_LENGTH, "AutoLoad_", _TRUNCATE );								// *[2] Replaced strcat with strncat_s.
				strncat_s( AutoOutputImageFileSpec, FILE_PATH_STRING_LENGTH, pChar, _TRUNCATE );									// *[2] Replaced strncat with strncat_s.
				// Copy the specified file to the watch folder.
				CopyFile( AutoOpenFileSpec, AutoOutputImageFileSpec, FALSE );
				ThisBViewerApp.m_lpCmdLine[0] = _T('\0');
				ThisBViewerApp.m_bAutoViewStudyReceived = TRUE;
				}
			return FALSE;			// Exit the program.
			}

		AfxEnableControlContainer();

		if ( ControlTipWindowClass.GetLength() == 0 )
			ControlTipWindowClass = AfxRegisterWndClass( 0, 0, 0, m_hApplicationIcon );

		// Display login screen and solicit user identification.
		bSuccessfulLogin = FALSE;
		ReadUserList();	// Read the list of registered users for this workstation from the CriticalData1.sav file.

		// *[3] Begin add new section.
		if ( BViewerCustomization.m_NumberOfRegisteredUsers == 0 )
			{
			if ( strlen( BViewerCustomization.m_ReaderInfo.ReportSignatureName ) > 0 )
				{
				// Preserve existing reader info if it only exists in the BViewerCustomization structure and no
				// readers are found in the reader list.  Copy it as the first entry in the list.
				pReaderInfo = (READER_PERSONAL_INFO*)malloc( sizeof(READER_PERSONAL_INFO) );
				if ( pReaderInfo != 0 )
					{
					memcpy( (void*)pReaderInfo, (void*)&BViewerCustomization.m_ReaderInfo, sizeof(READER_PERSONAL_INFO) );
					memcpy( (void*)&pReaderInfo -> m_CountryInfo, (void*)&BViewerCustomization.m_CountryInfo, sizeof(COUNTRY_INFO) );
					pReaderInfo -> IsDefaultReader = TRUE;
					if ( pReaderInfo -> pwLength == 0 || pReaderInfo -> pwLength >= 32 )
						pReaderInfo -> pwLength = 32;

					AppendToList( &RegisteredUserList, (void*)pReaderInfo );
					BViewerCustomization.m_NumberOfRegisteredUsers++;
					}
				}
			else
				while ( BViewerCustomization.m_NumberOfRegisteredUsers == 0 )
					{
					AddNewReader();
					if ( BViewerCustomization.m_NumberOfRegisteredUsers > 0 )
						{
						pReaderInfo = (READER_PERSONAL_INFO*)RegisteredUserList -> pItem;
						if ( pReaderInfo != 0 )
							{
							memcpy( (void*)&BViewerCustomization.m_ReaderInfo, (void*)pReaderInfo, sizeof(READER_PERSONAL_INFO) );
							memcpy( &BViewerCustomization.m_CountryInfo, &pReaderInfo -> m_CountryInfo, sizeof(COUNTRY_INFO) );
							}
						}
					}
			}

		// Copy the default reader info to the BViewerCustomization structure.
		pReaderListElement = RegisteredUserList;
		while ( pReaderListElement != 0 )
			{
			pReaderInfo = (READER_PERSONAL_INFO*)pReaderListElement -> pItem;
			if ( pReaderInfo -> IsDefaultReader )
				{
				memcpy( (void*)&BViewerCustomization.m_ReaderInfo, (void*)pReaderInfo, sizeof(READER_PERSONAL_INFO) );
				}
			pReaderListElement = pReaderListElement -> pNextListElement;
			}				memcpy( &BViewerCustomization.m_CountryInfo, &pReaderInfo -> m_CountryInfo, sizeof(COUNTRY_INFO) );

		// *[3] End add new section.

		if ( strlen( BViewerCustomization.m_ReaderInfo.LoginName ) == 0 )
			{
			bSuccessfulLogin = TRUE;	// If no login name has been set or this is a new installation, proceed.
			if ( strlen( BViewerCustomization.m_ReaderInfo.LastName ) == 0 )
				{
				if ( RegisteredUserList != 0 )			// Take the user info from the first item in the RegisteredUserList.
					{
					pReaderInfo = (READER_PERSONAL_INFO*)RegisteredUserList -> pItem;
					memcpy( &BViewerCustomization.m_ReaderInfo, (void*)pReaderInfo, sizeof( READER_PERSONAL_INFO ) );
					}
				}
			else
				{
				if ( RegisteredUserList == 0 )			// *[3] Add the user info from the configuration file to the (empty) RegisteredUserList.
					AppendToList( &RegisteredUserList, (void*)&BViewerCustomization.m_ReaderInfo );		// *[3]
				}
			}
		else
			bSuccessfulLogin = SuccessfulLogin();			// *[3] Replaced inline logic with a separate function cal.
		if ( !bSuccessfulLogin )
			return FALSE;
		else if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_TEST )		// *[3] Added reader info confirmation for Test mode.
			EditCurrentReader();																	// *[3]

		sprintf_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "Current reader logged in: %s", BViewerCustomization.m_ReaderInfo.ReportSignatureName );	// *[3] Log the current reader.
		LogMessage( Msg, MESSAGE_TYPE_NORMAL_LOG );

		// Save the reader info in case it is overwritten by an imported .axt study.
		memcpy( &LoggedInReaderInfo, &BViewerCustomization.m_ReaderInfo, sizeof( READER_PERSONAL_INFO ) );

		bMakeDumbButtons = ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_TEST );

		// Read any files with extensions .epc located in the BViewer\Config folder.  These files specify
		// calls to be made to external programs.
		ReadAllEventSubscriptionFiles();

		ReadAllClientFiles();

		SetUpAvailableStudies();
		// Start the timer for checking on new studies to be imported.
		LaunchStudyUpdateTimer();

		CMainFrame *pFrame = new CMainFrame;
		if ( !pFrame )
			return FALSE;
		m_pMainWnd = pFrame;

		pFrame -> SurveyGraphicsAdapters();
		pFrame -> OrganizeMultipleDisplayMonitorLayout();

		// Load stock cursor, brush, and icon for the main window class.
		m_MainWindowClassName = AfxRegisterWndClass( CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW, ::LoadCursor( NULL, IDC_ARROW ),
											(HBRUSH)::GetStockObject( BLACK_BRUSH ), m_hApplicationIcon );
		pFrame -> CreateEx( WS_EX_APPWINDOW | WS_EX_OVERLAPPEDWINDOW | WS_EX_CONTEXTHELP, m_MainWindowClassName, "Application Window",
									WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE |
									WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU,	//  | WS_MAXIMIZE
									CRect( 0, 0, 600, 400 ), NULL, 0, NULL );

		if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
			{
			pFrame -> ShowWindow( SW_SHOW );
			pFrame -> UpdateWindow();
			}
		}

	return bOK;							// *[2] Added return of error condition.
}


// *[3] Moved the following to create a separate function which can also
//		be called from the CustomizePage tab.
BOOL SuccessfulLogin()
{
	BOOL					bSuccessfulLogin;
	BOOL					bCancel;
	BOOL					bUserRecognized;
	BOOL					bAccessGranted;
	CLoginScreen			*pLoginScreen;

	bCancel = FALSE;
	bSuccessfulLogin = FALSE;
	bUserRecognized = TRUE;
	bAccessGranted = TRUE;
	while ( !bCancel && !bSuccessfulLogin )		// If a login request was made...
		{
		pLoginScreen = new CLoginScreen( NULL, &BViewerCustomization.m_ReaderInfo );			// *[3] Added 2nd parameter to the function call.
		if ( pLoginScreen != 0 )
			{
			// Display login error message on the next go around.
			if ( !bUserRecognized )
				{
				pLoginScreen -> m_bUserRecognizedOnLastPass = FALSE;
				bUserRecognized = TRUE;		// Reset default status.
				}
			else if ( !bAccessGranted )
				{
				pLoginScreen -> m_bAccessGrantedOnLastPass = FALSE;
				bAccessGranted = TRUE;		// Reset default status.
				}

			pLoginScreen -> DoModal();
			bCancel = pLoginScreen -> m_bLoginCancelled;
			if ( !bCancel )
				{
				bSuccessfulLogin = pLoginScreen -> CertifyLogin();
				if ( !bSuccessfulLogin )
					{
					if ( !pLoginScreen -> m_bUserRecognized )
						bUserRecognized = FALSE;
					else if ( !pLoginScreen -> m_bAccessGranted )
						bAccessGranted = FALSE;
					}
				}
				
			delete pLoginScreen;
			}
		else
			bCancel = TRUE;
		}

	return bSuccessfulLogin;
}


int CBViewerApp::ExitInstance()
{
	MSG						WindowsMessage;

	bTerminateTimer = TRUE;
	while ( m_NumberOfSpawnedThreads > 0 )
		{
		if ( PeekMessage( &WindowsMessage, NULL, 0, 0, PM_REMOVE ) )	// Is There A Message Waiting?
			{
			TranslateMessage( &WindowsMessage );					// Translate The Message
			DispatchMessage( &WindowsMessage );						// Dispatch The Message
			}
		}
	DeleteUserNoticeList();				// *[1] Clear out any unprocessed user notifications.
	CloseSoftwareModules();
	DeallocateListOfStudies();
	
	return CWinApp::ExitInstance();
}


// Read in the configuration information from the saved configuration file.
BOOL CBViewerApp::ReadBViewerConfiguration()
{
	BOOL					bNoError;
	char					FileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					ConfigurationDirectory[ FILE_PATH_STRING_LENGTH ];
	FILE					*pBViewerCfgFile;
	size_t					ExpectedFileSizeInBytes;
	size_t					FileSizeInBytes;
	size_t					nBytesToRead;
	size_t					nBytesRead;
	char					*pTempBuffer;
	BOOL					bFileReadSuccessfully;

	bFileReadSuccessfully = FALSE;
	ExpectedFileSizeInBytes = sizeof( CCustomization );
	ConfigurationDirectory[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
	strncat_s( ConfigurationDirectory, FILE_PATH_STRING_LENGTH, BViewerConfiguration.ConfigDirectory, _TRUNCATE );		// *[2] Replaced strncat with strncat_s.
	if ( ConfigurationDirectory[ strlen( ConfigurationDirectory ) - 1 ] != '\\' )
		strncat_s( ConfigurationDirectory, FILE_PATH_STRING_LENGTH, "\\", _TRUNCATE );									// *[2] Replaced strcat with strncat_s.
	// Check existence of path to configuration directory.
	bNoError = SetCurrentDirectory( ConfigurationDirectory );
	if ( bNoError )
		{
		strncpy_s( FileSpec, FULL_FILE_SPEC_STRING_LENGTH, ConfigurationDirectory, _TRUNCATE );							// *[1] Replaced strcpy with strncpy_s.
		strncat_s( FileSpec, FULL_FILE_SPEC_STRING_LENGTH, "CriticalData2.sav", _TRUNCATE );							// *[2] Replaced strcat with strncat_s.
		pBViewerCfgFile = fopen( FileSpec, "rb" );
		if ( pBViewerCfgFile != 0 )
			{
			nBytesToRead = sizeof( unsigned long);
			nBytesRead = fread_s( &FileSizeInBytes, sizeof(size_t), 1, nBytesToRead, pBViewerCfgFile );					// *[2] Converted from fread to fread_s.
			if ( nBytesRead == nBytesToRead && FileSizeInBytes <= ExpectedFileSizeInBytes )								// If the size was read correctly...
				{
				pTempBuffer = (char*)malloc( ExpectedFileSizeInBytes );
				if ( pTempBuffer != 0 )
					{
					nBytesToRead = FileSizeInBytes;
					nBytesRead = fread_s( pTempBuffer, ExpectedFileSizeInBytes, 1, nBytesToRead, pBViewerCfgFile );		// *[1] Replaced fread with fread_s.
					// If the read was successful, load the configuration object into memory.
					if ( nBytesRead == nBytesToRead )
						{
						memcpy( (char*)&BViewerCustomization, pTempBuffer, nBytesRead );
						pBViewerCustomization = &BViewerCustomization;
						bFileReadSuccessfully = TRUE;
						}
					free( pTempBuffer );
					}
				}
			fclose( pBViewerCfgFile );
			}
		}

	return bFileReadSuccessfully;
}


// This function builds a list of CStudy objects from the current Abstract data in the
// Patients table and the associated tables.  This is the point where the study
// semantics are imposed on the otherwise configurable abstract data fields.
// In other words, the abstract column title names MUST match the expected names.
BOOL CBViewerApp::SetUpAvailableStudies()
{
	BOOL					bNoError = TRUE;
	CStudy					*pNewStudy;
	char					DataDirectory[ FILE_PATH_STRING_LENGTH ];
	char					SearchFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					FoundFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	WIN32_FIND_DATA			FindFileInfo;
	HANDLE					hFindFile;
	BOOL					bFileFound;

	// Add the edited studies to the study list by reading the saved study data files.
	strncpy_s( DataDirectory, FILE_PATH_STRING_LENGTH, BViewerConfiguration.DataDirectory, _TRUNCATE );		// *[2] Replaced strncat with strncpy_s.
	if ( DataDirectory[ strlen( DataDirectory ) - 1 ] != '\\' )
		strncat_s( DataDirectory, FILE_PATH_STRING_LENGTH, "\\", _TRUNCATE );								// *[2] Replaced strcat with strncat_s.
	// Check existence of path to configuration directory.
	bNoError = SetCurrentDirectory( DataDirectory );
	if ( bNoError )
		{
		strncpy_s( SearchFileSpec, FULL_FILE_SPEC_STRING_LENGTH, DataDirectory, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
		strncat_s( SearchFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "*.sdy", _TRUNCATE );						// *[2] Replaced strcat with strncat_s.
		hFindFile = FindFirstFile( SearchFileSpec, &FindFileInfo );
		bFileFound = ( hFindFile != INVALID_HANDLE_VALUE );
		while ( bFileFound )
			{
			strncpy_s( FoundFileSpec, FULL_FILE_SPEC_STRING_LENGTH, DataDirectory, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
			strncat_s( FoundFileSpec, FULL_FILE_SPEC_STRING_LENGTH, FindFileInfo.cFileName, _TRUNCATE );	// *[2] Replaced strncat with strncat_s.
			
			pNewStudy = new CStudy();
			if ( pNewStudy != 0 )
				{
				bNoError = pNewStudy -> Restore( FoundFileSpec );
				if ( bNoError && pNewStudy -> m_pDiagnosticStudyList != 0 )
					AppendToList( &m_AvailableStudyList, (void*)pNewStudy );
				else
					delete pNewStudy;
				}
			// Look for another file in the source directory.
			bFileFound = FindNextFile( hFindFile, &FindFileInfo );
			}
		if ( hFindFile != INVALID_HANDLE_VALUE )
			FindClose( hFindFile );
		}

	return bNoError;
}


static void ProcessAbstractDataRow( char *pTitleRow, char *pDataRow )
{
	BOOL					bNoError = TRUE;
	BOOL					bFieldFound;
	char					TypeOfReadingString[ FILE_PATH_STRING_LENGTH ];
	CStudy					*pNewStudy;
	BOOL					bNewStudyMergedWithExistingStudy;
	char					TextString[ FILE_PATH_STRING_LENGTH ];
	BOOL					bThisStudyIsAutoLoadable;
	char					ModalityString[ FILE_PATH_STRING_LENGTH ];
	char					PatientNameString[ FILE_PATH_STRING_LENGTH ];
	char					PatientIDString[ FILE_PATH_STRING_LENGTH ];
	char					DOBString[ FILE_PATH_STRING_LENGTH ];
	char					StudyDateString[ FILE_PATH_STRING_LENGTH ];
	char					StudyDescriptionString[ FILE_PATH_STRING_LENGTH ];
	char					BodyPartExaminedString[ FILE_PATH_STRING_LENGTH ];
	char					SeriesDescriptionString[ 62564 ];
	char					ReferringPhysiciansNameString[ FILE_PATH_STRING_LENGTH ];
	char					ReferringPhysiciansPhoneString[ FILE_PATH_STRING_LENGTH ];
	char					InstitutionNameString[ FILE_PATH_STRING_LENGTH ];
	DIAGNOSTIC_STUDY		*pStudyDataRow;
	DIAGNOSTIC_SERIES		*pSeriesDataRow;
	DIAGNOSTIC_IMAGE		*pImageDataRow;

	// Check whether this abstract represents a previously interpreted study.
	bFieldFound = GetAbstractColumnValueForSpecifiedField( "TypeOfReading", pTitleRow, pDataRow, TypeOfReadingString, FILE_PATH_STRING_LENGTH );
	if ( bFieldFound )
		// The presence of the TypeOfReading field indicates that this .axt file represents
		//  a previously interpreted study.
		ImportAbstractStudyRow( pTitleRow, pDataRow );
	else
		{
		// The .axt file represents new study information from BRetriever.
		LogMessage( "New data abstract row:", MESSAGE_TYPE_SUPPLEMENTARY );
		LogMessage( pTitleRow, MESSAGE_TYPE_SUPPLEMENTARY );
		LogMessage( pDataRow, MESSAGE_TYPE_SUPPLEMENTARY );
		bFieldFound = GetAbstractColumnValueForSpecifiedField( "Modality", pTitleRow, pDataRow, ModalityString, FILE_PATH_STRING_LENGTH );
		if ( !bFieldFound )
			strncpy_s( ModalityString, FILE_PATH_STRING_LENGTH, "Not Specified", _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
		bFieldFound = GetAbstractColumnValueForSpecifiedField( "PatientsName", pTitleRow, pDataRow, PatientNameString, FILE_PATH_STRING_LENGTH );
		if ( !bFieldFound )
			strncpy_s( PatientNameString, FILE_PATH_STRING_LENGTH, "Not Specified", _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
		bFieldFound = GetAbstractColumnValueForSpecifiedField( "PatientID", pTitleRow, pDataRow, PatientIDString, FILE_PATH_STRING_LENGTH );
		if ( !bFieldFound )
			strncpy_s( PatientIDString, FILE_PATH_STRING_LENGTH, "Not Specified", _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
		bFieldFound = GetAbstractColumnValueForSpecifiedField( "PatientsBirthDate", pTitleRow, pDataRow, DOBString, FILE_PATH_STRING_LENGTH );
		if ( !bFieldFound )
			strncpy_s( DOBString, FILE_PATH_STRING_LENGTH, "Not Specified", _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.
		bFieldFound = GetAbstractColumnValueForSpecifiedField( "StudyDate", pTitleRow, pDataRow, StudyDateString, FILE_PATH_STRING_LENGTH );
		if ( !bFieldFound )
			strncpy_s( StudyDateString, FILE_PATH_STRING_LENGTH, "Not Specified", _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
		bFieldFound = GetAbstractColumnValueForSpecifiedField( "StudyDescription", pTitleRow, pDataRow, StudyDescriptionString, FILE_PATH_STRING_LENGTH );
		if ( !bFieldFound )
			strncpy_s( StudyDescriptionString, FILE_PATH_STRING_LENGTH, "Not Specified", _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
		bFieldFound = GetAbstractColumnValueForSpecifiedField( "BodyPartExamined", pTitleRow, pDataRow, BodyPartExaminedString, FILE_PATH_STRING_LENGTH );
		if ( !bFieldFound )
			strncpy_s( BodyPartExaminedString, FILE_PATH_STRING_LENGTH, "Not Specified", _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
		bFieldFound = GetAbstractColumnValueForSpecifiedField( "SeriesDescription", pTitleRow, pDataRow, SeriesDescriptionString, FILE_PATH_STRING_LENGTH );
		if ( !bFieldFound )
			strncpy_s( SeriesDescriptionString, FILE_PATH_STRING_LENGTH, "Not Specified", _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
		bFieldFound = GetAbstractColumnValueForSpecifiedField( "ReferringPhysicianName", pTitleRow, pDataRow, ReferringPhysiciansNameString, FILE_PATH_STRING_LENGTH );
		if ( !bFieldFound )
			{
			// Look for alternate Dicom dictionary name.
			bFieldFound = GetAbstractColumnValueForSpecifiedField( "ReferringPhysiciansName", pTitleRow, pDataRow, ReferringPhysiciansNameString, FILE_PATH_STRING_LENGTH );
			if ( !bFieldFound )
				strncpy_s( ReferringPhysiciansNameString, FILE_PATH_STRING_LENGTH, "Not Specified", _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
			}
		bFieldFound = GetAbstractColumnValueForSpecifiedField( "ReferringPhysiciansTelephoneNumbers", pTitleRow, pDataRow, ReferringPhysiciansPhoneString, FILE_PATH_STRING_LENGTH );
		if ( !bFieldFound )
			strncpy_s( ReferringPhysiciansPhoneString, FILE_PATH_STRING_LENGTH, "Not Specified", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
		bFieldFound = GetAbstractColumnValueForSpecifiedField( "InstitutionName", pTitleRow, pDataRow, InstitutionNameString, FILE_PATH_STRING_LENGTH );
		if ( !bFieldFound )
			strncpy_s( InstitutionNameString, FILE_PATH_STRING_LENGTH, "Not Specified", _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.

		sprintf_s( TextString, FILE_PATH_STRING_LENGTH, "Received new %s study:  %s  ID:  %s  DOB:  %s, Study Date:  %s",
											ModalityString, PatientNameString, PatientIDString, DOBString, StudyDateString );			// *[1] Replaced sprintf with sprintf_s.
		LogMessage( TextString, MESSAGE_TYPE_NORMAL_LOG );
		LogMessage( TextString, MESSAGE_TYPE_SUPPLEMENTARY );
		sprintf_s( TextString, FILE_PATH_STRING_LENGTH, "                        %s   [%s]   %s",
											StudyDescriptionString, BodyPartExaminedString, SeriesDescriptionString );					// *[1] Replaced sprintf with sprintf_s.
		LogMessage( TextString, MESSAGE_TYPE_NORMAL_LOG );
		LogMessage( TextString, MESSAGE_TYPE_SUPPLEMENTARY );
		sprintf_s( TextString, FILE_PATH_STRING_LENGTH, "                        referred by %s (phone %s) from %s",
											ReferringPhysiciansNameString, ReferringPhysiciansPhoneString, InstitutionNameString );		// *[1] Replaced sprintf with sprintf_s.
		LogMessage( TextString, MESSAGE_TYPE_NORMAL_LOG );
		LogMessage( TextString, MESSAGE_TYPE_SUPPLEMENTARY );

		ThisBViewerApp.m_nNewStudiesImported++;
		pNewStudy = new CStudy();
		if ( pNewStudy != 0 )
			{
			LogMessage( "    Populating new study info.", MESSAGE_TYPE_SUPPLEMENTARY );
			pNewStudy -> PopulateFromAbstractDataRow( pTitleRow, pDataRow );

			LogMessage( "    Check for merging with existing study.", MESSAGE_TYPE_SUPPLEMENTARY );
			bNoError = pNewStudy -> MergeWithExistingStudies( &bNewStudyMergedWithExistingStudy );

			if ( bNoError )
				{
				bThisStudyIsAutoLoadable = FALSE;
				// The newly-created study will have only a single image.  Save its SOPInstanceUID for possible later matching.
				// This is used to auto-select for viewing.
				pStudyDataRow = pNewStudy -> m_pDiagnosticStudyList;
				if ( pStudyDataRow != 0 )
					{
					pSeriesDataRow = pStudyDataRow -> pDiagnosticSeriesList;
					if ( pSeriesDataRow != 0 )
						{
						pImageDataRow = pSeriesDataRow -> pDiagnosticImageList;
						if ( pImageDataRow != 0 )
							{
							if ( strncmp( pImageDataRow -> SOPInstanceUID, "AutoLoad_", 9 ) == 0 )
								{
								strncpy_s( ThisBViewerApp.m_AutoLoadSOPInstanceUID, DICOM_ATTRIBUTE_UI_STRING_LENGTH,
																			pImageDataRow -> SOPInstanceUID, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
								ThisBViewerApp.m_bAutoViewStudyReceived = TRUE;
								bThisStudyIsAutoLoadable = TRUE;
								}
							}
						}
					}
				if ( bNewStudyMergedWithExistingStudy )
					{
					LogMessage( "Adding new image to existing study.", MESSAGE_TYPE_SUPPLEMENTARY );
					delete pNewStudy;
					pNewStudy = 0;
					}
				else
					{
					sprintf_s( TextString, FILE_PATH_STRING_LENGTH, "Adding new study for AE_TITLE %s.", pNewStudy -> m_ReaderInfo.AE_TITLE );	// *[1] Replaced sprintf with sprintf_s.
					LogMessage( TextString, MESSAGE_TYPE_SUPPLEMENTARY );
					if ( bThisStudyIsAutoLoadable )
						AppendToList( &ThisBViewerApp.m_AvailableStudyList, (void*)pNewStudy );
					else
						AppendToList( &ThisBViewerApp.m_NewlyArrivedStudyList, (void*)pNewStudy );
					}
				}
			else						// *[1] Fix memory leak in event of an error.
				delete pNewStudy;		// *[1]
			}
		}
}


// Light up the "new images" button.
void CBViewerApp::EnableNewStudyPosting()
{
	BOOL				bNoError = TRUE;
	CMainFrame			*pMainFrame;
	CControlPanel		*pControlPanel;
	CSelectStudyPage	*pSelectStudyPage;

	if ( ThisBViewerApp.m_NewlyArrivedStudyList != 0 )
		{
		LogMessage( "Enabling new images button.", MESSAGE_TYPE_SUPPLEMENTARY );
		pMainFrame = (CMainFrame*)m_pMainWnd;
		if ( pMainFrame != 0 )
			{
			pControlPanel = pMainFrame -> m_pControlPanel;
			if ( pControlPanel != 0 )
				{
				pSelectStudyPage = &pControlPanel -> m_SelectStudyPage;
				if ( pSelectStudyPage != 0 && pSelectStudyPage -> GetSafeHwnd() != 0 && pSelectStudyPage -> IsWindowVisible() )
					{
					pSelectStudyPage -> ResetCurrentSelection();
					pMainFrame -> m_wndDlgBar.m_ButtonShowNewImages.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
					pMainFrame -> m_wndDlgBar.Invalidate();
					pMainFrame -> m_wndDlgBar.UpdateWindow();
					}
				}
			}
		}
}


void CBViewerApp::MakeAnnouncement( char *pMsg )
{
 	CMainFrame						*pMainFrame;

	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( pMainFrame != 0 )
		pMainFrame -> MakeAnnouncement( pMsg );
}


void CBViewerApp::NotifyUserToAcknowledgeContinuation( char *pNoticeText )
{
 	CMainFrame						*pMainFrame;
	static USER_NOTIFICATION		UserNotice;

	strncpy_s( UserNotice.Source, 16, BViewerConfiguration.ProgramName, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
	UserNotice.ModuleCode = MODULE_IMAGE;
	UserNotice.ErrorCode = 0;
	strncpy_s( UserNotice.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, pNoticeText, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
	UserNotice.TypeOfUserResponseSupported = USER_RESPONSE_TYPE_CONTINUE;
	UserNotice.UserNotificationCause = USER_NOTIFICATION_CAUSE_NEEDS_ACKNOWLEDGMENT;
	UserNotice.SuggestedActionText[ 0 ] = '\0';														// *[1] Eliminated call to strcpy.
	UserNotice.UserResponseCode = 0L;
	UserNotice.TextLinesRequired = 10;
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( pMainFrame != 0 )
		pMainFrame -> ProcessUserNotificationAndWaitForResponse( &UserNotice );
}


void CBViewerApp::NotifyUserOfImageFileError( unsigned int ErrorCode, char *pNoticeText, char *pSuggestionText )
{
 	CMainFrame						*pMainFrame;
	static USER_NOTIFICATION		NoticeOfImageFileError;

	RespondToError( MODULE_IMAGE, ErrorCode );
	strncpy_s( NoticeOfImageFileError.Source, 16, BViewerConfiguration.ProgramName, _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
	NoticeOfImageFileError.ModuleCode = MODULE_IMAGE;
	NoticeOfImageFileError.ErrorCode = ErrorCode;
	strncpy_s( NoticeOfImageFileError.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, pNoticeText, _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
	NoticeOfImageFileError.TypeOfUserResponseSupported = USER_RESPONSE_TYPE_ERROR | USER_RESPONSE_TYPE_CONTINUE;
	NoticeOfImageFileError.UserNotificationCause = USER_NOTIFICATION_CAUSE_IMAGE_PROCESSING_ERROR;
	strncpy_s( NoticeOfImageFileError.SuggestedActionText, MAX_CFG_STRING_LENGTH, pSuggestionText, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
	NoticeOfImageFileError.UserResponseCode = 0L;
	NoticeOfImageFileError.TextLinesRequired = 10;
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( pMainFrame != 0 )
		pMainFrame -> ProcessUserNotificationAndWaitForResponse( &NoticeOfImageFileError );
}


void CBViewerApp::NotifyUserOfImportSearchStatus( unsigned int ErrorCode, char *pNoticeText, char *pSuggestionText )
{
 	CMainFrame						*pMainFrame;
	static USER_NOTIFICATION		NoticeOfImportError;

	RespondToError( MODULE_INSTALL, ErrorCode );
	strncpy_s( NoticeOfImportError.Source, 16, BViewerConfiguration.ProgramName, _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
	NoticeOfImportError.ModuleCode = MODULE_IMPORT;
	NoticeOfImportError.ErrorCode = ErrorCode;
	strncpy_s( NoticeOfImportError.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, pNoticeText, _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
	NoticeOfImportError.TypeOfUserResponseSupported = USER_RESPONSE_TYPE_ERROR | USER_RESPONSE_TYPE_CONTINUE;
	NoticeOfImportError.UserNotificationCause = USER_NOTIFICATION_CAUSE_IMPORT_PROCESSING_ERROR;
	strncpy_s( NoticeOfImportError.SuggestedActionText, MAX_CFG_STRING_LENGTH, pSuggestionText, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
	NoticeOfImportError.UserResponseCode = 0L;
	NoticeOfImportError.TextLinesRequired = 10;
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( pMainFrame != 0 )
		pMainFrame -> ProcessUserNotificationAndWaitForResponse( &NoticeOfImportError );
}


void CBViewerApp::NotifyUserOfInstallSearchStatus( unsigned int ErrorCode, char *pNoticeText, char *pSuggestionText )
{
 	CMainFrame						*pMainFrame;
	static USER_NOTIFICATION		NoticeOfInstallError;

	RespondToError( MODULE_INSTALL, ErrorCode );
	strncpy_s( NoticeOfInstallError.Source, 16, BViewerConfiguration.ProgramName, _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
	NoticeOfInstallError.ModuleCode = MODULE_INSTALL;
	NoticeOfInstallError.ErrorCode = ErrorCode;
	strncpy_s( NoticeOfInstallError.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH, pNoticeText, _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
	NoticeOfInstallError.TypeOfUserResponseSupported = USER_RESPONSE_TYPE_ERROR | USER_RESPONSE_TYPE_CONTINUE;
	NoticeOfInstallError.UserNotificationCause = USER_NOTIFICATION_CAUSE_IMPORT_PROCESSING_ERROR;
	strncpy_s( NoticeOfInstallError.SuggestedActionText, MAX_CFG_STRING_LENGTH, pSuggestionText, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
	NoticeOfInstallError.UserResponseCode = 0L;
	NoticeOfInstallError.TextLinesRequired = 10;
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( pMainFrame != 0 )
		pMainFrame -> ProcessUserNotificationAndWaitForResponse( &NoticeOfInstallError );
}


BOOL CBViewerApp::WarnUserOfDataResetConsequences()
{
 	CMainFrame						*pMainFrame;
	static USER_NOTIFICATION		NoticeOfImageFileReset;
	BOOL							bProceedWithReset = FALSE;											// *[2] Initialize return value.

	strncpy_s( NoticeOfImageFileReset.Source, 16, BViewerConfiguration.ProgramName, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
	NoticeOfImageFileReset.ModuleCode = MODULE_MAIN;
	NoticeOfImageFileReset.ErrorCode = 0;
	strncpy_s( NoticeOfImageFileReset.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH,
				"Clearing all the subject study\nimage folders will delete all\nsubject images and data.\n\nAre you sure you want to proceed?", _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
	NoticeOfImageFileReset.TypeOfUserResponseSupported = USER_RESPONSE_TYPE_YESNO_NO_CANCEL;
	NoticeOfImageFileReset.UserNotificationCause = USER_NOTIFICATION_CAUSE_CONFIRM_USER_REQUEST;
	NoticeOfImageFileReset.SuggestedActionText[ 0 ] = '\0';												// *[1] Eliminated call to strcpy.
	NoticeOfImageFileReset.UserResponseCode = 0L;
	NoticeOfImageFileReset.TextLinesRequired = 10;
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( pMainFrame != 0 )
		{
		pMainFrame -> ProcessUserNotificationAndWaitForResponse( &NoticeOfImageFileReset );
		bProceedWithReset = ( NoticeOfImageFileReset.UserResponseCode == USER_RESPONSE_CODE_YES );
		}

	return bProceedWithReset;
}


// Read and process the latest Import.axt abstract data file created by BRetriever.
void CBViewerApp::ReadNewAbstractData()
{
	ImportNewAbstractData( ProcessAbstractDataRow );
	m_bNewAbstractsAreAvailable = FALSE;
}


void CheckWindowsMessages()
{
	MSG						WindowsMessage;
	int						nMessage = 0;

	while ( nMessage < 20 && PeekMessage( &WindowsMessage, NULL, 0, 0, PM_REMOVE ) != 0 )	// Is There A Message Waiting?
		{
		TranslateMessage( &WindowsMessage );								// Translate The Message
		DispatchMessage( &WindowsMessage );									// Dispatch The Message
		nMessage++;
		}
}


#define				IMPORT_CHECK_INTERVAL_IN_SECONDS		4

static BOOL			bNewStudiesHaveBeenImported = FALSE;


// CAUTION:  This function runs on a different thread from most of the windows
// in this application.  Care should be exercised in accessing windows from this
// thread.  Results could be unpredictable.
static unsigned __stdcall TimerThreadFunction( VOID *pUnusedData )
{
	int					SecondsRemainingBeforeImportCheck;
	int					SecondsSinceErrorSuspension;
	BOOL				bUserNoticesHaveBeenPosted;
	CMainFrame			*pMainFrame;
	CControlPanel		*pControlPanel;
	CSelectStudyPage	*pSelectStudyPage;
	LIST_ELEMENT		*pListElement;
	USER_NOTIFICATION	*pUserNoticeDescriptor;
	BOOL				bSuspendErrorNotices;
	BOOL				bEnableAutoReportGeneration;

	ThisBViewerApp.m_NumberOfSpawnedThreads++;
	SecondsRemainingBeforeImportCheck = 10;
	bSuspendErrorNotices = FALSE;
	SecondsSinceErrorSuspension = 0;
	while ( !bTimerHasTerminated )
		{
		SecondsRemainingBeforeImportCheck--;
		ThisBViewerApp.m_nSecondsSinceHearingFromBRetriever++;
		if ( ThisBViewerApp.m_nSecondsSinceHearingFromBRetriever > 45L )
			ThisBViewerApp.m_BRetrieverStatus = BRETRIEVER_STATUS_STOPPED;
		if ( bSuspendErrorNotices )
			SecondsSinceErrorSuspension++;
		if ( SecondsSinceErrorSuspension >= 3600 )
			{
			SecondsSinceErrorSuspension = 0;
			bSuspendErrorNotices = FALSE;
			}
		Sleep( 1000 );		// Sleep for one second.
		if ( !bTerminateTimer )
			ThisBViewerApp.UpdateBRetrieverStatusDisplay();
		if ( bTerminateTimer )
			bTimerHasTerminated = TRUE;
		else if ( SecondsRemainingBeforeImportCheck == 0 )
			{
			SecondsRemainingBeforeImportCheck = IMPORT_CHECK_INTERVAL_IN_SECONDS;
			if ( !ThisBViewerApp.m_bNewAbstractsAreAvailable )
				{
				// Check for the presence of a new abstract file.
				ThisBViewerApp.m_bNewAbstractsAreAvailable = NewAbstractDataAreAvailable();
				if ( ThisBViewerApp.m_bNewAbstractsAreAvailable )
					{
					// Create new studies and link them to the m_NewlyArrivedStudyList.
					ThisBViewerApp.ReadNewAbstractData();
//					bEnableAutoReportGeneration = TRUE;

					if ( ThisBViewerApp.m_bAutoViewStudyReceived )
						{
						pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
						if ( pMainFrame != 0 )
							{
							pControlPanel = pMainFrame -> m_pControlPanel;
							if ( pControlPanel != 0 )
								{
								pSelectStudyPage = &pControlPanel -> m_SelectStudyPage;
								if ( pSelectStudyPage != 0 && pSelectStudyPage -> GetSafeHwnd() != 0 && pSelectStudyPage -> IsWindowVisible() )
									pSelectStudyPage -> ResetCurrentSelection();
								if ( !ThisBViewerApp.m_bAutoViewStudyInProgress )
									{
									ThisBViewerApp.m_bAutoViewStudyInProgress = TRUE;
									PostMessage( pMainFrame -> GetSafeHwnd(), WM_AUTOLOAD, 0, 0 );
									}
								}
							}
						}
					}
				if ( BViewerConfiguration.bAutoGeneratePDFReportsFromAXTFiles && !bABatchStudyIsBeingProcessed )
					{
					LIST_ELEMENT			*pAvailableStudyListElement;
					CStudy					*pStudy;
					BOOL					bEligibleStudyFound;

					// Check the list of available studies to see if there are any previously interpreted ones
					//  imported from .axt files.
					bEligibleStudyFound = FALSE;
					pAvailableStudyListElement = ThisBViewerApp.m_AvailableStudyList;
					while ( pAvailableStudyListElement != 0 && !bEligibleStudyFound )
						{
						pStudy = (CStudy*)pAvailableStudyListElement -> pItem;
						if ( pStudy != 0 && pStudy -> m_bStudyWasPreviouslyInterpreted )
							bEligibleStudyFound = TRUE;
						pAvailableStudyListElement = pAvailableStudyListElement -> pNextListElement;
						}
					bEnableAutoReportGeneration = bEligibleStudyFound;
					if ( bEnableAutoReportGeneration )
						{
						pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
						if ( pMainFrame != 0 )
							{
							pControlPanel = pMainFrame -> m_pControlPanel;
							if ( pControlPanel != 0 )
								{
								pSelectStudyPage = &pControlPanel -> m_SelectStudyPage;
								if ( pSelectStudyPage != 0 && pSelectStudyPage -> GetSafeHwnd() != 0 )
									{
									bABatchStudyIsBeingProcessed = TRUE;
									PostMessage( pMainFrame -> GetSafeHwnd(), WM_AUTOPROCESS, 0, 0 );
									}
								}
							}
						}
					}
				}
			if ( ThisBViewerApp.m_NewlyArrivedStudyList != 0 )
				ThisBViewerApp.EnableNewStudyPosting();
			}
		// Check for user notifications from BRetriever.
		bUserNoticesHaveBeenPosted = CheckForUserNotification();
		if ( bUserNoticesHaveBeenPosted )
			{
			pListElement = UserNoticeList;
			if ( pListElement != 0 )
				{
				pUserNoticeDescriptor = (USER_NOTIFICATION*)pListElement -> pItem;
				pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
				if ( pMainFrame != 0 && pUserNoticeDescriptor != 0 )
					{
					if ( !bSuspendErrorNotices )
						{
						pUserNoticeDescriptor -> TypeOfUserResponseSupported |= USER_RESPONSE_TYPE_SUSPEND;
						pMainFrame -> ProcessUserNotificationWithoutWaiting( pUserNoticeDescriptor );			// *[2] Eliminated redundant return code.
						if ( ( pUserNoticeDescriptor -> UserResponseCode & USER_RESPONSE_CODE_SUSPEND ) != 0 )
							bSuspendErrorNotices = TRUE;
						RemoveFromList( &UserNoticeList, (void*)pUserNoticeDescriptor );
						free( pUserNoticeDescriptor );
						pUserNoticeDescriptor = 0;			// *[1]
						}
					else
						{
						RemoveFromList( &UserNoticeList, (void*)pUserNoticeDescriptor );
						free( pUserNoticeDescriptor );
						pUserNoticeDescriptor = 0;				// *[1]
						}
					}
				}
			}
		}
	ThisBViewerApp.m_NumberOfSpawnedThreads--;

	return 0;
}


// *[1] Created this function to prevent a terminal memory leak by deleting any
// *[1] unprocessed user notifications on program termination.
void CBViewerApp::DeleteUserNoticeList()
{
	LIST_ELEMENT		*pListElement;
	LIST_ELEMENT		*pPrevListElement;
	USER_NOTIFICATION	*pUserNoticeDescriptor;

	pListElement = UserNoticeList;
	while ( pListElement != 0 )
		{
		pUserNoticeDescriptor = (USER_NOTIFICATION*)pListElement -> pItem;
		pPrevListElement = pListElement;
		pListElement = pListElement -> pNextListElement;
		free( pPrevListElement );
		if ( pUserNoticeDescriptor != 0 )
			free( pUserNoticeDescriptor );
		}
	UserNoticeList = 0;
}


void CBViewerApp::UpdateBRetrieverStatusDisplay()
{
	CMainFrame			*pMainFrame;
	CRect				StatusRect;

	pMainFrame = (CMainFrame*)m_pMainWnd;
	if ( pMainFrame != 0 && pMainFrame -> m_hWnd != 0 )
		{
		switch ( m_BRetrieverStatus )
			{
			case BRETRIEVER_STATUS_STOPPED:
				pMainFrame -> m_wndDlgBar.m_StaticBRetrieverStatus.m_ControlText = "BRetriever\nhas Stopped";
				pMainFrame -> m_wndDlgBar.m_StaticBRetrieverStatus.m_IdleBkgColor = COLOR_RED;
				pMainFrame -> m_wndDlgBar.m_StaticBRetrieverStatus.m_TextColor = COLOR_WHITE;
				if ( pMainFrame -> m_wndDlgBar.m_StaticBRetrieverStatus.IsVisible() )
					pMainFrame -> m_wndDlgBar.m_StaticBRetrieverStatus.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
				else
					pMainFrame -> m_wndDlgBar.m_StaticBRetrieverStatus.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
				break;
			case BRETRIEVER_STATUS_ACTIVE:
				pMainFrame -> m_wndDlgBar.m_StaticBRetrieverStatus.m_ControlText = "BRetriever\nis Awake";
				pMainFrame -> m_wndDlgBar.m_StaticBRetrieverStatus.m_IdleBkgColor = COLOR_DARK_GREEN;
				pMainFrame -> m_wndDlgBar.m_StaticBRetrieverStatus.m_TextColor = COLOR_WHITE;
				pMainFrame -> m_wndDlgBar.m_StaticBRetrieverStatus.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
				break;
			case BRETRIEVER_STATUS_PROCESSING:
				pMainFrame -> m_wndDlgBar.m_StaticBRetrieverStatus.m_ControlText = "BRetriever\nis Importing";
				pMainFrame -> m_wndDlgBar.m_StaticBRetrieverStatus.m_IdleBkgColor = COLOR_GREEN;
				pMainFrame -> m_wndDlgBar.m_StaticBRetrieverStatus.m_TextColor = COLOR_BLACK;
				pMainFrame -> m_wndDlgBar.m_StaticBRetrieverStatus.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
				break;
			}
		pMainFrame -> m_wndDlgBar.m_StaticBRetrieverStatus.GetWindowRect( &StatusRect );
		pMainFrame -> m_wndDlgBar.ScreenToClient( &StatusRect );
		pMainFrame -> m_wndDlgBar.InvalidateRect( &StatusRect, TRUE );
		
		// Handle the problem of exposed windows not being repainted.  Maybe too many messages in the queue.
		if ( pMainFrame -> IsWindowVisible() )
			pMainFrame -> UpdateWindow();
		}
}


void CBViewerApp::LaunchStudyUpdateTimer()
{
	HANDLE					hTimerThreadHandle;
    unsigned				TimerThreadID;

	hTimerThreadHandle = (HANDLE)_beginthreadex(	NULL,					// No security issues for child processes.
													0,						// Use same stack size as parent process.
													TimerThreadFunction,	// Thread function.
													0,						// Argument for thread function.
													0,						// Initialize thread state as running.
													&TimerThreadID );
	bTimerHasBeenStarted = ( hTimerThreadHandle != 0 );
}


void CBViewerApp::DeallocateListOfStudies()
{
	LIST_ELEMENT			*pListElement;
	LIST_ELEMENT			*pPrevListElement;
	CStudy					*pStudy;
	
	pListElement = m_AvailableStudyList;
	while ( pListElement != 0 )
		{
		pStudy = ( CStudy* )pListElement -> pItem;
		if ( pStudy != 0 )
			delete pStudy;
		pPrevListElement = pListElement;
		pListElement = pListElement -> pNextListElement;
		free( pPrevListElement );
		}
	m_AvailableStudyList = 0;
}


void CBViewerApp::TerminateTimers()
{
	MSG						WindowsMessage;

	bTerminateTimer = TRUE;
	while ( m_NumberOfSpawnedThreads > 0 )
		{
		if ( PeekMessage( &WindowsMessage, NULL, 0, 0, PM_REMOVE ) )	// Is There A Message Waiting?
			{
			TranslateMessage( &WindowsMessage );					// Translate The Message
			DispatchMessage( &WindowsMessage );						// Dispatch The Message
			}
		}
}


void CBViewerApp::EraseReaderList()											// *[5] Changed function name.
{
	LIST_ELEMENT			*pUserListElement;
	READER_PERSONAL_INFO	*pReaderInfo;

	pUserListElement = RegisteredUserList;
	while ( pUserListElement != 0 )
		{
		pReaderInfo = (READER_PERSONAL_INFO*)pUserListElement -> pItem;
		RemoveFromList( &RegisteredUserList, (void*)pReaderInfo );
		if ( pReaderInfo -> pSignatureBitmap != 0 )							// *[5] Added this deletion to fix a memory leak.
			{
			if ( pReaderInfo -> pSignatureBitmap -> pImageData != 0 )
				free( pReaderInfo -> pSignatureBitmap -> pImageData );
			free( pReaderInfo -> pSignatureBitmap );
			}
		free( pReaderInfo );
		pUserListElement = RegisteredUserList;
		}
}


void CBViewerApp::OnAppExit()
{
	MSG						WindowsMessage;

	bTerminateTimer = TRUE;
	while ( m_NumberOfSpawnedThreads > 0 )
		{
		if ( PeekMessage( &WindowsMessage, NULL, 0, 0, PM_REMOVE ) )	// Is There A Message Waiting?
			{
			TranslateMessage( &WindowsMessage );					// Translate The Message
			DispatchMessage( &WindowsMessage );						// Dispatch The Message
			}
		}

	CWinApp::OnAppExit();
}




