// ImageFrame.cpp : Implementation file for the CImageFrame class, which
//  implements the image viewing windows within which the subject study, standard
//  and report images are displayed.
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
//	*[4] 11/08/2023 by Tom Atwood
//		Fixed a bug where the display was not updated after a measurement calibration.
//	*[3] 07/19/2023 by Tom Atwood
//		Fixed code security issues.
//	*[2] 03/30/2023 by Tom Atwood
//		Fixed code security issues.
//	*[1] 12/20/2022 by Tom Atwood
//		Fixed code security issues.
//
#include "stdafx.h"
#include <math.h>
#include <commctrl.h>
#include "Module.h"
#include "ReportStatus.h"
#include "BViewer.h"
#include "DiagnosticImage.h"
#include "Mouse.h"
#include "ImageView.h"
#include "MainFrm.h"
#include "ImageFrame.h"
#include "SelectStandard.h"
#include "PopupDialog.h"


extern CBViewerApp			ThisBViewerApp;
extern CONFIGURATION		BViewerConfiguration;
extern CCustomization		BViewerCustomization;
extern CString				PopupWindowClass;
extern LIST_HEAD			AvailablePresetList;
extern BOOL					bTheLastKeyPressedWasESC;


// CImageFrame
CImageFrame::CImageFrame()
{
	m_pAssignedDiagnosticImage = 0;
	m_CurrentReportFileName[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
	m_bALuminosityTransformationHasBeenApplied = TRUE;
}


CImageFrame::~CImageFrame()
{
	LRESULT			Result;

	OnButtonEraseMeasurements( 0, &Result );
	if ( m_pAssignedDiagnosticImage != 0 )			// *[1] Prevent memory leak.
		delete m_pAssignedDiagnosticImage;			// *[1]

}


BEGIN_MESSAGE_MAP( CImageFrame, CFrameWnd )
	//{{AFX_MSG_MAP(CImageFrame)
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_RESET_IMAGE, OnResetImage )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_CLEAR_IMAGE, OnClearImage )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_WINDOWING_NOT_APPLIED, OnBnClickedNoWindowing )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_LINEAR_WINDOWING, OnBnClickedLinearWindowing )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_SIGMOID_WINDOWING, OnBnClickedSigmoidWindowing )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_INVERT_IMAGE, OnInvertImageColors )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_FLIP_IMAGE_VERT, OnFlipImageVertically )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_FLIP_IMAGE_HORIZ, OnFlipImageHorizontally )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_ROTATE_IMAGE, OnRotateImage )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_SAVE_IMAGE_SETTINGS, OnSaveImageSettings )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_APPLY_IMAGE_SETTINGS, OnApplyImagePreset )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_SET_REPORT_PAGE, OnSetReportPage )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_PRINT_REPORT, OnPrintReport )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_SAVE_REPORT, OnSaveReport )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_FULL_SIZE, OnSetImageSize )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_MEASURE_DISTANCE, OnButtonMeasureDistance )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_ERASE_MEASUREMENTS, OnButtonEraseMeasurements )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_CALIBRATE_MEASUREMENT, OnButtonCalibrateMeasurements )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_ENABLE_ANNOTATIONS, OnButtonEnableAnnotations )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_SHOW_HISTOGRAM, OnButtonShowHistogram )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_FLATTEN_HISTOGRAM, OnButtonFlattenHistogram )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_CENTER_HISTOGRAM, OnButtonCenterHistogram )
	ON_WM_CREATE()
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	ON_WM_MOUSEWHEEL()
	ON_WM_CTLCOLOR()
	ON_WM_CLOSE()
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_WINDOW_CENTER, OnEditWindowCenterKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_GAMMA, OnEditGammaKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_WINDOW_WIDTH, OnEditWindowWidthKillFocus )
	ON_WM_MOUSEMOVE()
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CImageFrame::PreCreateWindow( CREATESTRUCT &cs ) 
{
	if ( !CFrameWnd::PreCreateWindow( cs ) )
		return FALSE;

	cs.hMenu = NULL;
	cs.lpszClass = AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, 
		::LoadCursor( NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), ThisBViewerApp.m_hApplicationIcon );

	return TRUE;
}


int CImageFrame::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	RECT			ClientRect;
	INT				ClientWidth;
	INT				ClientHeight;
	RECT			DialogBarRect;
	INT				DialogBarHeight;
	BOOL			bCreatedOK = FALSE;			// [2] Initialized variable.
	TomEdit			*pCtrlGamma;
	TomEdit			*pCtrlWindowCenter;
	TomEdit			*pCtrlWindowWidth;
	WNDCLASS		wndClass;
	unsigned int	WindowID;

	if ( CFrameWnd::OnCreate( lpCreateStruct ) == -1 )
		return -1;
	
	SetIcon( ThisBViewerApp.m_hApplicationIcon, FALSE );
	GetClientRect( &ClientRect );
	ClientWidth = ClientRect.right - ClientRect.left;

	switch ( m_FrameFunction )
		{
		case IMAGE_FRAME_FUNCTION_PATIENT:
			bCreatedOK = m_wndDlgBar.Create( this, IDD_DIALOGBAR_IMAGE, WS_CHILD, IDD_DIALOGBAR_PATIENT );
			break;
		case IMAGE_FRAME_FUNCTION_STANDARD:
			bCreatedOK = m_wndDlgBar.Create( this, IDD_DIALOGBAR_IMAGE, WS_CHILD, IDD_DIALOGBAR_STANDARD );
			break;
		case IMAGE_FRAME_FUNCTION_REPORT:
			bCreatedOK = m_wndDlgBar.Create( this, IDD_DIALOGBAR_IMAGE, WS_CHILD, IDD_DIALOGBAR_REPORT );
			break;
		}
	if ( !bCreatedOK )
		return -1;      // fail to create
	else
		{
		m_wndDlgBar.SetWindowPos( 0, ClientRect.left, ClientRect.top, ClientWidth, IMAGE_DIALOG_BAR_HEIGHT, 0 );
		if ( m_FrameFunction == IMAGE_FRAME_FUNCTION_PATIENT )
			{
			if ( BViewerCustomization.m_WindowingAlgorithmSelection == SELECT_LINEAR_WINDOWING )
				m_wndDlgBar.m_ButtonLinearWindowing.m_ToggleState = BUTTON_ON;
			else
				m_wndDlgBar.m_ButtonLinearWindowing.m_ToggleState = BUTTON_OFF;
			if ( BViewerCustomization.m_WindowingAlgorithmSelection == SELECT_SIGMOID_WINDOWING )
				m_wndDlgBar.m_ButtonSigmoidWindowing.m_ToggleState = BUTTON_ON;
			else
				m_wndDlgBar.m_ButtonSigmoidWindowing.m_ToggleState = BUTTON_OFF;
			pCtrlGamma = (TomEdit*)m_wndDlgBar.GetDlgItem( IDC_EDIT_GAMMA );
			if ( pCtrlGamma != 0 )
				pCtrlGamma -> SetWindowText( "1.0" );
			pCtrlWindowCenter = (TomEdit*)m_wndDlgBar.GetDlgItem( IDC_EDIT_WINDOW_CENTER );
			if ( pCtrlWindowCenter != 0 )
				pCtrlWindowCenter -> SetWindowText( "0.0" );
			pCtrlWindowWidth = (TomEdit*)m_wndDlgBar.GetDlgItem( IDC_EDIT_WINDOW_WIDTH );
			if ( pCtrlWindowWidth != 0 )
				pCtrlWindowWidth -> SetWindowText( "0.0" );
			}
		}

	GetClientRect( &ClientRect );
	m_wndDlgBar.GetWindowRect( &DialogBarRect );
	DialogBarHeight = DialogBarRect.bottom - DialogBarRect.top;
	ClientRect.top += DialogBarHeight;
	ClientWidth = ClientRect.right - ClientRect.left;
	ClientHeight = ClientRect.bottom - ClientRect.top;

	// Create potentially separate window classes for each of the image windows.
	// CS_OWNDC is needed to prevent having to re-establish the DC details
	// for each rendering of an image.  This is recommended for OpenGL.
	wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_OWNDC;
	wndClass.lpfnWndProc = AfxWndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = AfxGetInstanceHandle();
	wndClass.hIcon = NULL;
	wndClass.hCursor = ::LoadCursor( NULL, IDC_ARROW );
	wndClass.hbrBackground = 0;
	wndClass.lpszMenuName = NULL;
	m_ImageView.m_pDisplayMonitor = m_pDisplayMonitor;
	switch ( m_FrameFunction )
		{
		case IMAGE_FRAME_FUNCTION_PATIENT:
			wndClass.lpszClassName = "OpenGLClass1";
			WindowID = IDC_WND_IMAGE1;
			m_ImageView.m_ViewFunction = IMAGE_VIEW_FUNCTION_PATIENT;
			break;
		case IMAGE_FRAME_FUNCTION_STANDARD:
			wndClass.lpszClassName = "OpenGLClass2";
			WindowID = IDC_WND_IMAGE2;
			m_ImageView.m_ViewFunction = IMAGE_VIEW_FUNCTION_STANDARD;
			break;
		case IMAGE_FRAME_FUNCTION_REPORT:
			wndClass.lpszClassName = "OpenGLClass3";
			WindowID = IDC_WND_IMAGE3;
			m_ImageView.m_ViewFunction = IMAGE_VIEW_FUNCTION_REPORT;
			break;
		default:															// *[2] Added default case.
			wndClass.lpszClassName = "OpenGLClass1";
			WindowID = IDC_WND_IMAGE1;
			m_ImageView.m_ViewFunction = IMAGE_VIEW_FUNCTION_PATIENT;
			break;
		}
	m_ImageView.m_pWndDlgBar = &m_wndDlgBar;

	AfxRegisterClass( &wndClass );		// Register the window class.
	if (!m_ImageView.Create( wndClass.lpszClassName, "Image View", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
						CRect( ClientRect.left, ClientRect.top, ClientWidth / 2, ClientHeight ), this, WindowID, NULL ))
		return -1;
	strncpy_s( m_ImageView.m_ViewName, 32, m_FrameName, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
	m_ImageView.m_WindowSizeInPixels = ClientRect;

	LogMessage( "Image view successfully created.", MESSAGE_TYPE_SUPPLEMENTARY );


	return 0;
}


