// SelectUser.cpp : Implementation file for the  for BViewer user management.
//
//	Written by Thomas L. Atwood
//	P.O. Box 1089
//	West Fork, Arkansas 72774
//	(479)445-4690
//	TomAtwood@Earthlink.net
//
//	Copyright © 2023 CDC
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
//	*[2] 02/03/20224 by Tom Atwood
//		Handled a backward compatibility problem resulting from an increase in
//		size of the READER_PERSONAL_INFO structure.  The fix is to write the
//		new reader data structures to CriticalData3.  If a CriticalData1 file
//		is found, but no CriticalData3 file is found, the old reader data
//		structures are read from it and padded with the newly added members.
//	*[1] 07/31/2023 by Tom Atwood
//		Created this module.
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
#include "SelectUser.h"
#include "ReaderInfoScreen.h"


extern CONFIGURATION			BViewerConfiguration;
extern CCustomization			BViewerCustomization;

extern LIST_HEAD				RegisteredUserList;



// CSelectUser dialog

CSelectUser::CSelectUser( CWnd *pParent /*=NULL*/, READER_PERSONAL_INFO *pReaderInfo,  BOOL bSetInitialReader ) : CDialog( CSelectUser::IDD, pParent ),
				m_StaticReaderSelection( "Reader Selection", 200, 50, 18, 9, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_TOP_JUSTIFIED | CONTROL_VISIBLE,
										IDC_STATIC_READER_SELECTION ),
				m_StaticSelectReader( "Select a Reader", 200, 20, 14, 7, 6,
										COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_MULTILINE | CONTROL_VISIBLE,
										IDC_STATIC_READER_SELECTION_HELP_INFO ),

				m_ComboBoxSelectReader( "", 280, 300, 18, 9, 5, VARIABLE_PITCH_FONT,
										COLOR_BLACK, COLOR_UNTOUCHED_LIGHT, COLOR_COMPLETED_LIGHT, COLOR_TOUCHED,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_VSCROLL | EDIT_BORDER | LIST_SORT | CONTROL_VISIBLE,
										EDIT_VALIDATION_NONE, IDC_COMBO_SELECT_READER ),
				m_ButtonAddReader( "Add a\nNew Reader", 150, 40, 16, 8, 6,
										COLOR_BLACK, COLOR_REPORT, COLOR_REPORT, COLOR_REPORT,
										BUTTON_PUSHBUTTON | CONTROL_MULTILINE | CONTROL_VISIBLE | CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
										IDC_BUTTON_ADD_READER ),
				m_ButtonEditReader( "Edit Info for\nSelected Reader", 150, 40, 16, 8, 6,
										COLOR_BLACK, COLOR_REPORT, COLOR_REPORT, COLOR_REPORT,
										BUTTON_PUSHBUTTON | CONTROL_MULTILINE | CONTROL_VISIBLE |
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
										IDC_BUTTON_EDIT_READER ),
				m_ButtonDeleteReader( "Remove\nSelected Reader", 150, 40, 16, 8, 6,
										COLOR_BLACK, COLOR_REPORT, COLOR_REPORT, COLOR_REPORT,
										BUTTON_PUSHBUTTON | CONTROL_MULTILINE | CONTROL_VISIBLE | CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
										IDC_BUTTON_DELETE_READER ),
				m_ButtonSetdDefaultReader( "Set As Current\n(Default) Reader", 150, 40, 16, 8, 6,
										COLOR_BLACK, COLOR_REPORT, COLOR_REPORT, COLOR_REPORT,
										BUTTON_PUSHBUTTON | CONTROL_MULTILINE | CONTROL_VISIBLE |
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
										IDC_BUTTON_SET_DEFAULT_READER,
											"The default reader becomes the current reader and is\n"
											"the one BViewer will expect to log in next." ),

				m_StaticReaderReportSignatureName( "Current Reader:", 200, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_READER_SIGNATURE_NAME ),
				m_EditReaderReportSignatureName( "", 280, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | EDIT_READONLY | CONTROL_VISIBLE,
										EDIT_VALIDATION_NONE, IDC_EDIT_READER_SIGNATURE_NAME ),


				m_ButtonExit( "Exit", 150, 40, 16, 8, 6,
										COLOR_BLACK, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR,
										BUTTON_PUSHBUTTON | CONTROL_VISIBLE |
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
										IDC_BUTTON_EXIT_READER_SELECTION )
		{
	m_BkgdBrush.CreateSolidBrush( COLOR_CONFIG );
	if ( pReaderInfo != 0 )
		memcpy( (void*)&m_ReaderInfo, (void*)pReaderInfo, sizeof(READER_PERSONAL_INFO) );
	else
		memset( (void*)&m_ReaderInfo, 0, sizeof(READER_PERSONAL_INFO) );
}


