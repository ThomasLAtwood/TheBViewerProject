// StudySelector.cpp : Implementation file defining the structure of the CStudySelector
//  class of CListCtrl, which implements the Subject Study selection list.
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
//	*[4] 11/3/2023 by Tom Atwood
//		Replaced pCurrentReaderInfo with pBViewerCustomization -> m_ReaderInfo.
//	*[3] 07/19/2023 by Tom Atwood
//		Fixed code security issues.
//	*[2] 03/14/2023 by Tom Atwood
//		Fixed code security issues.
//	*[1] 01/06/2023 by Tom Atwood
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
#include "DicomDictionary.h"
#include "Study.h"
#include "StudySelector.h"


extern CONFIGURATION				BViewerConfiguration;
extern CCustomization				*pBViewerCustomization;
extern LIST_HEAD					RegisteredUserList;
extern READER_PERSONAL_INFO			LoggedInReaderInfo;			// Saved reader info, used for restoring overwrites from imported studies.


// CStudySelector
CStudySelector::CStudySelector()
{
	m_pListFormat = 0;
	m_nCurrentlySelectedItem = -1;
	m_SelectorHeading.m_pParentStudySelector = (void*)this;
	m_nColumns = 0;
	m_nColumnToSort = 0;
}


CStudySelector::~CStudySelector()
{
}


BEGIN_MESSAGE_MAP( CStudySelector, CListCtrl )
	//{{AFX_MSG_MAP(CStudySelector)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_NOTIFY_REFLECT( NM_CLICK, OnNMClick )
	ON_NOTIFY( HDN_ENDTRACKA, 0, OnHdnEndtrack )
	ON_NOTIFY( HDN_ENDTRACKW, 0, OnHdnEndtrack )
END_MESSAGE_MAP()


static BOOL		bSortAscending[ 22 ];			// Storage for the flags that toggle between sorting the column in ascending or descending order.


int CStudySelector::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	int			nColumn;

	SetBkColor( COLOR_PATIENT );
	if ( CListCtrl::OnCreate( lpCreateStruct ) == -1 )
		return -1;

	m_SelectorHeading.SubclassHeaderCtrl( GetHeaderCtrl() );

	// Initialize the column sorting order flags.
	bSortAscending[ 0 ] = TRUE;
	for ( nColumn = 1; nColumn < 22; nColumn++ )
		bSortAscending[ nColumn ] = FALSE;

	return 0;
}


// NOTE:  The last column in each format must be the "SOP Instance UID".  This serves as the
//			image matching column.
LIST_FORMAT		PatientEmphasisListFormat =
					{ 16, {
						{ " Last Name",			ABSTRACT_LEVEL_PATIENT,	120,	offsetof( CStudy, m_PatientLastName )					},
						{ " First Name",		ABSTRACT_LEVEL_PATIENT, 80,		offsetof( CStudy, m_PatientFirstName )					},
						{ " ID",				ABSTRACT_LEVEL_PATIENT, 100,	offsetof( CStudy, m_PatientID )							},
						{ " Birth Date",		ABSTRACT_LEVEL_PATIENT, 80,		offsetof( CStudy, m_PatientsBirthDate )					},
						{ " Sex",				ABSTRACT_LEVEL_PATIENT, 40,		offsetof( CStudy, m_PatientsSex )						},
						{ " Modality",			ABSTRACT_LEVEL_SERIES,	50,		offsetof( DIAGNOSTIC_SERIES, Modality )					},
						{ " Study Desc.",		ABSTRACT_LEVEL_STUDY,	200,	offsetof( DIAGNOSTIC_STUDY, StudyDescription )			},
						{ " Body Part",			ABSTRACT_LEVEL_SERIES,	100,	offsetof( DIAGNOSTIC_SERIES, BodyPartExamined )			},
						{ " Series Desc.",		ABSTRACT_LEVEL_SERIES,	160,	offsetof( DIAGNOSTIC_SERIES, SeriesDescription )		},
						{ " Study Date",		ABSTRACT_LEVEL_STUDY,	80,		offsetof( DIAGNOSTIC_STUDY, StudyDate )					},
						{ " Date Read",			ABSTRACT_LEVEL_PATIENT,	80,		offsetof( CStudy, m_DateOfReading )						},
						{ " Ref. Physician",	ABSTRACT_LEVEL_STUDY,	160,	offsetof( DIAGNOSTIC_STUDY, ReferringPhysiciansName )	},
						{ " Ref. Phone",		ABSTRACT_LEVEL_STUDY,	100,	offsetof( DIAGNOSTIC_STUDY, ReferringPhysiciansPhone )	},
						{ " Institution",		ABSTRACT_LEVEL_STUDY,	200,	offsetof( DIAGNOSTIC_STUDY, InstitutionName )			},
						{ " Pt. Comments",		ABSTRACT_LEVEL_PATIENT, 600,	offsetof( CStudy, m_PatientComments )					},
						{ " SOP Instance UID",	ABSTRACT_LEVEL_IMAGE,	350,	offsetof( DIAGNOSTIC_IMAGE, SOPInstanceUID )			}
					 } };


