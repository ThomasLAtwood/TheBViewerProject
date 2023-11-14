// FrameHeader.cpp : Implementation file for the CFrameHeader class
//  of CDialogBar, which implements the header at the top of each image window.
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
#include "FrameHeader.h"


extern CONFIGURATION				BViewerConfiguration;

// CFrameHeader
CFrameHeader::CFrameHeader() :
				m_ButtonExitBViewer( "Exit BViewer", 120, 36, 14, 7, 6, COLOR_WHITE, COLOR_CANCEL, COLOR_CANCEL, COLOR_CANCEL,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, ID_APP_EXIT,
										"Save the current study (if any) and terminate the\n"
										"BViewer application.  If BRetriever is running,\n"
										"it will continue running in the background."  ),
				m_ButtonDeleteCheckedImages( "Remove Checked\nStudies", 150, 36, 14, 7, 6,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_MULTILINE |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_DELETE_IMAGES,
										"Delete the full study of any image whose checkbox is checked.\n"
										"This deletes all the images in the study, not just the checked one.\n"
										"So, for multiple-image studies, you only have to check one to\n"
										"get all the study's images deleted." ),
				m_ButtonImportImages( "Import Studies\nfrom Local Media", 150, 36, 14, 7, 6,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_MULTILINE |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_IMPORT_IMAGES,
										"Click this button to load images from a CD, DVD, flash drive, etc." ),
				m_ButtonShowNewImages( "Add Newly\nArrived Images", 150, 36, 14, 7, 6,
									COLOR_BLACK, COLOR_GREEN, COLOR_GREEN, COLOR_GREEN,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_MULTILINE |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_SHOW_NEW_IMAGES,
										"BRetriever has imported new images.  Click\n"
										"this button to update the image selection list." ),
				m_ButtonShowLogDetail( "Show\nDetailed Log", 150, 36, 14, 7, 6,
									COLOR_LOG_FONT, COLOR_LOG_BKGD, COLOR_LOG_BKGD, COLOR_LOG_BKGD,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_MULTILINE |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_SHOW_LOG_DETAILS,
										"The normal log keeps a list of the studies that have been\n"
										"imported and read.  The detailed log includes lots of\n"
										"technical information, which may be needed in order to\n"
										"diagnose any software or hardware problems." ),
				m_StaticBRetrieverStatus( "BRetriever\nhas Stopped", 80, 36, 14, 7, 5, COLOR_WHITE, COLOR_RED, COLOR_RED,
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE  | CONTROL_MULTILINE,
									IDC_STATIC_BRETRIEVER_STATUS,
										"If BRetriever is not active and correctly importing\n"
										"any studies sent to it or imported from CDs, etc.,\n"
										"you can control it using the \"Control BRetriever\"\n"
										"button on the \"Set Up BViewer\" tab." ),
				m_ButtonEnterManualStudy( "Enter Data to\nCompose a Study", 150, 36, 14, 7, 6,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_MULTILINE |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_ENTER_MANUAL_STUDY,
										"Click this button to enter patient data manually to\n"
										"perform BViewer analysis and reporting on an external\n"
										"(analog) image." ),

				m_EditImageName( "", 250, 20, 16, 8, 5, VARIABLE_PITCH_FONT, COLOR_WHITE, COLOR_PATIENT, COLOR_PATIENT, COLOR_PATIENT,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_TOP_JUSTIFIED | CONTROL_CLIP | CONTROL_VISIBLE,
									EDIT_VALIDATION_NONE, IDC_EDIT_IMAGE_NAME ),

				m_ButtonResetImage( "Reset\nImage", 70, 50, 16, 8, 6, COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_MULTILINE |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_RESET_IMAGE,
										"Restore the image to its original presentation." ),
				m_ButtonClearImage( "Clear\nImage", 70, 50,  16, 8, 6, COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_MULTILINE |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_CLEAR_IMAGE,
										"Clear this image display." ),
				m_ButtonImageSize( "Adjust to\nFull Size", 95, 50,  16, 8, 6, COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_MULTILINE |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_FULL_SIZE,
										"A full size image display requires correct screen width\n"
										"and height settings on the \"Set Up BViewer\" tab." ),

				m_StaticSelectWindowingBehavior( "Windowing:", 80, 20, 14, 7, 5, COLOR_WHITE, COLOR_PATIENT, COLOR_PATIENT,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_SELECT_WINDOWING_BEHAVIOR,
										"\"Linear\" is the usual straight line windowing.\n"
										"\"Sigmoid\" is a nonlinear filter that approximates\n"
										"linear windowing, but doesn't clip the grayscale\n"
										"extremes, gradually compressing them appropriately." ),
				m_ButtonNoWindowing( "Not\nApplied", 70, 50, 14, 7, 6, COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_MULTILINE |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_WINDOWING_NOT_APPLIED,
										"clicking this button changes the windowing values so that\n"
										"there is no grayscale clipping of the original image.\n"
										"Resetting the image will reapply the original windowing." ),
				m_ButtonLinearWindowing( "Linear", 80, 30, 14, 7, 6, COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_CHECKBOX | BUTTON_NO_TOGGLE_OFF | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_LINEAR_WINDOWING,
										"Select the usual linear windowing behavior." ),
				m_ButtonSigmoidWindowing( "Sigmoid", 80, 30, 14, 7, 6, COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_CHECKBOX | BUTTON_NO_TOGGLE_OFF | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_SIGMOID_WINDOWING,
										"Select the sigmoid smoothed windowing behavior." ),
				m_GroupWindowingBehaviorButtons( BUTTON_CHECKBOX, GROUP_SINGLE_SELECT | GROUP_ONE_TOUCHES_ALL, 2,
									&m_ButtonLinearWindowing, &m_ButtonSigmoidWindowing ),
										
				m_StaticGamma( "Gamma:", 80, 20, 14, 7, 5, COLOR_WHITE, COLOR_PATIENT, COLOR_PATIENT,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE,
									IDC_STATIC_GAMMA,
										"Enter a gamma number from 0.1 to 10.0.\n"
										"A value of 1.0 does not change the brightness and contrast linearity,\n"
										"which is appropriate for calibrated display monitors.\n"
										"A value of 2.2 is often a good guess for uncalibrated display monitors." ),
				m_StaticWindowCenter( "Window Center:", 110, 20, 14, 7, 5, COLOR_WHITE, COLOR_PATIENT, COLOR_PATIENT,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE,
									IDC_STATIC_WINDOW_CENTER,
										"Enter a new numerical value, then press the Enter key.\n"
										"Changing the window center and window width can be used\n"
										"to change image brightness and contrast.  You can also do\n"
										"this by holding down the right mouse button while moving\n"
										"the mouse over the image." ),
				m_StaticWindowWidth( "Window Width:", 110, 20, 14, 7, 5, COLOR_WHITE, COLOR_PATIENT, COLOR_PATIENT,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE,
									IDC_STATIC_WINDOW_WIDTH,
										"Enter a new numerical value, then press the Enter key.\n"
										"Changing the window center and window width can be used\n"
										"to change image brightness and contrast.  You can also \n"
										"do this by holding down the right mouse button while\n"
										"moving the mouse over the image." ),
				m_EditGamma( "", 70, 20, 14, 7, 6, VARIABLE_PITCH_FONT,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_TOP_JUSTIFIED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_DECIMAL | EDIT_VALIDATION_DECIMAL_RANGE, IDC_EDIT_GAMMA ),
				m_EditWindowCenter( "", 70, 20, 14, 7, 6, VARIABLE_PITCH_FONT,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_TOP_JUSTIFIED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_DECIMAL | EDIT_VALIDATION_DECIMAL_RANGE, IDC_EDIT_WINDOW_CENTER ),
				m_EditWindowWidth( "", 70, 20, 14, 7, 6, VARIABLE_PITCH_FONT,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_TOP_JUSTIFIED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_DECIMAL | EDIT_VALIDATION_DECIMAL_RANGE, IDC_EDIT_WINDOW_WIDTH ),
				m_ButtonSaveImageSettings( "Save Window Settings", 200, 30, 14, 7, 5, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_SAVE_IMAGE_SETTINGS,
										"Save the current window width/window center and gamma settings\n"
										"for future application to this or other images." ),
				m_ButtonApplySavedImagePreset( "Apply Saved Window Preset", 200, 30, 14, 7, 5, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_APPLY_IMAGE_SETTINGS,
										"Apply one of the saved windowing presets\n"
										"to the currently displayed subject study image." ),
				m_ButtonViewAlternatePage( "Show Page 2", 100, 30, 14, 7, 5, COLOR_BLACK, COLOR_REPORT_SELECTOR, COLOR_REPORT_SELECTOR, COLOR_REPORT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_SET_REPORT_PAGE,
										"View the other page of the 2-page report form." ),
				m_ButtonPrintReport( "Print Report", 100, 30, 14, 7, 5, COLOR_BLACK, COLOR_REPORT_SELECTOR, COLOR_REPORT_SELECTOR, COLOR_REPORT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_PRINT_REPORT,
										"Print a paper copy of the report form pages." ),
				m_StaticNoDataEntryHere( "To enter data into the report, you must first complete\nthe \"Enter Interpretation\" tab, then complete the \"Produce Report\" tab.",
									600, 30, 14, 7, 5, COLOR_BLACK, COLOR_REPORT_HEADER, COLOR_REPORT_HEADER,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE | CONTROL_MULTILINE,
									IDC_STATIC_NO_DATA_ENTRY_HERE ),

				m_ButtonInvertColors( "Invert Colors", 100, 30, 14, 7, 5, COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_INVERT_IMAGE,
										"This may be a little slow.  Also, you may\n"
										"need to readjust brightness and contrast." ),
				m_ButtonRotateImage( "Rotate Image", 100, 30, 14, 7, 5, COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_ROTATE_IMAGE ),
				m_ButtonFlipVertically( "Flip Vertically", 120, 30, 14, 7, 5, COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_FLIP_IMAGE_VERT ),
				m_ButtonFlipHorizontally( "Flip Horizontally", 120, 30, 14, 7, 5,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_FLIP_IMAGE_HORIZ ),
				m_ButtonMeasureDistance( "Measure Distances", 180, 30, 14, 7, 5,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_MEASURE_DISTANCE,
										"Clicking this button enables/disables the use of\n"
										"the right mouse button for measuring distances across\n"
										"the image in millimeters.  After measurements are\n"
										"enabled, you can start a measurment by pressing\n"
										"down the right mouse button over the image starting\n"
										"point, dragging the mouse to the measurement ending\n"
										"point, then releasing the right mouse button." ),
				m_ButtonEraseMeasurements( "Erase Measurements", 180, 30, 14, 7, 5,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_ERASE_MEASUREMENTS,
										"Clicking this button will erase all measurements\n"
										"for this image." ),
				m_ButtonCalibrateMeasurements( "Calibrate Last Measurement", 200, 30, 14, 7, 5,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_CALIBRATE_MEASUREMENT,
										"Clicking this button allows you to override the result\n"
										"of the most recent measurement with a more accurate\n"
										"replacement value that you specify.  All measurements\n"
										"will then be calibrated to this new scale.  The default\n"
										"scale is based on an assumed film size and orientation." ),
				m_ButtonEnableAnnotations( "Hide Study Info", 200, 30, 14, 7, 5,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_ENABLE_ANNOTATIONS,
										"This button clears the descriptive text written onto the image." ),
				m_ButtonShowHistogram( "Show Histogram", 150, 30, 14, 7, 5,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_SHOW_HISTOGRAM,
										"This button calculates an image luminosity histogram to show\n"
										"the current distribution of pixel luminosities." ),
				m_ButtonFlattenHistogram( "Flatten", 70, 30, 14, 7, 5,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_FLATTEN_HISTOGRAM,
										"This button redistributes the pixel luminosities to make\n"
										"the grayscale values equally distributed.  This condenses\n"
										"sparse regions of the grayscale and expands crowded regions." ),
				m_ButtonCenterHistogram( "Center", 70, 30, 14, 7, 5,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_INVISIBLE, IDC_BUTTON_CENTER_HISTOGRAM,
										"This button redistributes the pixel luminosities to move\n"
										"the average luminosity value of the image to mid-range on\n"
										"the display.  A nonlinear adjustment is made to avoid\n"
										"saturating the extreme luminosity values." ),
				m_StaticHistogram( "", 220, 60, 14, 7, 6, COLOR_PATIENT_SELECTOR, COLOR_WHITE, COLOR_WHITE,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE | CONTROL_HISTOGRAM,
									IDC_EDIT_HISTOGRAM )
{
	m_pControlTip = 0;
}


