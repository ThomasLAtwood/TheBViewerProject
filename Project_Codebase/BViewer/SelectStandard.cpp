// SelectStandard.cpp - Implements the class for the standard image selection
//  dialog box.  Clicking on one of the buttons in this dialog box results in
//  the correspoinding standard image being displayed.
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
#include "Configuration.h"
#include "DiagnosticImage.h"
#include "Mouse.h"
#include "ImageView.h"
#include "MainFrm.h"
#include "ImageFrame.h"
#include "SelectStandard.h"


extern CBViewerApp			ThisBViewerApp;
extern CONFIGURATION		BViewerConfiguration;

char		*pOwnershipDeclaration =
"This standard radiographic image file is the property of the International Labour Organization (ILO), Geneva, Switzerland. This image is licensed, not sold. Title and copyrights to the image and all copies thereof, and all modifications, enhancements, derivatives and other alterations of the image are the sole and exclusive property of the ILO.";


// CSelectStandard dialog class.
CSelectStandard::CSelectStandard( CWnd *pParent )
			: CDialog( CSelectStandard::IDD, pParent ),
				m_StaticExample1( "Example 1", 56, 15, 12, 6, 5, COLOR_WHITE, COLOR_STANDARD, COLOR_STANDARD,
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE,
										IDC_STATIC_SELECT_STD_EXAMPLE1 ),
				m_StaticExample2( "Example 2", 56, 15, 12, 6, 5, COLOR_WHITE, COLOR_STANDARD, COLOR_STANDARD,
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE,
										IDC_STATIC_SELECT_STD_EXAMPLE2 ),
				m_StaticClickButtonToView( "Click Button To View Standard Radiograph", 300, 15, 14, 7, 5, COLOR_WHITE, COLOR_STANDARD, COLOR_STANDARD,
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE,
										IDC_STATIC_SELECT_STD_CLICK_BUTTON ),
				m_ButtonSelectStdExample1( "0", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_EXAMPLE1 ),
				m_ButtonSelectStdExample2( "0", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_EXAMPLE2 ),

				m_ButtonSelectStd_1p( "1 p", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_STD11PP ),
				m_ButtonSelectStd_2p( "2 p", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_STD22PP ),
				m_ButtonSelectStd_3p( "3 p", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_STD33PP ),

				m_ButtonSelectStd_1q( "1 q", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_STD11QQ ),
				m_ButtonSelectStd_2q( "2 q", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_STD22QQ ),
				m_ButtonSelectStd_3q( "3 q", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_STD33QQ ),

				m_ButtonSelectStd_1r( "1 r", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_STD11RR ),
				m_ButtonSelectStd_2r( "2 r", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_STD22RR ),
				m_ButtonSelectStd_3r( "3 r", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_STD33RR ),

				m_ButtonSelectStd_A( "A", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_STDA ),
				m_ButtonSelectStd_B( "B", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_STDB ),
				m_ButtonSelectStd_C( "C", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_STDC ),

				m_ButtonSelectStd_1s( "1 s", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_STD11ST ),
				m_ButtonSelectStd_2s( "2 s", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_STD22SS ),
				m_ButtonSelectStd_3s( "3 s", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_STD33SS ),

				m_ButtonSelectStd_1t( "1 t", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_STD11TT ),
				m_ButtonSelectStd_2t( "2 t", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_STD22TT ),
				m_ButtonSelectStd_3t( "3 t", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_STD33TT ),

				m_ButtonSelectStd_u( "u", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_STDUU ),
				m_ButtonSelectStd_abn( "plu", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_STDABN ),
				m_ButtonSelectStd_CPAngle( "CPa", 56, 34, 18, 9, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_STDCPANGLE ),

				m_GroupSelectStdButtons( BUTTON_PUSHBUTTON, GROUP_SINGLE_SELECT | GROUP_ONE_TOUCHES_ALL, 23,
									&m_ButtonSelectStdExample1, &m_ButtonSelectStdExample2,
									&m_ButtonSelectStd_1p, &m_ButtonSelectStd_2p, &m_ButtonSelectStd_3p,
									&m_ButtonSelectStd_1q, &m_ButtonSelectStd_2q, &m_ButtonSelectStd_3q,
									&m_ButtonSelectStd_1r, &m_ButtonSelectStd_2r, &m_ButtonSelectStd_3r,
									&m_ButtonSelectStd_A, &m_ButtonSelectStd_B, &m_ButtonSelectStd_C,
									&m_ButtonSelectStd_1s, &m_ButtonSelectStd_2s, &m_ButtonSelectStd_3s,
									&m_ButtonSelectStd_1t, &m_ButtonSelectStd_2t, &m_ButtonSelectStd_3t,
									&m_ButtonSelectStd_u, &m_ButtonSelectStd_abn, &m_ButtonSelectStd_CPAngle )
{
	m_BkgdBrush.CreateSolidBrush( COLOR_STANDARD );
	Create( IDD_DIALOG_SELECT_STD, GetDesktopWindow() );
	SetForegroundWindow();
}


