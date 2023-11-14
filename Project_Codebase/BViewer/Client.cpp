// Client.cpp : Implementation file for the client information dialog
//  box.  This provides the client name and address for the report.
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
//	*[3] 11/07/2023 by Tom Atwood
//		Eliminated erroneous EraseClientList() call on form deletion.
//	*[2] 07/17/2023 by Tom Atwood
//		Fixed code security issues.
//	*[1] 12/22/2022 by Tom Atwood
//		Fixed code security issues.
//
//
#include "stdafx.h"
#include "BViewer.h"
#include "Module.h"
#include "ReportStatus.h"
#include "Configuration.h"
#include "Access.h"
#include "DiagnosticImage.h"
#include "Mouse.h"
#include "ImageView.h"
#include "MainFrm.h"
#include "ImageFrame.h"
#include "Client.h"


extern CONFIGURATION			BViewerConfiguration;

LIST_HEAD						AvailableClientList = 0;


//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		ClientModuleInfo = { MODULE_CLIENT, "Client Module", InitClientModule, CloseClientModule };


static ERROR_DICTIONARY_ENTRY	ClientErrorCodes[] =
			{
				{ CLIENT_ERROR_INSUFFICIENT_MEMORY		, "An error occurred allocating a memory block for data storage." },
				{ CLIENT_ERROR_PATH_NOT_FOUND			, "The directory containing the client information files could not be located." },
				{ CLIENT_ERROR_FILE_OPEN_FOR_READ		, "An error occurred attempting to open a client information file for reading." },
				{ CLIENT_ERROR_FILE_READ				, "An error occurred attempting to read a client information file." },
				{ CLIENT_ERROR_FILE_OPEN_FOR_WRITE		, "An error occurred attempting to open a client information file for writing." },
				{ CLIENT_ERROR_PARSE_UNKNOWN_ATTR		, "An unrecognized line name was encountered in a client information file." },
				{ 0										, NULL }
			};

static ERROR_DICTIONARY_MODULE		ClientStatusErrorDictionary =
										{
										MODULE_CLIENT,
										ClientErrorCodes,
										CLIENT_ERROR_DICT_LENGTH,
										0
										};

// This function must be called before any other function in this module.
void InitClientModule()
{
	LinkModuleToList( &ClientModuleInfo );
	RegisterErrorDictionary( &ClientStatusErrorDictionary );
}


void CloseClientModule()
{
}


// CClient dialog

