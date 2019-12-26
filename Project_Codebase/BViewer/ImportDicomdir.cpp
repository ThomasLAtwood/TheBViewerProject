// ImportDicomdir.cpp : Implementation file for the CImportDicomdir
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
#include "stdafx.h"
#include <direct.h>
#include <stdio.h>
#include <errno.h>
#include <afxcmn.h>
#include "BViewer.h"
#include "ImportDicomdir.h"


extern CONFIGURATION				BViewerConfiguration;
extern CBViewerApp					ThisBViewerApp;

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

static MODULE_INFO		ImportDicomdirModuleInfo = { MODULE_IMPORT_DICOMDIR, "Import DicomDir Module", InitImportDicomdirModule, CloseImportDicomdirModule };


static ERROR_DICTIONARY_ENTRY	ImportDicomdirErrorCodes[] =
			{
				{ IMPORT_DICOMDIR_ERROR_INSUFFICIENT_MEMORY		, "An error occurred allocating a memory block for data storage." },
				{ IMPORT_DICOMDIR_ERROR_FILE_MOVE				, "An error occurred attempting to stage the imported image file." },
				{ IMPORT_DICOMDIR_ERROR_DICOMDIR_READ			, "An error occurred attempting to read a DICOMDIR file." },
				{ IMPORT_DICOMDIR_ERROR_NO_DICOMDIR_FILES		, "Your search encountered no DICOMDIR file sets." },
				{ 0												, NULL }
			};

static ERROR_DICTIONARY_MODULE		ImportDicomdirStatusErrorDictionary =
										{
										MODULE_IMPORT_DICOMDIR,
										ImportDicomdirErrorCodes,
										IMPORT_DICOMDIR_ERROR_DICT_LENGTH,
										0
										};

// This function must be called before any other function in this module.
void InitImportDicomdirModule()
{
	LinkModuleToList( &ImportDicomdirModuleInfo );
	RegisterErrorDictionary( &ImportDicomdirStatusErrorDictionary );
}


void CloseImportDicomdirModule()
{
}


// CImportDicomdir
CImportDicomdir::CImportDicomdir( BOOL bSelectionIsAFolder, BOOL bSelectionIsADICOMDIR, char *pSelectedFileSpec,
									IMPORT_CALLBACK_FUNCTION CallbackFunction,
									int DialogWidth, int DialogHeight, COLORREF BackgroundColor, DWORD WindowStyle ):
				m_StaticUserMessage( "Select Subject Study Image Files to be Imported", 500, 40, 18, 9, 6, COLOR_WHITE, COLOR_PATIENT, COLOR_PATIENT,
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_MESSAGE_FOR_USER ),
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
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_IMPORT_DICOMDIR ),
				m_ButtonImportCancel( "Cancel", 150, 30, 14, 7, 6,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_IMPORT_DICOMDIR_CANCEL )
{
	m_DialogWidth = DialogWidth;
	m_DialogHeight = DialogHeight;
	m_BackgroundColor = BackgroundColor;
	m_WindowStyle = WindowStyle;
	m_pExplorer = new CTreeCtrl;
	m_pListOfFileSetItems = 0;
	m_TotalImageFilesImported = 0;
	strcpy( m_SelectedFileSpec, pSelectedFileSpec );
	m_bSelectionIsAFolder = bSelectionIsAFolder;
	m_bSelectionIsADICOMDIR = bSelectionIsADICOMDIR;
	m_CallbackFunction = CallbackFunction;
}

CImportDicomdir::~CImportDicomdir()
{
	if ( m_pExplorer != 0 )
		delete m_pExplorer;
}


BEGIN_MESSAGE_MAP( CImportDicomdir, CWnd )
	//{{AFX_MSG_MAP(CImportDicomdir)
	ON_WM_CREATE()
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_IMPORT_DICOMDIR, OnBnClickedImportCheckedItems )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_IMPORT_DICOMDIR_CANCEL, OnBnClickedImportCancel )
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