CFrameHeader::~CFrameHeader()
{
	if ( m_pControlTip != 0 )
		{
		delete m_pControlTip;
		m_pControlTip = 0;
		}
}


BEGIN_MESSAGE_MAP( CFrameHeader, CDialogBar )
	//{{AFX_MSG_MAP(CFrameHeader)
	ON_WM_CTLCOLOR()
	ON_WM_CREATE()
	ON_WM_HSCROLL()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


HBRUSH CFrameHeader::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
{
	HBRUSH			hBrush;

	if ( nCtlColor == CTLCOLOR_EDIT )
		{
		switch ( m_FrameFunction )
			{
			case IMAGE_FRAME_FUNCTION_STANDARD:
				pDC -> SetBkColor( COLOR_STD_SELECTOR );
				pDC -> SetTextColor( COLOR_WHITE );
				break;
			case IMAGE_FRAME_FUNCTION_PATIENT:
				pDC -> SetBkColor( COLOR_PATIENT_SELECTOR );
				pDC -> SetTextColor( COLOR_WHITE );
				break;
			case IMAGE_FRAME_FUNCTION_REPORT:
				pDC -> SetBkColor( COLOR_REPORT_HEADER );
				pDC -> SetTextColor( COLOR_BLACK );
				break;
			case IMAGE_FRAME_FUNCTION_CONTROL:
				pDC -> SetBkColor( COLOR_PANEL_BKGD );
				pDC -> SetTextColor( COLOR_PANEL_FONT );
				break;
			}

		pDC -> SetBkMode( OPAQUE );
		hBrush = HBRUSH( m_BkgdBrush );
		}
	else
		hBrush = HBRUSH( m_BkgdBrush );

	return hBrush;
}