CClient::CClient( CWnd *pParent /*=NULL*/, CLIENT_INFO *pClientInfo ) : CDialog( CClient::IDD, pParent ),
				m_StaticClientIdentification( "Client Information", 200, 50, 18, 9, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_TOP_JUSTIFIED | CONTROL_VISIBLE,
										IDC_STATIC_CLIENT_IDENTIFICATION ),
				m_StaticClientHelpInfo( "This information is used to add \"letterhead\" labelling\nat the top of the report.  If you are reading on behalf\nof a client, you can use this to label the report\nwith that client's information.",
											390, 50, 12, 6, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE | CONTROL_MULTILINE,
										IDC_STATIC_CLIENT_HELP_INFO ),
				m_StaticClientName( "Client Name", 200, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_CLIENT_NAME ),
				m_EditClientName( "", 300, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_CLIENT_NAME ),

				m_StaticClientStreetAddress( "Street Address", 150, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_CLIENT_STREET_ADDRESS ),
				m_EditClientStreetAddress( "", 300, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_CLIENT_STREET_ADDRESS ),

				m_StaticClientCity( "City", 100, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_CLIENT_CITY ),
				m_EditClientCity( "", 200, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_CLIENT_CITY ),

				m_StaticClientState( "State", 60, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_CLIENT_STATE ),
				m_EditClientState( "", 50, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_CLIENT_STATE ),

				m_StaticClientZipCode( "Zip Code", 100, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_CLIENT_ZIPCODE ),
				m_EditClientZipCode( "123", 120, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_CLIENT_ZIPCODE ),

				m_StaticClientPhone( "Phone", 150, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_CLIENT_PHONE ),
				m_EditClientPhone( "", 200, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_CLIENT_PHONE ),

				m_StaticClientOtherContactInfo( "Other Contact Info", 150, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_CLIENT_OTHER_ADDRESS ),
				m_EditClientOtherContactInfo( "", 300, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_CLIENT_OTHER_ADDRESS ),

				m_GroupEditSequencing( GROUP_EDIT, GROUP_SEQUENCING, 7,
									&m_EditClientName, &m_EditClientStreetAddress, &m_EditClientCity, &m_EditClientState, &m_EditClientZipCode,
									&m_EditClientPhone, &m_EditClientOtherContactInfo ),

				m_ButtonSave( "Save This Client\nInformation", 180, 40, 16, 8, 6,
								COLOR_BLACK, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR,
								BUTTON_PUSHBUTTON | CONTROL_VISIBLE | CONTROL_MULTILINE |
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
								IDC_BUTTON_SAVE_CLIENT_INFO ),
				m_ButtonDelete( "Delete This Client\nInformation", 180, 40, 16, 8, 6,
								COLOR_BLACK, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR,
								BUTTON_PUSHBUTTON | CONTROL_VISIBLE | CONTROL_MULTILINE |
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
								IDC_BUTTON_DELETE_CLIENT_INFO ),
				m_ButtonCancel( "Cancel", 180, 40, 16, 8, 6,
								COLOR_BLACK, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR,
								BUTTON_PUSHBUTTON | CONTROL_VISIBLE |
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
								IDC_BUTTON_CANCEL_CLIENT_INFO )
{
	m_BkgdBrush.CreateSolidBrush( COLOR_CONFIG );
	if ( pClientInfo != 0 )
		{
		memcpy( (void*)&m_ClientInfo, (void*)pClientInfo, sizeof(CLIENT_INFO) );
		m_bAddingNewClient = FALSE;
		}
	else
		{
		memset( (void*)&m_ClientInfo, 0, sizeof(CLIENT_INFO) );
		m_bAddingNewClient = TRUE;
		}
}


CClient::~CClient()
{
	DestroyWindow();
}


BEGIN_MESSAGE_MAP( CClient, CDialog )
	//{{AFX_MSG_MAP(CClient)
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_SAVE_CLIENT_INFO, OnBnClickedSaveClientInfo )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_DELETE_CLIENT_INFO, OnBnClickedDeleteClientInfo )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_CANCEL_CLIENT_INFO, OnBnClickedCancelClientInfo )
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CClient::OnInitDialog()
{
	static char		TextString[ 64 ];
	int				PrimaryScreenWidth;
	int				PrimaryScreenHeight;

	CDialog::OnInitDialog();

	m_StaticClientIdentification.SetPosition( 40, 20, this );
	m_StaticClientHelpInfo.SetPosition( 260, 10, this );
	m_StaticClientName.SetPosition( 40, 70, this );
	m_EditClientName.SetPosition( 200, 70, this );

	m_StaticClientStreetAddress.SetPosition( 40, 100, this );
	m_EditClientStreetAddress.SetPosition( 200, 100, this );

	m_StaticClientCity.SetPosition( 40, 130, this );
	m_EditClientCity.SetPosition( 200, 130, this );

	m_StaticClientState.SetPosition( 40, 160, this );
	m_EditClientState.SetPosition( 200, 160, this );

	m_StaticClientZipCode.SetPosition( 40, 190, this );
	m_EditClientZipCode.SetPosition( 200, 190, this );

	m_StaticClientPhone.SetPosition( 40, 220, this );
	m_EditClientPhone.SetPosition( 200, 220, this );

	m_StaticClientOtherContactInfo.SetPosition( 40, 250, this );
	m_EditClientOtherContactInfo.SetPosition( 200, 250, this );

	m_ButtonSave.SetPosition( 40, 300, this );
	m_ButtonDelete.SetPosition( 240, 300, this );
	m_ButtonCancel.SetPosition( 440, 300, this );

	PrimaryScreenWidth = ::GetSystemMetrics( SM_CXSCREEN );
	PrimaryScreenHeight = ::GetSystemMetrics( SM_CYSCREEN );

	m_EditClientName.SetWindowText( m_ClientInfo.Name );
	m_EditClientStreetAddress.SetWindowText( m_ClientInfo.StreetAddress );
	m_EditClientCity.SetWindowText( m_ClientInfo.City );
	m_EditClientState.SetWindowText( m_ClientInfo.State );
	m_EditClientZipCode.SetWindowText( m_ClientInfo.ZipCode );
	m_EditClientPhone.SetWindowText( m_ClientInfo.Phone );
	m_EditClientOtherContactInfo.SetWindowText( m_ClientInfo.OtherContactInfo );

	SetWindowPos( &wndTop, ( PrimaryScreenWidth - 660 ) / 2, ( PrimaryScreenHeight - 350 ) / 2, 660, 400, SWP_SHOWWINDOW );

	return TRUE; 
}


