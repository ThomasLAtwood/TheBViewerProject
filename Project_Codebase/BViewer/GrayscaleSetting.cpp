// GrayscaleSetting.cpp - Implements the data structures and functions for the grayscale
// settings associated with each image.
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
//	*[3] 07/17/2023 by Tom Atwood
//		Fixed code security issues.
//	*[2] 03/14/2023 by Tom Atwood
//		Fixed code security issues.
//	*[1] 01/19/2023 by Tom Atwood
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
#include "GrayscaleSetting.h"


extern CONFIGURATION			BViewerConfiguration;

LIST_HEAD						AvailablePresetList = 0;


//___________________________________________________________________________
//
// The module header for this module:
//

static MODULE_INFO		PresetModuleInfo = { MODULE_PRESET, "Preset Module", InitPresetModule, ClosePresetModule };


static ERROR_DICTIONARY_ENTRY	PresetErrorCodes[] =
			{
				{ PRESET_ERROR_INSUFFICIENT_MEMORY		, "An error occurred allocating a memory block for data storage." },
				{ PRESET_ERROR_FILE_OPEN_FOR_READ		, "An error occurred attempting to open the grayscale preset file for reading." },
				{ PRESET_ERROR_FILE_OPEN_FOR_WRITE		, "An error occurred attempting to open the grayscale preset file for writing." },
				{ PRESET_ERROR_FILE_WRITE_ERROR			, "An error occurred writing to the grayscale preset file." },			// *[2] Added write error message.
				{ 0										, NULL }
			};

static ERROR_DICTIONARY_MODULE		PresetStatusErrorDictionary =
										{
										MODULE_PRESET,
										PresetErrorCodes,
										PRESET_ERROR_DICT_LENGTH,
										0
										};

// This function must be called before any other function in this module.
void InitPresetModule()
{
	LinkModuleToList( &PresetModuleInfo );
	RegisterErrorDictionary( &PresetStatusErrorDictionary );
	ReadPresetFile();
}


void ClosePresetModule()
{
	WritePresetFile();
	ErasePresetList();			// *[1] Eliminate memory leak by deallocating the preset list before program exit.
}


// CClient dialog

// IMPLEMENT_DYNAMIC( CPreset, CDialog )

CPreset::CPreset( CWnd *pParent /*=NULL*/ ) : CDialog( CPreset::IDD, pParent ),
				m_StaticGrayscalePresets( "Image Grayscale Windowing Presets", 390, 50, 18, 9, 6, COLOR_WHITE, COLOR_PATIENT, COLOR_PATIENT,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_TOP_JUSTIFIED | CONTROL_VISIBLE,
										IDC_STATIC_PRESET_TITLE ),
				m_StaticApplyPresetHelpInfo( "These presets are image grayscale settings (windowing, gamma, etc.) which you previously\nsaved.  You can select one and apply it to the current subject study image.",
											550, 50, 12, 6, 6, COLOR_WHITE, COLOR_PATIENT, COLOR_PATIENT,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE | CONTROL_MULTILINE,
										IDC_STATIC_APPLY_PRESET_HELP_INFO ),
				m_StaticSavePresetHelpInfo( "To save the current image grayscale settings (windowing, gamma, etc.) for future use, enter\na unique name for this preset.",
											550, 50, 12, 6, 6, COLOR_WHITE, COLOR_PATIENT, COLOR_PATIENT,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE | CONTROL_MULTILINE,
										IDC_STATIC_SAVE_PRESET_HELP_INFO ),

				m_StaticEditPresetName( "Enter a unique name for this new preset, or else just leave\nthis name and use it until you save another in its place.", 450, 60, 14, 7, 6,
									COLOR_WHITE, COLOR_PATIENT, COLOR_PATIENT,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_MULTILINE | CONTROL_VISIBLE,
									IDC_STATIC_EDIT_PRESET_NAME ),
				m_EditPresetName( "", 450, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_PRESET_NAME ),
				m_StaticCurrentPresets( "View current\nimage presets", 150, 40, 14, 7, 6,
									COLOR_WHITE, COLOR_PATIENT, COLOR_PATIENT,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_MULTILINE | CONTROL_VISIBLE,
									IDC_STATIC_CURRENT_PRESETS ),
	
				m_StaticSelectPreset( "Select image\npreset for\ncurrent image", 150, 60, 14, 7, 6,
									COLOR_WHITE, COLOR_PATIENT, COLOR_PATIENT,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_MULTILINE | CONTROL_VISIBLE,
									IDC_STATIC_SELECT_PRESET ),
				m_ComboBoxSelectPreset( "", 450, 300, 18, 9, 5, VARIABLE_PITCH_FONT,
								COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_VSCROLL | EDIT_BORDER | LIST_SORT | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_COMBO_SELECT_PRESET ),
				m_ButtonSave( "Save This Image\nPreset", 180, 40, 16, 8, 6,
								COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
								BUTTON_PUSHBUTTON | CONTROL_VISIBLE | CONTROL_MULTILINE |
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
								IDC_BUTTON_SAVE_IMAGE_PRESET ),
				m_ButtonApply( "Apply Selected\nImage Preset", 180, 40, 16, 8, 6,
								COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
								BUTTON_PUSHBUTTON | CONTROL_VISIBLE | CONTROL_MULTILINE |
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
								IDC_BUTTON_APPLY_IMAGE_PRESET ),
				m_ButtonDelete( "Delete The\nSelected Preset", 180, 40, 16, 8, 6,
								COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
								BUTTON_PUSHBUTTON | CONTROL_VISIBLE | CONTROL_MULTILINE |
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
								IDC_BUTTON_DELETE_IMAGE_PRESET ),
				m_ButtonCancel( "Cancel", 180, 40, 16, 8, 6,
								COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
								BUTTON_PUSHBUTTON | CONTROL_VISIBLE |
								CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
								IDC_BUTTON_CANCEL_IMAGE_PRESET )
{
	m_BkgdBrush.CreateSolidBrush( COLOR_CONFIG );
	m_bSaveImageSetting = FALSE;
}


