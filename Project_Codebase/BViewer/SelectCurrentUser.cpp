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
//	*[1] 07/31/2023 by Tom Atwood
//		Created this file.
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
#include "SelectCurrentUser.h"


extern CONFIGURATION			BViewerConfiguration;
extern CCustomization			BViewerCustomization;

extern LIST_HEAD				RegisteredUserList;



// CSelectCurrentUser dialog

CSelectCurrentUser::CSelectCurrentUser( CWnd *pParent /*=NULL*/ ) : CDialog( CSelectCurrentUser::IDD, pParent ),
				m_StaticReaderReportSignatureName( "Current Reader:", 200, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_READER_SIGNATURE_NAME ),
				m_EditReaderReportSignatureName( "", 280, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | EDIT_READONLY | CONTROL_VISIBLE,
										EDIT_VALIDATION_NONE, IDC_EDIT_READER_SIGNATURE_NAME ),

				m_StaticSelectReader( "Select a Different Reader", 200, 60, 14, 7, 6,
										COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_MULTILINE | CONTROL_VISIBLE,
										IDC_STATIC_READER_SELECTION_HELP_INFO ),

				m_ComboBoxSelectReader( "", 280, 300, 18, 9, 5, VARIABLE_PITCH_FONT,
										COLOR_BLACK, COLOR_UNTOUCHED_LIGHT, COLOR_COMPLETED_LIGHT, COLOR_TOUCHED,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_VSCROLL | EDIT_BORDER | LIST_SORT | CONTROL_VISIBLE,
										EDIT_VALIDATION_NONE, IDC_COMBO_SELECT_CURRENT_READER ),

				m_ButtonExit( "Continue", 150, 40, 16, 8, 6,
										COLOR_BLACK, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR,
										BUTTON_PUSHBUTTON | CONTROL_VISIBLE |
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
										IDC_BUTTON_EXIT_READER_SELECTION )

{
	m_BkgdBrush.CreateSolidBrush( COLOR_CONFIG );
	memset( (void*)&m_DefaultReaderInfo, 0, sizeof(READER_PERSONAL_INFO) );
}


CSelectCurrentUser::~CSelectCurrentUser()
{
	DestroyWindow();
}


BEGIN_MESSAGE_MAP( CSelectCurrentUser, CDialog )
	//{{AFX_MSG_MAP(CSelectCurrentUser)
	ON_CBN_SELENDOK( IDC_COMBO_SELECT_CURRENT_READER, OnReaderSelected )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_EXIT_READER_SELECTION, OnBnClickedExitReaderSelection )
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CSelectCurrentUser::OnInitDialog()
{
	static char		TextString[ 64 ];
	int				PrimaryScreenWidth;
	int				PrimaryScreenHeight;

	CDialog::OnInitDialog();

	m_StaticReaderReportSignatureName.SetPosition( 40, 10, this );
	m_EditReaderReportSignatureName.SetPosition( 40, 30, this );

	m_StaticSelectReader.SetPosition( 40, 60, this );
	m_ComboBoxSelectReader.SetPosition( 40, 100, this );

	m_ButtonExit.SetPosition( 110, 200, this );

	PrimaryScreenWidth = ::GetSystemMetrics( SM_CXSCREEN );
	PrimaryScreenHeight = ::GetSystemMetrics( SM_CYSCREEN );

	LoadReaderSelectionList();

	SetWindowPos( &wndTop, ( PrimaryScreenWidth - 370 ) / 2, ( PrimaryScreenHeight - 300 ) / 2, 370, 300, SWP_SHOWWINDOW );

	return TRUE; 
}