int CFrameHeader::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if (CDialogBar::OnCreate( lpCreateStruct ) == -1)
		return -1;

	m_EditImageName.SetPosition( 5, 5, this );
	switch ( m_FrameFunction )
		{
		case IMAGE_FRAME_FUNCTION_STANDARD:
			m_EditImageName.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );

			m_ButtonResetImage.SetPosition( 10, 25, this );
			m_ButtonResetImage.m_IdleBkgColor = COLOR_STD_SELECTOR;
			m_ButtonResetImage.m_OriginalIdleBkgColor = COLOR_STD_SELECTOR;
			m_ButtonResetImage.m_ActivatedBkgdColor = COLOR_STD_SELECTOR;
			m_ButtonResetImage.m_VisitedBkgdColor = COLOR_STD_SELECTOR;
			m_ButtonResetImage.RecomputePressedColor();
			m_ButtonResetImage.m_ControlStyle &= ~CONTROL_INVISIBLE;
			m_ButtonResetImage.m_ControlStyle |= CONTROL_VISIBLE;

			m_ButtonClearImage.SetPosition( 80, 25, this );
			m_ButtonClearImage.m_IdleBkgColor = COLOR_STD_SELECTOR;
			m_ButtonClearImage.m_OriginalIdleBkgColor = COLOR_STD_SELECTOR;
			m_ButtonClearImage.m_ActivatedBkgdColor = COLOR_STD_SELECTOR;
			m_ButtonClearImage.m_VisitedBkgdColor = COLOR_STD_SELECTOR;
			m_ButtonClearImage.RecomputePressedColor();
			m_ButtonClearImage.m_ControlStyle &= ~CONTROL_INVISIBLE;
			m_ButtonClearImage.m_ControlStyle |= CONTROL_VISIBLE;

			m_ButtonImageSize.SetPosition( 150, 25, this );
			m_ButtonImageSize.m_ControlStyle &= ~CONTROL_INVISIBLE;
			m_ButtonImageSize.m_ControlStyle |= CONTROL_VISIBLE;
			m_ButtonImageSize.m_IdleBkgColor = COLOR_STD_SELECTOR;
			m_ButtonImageSize.m_OriginalIdleBkgColor = COLOR_STD_SELECTOR;
			m_ButtonImageSize.m_ActivatedBkgdColor = COLOR_STD_SELECTOR;
			m_ButtonImageSize.m_VisitedBkgdColor = COLOR_STD_SELECTOR;
			m_ButtonImageSize.RecomputePressedColor();

			m_EditImageName.m_IdleBkgColor = COLOR_STANDARD;
			m_EditImageName.m_OriginalIdleBkgColor = COLOR_STANDARD;
			m_EditImageName.m_ActivatedBkgdColor = COLOR_STANDARD;
			m_EditImageName.m_VisitedBkgdColor = COLOR_STANDARD;
			break;

		case IMAGE_FRAME_FUNCTION_PATIENT:
			m_EditImageName.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );

			m_ButtonResetImage.SetPosition( 10, 25, this );
			m_ButtonResetImage.m_ControlStyle &= ~CONTROL_INVISIBLE;
			m_ButtonResetImage.m_ControlStyle |= CONTROL_VISIBLE;

			m_ButtonClearImage.SetPosition( 85, 25, this );
			m_ButtonClearImage.m_ControlStyle &= ~CONTROL_INVISIBLE;
			m_ButtonClearImage.m_ControlStyle |= CONTROL_VISIBLE;

			m_ButtonImageSize.SetPosition( 160, 25, this );
			m_ButtonImageSize.m_ControlStyle &= ~CONTROL_INVISIBLE;
			m_ButtonImageSize.m_ControlStyle |= CONTROL_VISIBLE;

			m_StaticSelectWindowingBehavior.SetPosition( 260, 5, this );
			m_ButtonNoWindowing.SetPosition( 265, 25, this );
			m_ButtonLinearWindowing.SetPosition( 340, 10, this );
			m_ButtonSigmoidWindowing.SetPosition( 340, 45, this );

			m_StaticWindowCenter.SetPosition( 430, 5, this );
			m_StaticWindowWidth.SetPosition( 430, 30, this );
			m_StaticGamma.SetPosition( 430, 55, this );
			m_EditWindowCenter.SetPosition( 535, 5, this );
			m_EditWindowCenter.SetDecimalRange( 0.0, 65536.0, 1 );
			m_EditWindowWidth.SetPosition( 535, 30, this );
			m_EditWindowWidth.SetDecimalRange( 0.0, 65536.0, 1 );
			m_EditGamma.SetPosition( 535, 55, this );
			m_EditGamma.SetDecimalRange( 0.1, 10.0, 1 );
			m_StaticGamma.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
			m_StaticWindowCenter.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
			m_StaticWindowWidth.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
			m_EditGamma.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
			m_EditWindowCenter.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
			m_EditWindowWidth.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );

			m_ButtonInvertColors.SetPosition( 615, 10, this );
			m_ButtonInvertColors.m_ControlStyle &= ~CONTROL_INVISIBLE;
			m_ButtonInvertColors.m_ControlStyle |= CONTROL_VISIBLE;

			m_ButtonRotateImage.SetPosition( 615, 45, this );
			m_ButtonRotateImage.m_ControlStyle &= ~CONTROL_INVISIBLE;
			m_ButtonRotateImage.m_ControlStyle |= CONTROL_VISIBLE;

			m_ButtonFlipVertically.SetPosition( 720, 10, this );
			m_ButtonFlipVertically.m_ControlStyle &= ~CONTROL_INVISIBLE;
			m_ButtonFlipVertically.m_ControlStyle |= CONTROL_VISIBLE;

			m_ButtonFlipHorizontally.SetPosition( 720, 45, this );
			m_ButtonFlipHorizontally.m_ControlStyle &= ~CONTROL_INVISIBLE;
			m_ButtonFlipHorizontally.m_ControlStyle |= CONTROL_VISIBLE;

			m_ButtonMeasureDistance.SetPosition( 860, 10, this );
			m_ButtonMeasureDistance.m_ControlStyle &= ~CONTROL_INVISIBLE;
			m_ButtonMeasureDistance.m_ControlStyle |= CONTROL_VISIBLE;

			m_ButtonEraseMeasurements.SetPosition( 860, 45, this );
			m_ButtonEraseMeasurements.m_ControlStyle &= ~CONTROL_INVISIBLE;
			m_ButtonEraseMeasurements.m_ControlStyle |= CONTROL_VISIBLE;

			m_ButtonCalibrateMeasurements.SetPosition( 1050, 10, this );
			m_ButtonCalibrateMeasurements.m_ControlStyle &= ~CONTROL_INVISIBLE;
			m_ButtonCalibrateMeasurements.m_ControlStyle |= CONTROL_VISIBLE;

			m_ButtonEnableAnnotations.SetPosition( 1050, 45, this );
			m_ButtonEnableAnnotations.m_ControlStyle &= ~CONTROL_INVISIBLE;
			m_ButtonEnableAnnotations.m_ControlStyle |= CONTROL_VISIBLE;
			if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_TEST )
				m_ButtonEnableAnnotations.m_ControlText = "Show Study Info";
			else if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
				m_ButtonEnableAnnotations.m_ControlText = "Hide Study Info";

			m_ButtonSaveImageSettings.SetPosition( 1270, 10, this );
			m_ButtonSaveImageSettings.m_ControlStyle &= ~CONTROL_INVISIBLE;
			m_ButtonSaveImageSettings.m_ControlStyle |= CONTROL_VISIBLE;
			m_ButtonSaveImageSettings.m_IdleBkgColor = COLOR_PATIENT_SELECTOR;
			m_ButtonSaveImageSettings.m_OriginalIdleBkgColor = COLOR_PATIENT_SELECTOR;
			m_ButtonSaveImageSettings.m_ActivatedBkgdColor = COLOR_PATIENT_SELECTOR;
			m_ButtonSaveImageSettings.m_VisitedBkgdColor = COLOR_PATIENT_SELECTOR;

			m_ButtonApplySavedImagePreset.SetPosition( 1270, 45, this );
			m_ButtonApplySavedImagePreset.m_ControlStyle &= ~CONTROL_INVISIBLE;
			m_ButtonApplySavedImagePreset.m_ControlStyle |= CONTROL_VISIBLE;
			m_ButtonApplySavedImagePreset.m_IdleBkgColor = COLOR_PATIENT_SELECTOR;
			m_ButtonApplySavedImagePreset.m_OriginalIdleBkgColor = COLOR_PATIENT_SELECTOR;
			m_ButtonApplySavedImagePreset.m_ActivatedBkgdColor = COLOR_PATIENT_SELECTOR;
			m_ButtonApplySavedImagePreset.m_VisitedBkgdColor = COLOR_PATIENT_SELECTOR;

			if ( BViewerConfiguration.bEnableHistogram )
				{
				m_StaticHistogram.SetPosition( 1640, 12, this );
				m_StaticHistogram.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
				m_ButtonShowHistogram.SetPosition( 1480, 10, this );
				m_ButtonShowHistogram.m_ControlStyle &= ~CONTROL_INVISIBLE;
				m_ButtonShowHistogram.m_ControlStyle |= CONTROL_VISIBLE;
				m_ButtonFlattenHistogram.SetPosition( 1480, 45, this );
				m_ButtonFlattenHistogram.m_ControlStyle &= ~CONTROL_INVISIBLE;
				m_ButtonFlattenHistogram.m_ControlStyle |= CONTROL_VISIBLE;
				m_ButtonCenterHistogram.SetPosition( 1560, 45, this );
				m_ButtonCenterHistogram.m_ControlStyle &= ~CONTROL_INVISIBLE;
				m_ButtonCenterHistogram.m_ControlStyle |= CONTROL_VISIBLE;
				}
			break;

		case IMAGE_FRAME_FUNCTION_REPORT:
			m_EditImageName.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );

			m_ButtonResetImage.SetPosition( 10, 25, this );
			m_ButtonResetImage.m_ControlStyle &= ~CONTROL_INVISIBLE;
			m_ButtonResetImage.m_ControlStyle |= CONTROL_VISIBLE;
			m_ButtonResetImage.m_IdleBkgColor = COLOR_REPORT_SELECTOR;
			m_ButtonResetImage.m_OriginalIdleBkgColor = COLOR_REPORT_SELECTOR;
			m_ButtonResetImage.m_ActivatedBkgdColor = COLOR_REPORT_SELECTOR;
			m_ButtonResetImage.m_VisitedBkgdColor = COLOR_REPORT_SELECTOR;
			m_ButtonResetImage.m_TextColor = COLOR_BLACK;
			m_ButtonResetImage.RecomputePressedColor();

			m_ButtonClearImage.SetPosition( 80, 25, this );
			m_ButtonClearImage.m_ControlStyle &= ~CONTROL_INVISIBLE;
			m_ButtonClearImage.m_ControlStyle |= CONTROL_VISIBLE;
			m_ButtonClearImage.m_IdleBkgColor = COLOR_REPORT_SELECTOR;
			m_ButtonClearImage.m_OriginalIdleBkgColor = COLOR_REPORT_SELECTOR;
			m_ButtonClearImage.m_ActivatedBkgdColor = COLOR_REPORT_SELECTOR;
			m_ButtonClearImage.m_VisitedBkgdColor = COLOR_REPORT_SELECTOR;
			m_ButtonClearImage.m_TextColor = COLOR_BLACK;
			m_ButtonClearImage.RecomputePressedColor();

			m_ButtonViewAlternatePage.SetPosition( 150, 45, this );
			m_ButtonViewAlternatePage.m_ControlStyle &= ~CONTROL_INVISIBLE;
			m_ButtonViewAlternatePage.m_ControlStyle |= CONTROL_VISIBLE;

			m_ButtonPrintReport.SetPosition( 260, 45, this );
			m_ButtonPrintReport.m_ControlStyle &= ~CONTROL_INVISIBLE;
			m_ButtonPrintReport.m_ControlStyle |= CONTROL_VISIBLE;

			m_StaticNoDataEntryHere.SetPosition( 300, 8, this );
			m_StaticNoDataEntryHere.ChangeStatus( CONTROL_VISIBLE, CONTROL_VISIBLE );
			break;

		case IMAGE_FRAME_FUNCTION_CONTROL:			// This is the control panel header.
			m_ButtonExitBViewer.SetPosition( 780, 5, this );
			m_ButtonExitBViewer.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );

			m_ButtonShowLogDetail.SetPosition( 300, 5, this );
			m_ButtonShowLogDetail.EnableWindow( FALSE );
			m_ButtonShowLogDetail.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
			
			m_ButtonDeleteCheckedImages.SetPosition( 300, 5, this );
			m_ButtonDeleteCheckedImages.EnableWindow( FALSE );
			m_ButtonDeleteCheckedImages.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
			
			m_ButtonImportImages.SetPosition( 460, 5, this );
			m_ButtonImportImages.EnableWindow( FALSE );
			m_ButtonImportImages.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );

			m_ButtonShowNewImages.SetPosition( 620, 5, this );
			m_ButtonShowNewImages.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
			
			m_StaticBRetrieverStatus.SetPosition( 910, 5, this );
			m_StaticBRetrieverStatus.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );

			m_ButtonEnterManualStudy.SetPosition( 1000, 5, this );
			m_ButtonEnterManualStudy.EnableWindow( FALSE );
			m_ButtonEnterManualStudy.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );

			m_EditImageName.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
			m_EditImageName.m_IdleBkgColor = COLOR_PATIENT;
			m_EditImageName.m_OriginalIdleBkgColor = COLOR_PATIENT;
			m_EditImageName.m_ActivatedBkgdColor = COLOR_PATIENT;
			m_EditImageName.m_VisitedBkgdColor = COLOR_PATIENT;
			m_EditImageName.m_TextColor = COLOR_PANEL_FONT;
			break;
		}
	InitializeControlTips();

	return 0;
}