void CImageFrame::OnShowWindow( BOOL bShow, UINT nStatus )
{
	CFrameWnd::OnShowWindow( bShow, nStatus );
	m_ImageView.UpdateWindow();
}


void CImageFrame::OnSize( UINT nType, int cx, int cy )
{
	CFrameWnd::OnSize( nType, cx, cy );

	RECT			ClientRect;
	INT				ClientWidth;
	RECT			DialogBarRect;
	INT				DialogBarHeight;
	LRESULT			Result;


	GetClientRect( &ClientRect );
	ClientWidth = ClientRect.right - ClientRect.left;
	m_wndDlgBar.SetWindowPos( 0, ClientRect.left, ClientRect.top, ClientWidth, IMAGE_DIALOG_BAR_HEIGHT, 0 );

	m_wndDlgBar.GetWindowRect( &DialogBarRect );
	DialogBarHeight = DialogBarRect.bottom - DialogBarRect.top;
	ClientRect.top += DialogBarHeight;

	m_ImageView.m_WindowSizeInPixels = ClientRect;
	m_ImageView.MoveWindow( &m_ImageView.m_WindowSizeInPixels, TRUE );
	if ( m_pAssignedDiagnosticImage != 0 )
		OnResetImage( 0, &Result );
}


BOOL CImageFrame::OnSelectImage( void *pStudy, char *pImagePath, char *pImageFileName, char *pImageFileExtension )
{
	char				FileSpecForOpening[ FULL_FILE_SPEC_STRING_LENGTH ];
	CDiagnosticImage	*pDiagnosticImage;
	BOOL				bNoError = TRUE;			// *[2] Added redundant initialization to please Fortify.
	CMainFrame			*pMainFrame;
	CEdit				*pCtrlFileName;
	char				SubjectName[ MAX_LOGGING_STRING_LENGTH ];
	char				Msg[ FULL_FILE_SPEC_STRING_LENGTH ];
	LRESULT				Result;

	pDiagnosticImage = new CDiagnosticImage();
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	bNoError = ( pDiagnosticImage != 0 && pMainFrame != 0 );												// *[1] Eliminated potential memory leak.
	if ( bNoError )
		{
		pCtrlFileName = (CEdit*)m_wndDlgBar.GetDlgItem( IDC_EDIT_IMAGE_NAME );
		strncpy_s( FileSpecForOpening, FULL_FILE_SPEC_STRING_LENGTH, pImagePath, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
		strncat_s( FileSpecForOpening, FULL_FILE_SPEC_STRING_LENGTH, pImageFileName, _TRUNCATE );			// *[2] Replaced strncat with strncat_s.
		strncat_s( FileSpecForOpening, FULL_FILE_SPEC_STRING_LENGTH, pImageFileExtension, _TRUNCATE );		// *[2] Replaced strncat with strncat_s.
		bNoError = pDiagnosticImage -> ReadPNGImageFile( FileSpecForOpening, m_pDisplayMonitor, m_FrameFunction );
		if ( !bNoError && m_FrameFunction == IMAGE_FRAME_FUNCTION_STANDARD )	// If this is the standard image window...
			{
			strncpy_s( FileSpecForOpening, FULL_FILE_SPEC_STRING_LENGTH, pImagePath, _TRUNCATE );			// *[1] Replaced strcpy with strncpy_s.
			strncat_s( FileSpecForOpening, FULL_FILE_SPEC_STRING_LENGTH, "DemoStandard.png", _TRUNCATE );	// *[2] Replaced strncat with strncat_s.
			bNoError = pDiagnosticImage -> ReadPNGImageFile( FileSpecForOpening, m_pDisplayMonitor, m_FrameFunction );
			}
		if ( bNoError )
			{
			// If there was a previous image loaded, delete it.
			if ( m_pAssignedDiagnosticImage != 0 )
				{
				if ( m_FrameFunction == IMAGE_FRAME_FUNCTION_PATIENT )	// If this is the subject study image window...
					{
					OnButtonEraseMeasurements( 0, &Result );
					pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> OnClearImage( 0, &Result );
					if ( pMainFrame -> m_pImageFrame[ IMAGE_FRAME_REPORT ] != 0 )
						pMainFrame -> m_pImageFrame[ IMAGE_FRAME_REPORT ] -> OnClearImage( 0, &Result );
					}
				delete m_pAssignedDiagnosticImage;
				}
			m_pAssignedDiagnosticImage = pDiagnosticImage;
			// Load the default measurement calibration.
			m_ImageView.m_PixelsPerMillimeter = m_pAssignedDiagnosticImage -> m_PixelsPerMillimeter;

			if ( m_FrameFunction == IMAGE_FRAME_FUNCTION_STANDARD )	// If this is the standard image window...
				{
				pDiagnosticImage -> m_OriginalGrayscaleSetting.m_Gamma = 1.0;
				pDiagnosticImage -> m_bEnableGammaCorrection = FALSE;
				strncpy_s( SubjectName, MAX_LOGGING_STRING_LENGTH, "Standard:  ", _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
				strncat_s( SubjectName, MAX_LOGGING_STRING_LENGTH, pImageFileName, _TRUNCATE );				// *[2] Replaced strncat with strncat_s.
				pCtrlFileName -> SetWindowText( SubjectName );
				m_pAssignedDiagnosticImage -> m_bEnableOverlays = FALSE;

				sprintf_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, "                  Open standard file %s for viewing.", pImageFileName );		// *[1] Replaced sprintf with sprintf_s.
				LogMessage( Msg, MESSAGE_TYPE_NORMAL_LOG );
				pStudy = 0;
				}
			else
				{
				pDiagnosticImage -> m_OriginalGrayscaleSetting.m_Gamma = 1.0;
				if ( pStudy != 0 )
					{
					strncpy_s( SubjectName, MAX_LOGGING_STRING_LENGTH, ( (CStudy*)pStudy ) -> m_PatientLastName, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
					if ( strlen( ( (CStudy*)pStudy ) -> m_PatientLastName ) > 0 && strlen( ( (CStudy*)pStudy ) -> m_PatientFirstName ) > 0  )
						strncat_s( SubjectName, FILE_PATH_STRING_LENGTH, ", ", _TRUNCATE );										// *[1] Replaced strcat with strncat_s.
					strncat_s( SubjectName, FILE_PATH_STRING_LENGTH, ( (CStudy*)pStudy ) -> m_PatientFirstName, _TRUNCATE );	// *[1] Replaced strcat with strncat_s.
					sprintf_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, "   ********   Subject study file for %s selected for viewing.", SubjectName );	// *[1] Replaced sprintf with sprintf_s.
					LogMessage( Msg, MESSAGE_TYPE_NORMAL_LOG );
					pCtrlFileName -> SetWindowText( SubjectName );
					pMainFrame -> m_wndDlgBar.m_EditImageName.SetWindowText( SubjectName );
					if ( pMainFrame -> m_pImageFrame[ IMAGE_FRAME_REPORT ] != 0 )
						pMainFrame -> m_pImageFrame[ IMAGE_FRAME_REPORT ] -> m_wndDlgBar.m_EditImageName.SetWindowText( SubjectName );
					pDiagnosticImage -> m_OriginalGrayscaleSetting.m_Gamma = ( (CStudy*)pStudy ) -> m_GammaSetting;
					pDiagnosticImage -> m_bEnableGammaCorrection = TRUE;

					m_bALuminosityTransformationHasBeenApplied = FALSE;
					if ( BViewerConfiguration.bEnableHistogram )
						{
						m_wndDlgBar.m_ButtonFlattenHistogram.EnableWindow( TRUE );
						m_wndDlgBar.m_ButtonCenterHistogram.EnableWindow( TRUE );
						m_wndDlgBar.m_StaticHistogram.m_pHistogramData = &pDiagnosticImage -> m_LuminosityHistogram;
						}
					sprintf_s( Msg, FULL_FILE_SPEC_STRING_LENGTH, "Open subject study file %s for viewing.", pImageFileName );	// *[1] Replaced sprintf with sprintf_s.
					LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
					}
				}
			LogMessage( "Setting diagnostic image into view.", MESSAGE_TYPE_SUPPLEMENTARY );
			m_ImageView.SetDiagnosticImage( pDiagnosticImage, (CStudy*)pStudy );
			LogMessage( "Bringing image window to top.", MESSAGE_TYPE_SUPPLEMENTARY );
			SetFocus();						// Bring the image window to the display forefront.
			}
		}
	if ( !bNoError )
		delete pDiagnosticImage;

	return bNoError;
}


void CImageFrame::OnInvertImageColors( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( m_pAssignedDiagnosticImage != 0 && m_FrameFunction == IMAGE_FRAME_FUNCTION_PATIENT )
		{
		m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_bColorsInverted = !m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_bColorsInverted;
			m_ImageView.RepaintFast();
		}
	*pResult = 0;
}


void CImageFrame::OnFlipImageVertically( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( m_pAssignedDiagnosticImage != 0 && m_FrameFunction == IMAGE_FRAME_FUNCTION_PATIENT )
		{
		m_pAssignedDiagnosticImage -> FlipVertically();
		m_ImageView.PrepareImage();
		m_ImageView.RepaintFast();
		}
	*pResult = 0;
}


void CImageFrame::OnFlipImageHorizontally( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( m_pAssignedDiagnosticImage != 0 && m_FrameFunction == IMAGE_FRAME_FUNCTION_PATIENT )
		{
		m_pAssignedDiagnosticImage -> FlipHorizontally();
		m_ImageView.PrepareImage();
		m_ImageView.RepaintFast();
		}
	*pResult = 0;
}


void CImageFrame::OnRotateImage( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( m_pAssignedDiagnosticImage != 0 && m_FrameFunction == IMAGE_FRAME_FUNCTION_PATIENT )
		{
		m_pAssignedDiagnosticImage -> AdjustRotationAngle();
		m_ImageView.PrepareImage();
		m_ImageView.RepaintFast();
		}
	*pResult = 0;
}


void CImageFrame::OnResetImage( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( m_pAssignedDiagnosticImage != 0 )
		{
		m_ImageView.ResetDiagnosticImage( FALSE );
		}

	*pResult = 0;
}


void CImageFrame::OnClearImage( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( m_pAssignedDiagnosticImage != 0 )
		{
		m_ImageView.ClearDiagnosticImage();
		m_ImageView.LoadCurrentImageSettingsIntoEditBoxes();
		delete m_pAssignedDiagnosticImage;
		m_pAssignedDiagnosticImage = 0;
		}
	*pResult = 0;
}


void CImageFrame::OnBnClickedNoWindowing( NMHDR *pNMHDR, LRESULT *pResult )
{
 	CMainFrame				*pMainFrame;

	if ( m_pAssignedDiagnosticImage != 0 )
		{
		m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowCenter = m_pAssignedDiagnosticImage -> m_MaxGrayscaleValue / 2;
		m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowWidth = m_pAssignedDiagnosticImage -> m_MaxGrayscaleValue;
		m_ImageView.LoadCurrentImageSettingsIntoEditBoxes();
		pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
		if ( pMainFrame != 0 )
			pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> ApplyCurrentWindowingSettings();
		}

	*pResult = 0;
}


void CImageFrame::OnBnClickedLinearWindowing( NMHDR *pNMHDR, LRESULT *pResult )
{
 	CMainFrame				*pMainFrame;

	m_wndDlgBar.m_ButtonLinearWindowing.m_pGroup -> RespondToSelection( (void*)&m_wndDlgBar.m_ButtonLinearWindowing );
	if ( m_wndDlgBar.m_ButtonLinearWindowing.m_ToggleState == BUTTON_ON )
		BViewerCustomization.m_WindowingAlgorithmSelection = SELECT_LINEAR_WINDOWING;
	m_wndDlgBar.Invalidate( TRUE );

	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( pMainFrame != 0 )
		pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> ApplyCurrentWindowingSettings();

	*pResult = 0;
}


void CImageFrame::OnBnClickedSigmoidWindowing( NMHDR *pNMHDR, LRESULT *pResult )
{
 	CMainFrame				*pMainFrame;

	m_wndDlgBar.m_ButtonSigmoidWindowing.m_pGroup -> RespondToSelection( (void*)&m_wndDlgBar.m_ButtonSigmoidWindowing );
	if ( m_wndDlgBar.m_ButtonSigmoidWindowing.m_ToggleState == BUTTON_ON )
		BViewerCustomization.m_WindowingAlgorithmSelection = SELECT_SIGMOID_WINDOWING;
	m_wndDlgBar.Invalidate( TRUE );

	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( pMainFrame != 0 )
		pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> ApplyCurrentWindowingSettings();

	*pResult = 0;
}


void CImageFrame::OnSaveImageSettings( NMHDR *pNMHDR, LRESULT *pResult )
{
	CPreset						*pImagePresetScreen;
	LIST_ELEMENT				*pPresetListElement;
	IMAGE_GRAYSCALE_SETTING		*pNewGrayscalePreset;
	IMAGE_GRAYSCALE_SETTING		*pGrayscalePreset;
	BOOL						bCancel;
	BOOL						bMatchFound;

	if ( m_FrameFunction == IMAGE_FRAME_FUNCTION_PATIENT && ThisBViewerApp.m_pCurrentStudy != 0 )		// If this is a subject study image...
		{
		if ( m_pAssignedDiagnosticImage != 0 )
			{
			pImagePresetScreen = new( CPreset );
			if ( pImagePresetScreen != 0 )
				{
				pNewGrayscalePreset = (IMAGE_GRAYSCALE_SETTING*)malloc( sizeof(IMAGE_GRAYSCALE_SETTING) );
				if ( pNewGrayscalePreset != 0 )
					{
					memcpy( pNewGrayscalePreset, &m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting, sizeof(IMAGE_GRAYSCALE_SETTING) );
					pImagePresetScreen -> m_pCurrentPreset = pNewGrayscalePreset;
					strncpy_s( pNewGrayscalePreset -> m_PresetName, MAX_CFG_STRING_LENGTH, "Most recently saved image window setting.", _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
					pImagePresetScreen -> m_bSaveImageSetting = TRUE;
					bCancel = !( pImagePresetScreen -> DoModal() == IDOK );
					if ( !bCancel )
						{
						bMatchFound = FALSE;
						pPresetListElement = AvailablePresetList;
						while ( pPresetListElement != 0 )
							{
							pGrayscalePreset = (IMAGE_GRAYSCALE_SETTING*)pPresetListElement -> pItem;
							if ( strcmp( pGrayscalePreset -> m_PresetName, pNewGrayscalePreset -> m_PresetName ) == 0 )
								{
								free( pPresetListElement -> pItem );
								pPresetListElement -> pItem = (void*)pNewGrayscalePreset;
								bMatchFound = TRUE;
								pPresetListElement = 0;
								}
							else
								pPresetListElement = pPresetListElement -> pNextListElement;
							}
						if ( !bMatchFound )
							AppendToList( &AvailablePresetList, (void*)pNewGrayscalePreset );
						}
					else
						free( pNewGrayscalePreset );
					}
				delete pImagePresetScreen;
				}
			}
		Invalidate( TRUE );
		}
	*pResult = 0;
}


void CImageFrame::OnApplyImagePreset( NMHDR *pNMHDR, LRESULT *pResult )
{
	CPreset						*pImagePresetScreen;
	BOOL						bCancel;

	if ( m_FrameFunction == IMAGE_FRAME_FUNCTION_PATIENT && ThisBViewerApp.m_pCurrentStudy != 0 )		// If this is a subject study image...
		{
		pImagePresetScreen = new( CPreset );
		if ( pImagePresetScreen != 0 )
			{
			pImagePresetScreen -> m_bSaveImageSetting = FALSE;
			bCancel = !( pImagePresetScreen -> DoModal() == IDOK );
			if ( !bCancel && m_pAssignedDiagnosticImage != 0 && pImagePresetScreen -> m_pCurrentPreset != 0 )
				m_ImageView.UpdateImageGrayscaleDisplay( pImagePresetScreen -> m_pCurrentPreset );
			delete pImagePresetScreen;
			}
		Invalidate( TRUE );
		}
	*pResult = 0;
}


void CImageFrame::OnButtonMeasureDistance( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( m_ImageView.m_bEnableMeasure )
		{
		// Restore the measurement buttons to their dormant state.
		m_wndDlgBar.m_ButtonMeasureDistance.m_IdleBkgColor = COLOR_PATIENT_SELECTOR;
		m_wndDlgBar.m_ButtonMeasureDistance.m_VisitedBkgdColor = COLOR_PATIENT_SELECTOR;
		m_wndDlgBar.m_ButtonMeasureDistance.m_TextColor = COLOR_WHITE;
		m_wndDlgBar.m_ButtonEraseMeasurements.m_IdleBkgColor = COLOR_PATIENT_SELECTOR;
		m_wndDlgBar.m_ButtonEraseMeasurements.m_VisitedBkgdColor = COLOR_PATIENT_SELECTOR;
		m_wndDlgBar.m_ButtonEraseMeasurements.m_TextColor = COLOR_WHITE;
		m_wndDlgBar.m_ButtonCalibrateMeasurements.m_IdleBkgColor = COLOR_PATIENT_SELECTOR;
		m_wndDlgBar.m_ButtonCalibrateMeasurements.m_VisitedBkgdColor = COLOR_PATIENT_SELECTOR;
		m_wndDlgBar.m_ButtonCalibrateMeasurements.m_TextColor = COLOR_WHITE;
		
		m_ImageView.m_bEnableMeasure = FALSE;
		m_ImageView.m_Mouse.m_bEnableMeasure = FALSE;
		m_wndDlgBar.m_ButtonMeasureDistance.m_ControlText = "Measure Distance";
		}
	else
		{
		// Emphasize the measurement button group while they are active.
		m_wndDlgBar.m_ButtonMeasureDistance.m_IdleBkgColor = COLOR_GREEN;
		m_wndDlgBar.m_ButtonMeasureDistance.m_VisitedBkgdColor = COLOR_GREEN;
		m_wndDlgBar.m_ButtonMeasureDistance.m_TextColor = COLOR_BLACK;
		m_wndDlgBar.m_ButtonEraseMeasurements.m_IdleBkgColor = COLOR_GREEN;
		m_wndDlgBar.m_ButtonEraseMeasurements.m_VisitedBkgdColor = COLOR_GREEN;
		m_wndDlgBar.m_ButtonEraseMeasurements.m_TextColor = COLOR_BLACK;
		m_wndDlgBar.m_ButtonCalibrateMeasurements.m_IdleBkgColor = COLOR_GREEN;
		m_wndDlgBar.m_ButtonCalibrateMeasurements.m_VisitedBkgdColor = COLOR_GREEN;
		m_wndDlgBar.m_ButtonCalibrateMeasurements.m_TextColor = COLOR_BLACK;
		m_ImageView.m_bEnableMeasure = TRUE;
		m_ImageView.m_Mouse.m_bEnableMeasure = TRUE;
		m_wndDlgBar.m_ButtonMeasureDistance.m_ControlText = "Stop Measuring";
		}
	m_wndDlgBar.Invalidate();
	*pResult = 0;
}


void CImageFrame::OnButtonEraseMeasurements( NMHDR *pNMHDR, LRESULT *pResult )
{
	MEASURED_INTERVAL			*pMeasuredInterval;
	MEASURED_INTERVAL			*pPrevMeasuredInterval;

	pMeasuredInterval = m_ImageView.m_pMeasuredIntervalList;
	while( pMeasuredInterval != 0 )
		{
		pPrevMeasuredInterval = pMeasuredInterval;
		pMeasuredInterval = pMeasuredInterval -> pNextInterval;
		free( pPrevMeasuredInterval );
		}
	m_ImageView.m_pMeasuredIntervalList = 0;
	m_ImageView.Invalidate( TRUE );
	*pResult = 0;
}


// This function will not wait for a user response before it returns to the calling function.
void CImageFrame::PerformUserInput( USER_NOTIFICATION_INFO *pUserNotificationInfo )
{
	CPopupDialog			*pPopupDialog;
	RECT					ClientRect;
	INT						ClientWidth;
	INT						ClientHeight;
	int						DialogWidth;
	int						DialogHeight;

	GetClientRect( &ClientRect );
	ClientWidth = ClientRect.right - ClientRect.left;
	ClientHeight = ClientRect.bottom - ClientRect.top;
	DialogWidth = pUserNotificationInfo -> WindowWidth;
	DialogHeight = pUserNotificationInfo -> WindowHeight;

	pPopupDialog = new CPopupDialog( DialogWidth, DialogHeight, COLOR_CONFIG, 0, IDD_DIALOG_POPUP );
	if ( pPopupDialog != 0 )
		{
		pPopupDialog -> m_pUserNotificationInfo = pUserNotificationInfo;
		pPopupDialog -> SetPosition( ( ClientWidth - DialogWidth ) / 2, 200, this, PopupWindowClass );
		pPopupDialog -> BringWindowToTop();
		pPopupDialog -> SetFocus();
		}
}


void CImageFrame::RebuildHistogram()
{
	int						nBitDepth;
	unsigned long			MaxGrayscaleValue;

	if ( m_pAssignedDiagnosticImage != 0 && m_pAssignedDiagnosticImage -> m_pImageData != 0 )
		{
		if ( m_pAssignedDiagnosticImage -> m_LuminosityHistogram.pHistogramArray != 0 )
			free( m_pAssignedDiagnosticImage -> m_LuminosityHistogram.pHistogramArray );
		nBitDepth = m_pAssignedDiagnosticImage -> m_ImageBitDepth;
		if ( nBitDepth <= 8 )
			MaxGrayscaleValue = 255;
		else
			MaxGrayscaleValue = m_pAssignedDiagnosticImage -> m_MaxGrayscaleValue;
		m_pAssignedDiagnosticImage -> m_LuminosityHistogram.nNumberOfBins = MaxGrayscaleValue + 1;
		m_pAssignedDiagnosticImage -> m_LuminosityHistogram.AverageBinValue = 0.0;
		m_pAssignedDiagnosticImage -> m_LuminosityHistogram.AverageViewableBinValue = 0.0;
		m_pAssignedDiagnosticImage -> m_LuminosityHistogram.pHistogramArray = (unsigned long*)malloc( sizeof(unsigned long) * ( MaxGrayscaleValue + 1 ) );
		}
	m_ImageView.CreateGrayscaleHistogram();
}


void CImageFrame::OnButtonShowHistogram( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( BViewerConfiguration.bEnableHistogram )
		{
		RebuildHistogram();
		m_wndDlgBar.Invalidate();
		m_wndDlgBar.UpdateWindow();
		}

	*pResult = 0;
}


// Adjust the image grayscale so that its histogram is evenly distributed
// across the full range of the grayscale.
void CImageFrame::OnButtonFlattenHistogram( NMHDR *pNMHDR, LRESULT *pResult )
{
	long					nImageRows;
	long					nImagePixelsPerRow;
	long					nImageBytesPerRow;
	long					nRow;
	long					nPixel;
	unsigned char			*pBuffer;
	unsigned char			*pInputReadPoint;
	int						nBitDepth;
	int						InputPixelValue;
	unsigned short			AdjustedPixelValue;
	int						MaxGrayscaleValue;
	HISTOGRAM_DATA			*pLuminosityHistogram;
	unsigned short			*pLuminosityLookupTable;
	double					ResidualHistogramArea;
	int						nHistogramBin;
	int						nNewHistogramBin;
	char					Msg[ MAX_EXTRA_LONG_STRING_LENGTH ];

	if ( BViewerConfiguration.bEnableHistogram )
		{
		if ( m_wndDlgBar.m_ButtonFlattenHistogram.IsWindowEnabled() && m_bALuminosityTransformationHasBeenApplied )
			{
			strncpy_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "You must reload the image before applying another\n", _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
			strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "pixel luminosity transformation.\n", _TRUNCATE );					// *[3] Replaced strcat with strncat_s.
			ThisBViewerApp.NotifyUserToAcknowledgeContinuation( Msg );
			// You can only apply the luminosity redistribution one time.
			m_wndDlgBar.m_ButtonFlattenHistogram.EnableWindow( FALSE );
			m_wndDlgBar.m_ButtonCenterHistogram.EnableWindow( FALSE );
			}
		if ( m_pAssignedDiagnosticImage != 0 && m_pAssignedDiagnosticImage -> m_pImageData != 0 && !m_bALuminosityTransformationHasBeenApplied )
			{
			RebuildHistogram();
			pLuminosityHistogram = &m_pAssignedDiagnosticImage -> m_LuminosityHistogram;
			nImageRows = (long)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels;
			nImagePixelsPerRow = (long)( m_pAssignedDiagnosticImage -> m_ImageWidthInPixels );
			nBitDepth = m_pAssignedDiagnosticImage -> m_ImageBitDepth;
			if ( nBitDepth <= 8 )
				{
				nImageBytesPerRow = nImagePixelsPerRow;
				MaxGrayscaleValue = 255;
				}
			else
				{
				nImageBytesPerRow = nImagePixelsPerRow * 2;
				MaxGrayscaleValue = (int)m_pAssignedDiagnosticImage -> m_MaxGrayscaleValue;
				}
			pBuffer = m_pAssignedDiagnosticImage -> m_pImageData;
			pInputReadPoint = pBuffer;
			// Build a lookup table that flattens the luminosity distribution as much as possible and
			// stretches it to fill the full grayscale range.
			pLuminosityLookupTable = (unsigned short*)calloc( 1, ( 2 * ( MaxGrayscaleValue + 1 ) ) );
			if ( pLuminosityLookupTable != 0 )
				{
				ResidualHistogramArea = 0.0;
				nNewHistogramBin = 0;
				for ( nHistogramBin = 0; nHistogramBin <= MaxGrayscaleValue; nHistogramBin++ )
					{
					if ( nHistogramBin > 0 )
						ResidualHistogramArea += (double)pLuminosityHistogram -> pHistogramArray[ nHistogramBin ];
					pLuminosityLookupTable[ nHistogramBin ] = (unsigned short)nNewHistogramBin;
					while ( ResidualHistogramArea >= pLuminosityHistogram -> AverageBinValue )
						{
						ResidualHistogramArea -= pLuminosityHistogram -> AverageBinValue;
						nNewHistogramBin++;
						if ( nNewHistogramBin > MaxGrayscaleValue )
							nNewHistogramBin = MaxGrayscaleValue;
						}
					}
				// Apply the lookup table to the stored image.
				for ( nRow = 0; nRow < nImageRows; nRow++ )
					{
					// Process the row in the image buffer.
					for ( nPixel = 0; nPixel < nImagePixelsPerRow; nPixel++ )
						{
						if ( nBitDepth <= 8 )
							InputPixelValue = (int)( ( (unsigned char*)pInputReadPoint )[ nPixel ] );
						else
							InputPixelValue = (int)( ( (unsigned short*)pInputReadPoint )[ nPixel ] );
						AdjustedPixelValue = pLuminosityLookupTable[ InputPixelValue ];
						if ( nBitDepth <= 8 )
							( (unsigned char*)pInputReadPoint )[ nPixel ] = (unsigned char)AdjustedPixelValue;
						else
							( (unsigned short*)pInputReadPoint )[ nPixel ] = AdjustedPixelValue;
						}
					pInputReadPoint += nImageBytesPerRow;
					}
				free( pLuminosityLookupTable );
				RebuildHistogram();
				m_bALuminosityTransformationHasBeenApplied = TRUE;
				}
			if ( m_ImageView.LoadImageAsTexture() )
				{
				m_ImageView.PrepareImage();
				m_ImageView.RepaintFast();
				}
			}
		m_wndDlgBar.Invalidate();
		m_wndDlgBar.UpdateWindow();
		m_ImageView.Invalidate();
		m_ImageView.UpdateWindow();
		}

	*pResult = 0;
}


// Adjust the image grayscale so that its histogram is evenly distributed
// across the full range of the grayscale.
void CImageFrame::OnButtonCenterHistogram( NMHDR *pNMHDR, LRESULT *pResult )
{
	long					nImageRows;
	long					nImagePixelsPerRow;
	long					nImageBytesPerRow;
	long					nRow;
	long					nPixel;
	unsigned char			*pBuffer;
	unsigned char			*pInputReadPoint;
	int						nBitDepth;
	int						InputPixelValue;
	unsigned short			AdjustedPixelValue;
	int						MaxGrayscaleValue;
	HISTOGRAM_DATA			*pLuminosityHistogram;
	unsigned short			*pLuminosityLookupTable;
	int						nHistogramBin;
	char					Msg[ MAX_EXTRA_LONG_STRING_LENGTH ];
	double					AvgMeasuredLuminosity;
	double					TargetAvgLuminosity;
	double					AParameter;
	double					BParameter;
	double					MeasuredLuminosity;
	double					NewValue;

	if ( BViewerConfiguration.bEnableHistogram )
		{
		if ( m_wndDlgBar.m_ButtonFlattenHistogram.IsWindowEnabled() && m_bALuminosityTransformationHasBeenApplied )
			{
			strncpy_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "You must reload the image before applying another\n", _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
			strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "pixel luminosity transformation.\n", _TRUNCATE );					// *[3] Replaced strcat with strncat_s.
			ThisBViewerApp.NotifyUserToAcknowledgeContinuation( Msg );
			// You can only apply the luminosity redistribution one time.
			m_wndDlgBar.m_ButtonFlattenHistogram.EnableWindow( FALSE );
			m_wndDlgBar.m_ButtonCenterHistogram.EnableWindow( FALSE );
			}
		if ( m_pAssignedDiagnosticImage != 0 && m_pAssignedDiagnosticImage -> m_pImageData != 0 && !m_bALuminosityTransformationHasBeenApplied )
			{
			RebuildHistogram();
			pLuminosityHistogram = &m_pAssignedDiagnosticImage -> m_LuminosityHistogram;
			nImageRows = (long)m_pAssignedDiagnosticImage -> m_ImageHeightInPixels;
			nImagePixelsPerRow = (long)( m_pAssignedDiagnosticImage -> m_ImageWidthInPixels );
			nBitDepth = m_pAssignedDiagnosticImage -> m_ImageBitDepth;
			if ( nBitDepth <= 8 )
				{
				nImageBytesPerRow = nImagePixelsPerRow;
				MaxGrayscaleValue = 255;
				}
			else
				{
				nImageBytesPerRow = nImagePixelsPerRow * 2;
				MaxGrayscaleValue = (int)m_pAssignedDiagnosticImage -> m_MaxGrayscaleValue;
				}
			// Calculate the quadratic transform parameter values to be used for generating the lookup table.
			AvgMeasuredLuminosity = m_ImageView.CalculateGrayscaleHistogramMeanLuminosity();
			TargetAvgLuminosity = 5.0 * (double)MaxGrayscaleValue / 10.0;
			AParameter = ( TargetAvgLuminosity - AvgMeasuredLuminosity ) / ( AvgMeasuredLuminosity * ( AvgMeasuredLuminosity - (double)MaxGrayscaleValue ) );
			BParameter = - (double)MaxGrayscaleValue * ( TargetAvgLuminosity - AvgMeasuredLuminosity ) / ( AvgMeasuredLuminosity * ( AvgMeasuredLuminosity - (double)MaxGrayscaleValue ) );

			pBuffer = m_pAssignedDiagnosticImage -> m_pImageData;
			pInputReadPoint = pBuffer;
			// Build a lookup table that transforms the luminosity distribution so that the average measured value
			// gets moved to the midpoint of the range, while the extreme values don't change at all.
			pLuminosityLookupTable = (unsigned short*)calloc( 1, ( 2 * ( MaxGrayscaleValue + 1 ) ) );
			if ( pLuminosityLookupTable != 0 )
				{
				for ( nHistogramBin = 0; nHistogramBin <= MaxGrayscaleValue; nHistogramBin++ )
					{
					MeasuredLuminosity = (double)nHistogramBin;
					NewValue = AParameter * MeasuredLuminosity * MeasuredLuminosity + ( BParameter + 1.0 ) * MeasuredLuminosity;
					if ( NewValue < 0.0 )
						NewValue = 0.0;
					else if ( NewValue > (double)MaxGrayscaleValue )
						NewValue = (double)MaxGrayscaleValue;
					pLuminosityLookupTable[ nHistogramBin ] = (unsigned short)NewValue;
					}
				// Apply the lookup table to the stored image.
				for ( nRow = 0; nRow < nImageRows; nRow++ )
					{
					// Process the row in the image buffer.
					for ( nPixel = 0; nPixel < nImagePixelsPerRow; nPixel++ )
						{
						if ( nBitDepth <= 8 )
							InputPixelValue = (int)( ( (unsigned char*)pInputReadPoint )[ nPixel ] );
						else
							InputPixelValue = (int)( ( (unsigned short*)pInputReadPoint )[ nPixel ] );

						AdjustedPixelValue = pLuminosityLookupTable[ InputPixelValue ];
						if ( nBitDepth <= 8 )
							( (unsigned char*)pInputReadPoint )[ nPixel ] = (unsigned char)AdjustedPixelValue;
						else
							( (unsigned short*)pInputReadPoint )[ nPixel ] = AdjustedPixelValue;
						}
					pInputReadPoint += nImageBytesPerRow;
					}
				free( pLuminosityLookupTable );
				RebuildHistogram();
				m_bALuminosityTransformationHasBeenApplied = TRUE;
				}
			if ( m_ImageView.LoadImageAsTexture() )
				{
				m_ImageView.PrepareImage();
				m_ImageView.RepaintFast();
				}
			}
		m_wndDlgBar.Invalidate();
		m_wndDlgBar.UpdateWindow();
		m_ImageView.Invalidate();
		m_ImageView.UpdateWindow();
		}

	*pResult = 0;
}