// Destructor:
CSelectStandard::~CSelectStandard()
{
}


BEGIN_MESSAGE_MAP( CSelectStandard, CDialog )
	//{{AFX_MSG_MAP(CSelectStandard)
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_EXAMPLE1, OnBnClickedButtonExample1 )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_EXAMPLE2, OnBnClickedButtonExample2 )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STD11PP, OnBnClickedButtonStd11pp )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STD22PP, OnBnClickedButtonStd22pp )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STD33PP, OnBnClickedButtonStd33pp )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STD11QQ, OnBnClickedButtonStd11qq )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STD22QQ, OnBnClickedButtonStd22qq )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STD33QQ, OnBnClickedButtonStd33qq )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STD11RR, OnBnClickedButtonStd11rr )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STD22RR, OnBnClickedButtonStd22rr )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STD33RR, OnBnClickedButtonStd33rr )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STDA, OnBnClickedButtonStdA )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STDB, OnBnClickedButtonStdB )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STDC, OnBnClickedButtonStdC )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STD11ST, OnBnClickedButtonStd11st )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STD22SS, OnBnClickedButtonStd22ss )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STD33SS, OnBnClickedButtonStd33ss )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STD11TT, OnBnClickedButtonStd11tt )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STD22TT, OnBnClickedButtonStd22tt )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STD33TT, OnBnClickedButtonStd33tt )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STDUU, OnBnClickedButtonStduu )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STDABN, OnBnClickedButtonStdabn )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STDCPANGLE, OnBnClickedButtonStdCPangle )
	ON_WM_CTLCOLOR()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


static char	*OriginalStandardFileNames[] =
				{
				"0example1",
				"0example2",
				"1p",
				"2p",
				"3p",
				"1q",
				"2q",
				"3q",
				"1r",
				"2r",
				"3r",
				"A",
				"B",
				"C",
				"1s",
				"2s",
				"3s",
				"1t",
				"2t",
				"3t",
				"quad_u",
				"quad_calcification_thickening"
				};


static char	*DigitalStandardFileNames[] =
				{
				"00_Normal_1",
				"00_Normal_2",
				"11_pp",
				"22_pp",
				"33_pp",
				"11_qq",
				"22_qq",
				"33_qq",
				"11_rr",
				"22_rr",
				"33_rr",
				"A_22_qq",
				"B_23_qr",
				"C_3+_rr",
				"11_ss",
				"22_ss",
				"33_ss",
				"11_tt",
				"22_tt",
				"33_ts",
				"123u",
				"Pleural",
				"CPangle"
				};


static char **pFileName = OriginalStandardFileNames;


