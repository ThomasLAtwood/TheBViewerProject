// StandardSelector.cpp : Implementation file for the CStandardSelector
//  class, , which implements the reference image selection for the standards
//  installation process.
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
//	*[5] 02/21/2024 by Tom Atwood
//		Fixed code security issues.
//	*[4] 02/05/2024 by Tom Atwood
//		Fixed code security issues.
//	*[3] 07/19/2023 by Tom Atwood
//		Fixed code security issues.
//	*[2] 03/16/2023 by Tom Atwood
//		Fixed code security issues.
//	*[1] 12/21/2022 by Tom Atwood
//		Fixed code security issues.
//
//
#include "stdafx.h"
#include <direct.h>
#include <stdio.h>
#include <errno.h>
#include <afxcmn.h>
#include "BViewer.h"
#include "StandardSelector.h"
#include "Access.h"
#include "CustomizePage.h"


extern CONFIGURATION				BViewerConfiguration;
extern CCustomizePage				*pCustomizePage;

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

static MODULE_INFO		InstallModuleInfo = { MODULE_INSTALL, "Install Module", InitInstallModule, CloseInstallModule };


static ERROR_DICTIONARY_ENTRY	InstallErrorCodes[] =
			{
				{ INSTALL_ERROR_INSUFFICIENT_MEMORY		, "An error occurred allocating a memory block for data storage." },
				{ INSTALL_ERROR_FILE_COPY				, "An error occurred attempting to install the selected image file." },
				{ INSTALL_ERROR_FILE_MOVE				, "An error occurred attempting to stage the installable image file." },
				{ INSTALL_ERROR_DICOMDIR_READ			, "An error occurred attempting to read a DICOMDIR file." },
				{ INSTALL_ERROR_WAITING_FOR_MEDIA		, "The I.L.O. standards disk needs to be mounted." },
				{ 0										, NULL }
			};

static ERROR_DICTIONARY_MODULE		InstallStatusErrorDictionary =
										{
										MODULE_INSTALL,
										InstallErrorCodes,
										INSTALL_ERROR_DICT_LENGTH,
										0
										};

extern BOOL					bLoadingStandards;

static char					*pTechSupportMessage = "Request technical support.";
static unsigned long		nNumberOfFilesVisited;
static unsigned long		nNumberOfDICOMDIRFilesFound;
static char					*pRefineSelectionMessage = "Try selecting fewer files to search.";
static char					*pReviseSelectionMessage = "Try installing available .dcm image files.";

static char					Step1StatusMessage[ MAX_LOGGING_STRING_LENGTH ];
static char					Step2StatusMessage[ MAX_LOGGING_STRING_LENGTH ];
static char					Step3StatusMessage[ MAX_LOGGING_STRING_LENGTH ];
static char					Step4StatusMessage[ MAX_LOGGING_STRING_LENGTH ];
static char					*pStandardListOffset = 0;