static void ProcessMeasurementToolCalibrationResponse( void *pResponseDialog )
{
	CPopupDialog			*pPopupDialog;
	CString					UserResponseString;
	CImageFrame				*pImageFrame;
	CImageView				*pImageView;
	MEASURED_INTERVAL		*pMeasuredInterval;
	double					CalibrationDistance;

	
	pPopupDialog = (CPopupDialog*)pResponseDialog;
	if ( pPopupDialog != 0 )			// *[1] Added safety check.
		{
		if ( pPopupDialog -> m_pUserNotificationInfo -> UserResponse == POPUP_RESPONSE_SAVE )
			{
			pImageFrame = (CImageFrame*)pPopupDialog -> m_pUserNotificationInfo -> pUserData;
			pImageView = &pImageFrame -> m_ImageView;
			if ( pImageView != 0 )
				{
				pMeasuredInterval = pImageView -> m_pMeasuredIntervalList;
				// Advance to the last entry in the measurement list.
				while( pMeasuredInterval != 0 && pMeasuredInterval -> pNextInterval != 0 )
					pMeasuredInterval = pMeasuredInterval -> pNextInterval;
				if ( pMeasuredInterval != 0 && pImageView -> m_pAssignedDiagnosticImage != 0 )
					{
					pPopupDialog -> m_EditUserTextInput.GetWindowText( UserResponseString );
					strncpy_s( pPopupDialog -> m_pUserNotificationInfo -> UserTextResponse, MAX_CFG_STRING_LENGTH, (const char*)UserResponseString, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
					CalibrationDistance = atof( pPopupDialog -> m_pUserNotificationInfo -> UserTextResponse );
					if ( CalibrationDistance > 1.0 )
						pImageView -> m_PixelsPerMillimeter = pMeasuredInterval -> Distance / CalibrationDistance;
					pImageFrame -> BringWindowToTop();
					pImageView ->Invalidate( TRUE );			// *[4] Update display.
					}
				}
			}
		delete pPopupDialog;
		}
}


void CImageFrame::OnButtonCalibrateMeasurements( NMHDR *pNMHDR, LRESULT *pResult )
{
	MEASURED_INTERVAL				*pMeasuredInterval;
	double							MeasuredLength;
	static USER_NOTIFICATION_INFO	UserNotificationInfo;
	char							TextField[ 64 ];

	pMeasuredInterval = m_ImageView.m_pMeasuredIntervalList;
	// Advance to the last entry in the measurement list.
	while( pMeasuredInterval != 0 && pMeasuredInterval -> pNextInterval != 0 )
		pMeasuredInterval = pMeasuredInterval -> pNextInterval;
	if ( pMeasuredInterval != 0 && m_pAssignedDiagnosticImage != 0 )
		{
		LogMessage( "Calibrating the measurement tool.", MESSAGE_TYPE_NORMAL_LOG );

		// Request the user to enter the distance measured for the most recent screen measurement.
		UserNotificationInfo.WindowWidth = 400;
		UserNotificationInfo.WindowHeight = 300;
		UserNotificationInfo.FontHeight = 0;	// Use default setting;
		UserNotificationInfo.FontWidth = 0;		// Use default setting;
		UserNotificationInfo.UserInputType = USER_INPUT_TYPE_BOOLEAN | USER_INPUT_TYPE_EDIT;
		UserNotificationInfo.pUserNotificationMessage = "Enter the distance for\nthe last measurement\nin millimeters.";
		UserNotificationInfo.CallbackFunction = ProcessMeasurementToolCalibrationResponse;
		UserNotificationInfo.pUserData = (void*)this;

		MeasuredLength = pMeasuredInterval -> Distance / m_ImageView.m_PixelsPerMillimeter;
		_snprintf_s( TextField, 64, _TRUNCATE, "%6.2f", MeasuredLength );									// *[1] Replaced sprintf with _snprintf_s.
		strncpy_s( UserNotificationInfo.UserTextResponse, MAX_CFG_STRING_LENGTH, TextField, _TRUNCATE );	// *[1] Replaced strcpy_s with strncpy_s.
		CWaitCursor			HourGlass;
			
		PerformUserInput( &UserNotificationInfo );
		}
	*pResult = 0;
}


void CImageFrame::OnButtonEnableAnnotations( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( m_ImageView.m_bEnableAnnotations )
		{
		m_ImageView.m_bEnableAnnotations = FALSE;
		m_wndDlgBar.m_ButtonEnableAnnotations.m_ControlText = "Show Study Info";
		}
	else
		{
		m_ImageView.m_bEnableAnnotations = TRUE;
		m_wndDlgBar.m_ButtonEnableAnnotations.m_ControlText = "Hide Study Info";
		}
	m_wndDlgBar.Invalidate();
	m_ImageView.Invalidate();
	*pResult = 0;
}


BOOL CImageFrame::LoadReportPage( int nPageNumber, BOOL *pbUseCurrentStudy )
{
	BOOL					bNoError = TRUE;
	char					ImagePath[ FILE_PATH_STRING_LENGTH ];
	char					ImageFileName[ FILE_PATH_STRING_LENGTH ];
	char					ImageFileExtension[ FILE_PATH_STRING_LENGTH ];
	CDiagnosticImage		*pDiagnosticImage;
	CMainFrame				*pMainFrame;
	CEdit					*pCtrlFileName;
	char					FileSpecForOpening[ FULL_FILE_SPEC_STRING_LENGTH ];
	BOOL					bUseCurrentStudy = TRUE;			// *[2] Initialize variable.
	char					SubjectName[ 256 ];
	CStudy					*pCurrentStudy;
	size_t					nChar;

	if ( m_FrameFunction == IMAGE_FRAME_FUNCTION_REPORT )
		{
		if ( nPageNumber == 2 )
			{
			m_ImageView.m_PageNumber = 2;
			m_wndDlgBar.m_ButtonViewAlternatePage.m_ControlText = "Show Page 1";
			}
		else
			{
			m_ImageView.m_PageNumber = 1;
			m_wndDlgBar.m_ButtonViewAlternatePage.m_ControlText = "Show Page 2";
			}
		m_wndDlgBar.Invalidate();

		pCurrentStudy = 0;
		ImagePath[ 0 ] = '\0';			// *[1] Eliminated call to strcpy.
		// If this is a report associated with the current study, load the blank form.
		if ( ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL && strstr( m_CurrentReportFileName, "GPReport" ) != 0 ) ||
					( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_GENERAL && strstr( m_CurrentReportFileName, "CWHSPReport" ) != 0 ) )
			{
			bUseCurrentStudy = TRUE;
			strncat_s( ImagePath, FILE_PATH_STRING_LENGTH, BViewerConfiguration.ConfigDirectory, _TRUNCATE );	// *[2] Replaced strncat with strncat_s.
			}
		// Otherwise, load a previously-completed report image.
		else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
			{
			bUseCurrentStudy = FALSE;
			strncat_s( ImagePath, FILE_PATH_STRING_LENGTH, BViewerConfiguration.ReportDirectory, _TRUNCATE );	// *[2] Replaced strncat with strncat_s.
			}
		*pbUseCurrentStudy = bUseCurrentStudy;
		if ( ImagePath[ strlen( ImagePath ) - 1 ] != '\\' )
			strncat_s( ImagePath, FILE_PATH_STRING_LENGTH, "\\", _TRUNCATE );						// *[3] Replaced strcat with strncat_s.
		strncpy_s( ImageFileName, FILE_PATH_STRING_LENGTH, m_CurrentReportFileName, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
		if ( !bUseCurrentStudy )
			strncat_s( ImageFileName, FILE_PATH_STRING_LENGTH, "__Report", _TRUNCATE );				// *[3] Replaced strcat with strncat_s.
		if ( nPageNumber == 2 )
			strncat_s( ImageFileName, FILE_PATH_STRING_LENGTH, "Page2", _TRUNCATE );				// *[3] Replaced strcat with strncat_s.
		else
			strncat_s( ImageFileName, FILE_PATH_STRING_LENGTH, "Page1", _TRUNCATE );				// *[3] Replaced strcat with strncat_s.
		strncpy_s( ImageFileExtension, FILE_PATH_STRING_LENGTH, ".png", _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.

		pDiagnosticImage = new CDiagnosticImage();
		pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
		if ( pDiagnosticImage != 0 && pMainFrame != 0 )
			{
			pDiagnosticImage -> m_bEnableOverlays = bUseCurrentStudy;
			strncpy_s( FileSpecForOpening, FULL_FILE_SPEC_STRING_LENGTH, ImagePath, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
			strncat_s( FileSpecForOpening, FULL_FILE_SPEC_STRING_LENGTH, ImageFileName, _TRUNCATE );			// *[2] Replaced strncat with strncat_s.
			strncat_s( FileSpecForOpening, FULL_FILE_SPEC_STRING_LENGTH, ImageFileExtension, _TRUNCATE );		// *[3] Replaced strcat with strncat_s.
			LogMessage( "Reading report form image.", MESSAGE_TYPE_SUPPLEMENTARY );
			bNoError = pDiagnosticImage -> ReadPNGImageFile( FileSpecForOpening, m_pDisplayMonitor, m_FrameFunction );
			if ( bNoError )
				{
				if ( m_pAssignedDiagnosticImage != 0 )
					delete m_pAssignedDiagnosticImage;
				m_pAssignedDiagnosticImage = pDiagnosticImage;
				pDiagnosticImage -> m_OriginalGrayscaleSetting.m_Gamma = 1.0;
				SubjectName[ 0 ] = '\0';																		// *[1] Eliminated call to strcpy.
				if ( bUseCurrentStudy )
					{
					pCurrentStudy = ThisBViewerApp.m_pCurrentStudy;
					if ( pCurrentStudy != 0 )
						{
						strncat_s( SubjectName, FILE_PATH_STRING_LENGTH, pCurrentStudy -> m_PatientLastName, _TRUNCATE );				// *[3] Replaced strcat with strncat_s.
						if ( strlen( pCurrentStudy -> m_PatientLastName ) > 0 && strlen( pCurrentStudy -> m_PatientFirstName ) > 0 )
							strncat_s( SubjectName, FILE_PATH_STRING_LENGTH, ", ", _TRUNCATE );											// *[3] Replaced strcat with strncat_s.
						strncat_s( SubjectName, FILE_PATH_STRING_LENGTH, pCurrentStudy -> m_PatientFirstName, _TRUNCATE );				// *[3] Replaced strcat with strncat_s.
						pDiagnosticImage -> m_OriginalGrayscaleSetting.m_Gamma = pCurrentStudy -> m_GammaSetting;
						}
					}
				else
					{
					strncat_s( SubjectName, FILE_PATH_STRING_LENGTH, m_CurrentReportFileName, _TRUNCATE );								// *[3] Replaced strcat with strncat_s.
					nChar = strlen( SubjectName ) - 16;
					SubjectName[ nChar ] = '\0';
					}
				pCtrlFileName = (CEdit*)m_wndDlgBar.GetDlgItem( IDC_EDIT_IMAGE_NAME );
				pCtrlFileName -> SetWindowText( SubjectName );
				pCtrlFileName -> Invalidate();
				if ( bUseCurrentStudy )
					pCurrentStudy = ThisBViewerApp.m_pCurrentStudy;
				else
					pCurrentStudy = 0;
				m_ImageView.SetDiagnosticImage( pDiagnosticImage, pCurrentStudy );
				SetFocus();
				}
			if ( !bNoError )
				{
				delete pDiagnosticImage;
				m_pAssignedDiagnosticImage = 0;			// *[1]
				LogMessage( ">>> Error in LoadReportPage().", MESSAGE_TYPE_SUPPLEMENTARY );
				}
			else
				LogMessage( "Report image loaded.", MESSAGE_TYPE_SUPPLEMENTARY );
			}
		}
	
	return bNoError;
}


void CImageFrame::OnSetReportPage( NMHDR *pNMHDR, LRESULT *pResult )
{
	BOOL			bUseCurrentStudy;

	if ( m_ImageView.m_PageNumber == 1 )
		m_ImageView.m_PageNumber = 2;
	else
		m_ImageView.m_PageNumber = 1;
	LoadReportPage( m_ImageView.m_PageNumber, &bUseCurrentStudy );
}


void CImageFrame::OnPrintReport( NMHDR *pNMHDR, LRESULT *pResult )
{
	CWaitCursor		DisplaysHourglass;
	BOOL			bUseCurrentStudy;

	m_ImageView.m_PageNumber = 1;
	LoadReportPage( 1, &bUseCurrentStudy );
	if ( m_ImageView.OpenReportForPrinting( TRUE ) )
		{
		m_ImageView.PrintReportPage( bUseCurrentStudy );
		m_ImageView.m_PageNumber = 2;
		LoadReportPage( 2, &bUseCurrentStudy );
		m_ImageView.PrintReportPage( bUseCurrentStudy );
		m_ImageView.CloseReportForPrinting();
		m_ImageView.m_PrinterDC.Detach();

		m_ImageView.m_PageNumber = 1;
		LoadReportPage( 1, &bUseCurrentStudy );
		m_ImageView.Invalidate( TRUE );
		}
}


void CImageFrame::OnSaveReport( NMHDR *pNMHDR, LRESULT *pResult )
{
	CWaitCursor		DisplaysHourglass;
	BOOL			bNoError;
	BOOL			bUseCurrentStudy;

	m_ImageView.m_PageNumber = 1;
	bNoError = LoadReportPage( 1, &bUseCurrentStudy );
	if ( bNoError )
		{
		m_ImageView.SaveReport();
		m_ImageView.m_PageNumber = 2;
		bNoError = LoadReportPage( 2, &bUseCurrentStudy );
		}
	if ( bNoError )
		{
		m_ImageView.SaveReport();
		m_ImageView.m_PageNumber = 1;
		bNoError = LoadReportPage( 1, &bUseCurrentStudy );
		}
	m_bReportSavedSuccessfully = bNoError;
}


void CImageFrame::OnSetImageSize( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( m_ImageView.m_DefaultImageSize == IMAGE_VIEW_FULL_SIZE )
		{
		m_ImageView.m_DefaultImageSize = IMAGE_VIEW_FIT_TO_SCREEN;
		m_wndDlgBar.m_ButtonImageSize.m_ControlText = "Adjust to\nFull Size";
		}
	else
		{
		m_ImageView.m_DefaultImageSize = IMAGE_VIEW_FULL_SIZE;
		m_wndDlgBar.m_ButtonImageSize.m_ControlText = "Adjust to\nFit Screen";
		}
	m_ImageView.ResetDiagnosticImage( TRUE );
	ApplyCurrentWindowingSettings();
	m_wndDlgBar.Invalidate();
}


BOOL CImageFrame::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
{
	m_ImageView.OnMouseWheel( nFlags, zDelta, pt );

	return CFrameWnd::OnMouseWheel(nFlags, zDelta, pt);
}


BOOL CImageFrame::GetEditWindowValue( int EditWindowResourceID, double *pNumericalValue )
{
	BOOL					bNoError = TRUE;
	TomEdit					*pEditControl;
	double					WindowValueEntered;
	char					NumberConvertedToText[ _CVTBUFSIZE ];
	int						nChars;
	int						nChar;
	BOOL					bNonNumericCharEncountered;

	pEditControl = (TomEdit*)m_wndDlgBar.GetDlgItem( EditWindowResourceID );
	if ( pEditControl != 0 && m_pAssignedDiagnosticImage != 0 )
		{
		pEditControl -> GetWindowText( NumberConvertedToText, _CVTBUFSIZE );
		TrimBlanks( NumberConvertedToText );
		bNonNumericCharEncountered = FALSE;
		nChars = (int)strlen( NumberConvertedToText );
		for ( nChar = 0; nChar < nChars; nChar++ )
			if ( !( isdigit( NumberConvertedToText[ nChar ] ) || NumberConvertedToText[ nChar ] == '.' ||
							NumberConvertedToText[ nChar ] == '-' || NumberConvertedToText[ nChar ] == '+' ) )
				bNonNumericCharEncountered = TRUE;
		if ( bNonNumericCharEncountered )
			{
			bNoError = FALSE;
			pEditControl -> SetWindowText( "Error" );
			pEditControl -> Invalidate( TRUE );
			}
		else if ( strlen( NumberConvertedToText ) > 0 )
			{
			WindowValueEntered = atof( NumberConvertedToText );
			*pNumericalValue = WindowValueEntered;
			}
		else
			{
			bNoError = FALSE;
			*pNumericalValue = 0.0;
			}
		}
	else
		bNoError = FALSE;

	return bNoError;
}


void CImageFrame::OnEditGammaKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	TomEdit				*pEditControl;
	double				GammaValueEntered;

	if ( GetEditWindowValue( IDC_EDIT_GAMMA, &GammaValueEntered ) )
		{
		pEditControl = (TomEdit*)m_wndDlgBar.GetDlgItem( IDC_EDIT_GAMMA );
		if ( GammaValueEntered < 0.1 )
			{
			GammaValueEntered = 0.1;
			pEditControl -> SetWindowText( "0.1" );
			pEditControl -> Invalidate( TRUE );
			}
		else if ( GammaValueEntered > 10.0 )
			{
			GammaValueEntered = 10.0;
			pEditControl -> SetWindowText( "10.0" );
			pEditControl -> Invalidate( TRUE );
			}
		if ( !m_pAssignedDiagnosticImage -> m_bEnableGammaCorrection )
			{
			GammaValueEntered = 1.0;
			pEditControl -> SetWindowText( "1.0" );
			pEditControl -> Invalidate( TRUE );
			}
		if ( ThisBViewerApp.m_pCurrentStudy != 0 )
			ThisBViewerApp.m_pCurrentStudy -> m_GammaSetting = GammaValueEntered;
		ApplyCurrentWindowingSettings();
		}

	*pResult = 0;
}


void CImageFrame::UpdateEffectiveWindowCenterValue( double WindowCenterValueEntered )
{
	double					AdjustedMaxPixelValue;
	IMAGE_CALIBRATION_INFO	*pImageCalibrationInfo;
	double					OriginalMaxPixelValue;
	int						nHighBit;

	// Get the scale factors for converting from original to adjusted pixel value scale.
	AdjustedMaxPixelValue = m_pAssignedDiagnosticImage -> m_MaxGrayscaleValue;
	pImageCalibrationInfo = m_pAssignedDiagnosticImage -> m_pImageCalibrationInfo;
	nHighBit = m_pAssignedDiagnosticImage -> m_nHighBit;
	if ( pImageCalibrationInfo != 0 )
		OriginalMaxPixelValue = pow( (double)2, nHighBit + 1 ) - 1;
	else
		OriginalMaxPixelValue = AdjustedMaxPixelValue;

	// Adjust the window center from the original max pixel value to the 16-bit max pixel value.
	m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowCenter =
				( AdjustedMaxPixelValue * ( WindowCenterValueEntered - 0.5 ) / OriginalMaxPixelValue ) + 0.5;
}


void CImageFrame::UpdateEffectiveWindowWidthValue( double WindowWidthValueEntered )
{
	double					AdjustedMaxPixelValue;
	IMAGE_CALIBRATION_INFO	*pImageCalibrationInfo;
	double					OriginalMaxPixelValue;
	int						nHighBit;

	// Get the scale factors for converting from original to adjusted pixel value scale.
	AdjustedMaxPixelValue = m_pAssignedDiagnosticImage -> m_MaxGrayscaleValue;
	pImageCalibrationInfo = m_pAssignedDiagnosticImage -> m_pImageCalibrationInfo;
	nHighBit = m_pAssignedDiagnosticImage -> m_nHighBit;
	if ( pImageCalibrationInfo != 0 )
		OriginalMaxPixelValue = pow( (double)2, nHighBit + 1 ) - 1;
	else
		OriginalMaxPixelValue = AdjustedMaxPixelValue;

	// Adjust the window width from the original max pixel value to the 16-bit max pixel value.
	m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowWidth =
				( AdjustedMaxPixelValue * ( WindowWidthValueEntered - 1.0 ) / OriginalMaxPixelValue ) + 1.0;
}


void CImageFrame::OnEditWindowCenterKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	double					WindowCenterValueEntered;

	if ( GetEditWindowValue( IDC_EDIT_WINDOW_CENTER, &WindowCenterValueEntered ) )
		ApplyCurrentWindowingSettings();

	*pResult = 0;
}


void CImageFrame::OnEditWindowWidthKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	double					WindowWidthValueEntered;

	if ( GetEditWindowValue( IDC_EDIT_WINDOW_WIDTH, &WindowWidthValueEntered ) )
		ApplyCurrentWindowingSettings();

	*pResult = 0;
}