CSelectUser::~CSelectUser()
{
	DestroyWindow();
}


BEGIN_MESSAGE_MAP( CSelectUser, CDialog )
	//{{AFX_MSG_MAP(CSelectUser)
	ON_CBN_SELENDOK( IDC_COMBO_SELECT_READER, OnReaderSelected )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_ADD_READER, OnBnClickedAddNewReader )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_EDIT_READER, OnBnClickedEditReader )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_DELETE_READER, OnBnClickedRemoveReader )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_SET_DEFAULT_READER, OnBnClickedSetDefaultReader )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_EXIT_READER_SELECTION, OnBnClickedExitReaderSelection )
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CSelectUser::OnInitDialog()
{
	static char		TextString[ 64 ];
	int				PrimaryScreenWidth;
	int				PrimaryScreenHeight;

	CDialog::OnInitDialog();

	m_StaticReaderSelection.SetPosition( 40, 20, this );
	m_StaticSelectReader.SetPosition( 40, 60, this );
	m_ComboBoxSelectReader.SetPosition( 40, 100, this );
	m_ButtonAddReader.SetPosition( 370,30, this );
	m_ButtonEditReader.SetPosition( 370, 90, this );
	m_ButtonDeleteReader.SetPosition( 370, 150, this );
	m_ButtonSetdDefaultReader.SetPosition( 370, 210, this );

	m_StaticReaderReportSignatureName.SetPosition( 40, 180, this );
	m_EditReaderReportSignatureName.SetPosition( 40, 210, this );

	m_ButtonExit.SetPosition( 370, 300, this );

	PrimaryScreenWidth = ::GetSystemMetrics( SM_CXSCREEN );
	PrimaryScreenHeight = ::GetSystemMetrics( SM_CYSCREEN );

	LoadReaderSelectionList();
	InitializeControlTips();

	m_bChangingCurrentReader = FALSE;
	m_pInitialDefaultReaderInfo = GetDefaultReader();

	SetWindowPos( &wndTop, ( PrimaryScreenWidth - 600 ) / 2, ( PrimaryScreenHeight - 400 ) / 2, 600, 400, SWP_SHOWWINDOW );

	return TRUE; 
}


static void ControlTipActivationFunction( CWnd *pDialogWindow, char *pTipText, CPoint MouseCursorLocation )
{
	CSelectUser			*pSelectUser;

	pSelectUser = (CSelectUser*)pDialogWindow;
	if ( pSelectUser != 0 )
		{
		// If there has been a change in the tip text, reset the tip display window.
		if ( pTipText != 0 && strlen( pTipText ) > 0 && pTipText != pSelectUser -> m_pControlTip -> m_pTipText &&
																	pSelectUser -> m_pControlTip -> m_pTipText != 0 )
			{
			pSelectUser -> m_pControlTip -> ShowWindow( SW_HIDE );
			pSelectUser -> m_pControlTip -> m_pTipText = pTipText;
			pSelectUser -> m_pControlTip -> ShowTipText( MouseCursorLocation, pSelectUser );
			}
		else if ( pTipText == 0 )
			pSelectUser -> m_pControlTip -> ShowWindow( SW_HIDE );
		else
			{
			pSelectUser -> m_pControlTip -> m_pTipText = pTipText;
			pSelectUser -> m_pControlTip -> ShowTipText( MouseCursorLocation, pSelectUser );
			}
		}
}