// Caution:		Since this function creates the object, it must be called before any
//				functions such as Invalidate(), etc., that manipulate an active window.
BOOL CImportDicomdir::SetPosition( int x, int y, CWnd *pParentWnd, CString WindowClass )
{
	BOOL			bResult;
	CRect			DialogRect;
	DWORD			WindowsStyle;

	WindowsStyle = DS_MODALFRAME | WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_EX_TOPMOST;
	DialogRect.SetRect( x, y, x + m_DialogWidth, y + m_DialogHeight );
	bResult = CreateEx( WS_EX_DLGMODALFRAME, (const char*)WindowClass, "Select DicomDir Images for Importing", WindowsStyle, DialogRect, pParentWnd, 0, NULL );
	
	return bResult;
}


int CImportDicomdir::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
		return -1;

	m_Row1YOffset = 10;
	m_Row2YOffset = m_Row1YOffset + 50;
	m_Column1XOffset = 30;
	m_Column2XOffset = m_DialogWidth - 180;
	m_StaticUserMessage.SetPosition( m_Column1XOffset, m_Row1YOffset, this );
	m_StaticStep4.SetPosition( m_Column2XOffset - 20, m_Row2YOffset + 350, this );
	m_StaticStep4Text.SetPosition( m_Column2XOffset, m_Row2YOffset + 370, this );

	m_ButtonImport.SetPosition( m_Column2XOffset, m_Row2YOffset + 450, this );
	m_ButtonImportCancel.SetPosition( m_Column2XOffset, m_DialogHeight - 80, this );
	
	// TVS_CHECKBOXES: 
	// Enables check boxes for items in a tree-view control. A check box is displayed only if an image is associated with the item. When set to this style, the control
	//effectively uses DrawFrameControl to create and set a state image list containing two images. State image 1 is the unchecked box and state image 2 is the checked
	// box. Setting the state image to zero removes the check box altogether. 
	m_pExplorer -> Create( WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | TVS_HASLINES | TVS_TRACKSELECT |  TVS_SHOWSELALWAYS | TVS_CHECKBOXES | TVS_DISABLEDRAGDROP,
							CRect( m_Column1XOffset, m_Row2YOffset, m_Column2XOffset - 30, m_DialogHeight - 80 + 30 ), this, IDC_TREE_CTRL_DICOMDIR_EXPLORER );
	m_FolderIcons.Create( 16, 16, ILC_COLOR32, 6, 6 );
	
	// NOTE:  Icon files need to be 16 x 16 pixel .bmp files with 24-bit color and without color information.
	m_DriveBitmap.LoadBitmap( IDB_DRIVE_BITMAP );
	m_FolderBitmap.LoadBitmap( IDB_FOLDER_BITMAP );
	m_FolderOpenBitmap.LoadBitmap( IDB_FOLDER_OPEN_BITMAP );
	m_DicomImageBitmap.LoadBitmap( IDB_IMAGE_BITMAP );
	// Create the CImageList of drive and folder Icons.  The associated symbol values must correspond with this sequential order.
	m_FolderIcons.Add( &m_DriveBitmap, RGB(0,0,0) );		// ICON_DRIVE_UNSELECTED
	m_FolderIcons.Add( &m_DriveBitmap, RGB(0,0,0) );		// ICON_DRIVE_SELECTED
	m_FolderIcons.Add( &m_FolderBitmap, RGB(0,0,0) );		// ICON_FOLDER_UNSELECTED
	m_FolderIcons.Add( &m_FolderOpenBitmap, RGB(0,0,0) );	// ICON_FOLDER_SELECTED
	m_FolderIcons.Add( &m_DicomImageBitmap, RGB(0,0,0) );	// ICON_IMAGE_UNSELECTED
	m_FolderIcons.Add( &m_DicomImageBitmap, RGB(0,0,0) );	// ICON_IMAGE_SELECTED

	// Assign the list of item images (icons) to be associated with this CTreeCtrl.
	m_pExplorer -> SetImageList( &m_FolderIcons, TVSIL_NORMAL );

	DisplayDicomdirFileTree();

	return 0;
}

static char			*pTechSupportMessage = "Request technical support.";

