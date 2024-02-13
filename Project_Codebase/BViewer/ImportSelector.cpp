// ImportSelector.cpp : Implementation file for the CImportSelector
//  class, which implements the image import selection process.
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
//	*[4] 01/02/2024 by Tom Atwood
//		Fixed code security issues.
//	*[3] 07/19/2023 by Tom Atwood
//		Fixed code security issues.
//	*[2] 03/29/2023 by Tom Atwood
//		Fixed code security issues.
//	*[1] 01/03/2023 by Tom Atwood
//		Fixed code security issues.
//
//
#include "stdafx.h"
#include <direct.h>
#include <stdio.h>
#include <errno.h>
#include <afxcmn.h>
#include "BViewer.h"
#include "ImportSelector.h"


extern CONFIGURATION				BViewerConfiguration;
extern CBViewerApp					ThisBViewerApp;
extern CString						ExplorerWindowClass;

// Symbol definitions for the item icons to be used for drives and folders.
#define ICON_OMITTED						-1
#define ICON_DRIVE_UNSELECTED				0
#define ICON_DRIVE_SELECTED					1
#define ICON_FOLDER_UNSELECTED				2
#define ICON_FOLDER_SELECTED				3
#define ICON_IMAGE_UNSELECTED				4
#define ICON_IMAGE_SELECTED					5


//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		ImportModuleInfo = { MODULE_IMPORT, "Import Module", InitImportModule, CloseImportModule };


static ERROR_DICTIONARY_ENTRY	ImportErrorCodes[] =
			{
				{ IMPORT_ERROR_INSUFFICIENT_MEMORY		, "An error occurred allocating a memory block for data storage." },
				{ IMPORT_ERROR_FILE_COPY				, "An error occurred attempting to import the selected image file." },
				{ IMPORT_ERROR_FILE_MOVE				, "An error occurred attempting to stage the imported image file." },
				{ IMPORT_ERROR_DICOMDIR_READ			, "An error occurred attempting to read a DICOMDIR file." },
				{ 0										, NULL }
			};

static ERROR_DICTIONARY_MODULE		ImportStatusErrorDictionary =
										{
										MODULE_IMPORT,
										ImportErrorCodes,
										IMPORT_ERROR_DICT_LENGTH,
										0
										};

// This function must be called before any other function in this module.
void InitImportModule()
{
	LinkModuleToList( &ImportModuleInfo );
	RegisterErrorDictionary( &ImportStatusErrorDictionary );
}


void CloseImportModule()
{
}


// CImportSelector
CImportSelector::CImportSelector( int DialogWidth, int DialogHeight, COLORREF BackgroundColor, DWORD WindowStyle ):
				m_StaticUserMessage( "Select Subject Study Image Files to be Imported", 500, 40, 18, 9, 6, COLOR_WHITE, COLOR_PATIENT, COLOR_PATIENT,
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_MESSAGE_FOR_USER ),
				m_StaticStep1( "Step 1:", 60, 20, 16, 8, 6, COLOR_WHITE, COLOR_PATIENT, COLOR_PATIENT,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_IMPORT_STEP1 ),
				m_StaticStep1Text( "If you haven't already\ninserted your storage\nmedia, do it now and\npress \"Refresh View\".", 200, 80, 14, 7, 6,
										COLOR_WHITE, COLOR_PATIENT, COLOR_PATIENT,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE |
										CONTROL_CLIP | CONTROL_VISIBLE, IDC_STATIC_IMPORT_STEP1_TEXT ),
				m_ButtonRefreshView( "Refresh View", 150, 30, 14, 7, 6,
									COLOR_WHITE, COLOR_PATIENT_OPTIONAL, COLOR_PATIENT_OPTIONAL, COLOR_PATIENT_OPTIONAL,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_IMPORT_REFRESH_VIEW ),
				m_StaticStep2( "Step 2:", 60, 20, 16, 8, 6, COLOR_WHITE, COLOR_PATIENT, COLOR_PATIENT,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_IMPORT_STEP2 ),
				m_StaticStep2Text( "Select the device or\nfolder containing\nthe image(s).", 200, 60, 14, 7, 6,
										COLOR_WHITE, COLOR_PATIENT, COLOR_PATIENT,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE |
										CONTROL_CLIP | CONTROL_VISIBLE, IDC_STATIC_IMPORT_STEP2_TEXT ),
				m_StaticStep3( "Step 3:", 60, 20, 16, 8, 6, COLOR_WHITE, COLOR_PATIENT, COLOR_PATIENT,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_IMPORT_STEP3 ),
				m_StaticStep3Text( "Optional:  Show the\ncontents of any\nDICOM file sets.", 200, 60, 14, 7, 6,
										COLOR_WHITE, COLOR_PATIENT, COLOR_PATIENT,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE |
										CONTROL_CLIP | CONTROL_VISIBLE, IDC_STATIC_IMPORT_STEP3_TEXT ),
				m_ButtonAutoImport( "Show DICOM\nFile Sets", 150, 40, 14, 7, 6,
									COLOR_WHITE, COLOR_PATIENT_OPTIONAL, COLOR_PATIENT_OPTIONAL, COLOR_PATIENT_OPTIONAL,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_MULTILINE |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_AUTO_IMPORT ),
				m_StaticStep4( "Step 4:", 60, 20, 16, 8, 6, COLOR_WHITE, COLOR_PATIENT, COLOR_PATIENT,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_IMPORT_STEP4 ),
				m_StaticStep4Text( "Check the checkbox\nof each image file\nyou wish to import.", 200, 60, 14, 7, 6,
										COLOR_WHITE, COLOR_PATIENT, COLOR_PATIENT,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE |
										CONTROL_CLIP | CONTROL_VISIBLE, IDC_STATIC_IMPORT_STEP4_TEXT ),
				m_ButtonImport( "Import Checked\nFiles", 150, 50, 14, 7, 6,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_MULTILINE |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_IMPORT ),
				m_ButtonImportCancel( "Cancel", 150, 30, 14, 7, 6,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_IMPORT_CANCEL )
{
	m_DialogWidth = DialogWidth;
	m_DialogHeight = DialogHeight;
	m_BackgroundColor = BackgroundColor;
	m_WindowStyle = WindowStyle;
	m_pExplorer = new CTreeCtrl;
	m_pListOfFileSetItems = 0;
	m_ListOfProcessedItemFileNames = 0;
	m_TotalImageFilesImported = 0;
	m_pImportDicomdir = 0;
}