CPreset::~CPreset()
{
	DestroyWindow();
}


BEGIN_MESSAGE_MAP( CPreset, CDialog )
	//{{AFX_MSG_MAP(CClient)
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_SAVE_IMAGE_PRESET, OnBnClickedSaveGrayscalePreset )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_APPLY_IMAGE_PRESET, OnBnClickedApplyGrayscalePreset )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_DELETE_IMAGE_PRESET, OnBnClickedDeleteGrayscalePreset )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_CANCEL_IMAGE_PRESET, OnBnClickedCancelGrayscalePresets )
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CPreset::OnInitDialog()
{
	RECT			ClientRect;
	static char		TextString[ 64 ];
	int				PrimaryScreenWidth;
	int				PrimaryScreenHeight;

	CDialog::OnInitDialog();

	GetClientRect( &ClientRect );

	if ( m_bSaveImageSetting )
		{
		m_StaticGrayscalePresets.SetPosition( 40, 20, this );
		m_StaticSavePresetHelpInfo.SetPosition( 40, 40, this );
		m_StaticEditPresetName.SetPosition( 20, 90, this );

		m_EditPresetName.SetPosition( 20, 140, this );
		m_EditPresetName.SetWindowTextA( m_pCurrentPreset -> m_PresetName );

		m_ButtonSave.SetPosition( 40, 180, this );
		m_ButtonCancel.SetPosition( 440, 180, this );

		m_StaticCurrentPresets.SetPosition( 20, 290, this );
		m_ComboBoxSelectPreset.SetPosition( 180, 300, this );
		}
	else
		{
		m_StaticGrayscalePresets.SetPosition( 40, 20, this );
		m_StaticApplyPresetHelpInfo.SetPosition( 40, 40, this );

		m_StaticSelectPreset.SetPosition( 20, 90, this );
		m_ComboBoxSelectPreset.SetPosition( 180, 100, this );

		m_ButtonApply.SetPosition( 40, 300, this );
		m_ButtonDelete.SetPosition( 240, 300, this );
		m_ButtonCancel.SetPosition( 440, 300, this );
		}

	LoadPresetSelectionList();

	PrimaryScreenWidth = ::GetSystemMetrics( SM_CXSCREEN );
	PrimaryScreenHeight = ::GetSystemMetrics( SM_CYSCREEN );
	
	SetWindowPos( &wndTop, ( PrimaryScreenWidth - 660 ) / 2, ( PrimaryScreenHeight - 350 ) / 2, 660, 380, SWP_SHOWWINDOW );

	return TRUE; 
}


void ErasePresetList()
{
	LIST_ELEMENT			*pPresetListElement;
	IMAGE_GRAYSCALE_SETTING	*pGrayscalePreset;

	pPresetListElement = AvailablePresetList;
	while ( pPresetListElement != 0 )
		{
		pGrayscalePreset = (IMAGE_GRAYSCALE_SETTING*)pPresetListElement -> pItem;
		RemoveFromList( &AvailablePresetList, (void*)pGrayscalePreset );
		if ( pGrayscalePreset != 0 )
			free( pGrayscalePreset );
		pPresetListElement = AvailablePresetList;
		}
}