// Import a designated file into the BRetriever watch folder, from which
// it will be picked up and processed.
BOOL CImportDicomdir::CopyDesignatedFile( char *pSourceImageFileSpec )
{
	BOOL					bNoError = TRUE;
	char					InitialOutputImageFileSpec[ FILE_PATH_STRING_LENGTH ];
	char					RevisedOutputImageFileSpec[ FILE_PATH_STRING_LENGTH ];
	char					CurrentFileNameWithExtension[ FILE_PATH_STRING_LENGTH ];
	LIST_ELEMENT			*pListElement;
	char					*pEarlierFileName;
	char					*pCurrentFileName;
	char					Msg[ 512 ];
	char					*pChar;
	char					*pExtension;
	char					Version[ 20 ];
	DWORD					FileAttributes;
	int						RenameResult;

	if ( strlen( pSourceImageFileSpec ) > 0 )
		{
		pCurrentFileName = (char*)malloc( FILE_PATH_STRING_LENGTH );
		bNoError = ( pCurrentFileName != 0 );
		if ( bNoError )
			{
			// Extract the source file name with the file extension (if any) removed.
			strcpy( pCurrentFileName, "" );
			pChar = strrchr( pSourceImageFileSpec, '\\' );
			pChar++;
			strncat( pCurrentFileName, pChar, FILE_PATH_STRING_LENGTH - 1 );
			pExtension = strrchr( pCurrentFileName, '.' );
			if ( pExtension != 0 )
				*pExtension = '\0';
			// Compare the file name with those of files already processed.  If there is duplication,
			//  resolve it.  Then append the current file spec to the list.
			strcpy( CurrentFileNameWithExtension, pCurrentFileName );
			strncat( CurrentFileNameWithExtension,  ".dcm", FILE_PATH_STRING_LENGTH - strlen( CurrentFileNameWithExtension ) - 1 );
			pListElement = m_ListOfProcessedItemFileNames;
			while ( pListElement != 0 )
				{
				pEarlierFileName = (char*)pListElement -> pItem;
				if ( pEarlierFileName != 0 && strcmp( pEarlierFileName, CurrentFileNameWithExtension ) == 0 )
					{
					m_nDuplicateFileNamesDetected++;		// Make each resolved duplicate file name unique.
					sprintf( Version, "_Instance%d", m_nDuplicateFileNamesDetected );
					strncat( pCurrentFileName, Version, FILE_PATH_STRING_LENGTH - strlen( pCurrentFileName ) - 1 );
					}
				pListElement = pListElement -> pNextListElement;
				}
			// Add the Dicom extension, whether it was originally present or not.
			strncat( pCurrentFileName,  ".dcm", FILE_PATH_STRING_LENGTH - strlen( pCurrentFileName ) - 1 );
			// Add the name of the current file to the list of files to be checked for duplication.
			AppendToList( &m_ListOfProcessedItemFileNames, pCurrentFileName );
			}
		// Copy the file to the Inbox directory.
		strcpy( InitialOutputImageFileSpec, BViewerConfiguration.InboxDirectory );
		if ( InitialOutputImageFileSpec[ strlen( InitialOutputImageFileSpec ) - 1 ] != '\\' )
			strcat( InitialOutputImageFileSpec, "\\" );
		strncat( InitialOutputImageFileSpec, pCurrentFileName,
							FILE_PATH_STRING_LENGTH - strlen( InitialOutputImageFileSpec ) - 1 );

		if ( bNoError )
			{
			// First, copy the file to the Inbox directory.
			bNoError = CopyFile( pSourceImageFileSpec, InitialOutputImageFileSpec, FALSE );
			if ( bNoError )
				{
				FileAttributes = GetFileAttributes( InitialOutputImageFileSpec );
				// Remove the read-only attribute, if set.
				FileAttributes &= ~FILE_ATTRIBUTE_READONLY;
				SetFileAttributes( InitialOutputImageFileSpec, FileAttributes );
				// Rename it over to the Watch Folder, where BRetriever will pick it up and process it.
				strcpy( RevisedOutputImageFileSpec, BViewerConfiguration.WatchDirectory );
				if ( RevisedOutputImageFileSpec[ strlen( RevisedOutputImageFileSpec ) - 1 ] != '\\' )
					strcat( RevisedOutputImageFileSpec, "\\" );
				strncat( RevisedOutputImageFileSpec, pCurrentFileName,
									FILE_PATH_STRING_LENGTH - strlen( RevisedOutputImageFileSpec ) - 1 );
				// Then rename it into the Watch directory.  This two-stage file movement avoids having
				// BRetriever try to grab the file for processing while it is still being copied into
				// the Watch directory.  The rename operation is just a modification of a directory
				// entry, so the file appears in the watch directory instantaneously.
				RenameResult = rename( InitialOutputImageFileSpec, RevisedOutputImageFileSpec );
				if ( RenameResult != 0 )
					{
					bNoError = FALSE;
					strcpy( Msg, "Unable to import\n" );
					strcat( Msg, InitialOutputImageFileSpec );
					ThisBViewerApp.NotifyUserOfImportSearchStatus( IMPORT_DICOMDIR_ERROR_FILE_MOVE, Msg, pTechSupportMessage );
					}
				}
			else
				{
				strcpy( Msg, "Unable to stage\n" );
				strcat( Msg, InitialOutputImageFileSpec );
				strcat( Msg, "\nfor import." );
				ThisBViewerApp.NotifyUserOfImportSearchStatus( IMPORT_DICOMDIR_ERROR_FILE_MOVE, Msg, pTechSupportMessage );
				}
			}
		}
	if ( bNoError )
		m_TotalImageFilesImported++;

	return bNoError;
}


