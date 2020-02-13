// ReportSelector.cpp : Implementation file for the CReportSelector
//  class of CListCtrl, which implements the Report selection list.
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
#include "BViewer.h"
#include "Module.h"
#include "ReportStatus.h"
#include "DiagnosticImage.h"
#include "Mouse.h"
#include "ImageView.h"
#include "MainFrm.h"
#include "ReportSelector.h"


extern CONFIGURATION				BViewerConfiguration;

// CReportSelector
CReportSelector::CReportSelector()
{
	m_pFirstReport = 0;
	m_nCurrentlySelectedItem = -1;
}

CReportSelector::~CReportSelector()
{
	EraseReportList();
}


BEGIN_MESSAGE_MAP( CReportSelector, CListCtrl )
	//{{AFX_MSG_MAP(CReportSelector)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


int CReportSelector::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	SetBkColor( COLOR_REPORT_SELECTOR_BKGD );
	if ( CListCtrl::OnCreate( lpCreateStruct ) == -1 )
		return -1;

	m_ReportSelectorHeading.SubclassHeaderCtrl( GetHeaderCtrl() );

	return 0;
}


BOOL CReportSelector::OnEraseBkgnd( CDC *pDC )
{
	CBrush		BackgroundBrush( COLOR_REPORT_SELECTOR_BKGD );
	CRect		BackgroundRectangle;
	CBrush		*pOldBrush = pDC -> SelectObject( &BackgroundBrush );

	GetClientRect( BackgroundRectangle );
	pDC -> FillRect( BackgroundRectangle, &BackgroundBrush );
	pDC -> SelectObject( pOldBrush );

	return TRUE;
}


void CReportSelector::EraseReportList()
{
	REPORT_INFO				*pReportInfo;
	REPORT_INFO				*pPrevReportInfo;

	pReportInfo = m_pFirstReport;
	while ( pReportInfo != 0 )
		{
		pPrevReportInfo = pReportInfo;
		pReportInfo = pReportInfo -> pNextReportInfo;
		free( pPrevReportInfo );
		}
	m_pFirstReport = 0;
}


void CReportSelector::CreateReportList()
{
	BOOL					bNoError = TRUE;
	char					ReportDirectory[ FILE_PATH_STRING_LENGTH ];
	char					SearchFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					FoundFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					FoundFileName[ FULL_FILE_SPEC_STRING_LENGTH ];
	WIN32_FIND_DATA			FindFileInfo;
	HANDLE					hFindFile;
	BOOL					bFileFound;
	REPORT_INFO				*pLastReportInfoInList;
	REPORT_INFO				*pNewReportInfo;
	char					*pChar;
	int						nChar;

	// Add the edited studies to the study list by reading the saved study data files.
	strcpy( ReportDirectory, "" );
	strncat( ReportDirectory, BViewerConfiguration.ReportDirectory, FILE_PATH_STRING_LENGTH );
	if ( ReportDirectory[ strlen( ReportDirectory ) - 1 ] != '\\' )
		strcat( ReportDirectory, "\\" );
	// Check existence of path to configuration directory.
	bNoError = SetCurrentDirectory( ReportDirectory );
	if ( bNoError )
		{
		pLastReportInfoInList = 0;
		EraseReportList();
		strcpy( SearchFileSpec, ReportDirectory );
		strcat( SearchFileSpec, "*ReportPage1.png" );
		hFindFile = FindFirstFile( SearchFileSpec, &FindFileInfo );
		bFileFound = ( hFindFile != INVALID_HANDLE_VALUE );
		while ( bFileFound )
			{
			strcpy( FoundFileSpec, ReportDirectory );
			strncat( FoundFileSpec, FindFileInfo.cFileName,
							FULL_FILE_SPEC_STRING_LENGTH - strlen( ReportDirectory ) - 1 );
			strcpy( FoundFileName, FindFileInfo.cFileName );
			pNewReportInfo = (REPORT_INFO*)malloc( sizeof(REPORT_INFO) );
			if ( pNewReportInfo != 0 )
				{
				// Terminate the string at the end of the time field.
				pChar = strstr( FoundFileName, "__ReportPage1" );
				if ( pChar != 0 )
					{
					*pChar = '\0';
					strcpy( pNewReportInfo -> ReportFileName, FoundFileName );
					pChar -= 6;		// Back up six characters to point to the time field.
					strcpy( pNewReportInfo -> Time, pChar );
					pChar--;
					*pChar = '\0';
					pChar -= 8;		// Back up eight characters to point to the date field.
					strcpy( pNewReportInfo -> Date, pChar );
					pChar--;
					*pChar = '\0';			// Terminate the name field.
					strcpy( pNewReportInfo -> SubjectLastName, "" );
					strcpy( pNewReportInfo -> SubjectFirstName, "" );
					pChar = strrchr( FoundFileName, '-' );
					if ( pChar != 0 )
						{
						nChar = (int)( pChar - FoundFileName );
						strncat( pNewReportInfo -> SubjectLastName, FoundFileName, nChar );
						pChar++;
						strncat( pNewReportInfo -> SubjectFirstName, pChar, 63 );
						strcpy( pNewReportInfo -> SubjectName, pNewReportInfo -> SubjectLastName );
						if ( strlen( pNewReportInfo -> SubjectLastName ) > 0 && strlen( pNewReportInfo -> SubjectFirstName ) > 0 )
							strcat( pNewReportInfo -> SubjectName, ", " );
						strcat( pNewReportInfo -> SubjectName, pNewReportInfo -> SubjectFirstName );
						}
					SubstituteCharacterInText( pNewReportInfo -> SubjectLastName, '_', ' ' );
					SubstituteCharacterInText( pNewReportInfo -> SubjectFirstName, '_', ' ' );
					pNewReportInfo -> pNextReportInfo = 0;
					// Link the new report to the list.
					if ( m_pFirstReport == 0 )
						m_pFirstReport = pNewReportInfo;
					else
						pLastReportInfoInList -> pNextReportInfo = pNewReportInfo;
					pLastReportInfoInList = pNewReportInfo;
					}
				else
					free( pNewReportInfo );
				}
			// Look for another file in the source directory.
			bFileFound = FindNextFile( hFindFile, &FindFileInfo );
			}
		if ( hFindFile != INVALID_HANDLE_VALUE )
			FindClose( hFindFile );
		}
}