void CImageFrame::ApplyCurrentWindowingSettings()
{
	double					WindowWidthValueEntered;
	double					WindowCenterValueEntered;
	double					GammaValueEntered;

	if ( GetEditWindowValue( IDC_EDIT_GAMMA, &GammaValueEntered ) )
		m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_Gamma = GammaValueEntered;

	if ( GetEditWindowValue( IDC_EDIT_WINDOW_WIDTH, &WindowWidthValueEntered ) )
		UpdateEffectiveWindowWidthValue( WindowWidthValueEntered );
	
	if ( GetEditWindowValue( IDC_EDIT_WINDOW_CENTER, &WindowCenterValueEntered ) )
		UpdateEffectiveWindowCenterValue( WindowCenterValueEntered );

	m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowMinPixelAmplitude =
				m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowCenter - ( m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowWidth - 1.0 ) / 2.0;
	m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowMaxPixelAmplitude =
				m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowCenter - 0.5 + ( m_pAssignedDiagnosticImage -> m_CurrentGrayscaleSetting.m_WindowWidth - 1.0 ) / 2.0;

	m_pAssignedDiagnosticImage -> LoadStudyWindowCenterAndWidth();

	m_ImageView.RepaintFast();
	}


HBRUSH CImageFrame::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
{
    // Return handle to the parent (this) window's background CBrush object
    // for use in painting by the child controls.
	return HBRUSH( m_BkgdBrush );
}