void CImportDicomdir::EraseFileSpecList( IMAGE_FILE_SET_SPECIFICATION **ppFileSpecList )
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


BOOL CImportDicomdir::ReadDicomDirectoryFile( char *pDicomdirFileSpec )
{
	BOOL							bNoError = TRUE;
	DICOM_HEADER_SUMMARY			*pDicomHeader;
	IMAGE_FILE_SET_SPECIFICATION	*pImageFileSetSpecification;
	char							FullSourceFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char							*pChar;
	char							Msg[ 512 ];

	pDicomHeader = (DICOM_HEADER_SUMMARY*)malloc( sizeof(DICOM_HEADER_SUMMARY) );
	if ( pDicomHeader == 0 )
		{
		bNoError = FALSE;
		RespondToError( MODULE_IMPORT, IMPORT_DICOMDIR_ERROR_INSUFFICIENT_MEMORY );
		}
	else
		{
		// Decode and parse the Dicom information from the file.  The Dicom file contents
		// are retained in a series of memory buffers in the list, pDicomHeader -> ListOfInputBuffers.
		// A DICOMDIR file will contain element (4,1430), from which the ListOfImageFileSetSpecifications
		// is populated.
		bNoError = ReadDicomHeaderInfo( pDicomdirFileSpec, &pDicomHeader, TRUE );

		if ( bNoError )
			{
			// Set up the absolute file specifications.
			pImageFileSetSpecification = pDicomHeader -> ListOfImageFileSetSpecifications;
			while ( pImageFileSetSpecification != 0 )
				{
				strcpy( pImageFileSetSpecification -> DICOMDIRFileSpec, pDicomdirFileSpec );
				if ( pImageFileSetSpecification -> DicomNodeType == DICOM_NODE_IMAGE )
					{
					strcpy( FullSourceFileSpec, pDicomdirFileSpec );
					pChar = strrchr( FullSourceFileSpec, '\\' );
					if ( pChar != 0 )
						{
						pChar++;
						strcpy( pChar, pImageFileSetSpecification -> NodeInformation );
						}
					}
				pImageFileSetSpecification = pImageFileSetSpecification -> pNextFileSetStruct;
				}
			}
		else
			{
			strcpy( Msg, "An error occurred interpreting\nthe Dicom file set information from" );
			strcat( Msg, pDicomdirFileSpec );
			strcat( Msg, "\nfor import." );
			ThisBViewerApp.NotifyUserOfImportSearchStatus( IMPORT_DICOMDIR_ERROR_DICOMDIR_READ, Msg, pTechSupportMessage );
			}
		if ( bNoError )
			{
			// Append the new list of filespec items to the import master list.
			if ( m_pListOfFileSetItems == 0 )
				m_pListOfFileSetItems = pDicomHeader -> ListOfImageFileSetSpecifications;
			else
				{
				pImageFileSetSpecification = m_pListOfFileSetItems;
				// Point to the end of the master import list.
				while ( pImageFileSetSpecification != 0 && pImageFileSetSpecification -> pNextFileSetStruct != 0 )
					pImageFileSetSpecification = pImageFileSetSpecification -> pNextFileSetStruct;
				pImageFileSetSpecification -> pNextFileSetStruct = pDicomHeader -> ListOfImageFileSetSpecifications;
				}
			}
		DeallocateInputBuffers( pDicomHeader );
		FreeDicomHeaderInfo( pDicomHeader );
		free( pDicomHeader );
		}

	return bNoError;
}