void EraseClientList()
{
	LIST_ELEMENT			*pClientListElement;
	CLIENT_INFO				*pClientInfo;

	pClientListElement = AvailableClientList;
	while ( pClientListElement != 0 )
		{
		pClientInfo = (CLIENT_INFO*)pClientListElement -> pItem;
		RemoveFromList( &AvailableClientList, (void*)pClientInfo );
		if ( pClientInfo != 0 )
			free( pClientInfo );
		pClientListElement = AvailableClientList;
		}
}


static BOOL ParseClientInformationLine( CLIENT_INFO *pClientInfo, char *pTextLine )
{
	BOOL			bNoError = TRUE;
	char			TextLine[ MAX_CFG_STRING_LENGTH ];
	char			*pAttributeName;
	char			*pAttributeValue;
	char			*pNextToken;			// *[2] Added pointer for strtok_s calls.
	BOOL			bSkipLine = FALSE;

	strncpy_s( TextLine, MAX_CFG_STRING_LENGTH, pTextLine, _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
	// Look for validly formatted attribute name and value.  Find a colon or an end-of-line.
	pAttributeName = strtok_s( TextLine, ":\n", &pNextToken );			// *[2] Replaced strtok with strtok_s.
	if ( pAttributeName == NULL )
		bSkipLine = TRUE;			// If neither found, skip this line.
	if ( !bSkipLine )
		{
		pAttributeValue = strtok_s( NULL, "\n", &pNextToken );  // Point to the value following the colon.   *[2] Replaced strtok with strtok_s.
		if ( pAttributeValue == NULL )
			pAttributeValue = "";
		else
			TrimBlanks( pAttributeValue );
		if ( _stricmp( pAttributeName, "NAME" ) == 0 )
			{
			pClientInfo -> Name[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
			strncat_s( pClientInfo -> Name, MAX_CFG_STRING_LENGTH, pAttributeValue, _TRUNCATE );			// *[2] Replaced strncat with strncat_s.
			}
		else if ( _stricmp( pAttributeName, "STREET ADDRESS" ) == 0 )
			{
			pClientInfo -> StreetAddress[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
			strncat_s( pClientInfo -> StreetAddress, MAX_CFG_STRING_LENGTH, pAttributeValue, _TRUNCATE );	// *[2] Replaced strncat with strncat_s.
			}
		else if ( _stricmp( pAttributeName, "CITY" ) == 0 )
			{
			pClientInfo -> City[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
			strncat_s( pClientInfo -> City, MAX_CFG_STRING_LENGTH, pAttributeValue, _TRUNCATE );			// *[2] Replaced strncat with strncat_s.
			}
		else if ( _stricmp( pAttributeName, "STATE" ) == 0 )
			{
			pClientInfo -> State[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
			strncat_s( pClientInfo -> State, MAX_CFG_STRING_LENGTH, pAttributeValue, _TRUNCATE );			// *[2] Replaced strncat with strncat_s.
			}
		else if ( _stricmp( pAttributeName, "ZIPCODE" ) == 0 )
			{
			pClientInfo -> ZipCode[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
			strncat_s( pClientInfo -> ZipCode, MAX_CFG_STRING_LENGTH, pAttributeValue, _TRUNCATE );			// *[2] Replaced strncat with strncat_s.
			}
		else if ( _stricmp( pAttributeName, "PHONE" ) == 0 )
			{
			pClientInfo -> Phone[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
			strncat_s( pClientInfo -> Phone, MAX_CFG_STRING_LENGTH, pAttributeValue, _TRUNCATE );			// *[2] Replaced strncat with strncat_s.
			}
		else if ( _stricmp( pAttributeName, "OTHER CONTACT INFO" ) == 0 )
			{
			pClientInfo -> OtherContactInfo[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
			strncat_s( pClientInfo -> OtherContactInfo, MAX_CFG_STRING_LENGTH, pAttributeValue, _TRUNCATE ); // *[2] Replaced strncat with strncat_s.
			}
		else
			{
			RespondToError( MODULE_CLIENT, CLIENT_ERROR_PARSE_UNKNOWN_ATTR );
			bNoError = FALSE;
			}
		}
	if ( !bNoError )
		{
		strncpy_s( TextLine, MAX_CFG_STRING_LENGTH, "Error in client information line:  ", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
		strncat_s( TextLine, MAX_CFG_STRING_LENGTH, pTextLine, _TRUNCATE );									// *[1] Replaced strncat with strncat_s.
		LogMessage( TextLine, MESSAGE_TYPE_ERROR );
		}
	// Don't create an error condition just becuase a client information line was bad.
	bNoError = TRUE;

	return bNoError;
}


static BOOL ReadClientFile( char *pFileSpecification )
{
	BOOL			bNoError = TRUE;
	FILE			*pClientInfoFile;
	char			TextLine[ MAX_CFG_STRING_LENGTH ];
	char			EditLine[ MAX_CFG_STRING_LENGTH ];
	BOOL			bEndOfFile;
	BOOL			bFileReadError;
	BOOL			bSkipLine;
	char			*pAttributeName;
	char			*pAttributeValue;
	char			*pNextToken;			// *[2] Added pointer for strtok_s calls.
	CLIENT_INFO		*pNewClientInfo;

	bEndOfFile = FALSE;
	bFileReadError = FALSE;
	pNewClientInfo = (CLIENT_INFO*)malloc( sizeof(CLIENT_INFO) );
	if ( pNewClientInfo == 0 )
		{
		RespondToError( MODULE_CLIENT, CLIENT_ERROR_INSUFFICIENT_MEMORY );
		bNoError = FALSE;
		}
	else									// *[1] Eliminated superfluous condition checks.
		{
		pClientInfoFile = fopen( pFileSpecification, "rt" );
		if ( pClientInfoFile != 0 )			// *[1] Added test for successful file open.
			{
			do
				{
				if ( fgets( TextLine, MAX_CFG_STRING_LENGTH - 1, pClientInfoFile ) == NULL )
					{
					if ( feof( pClientInfoFile ) )
						bEndOfFile = TRUE;
					else if ( ferror( pClientInfoFile ) )
						{
						bFileReadError = TRUE;
						RespondToError( MODULE_CLIENT, CLIENT_ERROR_FILE_READ );
						}
					}
				if ( !bEndOfFile && !bFileReadError )
					{
					bSkipLine = FALSE;
					TrimBlanks( TextLine );
					strncpy_s( EditLine, MAX_CFG_STRING_LENGTH, TextLine, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
					// Look for validly formatted attribute name and value.  Find a colon or an end-of-line.
					pAttributeName = strtok_s( EditLine, ":\n", &pNextToken );			// *[2] Replaced strtok with strtok_s.
					if ( pAttributeName == NULL )
						bSkipLine = TRUE;			// If neither found, skip this line.
					if ( TextLine[0] == '#' || strlen( TextLine ) == 0 )
						bSkipLine = TRUE;
					if ( !bSkipLine )
						{
						pAttributeValue = strtok_s( NULL, "\n", &pNextToken );  // Point to the value following the colon.  *[2] Replaced strtok with strtok_s.
						if ( pAttributeValue != NULL )
							TrimBlanks( pAttributeValue );

						}
					if ( !bSkipLine )
						bNoError = ParseClientInformationLine( pNewClientInfo, TextLine );
					}
				}
			while ( bNoError && !bEndOfFile && !bFileReadError );
			fclose( pClientInfoFile );
			}
		else
			{
			RespondToError( MODULE_CLIENT, CLIENT_ERROR_FILE_OPEN_FOR_READ );
			bNoError = FALSE;
			}
		}

	if ( bNoError )
		AppendToList( &AvailableClientList, (void*)pNewClientInfo );
	else if ( pNewClientInfo != 0 )
		free( pNewClientInfo );

	return bNoError;
}


BOOL ReadAllClientFiles()
{
	BOOL						bNoError = TRUE;
	char						ClientsDirectory[ FULL_FILE_SPEC_STRING_LENGTH ];
	char						SearchFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char						FoundFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	WIN32_FIND_DATA				FindFileInfo;
	HANDLE						hFindFile;
	BOOL						bFileFound;

	EraseClientList();
	ClientsDirectory[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
	strncat_s( ClientsDirectory, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.ClientDirectory, _TRUNCATE );	// *[2] Replaced strncat with strncat_s.
	LocateOrCreateDirectory( ClientsDirectory );	// Ensure directory exists.
	if ( ClientsDirectory[ strlen( ClientsDirectory ) - 1 ] != '\\' )
		strncat_s( ClientsDirectory, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );								// *[2] Replaced strcat with strncat_s.
	// Check existence of source path.
	bNoError = SetCurrentDirectory( ClientsDirectory );
	if ( bNoError )
		{
		strncpy_s( SearchFileSpec, FULL_FILE_SPEC_STRING_LENGTH, ClientsDirectory, _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.
		strncat_s( SearchFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "Client*.cfg", _TRUNCATE );						// *[2] Replaced strcat with strncat_s.
		hFindFile = FindFirstFile( SearchFileSpec, &FindFileInfo );
		bFileFound = ( hFindFile != INVALID_HANDLE_VALUE );
		while ( bFileFound )
			{
			if ( ( FindFileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
				{
				strncpy_s( FoundFileSpec, FULL_FILE_SPEC_STRING_LENGTH, ClientsDirectory, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
				strncat_s( FoundFileSpec, FULL_FILE_SPEC_STRING_LENGTH, FindFileInfo.cFileName, _TRUNCATE );		// *[2] Replaced strncat with strncat_s.
				bNoError = ReadClientFile( FoundFileSpec );
				}
			// Look for another file in the source directory.
			bFileFound = FindNextFile( hFindFile, &FindFileInfo );
			}
		if ( hFindFile != INVALID_HANDLE_VALUE )
			FindClose( hFindFile );
		}
	else
		{
		RespondToError( MODULE_CLIENT, CLIENT_ERROR_PATH_NOT_FOUND );
		bNoError = FALSE;
		}
	
	return bNoError;
}


void CClient::SetClientFileSpecification( char *pClientInfoFileSpec )
{
	char					ClientName[ FULL_FILE_SPEC_STRING_LENGTH ];

	strncpy_s( pClientInfoFileSpec, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.ClientDirectory, _TRUNCATE );	// *[2] Replaced strncat with strncpy_s.
	LocateOrCreateDirectory( pClientInfoFileSpec );	// Ensure directory exists.
	if ( pClientInfoFileSpec[ strlen( pClientInfoFileSpec ) - 1 ] != '\\' )
		strncat_s( pClientInfoFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );								// *[2] Replaced strcat with strncat_s.
	strncpy_s( ClientName, FULL_FILE_SPEC_STRING_LENGTH, m_ClientInfo.Name, _TRUNCATE );								// *[1] Replaced strcpy with strncpy_s.
	PruneEmbeddedSpaceAndPunctuation( ClientName );
	strncat_s( pClientInfoFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "Client", _TRUNCATE );								// *[2] Replaced strncat with strncat_s.
	strncat_s( pClientInfoFileSpec, FULL_FILE_SPEC_STRING_LENGTH, ClientName, _TRUNCATE );								// *[2] Replaced strncat with strncat_s.
	strncat_s( pClientInfoFileSpec, FULL_FILE_SPEC_STRING_LENGTH, ".cfg", _TRUNCATE );									// *[2] Replaced strncat with strncat_s.
}


BOOL CClient::WriteClientFile()
{
	BOOL					bNoError = TRUE;
	char					ClientInfoFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	FILE					*pClientInfoFile;
	char					TextLine[ MAX_CFG_STRING_LENGTH + 40 ];

	SetClientFileSpecification( ClientInfoFileSpec );
	pClientInfoFile = fopen( ClientInfoFileSpec, "wt" );
	if ( pClientInfoFile != 0 )
		{
		strncpy_s( TextLine, MAX_CFG_STRING_LENGTH + 40, "Name:  ", _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
		strncat_s( TextLine, MAX_CFG_STRING_LENGTH + 40, m_ClientInfo.Name, _TRUNCATE );				// *[2] Replaced strcat with strncat_s.
		strncat_s( TextLine, MAX_CFG_STRING_LENGTH + 40, "\n", _TRUNCATE );							// *[2] Replaced strcat with strncat_s.
		fputs( TextLine, pClientInfoFile );

		strncpy_s( TextLine, MAX_CFG_STRING_LENGTH + 40, "Street Address:  ", _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
		strncat_s( TextLine, MAX_CFG_STRING_LENGTH + 40, m_ClientInfo.StreetAddress, _TRUNCATE );	// *[2] Replaced strcat with strncat_s.
		strncat_s( TextLine, MAX_CFG_STRING_LENGTH + 40, "\n", _TRUNCATE );							// *[2] Replaced strcat with strncat_s.
		fputs( TextLine, pClientInfoFile );

		strncpy_s( TextLine, MAX_CFG_STRING_LENGTH + 40, "City:  ", _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
		strncat_s( TextLine, MAX_CFG_STRING_LENGTH + 40, m_ClientInfo.City, _TRUNCATE );			// *[2] Replaced strcat with strncat_s.
		strncat_s( TextLine, MAX_CFG_STRING_LENGTH + 40, "\n", _TRUNCATE );							// *[2] Replaced strcat with strncat_s.
		fputs( TextLine, pClientInfoFile );

		strncpy_s( TextLine, MAX_CFG_STRING_LENGTH + 40, "State:  ", _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
		strncat_s( TextLine, MAX_CFG_STRING_LENGTH + 40, m_ClientInfo.State, _TRUNCATE );			// *[2] Replaced strcat with strncat_s.
		strncat_s( TextLine, MAX_CFG_STRING_LENGTH + 40, "\n", _TRUNCATE );							// *[2] Replaced strcat with strncat_s.
		fputs( TextLine, pClientInfoFile );

		strncpy_s( TextLine, MAX_CFG_STRING_LENGTH + 40, "ZipCode:  ", _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
		strncat_s( TextLine, MAX_CFG_STRING_LENGTH + 40, m_ClientInfo.ZipCode, _TRUNCATE );			// *[2] Replaced strcat with strncat_s.
		strncat_s( TextLine, MAX_CFG_STRING_LENGTH + 40, "\n", _TRUNCATE );							// *[2] Replaced strcat with strncat_s.
		fputs( TextLine, pClientInfoFile );

		strncpy_s( TextLine, MAX_CFG_STRING_LENGTH + 40, "Phone:  ", _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
		strncat_s( TextLine, MAX_CFG_STRING_LENGTH + 40, m_ClientInfo.Phone, _TRUNCATE );			// *[2] Replaced strcat with strncat_s.
		strncat_s( TextLine, MAX_CFG_STRING_LENGTH + 40, "\n", _TRUNCATE );							// *[2] Replaced strcat with strncat_s.
		fputs( TextLine, pClientInfoFile );

		strncpy_s( TextLine, MAX_CFG_STRING_LENGTH + 40, "Other Contact Info:  ", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
		strncat_s( TextLine, MAX_CFG_STRING_LENGTH + 40, m_ClientInfo.OtherContactInfo, _TRUNCATE );// *[2] Replaced strcat with strncat_s.
		strncat_s( TextLine, MAX_CFG_STRING_LENGTH + 40, "\n", _TRUNCATE );							// *[2] Replaced strcat with strncat_s.
		fputs( TextLine, pClientInfoFile );

		fclose( pClientInfoFile );
		}
	else
		{
		RespondToError( MODULE_CLIENT, CLIENT_ERROR_FILE_OPEN_FOR_WRITE );
		bNoError = FALSE;
		}
	
	return bNoError;
}


void CClient::OnBnClickedSaveClientInfo( NMHDR *pNMHDR, LRESULT *pResult )
{
	char							TextString[ 64 ];

	m_EditClientName.GetWindowText( TextString, 64 );
	strncpy_s( m_ClientInfo.Name, 64, TextString, _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.

	m_EditClientStreetAddress.GetWindowText( TextString, 64 );
	strncpy_s( m_ClientInfo.StreetAddress, 64, TextString, _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.

	m_EditClientCity.GetWindowText( TextString, 32 );
	strncpy_s( m_ClientInfo.City, 64, TextString, _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.

	m_EditClientState.GetWindowText( TextString, 4 );
	strncpy_s( m_ClientInfo.State, 64, TextString, _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.

	m_EditClientZipCode.GetWindowText( TextString, 12 );
	strncpy_s( m_ClientInfo.ZipCode, 64, TextString, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.

	m_EditClientPhone.GetWindowText( TextString, MAX_USER_INFO_LENGTH );
	strncpy_s( m_ClientInfo.Phone, 64, TextString, _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.

	m_EditClientOtherContactInfo.GetWindowText( TextString, 64 );
	strncpy_s( m_ClientInfo.OtherContactInfo, 64, TextString, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.

	WriteClientFile();

	m_ButtonSave.HasBeenPressed( TRUE );
	CDialog::OnOK();

	*pResult = 0;
}



void CClient::OnBnClickedDeleteClientInfo( NMHDR *pNMHDR, LRESULT *pResult )
{
	char					ClientInfoFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					Msg[ FULL_FILE_SPEC_STRING_LENGTH ];				// *[2] Added for error check.
	int						Result;												// *[2] Added for error check.

	if ( !m_bAddingNewClient )
		{
		SetClientFileSpecification( ClientInfoFileSpec );
		Result = remove( ClientInfoFileSpec );									// *[2] Added error response.
		if ( Result != 0 )
			{
			sprintf_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, "Error:  Unable to delete %s client information file requested to be discarded.", ClientInfoFileSpec );
			LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
			}
		ReadAllClientFiles();
		}
	m_ButtonDelete.HasBeenPressed( TRUE );

	CDialog::OnCancel();

	*pResult = 0;
}


void CClient::OnBnClickedCancelClientInfo( NMHDR *pNMHDR, LRESULT *pResult )
{
	m_ButtonCancel.HasBeenPressed( TRUE );
	CDialog::OnCancel();

	*pResult = 0;
}


HBRUSH CClient::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
{
	HBRUSH			hBrush;

	if ( nCtlColor == CTLCOLOR_EDIT )
		{
		pDC -> SetBkColor( ( (TomEdit*)pWnd ) -> m_IdleBkgColor );
		pDC -> SetTextColor( ( (TomEdit*)pWnd ) -> m_TextColor );
		pDC -> SetBkMode( OPAQUE );
		hBrush = HBRUSH( *( (TomEdit*)pWnd ) -> m_pCurrentBkgdBrush );
		}
	else
		hBrush = HBRUSH( m_BkgdBrush );

	return hBrush;
}


BOOL CClient::OnEraseBkgnd( CDC *pDC )
{
	CBrush		BackgroundBrush( COLOR_CONFIG );
	CRect		BackgroundRectangle;
	CBrush		*pOldBrush = pDC -> SelectObject( &BackgroundBrush );

	GetClientRect( BackgroundRectangle );
	pDC -> FillRect( BackgroundRectangle, &BackgroundBrush );
	pDC -> SelectObject( pOldBrush );

	return TRUE;
}


void CClient::OnClose()
{
	CDialog::OnClose();
}