static void ControlTipActivationFunction( CWnd *pDialogWindow, char *pTipText, CPoint MouseCursorLocation )
{
	CFrameHeader			*pFrameHeader;

	pFrameHeader = (CFrameHeader*)pDialogWindow;
	if ( pFrameHeader != 0 )
		{
		// If there has been a change in the tip text, reset the tip display window.
		if ( pTipText != 0 && strlen( pTipText ) > 0 && pTipText != pFrameHeader -> m_pControlTip -> m_pTipText &&
																	pFrameHeader -> m_pControlTip -> m_pTipText != 0 )
			{
			pFrameHeader -> m_pControlTip -> ShowWindow( SW_HIDE );
			pFrameHeader -> m_pControlTip -> m_pTipText = pTipText;
			pFrameHeader -> m_pControlTip -> ShowTipText( MouseCursorLocation, pFrameHeader );
			}
		else if ( pTipText == 0 )
			pFrameHeader -> m_pControlTip -> ShowWindow( SW_HIDE );
		else
			{
			pFrameHeader -> m_pControlTip -> m_pTipText = pTipText;
			pFrameHeader -> m_pControlTip -> ShowTipText( MouseCursorLocation, pFrameHeader );
			}
		}
}


void CFrameHeader::InitializeControlTips()
{
	CWnd					*pChildWindow;
	CRuntimeClass			*pRuntimeClassInfo;

	m_pControlTip = new CControlTip();
	if ( m_pControlTip != 0 )
		{
		m_pControlTip -> ActivateTips();
		pChildWindow = GetWindow( GW_CHILD );		// Get the first child window.
		while ( pChildWindow != NULL )
			{
			pRuntimeClassInfo = pChildWindow -> GetRuntimeClass();
			if ( pRuntimeClassInfo != 0 && 
						( _stricmp( pRuntimeClassInfo -> m_lpszClassName, "TomButton" ) == 0 ||
						_stricmp( pRuntimeClassInfo -> m_lpszClassName, "TomStatic" ) == 0 ) )
				if ( ( (TomButton*)pChildWindow ) -> IsVisible()
								// The following buttons come and go, depending upon the tab.
								// They may not be visible when this function runs.
								|| (TomButton*)pChildWindow == &m_ButtonDeleteCheckedImages
								|| (TomButton*)pChildWindow == &m_ButtonImportImages
								|| (TomButton*)pChildWindow == &m_ButtonEnterManualStudy
								|| (TomButton*)pChildWindow == &m_ButtonShowLogDetail )
					( (TomButton*)pChildWindow ) -> m_ControlTipActivator = ControlTipActivationFunction;
			pChildWindow = pChildWindow -> GetWindow( GW_HWNDNEXT );
			}
		}
}

void CFrameHeader::OnMouseMove( UINT nFlags, CPoint MouseCursorLocation )
{
	// If the window owning the controls (this window) receives a WM_MOUSEMOVE message, this
	// indicates that the mouse is not over any of the controls.  Therefore, disable the
	// control tip.
	if ( m_pControlTip -> m_pTipText != 0 && strlen( m_pControlTip -> m_pTipText ) > 0 )
		{
		m_pControlTip -> m_pTipText = "";
		m_pControlTip -> ShowTipText( MouseCursorLocation, this );
		}
	
	CDialogBar::OnMouseMove( nFlags, MouseCursorLocation );
}