static unsigned long		nNumberOfFilesVisited;
static unsigned long		nNumberOfDICOMDIRFilesFound;
static char					*pRefineSelectionMessage = "Try selecting fewer files to search.";
static char					*pReviseSelectionMessage = "Try importing available .dcm image files.";


BOOL CImportDicomdir::SearchForDICOMDIRFiles( char *pSourceDirectorySpec )
{
	char				SearchPath[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				LowerLevelDirectory[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				FullSourceFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	WIN32_FIND_DATA		FindFileInfo;
	HANDLE				hFindFile;
	BOOL				bFileFound;
	BOOL				bNoError = TRUE;
	char				Msg[ 512 ];
	CWaitCursor			DisplaysHourglass;

	strcpy( SearchPath, pSourceDirectorySpec );
	if ( SearchPath[ strlen( SearchPath ) - 1 ] != '\\' )
		strcat( SearchPath, "\\" );
	strcat( SearchPath, "*.*" );
	hFindFile = FindFirstFile( SearchPath, &FindFileInfo );
	bFileFound = ( hFindFile != INVALID_HANDLE_VALUE );
	while ( bNoError && bFileFound )
		{
		nNumberOfFilesVisited++;
		if ( nNumberOfFilesVisited >= 10000 )
			{
			bNoError = FALSE;
			strcpy( Msg, "Your requested search\nencountered over 10,000 files." );
			ThisBViewerApp.NotifyUserOfImportSearchStatus( IMPORT_DICOMDIR_ERROR_DICOMDIR_READ, Msg, pRefineSelectionMessage );
			nNumberOfFilesVisited = 0L;
			}
		if ( bNoError )
			{
			if ( ( FindFileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
				{
				if ( strcmp( FindFileInfo.cFileName, "." ) != 0 && strcmp( FindFileInfo.cFileName, ".." ) != 0 )
					{
					strcpy( LowerLevelDirectory, pSourceDirectorySpec );
					if ( LowerLevelDirectory[ strlen( LowerLevelDirectory ) - 1 ] != '\\' )
						strcat( LowerLevelDirectory, "\\" );
					strcat( LowerLevelDirectory, FindFileInfo.cFileName );
					bNoError = SearchForDICOMDIRFiles( LowerLevelDirectory );
					}
				}
			else
				{
				if ( _stricmp( FindFileInfo.cFileName, "DICOMDIR" ) == 0 )
					{
					nNumberOfDICOMDIRFilesFound++;
					strcpy( FullSourceFileSpec, pSourceDirectorySpec );
					if ( FullSourceFileSpec[ strlen( FullSourceFileSpec ) - 1 ] != '\\' )
						strcat( FullSourceFileSpec, "\\" );
					strcat( FullSourceFileSpec, FindFileInfo.cFileName );
					bNoError = ReadDicomDirectoryFile( FullSourceFileSpec );
					}
				}
			// Look for another file in the source directory.
			bFileFound = FindNextFile( hFindFile, &FindFileInfo );
			}
		}			// ...end while another file found.
	if ( hFindFile != INVALID_HANDLE_VALUE )
		FindClose( hFindFile );
	
	return bNoError;
}


void CImportDicomdir::OnExitImportDicomdirSelector()
{
	static char			Msg[ 512 ];

	if ( m_TotalImageFilesImported == 1 )
		sprintf( Msg, "%d image\nis being imported.", m_TotalImageFilesImported );
	else
		sprintf( Msg, "%d images\nare being imported.", m_TotalImageFilesImported );
	ThisBViewerApp.MakeAnnouncement( Msg );
}


// Import any checked items from the DICOMDIR file tree.
void CImportDicomdir::OnBnClickedImportCheckedItems( NMHDR *pNMHDR, LRESULT *pResult )
{
	IMAGE_FILE_SET_SPECIFICATION	*pImageFileSetSpecification;
	HTREEITEM						hTreeItem;
	IMAGE_FILE_SET_SPECIFICATION	*pListOfCheckedItems;
	IMAGE_FILE_SET_SPECIFICATION	*pCheckedFileSetItem;
	IMAGE_FILE_SET_SPECIFICATION	*pPrevCheckedFileSetItem;
	char							FullSourceFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char							*pChar;
	CWnd							*pParentWindow;
	CWaitCursor						DisplaysHourglass;

	m_ListOfProcessedItemFileNames = 0;
	m_nDuplicateFileNamesDetected = 0;
	pImageFileSetSpecification = m_pListOfFileSetItems;
	pListOfCheckedItems = 0;
	pPrevCheckedFileSetItem = 0;
	while ( pImageFileSetSpecification != 0 )
		{
		if ( pImageFileSetSpecification -> DicomNodeType == DICOM_NODE_IMAGE )
			{
			hTreeItem = (HTREEITEM)pImageFileSetSpecification -> hTreeHandle;
			if ( hTreeItem != 0 && m_pExplorer -> GetCheck( hTreeItem ) != 0 )
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
		pImageFileSetSpecification = pImageFileSetSpecification -> pNextFileSetStruct;
		}			// ... end while another tree item found.
	// Process the list of checked items.
	pImageFileSetSpecification = pListOfCheckedItems;
	while ( pImageFileSetSpecification != 0 )
		{
		strcpy( FullSourceFileSpec, pImageFileSetSpecification -> DICOMDIRFileSpec );
		pChar = strrchr( FullSourceFileSpec, '\\' );
		if ( pChar != 0 )
			{
			pChar++;
			strcpy( pChar, pImageFileSetSpecification -> NodeInformation );
			CopyDesignatedFile( FullSourceFileSpec );
			}
		pImageFileSetSpecification = pImageFileSetSpecification -> pNextFileSetStruct;
		}
	OnExitImportDicomdirSelector();
	m_pExplorer -> DeleteAllItems();
	delete m_pExplorer;
	m_pExplorer = 0;
	EraseFileSpecList( &pListOfCheckedItems );
	EraseFileSpecList( &m_pListOfFileSetItems );
	EraseList( &m_ListOfProcessedItemFileNames );
	pParentWindow = GetParent();
	if ( m_CallbackFunction != 0 && pParentWindow != 0 )
		m_CallbackFunction( pParentWindow );
	DestroyWindow();

	*pResult = 0;
}


// Display any DICOMDIR filesets from the currently open folder.
void CImportDicomdir::DisplayDicomdirFileTree()
{
	BOOL							bNoError = TRUE;
	IMAGE_FILE_SET_SPECIFICATION	*pImageFileSetSpecification;
	HTREEITEM						hTreeItem;
	HTREEITEM						hCurrentPatientItem;
	HTREEITEM						hCurrentStudyItem;
	HTREEITEM						hCurrentSeriesItem;
	char							*pFileName;
	char							*pExtension;
	CWaitCursor						DisplaysHourglass;
	char							Msg[ 512 ];

	EraseFileSpecList( &m_pListOfFileSetItems );
	if ( m_bSelectionIsAFolder )
		{
		nNumberOfFilesVisited = 0L;
		nNumberOfDICOMDIRFilesFound = 0L;
		bNoError = SearchForDICOMDIRFiles( m_SelectedFileSpec );
		if ( nNumberOfDICOMDIRFilesFound == 0L )
			{
			bNoError = FALSE;
			strcpy( Msg, "Your requested search encountered\n No DICOMDIR structured file sets." );
			ThisBViewerApp.NotifyUserOfImportSearchStatus( IMPORT_DICOMDIR_ERROR_NO_DICOMDIR_FILES, Msg, pReviseSelectionMessage );
			}
		}
	else if ( m_bSelectionIsADICOMDIR )
		bNoError = ReadDicomDirectoryFile( m_SelectedFileSpec );
	// Repopulate the CTreeCtrl with the subject study image heirarchy.
	if ( bNoError && m_pListOfFileSetItems != 0 )
		{
		m_pExplorer -> DeleteAllItems();
		m_SelectedItem = 0;

		pImageFileSetSpecification = m_pListOfFileSetItems;
		while ( pImageFileSetSpecification != 0 )
			{
			switch ( pImageFileSetSpecification -> DicomNodeType )
				{
				case DICOM_NODE_PATIENT:
					hCurrentPatientItem = m_pExplorer -> InsertItem( pImageFileSetSpecification -> NodeInformation, ICON_DRIVE_UNSELECTED, ICON_DRIVE_SELECTED, TVI_ROOT );
					if ( hCurrentPatientItem != NULL )
						{
						// Zero the state image (don't declare any bits) to eliminate the checkbox at this level.
						m_pExplorer -> SetItemState( hCurrentPatientItem, TVIS_BOLD, TVIS_BOLD | TVIS_STATEIMAGEMASK );
						pImageFileSetSpecification -> hTreeHandle = (INT_PTR)hCurrentPatientItem;
						}
					break;
				case DICOM_NODE_STUDY:
					if ( hCurrentPatientItem != 0 )
						hCurrentStudyItem = m_pExplorer -> InsertItem( pImageFileSetSpecification -> NodeInformation, ICON_FOLDER_UNSELECTED, ICON_FOLDER_SELECTED, hCurrentPatientItem );
					else
						hCurrentStudyItem = m_pExplorer -> InsertItem( pImageFileSetSpecification -> NodeInformation, ICON_FOLDER_UNSELECTED, ICON_FOLDER_SELECTED, TVI_ROOT );
					if ( hCurrentPatientItem != 0 )
						m_pExplorer -> Expand( hCurrentPatientItem, TVE_EXPAND );
					if ( hCurrentStudyItem != 0 )
						{
						pImageFileSetSpecification -> hTreeHandle = (INT_PTR)hCurrentStudyItem;
						// Zero the state image (don't declare any bits) to eliminate the checkbox at this level.
						m_pExplorer -> SetItemState( hCurrentStudyItem, 0, TVIS_STATEIMAGEMASK );
						}
					break;
				case DICOM_NODE_SERIES:
					if ( hCurrentStudyItem != 0 )
						hCurrentSeriesItem = m_pExplorer -> InsertItem( pImageFileSetSpecification -> NodeInformation, ICON_FOLDER_UNSELECTED, ICON_FOLDER_SELECTED, hCurrentStudyItem );
					else if ( hCurrentPatientItem != 0 )
						hCurrentSeriesItem = m_pExplorer -> InsertItem( pImageFileSetSpecification -> NodeInformation, ICON_FOLDER_UNSELECTED, ICON_FOLDER_SELECTED, hCurrentPatientItem );
					else
						hCurrentSeriesItem = m_pExplorer -> InsertItem( pImageFileSetSpecification -> NodeInformation, ICON_FOLDER_UNSELECTED, ICON_FOLDER_SELECTED, TVI_ROOT );
					if ( hCurrentStudyItem != 0 )
						m_pExplorer -> Expand( hCurrentStudyItem, TVE_EXPAND );
					if ( hCurrentSeriesItem != 0 )
						{
						pImageFileSetSpecification -> hTreeHandle = (INT_PTR)hCurrentSeriesItem;
						// Zero the state image (don't declare any bits) to eliminate the checkbox at this level.
						m_pExplorer -> SetItemState( hCurrentSeriesItem, 0, TVIS_STATEIMAGEMASK );
						}
					break;
				case DICOM_NODE_IMAGE:
					pFileName = strrchr( pImageFileSetSpecification -> NodeInformation, '\\' );
					pFileName++;
					pExtension = strrchr(pFileName, '.' );
					// Flag any file with a .dcm extension as a dicom image file by attaching a special icon to draw attention to it.
					if ( pExtension != 0 && _stricmp( pExtension, ".dcm" ) == 0 )
						{
						if ( hCurrentSeriesItem != 0 )
							hTreeItem = m_pExplorer -> InsertItem( pFileName, ICON_IMAGE_UNSELECTED, ICON_IMAGE_SELECTED, hCurrentSeriesItem );
						else if ( hCurrentStudyItem != 0 )
							hTreeItem = m_pExplorer -> InsertItem( pFileName, ICON_IMAGE_UNSELECTED, ICON_IMAGE_SELECTED, hCurrentStudyItem );
						else if ( hCurrentPatientItem != 0 )
							hTreeItem = m_pExplorer -> InsertItem( pFileName, ICON_IMAGE_UNSELECTED, ICON_IMAGE_SELECTED, hCurrentPatientItem );
						else
							hTreeItem = m_pExplorer -> InsertItem( pFileName, ICON_IMAGE_UNSELECTED, ICON_IMAGE_SELECTED, TVI_ROOT );
						}
					else
						{
						// Since this entry isn't a folder, don't show the folder icon, but also don't disable
						// the checkbox.  This new item is linked as a child item under hNewlySelectedItem.
						if ( hCurrentSeriesItem != 0 )
							hTreeItem = m_pExplorer -> InsertItem( pFileName, ICON_OMITTED, ICON_OMITTED, hCurrentSeriesItem );
						else if ( hCurrentStudyItem != 0 )
							hTreeItem = m_pExplorer -> InsertItem( pFileName, ICON_OMITTED, ICON_OMITTED, hCurrentStudyItem );
						else if ( hCurrentPatientItem != 0 )
							hTreeItem = m_pExplorer -> InsertItem( pFileName, ICON_OMITTED, ICON_OMITTED, hCurrentPatientItem );
						else
							hTreeItem = m_pExplorer -> InsertItem( pFileName, ICON_OMITTED, ICON_OMITTED, TVI_ROOT );
						}
					if ( hCurrentSeriesItem != 0 )
						m_pExplorer -> Expand( hCurrentSeriesItem, TVE_EXPAND );
					if ( hTreeItem != 0 )
						pImageFileSetSpecification -> hTreeHandle = (INT_PTR)hTreeItem;
					break;
				} 
			pImageFileSetSpecification = pImageFileSetSpecification -> pNextFileSetStruct;
			}
		if ( hCurrentPatientItem != 0 )
			m_pExplorer -> Expand( hCurrentPatientItem, TVE_EXPAND );
		}
}


void CImportDicomdir::OnBnClickedImportCancel( NMHDR *pNMHDR, LRESULT *pResult )
{
	m_pExplorer -> DeleteAllItems();
	delete m_pExplorer;
	m_pExplorer = 0;
	EraseFileSpecList( &m_pListOfFileSetItems );
	EraseList( &m_ListOfProcessedItemFileNames );
	DestroyWindow();

	*pResult = 0;
}


BOOL CImportDicomdir::OnEraseBkgnd( CDC *pDC )
{
	CBrush		BackgroundBrush( m_BackgroundColor );
	CRect		BackgroundRectangle;
	CBrush		*pOldBrush = pDC -> SelectObject( &BackgroundBrush );

	GetClientRect( BackgroundRectangle );
	pDC -> FillRect( BackgroundRectangle, &BackgroundBrush );
	pDC -> SelectObject( pOldBrush );

	return TRUE;
}