BOOL CSelectCurrentUser::LoadReaderSelectionList()
{
	BOOL					bNoError = TRUE;
	LIST_ELEMENT			*pReaderListElement;
	READER_PERSONAL_INFO	*pReaderInfo;
	int						nItemIndex = 0;				// *[3] Initialize variable.
	int						nSelectedItem;

	m_ComboBoxSelectReader.ResetContent();
	m_ComboBoxSelectReader.SetWindowTextA( "Registered Readers" );
	pReaderListElement = RegisteredUserList;
	nSelectedItem = 0;
	while ( pReaderListElement != 0 )
		{
		pReaderInfo = (READER_PERSONAL_INFO*)pReaderListElement -> pItem;
		nItemIndex = m_ComboBoxSelectReader.AddString( pReaderInfo -> ReportSignatureName );

		if ( pReaderInfo -> IsDefaultReader )
			{
			m_EditReaderReportSignatureName.SetWindowText( pReaderInfo -> ReportSignatureName );
			memcpy( &m_DefaultReaderInfo, pReaderInfo, sizeof( READER_PERSONAL_INFO ) );
			}

		if ( strcmp( pReaderInfo -> ReportSignatureName, BViewerCustomization.m_ReaderInfo.ReportSignatureName ) == 0 )
			nSelectedItem = nItemIndex;
		m_ComboBoxSelectReader.SetItemDataPtr( nItemIndex, (void*)pReaderInfo );
		pReaderListElement = pReaderListElement -> pNextListElement;
		}
	m_ComboBoxSelectReader.SetCurSel( nSelectedItem );

	return bNoError;
}


void CSelectCurrentUser::OnReaderSelected()
{
	READER_PERSONAL_INFO	*pReaderInfo;
	int						nItemIndex;

	nItemIndex = m_ComboBoxSelectReader.GetCurSel();
	m_nSelectedReaderItem = nItemIndex;
	pReaderInfo = (READER_PERSONAL_INFO*)m_ComboBoxSelectReader.GetItemDataPtr( nItemIndex );
	m_EditReaderReportSignatureName.SetWindowText( pReaderInfo -> ReportSignatureName );
	ClearDefaultReaderFlag();
	pReaderInfo -> IsDefaultReader = TRUE;
	memcpy( (void*)&m_DefaultReaderInfo, pReaderInfo, sizeof( READER_PERSONAL_INFO ) );
}


void CSelectCurrentUser::ClearDefaultReaderFlag()
{
	LIST_ELEMENT			*pReaderListElement;
	READER_PERSONAL_INFO	*pReaderInfo;
	
	memset( (void*)&m_DefaultReaderInfo, 0, sizeof(READER_PERSONAL_INFO) );
	pReaderListElement = RegisteredUserList;
	while ( pReaderListElement != 0 )
		{
		pReaderInfo = (READER_PERSONAL_INFO*)pReaderListElement -> pItem;
		pReaderInfo -> IsDefaultReader = FALSE;
		pReaderListElement = pReaderListElement -> pNextListElement;
		}
}


void CSelectCurrentUser::OnBnClickedExitReaderSelection( NMHDR *pNMHDR, LRESULT *pResult )
{
	memcpy( (void*)&BViewerCustomization.m_ReaderInfo, (void*)&m_DefaultReaderInfo, sizeof(READER_PERSONAL_INFO) );
	memcpy( &BViewerCustomization.m_CountryInfo, &m_DefaultReaderInfo.m_CountryInfo, sizeof(COUNTRY_INFO) );
	m_ButtonExit.HasBeenPressed( TRUE );
	CDialog::OnOK();

	*pResult = 0;
}


HBRUSH CSelectCurrentUser::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
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


BOOL CSelectCurrentUser::OnEraseBkgnd( CDC *pDC )
{
	CBrush		BackgroundBrush( COLOR_CONFIG );
	CRect		BackgroundRectangle;
	CBrush		*pOldBrush = pDC -> SelectObject( &BackgroundBrush );

	GetClientRect( BackgroundRectangle );
	pDC -> FillRect( BackgroundRectangle, &BackgroundBrush );
	pDC -> SelectObject( pOldBrush );

	return TRUE;
}


void CSelectCurrentUser::OnClose()
{
	CDialog::OnClose();
}