BOOL CSelectStandard::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_StaticExample1.SetPosition( 10, 10, this );
	m_StaticExample2.SetPosition( 85, 10, this );
	m_StaticClickButtonToView.SetPosition( 160, 10, this );
	m_ButtonSelectStdExample1.SetPosition( 10, 30, this );
	m_ButtonSelectStdExample2.SetPosition( 85, 30, this );

	m_ButtonSelectStd_1p.SetPosition( 10, 70, this );
	m_ButtonSelectStd_2p.SetPosition( 85, 70, this );
	m_ButtonSelectStd_3p.SetPosition( 160, 70, this );

	m_ButtonSelectStd_1q.SetPosition( 10, 110, this );
	m_ButtonSelectStd_2q.SetPosition( 85, 110, this );
	m_ButtonSelectStd_3q.SetPosition( 160, 110, this );

	m_ButtonSelectStd_1r.SetPosition( 10, 150, this );
	m_ButtonSelectStd_2r.SetPosition( 85, 150, this );
	m_ButtonSelectStd_3r.SetPosition( 160, 150, this );
	
	m_ButtonSelectStd_A.SetPosition( 260, 30, this );
	m_ButtonSelectStd_B.SetPosition( 335, 30, this );
	m_ButtonSelectStd_C.SetPosition( 410, 30, this );
	
	m_ButtonSelectStd_1s.SetPosition( 260, 70, this );
	m_ButtonSelectStd_2s.SetPosition( 335, 70, this );
	m_ButtonSelectStd_3s.SetPosition( 410, 70, this );
	
	m_ButtonSelectStd_1t.SetPosition( 260, 110, this );
	m_ButtonSelectStd_2t.SetPosition( 335, 110, this );
	m_ButtonSelectStd_3t.SetPosition( 410, 110, this );
	
	m_ButtonSelectStd_u.SetPosition( 260, 150, this );

	if ( BViewerConfiguration.bUseDigitalStandards )
		{
		m_ButtonSelectStd_abn.SetPosition( 335, 150, this );
		m_ButtonSelectStd_CPAngle.SetPosition( 410, 150, this );
		pFileName = DigitalStandardFileNames;
		}
	else
		{
		m_ButtonSelectStd_abn.SetPosition( 410, 150, this );
		pFileName = OriginalStandardFileNames;
		}
	
	m_GroupSelectStdButtons.SetGroupVisibility( CONTROL_VISIBLE );
	SetIcon( ThisBViewerApp.m_hApplicationIcon, FALSE );

	return TRUE; 
}


void CSelectStandard::DisplaytSelectedStandardImage( unsigned long StandardIndex, char *pImageFileName )
{
	char					ImagePath[ FILE_PATH_STRING_LENGTH ];
	char					ImageFileExtension[ FILE_PATH_STRING_LENGTH ];
 	CMainFrame				*pMainFrame;
	CImageFrame				*pStandardImageFrame;
	WINDOWPLACEMENT			WindowPlacement;

	strcpy( ImagePath, "" );
	strncat( ImagePath, BViewerConfiguration.StandardDirectory, FILE_PATH_STRING_LENGTH );
	if ( ImagePath[ strlen( ImagePath ) - 1 ] != '\\' )
		strcat( ImagePath, "\\" );
	strcpy( ImageFileExtension, ".png" );
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( pMainFrame != 0 )
		{
		pStandardImageFrame = pMainFrame -> m_pImageFrame[ IMAGE_FRAME_STANDARD ];
		if ( pStandardImageFrame != 0 )
			{
			CWaitCursor			DisplaysHourglass;

			pStandardImageFrame -> OnSelectImage( 0, ImagePath, pImageFileName, ImageFileExtension );
			}
			
		// If the standard image window is minimized, restore it to normal viewing.
		pMainFrame -> m_pImageFrame[ IMAGE_FRAME_STANDARD ] -> GetWindowPlacement( &WindowPlacement );
		if ( WindowPlacement.showCmd == SW_SHOWMINIMIZED )
			{
			WindowPlacement.showCmd = SW_SHOWNORMAL;
			pMainFrame -> m_pImageFrame[ IMAGE_FRAME_STANDARD ] -> SetWindowPlacement( &WindowPlacement );
			}
		}
}

// CSelectStandard message handlers

void CSelectStandard::OnBnClickedButtonExample1( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_0_EXAMPLE1, pFileName[ STANDARD_0_EXAMPLE1 ] );

	*pResult = 0;
}

void CSelectStandard::OnBnClickedButtonExample2( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_0_EXAMPLE2, pFileName[ STANDARD_0_EXAMPLE2 ] );

	*pResult = 0;
}