void CImageFrame::OnClose()
{
	ShowWindow( SW_MINIMIZE );
}


static void ProcessEditUserResponse( void *pResponseDialog )
{
	CPopupDialog			*pPopupDialog;
	
	pPopupDialog = (CPopupDialog*)pResponseDialog;
	if ( pPopupDialog != 0 )			// *[1] Added safety check.
		delete pPopupDialog;
}


void CImageFrame::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	static	USER_NOTIFICATION_INFO	UserNotificationInfo;
			BOOL					bRequestChangeWindow;
		 	CMainFrame				*pMainFrame;
			CImageFrame				*pSubjectImageFrame;
			CImageFrame				*pReferenceImageFrame;
			CImageFrame				*pInterpretationReportImageFrame;
			WINDOWPLACEMENT			WindowPlacement;
			RECT					ImageWindowRect;

	bRequestChangeWindow = FALSE;
	if ( bTheLastKeyPressedWasESC )
		{
		bTheLastKeyPressedWasESC = FALSE;
		pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
		if ( pMainFrame != 0 )
			{
			if ( nChar == 's' || nChar == 'S' )				// If the ESC S sequence was pressed,
				{											//  reposition the mouse to the subject study window.
				pSubjectImageFrame = pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ];
				if ( pSubjectImageFrame != 0 )
					{
					pSubjectImageFrame -> GetWindowPlacement( &WindowPlacement );
					if ( WindowPlacement.showCmd == SW_SHOWMINIMIZED )
						{
						WindowPlacement.showCmd = SW_SHOWNORMAL;
						pSubjectImageFrame -> SetWindowPlacement( &WindowPlacement );
						}
					pSubjectImageFrame -> GetWindowRect( &ImageWindowRect );
					// Set the mouse cursor to the screen coordinates where this window is located.
					SetCursorPos( ( ImageWindowRect.left + ImageWindowRect.right ) / 2, ( ImageWindowRect.top + ImageWindowRect.bottom ) / 2 );
					pSubjectImageFrame -> SetFocus();
					}
				bRequestChangeWindow = TRUE;
				}
			else if ( nChar == 'r' || nChar == 'R' )		// If the ESC R sequence was pressed,
				{											//  reposition the mouse to the reference image window.
				pReferenceImageFrame = pMainFrame -> m_pImageFrame[ IMAGE_FRAME_STANDARD ];
				if ( pReferenceImageFrame != 0 )
					{
					pReferenceImageFrame -> GetWindowPlacement( &WindowPlacement );
					if ( WindowPlacement.showCmd == SW_SHOWMINIMIZED )
						{
						WindowPlacement.showCmd = SW_SHOWNORMAL;
						pReferenceImageFrame -> SetWindowPlacement( &WindowPlacement );
						}
					pReferenceImageFrame -> GetWindowRect( &ImageWindowRect );
					// Set the mouse cursor to the screen coordinates where this window is located.
					SetCursorPos( ( ImageWindowRect.left + ImageWindowRect.right ) / 2, ( ImageWindowRect.top + ImageWindowRect.bottom ) / 2 );
					pReferenceImageFrame -> SetFocus();
					}
				bRequestChangeWindow = TRUE;
				}
			else if ( nChar == 'i' || nChar == 'I' )		// If the ESC I sequence was pressed,
				{											//  reposition the mouse to the interpretation report image window.
				pInterpretationReportImageFrame = pMainFrame -> m_pImageFrame[ IMAGE_FRAME_REPORT ];
				if ( pInterpretationReportImageFrame != 0 )
					{
					pInterpretationReportImageFrame -> GetWindowPlacement( &WindowPlacement );
					if ( WindowPlacement.showCmd == SW_SHOWMINIMIZED )
						{
						WindowPlacement.showCmd = SW_SHOWNORMAL;
						pInterpretationReportImageFrame -> SetWindowPlacement( &WindowPlacement );
						}
					pInterpretationReportImageFrame -> GetWindowRect( &ImageWindowRect );
					// Set the mouse cursor to the screen coordinates where this window is located.
					SetCursorPos( ( ImageWindowRect.left + ImageWindowRect.right ) / 2, ( ImageWindowRect.top + ImageWindowRect.bottom ) / 2 );
					pInterpretationReportImageFrame -> SetFocus();
					}
				bRequestChangeWindow = TRUE;
				}
			else if ( nChar == 'c' || nChar == 'C' )		// If the ESC C sequence was pressed,
				{											//  reposition the mouse to the control panel window.
				pMainFrame -> GetWindowPlacement( &WindowPlacement );
				if ( WindowPlacement.showCmd == SW_SHOWMINIMIZED )
					{
					WindowPlacement.showCmd = SW_SHOWNORMAL;
					pMainFrame -> SetWindowPlacement( &WindowPlacement );
					}
				pMainFrame -> GetWindowRect( &ImageWindowRect );
				// Set the mouse cursor to the screen coordinates where this window is located.
				SetCursorPos( ( ImageWindowRect.left + ImageWindowRect.right ) / 2, ( ImageWindowRect.top + ImageWindowRect.bottom ) / 2 );
				pMainFrame -> SetFocus();
				bRequestChangeWindow = TRUE;
				}
			}
		}
	if ( nChar == 0x1B )		// If this is the ESC (escape) key...
		bTheLastKeyPressedWasESC = TRUE;
	
	if ( !bTheLastKeyPressedWasESC && !bRequestChangeWindow )
		{
		if ( m_FrameFunction == IMAGE_FRAME_FUNCTION_REPORT )
			{
			// Inform the user that data cannot be entered directly into the report.
			UserNotificationInfo.WindowWidth = 700;
			UserNotificationInfo.WindowHeight = 300;
			UserNotificationInfo.FontHeight = 18;	// Use default setting;
			UserNotificationInfo.FontWidth = 9;		// Use default setting;
			UserNotificationInfo.UserInputType = USER_INPUT_TYPE_OK;
			UserNotificationInfo.pUserNotificationMessage = "To enter data into the report you must:\n\nStep1:  Enter your data on the \"Enter Interpretation\" tab.\n\nStep2:  Enter your data on the \"Produce Report\" tab.\n\nStep 3:  Click on the \"Show Report\" button.";
			UserNotificationInfo.CallbackFunction = ProcessEditUserResponse;
			PerformUserInput( &UserNotificationInfo );
			}
		else
			CFrameWnd::OnChar( nChar, nRepCnt, nFlags );
		}
}


BOOL CImageFrame::OnNotify( WPARAM wParam, LPARAM lParam, LRESULT *pResult )
{
	return CFrameWnd::OnNotify( wParam, lParam, pResult );
}


void CImageFrame::ClearImageDisplay()
{
	CEdit					*pCtrlFileName;

	pCtrlFileName = (CEdit*)m_wndDlgBar.GetDlgItem( IDC_EDIT_IMAGE_NAME );
	pCtrlFileName -> SetWindowText( "" );
	m_ImageView.m_pAssignedDiagnosticImage = 0;
	m_ImageView.m_Mouse.m_pTargetImage = 0;
	m_ImageView.ResetDiagnosticImage( FALSE );
	m_ImageView.Invalidate( TRUE );
}


void CImageFrame::OnMouseMove(UINT nFlags, CPoint point)
{
	CFrameWnd::OnMouseMove(nFlags, point);
}

void CImageFrame::OnPaint()
{
	m_ImageView.OnPaint();
}
