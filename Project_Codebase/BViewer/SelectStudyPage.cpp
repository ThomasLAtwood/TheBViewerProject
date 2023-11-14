// SelectStudyPage.cpp : implementation file for the CSelectStudyPage class of
//	CPropertyPage, which implements the "Select Subject Study" tab of the main Control Panel.
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
//	*[2] 03/28/2023 by Tom Atwood
//		Fixed code security issues.
//	*[1] 01/10/2023 by Tom Atwood
//		Fixed code security issues.
//
//
#include "stdafx.h"
#include "BViewer.h"
#include "Module.h"
#include "ReportStatus.h"
#include "Configuration.h"
#include "DiagnosticImage.h"
#include "Mouse.h"
#include "ImageView.h"
#include "MainFrm.h"
#include "SelectStudyPage.h"
#include "Study.h"
#include "Dicom.h"
#include "ManualStudyEntry.h"


extern CBViewerApp					ThisBViewerApp;
extern CONFIGURATION				BViewerConfiguration;
extern CString						ExplorerWindowClass;
extern BOOL							bMakeDumbButtons;


// CSelectStudyPage dialog
CSelectStudyPage::CSelectStudyPage() : CPropertyPage( CSelectStudyPage::IDD )
{
	m_pPatientListCtrl = 0;
	m_pImportSelector = 0;
	m_BkgdBrush.CreateSolidBrush( COLOR_PATIENT );
}


CSelectStudyPage::~CSelectStudyPage()
{
	if ( m_pPatientListCtrl != 0 )
		delete m_pPatientListCtrl;
	m_pPatientListCtrl = 0;
	if ( m_pImportSelector != 0 )
		delete m_pImportSelector;
	m_pImportSelector = 0;
}