void CSelectUser::InitializeControlTips()
{
	CWnd					*pChildWindow;
	CRuntimeClass			*pRuntimeClassInfo;

	m_pControlTip = new CControlTip();
	if ( m_pControlTip != 0 )
		{
		m_pControlTip -> ActivateTips();
		pChildWindow = GetWindow( GW_CHILD );		// Get the first child window.
		while ( pChildWindow != NULL )
			{
			pRuntimeClassInfo = pChildWindow -> GetRuntimeClass();
			if ( pRuntimeClassInfo != 0 && 
						( _stricmp( pRuntimeClassInfo -> m_lpszClassName, "TomButton" ) == 0 ||
							_stricmp( pRuntimeClassInfo -> m_lpszClassName, "TomStatic" ) == 0 ) )
				if ( ( (TomButton*)pChildWindow ) -> IsVisible() )
					( (TomButton*)pChildWindow ) -> m_ControlTipActivator = ControlTipActivationFunction;
			pChildWindow = pChildWindow -> GetWindow( GW_HWNDNEXT );
			}
		}
}


BOOL CSelectUser::LoadReaderSelectionList()
{
	BOOL					bNoError = TRUE;
	LIST_ELEMENT			*pReaderListElement;
	READER_PERSONAL_INFO	*pReaderInfo;
	READER_PERSONAL_INFO	*pSelectedReaderInfo;
	int						nItemIndex = 0;				// *[3] Initialize variable.
	int						nSelectedItem;

	m_ComboBoxSelectReader.ResetContent();
	m_ComboBoxSelectReader.SetWindowTextA( "Registered Readers" );
	pReaderListElement = RegisteredUserList;
	pSelectedReaderInfo = 0;
	nSelectedItem = 0;
	while ( pReaderListElement != 0 )
		{
		pReaderInfo = (READER_PERSONAL_INFO*)pReaderListElement -> pItem;
		nItemIndex = m_ComboBoxSelectReader.AddString( pReaderInfo -> ReportSignatureName );

		if ( pReaderInfo -> IsDefaultReader )
			{
			m_EditReaderReportSignatureName.SetWindowText( pReaderInfo -> ReportSignatureName );
			m_pDefaultReaderInfo = pReaderInfo;
			pSelectedReaderInfo = pReaderInfo;
			m_nSelectedReaderItem = nItemIndex;
			}

//		if ( strcmp( pReaderInfo -> ReportSignatureName, BViewerCustomization.m_ReaderInfo.ReportSignatureName ) == 0 )
//			nSelectedItem = nItemIndex;
		m_ComboBoxSelectReader.SetItemDataPtr( nItemIndex, (void*)pReaderInfo );
		pReaderListElement = pReaderListElement -> pNextListElement;
		}
	if ( pSelectedReaderInfo != 0 )
		nItemIndex = m_ComboBoxSelectReader.SelectString( 0, pSelectedReaderInfo -> ReportSignatureName );

	return bNoError;
}


void CSelectUser::OnReaderSelected()
{
	READER_PERSONAL_INFO	*pReaderInfo;
	int						nItemIndex;

	nItemIndex = m_ComboBoxSelectReader.GetCurSel();
	m_nSelectedReaderItem = nItemIndex;
	pReaderInfo = (READER_PERSONAL_INFO*)m_ComboBoxSelectReader.GetItemDataPtr( nItemIndex );
}