LIST_FORMAT		StudyEmphasisListFormat =
					{ 12, {
						{ " Last Name",			ABSTRACT_LEVEL_PATIENT, 180,	offsetof( CStudy, m_PatientLastName )					},
						{ " First Name",		ABSTRACT_LEVEL_PATIENT, 80,		offsetof( CStudy, m_PatientFirstName )					},
						{ " ID",				ABSTRACT_LEVEL_PATIENT, 100,	offsetof( CStudy, m_PatientID )							},
						{ " Accession",			ABSTRACT_LEVEL_STUDY,	60,		offsetof( DIAGNOSTIC_STUDY, AccessionNumber )			},
						{ " Study Date",		ABSTRACT_LEVEL_STUDY,	80,		offsetof( DIAGNOSTIC_STUDY, StudyDate )					},
						{ " Study Time",		ABSTRACT_LEVEL_STUDY,	50,		offsetof( DIAGNOSTIC_STUDY, StudyTime )					},
						{ " Date Read",			ABSTRACT_LEVEL_PATIENT,	80,		offsetof( CStudy, m_DateOfReading )						},
						{ " Ref. Physician",	ABSTRACT_LEVEL_STUDY,	120,	offsetof( DIAGNOSTIC_STUDY, ReferringPhysiciansName )	},
						{ " Study ID",			ABSTRACT_LEVEL_STUDY,	200,	offsetof( DIAGNOSTIC_STUDY, StudyID )					},
						{ " Study Description",	ABSTRACT_LEVEL_STUDY,	150,	offsetof( DIAGNOSTIC_STUDY, StudyDescription )			},
						{ " Study Instance UID",ABSTRACT_LEVEL_STUDY,	350,	offsetof( DIAGNOSTIC_STUDY, StudyInstanceUID )			},
						{ " SOP Instance UID",	ABSTRACT_LEVEL_IMAGE,	350,	offsetof( DIAGNOSTIC_IMAGE, SOPInstanceUID )			}
					} };


LIST_FORMAT		SeriesEmphasisListFormat =
					{ 16, {
						{ " Last Name",			ABSTRACT_LEVEL_PATIENT, 180,	offsetof( CStudy, m_PatientLastName )					},
						{ " First Name",		ABSTRACT_LEVEL_PATIENT, 80,		offsetof( CStudy, m_PatientFirstName )					},
						{ " ID",				ABSTRACT_LEVEL_PATIENT, 100,	offsetof( CStudy, m_PatientID )							},
						{ " Study Date",		ABSTRACT_LEVEL_STUDY,	80,		offsetof( DIAGNOSTIC_STUDY, StudyDate )					},
						{ " Date Read",			ABSTRACT_LEVEL_PATIENT,	80,		offsetof( CStudy, m_DateOfReading )						},
						{ " Study Description",	ABSTRACT_LEVEL_STUDY,	150,	offsetof( DIAGNOSTIC_STUDY, StudyDescription )			},
						{ " Modality",			ABSTRACT_LEVEL_SERIES,	40,		offsetof( DIAGNOSTIC_SERIES, Modality )					},
						{ " Series",			ABSTRACT_LEVEL_SERIES,	40,		offsetof( DIAGNOSTIC_SERIES, SeriesNumber )				},
						{ " Series Time",		ABSTRACT_LEVEL_SERIES,	50,		offsetof( DIAGNOSTIC_SERIES, SeriesTime )				},
						{ " Protocol",			ABSTRACT_LEVEL_SERIES,	80,		offsetof( DIAGNOSTIC_SERIES, ProtocolName )				},
						{ " Laterality",		ABSTRACT_LEVEL_SERIES,	40,		offsetof( DIAGNOSTIC_SERIES, Laterality ),				},
						{ " Series Description",ABSTRACT_LEVEL_SERIES,	160,	offsetof( DIAGNOSTIC_SERIES, SeriesDescription )		},
						{ " Body Part",			ABSTRACT_LEVEL_SERIES,	60,		offsetof( DIAGNOSTIC_SERIES, BodyPartExamined )			},
						{ " Patient Position",	ABSTRACT_LEVEL_SERIES,	40,		offsetof( DIAGNOSTIC_SERIES, PatientPosition )			},
						{ " Orientation",		ABSTRACT_LEVEL_SERIES,	40,		offsetof( DIAGNOSTIC_SERIES, PatientOrientation )		},
						{ " SOP Instance UID",	ABSTRACT_LEVEL_IMAGE,	350,	offsetof( DIAGNOSTIC_IMAGE, SOPInstanceUID )			}
					} };