// This function must be called before any other function in this module.
void InitInstallModule()
{
	bLoadingStandards = FALSE;
	LinkModuleToList( &InstallModuleInfo );
	RegisterErrorDictionary( &InstallStatusErrorDictionary );
	strncpy_s( Step1StatusMessage, MAX_LOGGING_STRING_LENGTH, "Status:  Waiting for media insertion.", _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.
	strncpy_s( Step2StatusMessage, MAX_LOGGING_STRING_LENGTH, "Status:   0 of 22 files copied from I.L.O. media.", _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
	strncpy_s( Step3StatusMessage, MAX_LOGGING_STRING_LENGTH, "Status:   0 of 22 images extracted from Dicom files.", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
	strncpy_s( Step4StatusMessage, MAX_LOGGING_STRING_LENGTH, "Status:   0 of 22 standard images ready for viewing:\n", _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
	pStandardListOffset = &Step4StatusMessage[ strlen( Step4StatusMessage ) ];
}


void CloseInstallModule()
{
}


// CStandardSelector
CStandardSelector::CStandardSelector( int DialogWidth, int DialogHeight, COLORREF BackgroundColor, DWORD WindowStyle ):
				m_StaticUserMessage( "I.L.O. Reference Standard Image Installation", 640, 40, 16, 8, 6,
										COLOR_WHITE, COLOR_STANDARD, COLOR_STANDARD,
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_MESSAGE_FOR_USER ),
				m_ButtonInstallFromSelectedDrive( "Install I.L.O.\nReference Standards", 200, 50, 14, 7, 6,
									COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_MULTILINE |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_INSTALL_FROM_DRIVE ),

				m_StaticStep1( "Step 1:", 60, 40, 16, 8, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_STANDARD_STEP1 ),
				m_StaticStep1Text( "Insert I.L.O. media containing the\nreference standard Dicom images.", 350, 40, 14, 7, 6,
										COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE |
										CONTROL_CLIP | CONTROL_VISIBLE, IDC_STATIC_STANDARD_STEP1_TEXT ),
				m_StaticStep1Status( Step1StatusMessage, 390, 30, 14, 7, 6,
										COLOR_WHITE, COLOR_UNTOUCHED, COLOR_COMPLETED,
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED |
										CONTROL_CLIP | CONTROL_VISIBLE, IDC_STATIC_STANDARD_STEP1_STATUS ),
				m_StaticStep2( "Step 2:", 60, 40, 16, 8, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_STANDARD_STEP2 ),
				m_StaticStep2Text( "Standard reference Dicom image files\nare copied from I.L.O. media.", 350, 40, 14, 7, 6,
										COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE |
										CONTROL_CLIP | CONTROL_VISIBLE, IDC_STATIC_STANDARD_STEP2_TEXT ),
				m_StaticStep2Status( Step2StatusMessage, 390, 30, 14, 7, 6,
										COLOR_WHITE, COLOR_UNTOUCHED, COLOR_COMPLETED,
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED |
										CONTROL_CLIP | CONTROL_VISIBLE, IDC_STATIC_STANDARD_STEP2_STATUS ),
				m_StaticStep3( "Step 3:", 60, 40, 16, 8, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_STANDARD_STEP3 ),
				m_StaticStep3Text( "Standard reference images are\nextracted from Dicom image files.", 350, 40, 14, 7, 6,
										COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE |
										CONTROL_CLIP | CONTROL_VISIBLE, IDC_STATIC_STANDARD_STEP3_TEXT ),
				m_StaticStep3Status( Step3StatusMessage, 390, 30, 14, 7, 6,
										COLOR_WHITE, COLOR_UNTOUCHED, COLOR_COMPLETED,
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED |
										CONTROL_CLIP | CONTROL_VISIBLE, IDC_STATIC_STANDARD_STEP3_STATUS ),
				m_StaticStep4( "Step 4:", 60, 40, 16, 8, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_STANDARD_STEP4 ),
				m_StaticStep4Text( "Standard reference images are\nprepared and stored for use.", 350, 40, 14, 7, 6,
										COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE |
										CONTROL_CLIP | CONTROL_VISIBLE, IDC_STATIC_STANDARD_STEP4_TEXT ),
				m_StaticStep4Status( Step4StatusMessage, 390, 60, 14, 7, 6,
										COLOR_WHITE, COLOR_UNTOUCHED, COLOR_COMPLETED,
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE |
										CONTROL_CLIP | CONTROL_VISIBLE, IDC_STATIC_STANDARD_STEP4_STATUS ),


				m_ButtonInstallSelectedStandard( "Install Standard", 150, 30, 14, 7, 6,
									COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_INSTALL_SELECTION ),
				m_ButtonAutoInstall( "Show\nReference Images\nin Selected\nDrive or Folder", 150, 80, 14, 7, 6,
									COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_MULTILINE |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_AUTO_INSTALL ),
				m_ButtonInstall( "Install Selected\nFile or All\n.dcm Files in\nSelected Folder", 150, 80, 14, 7, 6,
									COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_MULTILINE |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_INSTALL ),
				m_ButtonInstallCancel( "Cancel", 150, 30, 14, 7, 6,
									COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_INSTALL_CANCEL )
{
	m_DialogWidth = DialogWidth;
	m_DialogHeight = DialogHeight;
	m_BackgroundColor = BackgroundColor;
	m_WindowStyle = WindowStyle;
	m_pExplorer = new CTreeCtrl;
	m_pListOfFileSetItems = 0;
	m_TotalImageFilesInstalled = 0;
	m_SelectedDriveSpec[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
	m_bStandardsVolumeFound = FALSE;
	m_nFilesCopiedFromMedia = 0L;
	m_nFilesEncodedAsPNG = 0L;
	m_nFilesReadyForUse = 0L;
}

CStandardSelector::~CStandardSelector()
{
	if ( m_pExplorer != 0 )
		delete m_pExplorer;
	DestroyWindow();
}


BEGIN_MESSAGE_MAP( CStandardSelector, CWnd )
	//{{AFX_MSG_MAP(CStandardSelector)
	ON_WM_CREATE()
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_INSTALL_FROM_DRIVE, OnBnClickedInstallFromSelectedDrive )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_INSTALL_CANCEL, OnBnClickedInstallCancel )
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



// Caution:		Since this function creates the object, it must be called before any
//				functions such as Invalidate(), etc., that manipulate an active window.
BOOL CStandardSelector::SetPosition( int x, int y, CWnd *pParentWnd, CString WindowClass )
{
	BOOL			bResult;
	CRect			DialogRect;
	DWORD			WindowsStyle;

	WindowsStyle = DS_MODALFRAME | WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_EX_TOPMOST;
	DialogRect.SetRect( x, y, x + m_DialogWidth, y + m_DialogHeight );
	bResult = CreateEx( WS_EX_DLGMODALFRAME, (const char*)WindowClass, "Install the I.L.O. Reference Images", WindowsStyle, DialogRect, pParentWnd, 0, NULL );
	
	return bResult;
}


int CStandardSelector::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	BOOL			bOK;						// *[2] Added image list creation result.

	if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
		return -1;

	if ( BViewerConfiguration.bUseDigitalStandards )
		m_nStandards = 23;
	else
		m_nStandards = 22;

	m_SelectedDriveSpec[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
	m_SelectedFileSpec[ 0 ] = '\0';				// *[1] Eliminated call to strcpy.
	m_bSelectionIsAFolder = FALSE;
	m_bSelectionIsADICOMDIR = FALSE;
	m_Row1YOffset = 10;
	m_Column1XOffset = 30;
	m_Column2XOffset = 700 - 180;
	m_StaticUserMessage.SetPosition( m_Column1XOffset, m_Row1YOffset, this );
	m_StaticStep1.SetPosition( m_Column2XOffset, m_DialogHeight - 460, this );
	m_StaticStep1Text.SetPosition( m_Column2XOffset + 70, m_DialogHeight - 460, this );
	m_StaticStep1Status.SetPosition( m_Column2XOffset + 70, m_DialogHeight - 410, this );
	m_StaticStep2.SetPosition( m_Column2XOffset, m_DialogHeight - 360, this );
	m_StaticStep2Text.SetPosition( m_Column2XOffset + 70, m_DialogHeight - 360, this );
	m_StaticStep2Status.SetPosition( m_Column2XOffset + 70, m_DialogHeight - 310, this );
	m_StaticStep3.SetPosition( m_Column2XOffset, m_DialogHeight - 260, this );
	m_StaticStep3Text.SetPosition( m_Column2XOffset + 70, m_DialogHeight - 260, this );
	m_StaticStep3Status.SetPosition( m_Column2XOffset + 70, m_DialogHeight - 210, this );
	m_StaticStep4.SetPosition( m_Column2XOffset, m_DialogHeight - 160, this );
	m_StaticStep4Text.SetPosition( m_Column2XOffset + 70, m_DialogHeight - 160, this );
	m_StaticStep4Status.SetPosition( m_Column2XOffset + 70, m_DialogHeight - 110, this );

	m_ButtonInstallFromSelectedDrive.SetPosition( m_Column2XOffset, m_Row1YOffset + 50, this );
	m_ButtonInstallCancel.SetPosition( m_DialogWidth - 180, m_Row1YOffset + 50, this );
	
	// TVS_CHECKBOXES: 
	// Enables check boxes for items in a tree-view control. A check box is displayed only if an image is associated with the item. When set to this style, the control
	//effectively uses DrawFrameControl to create and set a state image list containing two images. State image 1 is the unchecked box and state image 2 is the checked
	// box. Setting the state image to zero removes the check box altogether. 
	m_pExplorer -> Create( WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | TVS_HASLINES | TVS_TRACKSELECT |  TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP,
							CRect( m_Column1XOffset, m_Row1YOffset + 50, m_Column2XOffset - 30, m_DialogHeight - 80 + 30 ), this, IDC_TREE_CTRL_EXPLORER );
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
	ListStorageDevices();
	
	return 0;
}


void CStandardSelector::CheckWindowsMessages()
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


// List the available storage devices in the Tree Control.
void CStandardSelector::ListStorageDevices()
{
	char			StorageDeviceSpecification[ FILE_PATH_STRING_LENGTH ];
	unsigned long	StorageDeviceMask;
	char			VolumeLabel[ FILE_PATH_STRING_LENGTH ];
	char			VolumeName[ FILE_PATH_STRING_LENGTH ];
	char			*pChar;
	HTREEITEM		hRootItem;

	m_pExplorer -> DeleteAllItems();
	strncpy_s( StorageDeviceSpecification, FILE_PATH_STRING_LENGTH, "A:", _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.
	StorageDeviceMask = _getdrives();
	while ( StorageDeviceMask != 0 )
		{
		m_bStandardsVolumeFound = FALSE;
		if ( StorageDeviceMask & 1 )
			{
			GetDriveLabel( StorageDeviceSpecification, VolumeLabel );
			strncpy_s( VolumeName, FILE_PATH_STRING_LENGTH, VolumeLabel, _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.
			pChar = strchr( VolumeName, '(' );
			if ( pChar != 0 )
				*pChar = '\0';
			TrimBlanks( VolumeName );
			if ( strcmp( VolumeName, "2011-D" ) == 0 )
				{
				m_bStandardsVolumeFound = TRUE;
				strncpy_s( VolumeName, FILE_PATH_STRING_LENGTH, "ILO Reference Standards (", _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
				pChar = strchr( VolumeLabel, '(' );
				if ( pChar != 0 )
					{
					pChar++;
					strncat_s( VolumeName, FILE_PATH_STRING_LENGTH, pChar, _TRUNCATE );						// *[3] Replaced strcat with strncat_s.
					strncpy_s( m_SelectedDriveSpec, FULL_FILE_SPEC_STRING_LENGTH, pChar, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
					pChar = strchr( m_SelectedDriveSpec, ':' );
					if ( pChar != 0 )
						{
						pChar++;
						*pChar = '\0';
						}
					}
				// By default, all items display the first image in the image list for both the selected and nonselected states
				// You can change the default behavior for a particular item by specifying the indexes of the selected and
				// nonselected images when adding the item to the tree control using the InsertItem member function. You can
				// change the indexes after adding an item by using the SetItemImage member function. 
				hRootItem = m_pExplorer -> InsertItem( VolumeName, ICON_DRIVE_UNSELECTED, ICON_DRIVE_SELECTED, TVI_ROOT );
				if ( hRootItem != NULL )
					// Zero the state image (don't declare any bits) to eliminate the checkbox at this level.
					m_pExplorer -> SetItemState( hRootItem, TVIS_BOLD | TVIS_SELECTED, TVIS_BOLD | TVIS_SELECTED );
				}
			else
				{
				hRootItem = m_pExplorer -> InsertItem( VolumeLabel, ICON_DRIVE_UNSELECTED, ICON_DRIVE_SELECTED, TVI_ROOT );
				if ( hRootItem != NULL )
					m_pExplorer -> SetItemState( hRootItem, TVIS_BOLD, TVIS_BOLD );
				}
			}
		StorageDeviceSpecification[ 0 ]++;
		StorageDeviceMask >>= 1;
		}
	if ( strlen( m_SelectedDriveSpec ) != 0 )
		{
		m_StaticStep1Status.m_ControlText = "Status:  I.L.O. Files Have Been Located.";
		m_StaticStep1Status.HasBeenCompleted( TRUE );
		m_StaticStep1Status.Invalidate();
		m_StaticStep1Status.UpdateWindow();
		m_pExplorer -> ModifyStyle( 0, WS_DISABLED, 0);
		}
	m_pExplorer -> Invalidate();
	m_pExplorer -> UpdateWindow();

}


void CStandardSelector::InstallFromDICOMDIRFile()
{
	BOOL							bNoError = TRUE;
	char							SearchDirectory[ FULL_FILE_SPEC_STRING_LENGTH ];
	char							Msg[ MAX_EXTRA_LONG_STRING_LENGTH ];
	CWaitCursor						DisplaysHourglass;

	m_nFilesEncodedAsPNG = 0L;
	EraseFileSpecList( &m_pListOfFileSetItems );
	// Clear the Dicom file folders that the Standards import will be using.
	if ( pCustomizePage != 0 && pCustomizePage -> GetSafeHwnd() != 0 )
		{
		strncpy_s( SearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.InboxDirectory, _TRUNCATE );							// *[1] Replaced strcpy with strncpy_s.
		if ( SearchDirectory[ strlen( SearchDirectory ) - 1 ] != '\\' )
			strncat_s( SearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );													// *[3] Replaced strcat with strncat_s.
		pCustomizePage -> DeleteFolderContents( SearchDirectory, 0 );

		strncpy_s( SearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.WatchDirectory, _TRUNCATE );							// *[1] Replaced strcpy with strncpy_s.
		if ( SearchDirectory[ strlen( SearchDirectory ) - 1 ] != '\\' )
			strncat_s( SearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );													// *[3] Replaced strcat with strncat_s.
		pCustomizePage -> DeleteFolderContents( SearchDirectory, 0 );

		strncpy_s( SearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.BRetrieverDataDirectory, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
		if ( SearchDirectory[ strlen( SearchDirectory ) - 1 ] != '\\' )
			strncat_s( SearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );													// *[3] Replaced strcat with strncat_s.
		strncat_s( SearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, "Queued Files\\", _TRUNCATE );											// *[3] Replaced strcat with strncat_s.
		pCustomizePage -> DeleteFolderContents( SearchDirectory, 0 );
		}
	bNoError = ReadDicomDirectoryFile( m_SelectedFileSpec );
	if ( !bNoError )
		{
		strncpy_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "An error occurred interpreting\nthe Dicom file set information from", _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
		strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, m_SelectedFileSpec, _TRUNCATE );														// *[3] Replaced strcat with strncat_s.
		strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "\nfor install.", _TRUNCATE );														// *[3] Replaced strcat with strncat_s.
		ThisBViewerApp.NotifyUserOfInstallSearchStatus( INSTALL_ERROR_DICOMDIR_READ, Msg, pTechSupportMessage );
		}
	if ( bNoError )
		{
		// All the imported Dicom standard image files have been copied to the BRetriever
		// watch folder.  BRetriever converts them to PNG files and deposits them into
		// the .\images folder.  As they show up, they can be processed:
		while ( bNoError && bLoadingStandards && m_nFilesEncodedAsPNG < 22 )
			{
			ThisBViewerApp.ReadNewAbstractData();
			// Count the number of standard images available so far.
			bNoError = LoadReferenceStandards( TRUE);
			Sleep( 3000 );	// Wait 3 seconds for BRetriever to provide more PNG files.
			}
		}
	if ( bNoError )
		{
		// All the imported Dicom standard image files have been copied to the BRetriever
		// watch folder.  BRetriever converts them to PNG files and deposits them into
		// the .\images folder.  As they show up, they can be processed:
		while ( bNoError && bLoadingStandards )
			{
			ThisBViewerApp.ReadNewAbstractData();
			bNoError = LoadReferenceStandards( FALSE );
			_snprintf_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, _TRUNCATE, "Called LoadReferenceStandards() counting %d files ready for use (bLoadingStandards = %d).",	// *[2] Replaced sprintf() with _snprintf_s.
							m_nFilesReadyForUse, bLoadingStandards );
			LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
			}
		}

	if ( bNoError )
		{
		OnExitStandardSelector();
		m_pExplorer -> DeleteAllItems();
		delete m_pExplorer;
		m_pExplorer = 0;
		EraseFileSpecList( &m_pListOfFileSetItems );
		DestroyWindow();
		}
}


// Import a designated file into the BRetriever watch folder, from which
// it will be picked up and processed.
BOOL CStandardSelector::CopyDesignatedFile( char *pSourceImageFileSpec )
{
	char					FullInputImageFileSpec[ FILE_PATH_STRING_LENGTH ];
	char					FullOutputImageFileSpec[ FILE_PATH_STRING_LENGTH ];
	char					Msg[ MAX_EXTRA_LONG_STRING_LENGTH ];
	char					*pChar;
	BOOL					bNoError = TRUE;
	int						RenameResult;

	if ( strlen( pSourceImageFileSpec ) > 0 )
		{
		strncpy_s( FullInputImageFileSpec, FILE_PATH_STRING_LENGTH, pSourceImageFileSpec, _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.
		strncpy_s( FullOutputImageFileSpec, FILE_PATH_STRING_LENGTH, BViewerConfiguration.InboxDirectory, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
		if ( FullOutputImageFileSpec[ strlen( FullOutputImageFileSpec ) - 1 ] != '\\' )
			strncat_s( FullOutputImageFileSpec, FILE_PATH_STRING_LENGTH, "\\", _TRUNCATE );									// *[2] Replaced strcat with strncat_s.
		pChar = strrchr( pSourceImageFileSpec, '\\' );
		pChar++;
		strncat_s( FullOutputImageFileSpec, FILE_PATH_STRING_LENGTH, pChar, _TRUNCATE );									// *[2] Replaced strncat with strncat_s.
		bNoError = CopyFile( FullInputImageFileSpec, FullOutputImageFileSpec, FALSE );
		if ( bNoError )
			{
			bNoError = MakeFileWriteable( FullOutputImageFileSpec );										// *[4] Intrroduced this function to avoid a race condition.

			// Rename it over to the Watch Folder, where BRetriever will pick it up and process it.
			strncpy_s( FullInputImageFileSpec, FILE_PATH_STRING_LENGTH, FullOutputImageFileSpec, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
			strncpy_s( FullOutputImageFileSpec, FILE_PATH_STRING_LENGTH, BViewerConfiguration.WatchDirectory, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
			if ( FullOutputImageFileSpec[ strlen( FullOutputImageFileSpec ) - 1 ] != '\\' )
				strncat_s( FullOutputImageFileSpec, FILE_PATH_STRING_LENGTH, "\\", _TRUNCATE );								// *[2] Replaced strcat with strncat_s.
			strncat_s( FullOutputImageFileSpec, FILE_PATH_STRING_LENGTH, pChar, _TRUNCATE );								// *[2] Replaced strncat with strncat_s.
			RenameResult = rename( FullInputImageFileSpec, FullOutputImageFileSpec );
			if ( RenameResult != 0 )
				{
				// If there is a file of the same name in the watch folder, remove it and try again.
				RenameResult = remove( FullOutputImageFileSpec );
				if ( RenameResult == 0 )
					RenameResult = rename( FullInputImageFileSpec, FullOutputImageFileSpec );
				}
			if ( RenameResult != 0 )
				{
				bNoError = FALSE;
				strncpy_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "Unable to install\n", _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
				strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, FullInputImageFileSpec, _TRUNCATE );	// *[3] Replaced strcat with strncat_s.
				ThisBViewerApp.NotifyUserOfImportSearchStatus(	INSTALL_ERROR_FILE_MOVE, Msg, pTechSupportMessage );
				}
			}
		else
			{
			strncpy_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "Unable to stage\n", _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
			strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, FullInputImageFileSpec, _TRUNCATE );		// *[1] Replaced strcat with strncat_s.
			strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "\nfor import.", _TRUNCATE );				// *[1] Replaced strcat with strncat_s.
			ThisBViewerApp.NotifyUserOfImportSearchStatus( INSTALL_ERROR_FILE_COPY, Msg, pTechSupportMessage );
			}
		}
	if ( bNoError )
		m_TotalImageFilesInstalled++;

	return bNoError;
}


void CStandardSelector::OnBnClickedInstallFromSelectedDrive( NMHDR *pNMHDR, LRESULT *pResult )
{
	char							Msg[ MAX_EXTRA_LONG_STRING_LENGTH ];
	char							Suggestion[ MAX_EXTRA_LONG_STRING_LENGTH ];

	if ( strlen( m_SelectedDriveSpec ) != 0 )
		{
		m_StaticStep1Status.m_ControlText = "Status:  I.L.O. Files Have Been Located.";
		m_StaticStep1Status.HasBeenCompleted( TRUE );
		m_StaticStep1Status.Invalidate();
		m_StaticStep1Status.UpdateWindow();
		}

	m_nFilesCopiedFromMedia = 0L;
	sprintf_s( Step2StatusMessage, MAX_LOGGING_STRING_LENGTH, "Status:   0 of %2d files copied from I.L.O. media.", m_nStandards );
	m_StaticStep2Status.HasBeenCompleted( FALSE );

	m_nFilesEncodedAsPNG = 0L;
	sprintf_s( Step3StatusMessage, MAX_LOGGING_STRING_LENGTH, "Status:  0 of %2d images extracted from Dicom files.", m_nStandards );
	m_StaticStep3Status.HasBeenCompleted( FALSE );

	m_nFilesReadyForUse = 0L;
	sprintf_s( Step4StatusMessage, MAX_LOGGING_STRING_LENGTH, "Status:   0 of %2d standard images ready for viewing:\n", m_nStandards );
	UpdateStandardStatusDisplay( pStandardListOffset );
	m_StaticStep4Status.HasBeenCompleted( FALSE );

	ListStorageDevices();
	while ( strlen( m_SelectedDriveSpec ) == 0 )
		{
		strncpy_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "Please insert the I.L.O.\n", _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.
		strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "reference standards media.\n", _TRUNCATE );						// *[3] Replaced strcat with strncat_s.
		strncpy_s( Suggestion, MAX_EXTRA_LONG_STRING_LENGTH, "Wait for the I.L.O. viewer to appear,\n", _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
		strncat_s( Suggestion, MAX_EXTRA_LONG_STRING_LENGTH, "then click the X to exit out of it.\n", _TRUNCATE );		// *[3] Replaced strcat with strncat_s.
		ThisBViewerApp.NotifyUserOfImportSearchStatus( INSTALL_ERROR_WAITING_FOR_MEDIA, Msg, Suggestion );
		ListStorageDevices();
		}

	while ( strlen( m_SelectedDriveSpec ) == 0 )
		{
		Sleep( 1000 );
		ListStorageDevices();
		}

	if ( strlen( m_SelectedDriveSpec ) != 0 )
		{
		m_StaticStep1Status.m_ControlText = "Status:  I.L.O. Files Have Been Located.";
		m_StaticStep1Status.HasBeenCompleted( TRUE );
		m_StaticStep1Status.Invalidate();
		m_StaticStep1Status.UpdateWindow();

		bLoadingStandards = TRUE;

		// Preset all the bStandardSuccessfullyStored flags to FALSE;
		InitializeStandardFileImport();
		strncpy_s( m_SelectedFileSpec, FULL_FILE_SPEC_STRING_LENGTH, m_SelectedDriveSpec, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
		strncat_s( m_SelectedFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );					// *[3] Replaced strcat with strncat_s.
		strncat_s( m_SelectedFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "DICOMDIR", _TRUNCATE );			// *[3] Replaced strcat with strncat_s.
		InstallFromDICOMDIRFile();
		}

	*pResult = 0;
}



// Import all .dcm files in the specified directory, including subdirectories.
BOOL CStandardSelector::CopyDirectoryContents( char *pSourceDirectorySpec )
{
	char				SearchPath[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				LowerLevelDirectory[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				FullSourceFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	WIN32_FIND_DATA		FindFileInfo;
	HANDLE				hFindFile;
	BOOL				bFileFound;
	char				*pExtension;
	BOOL				bNoError = TRUE;

	strncpy_s( SearchPath, FULL_FILE_SPEC_STRING_LENGTH, pSourceDirectorySpec, _TRUNCATE );							// *[1] Replaced strcpy with strncpy_s.
	if ( SearchPath[ strlen( SearchPath ) - 1 ] != '\\' )
		strncat_s( SearchPath, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );										// *[1] Replaced strcat with strncat_s.
	strncat_s( SearchPath, FULL_FILE_SPEC_STRING_LENGTH, "*.*", _TRUNCATE );										// *[1] Replaced strcat with strncat_s.
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
			if ( pExtension != 0 && _stricmp( pExtension, ".dcm" ) == 0 )
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


void CStandardSelector::EraseFileSpecList( IMAGE_FILE_SET_SPECIFICATION **ppFileSpecList )
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


BOOL CStandardSelector::ReadDicomDirectoryFile( char *pDicomdirFileSpec )
{
	BOOL							bNoError = TRUE;
	BOOL							bAllFilesCopied;
	DICOM_HEADER_SUMMARY			*pDicomHeader;
	IMAGE_FILE_SET_SPECIFICATION	*pImageFileSetSpecification;
	char							FullSourceFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char							*pChar;
	char							Msg[ MAX_EXTRA_LONG_STRING_LENGTH ];

	bAllFilesCopied = FALSE;
	m_nFilesCopiedFromMedia = 0L;
	m_StaticStep2Status.HasBeenCompleted( FALSE );
	pDicomHeader = (DICOM_HEADER_SUMMARY*)malloc( sizeof(DICOM_HEADER_SUMMARY) );
	if ( pDicomHeader == 0 )
		{
		bNoError = FALSE;
		RespondToError( MODULE_INSTALL, INSTALL_ERROR_INSUFFICIENT_MEMORY );
		}
	else
		{
		// Decode and parse the Dicom information from the file.  The Dicom file contents
		// are retained in a series of memory buffers in the list, pDicomHeader -> ListOfInputBuffers.
		bNoError = ReadDicomHeaderInfo( pDicomdirFileSpec, &pDicomHeader, TRUE );

		if ( bNoError )
			{
			// Set up the absolute file specifications.
			pImageFileSetSpecification = pDicomHeader -> ListOfImageFileSetSpecifications;
			while ( pImageFileSetSpecification != 0 )
				{
				strncpy_s( pImageFileSetSpecification -> DICOMDIRFileSpec, FULL_FILE_SPEC_STRING_LENGTH, pDicomdirFileSpec, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
				if ( pImageFileSetSpecification -> DicomNodeType == DICOM_NODE_IMAGE )
					{
					strncpy_s( FullSourceFileSpec, FULL_FILE_SPEC_STRING_LENGTH, pDicomdirFileSpec, _TRUNCATE );								// *[1] Replaced strcpy with strncpy_s.
					pChar = strrchr( FullSourceFileSpec, '\\' );
					if ( pChar != 0 )
						{
						pChar++;
						strncpy_s( pChar, FULL_FILE_SPEC_STRING_LENGTH - (INT_PTR)( pChar - FullSourceFileSpec ),
												pImageFileSetSpecification -> NodeInformation, _TRUNCATE );										// *[1] Replaced strcpy with strncpy_s.
						}
					bNoError = CopyDesignatedFile( FullSourceFileSpec );
					m_nFilesCopiedFromMedia++;
					sprintf_s( Step2StatusMessage, MAX_LOGGING_STRING_LENGTH, "Status:  %d of %2d files copied from I.L.O. media.", m_nFilesCopiedFromMedia, m_nStandards );
					if ( m_nFilesCopiedFromMedia == m_nStandards )
						{
						m_StaticStep2Status.HasBeenCompleted( TRUE );
						bAllFilesCopied = TRUE;
						}
					CheckWindowsMessages();
					m_StaticStep2Status.Invalidate();
					m_StaticStep2Status.UpdateWindow();
					UpdateWindow();
					}
				pImageFileSetSpecification = pImageFileSetSpecification -> pNextFileSetStruct;
				}
			}
		else
			{
			strncpy_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "An error occurred interpreting\nthe Dicom file set information from", _TRUNCATE );		// *[1] Replace strcpy with strncpy_s.
			strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, pDicomdirFileSpec, _TRUNCATE );		// *[1] Replace strcat with strncat_s.
			strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "\nfor install.", _TRUNCATE );		// *[1] Replace strcat with strncat_s.
			ThisBViewerApp.NotifyUserOfInstallSearchStatus( INSTALL_ERROR_DICOMDIR_READ, Msg, pTechSupportMessage );
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

	return bNoError && bAllFilesCopied;
}


BOOL CStandardSelector::SearchForDICOMDIRFiles( char *pSourceDirectorySpec )
{
	char				SearchPath[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				LowerLevelDirectory[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				FullSourceFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	WIN32_FIND_DATA		FindFileInfo;
	HANDLE				hFindFile;
	BOOL				bFileFound;
	BOOL				bNoError = TRUE;
	char				Msg[ MAX_EXTRA_LONG_STRING_LENGTH ];
	CWaitCursor			DisplaysHourglass;

	strncpy_s( SearchPath, FULL_FILE_SPEC_STRING_LENGTH, pSourceDirectorySpec, _TRUNCATE );									// *[1] Replaced strcpy with strncpy_s.
	if ( SearchPath[ strlen( SearchPath ) - 1 ] != '\\' )
		strncat_s( SearchPath, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );												// *[1] Replaced strcat with strncat_s.
	strncat_s( SearchPath, FULL_FILE_SPEC_STRING_LENGTH, "*.*", _TRUNCATE );												// *[1] Replaced strcat with strncat_s.
	hFindFile = FindFirstFile( SearchPath, &FindFileInfo );
	bFileFound = ( hFindFile != INVALID_HANDLE_VALUE );
	while ( bNoError && bFileFound )
		{
		nNumberOfFilesVisited++;
		if ( nNumberOfFilesVisited >= 10000 )
			{
			bNoError = FALSE;
			strncpy_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "Your requested search\nencountered over 10,000 files.", _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
			ThisBViewerApp.NotifyUserOfInstallSearchStatus( INSTALL_ERROR_DICOMDIR_READ, Msg, pRefineSelectionMessage );
			nNumberOfFilesVisited = 0L;
			}
		if ( bNoError )
			{
			if ( ( FindFileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
				{
				if ( strcmp( FindFileInfo.cFileName, "." ) != 0 && strcmp( FindFileInfo.cFileName, ".." ) != 0 )
					{
					strncpy_s( LowerLevelDirectory, FULL_FILE_SPEC_STRING_LENGTH, pSourceDirectorySpec, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
					if ( LowerLevelDirectory[ strlen( LowerLevelDirectory ) - 1 ] != '\\' )
						strncat_s( LowerLevelDirectory, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );					// *[1] Replaced strcat with strncat_s.
					strncat_s( LowerLevelDirectory, FULL_FILE_SPEC_STRING_LENGTH, FindFileInfo.cFileName, _TRUNCATE );		// *[1] Replaced strcat with strncat_s.
					bNoError = SearchForDICOMDIRFiles( LowerLevelDirectory );
					}
				}
			else
				{
				if ( _stricmp( FindFileInfo.cFileName, "DICOMDIR" ) == 0 )
					{
					nNumberOfDICOMDIRFilesFound++;
					strncpy_s( FullSourceFileSpec, FULL_FILE_SPEC_STRING_LENGTH, pSourceDirectorySpec, _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
					if ( FullSourceFileSpec[ strlen( FullSourceFileSpec ) - 1 ] != '\\' )
						strncat_s( FullSourceFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );						// *[1] Replaced strcat with strncat_s.
					strncat_s( FullSourceFileSpec, FULL_FILE_SPEC_STRING_LENGTH, FindFileInfo.cFileName, _TRUNCATE );		// *[1] Replaced strcat with strncat_s.
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


BOOL CStandardSelector::LoadReferenceStandards( BOOL bCountOnly )
{
	BOOL					bNoError = TRUE;
	CStudy					*pStudy;
	LIST_ELEMENT			*pStudyListElement;
	DIAGNOSTIC_STUDY		*pDiagnosticStudy = 0;			// *[2] Initialize pointer.
	DIAGNOSTIC_SERIES		*pDiagnosticSeries;
	DIAGNOSTIC_SERIES		*pPrevDiagnosticSeries;
	DIAGNOSTIC_IMAGE		*pDiagnosticImage;
	char					*pImageFileName;
	BOOL					bValidLevel;

	m_nFilesEncodedAsPNG = 0L;
	pStudyListElement = ThisBViewerApp.m_NewlyArrivedStudyList;
	// Read study items until a standard image is found.
	while ( bNoError && pStudyListElement != 0 )
		{
		pStudy = (CStudy*)pStudyListElement -> pItem;
		if ( pStudy != 0 )
			{
			if ( !bCountOnly )
				LogMessage( "Study found in m_NewlyArrivedStudyList.", MESSAGE_TYPE_SUPPLEMENTARY );
			pDiagnosticStudy = pStudy -> m_pDiagnosticStudyList;
			bNoError = ( pDiagnosticStudy != 0 );			// *[5] Added NULL check.
			if ( bNoError )									// *[5]
				{
				bValidLevel = ( strncmp( pDiagnosticStudy -> ReferringPhysiciansName, "ILO/D", 5 ) == 0 );
				if ( bValidLevel )
					{
					pPrevDiagnosticSeries = 0;
					pDiagnosticSeries = pDiagnosticStudy -> pDiagnosticSeriesList;
					bNoError = TRUE;
					while ( bNoError && bValidLevel && pDiagnosticSeries != 0 )
						{
						bValidLevel = ( strncmp( pDiagnosticSeries -> BodyPartExamined, "Chest", 5 ) == 0 );
						if ( bValidLevel )
							{
							pDiagnosticImage = pDiagnosticSeries -> pDiagnosticImageList;
							if ( pDiagnosticImage != 0 )
								{
								pImageFileName = pDiagnosticImage -> SOPInstanceUID;
								if ( pImageFileName != 0 )
									{
									if ( bCountOnly )
										{
										m_nFilesEncodedAsPNG++;
										sprintf_s( Step3StatusMessage, MAX_LOGGING_STRING_LENGTH, "Status:  %d of %2d images extracted from Dicom files.", m_nFilesEncodedAsPNG, m_nStandards );
										if ( m_nFilesEncodedAsPNG >= m_nStandards )
											{
											m_StaticStep3Status.HasBeenCompleted( TRUE );
											LogMessage( "Installation Step 3 has been completed.", MESSAGE_TYPE_SUPPLEMENTARY );
											}
										if ( m_nFilesEncodedAsPNG <= m_nStandards )
											{
											m_StaticStep3Status.Invalidate();
											m_StaticStep3Status.UpdateWindow();
											UpdateWindow();
											CheckWindowsMessages();
											}
										}
									else
										{
										bNoError = ProcessReferenceImageFile( pImageFileName );
										if ( bNoError )
											{
											m_nFilesReadyForUse++;
											_snprintf_s( Step4StatusMessage, MAX_LOGGING_STRING_LENGTH, _TRUNCATE,	// *[2] Replaced sprintf() with _snprintf_s.
															"Status:  %d of %2d standard images ready for viewing:\n", m_nFilesReadyForUse, m_nStandards );
											pStandardListOffset = &Step4StatusMessage[ strlen( Step4StatusMessage ) ];
											UpdateStandardStatusDisplay( pStandardListOffset );
											if ( m_nFilesReadyForUse >= m_nStandards )
												{
												m_StaticStep4Status.HasBeenCompleted( TRUE );
												LogMessage( "Installation Step 4 has been completed.", MESSAGE_TYPE_SUPPLEMENTARY );
												}
											if ( m_nFilesReadyForUse <= m_nStandards )
												{
												m_StaticStep4Status.Invalidate();
												m_StaticStep4Status.UpdateWindow();
												CheckWindowsMessages();
												Sleep( 250 );	// Wait for screen to update.
												}
											}
										}
									}
								}
							if ( bNoError && !bCountOnly )
								{
								// Remove the current series and image from the list.
								if ( pPrevDiagnosticSeries == 0 )
									pDiagnosticStudy -> pDiagnosticSeriesList = pDiagnosticSeries -> pNextDiagnosticSeries;
								else
									pPrevDiagnosticSeries  -> pNextDiagnosticSeries = pDiagnosticSeries -> pNextDiagnosticSeries;

								free( pDiagnosticImage );
								free( pDiagnosticSeries );
							
								// Prepare to advance to the next series in the list.
								pDiagnosticSeries = pPrevDiagnosticSeries;
								}
							}			// ... end if valid series for a standard.
						pPrevDiagnosticSeries = pDiagnosticSeries;
						if ( pDiagnosticSeries != 0 )
							pDiagnosticSeries = pDiagnosticSeries -> pNextDiagnosticSeries;
						}			// ... end while another series in the list.
					}
				}
			}
		if ( bNoError && pStudy != 0 && !bCountOnly )				// *[2] *[5] Added NULL check.
			{
			if ( pDiagnosticStudy -> pDiagnosticSeriesList == 0 )
				{
				LogMessage( "Deleting installed standard study, etc.", MESSAGE_TYPE_SUPPLEMENTARY );
				pStudy -> DeleteStudyDataAndImages();
				RemoveFromList( &ThisBViewerApp.m_NewlyArrivedStudyList, (void*)pStudy );
				delete pStudy;
				ThisBViewerApp.m_pCurrentStudy = 0;
				LogMessage( "    Study, etc., deleted.", MESSAGE_TYPE_SUPPLEMENTARY );
				// Queue up the new first element in the list.
				pStudyListElement = ThisBViewerApp.m_NewlyArrivedStudyList;
				}
			}
		else
			pStudyListElement = pStudyListElement -> pNextListElement;
		}

	return bNoError;
}


void CStandardSelector::OnExitStandardSelector()
{
	static char			Msg[ MAX_EXTRA_LONG_STRING_LENGTH ];

	if ( m_TotalImageFilesInstalled == 1 )
		_snprintf_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, _TRUNCATE, "%d image file\nhas been installed.", m_TotalImageFilesInstalled );	// *[2] Replaced sprintf() with _snprintf_s.
	else
		_snprintf_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, _TRUNCATE, "%d image files\nhave been installed.", m_TotalImageFilesInstalled );	// *[2] Replaced sprintf() with _snprintf_s.
	ThisBViewerApp.MakeAnnouncement( Msg );
}


void CStandardSelector::OnBnClickedInstallCancel( NMHDR *pNMHDR, LRESULT *pResult )
{
	OnExitStandardSelector();
	m_pExplorer -> DeleteAllItems();
	delete m_pExplorer;
	m_pExplorer = 0;
	EraseFileSpecList( &m_pListOfFileSetItems );
	DestroyWindow();

	*pResult = 0;
}


BOOL CStandardSelector::OnEraseBkgnd( CDC *pDC )
{
	CBrush		BackgroundBrush( m_BackgroundColor );
	CRect		BackgroundRectangle;
	CBrush		*pOldBrush = pDC -> SelectObject( &BackgroundBrush );

	GetClientRect( BackgroundRectangle );
	pDC -> FillRect( BackgroundRectangle, &BackgroundBrush );
	pDC -> SelectObject( pOldBrush );

	return TRUE;
}


void CStandardSelector::DeleteChildItems( HTREEITEM hParentItem )
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
void CStandardSelector::ExpandSelection( HTREEITEM hNewlySelectedItem )
{
	HTREEITEM			hParentItem;
	char				FileSpecComponent[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				TempFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				FullFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				FoundFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char				SearchPath[ FULL_FILE_SPEC_STRING_LENGTH ];
	WIN32_FIND_DATA		FindFileInfo;
	HANDLE				hFindFile;
	BOOL				bFileFound;
	DWORD				FileAttributes;
	char				*pChar;
	char				*pExtension;
	
	FullFileSpec[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
	TempFileSpec[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
	hParentItem = hNewlySelectedItem;
	// Working the way up the tree from the selected item, compose the full
	// file specification for the selected item.  (The text for each item
	// consists only of the file or folder name for that item.)
	while ( hParentItem != NULL )
		{
		strncpy_s( FileSpecComponent, FULL_FILE_SPEC_STRING_LENGTH, (const char*)m_pExplorer -> GetItemText( hParentItem ), _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
		if ( strlen( FileSpecComponent ) > 0 )
			{
			hParentItem = m_pExplorer -> GetParentItem( hParentItem );
			if ( hParentItem != NULL )
				{
				strncpy_s( FullFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );													// *[1] Replaced strcpy with strncpy_s.
				strncat_s( FullFileSpec, FULL_FILE_SPEC_STRING_LENGTH, FileSpecComponent, _TRUNCATE );										// *[2] Replaced strcat with strncat_s.
				}
			else
				{
				pChar = strchr( FileSpecComponent, ':' );
				if ( pChar != 0 )
					{
					pChar--;
					strncpy_s( FullFileSpec, FULL_FILE_SPEC_STRING_LENGTH, pChar, 2 );														// *[2] Replaced strncat with strncpy_s.
					}
				}
			strncat_s( FullFileSpec, FULL_FILE_SPEC_STRING_LENGTH, TempFileSpec, _TRUNCATE );												// *[2] Replaced strcat with strncat_s.
			// Save the intermediate result for the next cycle.
			strncpy_s( TempFileSpec, FULL_FILE_SPEC_STRING_LENGTH, FullFileSpec, _TRUNCATE );												// *[1] Replaced strcpy with strncpy_s.
			}
		else
			hParentItem = NULL;
		}
	strncpy_s( m_SelectedFileSpec, FULL_FILE_SPEC_STRING_LENGTH, FullFileSpec, _TRUNCATE );													// *[1] Replaced strcpy with strncpy_s.
	FileAttributes = GetFileAttributes( m_SelectedFileSpec );
	m_bSelectionIsAFolder = ( ( FileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 );
	m_bSelectionIsADICOMDIR = FALSE;
	if ( !m_bSelectionIsAFolder )
		m_bSelectionIsADICOMDIR = ( _stricmp( "DICOMDIR", (const char*)m_pExplorer -> GetItemText( hNewlySelectedItem ) ) == 0 );

	// Perform the expansion of the selected item by searching through the selected
	// folder, assuming the selection was a folder.
	DeleteChildItems( hNewlySelectedItem );
	// Locate the first file or directory member in the current search directory.
	strncpy_s( SearchPath, FULL_FILE_SPEC_STRING_LENGTH, FullFileSpec, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
	strncat_s( SearchPath, FULL_FILE_SPEC_STRING_LENGTH, "\\*.*", _TRUNCATE );		// *[3] Replaced strcat with strncat_s.

	// First search for all the folders so they will be listed first.
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
			strncat_s( FoundFileSpec, FULL_FILE_SPEC_STRING_LENGTH, FindFileInfo.cFileName, _TRUNCATE );	// *[3] Replaced strcat with strncat_s.
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
				m_pExplorer -> InsertItem( FindFileInfo.cFileName, ICON_FOLDER_UNSELECTED, ICON_FOLDER_SELECTED, hNewlySelectedItem );
				}
			}
		// Look for another file in the source directory.
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
			strncat_s( FoundFileSpec, FULL_FILE_SPEC_STRING_LENGTH, FindFileInfo.cFileName, _TRUNCATE );	// *[3] Replaced strcat with strncat_s.
			// A check box is displayed only if an image is associated with the item. The control effectively
			// uses DrawFrameControl to create and set a state image list containing two images. State image 1 is the unchecked box
			// and state image 2 is the checked box. Setting the state image to zero removes the check box altogether. 
			if ( ( FindFileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
				{
				// By default, all items display the first image in the image list for both the selected and nonselected states
				// You can change the default behavior for a particular item by specifying the indexes of the selected and
				// nonselected images when adding the item to the tree control using the InsertItem member function. You can
				// change the indexes after adding an item by using the SetItemImage member function. 
				pExtension = strrchr( FindFileInfo.cFileName, '.' );
				// Flag any file with a .dcm extension as a dicom image file by attaching a special icon to draw attention to it.
				if ( pExtension != 0 && _stricmp( pExtension, ".dcm" ) == 0 )
					m_pExplorer -> InsertItem( FindFileInfo.cFileName, ICON_IMAGE_UNSELECTED, ICON_IMAGE_SELECTED, hNewlySelectedItem );
				else
					{
					// Since this entry isn't a folder, don't show the folder icon, but also don't disable
					// the checkbox.  This new item is linked as a child item under hNewlySelectedItem.
					m_pExplorer -> InsertItem( FindFileInfo.cFileName, ICON_OMITTED, ICON_OMITTED, hNewlySelectedItem );
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


BOOL CStandardSelector::OnNotify( WPARAM wParam, LPARAM lParam, LRESULT *pResult )
{
	NMTREEVIEW			*pTreeViewNotice;
	TVITEM				PreviouslySelectedItem;
	TVITEM				NewlySelectedItem;
	
	pTreeViewNotice = (LPNMTREEVIEW)lParam;
	if ( pTreeViewNotice != 0 && pTreeViewNotice -> hdr.code == TVN_SELCHANGED )
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

	return CWnd::OnNotify( wParam, lParam, pResult );
}