CImportSelector::~CImportSelector()
{
	if ( m_pExplorer != 0 )
		delete m_pExplorer;
}


BEGIN_MESSAGE_MAP( CImportSelector, CWnd )
	//{{AFX_MSG_MAP(CImportSelector)
	ON_WM_CREATE()
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_IMPORT_REFRESH_VIEW, OnBnClickedRefreshView )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_AUTO_IMPORT, OnBnClickedAutoImport )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_IMPORT, OnBnClickedImportCheckedItems )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_IMPORT_CANCEL, OnBnClickedImportCancel )
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



// Caution:		Since this function creates the object, it must be called before any
//				functions such as Invalidate(), etc., that manipulate an active window.
BOOL CImportSelector::SetPosition( int x, int y, CWnd *pParentWnd, CString WindowClass )
{
	BOOL			bResult;
	CRect			DialogRect;
	DWORD			WindowsStyle;

	WindowsStyle = DS_MODALFRAME | WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_EX_TOPMOST;
	DialogRect.SetRect( x, y, x + m_DialogWidth, y + m_DialogHeight );
	bResult = CreateEx( WS_EX_DLGMODALFRAME, (const char*)WindowClass, "Select Studies for Importing", WindowsStyle, DialogRect, pParentWnd, 0, NULL );
	
	return bResult;
}


int CImportSelector::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	BOOL			bOK;						// *[2] Added image list creation result.

	unsigned long	StorageDeviceMask;
	char			StorageDeviceSpecification[ 64 ];
	char			VolumeLabel[ 256 ];
	HTREEITEM		hRootItem;

	if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
		return -1;

	m_SelectedFileSpec[ 0 ] = '\0';				// *[1] Eliminated call to strcpy.
	m_bSelectionIsAFolder = FALSE;
	m_bSelectionIsADICOMDIR = FALSE;
	m_Row1YOffset = 10;
	m_Row2YOffset = m_Row1YOffset + 50;
	m_Column1XOffset = 30;
	m_Column2XOffset = m_DialogWidth - 180;
	m_StaticUserMessage.SetPosition( m_Column1XOffset, m_Row1YOffset, this );
	m_StaticStep1.SetPosition( m_Column2XOffset - 20, m_Row2YOffset, this );
	m_StaticStep1Text.SetPosition( m_Column2XOffset, m_Row2YOffset + 20, this );

	m_ButtonRefreshView.SetPosition( m_Column2XOffset, m_Row2YOffset + 100, this );

	m_StaticStep2.SetPosition( m_Column2XOffset - 20, m_Row2YOffset + 140, this );
	m_StaticStep2Text.SetPosition( m_Column2XOffset, m_Row2YOffset + 160, this );
	m_StaticStep3.SetPosition( m_Column2XOffset - 20, m_Row2YOffset + 220, this );
	m_StaticStep3Text.SetPosition( m_Column2XOffset, m_Row2YOffset + 240, this );

	m_ButtonAutoImport.SetPosition( m_Column2XOffset, m_Row2YOffset + 300, this );

	m_StaticStep4.SetPosition( m_Column2XOffset - 20, m_Row2YOffset + 350, this );
	m_StaticStep4Text.SetPosition( m_Column2XOffset, m_Row2YOffset + 370, this );

	m_ButtonImport.SetPosition( m_Column2XOffset, m_Row2YOffset + 450, this );
	m_ButtonImportCancel.SetPosition( m_Column2XOffset, m_DialogHeight - 80, this );
	
	// TVS_CHECKBOXES: 
	// Enables check boxes for items in a tree-view control. A check box is displayed only if an image is associated with the item. When set to this style, the control
	//effectively uses DrawFrameControl to create and set a state image list containing two images. State image 1 is the unchecked box and state image 2 is the checked
	// box. Setting the state image to zero removes the check box altogether. 
	m_pExplorer -> Create( WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | TVS_HASLINES | TVS_TRACKSELECT |  TVS_SHOWSELALWAYS | TVS_CHECKBOXES | TVS_DISABLEDRAGDROP,
							CRect( m_Column1XOffset, m_Row2YOffset, m_Column2XOffset - 30, m_DialogHeight - 80 + 30 ), this, IDC_TREE_CTRL_EXPLORER );
	bOK = m_FolderIcons.Create( 16, 16, ILC_COLOR32, 6, 6 );						// *[2] Added image list creation result.
	
	// NOTE:  Icon files need to be 16 x 16 pixel .bmp files with 24-bit color and without color information.
	m_DriveBitmap.LoadBitmap( IDB_DRIVE_BITMAP );
	m_FolderBitmap.LoadBitmap( IDB_FOLDER_BITMAP );
	m_FolderOpenBitmap.LoadBitmap( IDB_FOLDER_OPEN_BITMAP );
	m_DicomImageBitmap.LoadBitmap( IDB_IMAGE_BITMAP );
	if ( bOK )																		// *[2] Added image list creation check.
		{
		// Create the CImageList of drive and folder Icons.  The associated symbol values must correspond with this sequential order.
		m_FolderIcons.Add( &m_DriveBitmap, RGB(0,0,0) );		// ICON_DRIVE_UNSELECTED
		m_FolderIcons.Add( &m_DriveBitmap, RGB(0,0,0) );		// ICON_DRIVE_SELECTED
		m_FolderIcons.Add( &m_FolderBitmap, RGB(0,0,0) );		// ICON_FOLDER_UNSELECTED
		m_FolderIcons.Add( &m_FolderOpenBitmap, RGB(0,0,0) );	// ICON_FOLDER_SELECTED
		m_FolderIcons.Add( &m_DicomImageBitmap, RGB(0,0,0) );	// ICON_IMAGE_UNSELECTED
		m_FolderIcons.Add( &m_DicomImageBitmap, RGB(0,0,0) );	// ICON_IMAGE_SELECTED

		// Assign the list of item images (icons) to be associated with this CTreeCtrl.
		m_pExplorer -> SetImageList( &m_FolderIcons, TVSIL_NORMAL );
		}

	// List the available storage devices in the Tree Control.
	strncpy_s( StorageDeviceSpecification, 64, "A:", _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
	StorageDeviceMask = _getdrives();
	while ( StorageDeviceMask != 0 )
		{
		if ( StorageDeviceMask & 1 )
			{
			GetDriveLabel( StorageDeviceSpecification, VolumeLabel );
			// By default, all items display the first image in the image list for both the selected and nonselected states
			// You can change the default behavior for a particular item by specifying the indexes of the selected and
			// nonselected images when adding the item to the tree control using the InsertItem member function. You can
			// change the indexes after adding an item by using the SetItemImage member function. 
			hRootItem = m_pExplorer -> InsertItem( VolumeLabel, ICON_DRIVE_UNSELECTED, ICON_DRIVE_SELECTED, TVI_ROOT );
			if ( hRootItem != NULL )
				// Zero the state image (don't declare any bits) to eliminate the checkbox at this level.
				m_pExplorer -> SetItemState( hRootItem, TVIS_BOLD, TVIS_BOLD | TVIS_STATEIMAGEMASK );
			}
		StorageDeviceSpecification[ 0 ]++;
		StorageDeviceMask >>= 1;
		}
	
	return 0;
}