LIST_FORMAT		ImageEmphasisListFormat =
					{ 24, {
						{ " Last Name",			ABSTRACT_LEVEL_PATIENT, 180,	offsetof( CStudy, m_PatientLastName )					},
						{ " First Name",		ABSTRACT_LEVEL_PATIENT, 80,		offsetof( CStudy, m_PatientFirstName )					},
						{ " ID",				ABSTRACT_LEVEL_PATIENT, 100,	offsetof( CStudy, m_PatientID )							},
						{ " Study Date",		ABSTRACT_LEVEL_STUDY,	80,		offsetof( DIAGNOSTIC_STUDY, StudyDate )					},
						{ " Date Read",			ABSTRACT_LEVEL_PATIENT,	80,		offsetof( CStudy, m_DateOfReading )						},
						{ " Study Description",	ABSTRACT_LEVEL_STUDY,	150,	offsetof( DIAGNOSTIC_STUDY, StudyDescription ),			},
						{ " Body Part",			ABSTRACT_LEVEL_SERIES,	60,		offsetof( DIAGNOSTIC_SERIES, BodyPartExamined )			},
						{ " Image Type",		ABSTRACT_LEVEL_IMAGE,	120,	offsetof( DIAGNOSTIC_IMAGE, ImageType )					},
						{ " Image Number",		ABSTRACT_LEVEL_IMAGE,	40,		offsetof( DIAGNOSTIC_IMAGE, InstanceNumber )			},
						{ " Image Time",		ABSTRACT_LEVEL_IMAGE,	50,		offsetof( DIAGNOSTIC_IMAGE, InstanceCreationTime )		},
						{ " Acq Number",		ABSTRACT_LEVEL_IMAGE,	40,		offsetof( DIAGNOSTIC_IMAGE, AcquisitionNumber )			},
						{ " Samples/Pixel",		ABSTRACT_LEVEL_IMAGE,	40,		offsetof( DIAGNOSTIC_IMAGE, SamplesPerPixel )			},
						{ " Photometric Int",	ABSTRACT_LEVEL_IMAGE,	100,	offsetof( DIAGNOSTIC_IMAGE, PhotometricInterpretation )	},
						{ " Rows",				ABSTRACT_LEVEL_IMAGE,	50,		offsetof( DIAGNOSTIC_IMAGE, Rows )						},
						{ " Columns",			ABSTRACT_LEVEL_IMAGE,	50,		offsetof( DIAGNOSTIC_IMAGE, Columns )					},
						{ " Aspect Ratio",		ABSTRACT_LEVEL_IMAGE,	40,		offsetof( DIAGNOSTIC_IMAGE, PixelAspectRatio )			},
						{ " Bits Alloc",		ABSTRACT_LEVEL_IMAGE,	40,		offsetof( DIAGNOSTIC_IMAGE, BitsAllocated )				},
						{ " Bits Stored",		ABSTRACT_LEVEL_IMAGE,	40,		offsetof( DIAGNOSTIC_IMAGE, BitsStored )				},
						{ " High Bit",			ABSTRACT_LEVEL_IMAGE,	40,		offsetof( DIAGNOSTIC_IMAGE, HighBit )					},
						{ " Pix Repr",			ABSTRACT_LEVEL_IMAGE,	40,		offsetof( DIAGNOSTIC_IMAGE, PixelRepresentation )		},
						{ " Window Center",		ABSTRACT_LEVEL_IMAGE,	60,		offsetof( DIAGNOSTIC_IMAGE, WindowCenter )				},
						{ " Window Width",		ABSTRACT_LEVEL_IMAGE,	60,		offsetof( DIAGNOSTIC_IMAGE, WindowWidth )				},
						{ " Manufacturer",		ABSTRACT_LEVEL_SERIES,	40,		offsetof( DIAGNOSTIC_SERIES, Manufacturer )				},
						{ " SOP Instance UID",	ABSTRACT_LEVEL_IMAGE,	350,	offsetof( DIAGNOSTIC_IMAGE, SOPInstanceUID )			}
					} };



// Sorting is based on the values of ListCtrlItem.lParam = (DWORD_PTR)&pStudy -> m_PatientLastName.
// This needs to be changed to ListCtrlItem.pszText = ListItemText.