void AddNewReader()
{
	CReaderInfoScreen		*pReaderInfoScreen;
	READER_PERSONAL_INFO	*pNewReaderInfo;
	BOOL					bCancel;

	pReaderInfoScreen = new( CReaderInfoScreen );
	if ( pReaderInfoScreen != 0 )
		{
		bCancel = !( pReaderInfoScreen -> DoModal() == IDOK );
		if ( !bCancel )
			{
			pNewReaderInfo = (READER_PERSONAL_INFO*)malloc( sizeof(READER_PERSONAL_INFO) );
			if ( pNewReaderInfo != 0 )
				{
				memcpy( pNewReaderInfo, &pReaderInfoScreen -> m_ReaderInfo, sizeof(READER_PERSONAL_INFO) );
				if ( RegisteredUserList == 0 )				// If this is the first user added,
					pNewReaderInfo -> IsDefaultReader = TRUE;		// mark this reader as the default user.
				if ( strlen( pNewReaderInfo -> ReportSignatureName ) > 0 )
					{
					AppendToList( &RegisteredUserList, (void*)pNewReaderInfo );
					BViewerCustomization.m_NumberOfRegisteredUsers++;
					}
				else
					{
					memcpy( (void*)&BViewerCustomization.m_ReaderInfo, (void*)pNewReaderInfo, sizeof(READER_PERSONAL_INFO) );
					memcpy( &BViewerCustomization.m_CountryInfo, &pNewReaderInfo -> m_CountryInfo, sizeof(COUNTRY_INFO) );
					}
				}
			}
		delete pReaderInfoScreen;
		}
}


void CSelectUser::OnBnClickedAddNewReader( NMHDR *pNMHDR, LRESULT *pResult )
{
	AddNewReader();

	LoadReaderSelectionList();

	Invalidate( TRUE );

	*pResult = 0;
}


READER_PERSONAL_INFO *GetDefaultReader()
{
	LIST_ELEMENT			*pReaderListElement;
	READER_PERSONAL_INFO	*pReaderInfo;
	BOOL					bDefaultReaderFound;
	
	bDefaultReaderFound = FALSE;
	pReaderListElement = RegisteredUserList;
	while ( !bDefaultReaderFound && pReaderListElement != 0 )
		{
		pReaderInfo = (READER_PERSONAL_INFO*)pReaderListElement -> pItem;
		if ( pReaderInfo -> IsDefaultReader )
			bDefaultReaderFound = TRUE;
		pReaderListElement = pReaderListElement -> pNextListElement;
		}
	if ( !bDefaultReaderFound )
		pReaderInfo = 0;

	return pReaderInfo;
}


void EditCurrentReader()
{
	CReaderInfoScreen		*pReaderInfoScreen;
	READER_PERSONAL_INFO	*pReaderInfo;
	BOOL					bCancel;

	pReaderInfo = GetDefaultReader();
	if ( pReaderInfo != 0 )
		{
		pReaderInfoScreen = new CReaderInfoScreen( NULL, pReaderInfo, READER_INFO_CONTEXT_CONFIRM );
		if ( pReaderInfoScreen != 0 )
			{
			memcpy( &pReaderInfoScreen -> m_ReaderInfo, pReaderInfo, sizeof(READER_PERSONAL_INFO) );
			bCancel = !( pReaderInfoScreen -> DoModal() == IDOK );
			if ( !bCancel )
				{
				memcpy( pReaderInfo, &pReaderInfoScreen -> m_ReaderInfo, sizeof(READER_PERSONAL_INFO) );
				memcpy( (void*)&BViewerCustomization.m_ReaderInfo, (void*)pReaderInfo, sizeof(READER_PERSONAL_INFO) );
				memcpy( &BViewerCustomization.m_CountryInfo, &pReaderInfo -> m_CountryInfo, sizeof(COUNTRY_INFO) );
				}
			delete pReaderInfoScreen;
			}
		}

}