void SetPresetFileSpecification( char *pImagePresetFileSpec )
{

	strncpy_s( pImagePresetFileSpec, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.ClientDirectory, _TRUNCATE );	// *[2] Replaced strncat with strncpy_s.
	LocateOrCreateDirectory( pImagePresetFileSpec );	// Ensure directory exists.
	if ( pImagePresetFileSpec[ strlen( pImagePresetFileSpec ) - 1 ] != '\\' )
		strncat_s( pImagePresetFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );								// *[3] Replaced strcat with strncat_s.
	strncat_s( pImagePresetFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "ImagePresets", _TRUNCATE );							// *[2] Replaced strncat with strncat_s.
	strncat_s( pImagePresetFileSpec, FULL_FILE_SPEC_STRING_LENGTH, ".cfg", _TRUNCATE );									// *[2] Replaced strncat with strncat_s.
}


BOOL ReadPresetFile()
{
	BOOL						bNoError = TRUE;
	char						ImagePresetFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	FILE						*pImagePresetFile;
	BOOL						bFileHasBeenCompletelyRead;
	size_t						nBytesToRead;
	size_t						nBytesRead;
	IMAGE_GRAYSCALE_SETTING		GrayscalePreset;
	IMAGE_GRAYSCALE_SETTING		*pNewGrayscalePreset;

	SetPresetFileSpecification( ImagePresetFileSpec );
	pImagePresetFile = fopen( ImagePresetFileSpec, "rb" );
	if ( pImagePresetFile != 0 )
		{
		ErasePresetList();
		bFileHasBeenCompletelyRead = FALSE;
		while( !bFileHasBeenCompletelyRead )
			{
			nBytesToRead = sizeof(IMAGE_GRAYSCALE_SETTING);
			nBytesRead = fread_s( &GrayscalePreset, sizeof(IMAGE_GRAYSCALE_SETTING), 1, nBytesToRead, pImagePresetFile );		// *[2] Converted from fread to fread_s.
			bFileHasBeenCompletelyRead = ( nBytesRead < nBytesToRead );
			if ( !bFileHasBeenCompletelyRead )
				{
				pNewGrayscalePreset = (IMAGE_GRAYSCALE_SETTING*)malloc( sizeof(IMAGE_GRAYSCALE_SETTING) );
				if ( pNewGrayscalePreset != 0 )
					{
					memcpy( pNewGrayscalePreset, &GrayscalePreset, sizeof(IMAGE_GRAYSCALE_SETTING) );
					AppendToList( &AvailablePresetList, (void*)pNewGrayscalePreset );
					}
				}
			}
		fclose( pImagePresetFile );
		}
	else
		{
		RespondToError( MODULE_PRESET, PRESET_ERROR_FILE_OPEN_FOR_READ );
		bNoError = FALSE;
		}
	
	return bNoError;
}


BOOL WritePresetFile()
{
	BOOL						bNoError = TRUE;
	char						ImagePresetFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	FILE						*pImagePresetFile;
	LIST_ELEMENT				*pListElement;
	IMAGE_GRAYSCALE_SETTING		*pGrayscalePreset;
	size_t						nBytesWritten;

	SetPresetFileSpecification( ImagePresetFileSpec );
	pImagePresetFile = fopen( ImagePresetFileSpec, "wb" );
	if ( pImagePresetFile != 0 )
		{
		pListElement = AvailablePresetList;
		while ( bNoError && pListElement != 0 )											// *[2] Add test for error.
			{
			pGrayscalePreset = (IMAGE_GRAYSCALE_SETTING*)pListElement -> pItem;
			nBytesWritten = fwrite( (void*)pGrayscalePreset, 1, sizeof( IMAGE_GRAYSCALE_SETTING ), pImagePresetFile );
			bNoError = ( nBytesWritten == sizeof( IMAGE_GRAYSCALE_SETTING ) );			// *[2] Add test for error.
			if ( bNoError )																// *[2] 
				RespondToError( MODULE_PRESET, PRESET_ERROR_FILE_WRITE_ERROR );			// *[2] 
			pListElement = pListElement -> pNextListElement;
			}
		fclose( pImagePresetFile );
		}
	else
		{
		RespondToError( MODULE_PRESET, PRESET_ERROR_FILE_OPEN_FOR_WRITE );
		bNoError = FALSE;
		}
	
	return bNoError;
}