BEGIN_MESSAGE_MAP( CSelectStudyPage, CPropertyPage )
	//{{AFX_MSG_MAP(CSelectStudyPage)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CSelectStudyPage::OnInitDialog()
{
	RECT			ClientRect;

	CPropertyPage::OnInitDialog();
	GetClientRect( &ClientRect );
	m_pPatientListCtrl = new CStudySelector();
	if ( m_pPatientListCtrl != 0 )
		{
		m_pPatientListCtrl -> Create( WS_CHILD | WS_MAXIMIZE | WS_TABSTOP | WS_VISIBLE |
										LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL | LVS_SORTASCENDING,
										ClientRect, this, IDC_PATIENT_LIST );
		m_pPatientListCtrl -> SetExtendedStyle( LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
		m_pPatientListCtrl -> SetTextBkColor( COLOR_ANALYSIS_BKGD );
		m_pPatientListCtrl -> SetTextColor( COLOR_WHITE );
		}
	
	return TRUE;
}


void CSelectStudyPage::ResetCurrentSelection()
{
	m_pPatientListCtrl -> m_nCurrentlySelectedItem = -1;
}


void CSelectStudyPage::UpdateSelectionList()
{
	m_pPatientListCtrl -> UpdatePatientList();
	Invalidate();
	UpdateWindow();
}


BOOL CSelectStudyPage::OnSetActive()
{
	CMainFrame			*pMainFrame;
	char				*pChar;
	char				AutoOutputImageFileSpec[ FILE_PATH_STRING_LENGTH ];
	char				Msg[ MAX_EXTRA_LONG_STRING_LENGTH ];
	CControlPanel		*pControlPanel;

	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( pMainFrame != 0 )
		{
		if ( !bMakeDumbButtons )
			{
			pMainFrame -> m_wndDlgBar.m_ButtonDeleteCheckedImages.EnableWindow( TRUE );
			pMainFrame -> m_wndDlgBar.m_ButtonDeleteCheckedImages.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
			pMainFrame -> m_wndDlgBar.m_ButtonDeleteCheckedImages.Invalidate( TRUE );
			}
		
		pMainFrame -> m_wndDlgBar.m_ButtonImportImages.EnableWindow( TRUE );
		pMainFrame -> m_wndDlgBar.m_ButtonImportImages.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
		pMainFrame -> m_wndDlgBar.m_ButtonImportImages.Invalidate( TRUE );
		
		pMainFrame -> m_wndDlgBar.m_ButtonEnterManualStudy.EnableWindow( TRUE );
		pMainFrame -> m_wndDlgBar.m_ButtonEnterManualStudy.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
		pMainFrame -> m_wndDlgBar.m_ButtonEnterManualStudy.Invalidate( TRUE );
		}
	if ( ThisBViewerApp.m_lpCmdLine[0] != _T('\0') )
		{
		// Select/check and view a file passed as the first command line parameter.
		strncpy_s( m_AutoOpenFileSpec, FULL_FILE_SPEC_STRING_LENGTH, ThisBViewerApp.m_lpCmdLine, _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
		PruneQuotationMarks( m_AutoOpenFileSpec );
		_snprintf_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, _TRUNCATE, "Initializing for automatically viewing cmd line file specification: %s", m_AutoOpenFileSpec );	// *[2] Replaced sprintf() with _snprintf_s.
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
		// Create an output file spec for copying to the watch folder.
		pChar = strrchr( m_AutoOpenFileSpec, '\\' );
		pChar++;
		strncpy_s( AutoOutputImageFileSpec, FILE_PATH_STRING_LENGTH, BViewerConfiguration.WatchDirectory, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
		if ( AutoOutputImageFileSpec[ strlen( AutoOutputImageFileSpec ) - 1 ] != '\\' )
			strncat_s( AutoOutputImageFileSpec, FILE_PATH_STRING_LENGTH, "\\", _TRUNCATE );								// *[2] Replaced strcat with strncat_s.
		strncat_s( AutoOutputImageFileSpec, FILE_PATH_STRING_LENGTH, "AutoLoad_", _TRUNCATE );							// *[2] Replaced strcat with strncat_s.
		strncat_s( AutoOutputImageFileSpec, FILE_PATH_STRING_LENGTH, pChar, _TRUNCATE );								// *[2] Replaced strncat with strncat_s.
		// Copy the specified file to the watch folder.
		CopyFile( m_AutoOpenFileSpec, AutoOutputImageFileSpec, FALSE );
		ThisBViewerApp.m_lpCmdLine[0] = _T('\0');
		// The following setting is not reversed in the current software.  Therefore,
		// once a command line file has been processed, all further received files
		// will display automatically.
		ThisBViewerApp.m_bAutoViewStudyReceived = TRUE;
		}
	if ( pMainFrame != 0 )																								// *[2] Added NULL check.
		pMainFrame -> UpdateImageList();

	pControlPanel = (CControlPanel*)GetParent();
	if ( pControlPanel != 0 )
		pControlPanel -> m_CurrentlyActivePage = STUDY_SELECTION_PAGE;

	return CPropertyPage::OnSetActive();
}


BOOL CSelectStudyPage::OnKillActive()
{
	CMainFrame			*pMainFrame;

	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( pMainFrame != 0 )
		{
		pMainFrame -> m_wndDlgBar.m_ButtonDeleteCheckedImages.EnableWindow( FALSE );
		pMainFrame -> m_wndDlgBar.m_ButtonDeleteCheckedImages.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
		pMainFrame -> m_wndDlgBar.m_ButtonDeleteCheckedImages.Invalidate( TRUE );

		pMainFrame -> m_wndDlgBar.m_ButtonImportImages.EnableWindow( FALSE );
		pMainFrame -> m_wndDlgBar.m_ButtonImportImages.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
		pMainFrame -> m_wndDlgBar.m_ButtonImportImages.Invalidate( TRUE );

		pMainFrame -> m_wndDlgBar.m_ButtonEnterManualStudy.EnableWindow( FALSE );
		pMainFrame -> m_wndDlgBar.m_ButtonEnterManualStudy.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
		pMainFrame -> m_wndDlgBar.m_ButtonEnterManualStudy.Invalidate( TRUE );

		pMainFrame -> m_wndDlgBar.Invalidate( TRUE );
		}

	return CPropertyPage::OnKillActive();
}


void CSelectStudyPage::OnSize( UINT nType, int cx, int cy )
{
	CPropertyPage::OnSize( nType, cx, cy );

	if ( m_pPatientListCtrl != 0 )
		m_pPatientListCtrl -> SetWindowPos( 0, 0, 0, cx - 5, cy - 5, 0 );
}


void CSelectStudyPage::DeleteCheckedImages()
{
	int						nItem;
	int						nItems;
	LIST_ELEMENT			*pAvailableStudyListElement;
	DIAGNOSTIC_STUDY		*pStudyDataRow;
	DIAGNOSTIC_SERIES		*pSeriesDataRow;
	DIAGNOSTIC_IMAGE		*pImageDataRow;
	CStudy					*pStudy = 0;			// *[2] Added redundant initialization to please Fortify.
	BOOL					bMatchingImageFound;
 	CMainFrame				*pMainFrame;
	CString					SubitemText;

	if ( m_pPatientListCtrl != 0 )
		{
		nItems = m_pPatientListCtrl -> GetItemCount();
		for ( nItem = 0; nItem < nItems; nItem++ )
			{
			if ( m_pPatientListCtrl -> GetCheck( nItem ) != 0 )
				{
				bMatchingImageFound = FALSE;
				SubitemText = m_pPatientListCtrl -> GetItemText( nItem, m_pPatientListCtrl -> m_pListFormat -> nColumns - 1 );
				pAvailableStudyListElement = ThisBViewerApp.m_AvailableStudyList;
				while ( pAvailableStudyListElement != 0 && !bMatchingImageFound )
					{
					pStudy = (CStudy*)pAvailableStudyListElement -> pItem;
					if ( pStudy != 0 )
						{
						pStudyDataRow = pStudy -> m_pDiagnosticStudyList;
						while ( pStudyDataRow != 0 && !bMatchingImageFound )
							{
							pSeriesDataRow = pStudyDataRow -> pDiagnosticSeriesList;
							while ( pSeriesDataRow != 0 && !bMatchingImageFound )
								{
								pImageDataRow = pSeriesDataRow -> pDiagnosticImageList;
								while ( pImageDataRow != 0 && !bMatchingImageFound )
									{
									if ( strcmp( (const char*)SubitemText, pImageDataRow -> SOPInstanceUID ) == 0 )
										bMatchingImageFound = TRUE;
									pImageDataRow = pImageDataRow -> pNextDiagnosticImage;
									}
								pSeriesDataRow = pSeriesDataRow -> pNextDiagnosticSeries;
								}
							pStudyDataRow = pStudyDataRow -> pNextDiagnosticStudy;
							}	
						}
					pAvailableStudyListElement = pAvailableStudyListElement -> pNextListElement;
					}			// ...end while next study.
				if ( bMatchingImageFound )
					{
					if ( pStudy == ThisBViewerApp.m_pCurrentStudy )
						{
						// Remove any displayed image from subject study monitor.
						pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
						if ( pMainFrame != 0 )
							{
							if ( pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] != 0 )
								{
								pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> ClearImageDisplay();
								pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> UpdateWindow();
								}
							ThisBViewerApp.m_pCurrentStudy = 0;
							}
						}
					pStudy -> DeleteStudyDataAndImages();
					RemoveFromList( &ThisBViewerApp.m_AvailableStudyList, (void*)pStudy );
					delete pStudy;
					pStudy = 0;			// *[1] Added this for code safety.
					m_pPatientListCtrl -> m_nCurrentlySelectedItem = -1;
					}
				}			// ...end if item checked.
			}			// ...end for next list item.
		m_pPatientListCtrl -> UpdatePatientList();
		nItems = m_pPatientListCtrl -> GetItemCount();
		for ( nItem = 0; nItem < nItems; nItem++ )
			{
			m_pPatientListCtrl -> SetCheck( nItem, 0 );
			}
		Invalidate();
		UpdateWindow();
		}
}


void CSelectStudyPage::DeleteStudyList()
{
	int						nItem;
	int						nItems;
	LIST_ELEMENT			*pAvailableStudyListElement;
	CStudy					*pStudy;
 	CMainFrame				*pMainFrame;

	if ( m_pPatientListCtrl != 0 )
		{
		nItems = m_pPatientListCtrl -> GetItemCount();
		for ( nItem = 0; nItem < nItems; nItem++ )
			{
			pAvailableStudyListElement = ThisBViewerApp.m_AvailableStudyList;
			while ( pAvailableStudyListElement != 0 )
				{
				pStudy = (CStudy*)pAvailableStudyListElement -> pItem;
				if ( pStudy != 0 )
					{
					if ( pStudy == ThisBViewerApp.m_pCurrentStudy )
						{
						// Remove any displayed image from subject study monitor.
						pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
						if ( pMainFrame != 0 )
							{
							if ( pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] != 0 )
								{
								pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> ClearImageDisplay();
								pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> UpdateWindow();
								}
							ThisBViewerApp.m_pCurrentStudy = 0;
							}
						}
					pStudy -> DeleteStudyDataAndImages();
					RemoveFromList( &ThisBViewerApp.m_AvailableStudyList, (void*)pStudy );
					delete pStudy;
					pStudy = 0;
					pAvailableStudyListElement = 0;
					m_pPatientListCtrl -> m_nCurrentlySelectedItem = -1;
					}
				}
			}			// ...end for next list item.
		m_pPatientListCtrl -> UpdatePatientList();
		nItems = m_pPatientListCtrl -> GetItemCount();
		for ( nItem = 0; nItem < nItems; nItem++ )
			{
			m_pPatientListCtrl -> SetCheck( nItem, 0 );
			}
		Invalidate();
		UpdateWindow();
		}
}