void CSelectUser::OnBnClickedEditReader( NMHDR *pNMHDR, LRESULT *pResult )
{
	CReaderInfoScreen		*pReaderInfoScreen;
	READER_PERSONAL_INFO	*pReaderInfo;
	int						nItemIndex;
	BOOL					bCancel;

	nItemIndex = m_ComboBoxSelectReader.GetCurSel();
	pReaderInfo = (READER_PERSONAL_INFO*)m_ComboBoxSelectReader.GetItemDataPtr( nItemIndex );
	pReaderInfoScreen = new CReaderInfoScreen( NULL, pReaderInfo, READER_INFO_CONTEXT_INSERT );
	if ( pReaderInfoScreen != 0 )
		{
		memcpy( &pReaderInfoScreen -> m_ReaderInfo, pReaderInfo, sizeof(READER_PERSONAL_INFO) );
		bCancel = !( pReaderInfoScreen -> DoModal() == IDOK );
		if ( !bCancel )
			{
			memcpy( pReaderInfo, &pReaderInfoScreen -> m_ReaderInfo, sizeof(READER_PERSONAL_INFO) );
			}
		delete pReaderInfoScreen;
		}

	LoadReaderSelectionList();

	Invalidate( TRUE );

	*pResult = 0;
}


void CSelectUser::ClearDefaultReaderFlag()
{
	LIST_ELEMENT			*pReaderListElement;
	READER_PERSONAL_INFO	*pReaderInfo;
	
	m_pDefaultReaderInfo = 0;
	pReaderListElement = RegisteredUserList;
	while ( pReaderListElement != 0 )
		{
		pReaderInfo = (READER_PERSONAL_INFO*)pReaderListElement -> pItem;
		pReaderInfo -> IsDefaultReader = FALSE;
		pReaderListElement = pReaderListElement -> pNextListElement;
		}
}


void CSelectUser::SetAsDefaultReader( READER_PERSONAL_INFO *pReaderInfo )
{
	if ( pReaderInfo != 0 )
		{
		ClearDefaultReaderFlag();
		pReaderInfo -> IsDefaultReader = TRUE;
		m_pDefaultReaderInfo = pReaderInfo;
		m_EditReaderReportSignatureName.SetWindowText( pReaderInfo -> ReportSignatureName );
		}
	LoadReaderSelectionList();

	Invalidate( TRUE );
}


void CSelectUser::OnBnClickedSetDefaultReader( NMHDR *pNMHDR, LRESULT *pResult )
{
	READER_PERSONAL_INFO	*pReaderInfo;

	m_nSelectedReaderItem = m_ComboBoxSelectReader.GetCurSel();
	pReaderInfo = (READER_PERSONAL_INFO*)m_ComboBoxSelectReader.GetItemDataPtr( m_nSelectedReaderItem );
	SetAsDefaultReader( pReaderInfo );

	*pResult = 0;
}


void RemoveCurrentReader()
{
	READER_PERSONAL_INFO	*pReaderInfo;

	pReaderInfo = GetDefaultReader();
	if ( pReaderInfo != 0 )
		{
		RemoveFromList( &RegisteredUserList, (void*)pReaderInfo );
		if ( pReaderInfo != 0 )
			free( pReaderInfo );
		pReaderInfo = 0;
		}
	else
		EraseList( &RegisteredUserList );

}


void CSelectUser::OnBnClickedRemoveReader( NMHDR *pNMHDR, LRESULT *pResult )
{
	READER_PERSONAL_INFO	*pReaderInfo;

	if ( m_nSelectedReaderItem >= 0 && m_nSelectedReaderItem < (int)BViewerCustomization.m_NumberOfRegisteredUsers )
		{
		pReaderInfo = (READER_PERSONAL_INFO*)m_ComboBoxSelectReader.GetItemDataPtr( m_nSelectedReaderItem );
		RemoveFromList( &RegisteredUserList, (void*)pReaderInfo );
		if ( pReaderInfo != 0 )
			free( pReaderInfo );
		pReaderInfo = 0;
		}

	// If no registered readers remain, ask for a new one.
	if ( RegisteredUserList == 0 )
		{
		AddNewReader();
		if ( RegisteredUserList != 0 )
			m_pDefaultReaderInfo = (READER_PERSONAL_INFO*)RegisteredUserList -> pItem;
		}

	Invalidate( TRUE );

	*pResult = 0;
}