void CSelectStandard::OnBnClickedButtonStd11pp( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_1P, pFileName[ STANDARD_1P ] );

	*pResult = 0;
}

void CSelectStandard::OnBnClickedButtonStd22pp( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_2P, pFileName[ STANDARD_2P ] );

	*pResult = 0;
}

void CSelectStandard::OnBnClickedButtonStd33pp( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_3P, pFileName[ STANDARD_3P ] );

	*pResult = 0;
}

void CSelectStandard::OnBnClickedButtonStd11qq( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_1Q, pFileName[ STANDARD_1Q ] );

	*pResult = 0;
}

void CSelectStandard::OnBnClickedButtonStd22qq( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_2Q, pFileName[ STANDARD_2Q ] );

	*pResult = 0;
}

void CSelectStandard::OnBnClickedButtonStd33qq( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_3Q, pFileName[ STANDARD_3Q ] );

	*pResult = 0;
}

void CSelectStandard::OnBnClickedButtonStd11rr( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_1R, pFileName[ STANDARD_1R ] );

	*pResult = 0;
}

void CSelectStandard::OnBnClickedButtonStd22rr( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_2R, pFileName[ STANDARD_2R ] );

	*pResult = 0;
}

void CSelectStandard::OnBnClickedButtonStd33rr( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_3R, pFileName[ STANDARD_3R ] );

	*pResult = 0;
}

void CSelectStandard::OnBnClickedButtonStdA( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_A, pFileName[ STANDARD_A ] );

	*pResult = 0;
}

void CSelectStandard::OnBnClickedButtonStdB( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_B, pFileName[ STANDARD_B ] );

	*pResult = 0;
}

void CSelectStandard::OnBnClickedButtonStdC( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_C, pFileName[ STANDARD_C ] );

	*pResult = 0;
}

void CSelectStandard::OnBnClickedButtonStd11st( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_1S, pFileName[ STANDARD_1S ] );

	*pResult = 0;
}

void CSelectStandard::OnBnClickedButtonStd22ss( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_2S, pFileName[ STANDARD_2S ] );

	*pResult = 0;
}

void CSelectStandard::OnBnClickedButtonStd33ss( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_3S, pFileName[ STANDARD_3S ] );

	*pResult = 0;
}

void CSelectStandard::OnBnClickedButtonStd11tt( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_1T, pFileName[ STANDARD_1T ] );

	*pResult = 0;
}

void CSelectStandard::OnBnClickedButtonStd22tt( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_2T, pFileName[ STANDARD_2T ] );

	*pResult = 0;
}

void CSelectStandard::OnBnClickedButtonStd33tt( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_3T, pFileName[ STANDARD_3T ] );

	*pResult = 0;
}

void CSelectStandard::OnBnClickedButtonStduu( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_U, pFileName[ STANDARD_U ] );

	*pResult = 0;
}

void CSelectStandard::OnBnClickedButtonStdabn( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_ABN, pFileName[ STANDARD_ABN ] );

	*pResult = 0;
}


void CSelectStandard::OnBnClickedButtonStdCPangle( NMHDR *pNMHDR, LRESULT *pResult )
{
	DisplaytSelectedStandardImage( STANDARD_CPANGLE, pFileName[ STANDARD_CPANGLE ] );

	*pResult = 0;
}


HBRUSH CSelectStandard::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
{
    // Return handle to the parent (this) window's background CBrush object
    // for use in painting by the child controls.
	return HBRUSH( m_BkgdBrush );
}


void CSelectStandard::OnClose()
{
	if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_STANDARDS )
		{
		ThisBViewerApp.TerminateTimers();
		AfxGetMainWnd() -> SendMessage( WM_CLOSE );
		}
	else
		ShowWindow( SW_MINIMIZE );
}


void CSelectStandard::OnAppExit( NMHDR *pNMHDR, LRESULT *pResult )
{
	ThisBViewerApp.TerminateTimers();
	AfxGetMainWnd()->SendMessage(WM_CLOSE);

	*pResult = 0;
}