BOOL CPreset::LoadPresetSelectionList()
{
	BOOL						bNoError = TRUE;
	LIST_ELEMENT				*pPresetListElement;
	IMAGE_GRAYSCALE_SETTING		*pGrayscalePreset;
	int							nItemIndex;

	m_ComboBoxSelectPreset.ResetContent();
	m_ComboBoxSelectPreset.SetWindowTextA( "Client List" );

	pPresetListElement = AvailablePresetList;
	while ( pPresetListElement != 0 )
		{
		pGrayscalePreset = (IMAGE_GRAYSCALE_SETTING*)pPresetListElement -> pItem;
		nItemIndex = m_ComboBoxSelectPreset.AddString( pGrayscalePreset -> m_PresetName );
		m_ComboBoxSelectPreset.SetItemDataPtr( nItemIndex, (void*)pGrayscalePreset );
		pPresetListElement = pPresetListElement -> pNextListElement;
		}
	m_ComboBoxSelectPreset.SetCurSel( 0 );

	return bNoError;
}


void CPreset::OnPresetSelected()
{
	IMAGE_GRAYSCALE_SETTING		*pGrayscalePreset;
	int							nItemIndex;

	nItemIndex = m_ComboBoxSelectPreset.GetCurSel();
	if ( nItemIndex != CB_ERR )
		{
		pGrayscalePreset = (IMAGE_GRAYSCALE_SETTING*)m_ComboBoxSelectPreset.GetItemDataPtr( nItemIndex );
		m_pCurrentPreset = pGrayscalePreset;
		}
	else
		m_pCurrentPreset = 0;
}


void CPreset::OnBnClickedSaveGrayscalePreset( NMHDR *pNMHDR, LRESULT *pResult )
{
	m_EditPresetName.GetWindowText( m_pCurrentPreset -> m_PresetName, MAX_CFG_STRING_LENGTH );
	m_ButtonSave.HasBeenPressed( TRUE );
	CDialog::OnOK();

	*pResult = 0;
}


void CPreset::OnBnClickedApplyGrayscalePreset( NMHDR *pNMHDR, LRESULT *pResult )
{
	IMAGE_GRAYSCALE_SETTING		*pGrayscalePreset;
	int							nItemIndex;

	m_ButtonSave.HasBeenPressed( TRUE );
	nItemIndex = m_ComboBoxSelectPreset.GetCurSel();
	if ( nItemIndex != CB_ERR )
		{
		pGrayscalePreset = (IMAGE_GRAYSCALE_SETTING*)m_ComboBoxSelectPreset.GetItemDataPtr( nItemIndex );
		m_pCurrentPreset = pGrayscalePreset;
		}
	else
		m_pCurrentPreset = 0;
	CDialog::OnOK();

	*pResult = 0;
}



void CPreset::OnBnClickedDeleteGrayscalePreset( NMHDR *pNMHDR, LRESULT *pResult )
{
	IMAGE_GRAYSCALE_SETTING		*pGrayscalePreset;
	int							nItemIndex;

	m_ButtonDelete.HasBeenPressed( TRUE );
	nItemIndex = m_ComboBoxSelectPreset.GetCurSel();
	if ( nItemIndex != CB_ERR )
		{
		pGrayscalePreset = (IMAGE_GRAYSCALE_SETTING*)m_ComboBoxSelectPreset.GetItemDataPtr( nItemIndex );
		if ( pGrayscalePreset != 0 )
			{
			RemoveFromList( &AvailablePresetList, (void*)pGrayscalePreset );
			free( pGrayscalePreset );
			}
		m_pCurrentPreset = 0;
		}

	CDialog::OnCancel();

	*pResult = 0;
}


void CPreset::OnBnClickedCancelGrayscalePresets( NMHDR *pNMHDR, LRESULT *pResult )
{
	m_ButtonCancel.HasBeenPressed( TRUE );
	CDialog::OnCancel();

	*pResult = 0;
}


HBRUSH CPreset::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
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


BOOL CPreset::OnEraseBkgnd( CDC *pDC )
{
	CBrush		BackgroundBrush( COLOR_PATIENT );
	CRect		BackgroundRectangle;
	CBrush		*pOldBrush = pDC -> SelectObject( &BackgroundBrush );

	GetClientRect( BackgroundRectangle );
	pDC -> FillRect( BackgroundRectangle, &BackgroundBrush );
	pDC -> SelectObject( pOldBrush );

	return TRUE;
}


void CPreset::OnClose()
{
	WritePresetFile();

	CDialog::OnClose();
}