void CSelectStudyPage::OnImportLocalImages()
{
	BOOL					bNoError = TRUE;
	CString					FullFileSpec;
	int						DialogWidth;
	int						DialogHeight;
 	CMainFrame				*pMainFrame;
	RECT					ClientRect;
	INT						ClientWidth;
	INT						ClientHeight;
	
	DialogWidth = 700;
	DialogHeight = 700;
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( pMainFrame != 0 )
		{
		pMainFrame -> GetClientRect( &ClientRect );
		ClientWidth = ClientRect.right - ClientRect.left;
		ClientHeight = ClientRect.bottom - ClientRect.top;
		}
	else
		{
		ClientWidth = 1024;
		ClientHeight = 768;
		}

	if ( m_pImportSelector != 0 )
		delete m_pImportSelector;
	m_pImportSelector = new CImportSelector( DialogWidth, DialogHeight, COLOR_PATIENT, 0 );
	if ( m_pImportSelector != 0 )
		{
		m_pImportSelector -> SetPosition( ( ClientWidth - DialogWidth ) / 2, ( ClientHeight - DialogHeight ) / 2, this, ExplorerWindowClass );
		m_pImportSelector -> BringWindowToTop();
		m_pImportSelector -> SetFocus();
		}
}


void CSelectStudyPage::OnCreateAManualStudy()
{
	BOOL					bNoError = TRUE;
	CManualStudyEntry		*pManualStudyEntryDialog;
	BOOL					bCancel;
	char					AbstractTitlesTextLine[ 1024 ];
	char					AbstractDataTextLine[ 1024 ];
	SYSTEMTIME				CurrentTime;;
	char					SOPInstanceUID[ DICOM_ATTRIBUTE_UI_STRING_LENGTH ];
	char					AbstractFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	FILE					*pAbstractFile;
	DWORD					AbstractFileSize;
	char					Msg[ MAX_LOGGING_STRING_LENGTH ];
 	CMainFrame				*pMainFrame;
	
	pManualStudyEntryDialog = new CManualStudyEntry( this );
	if ( pManualStudyEntryDialog != 0 )
		{
		bCancel = !( pManualStudyEntryDialog -> DoModal() == IDOK );
		if ( !bCancel )
			{
			strncpy_s( AbstractTitlesTextLine, 1024,
						"DestinationAE,PatientsName,PatientID,PatientsBirthDate,PatientsSex,StudyDate,AccessionNumber,InstitutionName,ReferringPhysiciansName,StudyDescription,SOPInstanceUID\n",
						_TRUNCATE );									// *[1] Replaced strcpy with strncpy_s.);
			strncpy_s( AbstractDataTextLine, 1024, ",", _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
			strncat_s( AbstractDataTextLine, 1024, pManualStudyEntryDialog -> m_PatientLastName, _TRUNCATE );			// *[2] Replaced strcat with strncat_s.
			strncat_s( AbstractDataTextLine, 1024, "^", _TRUNCATE );													// *[2] Replaced strcat with strncat_s.
			strncat_s( AbstractDataTextLine, 1024, pManualStudyEntryDialog -> m_PatientFirstName, _TRUNCATE );			// *[2] Replaced strcat with strncat_s.
			strncat_s( AbstractDataTextLine, 1024, ",", _TRUNCATE );													// *[2] Replaced strcat with strncat_s.
			strncat_s( AbstractDataTextLine, 1024, pManualStudyEntryDialog -> m_PatientID, _TRUNCATE );					// *[2] Replaced strcat with strncat_s.
			strncat_s( AbstractDataTextLine, 1024, ",", _TRUNCATE );													// *[2] Replaced strcat with strncat_s.
			strncat_s( AbstractDataTextLine, 1024, pManualStudyEntryDialog -> m_DateOfBirthText, _TRUNCATE );			// *[2] Replaced strcat with strncat_s.
			strncat_s( AbstractDataTextLine, 1024, ",", _TRUNCATE );													// *[2] Replaced strcat with strncat_s.
			strncat_s( AbstractDataTextLine, 1024, pManualStudyEntryDialog -> m_PatientSex, _TRUNCATE );				// *[2] Replaced strcat with strncat_s.
			strncat_s( AbstractDataTextLine, 1024, ",", _TRUNCATE );													// *[2] Replaced strcat with strncat_s.
			strncat_s( AbstractDataTextLine, 1024, pManualStudyEntryDialog -> m_StudyDateText, _TRUNCATE );				// *[2] Replaced strcat with strncat_s.
			strncat_s( AbstractDataTextLine, 1024, ",", _TRUNCATE );													// *[2] Replaced strcat with strncat_s.
			strncat_s( AbstractDataTextLine, 1024, pManualStudyEntryDialog -> m_AccessionNumber, _TRUNCATE );			// *[2] Replaced strcat with strncat_s.
			strncat_s( AbstractDataTextLine, 1024, ",", _TRUNCATE );													// *[2] Replaced strcat with strncat_s.
			strncat_s( AbstractDataTextLine, 1024, pManualStudyEntryDialog -> m_OrderingInstitution, _TRUNCATE );		// *[2] Replaced strcat with strncat_s.
			strncat_s( AbstractDataTextLine, 1024, ",", _TRUNCATE );													// *[2] Replaced strcat with strncat_s.
			strncat_s( AbstractDataTextLine, 1024, pManualStudyEntryDialog -> m_ReferringPhysiciansName, _TRUNCATE );	// *[2] Replaced strcat with strncat_s.
			strncat_s( AbstractDataTextLine, 1024, ",", _TRUNCATE );													// *[2] Replaced strcat with strncat_s.
			strncat_s( AbstractDataTextLine, 1024, "Manual Data Entry   No Image", _TRUNCATE );							// *[2] Replaced strcat with strncat_s.
			strncat_s( AbstractDataTextLine, 1024, ",", _TRUNCATE );													// *[2] Replaced strcat with strncat_s.
			GetLocalTime( &CurrentTime );
			_snprintf_s( SOPInstanceUID, DICOM_ATTRIBUTE_UI_STRING_LENGTH, _TRUNCATE, "BViewer.Manual.%4u%u%u.%u%u%u",	// *[2] Replaced sprintf() with _snprintf_s.
							CurrentTime.wYear, CurrentTime.wMonth, CurrentTime.wDay, CurrentTime.wHour, CurrentTime.wMinute, CurrentTime.wSecond );
			strncat_s( AbstractDataTextLine, 1024, SOPInstanceUID, _TRUNCATE );											// *[2] Replaced strcat with strncat_s.
			strncat_s( AbstractDataTextLine, 1024, "\n", _TRUNCATE );													// *[2] Replaced strcat with strncat_s.

			// Create an abstract data file and write it out.  BViewer will pick it up and process it.
			strncpy_s( AbstractFileSpec, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.AbstractsDirectory, _TRUNCATE );	// *[2] Replaced strncat with strncpy_s.
			LocateOrCreateDirectory( AbstractFileSpec );	// Ensure directory exists.
			if ( AbstractFileSpec[ strlen( AbstractFileSpec ) - 1 ] != '\\' )
				strncat_s( AbstractFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );									// *[2] Replaced strcat with strncat_s.
			strncat_s( AbstractFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "Import.axt", _TRUNCATE );								// *[2] Replaced strcat with strncat_s.
			AbstractFileSize = GetCompressedFileSize( AbstractFileSpec, NULL );
			_snprintf_s( Msg, MAX_LOGGING_STRING_LENGTH, _TRUNCATE, "    Opening abstract file:  %s", AbstractFileSpec );		// *[2] Replaced sprintf() with _snprintf_s.
			LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );

			pAbstractFile = fopen( AbstractFileSpec, "at" );
			if ( pAbstractFile != 0 )
				{
				// Prefix with a line of column headings, if this is a new file.
				if ( AbstractFileSize == 0 || AbstractFileSize == INVALID_FILE_SIZE )
					fputs( AbstractTitlesTextLine, pAbstractFile );
				// Output the abstract value sequence line.
				fputs( AbstractDataTextLine, pAbstractFile );
				fclose( pAbstractFile );
				_snprintf_s( Msg, MAX_LOGGING_STRING_LENGTH, _TRUNCATE, "A manual study entry was appended to the abstract file:  %s", AbstractFileSpec );	// *[2] Replaced sprintf() with _snprintf_s.
				LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
				}
			pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
			if ( pMainFrame != 0 )
				{
				if ( pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] != 0 )
					{
					pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> ClearImageDisplay();
					pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> UpdateWindow();
					}
				}
			}
		delete pManualStudyEntryDialog;
		}
}


BOOL CSelectStudyPage::OnNotify( WPARAM wParam, LPARAM lParam, LRESULT *pResult )
{
	NMLISTVIEW			*pListViewNotification;
	BOOL				bOK = TRUE;

	pListViewNotification = (NMLISTVIEW*)lParam;
	if ( pListViewNotification -> hdr.code == HDN_ITEMCLICK && m_pPatientListCtrl != 0 )
		m_pPatientListCtrl -> OnHeaderClick( pListViewNotification, pResult );

	// This function override is required to handle windows message notifications
	// from some of the child controls.
	return CPropertyPage::OnNotify( wParam, lParam, pResult );
}


HBRUSH CSelectStudyPage::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
{
    // Return handle to the parent (this) window's background CBrush object
    // for use in painting by the child controls.
	return HBRUSH( m_BkgdBrush );
}