void CSelectUser::OnBnClickedExitReaderSelection( NMHDR *pNMHDR, LRESULT *pResult )
{
	WriteUserList();
	if ( RegisteredUserList != 0 )
		{
		memcpy( (void*)&BViewerCustomization.m_ReaderInfo, (void*)m_pDefaultReaderInfo, sizeof(READER_PERSONAL_INFO) );
		memcpy( &BViewerCustomization.m_CountryInfo, &m_pDefaultReaderInfo -> m_CountryInfo, sizeof(COUNTRY_INFO) );
		m_bChangingCurrentReader = ( m_pDefaultReaderInfo != m_pInitialDefaultReaderInfo );
		}
	m_ButtonExit.HasBeenPressed( TRUE );
	CDialog::OnOK();

	*pResult = 0;
}


// This is the READER_PERSONAL_INFO data structure used for storing
// the reader information in BViewer versions up through 1.2t.
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
	} OBSOLETE_READER_PERSONAL_INFO;


void ReadUserList()
{
	READER_PERSONAL_INFO	ReaderInfo;
	READER_PERSONAL_INFO	*pNewReaderInfo;
	BOOL					bIsOldFormatData = FALSE;
	char					ReaderInfoDirectory[ FULL_FILE_SPEC_STRING_LENGTH ];										// *[2] Added separate string for directory.
	char					ReaderInfoFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	FILE					*pReaderInfoFile;
	size_t					nBytesToRead;
	size_t					nBytesRead;
	char					SignatureFileName[ FULL_FILE_SPEC_STRING_LENGTH ];
	BOOL					bFileHasBeenCompletelyRead;

	BViewerCustomization.m_NumberOfRegisteredUsers = 0;
	strncpy_s( ReaderInfoDirectory, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.ConfigDirectory, _TRUNCATE );	// *[2] Use separate string for directory.
	LocateOrCreateDirectory( ReaderInfoDirectory );	// Ensure directory exists.
	if ( ReaderInfoDirectory[ strlen( ReaderInfoDirectory ) - 1 ] != '\\' )
		strncat_s( ReaderInfoDirectory, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );
	strncpy_s( ReaderInfoFileSpec, FULL_FILE_SPEC_STRING_LENGTH, ReaderInfoDirectory,  _TRUNCATE );						// *[2] Try to open CriticalData3.sav,
	strncat_s( ReaderInfoFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "CriticalData3.sav", _TRUNCATE );						// *[2]  which contains data in the new format.
	pReaderInfoFile = fopen( ReaderInfoFileSpec, "rb" );
	// *[2] If CriticalData3.sav is not found, look for CriticalData1.sav and if it is found, read the data in the old format.
	if ( pReaderInfoFile == 0 )
		{
		strncpy_s( ReaderInfoFileSpec, FULL_FILE_SPEC_STRING_LENGTH, ReaderInfoDirectory,  _TRUNCATE );					// *[2] Try to open CriticalData1.sav,
		strncat_s( ReaderInfoFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "CriticalData1.sav", _TRUNCATE );
		pReaderInfoFile = fopen( ReaderInfoFileSpec, "rb" );
		if ( pReaderInfoFile != 0 )
			bIsOldFormatData = TRUE;
		}
	if ( pReaderInfoFile != 0 )
		{
		ThisBViewerApp.EraseReaderList();																// *[2] Changed function name.
		bFileHasBeenCompletelyRead = FALSE;
		while( !bFileHasBeenCompletelyRead )
			{
			if ( bIsOldFormatData )
				nBytesToRead = sizeof( OBSOLETE_READER_PERSONAL_INFO );
			else
				nBytesToRead = sizeof( READER_PERSONAL_INFO );
			// *[2] In either case, read the data into the newer (current) data structure.
			nBytesRead = fread_s( &ReaderInfo, nBytesToRead, 1, nBytesToRead, pReaderInfoFile );		// *[1] Replaced fread with fread_s.
			bFileHasBeenCompletelyRead = ( nBytesRead < nBytesToRead );
			if ( !bFileHasBeenCompletelyRead )
				{
				pNewReaderInfo = (READER_PERSONAL_INFO*)malloc( sizeof(READER_PERSONAL_INFO) );
				if ( pNewReaderInfo != 0 )
					{
					if ( bIsOldFormatData )																// *[2] Handle backward compatibility.
						{
						memcpy( pNewReaderInfo, &ReaderInfo, sizeof(OBSOLETE_READER_PERSONAL_INFO) );
						pNewReaderInfo -> IsDefaultReader = TRUE;
						memcpy( (void*)&pNewReaderInfo -> m_CountryInfo, (void*)&BViewerCustomization.m_CountryInfo, sizeof(COUNTRY_INFO) );
						pNewReaderInfo -> pwLength = 32;
						}
					else
						memcpy( pNewReaderInfo, &ReaderInfo, sizeof(READER_PERSONAL_INFO) );

					strncpy_s( SignatureFileName, FULL_FILE_SPEC_STRING_LENGTH, pNewReaderInfo -> LastName, _TRUNCATE );		// *[2] Signature read added.  It should be done
					strncat_s( SignatureFileName, FULL_FILE_SPEC_STRING_LENGTH, "Signature", _TRUNCATE );						// *[2]  for each individual reader.
					pNewReaderInfo -> pSignatureBitmap = ReadSignatureFile( SignatureFileName );

					AppendToList( &RegisteredUserList, (void*)pNewReaderInfo );
					BViewerCustomization.m_NumberOfRegisteredUsers++;
					}
				}
			}
		fclose( pReaderInfoFile );
		}
}