REPORT_LIST_FORMAT		ReportListFormat =
							{ 3, {
								{ " Subject Name",	346,	offsetof( REPORT_INFO, SubjectName )},
								{ " Date",			100,	offsetof( REPORT_INFO, Date )		},
								{ " Time",			100,	offsetof( REPORT_INFO, Time )		}
							} };


static int CALLBACK TextColumnSortComparator( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
{
	int					ItemDifference;
	char				*pText1;
	char				*pText2;

	pText1 = (char*)lParam1;
	pText2 = (char*)lParam2;
	ItemDifference = _stricmp( pText1, pText2 );

	return ItemDifference;
}


void CReportSelector::UpdateReportListDisplay()
{
	BOOL						bNoError = TRUE;
	CHeaderCtrl					*pHdrCtrl;
	int							HeaderItemCount;
	int							nColumn;
	CRect						ClientRectangle;
	REPORT_LIST_COLUMN_FORMAT	*pColumnFormat;
	HDITEM						HeaderItem;
	REPORT_INFO					*pReportInfo;
	LVITEM						ListCtrlItem;
	int							nItemIndex;
	char						*pDataStructure;
	char						ListItemText[ 2048 ];
	char						*pText;

	DeleteAllItems();
	pHdrCtrl = GetHeaderCtrl();
	HeaderItemCount = pHdrCtrl -> GetItemCount();
	// Delete all of the current header items.
	for ( nColumn = HeaderItemCount - 1; nColumn >= 0; nColumn-- )
		DeleteColumn( nColumn );
	// Set up the data columns.
	m_pReportListFormat = &ReportListFormat;
	// Build the header structure.
	GetWindowRect( &ClientRectangle );
	for ( nColumn = 0; nColumn < (int)m_pReportListFormat -> nColumns; nColumn++ )
		{
		pColumnFormat = &m_pReportListFormat -> ColumnFormatArray[ nColumn ];
		InsertColumn( nColumn, pColumnFormat -> pColumnTitle, LVCFMT_LEFT, pColumnFormat -> ColumnWidth, nColumn );
		memset( &HeaderItem, 0, sizeof( HDITEM ));
		HeaderItem.mask = HDI_FORMAT;
		HeaderItem.fmt =  HDF_LEFT | HDF_STRING | HDF_OWNERDRAW;
		pHdrCtrl -> SetItem( nColumn, &HeaderItem );
		}
	// Populate the report list display.
	pReportInfo = m_pFirstReport;
	while ( pReportInfo != 0 )
		{
		bNoError = TRUE;
		for ( nColumn = 0; bNoError && nColumn < (int)m_pReportListFormat -> nColumns; nColumn++ )
			{
			pColumnFormat = &m_pReportListFormat -> ColumnFormatArray[ nColumn ];
			memset( &ListCtrlItem, 0, sizeof( LVITEM ) );
			ListCtrlItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE | LVIF_IMAGE;
			ListCtrlItem.stateMask = (unsigned int)-1;
			if ( nColumn != 0 )
				{
				ListCtrlItem.iSubItem = nColumn + 1;
				ListCtrlItem.iItem = nItemIndex;			// This value is defined for column 0.
				}
			ListCtrlItem.cchTextMax = pColumnFormat -> ColumnWidth / 3;
			pDataStructure = (char*)pReportInfo;
			pText = (char*)( pDataStructure + pColumnFormat -> DataStructureOffset );
			switch ( nColumn )
				{
				case 0:
					strcpy( ListItemText, pText );
					break;
				case 1:
					strcpy( ListItemText, "" );
					strncat( ListItemText, pText, 4 );
					strcat( ListItemText, "/" );
					strncat( ListItemText, pText + 4, 2 );
					strcat( ListItemText, "/" );
					strncat( ListItemText, pText + 6, 2 );
					break;
				case 2:
					strcpy( ListItemText, "" );
					strncat( ListItemText, pText, 2 );
					strcat( ListItemText, ":" );
					strncat( ListItemText, pText + 2, 2 );
					strcat( ListItemText, ":" );
					strncat( ListItemText, pText + 4, 2 );
					break;
				}
			ListCtrlItem.pszText = ListItemText;
			// Specify the sorting parameter.
			ListCtrlItem.lParam = (DWORD_PTR)&pReportInfo -> SubjectName;
			if ( nColumn == 0 )
				{
				nItemIndex = InsertItem( &ListCtrlItem );	// Insert a new row item.
				SetItemData( nItemIndex, (DWORD_PTR)pReportInfo );
				}
			else
				SetItemText( nItemIndex, nColumn, ListItemText );		// Modify this row to specify data for another column.
			}			// ...loop to next column for this selection list row.
		SetCheck( nItemIndex, FALSE );
		pReportInfo = pReportInfo -> pNextReportInfo;
		}
	SortItems( TextColumnSortComparator, (LPARAM)this );
}


void CReportSelector::UpdateSelectionList()
{
	CreateReportList();
	UpdateReportListDisplay();
}


void CReportSelector::OnReportItemSelected()
{
	POSITION				SelectedItemPosition;
	int						nSelectedItem;
	REPORT_INFO				*pSelectedReportInfo;

	pSelectedReportInfo = 0;
	SelectedItemPosition = GetFirstSelectedItemPosition();
	if ( SelectedItemPosition == 0 )
		nSelectedItem = -1;
	else
		{
		nSelectedItem = GetNextSelectedItem( SelectedItemPosition );
		m_nCurrentlySelectedItem = nSelectedItem;
		}
	if ( nSelectedItem >= 0 )
		pSelectedReportInfo = (REPORT_INFO*)GetItemData( nSelectedItem );
	if ( pSelectedReportInfo != 0 )
		{
		DisplaySelectedReportImage( pSelectedReportInfo );
		Invalidate();
		}
}


void CReportSelector::DisplaySelectedReportImage( REPORT_INFO *pSelectedReportInfo )
{
 	CMainFrame				*pMainFrame;
	CImageFrame				*pReportImageFrame;
	WINDOWPLACEMENT			WindowPlacement;
	BOOL					bUseCurrentStudy;

	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( pMainFrame != 0 )
		{
		pReportImageFrame = pMainFrame -> m_pImageFrame[ IMAGE_FRAME_REPORT ];
		if ( pReportImageFrame != 0 )
			{
			strcpy( pReportImageFrame -> m_CurrentReportFileName, pSelectedReportInfo -> ReportFileName );
			pReportImageFrame -> LoadReportPage( 1, &bUseCurrentStudy );
			}
		// If the report image window is minimized, restore it to normal viewing.
		pMainFrame -> m_pImageFrame[ IMAGE_FRAME_REPORT ] -> GetWindowPlacement( &WindowPlacement );
		if ( WindowPlacement.showCmd == SW_SHOWMINIMIZED )
			{
			WindowPlacement.showCmd = SW_SHOWNORMAL;
			pMainFrame -> m_pImageFrame[ IMAGE_FRAME_REPORT ] -> SetWindowPlacement( &WindowPlacement );
			}
		}
}


void CReportSelector::OnNMClick( NMHDR *pNMHDR, LRESULT *pResult )
{
	NMLISTVIEW		*pNMlv;

	pNMlv = (LPNMLISTVIEW)pNMHDR;
	if ( pNMlv -> ptAction.x > 22 )
		OnReportItemSelected();

	*pResult = 0;
}