static char			*pTechSupportMessage = "Request technical support.";

// Import a designated file into the BRetriever watch folder, from which
// it will be picked up and processed.
BOOL CImportSelector::CopyDesignatedFile( char *pSourceImageFileSpec )
{
	BOOL					bNoError = TRUE;
	char					InitialOutputImageFileSpec[ FILE_PATH_STRING_LENGTH ];
	char					RevisedOutputImageFileSpec[ FILE_PATH_STRING_LENGTH ];
	char					CurrentFileNameWithExtension[ FILE_PATH_STRING_LENGTH ];
	LIST_ELEMENT			*pListElement;
	char					*pEarlierFileName;
	char					*pCurrentFileName = 0;			// *[1]
	char					Msg[ MAX_EXTRA_LONG_STRING_LENGTH ];
	char					*pChar;
	char					*pExtension;
	char					Version[ 20 ];
	int						RenameResult;

	if ( strlen( pSourceImageFileSpec ) > 0 )
		{
		pCurrentFileName = (char*)malloc( FILE_PATH_STRING_LENGTH );
		bNoError = ( pCurrentFileName != 0 );
		if ( bNoError )
			{
			// Extract the source file name with the file extension (if any) removed.
			pCurrentFileName[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
			pChar = strrchr( pSourceImageFileSpec, '\\' );
			pChar++;
			strncat_s( pCurrentFileName, FILE_PATH_STRING_LENGTH, pChar, _TRUNCATE );							// *[2] Replaced strncat with strncat_s.
			pExtension = strrchr( pCurrentFileName, '.' );
			if ( pExtension != 0 )
				*pExtension = '\0';
			// Compare the file name with those of files already processed.  If there is duplication,
			//  resolve it.  Then append the current file spec to the list.
			strncpy_s( CurrentFileNameWithExtension, FILE_PATH_STRING_LENGTH, pCurrentFileName, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
			strncat_s( CurrentFileNameWithExtension, FILE_PATH_STRING_LENGTH,  ".dcm", _TRUNCATE );				// *[2] Replaced strncat with strncat_s.
			pListElement = m_ListOfProcessedItemFileNames;
			while ( pListElement != 0 )
				{
				pEarlierFileName = (char*)pListElement -> pItem;
				if ( pEarlierFileName != 0 && strcmp( pEarlierFileName, CurrentFileNameWithExtension ) == 0 )
					{
					m_nDuplicateFileNamesDetected++;		// Make each resolved duplicate file name unique.
					_snprintf_s( Version, 20, _TRUNCATE, "_Instance%d", m_nDuplicateFileNamesDetected );		// *[2] Replaced sprintf() with _snprintf_s.
					strncat_s( pCurrentFileName, FILE_PATH_STRING_LENGTH, Version, _TRUNCATE );					// *[2] Replaced strncat with strncat_s.
					}
				pListElement = pListElement -> pNextListElement;
				}
			// Add the Dicom extension, whether it was originally present or not.
			strncat_s( pCurrentFileName, FILE_PATH_STRING_LENGTH,  ".dcm", _TRUNCATE );							// *[2] Replaced strncat with strncat_s.
			// Add the name of the current file to the list of files to be checked for duplication.
			AppendToList( &m_ListOfProcessedItemFileNames, (void*)pCurrentFileName );
			}
		// Copy the file to the Inbox directory.
		strncpy_s( InitialOutputImageFileSpec, FILE_PATH_STRING_LENGTH, BViewerConfiguration.InboxDirectory, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
		if ( InitialOutputImageFileSpec[ strlen( InitialOutputImageFileSpec ) - 1 ] != '\\' )
			strncat_s( InitialOutputImageFileSpec, FILE_PATH_STRING_LENGTH, "\\", _TRUNCATE );					// *[2] Replaced strcat with strncat_s.
		strncat_s( InitialOutputImageFileSpec, FILE_PATH_STRING_LENGTH, pCurrentFileName, _TRUNCATE );			// *[2] Replaced strncat with strncat_s.

		if ( bNoError )
			{
			// First, copy the file to the Inbox directory.
			bNoError = CopyFile( pSourceImageFileSpec, InitialOutputImageFileSpec, FALSE );
			if ( bNoError )
				{
				bNoError = MakeFileWriteable( InitialOutputImageFileSpec );										// *[4] Intrroduced this function to avoid a race condition.
				// Rename it over to the Watch Folder, where BRetriever will pick it up and process it.
				strncpy_s( RevisedOutputImageFileSpec, FILE_PATH_STRING_LENGTH, BViewerConfiguration.WatchDirectory, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
				if ( RevisedOutputImageFileSpec[ strlen( RevisedOutputImageFileSpec ) - 1 ] != '\\' )
					strncat_s( RevisedOutputImageFileSpec, FILE_PATH_STRING_LENGTH, "\\", _TRUNCATE );			// *[2] Replaced strcat with strncat_s.
				strncat_s( RevisedOutputImageFileSpec, FILE_PATH_STRING_LENGTH, pCurrentFileName, _TRUNCATE );	// *[2] Replaced strncat with strncat_s.
				// Then rename it into the Watch directory.  This two-stage file movement avoids having
				// BRetriever try to grab the file for processing while it is still being copied into
				// the Watch directory.  The rename operation is just a modification of a directory
				// entry, so the file appears in the watch directory instantaneously.
				RenameResult = rename( InitialOutputImageFileSpec, RevisedOutputImageFileSpec );
				if ( RenameResult != 0 )
					{
					bNoError = FALSE;
					strncpy_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "Unable to import\n", _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
					strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, InitialOutputImageFileSpec, _TRUNCATE );		// *[3] Replaced strcat with strncat_s.
					ThisBViewerApp.NotifyUserOfImportSearchStatus( IMPORT_ERROR_FILE_MOVE, Msg, pTechSupportMessage );
					}
				}
			else
				{
				strncpy_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "Unable to stage\n", _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
				strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, InitialOutputImageFileSpec, _TRUNCATE );			// *[3] Replaced strcat with strncat_s.
				strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "\nfor import.", _TRUNCATE );						// *[3] Replaced strcat with strncat_s.
				ThisBViewerApp.NotifyUserOfImportSearchStatus( IMPORT_ERROR_FILE_COPY, Msg, pTechSupportMessage );
				}
			}
		}
	if ( bNoError )
		m_TotalImageFilesImported++;

	return bNoError;
}


// Import all .dcm files in the specified directory, including subdirectories.
BOOL CImportSelector::CopyDirectoryContents( char *pSourceDirectorySpec )
{
	char				SearchPath[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				LowerLevelDirectory[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				FullSourceFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	WIN32_FIND_DATA		FindFileInfo;
	HANDLE				hFindFile;
	BOOL				bFileFound;
	char				*pExtension;
	BOOL				bNoError = TRUE;

	strncpy_s( SearchPath, FULL_FILE_SPEC_STRING_LENGTH, pSourceDirectorySpec, _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
	if ( SearchPath[ strlen( SearchPath ) - 1 ] != '\\' )
		strncat_s( SearchPath, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );						// *[1] Replaced strcat with strncat_s.
	strncat_s( SearchPath, FULL_FILE_SPEC_STRING_LENGTH, "*.*", _TRUNCATE );						// *[1] Replaced strcat with strncat_s.
	hFindFile = FindFirstFile( SearchPath, &FindFileInfo );
	bFileFound = ( hFindFile != INVALID_HANDLE_VALUE );
	while ( bFileFound )
		{
		if ( ( FindFileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
			{
			if ( strcmp( FindFileInfo.cFileName, "." ) != 0 && strcmp( FindFileInfo.cFileName, ".." ) != 0 )
				{
				strncpy_s( LowerLevelDirectory, FULL_FILE_SPEC_STRING_LENGTH, pSourceDirectorySpec, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
				if ( LowerLevelDirectory[ strlen( LowerLevelDirectory ) - 1 ] != '\\' )
					strncat_s( LowerLevelDirectory, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );				// *[1] Replaced strcat with strncat_s.
				strncat_s( LowerLevelDirectory, FULL_FILE_SPEC_STRING_LENGTH, FindFileInfo.cFileName, _TRUNCATE );	// *[1] Replaced strcat with strncat_s.
				bNoError = CopyDirectoryContents( LowerLevelDirectory );
				}
			}
		else
			{
			pExtension = strrchr( FindFileInfo.cFileName, '.' );
			if ( ( pExtension == 0 && _strnicmp( FindFileInfo.cFileName, "DICOMDIR", 9 ) != 0 ) || (  pExtension != 0 &&  _stricmp( pExtension, ".dcm" ) == 0 ) )
				{
				strncpy_s( FullSourceFileSpec, FULL_FILE_SPEC_STRING_LENGTH, pSourceDirectorySpec, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
				if ( FullSourceFileSpec[ strlen( FullSourceFileSpec ) - 1 ] != '\\' )
					strncat_s( FullSourceFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );					// *[1] Replaced strcat with strncat_s.
				strncat_s( FullSourceFileSpec, FULL_FILE_SPEC_STRING_LENGTH, FindFileInfo.cFileName, _TRUNCATE );	// *[1] Replaced strcat with strncat_s.
				bNoError = CopyDesignatedFile( FullSourceFileSpec );
				}
			}
		// Look for another file in the source directory.
		bFileFound = FindNextFile( hFindFile, &FindFileInfo );
		}			// ...end while another file found.
	if ( hFindFile != INVALID_HANDLE_VALUE )
		FindClose( hFindFile );

	return bNoError;
}


void CImportSelector::EraseFileSpecList( IMAGE_FILE_SET_SPECIFICATION **ppFileSpecList )
{
	IMAGE_FILE_SET_SPECIFICATION		*pImageFileSetSpecification;
	IMAGE_FILE_SET_SPECIFICATION		*pPrevImageFileSetSpecification;

	pImageFileSetSpecification = *ppFileSpecList;
	while( pImageFileSetSpecification != 0 )
		{
		pPrevImageFileSetSpecification = pImageFileSetSpecification;
		pImageFileSetSpecification = pImageFileSetSpecification -> pNextFileSetStruct;
		free( pPrevImageFileSetSpecification );
		}
	*ppFileSpecList = 0;
}


static unsigned long		nNumberOfFilesVisited;
static unsigned long		nNumberOfDICOMDIRFilesFound;
static char					*pRefineSelectionMessage = "Try selecting fewer files to search.";
static char					*pReviseSelectionMessage = "Try importing available .dcm image files.";


void CImportSelector::OnExitImportSelector()
{
	static char			Msg[ MAX_EXTRA_LONG_STRING_LENGTH ];

	if ( m_TotalImageFilesImported == 1 )
		_snprintf_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, _TRUNCATE, "%d image\nis being imported.", m_TotalImageFilesImported );	// *[2] Replaced sprintf() with _snprintf_s.
	else
		_snprintf_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, _TRUNCATE, "%d images\nare being imported.", m_TotalImageFilesImported );	// *[2] Replaced sprintf() with _snprintf_s.
	ThisBViewerApp.MakeAnnouncement( Msg );
}


void CImportSelector::OnBnClickedRefreshView( NMHDR *pNMHDR, LRESULT *pResult )
{
	unsigned long	StorageDeviceMask;
	char			StorageDeviceSpecification[ 64 ];
	char			VolumeLabel[ 256 ];
	HTREEITEM		hRootItem;

	m_pExplorer -> DeleteAllItems();
	// List the available storage devices in the Tree Control.
	strncpy_s( StorageDeviceSpecification, 64, "A:", _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
	StorageDeviceMask = _getdrives();
	while ( StorageDeviceMask != 0 )
		{
		if ( StorageDeviceMask & 1 )
			{
			GetDriveLabel( StorageDeviceSpecification, VolumeLabel );
			hRootItem = m_pExplorer -> InsertItem( VolumeLabel, 0, 1, TVI_ROOT );
			if ( hRootItem != NULL )
				// Zero the state image (don't declare any bits) to eliminate the checkbox at this level.
				m_pExplorer -> SetItemState( hRootItem, TVIS_BOLD, TVIS_BOLD | TVIS_STATEIMAGEMASK );
			}
		StorageDeviceSpecification[ 0 ]++;
		StorageDeviceMask >>= 1;
		}
	m_pExplorer -> UpdateWindow();

	*pResult = 0;
}

// Import any checked items from the DICOMDIR file tree.
void CImportSelector::OnBnClickedImportCheckedItems( NMHDR *pNMHDR, LRESULT *pResult )
{
	IMAGE_FILE_SET_SPECIFICATION	*pImageFileSetSpecification;
	HTREEITEM						hTreeItem;
	IMAGE_FILE_SET_SPECIFICATION	*pListOfCheckedItems;
	IMAGE_FILE_SET_SPECIFICATION	*pCheckedFileSetItem;
	IMAGE_FILE_SET_SPECIFICATION	*pPrevCheckedFileSetItem;
	char							FullSourceFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
//	char							*pChar;
	TVITEM							TreeItemStruct;
	CWaitCursor						DisplaysHourglass;

	m_ListOfProcessedItemFileNames = 0;
	m_nDuplicateFileNamesDetected = 0;
	pImageFileSetSpecification = m_pListOfFileSetItems;
	pListOfCheckedItems = 0;
	pPrevCheckedFileSetItem = 0;
	while ( pImageFileSetSpecification != 0 )
		{
		hTreeItem = (HTREEITEM)pImageFileSetSpecification -> hTreeHandle;
		if ( hTreeItem != 0 )
			{
			// If the tree item is now part of a collapsed branch, its handle is no longer
			// valid.  Check this before looking at the state of the check box.  This avoids
			// a crash.
			memset( &TreeItemStruct, 0, sizeof(TreeItemStruct) );
			TreeItemStruct.hItem = hTreeItem;
			if ( m_pExplorer -> GetItem( &TreeItemStruct ) != 0 )
				{
				// If the item is checked...
				if ( m_pExplorer -> GetCheck( hTreeItem ) != 0 )
					{
					// Copy each checked item and link the copy to a temporary list.
					pCheckedFileSetItem = (IMAGE_FILE_SET_SPECIFICATION*)malloc( sizeof(IMAGE_FILE_SET_SPECIFICATION) );
					if ( pCheckedFileSetItem != 0 )
						{
						memcpy( (char*)pCheckedFileSetItem, (char*)pImageFileSetSpecification, sizeof(IMAGE_FILE_SET_SPECIFICATION) );
						pCheckedFileSetItem -> pNextFileSetStruct = 0;
						if ( pListOfCheckedItems == 0 )
							pListOfCheckedItems = pCheckedFileSetItem;
						else
							pPrevCheckedFileSetItem -> pNextFileSetStruct = pCheckedFileSetItem;
						pPrevCheckedFileSetItem = pCheckedFileSetItem;
						}
					}
				}
			}
		pImageFileSetSpecification = pImageFileSetSpecification -> pNextFileSetStruct;
		}			// ... end while another tree item found.
	// Process the list of checked items.
	pImageFileSetSpecification = pListOfCheckedItems;
	while ( pImageFileSetSpecification != 0 )
		{
		strncpy_s( FullSourceFileSpec, FULL_FILE_SPEC_STRING_LENGTH, pImageFileSetSpecification -> DICOMDIRFileSpec, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
		if ( FullSourceFileSpec[ strlen( FullSourceFileSpec ) - 1 ] != '\\' )
			strncat_s( FullSourceFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );											// *[3] Replaced strcat with strncat_s.
		strncat_s( FullSourceFileSpec, FULL_FILE_SPEC_STRING_LENGTH, pImageFileSetSpecification -> NodeInformation, _TRUNCATE );	// *[3] Replaced strcat with strncat_s.
		if ( pImageFileSetSpecification -> DicomNodeType == ITEM_IS_FOLDER )
			CopyDirectoryContents( FullSourceFileSpec );
		else
			CopyDesignatedFile( FullSourceFileSpec );
		pImageFileSetSpecification = pImageFileSetSpecification -> pNextFileSetStruct;
		}
	OnExitImportSelector();
	m_pExplorer -> DeleteAllItems();
	delete m_pExplorer;
	m_pExplorer = 0;
	EraseFileSpecList( &pListOfCheckedItems );
	EraseFileSpecList( &m_pListOfFileSetItems );
	EraseList( &m_ListOfProcessedItemFileNames );
	DestroyWindow();

	*pResult = 0;
}


void CImportSelector::OnBnClickedAutoImport( NMHDR *pNMHDR, LRESULT *pResult )
{
	BOOL					bNoError = TRUE;
	CString					FullFileSpec;
	int						DialogWidth;
	int						DialogHeight;
	RECT					WindowRect;
	
	DialogWidth = 700;
	DialogHeight = 700;
	if ( m_pImportDicomdir != 0 )
		delete m_pImportDicomdir;
	m_pImportDicomdir = new CImportDicomdir( m_bSelectionIsAFolder, m_bSelectionIsADICOMDIR, m_SelectedFileSpec, CloseImportSelector, DialogWidth, DialogHeight, COLOR_PATIENT, 0 );
	if ( m_pImportDicomdir != 0 )
		{
		GetWindowRect( &WindowRect );

		m_pImportDicomdir -> SetPosition( WindowRect.left, WindowRect.top, this, ExplorerWindowClass );
		m_pImportDicomdir -> BringWindowToTop();
		m_pImportDicomdir -> SetFocus();
		}
}


void CImportSelector::OnBnClickedImportCancel( NMHDR *pNMHDR, LRESULT *pResult )
{
	m_pExplorer -> DeleteAllItems();
	delete m_pExplorer;
	m_pExplorer = 0;
	EraseFileSpecList( &m_pListOfFileSetItems );
	EraseList( &m_ListOfProcessedItemFileNames );
	DestroyWindow();

	*pResult = 0;
}


BOOL CImportSelector::OnEraseBkgnd( CDC *pDC )
{
	CBrush		BackgroundBrush( m_BackgroundColor );
	CRect		BackgroundRectangle;
	CBrush		*pOldBrush = pDC -> SelectObject( &BackgroundBrush );

	GetClientRect( BackgroundRectangle );
	pDC -> FillRect( BackgroundRectangle, &BackgroundBrush );
	pDC -> SelectObject( pOldBrush );

	return TRUE;
}


void CImportSelector::DeleteChildItems( HTREEITEM hParentItem )
{
	HTREEITEM		hNextItem;
	HTREEITEM		hChildItem;

	// Delete all of the children of hParentItem.
	if ( m_pExplorer -> ItemHasChildren( hParentItem ) )
		{
		hChildItem = m_pExplorer -> GetChildItem( hParentItem );
		while ( hChildItem != NULL )
			{
			hNextItem = m_pExplorer -> GetNextItem( hChildItem, TVGN_NEXT );
			m_pExplorer -> DeleteItem( hChildItem );
			hChildItem = hNextItem;
			}
		}
}


// When an existing item on the tree control is selected, it is expanded
// to show its immediate subitems.  Any subitems that represent image files
// have checkboxes and may later be checked for selection by the user.
// Therefore, they must be linked to a list that can later be examined
// for checked items.
void CImportSelector::ExpandSelection( HTREEITEM hNewlySelectedItem )
{
	HTREEITEM						hParentItem;
	HTREEITEM						hNewItem;
	char							FileSpecComponent[ FULL_FILE_SPEC_STRING_LENGTH ];
	char							TempFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char							FullFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char							FoundFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char							SearchPath[ FULL_FILE_SPEC_STRING_LENGTH ];
	WIN32_FIND_DATA					FindFileInfo;
	HANDLE							hFindFile;
	BOOL							bFileFound;
	DWORD							FileAttributes;
	char							*pChar;
	char							*pExtension;
	IMAGE_FILE_SET_SPECIFICATION	*pNewImageFileSetSpecification;
	IMAGE_FILE_SET_SPECIFICATION	*pImageFileSetSpecification;
	BOOL							bItemAlreadyInList;
	
	FullFileSpec[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
	TempFileSpec[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
	hParentItem = hNewlySelectedItem;
	// Working the way up the tree from the selected item, compose the full
	// file specification for the selected item.  (The text for each item
	// consists only of the file or folder name for that item.)
	while ( hParentItem != NULL )
		{
		strncpy_s( FileSpecComponent, (const char*)m_pExplorer -> GetItemText( hParentItem ), _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
		if ( strlen( FileSpecComponent ) > 0 )
			{
			hParentItem = m_pExplorer -> GetParentItem( hParentItem );
			if ( hParentItem != NULL )
				{
				strncpy_s( FullFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
				strncat_s( FullFileSpec, FULL_FILE_SPEC_STRING_LENGTH, FileSpecComponent, _TRUNCATE );		// *[2] Replaced strcat with strncat_s.
				}
			else
				{
				pChar = strchr( FileSpecComponent, ':' );
				if ( pChar != 0 )
					{
					pChar--;
					strncpy_s( FullFileSpec, FULL_FILE_SPEC_STRING_LENGTH, pChar, 2 );						// *[2] Replaced strncat with strncpy_s.
					}
				}
			strncat_s( FullFileSpec, FULL_FILE_SPEC_STRING_LENGTH, TempFileSpec, _TRUNCATE );				// *[2] Replaced strcat with strncat_s.
			// Save the intermediate result for the next cycle.
			strncpy_s( TempFileSpec, FULL_FILE_SPEC_STRING_LENGTH, FullFileSpec, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
			}
		else
			hParentItem = NULL;
		}
	strncpy_s( m_SelectedFileSpec, FULL_FILE_SPEC_STRING_LENGTH, FullFileSpec, _TRUNCATE );					// *[2] Replaced strncat with strncpy_s.
	FileAttributes = GetFileAttributes( m_SelectedFileSpec );
	m_bSelectionIsAFolder = ( ( FileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 );
	m_bSelectionIsADICOMDIR = FALSE;
	if ( !m_bSelectionIsAFolder )
		m_bSelectionIsADICOMDIR = ( _stricmp( "DICOMDIR", (const char*)m_pExplorer -> GetItemText( hNewlySelectedItem ) ) == 0 );

	// Perform the expansion of the selected item by searching through the selected
	// folder, assuming the selection was a folder.
	DeleteChildItems( hNewlySelectedItem );
	strncpy_s( SearchPath, FULL_FILE_SPEC_STRING_LENGTH, FullFileSpec, _TRUNCATE );							// *[1] Replaced strcpy with strncpy_s.
	strncat_s( SearchPath, FULL_FILE_SPEC_STRING_LENGTH, "\\*.*", _TRUNCATE );								// *[2] Replaced strcat with strncat_s.

	// First search for all the folders so they will be listed first.
	// Locate the first file or directory member in the current search directory.
	hFindFile = FindFirstFile( SearchPath, &FindFileInfo );
	bFileFound = ( hFindFile != INVALID_HANDLE_VALUE );
	while ( bFileFound )
		{
		if ( strchr( FindFileInfo.cFileName, '\'' ) != 0 )
			bFileFound = FindNextFile( hFindFile, &FindFileInfo );
		// If the entry found in the search folder is not a subdirectory...
		if ( bFileFound && ( ( FindFileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 ||
				( strcmp( FindFileInfo.cFileName, "." ) != 0 && strcmp( FindFileInfo.cFileName, ".." ) != 0 ) )  )
			{
			strncpy_s( FoundFileSpec, FULL_FILE_SPEC_STRING_LENGTH, FullFileSpec, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
			strncat_s( FoundFileSpec, FULL_FILE_SPEC_STRING_LENGTH, FindFileInfo.cFileName, _TRUNCATE );	// *[2] Replaced strcat with strncat_s.
			// A check box is displayed only if an image is associated with the item. The control effectively
			// uses DrawFrameControl to create and set a state image list containing two images. State image 1 is the unchecked box
			// and state image 2 is the checked box. Setting the state image to zero removes the check box altogether. 
			if ( ( FindFileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
				{
				// By default, all items display the first image in the image list for both the selected and nonselected states
				// You can change the default behavior for a particular item by specifying the indexes of the selected and
				// nonselected images when adding the item to the tree control using the InsertItem member function. You can
				// change the indexes after adding an item by using the SetItemImage member function. 

				// Since this is a folder item, show the folder icon.
				hNewItem = m_pExplorer -> InsertItem( FindFileInfo.cFileName, ICON_FOLDER_UNSELECTED, ICON_FOLDER_SELECTED, hNewlySelectedItem );
				// Any item that is "checkable" needs an entry to be added to the list of
				// importable files or folders:
				// Add a new fileset item link, with a defined node type.
				pNewImageFileSetSpecification = (IMAGE_FILE_SET_SPECIFICATION*)calloc( 1, sizeof(IMAGE_FILE_SET_SPECIFICATION) );
				if ( pNewImageFileSetSpecification != 0 )
					{
					pNewImageFileSetSpecification -> DicomNodeType = ITEM_IS_FOLDER;
					pNewImageFileSetSpecification -> pNextFileSetStruct = 0;
					pNewImageFileSetSpecification -> hTreeHandle = (INT_PTR)hNewItem;
					strncpy_s( pNewImageFileSetSpecification -> NodeInformation, FULL_FILE_SPEC_STRING_LENGTH, FindFileInfo.cFileName, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
					strncpy_s( pNewImageFileSetSpecification -> DICOMDIRFileSpec, FULL_FILE_SPEC_STRING_LENGTH, FullFileSpec, _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.

					// Link this new item to the list if it is not already there.
					pImageFileSetSpecification = m_pListOfFileSetItems;
					if ( pImageFileSetSpecification == 0 )
						m_pListOfFileSetItems = pNewImageFileSetSpecification;
					else
						{
						bItemAlreadyInList = FALSE;
						while ( pImageFileSetSpecification != 0 && pImageFileSetSpecification -> pNextFileSetStruct != 0 && !bItemAlreadyInList )
							{
							bItemAlreadyInList = ( strcmp( pImageFileSetSpecification -> DICOMDIRFileSpec, pNewImageFileSetSpecification -> DICOMDIRFileSpec ) == 0 &&
													strcmp( pImageFileSetSpecification -> NodeInformation, pNewImageFileSetSpecification -> NodeInformation ) == 0 );
							pImageFileSetSpecification = pImageFileSetSpecification -> pNextFileSetStruct;
							}
						if ( pImageFileSetSpecification != 0 && !bItemAlreadyInList )
							pImageFileSetSpecification -> pNextFileSetStruct = pNewImageFileSetSpecification;
						}
					}
				}
			}

				// Look for another item in the source directory.
		bFileFound = FindNextFile( hFindFile, &FindFileInfo );
		}			// ...end while another file found.

	// Now repeat the directory search, this time picking out files and ignoring folders, which
	// have already been listed.
	hFindFile = FindFirstFile( SearchPath, &FindFileInfo );
	bFileFound = ( hFindFile != INVALID_HANDLE_VALUE );
	while ( bFileFound )
		{
		if ( strchr( FindFileInfo.cFileName, '\'' ) != 0 )
			bFileFound = FindNextFile( hFindFile, &FindFileInfo );
		// If the entry found in the search folder is not a subdirectory...
		if ( bFileFound && ( ( FindFileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 ||
				( strcmp( FindFileInfo.cFileName, "." ) != 0 && strcmp( FindFileInfo.cFileName, ".." ) != 0 ) )  )
			{
			strncpy_s( FoundFileSpec, FULL_FILE_SPEC_STRING_LENGTH, FullFileSpec, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
			strncat_s( FoundFileSpec, FULL_FILE_SPEC_STRING_LENGTH, FindFileInfo.cFileName, _TRUNCATE );	// *[2] Replaced strcat with strncat_s.
			// A check box is displayed only if an image is associated with the item. The control effectively
			// uses DrawFrameControl to create and set a state image list containing two images. State image 1 is the unchecked box
			// and state image 2 is the checked box. Setting the state image to zero removes the check box altogether. 
			if ( ( FindFileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
				{
				pExtension = strrchr( FindFileInfo.cFileName, '.' );
				// Flag any file with a .dcm extension as a dicom image file by attaching a special icon to draw attention to it.
				if ( pExtension != 0 && _stricmp( pExtension, ".dcm" ) == 0 )
					hNewItem = m_pExplorer -> InsertItem( FindFileInfo.cFileName, ICON_IMAGE_UNSELECTED, ICON_IMAGE_SELECTED, hNewlySelectedItem );
				else
					{
					// Since this entry isn't a folder, don't show the folder icon, but also don't disable
					// the checkbox.  This new item is linked as a child item under hNewlySelectedItem.
					hNewItem = m_pExplorer -> InsertItem( FindFileInfo.cFileName, ICON_OMITTED, ICON_OMITTED, hNewlySelectedItem );
					}

				// Any item that is "checkable" needs an entry to be added to the list of
				// importable files:
				// Add a new fileset item link, with a defined node type.
				pNewImageFileSetSpecification = (IMAGE_FILE_SET_SPECIFICATION*)calloc( 1, sizeof(IMAGE_FILE_SET_SPECIFICATION) );
				if ( pNewImageFileSetSpecification != 0 )
					{
					pNewImageFileSetSpecification -> DicomNodeType = ITEM_IS_IMAGE;
					pNewImageFileSetSpecification -> pNextFileSetStruct = 0;
					pNewImageFileSetSpecification -> hTreeHandle = (INT_PTR)hNewItem;
					strncpy_s( pNewImageFileSetSpecification -> NodeInformation, FULL_FILE_SPEC_STRING_LENGTH, FindFileInfo.cFileName, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
					strncpy_s( pNewImageFileSetSpecification -> DICOMDIRFileSpec, FULL_FILE_SPEC_STRING_LENGTH, FullFileSpec, _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
	
					// Link this new item to the list if it is not already there.
					pImageFileSetSpecification = m_pListOfFileSetItems;
					if ( pImageFileSetSpecification == 0 )
						m_pListOfFileSetItems = pNewImageFileSetSpecification;
					else
						{
						bItemAlreadyInList = FALSE;
						while ( pImageFileSetSpecification != 0 && pImageFileSetSpecification -> pNextFileSetStruct != 0 && !bItemAlreadyInList )
							{
							bItemAlreadyInList = ( strcmp( pImageFileSetSpecification -> DICOMDIRFileSpec, pNewImageFileSetSpecification -> DICOMDIRFileSpec ) == 0 &&
													strcmp( pImageFileSetSpecification -> NodeInformation, pNewImageFileSetSpecification -> NodeInformation ) == 0 );
							pImageFileSetSpecification = pImageFileSetSpecification -> pNextFileSetStruct;
							}
						if ( pImageFileSetSpecification != 0 && !bItemAlreadyInList )
							pImageFileSetSpecification -> pNextFileSetStruct = pNewImageFileSetSpecification;
						}
					}
				}
			}

		// Look for another file in the source directory.
		bFileFound = FindNextFile( hFindFile, &FindFileInfo );
		}			// ...end while another file found.

	if ( hFindFile != INVALID_HANDLE_VALUE )
		FindClose( hFindFile );
	m_pExplorer -> Expand( hNewlySelectedItem, TVE_EXPAND );
}


// NOTE:  The parent item, if expanded, is collapsed on a double-click of the mouse.
BOOL CImportSelector::OnNotify( WPARAM wParam, LPARAM lParam, LRESULT *pResult )
{
	NMTREEVIEW			*pTreeViewNotice;
	TVITEM				PreviouslySelectedItem;
	TVITEM				NewlySelectedItem;
	
	pTreeViewNotice = (LPNMTREEVIEW)lParam;
	if ( pTreeViewNotice != 0 )
		{
		if ( pTreeViewNotice -> hdr.code == TVN_SELCHANGED )
			{
			PreviouslySelectedItem = pTreeViewNotice -> itemOld;
			NewlySelectedItem = pTreeViewNotice -> itemNew;
			if ( NewlySelectedItem.hItem != 0 )
				{
				m_SelectedItem = NewlySelectedItem.hItem;
				if ( ( TVIS_EXPANDED & m_pExplorer -> GetItemState( m_SelectedItem, TVIS_EXPANDED ) ) == 0 )
					ExpandSelection( m_SelectedItem );
				}
			}
		}

	return CWnd::OnNotify( wParam, lParam, pResult );
}


void CImportSelector::CloseThisWindow()
{
	m_pExplorer -> DeleteAllItems();
	delete m_pExplorer;
	m_pExplorer = 0;
	EraseFileSpecList( &m_pListOfFileSetItems );
	DestroyWindow();
}


void CloseImportSelector( void *pParentWindow )
{
	CImportSelector		*pActiveImportSelector;

	pActiveImportSelector = (CImportSelector*)pParentWindow;
	if ( pActiveImportSelector != 0 )
		pActiveImportSelector -> CloseThisWindow();
}