// The first two parameters are the row numbers of the items to be sorted.
static int CALLBACK TextColumnSortComparator( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
{
	int					nRow1;
	int					nRow2;
	int					ItemDifference = 0;			// *[2] Initialize return value.
	char				Text1[ 256 ];
	char				Text2[ 256 ];

	CStudySelector* pStudySelector = (CStudySelector*) lParamSort;
	if ( pStudySelector != 0 )
		{
		nRow1 = (int)lParam1;
		nRow2 = (int)lParam2;
		pStudySelector -> GetItemText( nRow1, pStudySelector -> m_nColumnToSort, Text1, 255 );		// *[2] Eliminated unreferenced returned text length.
		pStudySelector -> GetItemText( nRow2, pStudySelector -> m_nColumnToSort, Text2, 255 );		// *[2] Eliminated unreferenced returned text length.
	
		
		if ( bSortAscending[ pStudySelector -> m_nColumnToSort ] )
			ItemDifference = _stricmp( Text1, Text2 );
		else
			ItemDifference = _stricmp( Text2, Text1 );
		}

	return ItemDifference;
}


void CStudySelector::UpdatePatientList()
{
	int						nItemIndex = 0;					// *[2] Initialized count.
	int						nColumn;
	LIST_ELEMENT			*pPatientListElement;
	char					ListItemText[ 2048 ];
	char					DateText[ 2048 ];
	LVITEM					ListCtrlItem;
	CHeaderCtrl				*pHdrCtrl;
	HDITEM					HeaderItem;
	int						HeaderItemCount;
	long					nImage;
	CStudy					*pStudy;
	char					*pListItemFieldValue;
	SYSTEMTIME				*pDate;
	LIST_COLUMN_FORMAT		*pColumnFormat;
	char					*pDataStructure = 0;			// [2] Initialized pointer.
	DIAGNOSTIC_STUDY		*pDiagnosticStudy;
	DIAGNOSTIC_SERIES		*pDiagnosticSeries;
	DIAGNOSTIC_IMAGE		*pDiagnosticImage;
	BOOL					bListSortedOK;
	BOOL					bAssignStudyToCurrentReader;
	BOOL					bStudyAetitleMatchesSomeReader;
	LIST_ELEMENT			*pUserListElement;
	READER_PERSONAL_INFO	*pSomeReaderInfo;

	EnableWindow( FALSE );
	DeleteAllItems();

	pHdrCtrl = GetHeaderCtrl();
	HeaderItemCount = pHdrCtrl -> GetItemCount();
	// Delete all of the current header items.
	for ( nColumn = HeaderItemCount - 1; nColumn >= 0; nColumn-- )
		DeleteColumn( nColumn );

	// Set up the data columns.
	switch ( pBViewerCustomization -> m_StudyInformationDisplayEmphasis )
		{
		case INFO_EMPHASIS_PATIENT:
			m_pListFormat = &PatientEmphasisListFormat;
			break;
		case INFO_EMPHASIS_STUDY:
			m_pListFormat = &StudyEmphasisListFormat;
			break;
		case INFO_EMPHASIS_SERIES:
			m_pListFormat = &SeriesEmphasisListFormat;
			break;
		case INFO_EMPHASIS_IMAGE:
			m_pListFormat = &ImageEmphasisListFormat;
			break;
		}
	// Build the header structure.
	for ( nColumn = 0; nColumn < (int)m_pListFormat -> nColumns; nColumn++ )
		{
		pColumnFormat = &m_pListFormat -> ColumnFormatArray[ nColumn ];
		InsertColumn( nColumn, pColumnFormat -> pColumnTitle, LVCFMT_LEFT, pColumnFormat -> ColumnWidth, nColumn );
		memset( &HeaderItem, 0, sizeof( HDITEM ));
		HeaderItem.mask = HDI_FORMAT;
		HeaderItem.fmt =  HDF_LEFT | HDF_STRING | HDF_OWNERDRAW;
		pHdrCtrl -> SetItem( nColumn, &HeaderItem );
		}
	// Populate the subject study list display.
	pPatientListElement = ThisBViewerApp.m_AvailableStudyList;
	nImage = 0;
	while( pPatientListElement != 0 )
		{
		pStudy = (CStudy*)pPatientListElement -> pItem;
		if ( pStudy != 0 )
			{
			bAssignStudyToCurrentReader = FALSE;
			// Studies not sent over the network will not have an AE_TITLE specified.  Automatically
			// assign these to the current user.
			if ( strlen( pStudy -> m_ReaderAddressed ) == 0 && strlen( pBViewerCustomization -> m_ReaderInfo.ReportSignatureName ) > 0 )				// *[4]
				{
				strncpy_s( pStudy -> m_ReaderAddressed, DICOM_ATTRIBUTE_STRING_LENGTH, pBViewerCustomization -> m_ReaderInfo.AE_TITLE, _TRUNCATE );		// *[4], *[1] Replaced strcpy with strncpy_s.
				bAssignStudyToCurrentReader = TRUE;
				}
			else if ( strlen( pBViewerCustomization -> m_ReaderInfo.ReportSignatureName ) == 0 || _stricmp( pStudy -> m_ReaderAddressed, pBViewerCustomization -> m_ReaderInfo.AE_TITLE ) == 0 )	// *[4]
				bAssignStudyToCurrentReader = TRUE;
			else
				{
				bStudyAetitleMatchesSomeReader = FALSE;
				pUserListElement = RegisteredUserList;
				while ( pUserListElement != 0 && !bStudyAetitleMatchesSomeReader )
					{
					pSomeReaderInfo = (READER_PERSONAL_INFO*)pUserListElement -> pItem;
					bStudyAetitleMatchesSomeReader = ( _stricmp( pStudy -> m_ReaderAddressed, pSomeReaderInfo -> AE_TITLE ) == 0 );
					if ( !bStudyAetitleMatchesSomeReader )
						pUserListElement = pUserListElement -> pNextListElement;
					}
				if ( !bStudyAetitleMatchesSomeReader )
					bAssignStudyToCurrentReader = TRUE;
				}
			if ( bAssignStudyToCurrentReader )
				{
				pDiagnosticStudy = pStudy -> m_pDiagnosticStudyList;
				while ( pDiagnosticStudy != 0 )
					{
					pDiagnosticSeries = pDiagnosticStudy -> pDiagnosticSeriesList;
					while ( pDiagnosticSeries != 0 )
						{
						pDiagnosticImage = pDiagnosticSeries -> pDiagnosticImageList;
						while ( pDiagnosticImage != 0 )
							{
							// Create a selection list row for this image.
							for ( nColumn = 0; nColumn < (int)m_pListFormat -> nColumns; nColumn++ )			// *[2] Removed unnecessary error test.
								{
								pColumnFormat = &m_pListFormat -> ColumnFormatArray[ nColumn ];
								memset( &ListCtrlItem, 0, sizeof( LVITEM ) );
								ListCtrlItem.mask = LVIF_TEXT | LVIF_PARAM;
								if ( nColumn != 0 )
									{
									ListCtrlItem.iSubItem = nColumn;
									ListCtrlItem.iItem = nItemIndex;
									}
								ListCtrlItem.cchTextMax = pColumnFormat -> ColumnWidth / 3;
								switch ( pColumnFormat -> DatabaseHierarchyLevel )
									{
									case ABSTRACT_LEVEL_PATIENT:
										pDataStructure = (char*)pStudy;
										break;
									case ABSTRACT_LEVEL_STUDY:
										pDataStructure = (char*)pDiagnosticStudy;
										break;
									case ABSTRACT_LEVEL_SERIES:
										pDataStructure = (char*)pDiagnosticSeries;
										break;
									case ABSTRACT_LEVEL_IMAGE:
										pDataStructure = (char*)pDiagnosticImage;
										break;
									}
								pListItemFieldValue = (char*)( pDataStructure + pColumnFormat -> DataStructureOffset );
								if ( strcmp( pColumnFormat -> pColumnTitle, " Birth Date" ) == 0 )
									{
									// bDateHasBeenEdited is also set if a non-blank value was read from the Dicom data element.
									if ( ( (EDITED_DATE*)pListItemFieldValue ) -> bDateHasBeenEdited )
										{
										pDate = &( (EDITED_DATE*)pListItemFieldValue ) -> Date;
										_snprintf_s( ListItemText, 2048, _TRUNCATE, "%2u/%2u/%4u", pDate -> wMonth, pDate -> wDay, pDate -> wYear );	// *[2] Replaced sprintf() with _snprintf_s.
										}
									else
										strncpy_s( ListItemText, 2048, "  /  /    ", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
									}
								else if ( strcmp( pColumnFormat -> pColumnTitle, " Date Read" ) == 0 )
									{
									pDate = &( (EDITED_DATE*)pListItemFieldValue ) -> Date;
									if ( pDate -> wYear > 1900 )
										_snprintf_s( ListItemText, 2048, _TRUNCATE, "%4u/%2u/%2u %2u:%2u:%2u", pDate -> wYear, pDate -> wMonth, pDate -> wDay,	// *[2] Replaced sprintf() with _snprintf_s.
																							pDate -> wHour, pDate -> wMinute, pDate -> wSecond );
									else
										strncpy_s( ListItemText, 2048, "  /  /    ", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
									}
								else if ( strcmp( pColumnFormat -> pColumnTitle, " Study Date" ) == 0 )
									{
									strncpy_s( DateText, 2048, (char*)( pDataStructure + pColumnFormat -> DataStructureOffset ), _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
									if ( strlen( DateText ) > 0 )
										{
										_snprintf_s( ListItemText, 2048, _TRUNCATE, "%.4s/%.2s/%.2s", DateText, &DateText[ 4 ], &DateText[ 6 ] );	// *[2] Replaced sprintf() with _snprintf_s.
										}
									else
										strncpy_s( ListItemText, 2048, "  /  /    ", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
									}
								else
									strncpy_s( ListItemText, 2048, (char*)( pDataStructure + pColumnFormat -> DataStructureOffset ), _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.

								ListCtrlItem.pszText = ListItemText;
								// Specify the sorting parameter.
								ListCtrlItem.lParam = (DWORD_PTR)&pStudy -> m_PatientLastName;
								if ( nColumn == 0 )
									nItemIndex = InsertItem( &ListCtrlItem );			// Insert a new row item.
								else
									SetItemText( nItemIndex, nColumn, ListItemText );	// Modify this row to specify data for another column.
								}			// ...loop to next column for this selection list row.
							if ( pStudy -> m_bStudyHasBeenEdited )
								SetCheck( nImage, TRUE );
							nImage++;
							pDiagnosticImage = pDiagnosticImage -> pNextDiagnosticImage;
							}
						pDiagnosticSeries = pDiagnosticSeries -> pNextDiagnosticSeries;
						}
					pDiagnosticStudy = pDiagnosticStudy -> pNextDiagnosticStudy;
					}
				}
			}
		pPatientListElement = pPatientListElement -> pNextListElement;
		}
	// Sort the items according to the selected column.
	bListSortedOK = SortItemsEx( TextColumnSortComparator, (LPARAM)this );
	if ( !bListSortedOK )																			// *[2] Added error response.
		LogMessage( "An error occurred sorting the patient list.", MESSAGE_TYPE_SUPPLEMENTARY );	// *[2]

	if (  m_nCurrentlySelectedItem >= 0 )
		{
		SetItemState( m_nCurrentlySelectedItem, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED );
		EnsureVisible( m_nCurrentlySelectedItem, 0 );
		}
	EnableWindow( TRUE );
}


// This provides an essential CListCtrl function that Microsoft omitted.
int CStudySelector::GetCurrentlySelectedItem()
{
	int				nItem;
	int				nItems;
	int				nSelectedItem;
	BOOL			bSelectedItemFound;
	unsigned int	ItemState;

	bSelectedItemFound = FALSE;
	nSelectedItem = -1;
	nItem = 0;
	nItems = GetItemCount();
	while ( nItem < nItems && !bSelectedItemFound )
		{
		ItemState = GetItemState( nItem, LVIS_SELECTED );
		if ( ItemState == LVIS_SELECTED )
			{
			bSelectedItemFound = TRUE;
			nSelectedItem = nItem;
			}
		nItem++;
		}

	return nSelectedItem;
}


void CStudySelector::AutoSelectPatientItem( char *pSelectedSOPInstanceUID )
{
	BOOL					bNoError;
	BOOL					bMatchingImageFound;
	BOOL					bSelectFirstItem;
	int						nListItem;
	int						nListItems;
	int						nSelectedItem;
	CString					SOPInstanceUIDString;
	RECT					SelectedItemRectangle;
	RECT					SelectionListRectangleInScreenCoordinates;
	double					ScreenWidth;
	double					ScreenHeight;
	double					MouseX;
	double					MouseY;
	INPUT					MouseInputSpecification;
	unsigned				nMouseEventsGenerated;
	char					Msg[ FILE_PATH_STRING_LENGTH ];

	sprintf_s( Msg, FILE_PATH_STRING_LENGTH, "Automatically selecting image for viewing:  %s", pSelectedSOPInstanceUID );	// *[1] Replaced sprintf with sprintf_s.
	LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
	bSelectFirstItem = ( pSelectedSOPInstanceUID == 0 );
	// Find the matching item from the list.
	nListItem = 0;
	nListItems = GetItemCount();
	bMatchingImageFound = FALSE;
	while ( nListItem < nListItems && !bMatchingImageFound )
		{
		if ( bSelectFirstItem )
			{
			bMatchingImageFound = TRUE;
			nListItem = 0;
			}
		else
			{
			SOPInstanceUIDString = GetItemText( nListItem, m_pListFormat -> nColumns - 1 );
			if ( strcmp( (const char*)SOPInstanceUIDString, pSelectedSOPInstanceUID ) == 0 )
				{
				bMatchingImageFound = TRUE;
				}
			}
		if ( bMatchingImageFound )
			{
			nSelectedItem = GetCurrentlySelectedItem();
			if ( nSelectedItem >= 0 )	// If there is a current selection
				{
				// Deselect the currently selected image.
				SetItemState( nSelectedItem, 0, LVIS_SELECTED );
				}
			// Select the matched item.
			SetItemState( nListItem, LVIS_SELECTED, LVIS_SELECTED );
			// This function is on the wrong thread for viewing the image.  To engage the
			// viewing, Send a message that this item has been clicked.
			bNoError = GetItemRect( nListItem, &SelectedItemRectangle, LVIR_BOUNDS );
			if ( bNoError )
				{
				GetWindowRect( &SelectionListRectangleInScreenCoordinates );
				OffsetRect( &SelectedItemRectangle, SelectionListRectangleInScreenCoordinates.left, SelectionListRectangleInScreenCoordinates.top );
				sprintf_s( Msg, FILE_PATH_STRING_LENGTH, "Selected item %d rectangle:  %d, %d, %d, %d", nListItem, SelectedItemRectangle.left,
															SelectedItemRectangle.top, SelectedItemRectangle.right, SelectedItemRectangle.bottom );	// *[1] Replaced sprintf with sprintf_s.
				LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
				ScreenWidth    = ::GetSystemMetrics( SM_CXSCREEN ) - 1; 
				ScreenHeight  = ::GetSystemMetrics( SM_CYSCREEN ) - 1; 
				// Generate a simulated mouse click in this rectangle.
				MouseX = ( SelectedItemRectangle.left + 100 ) * (65535.0 / ScreenWidth );
				MouseY = ( SelectedItemRectangle.top + ( SelectedItemRectangle. bottom - SelectedItemRectangle.top ) / 2.0 ) * (65535.0 / ScreenHeight );
				_snprintf_s( Msg, FILE_PATH_STRING_LENGTH, _TRUNCATE, "Calculated mouse hit:  %d, %d", (int)MouseX, (int)MouseY );	// *[2] Replaced sprintf() with _snprintf_s.
				LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
				// Move the mouse to the specified point.
				MouseInputSpecification.type = INPUT_MOUSE;
				MouseInputSpecification.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
				MouseInputSpecification.mi.dx = (long)MouseX;
				MouseInputSpecification.mi.dy = (long)MouseY;
				nMouseEventsGenerated = ::SendInput( 1, &MouseInputSpecification, sizeof(INPUT) );
				if ( nMouseEventsGenerated != 1 )
					LogMessage( "Mouse move error.", MESSAGE_TYPE_SUPPLEMENTARY );
				// Simulate a left button press.
				MouseInputSpecification.type = INPUT_MOUSE;
				MouseInputSpecification.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
				nMouseEventsGenerated = ::SendInput( 1, &MouseInputSpecification, sizeof(INPUT) );
				if ( nMouseEventsGenerated != 1 )
					LogMessage( "Mouse left button down error.", MESSAGE_TYPE_SUPPLEMENTARY );
				// Simulate a left button release.
				MouseInputSpecification.type = INPUT_MOUSE;
				MouseInputSpecification.mi.dwFlags = MOUSEEVENTF_LEFTUP;
				nMouseEventsGenerated = ::SendInput( 1, &MouseInputSpecification, sizeof(INPUT) );
				if ( nMouseEventsGenerated != 1 )
					LogMessage( "Mouse left button release error.", MESSAGE_TYPE_SUPPLEMENTARY );
				Invalidate();
				UpdateWindow();
				}
			}
		nListItem++;
		}
}


void CStudySelector::OnPatientItemSelected()
{
	LIST_ELEMENT			*pAvailableStudyListElement;
	DIAGNOSTIC_STUDY		*pStudyDataRow;
	DIAGNOSTIC_SERIES		*pSeriesDataRow;
	DIAGNOSTIC_IMAGE		*pImageDataRow;
	POSITION				SelectedItemPosition;
	int						nSelectedItem;
	char					ImagePath[ FILE_PATH_STRING_LENGTH ];
	char					ImageFileName[ FILE_PATH_STRING_LENGTH ];
	char					ImageFileExtension[ FILE_PATH_STRING_LENGTH ];
 	CMainFrame				*pMainFrame;
	CImageFrame				*pStudyImageFrame;
	CStudy					*pStudy;
	BOOL					bDataWereEnteredManually;
	BOOL					bMatchingDicomFileFound;
	CString					SubitemText;
	WINDOWPLACEMENT			WindowPlacement;
	char					SubjectName[ MAX_LOGGING_STRING_LENGTH ];
	CEdit					*pCtrlFileName;
	char					*pFirstName;
	char					Msg[ FULL_FILE_SPEC_STRING_LENGTH ];

	bMatchingDicomFileFound = FALSE;
	pMainFrame = 0;
	LogMessage( "A study has been selected.", MESSAGE_TYPE_SUPPLEMENTARY );
	SelectedItemPosition = GetFirstSelectedItemPosition();
	if ( SelectedItemPosition == 0 )
		nSelectedItem = -1;
	else
		{
		nSelectedItem = GetNextSelectedItem( SelectedItemPosition );
		m_nCurrentlySelectedItem = nSelectedItem;
		}
	if ( nSelectedItem >= 0 )
		{
		// Locate the matching image file.
		strncpy_s( ImagePath, FILE_PATH_STRING_LENGTH, BViewerConfiguration.ImageDirectory, _TRUNCATE );	// *[2] Replaced strncat with strncpy_s.
		if ( ImagePath[ strlen( ImagePath ) - 1 ] != '\\' )
			strncat_s( ImagePath, FILE_PATH_STRING_LENGTH, "\\", _TRUNCATE );								// *[2] Replaced strcat with strncat_s.
		SubitemText = GetItemText( nSelectedItem, m_pListFormat -> nColumns - 1 );
		pAvailableStudyListElement = ThisBViewerApp.m_AvailableStudyList;
		while ( pAvailableStudyListElement != 0 && !bMatchingDicomFileFound )
			{
			pStudy = (CStudy*)pAvailableStudyListElement -> pItem;
			if ( pStudy != 0 )
				{
				ThisBViewerApp.m_pCurrentStudy = pStudy;
				pStudyDataRow = pStudy -> m_pDiagnosticStudyList;
				while ( pStudyDataRow != 0 && !bMatchingDicomFileFound )
					{
					pSeriesDataRow = pStudyDataRow -> pDiagnosticSeriesList;
					while ( pSeriesDataRow != 0 && !bMatchingDicomFileFound )
						{
						pImageDataRow = pSeriesDataRow -> pDiagnosticImageList;
						while ( pImageDataRow != 0 && !bMatchingDicomFileFound )
							{
							if ( strcmp( (const char*)SubitemText, pImageDataRow -> SOPInstanceUID ) == 0 )
								{
								// The selected study has been identified.
								bMatchingDicomFileFound = TRUE;
								bDataWereEnteredManually = ( strcmp( pStudyDataRow -> StudyDescription, "Manual Data Entry   No Image" ) == 0 );
								pStudy -> m_pCurrentStudyInfo = pStudyDataRow;
								pStudy -> m_pCurrentSeriesInfo = pSeriesDataRow;
								pStudy -> m_pCurrentImageInfo = pImageDataRow;
								strncpy_s( ImageFileName, FILE_PATH_STRING_LENGTH, pImageDataRow -> SOPInstanceUID, _TRUNCATE );	// *[2] Replaced strncat with strncpy_s.
								strncpy_s( ImageFileExtension, FILE_PATH_STRING_LENGTH, ".png", _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.
								pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
								if ( pMainFrame != 0 )
									{
									// Display the study image.
									pStudyImageFrame = pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ];
									if ( pStudyImageFrame != 0 )
										{
										if ( bDataWereEnteredManually )		// There is no image for manually entered data.  Don't generate an error by trying to read a file.
											{
											pCtrlFileName = (CEdit*)pStudyImageFrame -> m_wndDlgBar.GetDlgItem( IDC_EDIT_IMAGE_NAME );
											pFirstName = ( (CStudy*)pStudy ) -> m_PatientFirstName;
											strncpy_s( SubjectName, MAX_LOGGING_STRING_LENGTH, ( (CStudy*)pStudy ) -> m_PatientLastName, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
											if ( strlen( ( (CStudy*)pStudy ) -> m_PatientLastName ) > 0 && strlen( ( (CStudy*)pStudy ) -> m_PatientFirstName ) > 0 )
												strncat_s( SubjectName, MAX_LOGGING_STRING_LENGTH, ", ", _TRUNCATE );										// *[3] Replaced strcat with strncat_s.
											strncat_s( SubjectName, MAX_LOGGING_STRING_LENGTH, ( (CStudy*)pStudy ) -> m_PatientFirstName, _TRUNCATE );		// *[3] Replaced strcat with strncat_s.
											_snprintf_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, _TRUNCATE,
														"   ********   Subject study file for %s selected for viewing.", SubjectName );						// *[2] Replaced sprintf() with _snprintf_s.
											LogMessage( Msg, MESSAGE_TYPE_NORMAL_LOG );
											pCtrlFileName -> SetWindowText( SubjectName );
											pMainFrame -> m_wndDlgBar.m_EditImageName.SetWindowText( SubjectName );
											pMainFrame -> m_pImageFrame[ 2 ] -> m_wndDlgBar.m_EditImageName.SetWindowText( SubjectName );
											}
										else
											{
											CWaitCursor			DisplaysHourglass;

											if (  m_nCurrentlySelectedItem >= 0 )
												{
												SetItemState( m_nCurrentlySelectedItem, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED );
												LogMessage( "The study has been marked as selected.", MESSAGE_TYPE_SUPPLEMENTARY );
												}
											LogMessage( "The study image is being activated.", MESSAGE_TYPE_SUPPLEMENTARY );
											if ( !BViewerConfiguration.bAutoGeneratePDFReportsFromAXTFiles )
												{
												pStudyImageFrame -> OnSelectImage( ThisBViewerApp.m_pCurrentStudy, ImagePath, ImageFileName, ImageFileExtension );
												// If the Study image window is minimized, restore it to normal viewing.
												pStudyImageFrame -> GetWindowPlacement( &WindowPlacement );
												if ( WindowPlacement.showCmd == SW_SHOWMINIMIZED )
													{
													WindowPlacement.showCmd = SW_SHOWNORMAL;
													pStudyImageFrame -> SetWindowPlacement( &WindowPlacement );
													}
												if ( strlen( pStudy -> m_TimeStudyFirstOpened ) == 0 )
													{
													GetDateAndTimeForFileName( pStudy -> m_TimeStudyFirstOpened, 32 );
													pStudy -> m_TimeStudyFirstOpened[ strlen( pStudy -> m_TimeStudyFirstOpened ) - 1 ] = '\0';
													}
												}
											}
										}
									}
								}
							pImageDataRow = pImageDataRow -> pNextDiagnosticImage;
							}
						pSeriesDataRow = pSeriesDataRow -> pNextDiagnosticSeries;
						}
					pStudyDataRow = pStudyDataRow -> pNextDiagnosticStudy;
					}	
				}
			pAvailableStudyListElement = pAvailableStudyListElement -> pNextListElement;
			}
		}

	if ( bMatchingDicomFileFound && pMainFrame != 0 )
		{
		if ( pStudy -> m_bStudyWasPreviouslyInterpreted )
			{
			memcpy( &BViewerConfiguration.m_ClientInfo, &pStudy -> m_ClientInfo, sizeof(CLIENT_INFO) );
			memcpy( &pBViewerCustomization -> m_ReaderInfo, &pStudy -> m_ReaderInfo, sizeof( READER_PERSONAL_INFO ) );
			}
		else
			{
			// Restore the displayed reader info for new studies.  (It can be overwritten by imported .AXT file studies.)
			memcpy( &pBViewerCustomization -> m_ReaderInfo, &LoggedInReaderInfo, sizeof( READER_PERSONAL_INFO ) );
			}
		if ( BViewerConfiguration.bAutoGeneratePDFReportsFromAXTFiles )
			{
			// A study has been newly selected.  Activate the report tab on the control panel.
			if ( pMainFrame -> m_pControlPanel != 0 )
				pMainFrame -> m_pControlPanel -> SetActivePage( REPORT_PAGE );
			}
		else if ( bMatchingDicomFileFound && BViewerConfiguration.bEnableAutoAdvanceToInterpretation )
			{
			// A study has been newly selected.  Activate the Interpretation tab on the control panel.
			if ( pMainFrame -> m_pControlPanel != 0 )
				pMainFrame -> m_pControlPanel -> SetActivePage( INTERPRETATION_PAGE );
			}
		}

}


void CStudySelector::ResetColumnWidth( int nColumn, int NewWidth )
{
	CHeaderCtrl				*pHdrCtrl;
	LIST_COLUMN_FORMAT		*pColumnFormat;

	pHdrCtrl = GetHeaderCtrl();
	if ( pHdrCtrl != 0 )
		{
		pColumnFormat = &m_pListFormat -> ColumnFormatArray[ nColumn ];
		if ( pColumnFormat != 0 )
			pColumnFormat -> ColumnWidth = NewWidth;
		}
}


BOOL CStudySelector::OnEraseBkgnd( CDC *pDC )
{
	CBrush		BackgroundBrush( COLOR_ANALYSIS_BKGD );
	CRect		BackgroundRectangle;
	CBrush		*pOldBrush = pDC -> SelectObject( &BackgroundBrush );

	GetClientRect( BackgroundRectangle );
	pDC -> FillRect( BackgroundRectangle, &BackgroundBrush );
	pDC -> SelectObject( pOldBrush );

	return TRUE;
}


void CStudySelector::OnHeaderClick( NMLISTVIEW *pListViewNotification, LRESULT *pResult )
{

	if ( pListViewNotification -> hdr.code == HDN_ITEMCLICK )
		{
		m_nColumnToSort = pListViewNotification -> iItem;
		if ( m_nColumnToSort >= 0 && m_nColumnToSort < m_SelectorHeading.GetItemCount() )
			{
			// Toggle the sort order on the selected column.
			bSortAscending[ m_nColumnToSort ] = !bSortAscending[ m_nColumnToSort ];
			UpdatePatientList();
			}
		}
}


// Intercept a Windows message that would otherwise go to the parent wincow.
void CStudySelector::OnNMClick( NMHDR *pNMHDR, LRESULT *pResult )
{
	LPNMLISTVIEW		lpnmlv;
	RECT				ColumnHeaderRect;
	CHeaderCtrl			*pHdrCtrl;
	HDHITTESTINFO		HitTestInfo;
	int					nHeaderColumnClicked;

	lpnmlv = (LPNMLISTVIEW)pNMHDR;
	m_SelectorHeading.GetItemRect( 0, &ColumnHeaderRect );
	if ( lpnmlv != 0 && lpnmlv -> ptAction.y < ColumnHeaderRect.bottom )
		{
		pHdrCtrl = GetHeaderCtrl();
		HitTestInfo.pt.x = lpnmlv -> ptAction.x;
		HitTestInfo.pt.y = lpnmlv -> ptAction.y;
		HitTestInfo.flags = 0;
		nHeaderColumnClicked = pHdrCtrl -> HitTest( &HitTestInfo );
		if ( nHeaderColumnClicked != -1 )
			m_nColumnToSort = nHeaderColumnClicked;
		}
	if ( lpnmlv != 0 && lpnmlv -> ptAction.x > 22 )
		OnPatientItemSelected();

	*pResult = 0;
}



void CStudySelector::OnHdnEndtrack( NMHDR *pNMHDR, LRESULT *pResult )
{
	NMHEADER			*pHdr = reinterpret_cast<LPNMHEADER>( pNMHDR );
	int					nItemAffected;
	HDITEM				*pAffectedItem;
	int					NewWidth;

	if ( pHdr != 0 )
		{
		nItemAffected = pHdr -> iItem;
		if ( pHdr -> iButton == 0 )		// If tracking the left mouse button...
			{
			pAffectedItem = pHdr -> pitem;
			if ( pAffectedItem != 0 && ( pAffectedItem -> mask & HDI_WIDTH ) != 0 )
				{
				NewWidth = pAffectedItem -> cxy;
				// Adjust the selector column width.
				ResetColumnWidth( nItemAffected, NewWidth );
				}
			}
		}

	*pResult = 0;
}