void WriteUserList()
{
	LIST_ELEMENT			*pUserListElement;
	READER_PERSONAL_INFO	*pReaderInfo;
	char					ReaderInfoFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	FILE					*pReaderInfoFile;

	strncpy_s( ReaderInfoFileSpec, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.ConfigDirectory, _TRUNCATE );
	LocateOrCreateDirectory( ReaderInfoFileSpec );	// Ensure directory exists.
	if ( ReaderInfoFileSpec[ strlen( ReaderInfoFileSpec ) - 1 ] != '\\' )
		strncat_s( ReaderInfoFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );
	strncat_s( ReaderInfoFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "CriticalData3.sav", _TRUNCATE );	// *[2] Beginning with BViewer version 1.2v, write file is CriticalData3.
	pReaderInfoFile = fopen( ReaderInfoFileSpec, "wb" );
	if ( pReaderInfoFile != 0 )
		{
		pUserListElement = RegisteredUserList;
		while ( pUserListElement != 0 )
			{
			pReaderInfo = (READER_PERSONAL_INFO*)pUserListElement -> pItem;
			fwrite( (void*)pReaderInfo, 1, sizeof( READER_PERSONAL_INFO ), pReaderInfoFile );
			pUserListElement = pUserListElement -> pNextListElement;
			}
		fclose( pReaderInfoFile );
		}
}


HBRUSH CSelectUser::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
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


BOOL CSelectUser::OnEraseBkgnd( CDC *pDC )
{
	CBrush		BackgroundBrush( COLOR_CONFIG );
	CRect		BackgroundRectangle;
	CBrush		*pOldBrush = pDC -> SelectObject( &BackgroundBrush );

	GetClientRect( BackgroundRectangle );
	pDC -> FillRect( BackgroundRectangle, &BackgroundBrush );
	pDC -> SelectObject( pOldBrush );

	return TRUE;
}


void CSelectUser::OnClose()
{
	CDialog::OnClose();
}



