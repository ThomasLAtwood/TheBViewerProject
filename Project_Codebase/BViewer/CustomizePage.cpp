// CustomizePage.cpp : Implementation file for the CCustomizePage class of
//	CPropertyPage, which implements the "Set Up BViewer" tab of the main Control Panel.
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
//	*[6] 01/19/2024 by Tom Atwood
//		Added acceptable extensions for data file clearing.  The recent
//		security fix was too severe.  Added .sdy files for deleting.
//	*[5] 08/29/2023 by Tom Atwood
//		Improved user editing.  Removed m_ButtonAddUser and
//		m_ButtonSaveBViewerConfiguration buttons, modified edit user button.
//		Moved user field editing functions to the ReaderInfoScreen module.
//		Added EditUserInfo() function.  Removed LoadCountrySelectionList() to
//		ReaderInfoScreen.cpp.  Changed the reader info edit fields to read-only.
//		Removed the OnEditFocus functions since the reader info edit fields
//		are being made read-only.  Equivalent functionality is now in a single
//		function in ReaderInfoScreen.cpp.  Replaced country selection list with
//		m_EditReaderCountry.  Removed redundant call to ResetPage().
//		Removed pCurrentReaderInfo pointer so that pBViewerCustomization -> m_ReaderInfo
//		becomes the unique identifier for the current reader.
//	*[4] 07/17/2023 by Tom Atwood
//		Fixed code security issues.
//	*[3] 04/11/2023 by Tom Atwood
//		Added the GetRenderingMethodText() function.
//	*[2] 03/13/2023 by Tom Atwood
//		Fixed code security issues.
//	*[1] 12/23/2022 by Tom Atwood
//		Fixed code security issues.
//
//
#include "stdafx.h"
#include <process.h>
#include <Iphlpapi.h>
#include "BViewer.h"
#include "Module.h"
#include "ReportStatus.h"
#include "Configuration.h"
#include "Access.h"
#include "CustomizePage.h"
#include "DiagnosticImage.h"
#include "Mouse.h"
#include "ImageView.h"
#include "MainFrm.h"
#include "StandardSelector.h"
#include "TextWindow.h"
#include "ReaderInfoScreen.h"
#include "SelectUser.h"				// *[5] Added include file.


extern CBViewerApp					ThisBViewerApp;
extern CONFIGURATION				BViewerConfiguration;
extern CCustomization				BViewerCustomization;
extern LIST_HEAD					RegisteredUserList;
extern CString						ExplorerWindowClass;
extern CString						PopupWindowClass;
extern BOOL							bMakeDumbButtons;
extern BOOL							bOKToSaveReaderInfo;
extern READER_PERSONAL_INFO			LoggedInReaderInfo;			// Saved reader info, used for restoring overwrites from imported studies.

static BOOL							bReaderInfoQuestionHasBeenAsked = FALSE;

// CCustomizePage dialog
CCustomizePage::CCustomizePage() : CPropertyPage( CCustomizePage::IDD ),
				m_StaticImagePlacement( "Image Placement", 180, 20, 18, 9, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_TOP_JUSTIFIED | CONTROL_VISIBLE,
									IDC_STATIC_IMAGE_PLACEMENT,
										"Here you designate which display monitors\n"
										"will display the reference and the study images."  ),
				m_StaticShowStdImageOn( "Select Reference\nImage Monitor:", 140, 40, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_SHOW_STD_IMAGE_ON,
										"Select the display monitor you want to use to display\n"
										"the standard reference images.  The \"primary\" monitor\n"
										"is your default Windows desktop monitor." ),
				m_ButtonStdDisplayAuto( "Auto Select", 140, 30, 14, 7, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_CHECKBOX | BUTTON_NO_TOGGLE_OFF | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_STD_DISPLAY_AUTO,
										"Let BViewer survey the available display\n"
										"monitors and decide which to use for the\n"
										"standard reference images." ),
				m_ButtonStdDisplayPrimary( "Primary", 140, 30, 14, 7, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_CHECKBOX | BUTTON_NO_TOGGLE_OFF | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_STD_DISPLAY_PRIMARY,
										"Show the standard reference images\n"
										"on the primary (desktop) monitor." ),
				m_ButtonStdDisplayMonitor2( "Monitor 2", 140, 30, 14, 7, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_CHECKBOX | BUTTON_NO_TOGGLE_OFF | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_STD_DISPLAY_MONITOR2,
										"Show the standard reference images\n"
										"on the second monitor.  This should\n"
										"be a medical image monitor." ),
				m_ButtonStdDisplayMonitor3( "Monitor 3", 140, 30, 14, 7, 6, COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_CHECKBOX | BUTTON_NO_TOGGLE_OFF | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_STD_DISPLAY_MONITOR3,
										"Show the standard reference images\n"
										"on the third monitor.  This should\n"
										"be a medical image monitor." ),
				m_GroupStdDisplayButtons( BUTTON_CHECKBOX, GROUP_SINGLE_SELECT | GROUP_ONE_TOUCHES_ALL, 4,
									&m_ButtonStdDisplayAuto, &m_ButtonStdDisplayPrimary, &m_ButtonStdDisplayMonitor2, &m_ButtonStdDisplayMonitor3 ),

				m_StaticShowStudyImageOn( "Select Study\nImage Monitor:", 140, 40, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_SHOW_STUDY_IMAGE_ON,
										"Select the display monitor you want to use to display\n"
										"the subject study images.  The \"primary\" monitor\n"
										"is your default Windows desktop monitor." ),
				m_ButtonStudyDisplayAuto( "Auto Select", 140, 30, 14, 7, 6, COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_CHECKBOX | BUTTON_NO_TOGGLE_OFF | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_STUDY_DISPLAY_AUTO,
										"Let BViewer survey the available display\n"
										"monitors and decide which to use for the\n"
										"subject study images." ),
				m_ButtonStudyDisplayPrimary( "Primary", 140, 30, 14, 7, 6, COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_CHECKBOX | BUTTON_NO_TOGGLE_OFF | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_STUDY_DISPLAY_PRIMARY,
										"Show the subject study images\n"
										"on the primary (desktop) monitor." ),
				m_ButtonStudyDisplayMonitor2( "Monitor 2", 140, 30, 14, 7, 6, COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_CHECKBOX | BUTTON_NO_TOGGLE_OFF | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_STUDY_DISPLAY_MONITOR2,
										"Show the subject study images\n"
										"on the second monitor.  This should\n"
										"be a medical image monitor." ),
				m_ButtonStudyDisplayMonitor3( "Monitor 3", 140, 30, 14, 7, 6, COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_CHECKBOX | BUTTON_NO_TOGGLE_OFF | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_STUDY_DISPLAY_MONITOR3,
										"Show the subject study images\n"
										"on the third monitor.  This should\n"
										"be a medical image monitor." ),
				m_GroupStudyDisplayButtons( BUTTON_CHECKBOX, GROUP_SINGLE_SELECT | GROUP_ONE_TOUCHES_ALL, 4,
									&m_ButtonStudyDisplayAuto, &m_ButtonStudyDisplayPrimary, &m_ButtonStudyDisplayMonitor2, &m_ButtonStudyDisplayMonitor3 ),

				m_StaticStudySelection( "Study\nSelection List", 150, 40, 18, 9, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE | CONTROL_VISIBLE,
									IDC_STATIC_STUDY_SELECTION,
										"Select the amount and type of detail to be\n"
										"displayed in the subject study selection list." ),
				m_StaticStudySelectionDisplayEmphasis( "Emphasize\nInformation\nRelated To:", 120, 60, 14, 7, 6, 0x00000000, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_SELECT_EMPHASIS ),
				m_ButtonShowSummaryInfo( "Summary", 120, 30, 14, 7, 6, COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_CHECKBOX | BUTTON_NO_TOGGLE_OFF | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_EMPHASIZE_PATIENT,
										"Only list information related to the subject\n"
										"of the study." ),
				m_ButtonShowStudyInfo( "Study", 120, 30, 14, 7, 6, COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_CHECKBOX | BUTTON_NO_TOGGLE_OFF | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_EMPHASIZE_STUDY,
										"List information about the study, but not\n"
										"the series or the image." ),
				m_ButtonShowSeriesInfo( "Series", 120, 30, 14, 7, 6, COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_CHECKBOX | BUTTON_NO_TOGGLE_OFF | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_EMPHASIZE_SERIES,
										"List information about the study and the\n"
										"series, but not the image details." ),
				m_ButtonShowImageInfo( "Image", 120, 30, 14, 7, 6, COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_CHECKBOX | BUTTON_NO_TOGGLE_OFF | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_EMPHASIZE_IMAGE,
										"List all the information about each image, including\n"
										"the subject, study and series information." ),
				m_GroupShowInfoButtons( BUTTON_CHECKBOX, GROUP_SINGLE_SELECT | GROUP_ONE_TOUCHES_ALL, 4,
									&m_ButtonShowSummaryInfo, &m_ButtonShowStudyInfo, &m_ButtonShowSeriesInfo, &m_ButtonShowImageInfo ),

				m_StaticImageFullSizeAdjust( "Image Full Size\nDisplay Adjustment", 200, 40, 18, 9, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE | CONTROL_VISIBLE,
									IDC_STATIC_IMAGE_SIZE_ADJUST,
										"Enter screen dimensions in millimeters to calibrate\n"
										"the \"Adjust to Full Size\" button at the top of the\n"
										"image display window.  Enter the full width and height\n"
										"of the visible screen." ),
				m_StaticPrimaryMonitor( "Primary (Desktop) Monitor:", 200, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_MONITOR_PRIMARY ),
				m_StaticMonitor2( "Monitor 2 (Image):", 200, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_MONITOR2 ),
				m_StaticMonitor3( "Monitor 3 (Image):", 200, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_MONITOR3 ),

				m_StaticScreenWidthInMillimeters( "Measured\nScreen Width\n(Millimeters)", 120, 50, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_SET_SCREEN_WIDTH,
										"Enter screen width in millimeters to calibrate the\n"
										"\"Adjust to Full Size\" button at the top of the\n"
										"image display window.  Enter the full width of the\n"
										"visible screen." ),
				m_EditPrimaryMonitorWidth( "", 60, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_NUMERIC, IDC_EDIT_PRIMARY_MONITOR_WIDTH ),
				m_EditMonitor2Width( "", 60, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_NUMERIC, IDC_EDIT_MONITOR2_WIDTH ),
				m_EditMonitor3Width( "", 60, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_NUMERIC, IDC_EDIT_MONITOR3_WIDTH ),

				m_StaticScreenHeightInMillimeters( "Measured\nScreen Height\n(Millimeters)", 120, 50, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_SET_SCREEN_HEIGHT,
										"Enter screen height in millimeters to calibrate the\n"
										"\"Adjust to Full Size\" button at the top of the\n"
										"image display window.  Enter the full height of the\n"
										"visible screen." ),
				m_EditPrimaryMonitorHeight( "", 60, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_NUMERIC, IDC_EDIT_PRIMARY_MONITOR_HEIGHT ),
				m_EditMonitor2Height( "", 60, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_NUMERIC, IDC_EDIT_MONITOR2_HEIGHT ),
				m_EditMonitor3Height( "", 60, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_NUMERIC, IDC_EDIT_MONITOR3_HEIGHT ),

				m_StaticGrayscaleResolution( "Display\nRendering Capability", 250, 40, 18, 9, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE | CONTROL_VISIBLE,
									IDC_STATIC_GRAYSCALE_RESOLUTION,
										"Specify the rendering capability of each display panel.\n"
										"Most garden variety panels display conventional 8-bit color.\n"
										"Special-purpose grayscale panels receive packed 10-bit grayscale.\n"
										"Modern color panels are capable of rendering 30-bit color." ),
				m_StaticGrayscaleBitDepth( "Select the Image\nRendering Method Supported\nBy Each Display", 250, 50, 14, 7, 6,
									COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_GRAYSCALE_BIT_DEPTH,
										"Specify the rendering capability of each display panel.\n"
										"Most garden variety panels display conventional 8-bit color.\n"
										"Special-purpose grayscale panels receive packed 10-bit grayscale.\n"
										"Modern color panels are capable of rendering 30-bit color." ),
				m_ComboBoxSelectPrimaryMonitorRenderingMethod( "", 280, 300, 18, 9, 5, VARIABLE_PITCH_FONT,
										COLOR_BLACK, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_VSCROLL | EDIT_BORDER | CONTROL_VISIBLE,
										EDIT_VALIDATION_NONE, IDC_SELECT_PRIMARY_MONITOR_RENDERING_METHOD ),
				m_ComboBoxSelectMonitor2RenderingMethod( "", 280, 300, 18, 9, 5, VARIABLE_PITCH_FONT,
										COLOR_BLACK, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_VSCROLL | EDIT_BORDER | CONTROL_VISIBLE,
										EDIT_VALIDATION_NONE, IDC_SELECT_MONITOR2_RENDERING_METHOD ),
				m_ComboBoxSelectMonitor3RenderingMethod( "", 280, 300, 18, 9, 5, VARIABLE_PITCH_FONT,
										COLOR_BLACK, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_VSCROLL | EDIT_BORDER | CONTROL_VISIBLE,
										EDIT_VALIDATION_NONE, IDC_SELECT_MONITOR3_RENDERING_METHOD ),

				m_StaticReaderIdentification( "Current Reader Information", 320, 20, 18, 9, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_TOP_JUSTIFIED | CONTROL_VISIBLE,
									IDC_STATIC_READER_IDENTIFICATION,
										"Enter the information describing the reader.\n"
										"Some of this information is included on each report." ),
				m_StaticReaderLastName( "Last Name (Family Name)", 200, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_READER_LAST_NAME,
										"Enter this reader's last name." ),
				m_EditReaderLastName( "", 160, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | EDIT_READONLY | CONTROL_VISIBLE,		// *[5] Set read-only.
									EDIT_VALIDATION_NONE, IDC_EDIT_READER_LAST_NAME ),

				m_StaticLoginName( "Login Name", 100, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_LOGIN_NAME,
										"This is the user name that you will\n"
										"provide when you log into BViewer." ),
				m_EditLoginName( "", 120, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | EDIT_READONLY | CONTROL_VISIBLE,		// *[5] Set read-only.
									EDIT_VALIDATION_NONE, IDC_EDIT_LOGIN_NAME ),

				m_StaticReaderID( "ID", 200, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_READER_SSN,
										"Enter numerical digits only.\n"
										"No spaces, dashes, etc." ),
				m_EditReaderID( "", 120, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | EDIT_READONLY | CONTROL_VISIBLE,		// *[5] Set read-only.
									EDIT_VALIDATION_NONE, IDC_EDIT_READER_ID ),

				m_StaticLoginPassword( "Login Password", 130, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_LOGIN_PASSWORD,
										"This is the password that you will\n"
										"provide when you log into BViewer.\n"
										"Be sure to keep a written record of\n"
										"it somewhere.  If you forget it, you\n"
										"will not be able to access BViewer.\n"
										"Your tech support will not be able\n"
										"to discover it." ),
				m_EditLoginPassword( "", 120, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | EDIT_READONLY | CONTROL_VISIBLE,		// *[5] Set read-only.
									EDIT_VALIDATION_NONE, IDC_EDIT_LOGIN_PASSWORD ),

				m_StaticReaderInitials( "Initials", 60, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_READER_INITIALS,
										"Enter your initials for the report." ),
				m_EditReaderInitials( "", 70, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | EDIT_READONLY | CONTROL_VISIBLE,		// *[5] Set read-only.
									EDIT_VALIDATION_NONE, IDC_EDIT_READER_INITIALS ),

				m_StaticAE_Title( "Local Dicom Name\n   (AE_TITLE)", 150, 30, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE | CONTROL_MULTILINE,
									IDC_STATIC_AE_TITLE,
										"Usually your last name will suffice.  This is used\n"
										"to identify the Dicom image destination, known as\n"
										"the AE_TITLE, for Dicom image senders.  If a sender\n"
										"doesn't specify this as the AE_TITLE, you might not\n"
										"receive the image, especially if there are multiple\n"
										"users of this workstation." ),
				m_EditAE_Title( "", 120, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | EDIT_READONLY | CONTROL_VISIBLE,		// *[5] Set read-only.
									EDIT_VALIDATION_NONE, IDC_EDIT_AE_TITLE ),

				m_StaticReaderReportSignatureName( "Signature Name for Report ", 200, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_READER_SIGNATURE_NAME,
										"This will appear on the report." ),
				m_EditReaderReportSignatureName( "", 460, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | EDIT_READONLY | CONTROL_VISIBLE,		// *[5] Set read-only.
									EDIT_VALIDATION_NONE, IDC_EDIT_READER_SIGNATURE_NAME ),

				m_StaticReaderStreetAddress( "Street Address", 120, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_READER_STREET_ADDRESS,
										"This will appear on the report." ),
				m_EditReaderStreetAddress( "", 300, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | EDIT_READONLY | CONTROL_VISIBLE,		// *[5] Set read-only.
									EDIT_VALIDATION_NONE, IDC_EDIT_READER_STREET_ADDRESS ),

				m_StaticReaderCity( "City", 40, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_READER_CITY,
										"This will appear on the report." ),
				m_EditReaderCity( "", 200, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | EDIT_READONLY | CONTROL_VISIBLE,		// *[5] Set read-only.
									EDIT_VALIDATION_NONE, IDC_EDIT_READER_CITY ),

				m_StaticReaderState( "State", 50, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_READER_STATE,
										"This will appear on the report." ),
				m_EditReaderState( "", 50, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | EDIT_READONLY | CONTROL_VISIBLE,		// *[5] Set read-only.
									EDIT_VALIDATION_NONE, IDC_EDIT_READER_STATE ),

				m_StaticReaderZipCode( "Zip Code", 70, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_READER_ZIPCODE,
										"This will appear on the report." ),
				m_EditReaderZipCode( "123", 120, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | EDIT_READONLY | CONTROL_VISIBLE,		// *[5] Set read-only.
									EDIT_VALIDATION_NONE, IDC_EDIT_READER_ZIPCODE ),

				m_GroupEditSequencing( GROUP_EDIT, GROUP_SEQUENCING, 20,
									&m_EditPrimaryMonitorWidth, &m_EditMonitor2Width, &m_EditMonitor3Width,
									&m_EditPrimaryMonitorHeight, &m_EditMonitor2Height, &m_EditMonitor3Height,
									&m_ComboBoxSelectPrimaryMonitorRenderingMethod, &m_ComboBoxSelectMonitor2RenderingMethod, &m_ComboBoxSelectMonitor3RenderingMethod,
									&m_EditReaderLastName, &m_EditLoginName, &m_EditReaderID, &m_EditLoginPassword,
									&m_EditReaderInitials, &m_EditAE_Title, &m_EditReaderReportSignatureName,
									&m_EditReaderStreetAddress, &m_EditReaderCity, &m_EditReaderState, &m_EditReaderZipCode ),

				m_ButtonInstallStandards( "Install I.L.O.\nStandard\nReference Images", 160, 60, 14, 7, 6,
									COLOR_WHITE, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR, COLOR_STD_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED | 
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE | CONTROL_VISIBLE, IDC_BUTTON_INSTALL_STANDARDS,
										"Start the process of installing the I.L.O. standard\n"
										"reference images from the external medium (DVD, etc.).\n"
										"You must purchase a license from the I.L.O. to obtain these." ),

				m_ButtonControlBRetriever( "Control\nBRetriever", 120, 40, 14, 7, 6,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED | 
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE | CONTROL_VISIBLE, IDC_BUTTON_CONTROL_BRETRIEVER,
										"Activate the BRetriever service manager.  This allows\n"
										"you to start and stop BRetriever and to view its log\n"
										"files.  You normally won't need to do this, unless\n"
										"you are having problems retrieving images, or if some of\n"
										"the image files get corrupted, or if image transmission\n"
										"problems occur that BRetriever is unable to correct." ),
				m_ButtonSetNetworkAddress( "Set Local\nNetwork\nAddress", 120, 50, 14, 7, 6,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED | 
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE | CONTROL_VISIBLE, IDC_BUTTON_SET_NETWORK_ADDRESS,
										"Set the IP address and port number to be used by\n"
										"remote Dicom image senders to access BRetriever.\n"
										"Usually, \"localhost\" can substitute for the IP\n"
										"address.  A typical port number would be 105,\n"
										"unless it is being used by some other Dicom device\n"
										"on this workstation.  Example:  localhost:105\n"
										"Setting this address will restart BRetriever." ),
				m_ButtonClearImageFolders( "Clear Image\nFolders", 120, 50, 14, 7, 6,
									COLOR_WHITE, COLOR_CANCEL, COLOR_CANCEL, COLOR_CANCEL,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE | CONTROL_VISIBLE, IDC_BUTTON_CLEAR_IMAGE_FOLDERS,
										"Clear out (delete) ALL the subject studies and related files and\n"
										"images.  If, as a result of file corruption, you aren't receiving\n"
										"or can't view images, this can reset everything.\n"
										"Any unread studies will have to be re-imported or re-sent to you." ),
				m_ButtonTechnicalRequirements( "Technical\nRequirements", 120, 40, 14, 7, 6,
									COLOR_BLACK, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE | CONTROL_VISIBLE, IDC_BUTTON_TECH_REQUIREMENTS,
										"Show a listing of the specific computer hardware and\n"
										"software requirements for setting up a BViewer workstation." ),
				m_ButtonAboutBViewer( "About BViewer", 120, 30, 14, 7, 6,
									COLOR_BLACK, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_ABOUT,
										"Display a list of the people whose efforts\n"
										"led to the creation of this BViewer software." ),
				m_ButtonEditUser( "Edit Users", 150, 30, 14, 7, 6,
									COLOR_BLACK, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_VISIBLE |
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
									IDC_BUTTON_EDIT_USER,
										"This shows how to change user information." ),
				m_StaticReaderCountry( "Country", 200, 30, 14, 7, 6,
										COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_MULTILINE | CONTROL_VISIBLE,
										IDC_STATIC_SELECT_COUNTRY ),
				m_EditReaderCountry( "", 280, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,								// *[5] Added edit box.
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | EDIT_READONLY | CONTROL_VISIBLE,
									EDIT_VALIDATION_NONE, IDC_EDIT_READER_COUNTRY ),
				m_ButtonBeginNewTestSession( "Clear Physician\nName and Address", 150, 40, 14, 7, 6,
									COLOR_BLACK, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_VISIBLE | CONTROL_MULTILINE  |
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
									IDC_BUTTON_BEGIN_NEW_TEST_SESSION,
										"Clear out the information from the previous BViewer user." ),
				m_StaticHelpfulTips( "You can see helpful tips for a button\nor label by moving the mouse over\nthe lower right corner of it.",
									180, 40, 12, 6, 5, COLOR_DAARK_GREEN, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE | CONTROL_MULTILINE,
									IDC_STATIC_HELPFUL_TIPS,
										"Helpful tips." )
{
	m_BkgdBrush.CreateSolidBrush( COLOR_CONFIG );
	m_bPageIsInitialized = FALSE;
	m_bImageDisplaysAreConfigured = FALSE;
	m_pStandardImageInstaller = 0;
	m_pControlTip = 0;
}


CCustomizePage::~CCustomizePage()
{
	WriteUserList();									// *[5] Ensure user list is updated before program exit.
	ThisBViewerApp.EraseReaderList();					// *[1] *[6] Corrected potential memory leak on program exit.
}


BEGIN_MESSAGE_MAP( CCustomizePage, CPropertyPage )
	//{{AFX_MSG_MAP(CCustomizePage)
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STD_DISPLAY_AUTO, OnBnClickedStdDisplayAuto )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STD_DISPLAY_PRIMARY, OnBnClickedStdDisplayPrimary )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STD_DISPLAY_MONITOR2, OnBnClickedStdDisplayMonitor2 )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STD_DISPLAY_MONITOR3, OnBnClickedStdDisplayMonitor3 )

	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STUDY_DISPLAY_AUTO, OnBnClickedStudyDisplayAuto )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STUDY_DISPLAY_PRIMARY, OnBnClickedStudyDisplayPrimary )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STUDY_DISPLAY_MONITOR2, OnBnClickedStudyDisplayMonitor2 )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_STUDY_DISPLAY_MONITOR3, OnBnClickedStudyDisplayMonitor3 )

	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_EMPHASIZE_PATIENT, OnBnClickedShowSummaryInfo )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_EMPHASIZE_STUDY, OnBnClickedShowStudyInfo )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_EMPHASIZE_SERIES, OnBnClickedShowSeriesInfo )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_EMPHASIZE_IMAGE, OnBnClickedShowImageInfo )

	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_INSTALL_STANDARDS, OnBnClickedInstallStandards )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_CONTROL_BRETRIEVER, OnBnClickedControlBRetriever )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_SET_NETWORK_ADDRESS, OnBnClickedSetNetworkAddress )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_CLEAR_IMAGE_FOLDERS, OnBnClickedClearImageFolders )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_ABOUT, OnAppAbout )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_TECH_REQUIREMENTS, OnBnClickedTechnicalRequirements )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_EDIT_USER, OnBnClickedEditUser )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_BEGIN_NEW_TEST_SESSION, OnBnClickedBeginNewTestSession )

	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_PRIMARY_MONITOR_WIDTH, OnEditPrimaryMonitorWidthKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_MONITOR2_WIDTH, OnEditMonitor2WidthKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_MONITOR3_WIDTH, OnEditMonitor3WidthKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_PRIMARY_MONITOR_HEIGHT, OnEditPrimaryMonitorHeightKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_MONITOR2_HEIGHT, OnEditMonitor2HeightKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_MONITOR3_HEIGHT, OnEditMonitor3HeightKillFocus )

	ON_CBN_SELENDOK( IDC_SELECT_PRIMARY_MONITOR_RENDERING_METHOD, OnPrimaryMonitorRenderingMethodSelected )
	ON_CBN_SELENDOK( IDC_SELECT_MONITOR2_RENDERING_METHOD, OnMonitor2RenderingMethodSelected )
	ON_CBN_SELENDOK( IDC_SELECT_MONITOR3_RENDERING_METHOD, OnMonitor3RenderingMethodSelected )

	ON_WM_MOUSEMOVE()
	ON_WM_CTLCOLOR()
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CCustomizePage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	
	m_StaticImagePlacement.SetPosition( 120, 20, this );
	m_StaticShowStdImageOn.SetPosition( 50, 60, this );
	m_ButtonStdDisplayAuto.SetPosition( 50, 100, this );
	m_ButtonStdDisplayPrimary.SetPosition( 50, 130, this );
	m_ButtonStdDisplayMonitor2.SetPosition( 50, 160, this );
	m_ButtonStdDisplayMonitor3.SetPosition( 50, 190, this );
	m_GroupStdDisplayButtons.SetGroupVisibility( CONTROL_VISIBLE );
	
	m_StaticShowStudyImageOn.SetPosition( 230, 60, this );
	m_ButtonStudyDisplayAuto.SetPosition( 230, 100, this );
	m_ButtonStudyDisplayPrimary.SetPosition( 230, 130, this );
	m_ButtonStudyDisplayMonitor2.SetPosition( 230, 160, this );
	m_ButtonStudyDisplayMonitor3.SetPosition( 230, 190, this );
	m_GroupStudyDisplayButtons.SetGroupVisibility( CONTROL_VISIBLE );

	m_StaticStudySelection.SetPosition( 215, 250, this );
	m_StaticStudySelectionDisplayEmphasis.SetPosition( 230, 290, this );
	m_ButtonShowSummaryInfo.SetPosition( 230, 350, this );
	m_ButtonShowStudyInfo.SetPosition( 230, 380, this );
	m_ButtonShowSeriesInfo.SetPosition( 230, 410, this );
	m_ButtonShowImageInfo.SetPosition( 230, 440, this );

	m_StaticImageFullSizeAdjust.SetPosition( 620, 20, this );
	m_StaticPrimaryMonitor.SetPosition( 400, 135, this );
	m_StaticMonitor2.SetPosition( 400, 165, this );
	m_StaticMonitor3.SetPosition( 400, 195, this );

	m_StaticScreenWidthInMillimeters.SetPosition( 595, 80, this );
	m_EditPrimaryMonitorWidth.SetPosition( 620, 135, this );
	m_EditMonitor2Width.SetPosition( 620, 165, this );
	m_EditMonitor3Width.SetPosition( 620, 195, this );

	m_StaticScreenHeightInMillimeters.SetPosition( 735, 80, this );
	m_EditPrimaryMonitorHeight.SetPosition( 760, 135, this );
	m_EditMonitor2Height.SetPosition( 760, 165, this );
	m_EditMonitor3Height.SetPosition( 760, 195, this );

	m_StaticGrayscaleResolution.SetPosition( 860, 20, this );
	m_StaticGrayscaleBitDepth.SetPosition( 860, 80, this );
	m_ComboBoxSelectPrimaryMonitorRenderingMethod.SetPosition( 860, 135, this );
	m_ComboBoxSelectMonitor2RenderingMethod.SetPosition( 860, 165, this );
	m_ComboBoxSelectMonitor3RenderingMethod.SetPosition( 860, 195, this );
	
	m_StaticReaderIdentification.SetPosition( 490, 260, this );
	m_StaticReaderLastName.SetPosition( 440, 300, this );
	m_EditReaderLastName.SetPosition( 640, 300, this );

	m_StaticLoginName.SetPosition( 820, 300, this );
	m_EditLoginName.SetPosition( 980, 300, this );
	
	m_StaticLoginPassword.SetPosition( 820, 330, this );
	m_EditLoginPassword.SetPosition( 980, 330, this );
	m_EditLoginPassword.SetWindowText( "" );				// *[5] Added initialization.
	m_EditLoginPassword.SetPasswordChar( '*' );
	
	m_StaticAE_Title.SetPosition( 820, 360, this );
	m_EditAE_Title.SetPosition( 980, 360, this );
	
	if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
		{
		m_StaticReaderReportSignatureName.SetPosition( 440, 410, this );
		m_EditReaderReportSignatureName.SetPosition( 640, 410, this );
		}
	else if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_NIOSH )
		{
		m_StaticReaderID.SetPosition( 440, 330, this );
		m_EditReaderID.SetPosition( 640, 330, this );

		m_StaticReaderInitials.SetPosition( 440, 360, this );
		m_EditReaderInitials.SetPosition( 640, 360, this );

		m_StaticReaderReportSignatureName.SetPosition( 440, 410, this );
		m_StaticReaderReportSignatureName.Invalidate();
		m_EditReaderReportSignatureName.SetPosition( 640, 410, this );
		}
	else if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_TEST )
		{
		m_StaticReaderID.SetPosition( 440, 330, this );
		m_EditReaderID.SetPosition( 640, 330, this );

		m_StaticReaderInitials.SetPosition( 440, 360, this );
		m_EditReaderInitials.SetPosition( 640, 360, this );
		m_ButtonBeginNewTestSession.SetPosition( 780, 610, this );

		m_StaticReaderReportSignatureName.SetPosition( 440, 410, this );
		m_StaticReaderReportSignatureName.Invalidate();
		m_EditReaderReportSignatureName.SetPosition( 640, 410, this );
		}

	m_StaticReaderStreetAddress.SetPosition( 440, 440, this );
	m_EditReaderStreetAddress.SetPosition( 640, 440, this );

	m_StaticReaderCity.SetPosition( 440, 470, this );
	m_EditReaderCity.SetPosition( 640, 470, this );

	m_StaticReaderState.SetPosition( 440, 500, this );
	m_EditReaderState.SetPosition( 640, 500, this );

	m_StaticReaderZipCode.SetPosition( 820, 500, this );
	m_EditReaderZipCode.SetPosition( 980, 500, this );

	if ( !BViewerConfiguration.bUseDigitalStandards )
		m_ButtonInstallStandards.SetPosition( 20, 260, this );
	m_ButtonAboutBViewer.SetPosition( 20, 390, this );
	m_ButtonTechnicalRequirements.SetPosition( 20, 430, this );
	m_ButtonControlBRetriever.SetPosition( 20, 550, this );
	m_ButtonSetNetworkAddress.SetPosition( 20, 600, this );
	m_ButtonClearImageFolders.SetPosition( 230, 600, this );

	m_StaticReaderCountry.SetPosition( 440, 540, this );			// *[5] Replaced selection list with this
	m_EditReaderCountry.SetPosition( 440, 565, this );				// *[5]  read-only edit box.

	m_StaticHelpfulTips.SetPosition( 440, 610, this );

	// Only enable the following buttons after the first user has entered
	// his reader information.
	if ( strlen( BViewerCustomization.m_ReaderInfo.LastName ) != 0 )
		{
		if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL ||
					BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_NIOSH )
			m_ButtonEditUser.SetPosition( 950, 560, this );
		}

	m_bPageIsInitialized = TRUE;
	m_bImageDisplaysAreConfigured = TRUE;
	LoadRenderingMethodSelectionLists();

	return TRUE;
}



static void ControlTipActivationFunction( CWnd *pDialogWindow, char *pTipText, CPoint MouseCursorLocation )
{
	CCustomizePage			*pCustomizePage;

	pCustomizePage = (CCustomizePage*)pDialogWindow;
	if ( pCustomizePage != 0 )
		{
		// If there has been a change in the tip text, reset the tip display window.
		if ( pTipText != 0 && strlen( pTipText ) > 0 && pTipText != pCustomizePage -> m_pControlTip -> m_pTipText &&
																	pCustomizePage -> m_pControlTip -> m_pTipText != 0 )
			{
			pCustomizePage -> m_pControlTip -> ShowWindow( SW_HIDE );
			pCustomizePage -> m_pControlTip -> m_pTipText = pTipText;
			pCustomizePage -> m_pControlTip -> ShowTipText( MouseCursorLocation, pCustomizePage );
			}
		else if ( pTipText == 0 )
			pCustomizePage -> m_pControlTip -> ShowWindow( SW_HIDE );
		else
			{
			pCustomizePage -> m_pControlTip -> m_pTipText = pTipText;
			pCustomizePage -> m_pControlTip -> ShowTipText( MouseCursorLocation, pCustomizePage );
			}
		}
}


void CCustomizePage::InitializeControlTips()
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
				if ( ( (TomButton*)pChildWindow ) -> IsVisible() )
					( (TomButton*)pChildWindow ) -> m_ControlTipActivator = ControlTipActivationFunction;
			pChildWindow = pChildWindow -> GetWindow( GW_HWNDNEXT );
			}
		}
}


BOOL CCustomizePage::OnSetActive()
{
	CMainFrame						*pMainFrame;
	CControlPanel					*pControlPanel;
	static USER_NOTIFICATION_INFO	UserNotificationInfo;

	ResetPage();

	if ( strlen( BViewerCustomization.m_ReaderInfo.LastName ) == 0  && !bReaderInfoQuestionHasBeenAsked && !bMakeDumbButtons )
		{
		EditUserInfo( TRUE );			// *[5] Force reader info entry if none has been provided.

		// Set a default AE_TITLE.
		if ( strlen( BViewerCustomization.m_ReaderInfo.AE_TITLE ) == 0 )
			{
			strncpy_s( BViewerCustomization.m_ReaderInfo.AE_TITLE, 20, "BViewer", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
			m_EditAE_Title.SetWindowText( BViewerCustomization.m_ReaderInfo.AE_TITLE );
			}
		// Request user to enter reader information.
		pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
		if ( pMainFrame != 0 )
			{
			UserNotificationInfo.WindowWidth = 500;
			UserNotificationInfo.WindowHeight = 400;
			UserNotificationInfo.FontHeight = 0;	// Use default setting;
			UserNotificationInfo.FontWidth = 0;		// Use default setting;
			UserNotificationInfo.UserInputType = USER_INPUT_TYPE_OK;
			UserNotificationInfo.pUserNotificationMessage = "Please enter the B reader's\ninformation\n\nFor the BViewer setup procedure,\n"
															"click the mouse on the\n\"View User Manual\" tab.";
			UserNotificationInfo.CallbackFunction = FinishReaderInfoResponse;
			pMainFrame -> PerformUserInput( &UserNotificationInfo );
			bReaderInfoQuestionHasBeenAsked = TRUE;
			}
		}
	InitializeControlTips();

	pControlPanel = (CControlPanel*)GetParent();
	if ( pControlPanel != 0 )
		pControlPanel -> m_CurrentlyActivePage = SETUP_PAGE;

	return CPropertyPage::OnSetActive();
}


void CCustomizePage::ClearReaderInfoDisplay()
{
	m_EditReaderLastName.SetWindowText( "" );
	m_EditLoginName.SetWindowText( "" );
	if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
		m_EditReaderReportSignatureName.SetWindowText( "" );
	else if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_NIOSH )
		{
		m_EditReaderID.SetWindowText( "" );
		m_EditReaderInitials.SetWindowText( "" );
		m_EditReaderReportSignatureName.SetWindowText( "" );
		}
	else if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_TEST )
		{
		m_EditReaderInitials.SetWindowText( "" );
		m_EditReaderReportSignatureName.SetWindowText( "" );
		}

	m_EditLoginPassword.SetWindowText( "" );
	m_EditAE_Title.SetWindowText( "BViewer" );				// *[5] Set default value which most seem to be using.
	m_EditReaderStreetAddress.SetWindowText( "" );
	m_EditReaderCity.SetWindowText( "" );
	m_EditReaderState.SetWindowText( "" );
	m_EditReaderZipCode.SetWindowText( "" );
	m_EditReaderCountry.SetWindowText( "" );				// *[5] Add initialization for new edit box.
}


// *[5] Created the following as a separate, modular function.
void CCustomizePage::ResetReaderInfo()
{
	char		TextString[ 65 ];

	m_EditReaderLastName.SetWindowText( BViewerCustomization.m_ReaderInfo.LastName );
	m_EditLoginName.SetWindowText( BViewerCustomization.m_ReaderInfo.LoginName );
	if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL )
		m_EditReaderReportSignatureName.SetWindowText( BViewerCustomization.m_ReaderInfo.ReportSignatureName );
	else if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_NIOSH )
		{
		m_EditReaderID.SetWindowText( BViewerCustomization.m_ReaderInfo.ID );
		m_EditReaderInitials.SetWindowText( BViewerCustomization.m_ReaderInfo.Initials );
		m_EditReaderReportSignatureName.SetWindowText( BViewerCustomization.m_ReaderInfo.ReportSignatureName );
		}
	else if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_TEST )
		{
		m_EditReaderInitials.SetWindowText( BViewerCustomization.m_ReaderInfo.Initials );
		m_EditReaderReportSignatureName.SetWindowText( BViewerCustomization.m_ReaderInfo.ReportSignatureName );
		}

	memcpy( TextString, BViewerCustomization.m_ReaderInfo.EncodedPassword, 64 );
	TextString[ BViewerCustomization.m_ReaderInfo.pwLength ] = '\0';
	m_EditLoginPassword.SetWindowText( TextString );

	m_EditAE_Title.SetWindowText( BViewerCustomization.m_ReaderInfo.AE_TITLE );
	m_EditReaderStreetAddress.SetWindowText( BViewerCustomization.m_ReaderInfo.StreetAddress );
	m_EditReaderCity.SetWindowText( BViewerCustomization.m_ReaderInfo.City );
	m_EditReaderState.SetWindowText( BViewerCustomization.m_ReaderInfo.State );
	m_EditReaderZipCode.SetWindowText( BViewerCustomization.m_ReaderInfo.ZipCode );
	m_EditReaderCountry.SetWindowText( BViewerCustomization.m_ReaderInfo.m_CountryInfo.CountryName );
}


void CCustomizePage::ResetPage()
{
	CMainFrame			*pMainFrame;
	int					nMonitorsAvailable;
	long				IntegerValue;
	char				NumberConvertedToText[ _CVTBUFSIZE ];

	if ( m_bPageIsInitialized )
		{
		pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
		nMonitorsAvailable = pMainFrame -> m_DisplayMonitorCount;

		// Respond to the monitor assignment for displaying the standard images.
		if ( ( BViewerCustomization.m_DisplayAssignments & ASSIGN_STD_DISPLAY_AUTO ) != 0 )
			m_ButtonStdDisplayAuto.m_ToggleState = BUTTON_ON;
		else
			m_ButtonStdDisplayAuto.m_ToggleState = BUTTON_OFF;

		if ( ( BViewerCustomization.m_DisplayAssignments & ASSIGN_STD_DISPLAY_PRIMARY ) != 0 )
			m_ButtonStdDisplayPrimary.m_ToggleState = BUTTON_ON;
		else
			m_ButtonStdDisplayPrimary.m_ToggleState = BUTTON_OFF;

		nMonitorsAvailable--;
		if ( nMonitorsAvailable > 0 )
			{
			if ( ( BViewerCustomization.m_DisplayAssignments & ASSIGN_STD_DISPLAY_MONITOR2 ) != 0 )
				m_ButtonStdDisplayMonitor2.m_ToggleState = BUTTON_ON;
			else
				m_ButtonStdDisplayMonitor2.m_ToggleState = BUTTON_OFF;
			m_StaticMonitor2.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
			}
		else
			{
			m_ButtonStdDisplayMonitor2.m_ControlStyle &= ~CONTROL_VISIBLE;
			m_ButtonStdDisplayMonitor2.m_ControlStyle |= CONTROL_INVISIBLE;
			m_StaticMonitor2.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
			m_EditMonitor2Width.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
			m_EditMonitor2Height.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
			m_ComboBoxSelectMonitor2RenderingMethod.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
			}

		nMonitorsAvailable--;
		if ( nMonitorsAvailable > 0 )
			{
			if ( ( BViewerCustomization.m_DisplayAssignments & ASSIGN_STD_DISPLAY_MONITOR3 ) != 0 )
				m_ButtonStdDisplayMonitor3.m_ToggleState = BUTTON_ON;
			else
				m_ButtonStdDisplayMonitor3.m_ToggleState = BUTTON_OFF;
			m_StaticMonitor3.ChangeStatus( CONTROL_INVISIBLE, CONTROL_VISIBLE );
			}
		else
			{
			m_ButtonStdDisplayMonitor3.m_ControlStyle &= ~CONTROL_VISIBLE;
			m_ButtonStdDisplayMonitor3.m_ControlStyle |= CONTROL_INVISIBLE;
			m_StaticMonitor3.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
			m_EditMonitor3Width.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
			m_EditMonitor3Height.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
			m_ComboBoxSelectMonitor3RenderingMethod.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
			}

		nMonitorsAvailable = pMainFrame -> m_DisplayMonitorCount;
		// Respond to the monitor assignment for displaying the subject study images.
		if ( ( BViewerCustomization.m_DisplayAssignments & ASSIGN_STUDY_DISPLAY_AUTO ) != 0 )
			m_ButtonStudyDisplayAuto.m_ToggleState = BUTTON_ON;
		else
			m_ButtonStudyDisplayAuto.m_ToggleState = BUTTON_OFF;
		if ( ( BViewerCustomization.m_DisplayAssignments & ASSIGN_STUDY_DISPLAY_PRIMARY ) != 0 )
			m_ButtonStudyDisplayPrimary.m_ToggleState = BUTTON_ON;
		else
			m_ButtonStudyDisplayPrimary.m_ToggleState = BUTTON_OFF;

		nMonitorsAvailable--;
		if ( nMonitorsAvailable > 0 )
			{
			if ( ( BViewerCustomization.m_DisplayAssignments & ASSIGN_STUDY_DISPLAY_MONITOR2 ) != 0 )
				m_ButtonStudyDisplayMonitor2.m_ToggleState = BUTTON_ON;
			else
				m_ButtonStudyDisplayMonitor2.m_ToggleState = BUTTON_OFF;
			}
		else
			{
			m_ButtonStudyDisplayMonitor2.m_ControlStyle &= ~CONTROL_VISIBLE;
			m_ButtonStudyDisplayMonitor2.m_ControlStyle |= CONTROL_INVISIBLE;
			}

		nMonitorsAvailable--;
		if ( nMonitorsAvailable > 0 )
			{
			if ( ( BViewerCustomization.m_DisplayAssignments & ASSIGN_STUDY_DISPLAY_MONITOR3 ) != 0 )
				m_ButtonStudyDisplayMonitor3.m_ToggleState = BUTTON_ON;
			else
				m_ButtonStudyDisplayMonitor3.m_ToggleState = BUTTON_OFF;
			}
		else
			{
			m_ButtonStudyDisplayMonitor3.m_ControlStyle &= ~CONTROL_VISIBLE;
			m_ButtonStudyDisplayMonitor3.m_ControlStyle |= CONTROL_INVISIBLE;
			}

		if ( ( BViewerCustomization.m_StudyInformationDisplayEmphasis & INFO_EMPHASIS_PATIENT ) != 0 )
			m_ButtonShowSummaryInfo.m_ToggleState = BUTTON_ON;
		else
			m_ButtonShowSummaryInfo.m_ToggleState = BUTTON_OFF;
		if ( ( BViewerCustomization.m_StudyInformationDisplayEmphasis & INFO_EMPHASIS_STUDY ) != 0 )
			m_ButtonShowStudyInfo.m_ToggleState = BUTTON_ON;
		else
			m_ButtonShowStudyInfo.m_ToggleState = BUTTON_OFF;
		if ( ( BViewerCustomization.m_StudyInformationDisplayEmphasis & INFO_EMPHASIS_SERIES ) != 0 )
			m_ButtonShowSeriesInfo.m_ToggleState = BUTTON_ON;
		else
			m_ButtonShowSeriesInfo.m_ToggleState = BUTTON_OFF;
		if ( ( BViewerCustomization.m_StudyInformationDisplayEmphasis & INFO_EMPHASIS_IMAGE ) != 0 )
			m_ButtonShowImageInfo.m_ToggleState = BUTTON_ON;
		else
			m_ButtonShowImageInfo.m_ToggleState = BUTTON_OFF;

		IntegerValue = (int)BViewerCustomization.m_PrimaryMonitorWidthInMM;
		_ltoa( IntegerValue, NumberConvertedToText, 10 );
		m_EditPrimaryMonitorWidth.SetWindowText( NumberConvertedToText );

		IntegerValue = (int)BViewerCustomization.m_Monitor2WidthInMM;
		_ltoa( IntegerValue, NumberConvertedToText, 10 );
		m_EditMonitor2Width.SetWindowText( NumberConvertedToText );

		IntegerValue = (int)BViewerCustomization.m_Monitor3WidthInMM;
		_ltoa( IntegerValue, NumberConvertedToText, 10 );
		m_EditMonitor3Width.SetWindowText( NumberConvertedToText );

		IntegerValue = (int)BViewerCustomization.m_PrimaryMonitorHeightInMM;
		_ltoa( IntegerValue, NumberConvertedToText, 10 );
		m_EditPrimaryMonitorHeight.SetWindowText( NumberConvertedToText );

		IntegerValue = (int)BViewerCustomization.m_Monitor2HeightInMM;
		_ltoa( IntegerValue, NumberConvertedToText, 10 );
		m_EditMonitor2Height.SetWindowText( NumberConvertedToText );

		IntegerValue = (int)BViewerCustomization.m_Monitor3HeightInMM;
		_ltoa( IntegerValue, NumberConvertedToText, 10 );
		m_EditMonitor3Height.SetWindowText( NumberConvertedToText );

		ResetReaderInfo();			// *[5] Called as a separate function.
		}
}


HBRUSH CCustomizePage::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
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


void CCustomizePage::OnBnClickedStdDisplayAuto( NMHDR *pNMHDR, LRESULT *pResult )
{
	CMainFrame			*pMainFrame;

	m_ButtonStdDisplayAuto.m_pGroup -> RespondToSelection( (void*)&m_ButtonStdDisplayAuto );
	BViewerCustomization.m_DisplayAssignments &= ~ASSIGN_STD_MASK;
	BViewerCustomization.m_DisplayAssignments |= ASSIGN_STD_DISPLAY_AUTO;
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;

	if ( m_bImageDisplaysAreConfigured )
		Invalidate( TRUE );

	*pResult = 0;
}


void CCustomizePage::OnBnClickedStdDisplayPrimary( NMHDR *pNMHDR, LRESULT *pResult )
{
	CMainFrame			*pMainFrame;
	RECT				ImageWindowRect;

	m_ButtonStdDisplayPrimary.m_pGroup -> RespondToSelection( (void*)&m_ButtonStdDisplayPrimary );
	BViewerCustomization.m_DisplayAssignments &= ~ASSIGN_STD_MASK;
	BViewerCustomization.m_DisplayAssignments |= ASSIGN_STD_DISPLAY_PRIMARY;
	ImageWindowRect = CRect( ::GetSystemMetrics( SM_CXSCREEN ) * 11 / 20,
								0,
								::GetSystemMetrics( SM_CXSCREEN ),
								::GetSystemMetrics( SM_CYSCREEN ) - 25 );
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	pMainFrame -> m_pImageFrame[ IMAGE_FRAME_STANDARD ] -> MoveWindow( &ImageWindowRect, TRUE );
	pMainFrame -> m_pImageFrame[ IMAGE_FRAME_STANDARD ] -> m_ImageView.ResetDiagnosticImage( TRUE );
	if ( m_bImageDisplaysAreConfigured )
		{
		Invalidate( TRUE );
		pMainFrame -> m_pImageFrame[ IMAGE_FRAME_STANDARD ] -> ShowWindow( SW_SHOW );
		}

	*pResult = 0;
}


void CCustomizePage::OnBnClickedStdDisplayMonitor2( NMHDR *pNMHDR, LRESULT *pResult )
{
	CMainFrame			*pMainFrame;
	RECT				ImageWindowRect;
	MONITOR_INFO		*pDisplayMonitorInfo;
	BOOL				bMonitorInfoFound;

	m_ButtonStdDisplayMonitor2.m_pGroup -> RespondToSelection( (void*)&m_ButtonStdDisplayMonitor2 );
	BViewerCustomization.m_DisplayAssignments &= ~ASSIGN_STD_MASK;
	BViewerCustomization.m_DisplayAssignments |= ASSIGN_STD_DISPLAY_MONITOR2;
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	bMonitorInfoFound = FALSE;
	pDisplayMonitorInfo = pMainFrame -> m_pDisplayMonitorInfoList;
	while ( !bMonitorInfoFound && pDisplayMonitorInfo != 0 )
		{
		if ( pDisplayMonitorInfo -> DisplayIdentity == DISPLAY_IDENTITY_IMAGE2 )
			bMonitorInfoFound = TRUE;
		else
			pDisplayMonitorInfo = pDisplayMonitorInfo -> pNextMonitor;
		}
	if ( bMonitorInfoFound )
		{
		ImageWindowRect = pDisplayMonitorInfo -> DesktopCoverageRectangle;
		pMainFrame -> m_pImageFrame[ IMAGE_FRAME_STANDARD ] -> MoveWindow( &ImageWindowRect, TRUE );
		pMainFrame -> m_pImageFrame[ IMAGE_FRAME_STANDARD ] -> m_ImageView.ResetDiagnosticImage( TRUE );
		}
	if ( m_bImageDisplaysAreConfigured )
		{
		Invalidate( TRUE );
		pMainFrame -> m_pImageFrame[ IMAGE_FRAME_STANDARD ] -> ShowWindow( SW_SHOW );
		}

	*pResult = 0;
}


void CCustomizePage::OnBnClickedStdDisplayMonitor3( NMHDR *pNMHDR, LRESULT *pResult )
{
	CMainFrame			*pMainFrame;
	RECT				ImageWindowRect;
	MONITOR_INFO		*pDisplayMonitorInfo;
	BOOL				bMonitorInfoFound;

	m_ButtonStdDisplayMonitor3.m_pGroup -> RespondToSelection( (void*)&m_ButtonStdDisplayMonitor3 );
	BViewerCustomization.m_DisplayAssignments &= ~ASSIGN_STD_MASK;
	BViewerCustomization.m_DisplayAssignments |= ASSIGN_STD_DISPLAY_MONITOR3;
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	bMonitorInfoFound = FALSE;
	pDisplayMonitorInfo = pMainFrame -> m_pDisplayMonitorInfoList;
	while ( !bMonitorInfoFound && pDisplayMonitorInfo != 0 )
		{
		if ( pDisplayMonitorInfo -> DisplayIdentity == DISPLAY_IDENTITY_IMAGE3 )
			bMonitorInfoFound = TRUE;
		else
			pDisplayMonitorInfo = pDisplayMonitorInfo -> pNextMonitor;
		}
	if ( bMonitorInfoFound )
		{
		ImageWindowRect = pDisplayMonitorInfo -> DesktopCoverageRectangle;
		pMainFrame -> m_pImageFrame[ IMAGE_FRAME_STANDARD ] -> MoveWindow( &ImageWindowRect, TRUE );
		pMainFrame -> m_pImageFrame[ IMAGE_FRAME_STANDARD ] -> m_ImageView.ResetDiagnosticImage( TRUE );
		}
	if ( m_bImageDisplaysAreConfigured )
		{
		Invalidate( TRUE );
		pMainFrame -> m_pImageFrame[ IMAGE_FRAME_STANDARD ] -> ShowWindow( SW_SHOW );
		}

	*pResult = 0;
}


void CCustomizePage::OnBnClickedStudyDisplayAuto( NMHDR *pNMHDR, LRESULT *pResult )
{
	CMainFrame			*pMainFrame;

	m_ButtonStudyDisplayAuto.m_pGroup -> RespondToSelection( (void*)&m_ButtonStudyDisplayAuto );
	BViewerCustomization.m_DisplayAssignments &= ~ASSIGN_STUDY_MASK;
	BViewerCustomization.m_DisplayAssignments |= ASSIGN_STUDY_DISPLAY_AUTO;
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( m_bImageDisplaysAreConfigured )
		Invalidate( TRUE );

	*pResult = 0;
}


void CCustomizePage::OnBnClickedStudyDisplayPrimary( NMHDR *pNMHDR, LRESULT *pResult )
{
	CMainFrame			*pMainFrame;
	RECT				ImageWindowRect;

	m_ButtonStudyDisplayPrimary.m_pGroup -> RespondToSelection( (void*)&m_ButtonStudyDisplayPrimary );
	BViewerCustomization.m_DisplayAssignments &= ~ASSIGN_STUDY_MASK;
	BViewerCustomization.m_DisplayAssignments |= ASSIGN_STUDY_DISPLAY_PRIMARY;
	ImageWindowRect = CRect( ::GetSystemMetrics( SM_CXSCREEN ) * 11 / 20,
								0,
								::GetSystemMetrics( SM_CXSCREEN ),
								::GetSystemMetrics( SM_CYSCREEN ) - 25 );
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> MoveWindow( &ImageWindowRect, TRUE );
	pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> m_ImageView.ResetDiagnosticImage( TRUE );
	if ( m_bImageDisplaysAreConfigured )
		{
		Invalidate( TRUE );
		pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> ShowWindow( SW_SHOW );
		}

	*pResult = 0;
}


void CCustomizePage::OnBnClickedStudyDisplayMonitor2( NMHDR *pNMHDR, LRESULT *pResult )
{
	CMainFrame			*pMainFrame;
	RECT				ImageWindowRect;
	MONITOR_INFO		*pDisplayMonitorInfo;
	BOOL				bMonitorInfoFound;

	m_ButtonStudyDisplayMonitor2.m_pGroup -> RespondToSelection( (void*)&m_ButtonStudyDisplayMonitor2 );
	BViewerCustomization.m_DisplayAssignments &= ~ASSIGN_STUDY_MASK;
	BViewerCustomization.m_DisplayAssignments |= ASSIGN_STUDY_DISPLAY_MONITOR2;
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	bMonitorInfoFound = FALSE;
	pDisplayMonitorInfo = pMainFrame -> m_pDisplayMonitorInfoList;
	while ( !bMonitorInfoFound && pDisplayMonitorInfo != 0 )
		{
		if ( pDisplayMonitorInfo -> DisplayIdentity == DISPLAY_IDENTITY_IMAGE2 )
			bMonitorInfoFound = TRUE;
		else
			pDisplayMonitorInfo = pDisplayMonitorInfo -> pNextMonitor;
		}
	if ( bMonitorInfoFound )
		{
		ImageWindowRect = pDisplayMonitorInfo -> DesktopCoverageRectangle;
		pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> MoveWindow( &ImageWindowRect, TRUE );
		pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> m_ImageView.ResetDiagnosticImage( TRUE );
		}
	if ( m_bImageDisplaysAreConfigured )
		{
		Invalidate( TRUE );
		pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> ShowWindow( SW_SHOW );
		}

	*pResult = 0;
}


void CCustomizePage::OnBnClickedStudyDisplayMonitor3( NMHDR *pNMHDR, LRESULT *pResult )
{
	CMainFrame			*pMainFrame;
	RECT				ImageWindowRect;
	MONITOR_INFO		*pDisplayMonitorInfo;
	BOOL				bMonitorInfoFound;

	m_ButtonStudyDisplayMonitor3.m_pGroup -> RespondToSelection( (void*)&m_ButtonStudyDisplayMonitor3 );
	BViewerCustomization.m_DisplayAssignments &= ~ASSIGN_STUDY_MASK;
	BViewerCustomization.m_DisplayAssignments |= ASSIGN_STUDY_DISPLAY_MONITOR3;
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	bMonitorInfoFound = FALSE;
	pDisplayMonitorInfo = pMainFrame -> m_pDisplayMonitorInfoList;
	while ( !bMonitorInfoFound && pDisplayMonitorInfo != 0 )
		{
		if ( pDisplayMonitorInfo -> DisplayIdentity == DISPLAY_IDENTITY_IMAGE3 )
			bMonitorInfoFound = TRUE;
		else
			pDisplayMonitorInfo = pDisplayMonitorInfo -> pNextMonitor;
		}
	if ( bMonitorInfoFound )
		{
		ImageWindowRect = pDisplayMonitorInfo -> DesktopCoverageRectangle;
		pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> MoveWindow( &ImageWindowRect, TRUE );
		pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> m_ImageView.ResetDiagnosticImage( TRUE );
		}
	if ( m_bImageDisplaysAreConfigured )
		{
		Invalidate( TRUE );
		pMainFrame -> m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> ShowWindow( SW_SHOW );
		}

	*pResult = 0;
}


void CCustomizePage::OnBnClickedShowSummaryInfo( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( !bMakeDumbButtons )
		{
		m_ButtonShowSummaryInfo.m_pGroup -> RespondToSelection( (void*)&m_ButtonShowSummaryInfo );
		if ( m_ButtonShowSummaryInfo.m_ToggleState == BUTTON_ON )
			BViewerCustomization.m_StudyInformationDisplayEmphasis = INFO_EMPHASIS_PATIENT;
		Invalidate( TRUE );
		}

	*pResult = 0;
}


void CCustomizePage::OnBnClickedShowStudyInfo( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( !bMakeDumbButtons )
		{
		m_ButtonShowStudyInfo.m_pGroup -> RespondToSelection( (void*)&m_ButtonShowStudyInfo );
		if ( m_ButtonShowStudyInfo.m_ToggleState == BUTTON_ON )
			BViewerCustomization.m_StudyInformationDisplayEmphasis = INFO_EMPHASIS_STUDY;
		Invalidate( TRUE );
		}

	*pResult = 0;
}


void CCustomizePage::OnBnClickedShowSeriesInfo( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( !bMakeDumbButtons )
		{
		m_ButtonShowSeriesInfo.m_pGroup -> RespondToSelection( (void*)&m_ButtonShowSeriesInfo );
		if ( m_ButtonShowSeriesInfo.m_ToggleState == BUTTON_ON )
			BViewerCustomization.m_StudyInformationDisplayEmphasis = INFO_EMPHASIS_SERIES;
		Invalidate( TRUE );
		}

	*pResult = 0;
}


void CCustomizePage::OnBnClickedShowImageInfo( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( !bMakeDumbButtons )
		{
		m_ButtonShowImageInfo.m_pGroup -> RespondToSelection( (void*)&m_ButtonShowImageInfo );
		if ( m_ButtonShowImageInfo.m_ToggleState == BUTTON_ON )
			BViewerCustomization.m_StudyInformationDisplayEmphasis = INFO_EMPHASIS_IMAGE;
		Invalidate( TRUE );
		}

	*pResult = 0;
}


void CCustomizePage::OnBnClickedInstallStandards( NMHDR *pNMHDR, LRESULT *pResult )
{
	BOOL					bNoError = TRUE;
	CString					FullFileSpec;
	int						DialogWidth;
	int						DialogHeight;
 	CMainFrame				*pMainFrame;
	RECT					ClientRect;
	INT						ClientWidth;
	INT						ClientHeight;
	char					Msg[ MAX_EXTRA_LONG_STRING_LENGTH ];
	
	if ( !bMakeDumbButtons )
		{
		strncpy_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "Note:  When you insert the I.L.O. reference standards \n", _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
		strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "media, the I.L.O. viewer may try to start.\n", _TRUNCATE );					// *[4] Replaced strcat with strncat_s.
		strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "If Windows asks your permission to run it, answer \"No\".\n", _TRUNCATE );	// *[4] Replaced strcat with strncat_s.
		strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "If the viewer starts automatically, wait patiently for\n", _TRUNCATE );		// *[4] Replaced strcat with strncat_s.
		strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "it to come up, then exit out of it before continuing\n", _TRUNCATE );		// *[4] Replaced strcat with strncat_s.
		strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "with this reference image installation.\n", _TRUNCATE );						// *[4] Replaced strcat with strncat_s.
		strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "\n", _TRUNCATE );															// *[4] Replaced strcat with strncat_s.
		strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "If you like, you may insert the I.L.O. media now.\n", _TRUNCATE );			// *[4] Replaced strcat with strncat_s.
		ThisBViewerApp.NotifyUserToAcknowledgeContinuation( Msg );

		DialogWidth = 1024;
		DialogHeight = 600;
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

		if ( m_pStandardImageInstaller != 0 )
			delete m_pStandardImageInstaller;
		m_pStandardImageInstaller = new CStandardSelector( DialogWidth, DialogHeight, COLOR_STANDARD, 0 );
		if ( m_pStandardImageInstaller != 0 )
			{
			m_pStandardImageInstaller -> SetPosition( ( ClientWidth - DialogWidth ) / 2, ( ClientHeight - DialogHeight ) / 2, this, ExplorerWindowClass );
			m_pStandardImageInstaller -> BringWindowToTop();
			m_pStandardImageInstaller -> SetFocus();
			}
		}

	*pResult = 0;
}


void CCustomizePage::OnBnClickedControlBRetriever( NMHDR *pNMHDR, LRESULT *pResult )
{
	char			ProgramPath[ FULL_FILE_SPEC_STRING_LENGTH ];
	char			ServiceControllerExeFile[ FULL_FILE_SPEC_STRING_LENGTH ];
	char			Msg[ MAX_EXTRA_LONG_STRING_LENGTH ];
	
	if ( !bMakeDumbButtons )
		{
		strncpy_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "Note:  Microsoft windows security requires that\n", _TRUNCATE );		// *[5] Added informative message for reader.
		strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "you must have started BViewer with\n", _TRUNCATE );					// *[5] Added informative message for reader.
		strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "\"Run as administrator\".\n", _TRUNCATE );							// *[5] Added informative message for reader.
		strncat_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "if you intend to control BRetriever. \n", _TRUNCATE );				// *[5] Added informative message for reader.
		ThisBViewerApp.NotifyUserToAcknowledgeContinuation( Msg );															// *[5] Added informative message for reader.

		strncpy_s( ProgramPath, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.ProgramPath, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
		strncat_s( ProgramPath, FULL_FILE_SPEC_STRING_LENGTH, "BRetriever\\", _TRUNCATE );							// *[4] Replaced strcat with strncat_s.
		strncpy_s( ServiceControllerExeFile, FULL_FILE_SPEC_STRING_LENGTH, ProgramPath, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
		strncat_s( ServiceControllerExeFile, FULL_FILE_SPEC_STRING_LENGTH, "ServiceController.exe", _TRUNCATE );	// *[4] Replaced strcat with strncat_s.
		LogMessage( "Launching the service controller:", MESSAGE_TYPE_NORMAL_LOG );
		LogMessage( ServiceControllerExeFile, MESSAGE_TYPE_NORMAL_LOG );
		_spawnl( _P_NOWAIT, ServiceControllerExeFile, ServiceControllerExeFile, 0 );
		}

	*pResult = 0;
}


static void ProcessBRetrieverNetworkAddressResponse( void *pResponseDialog )
{
	CPopupDialog			*pPopupDialog = 0;			// *[2] Added redundant initialization to please Fortify.
	CString					UserResponseString;
	char					ProgramPath[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					ServiceControllerExeFile[ FULL_FILE_SPEC_STRING_LENGTH ];
	
	pPopupDialog = (CPopupDialog*)pResponseDialog;
	if ( pPopupDialog != 0 )			// *[1] Reorganized to ensure pPopupDialog is deleted in all cases.
		{
		if ( !bMakeDumbButtons )
			{
			if ( pPopupDialog -> m_pUserNotificationInfo -> UserResponse == POPUP_RESPONSE_SAVE )
				{
				pPopupDialog -> m_EditUserTextInput.GetWindowText( UserResponseString );
				strncpy_s( pPopupDialog -> m_pUserNotificationInfo -> UserTextResponse, MAX_CFG_STRING_LENGTH, (const char*)UserResponseString, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
				strncpy_s( BViewerConfiguration.NetworkAddress, MAX_CFG_STRING_LENGTH, pPopupDialog -> m_pUserNotificationInfo -> UserTextResponse, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
				// Write out updated configuration file.
				RewriteConfigurationFile( BViewerConfiguration.BRetrieverServiceDirectory, "Shared.cfg" );
				// Restart BRetriever.
				strncpy_s( ProgramPath, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.ProgramPath, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.
				strncat_s( ProgramPath, FULL_FILE_SPEC_STRING_LENGTH, "BRetriever\\" , _TRUNCATE );							// *[4] Replaced strcat with strncat_s.
				strncpy_s( ServiceControllerExeFile, FULL_FILE_SPEC_STRING_LENGTH, ProgramPath, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
				strncat_s( ServiceControllerExeFile, FULL_FILE_SPEC_STRING_LENGTH, "ServiceController.exe", _TRUNCATE );	// *[4] Replaced strcat with strncat_s.
				LogMessage( "Launching the service controller for restart:", MESSAGE_TYPE_NORMAL_LOG );
				LogMessage( ServiceControllerExeFile, MESSAGE_TYPE_NORMAL_LOG );
				_spawnl( _P_NOWAIT, ServiceControllerExeFile, ServiceControllerExeFile, "/restart", 0 );
				}
			}
		delete pPopupDialog;
		}
}


void CCustomizePage::OnBnClickedSetNetworkAddress( NMHDR *pNMHDR, LRESULT *pResult )
{
 	CMainFrame						*pMainFrame;
	static USER_NOTIFICATION_INFO	UserNotificationInfo;

	if ( !bMakeDumbButtons )
		{
		LogMessage( "Configuring BRetriever:  Setting Dicom network address.", MESSAGE_TYPE_NORMAL_LOG );
		// Request user to set the BRetriever network address.
		pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
		if ( pMainFrame != 0 )
			{
			UserNotificationInfo.WindowWidth = 450;
			UserNotificationInfo.WindowHeight = 300;
			UserNotificationInfo.FontHeight = 18;	// Use default setting;
			UserNotificationInfo.FontWidth = 9;		// Use default setting;
			UserNotificationInfo.UserInputType = USER_INPUT_TYPE_EDIT;
			UserNotificationInfo.pUserNotificationMessage = "Enter BRetriever's network address.\n\nFormat    <IP Address>:<Port Number>\n\n(Example    localhost:105)";
			UserNotificationInfo.CallbackFunction = ProcessBRetrieverNetworkAddressResponse;
			UserNotificationInfo.pUserData = 0;
			strncpy_s( UserNotificationInfo.UserTextResponse, MAX_CFG_STRING_LENGTH, BViewerConfiguration.NetworkAddress, _TRUNCATE );		// *[1] Replaced strcpy with strncpy_s.

			CWaitCursor			HourGlass;
		
			pMainFrame -> PerformUserInput( &UserNotificationInfo );
			}
		}

	*pResult = 0;
}


BOOL CCustomizePage::DirectoryExists( char *pFullDirectorySpecification )
{
	BOOL			bDirectoryExists;
	char			SavedCurrentDirectory[ MAX_PATH ];				// *[2] Set buffer size to maximum path size.

	GetCurrentDirectory( MAX_PATH, SavedCurrentDirectory );			// *[2] Set buffer size to maximum path size.
	bDirectoryExists = SetCurrentDirectory( pFullDirectorySpecification );
	SetCurrentDirectory( SavedCurrentDirectory );
	
	return bDirectoryExists;
}


void CCustomizePage::DeleteFolderContents( char *SearchDirectory, int FolderIndent )
{
	BOOL						bNoError = TRUE;
	char						SearchFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	WIN32_FIND_DATA				FindFileInfo;
	HANDLE						hFindFile;
	BOOL						bFileFound;
	char						Msg[ MAX_LOGGING_STRING_LENGTH ];
	BOOL						bSpecialDirectory;
	BOOL						bIsFolder;
	char						FolderAnnotation[ MAX_CFG_STRING_LENGTH ];
	char						NewSearchDirectory[ FULL_FILE_SPEC_STRING_LENGTH ];
	char						FullFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char						*pExtension;			// *[1] Added extension requirement before deletion.


	strncpy_s( FolderAnnotation, MAX_CFG_STRING_LENGTH, "                                                   ", FolderIndent );	// *[3] Replaced strncat with strncpy_s.
	// Check existence of source path.
	bNoError = DirectoryExists( SearchDirectory );
	if ( bNoError )
		{
		sprintf_s( Msg, MAX_LOGGING_STRING_LENGTH, "    %sDeleting contents of %s:", FolderAnnotation, SearchDirectory );		// *[1] Replaced sprintf with sprintf_s.
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
		strncpy_s( SearchFileSpec, FULL_FILE_SPEC_STRING_LENGTH, SearchDirectory, _TRUNCATE );									// *[1] Replaced strcpy with strncpy_s.
		strncat_s( SearchFileSpec, FULL_FILE_SPEC_STRING_LENGTH, "*.*", _TRUNCATE );											// *[1] Replaced strcat with strncat_s.
		hFindFile = FindFirstFile( SearchFileSpec, &FindFileInfo );
		bFileFound = ( hFindFile != INVALID_HANDLE_VALUE );
		while ( bFileFound )
			{
			bIsFolder = ( ( FindFileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 );
			// Skip the file system's directory entries for the current and parent directory.
			bSpecialDirectory = ( ( FindFileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 &&
									( strcmp( FindFileInfo.cFileName, "." ) == 0 || strcmp( FindFileInfo.cFileName, ".." ) == 0 ) );
			if ( !bSpecialDirectory )
				{
				if ( bIsFolder )
					{
					strncpy_s( NewSearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, SearchDirectory, _TRUNCATE );					// *[1] Replaced strcpy with strncpy_s.
					strncat_s( NewSearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, FindFileInfo.cFileName, _TRUNCATE );			// *[4] Replaced strcat with strncat_s.
					strncat_s( NewSearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );								// *[4] Replaced strcat with strncat_s.
					DeleteFolderContents( NewSearchDirectory, FolderIndent + 4 );
					RemoveDirectory( NewSearchDirectory );
					}
				else
					{
					sprintf_s( Msg, MAX_LOGGING_STRING_LENGTH, "        %sDeleting %s", FolderAnnotation, FindFileInfo.cFileName );		// *[1] Replaced sprintf with sprintf_s.
					LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
					strncpy_s( FullFileSpec, FULL_FILE_SPEC_STRING_LENGTH, SearchDirectory, _TRUNCATE );								// *[1] Replaced strcpy with strncpy_s.
					strncat_s( FullFileSpec, FULL_FILE_SPEC_STRING_LENGTH, FindFileInfo.cFileName, _TRUNCATE );							// *[4] Replaced strcat with strncat_s.
					pExtension = strrchr( FindFileInfo.cFileName, '.' );																// *[1] Added extension requirement before deletion.
					if ( pExtension != 0 && ( _stricmp( pExtension, ".dcm" ) == 0 || _stricmp( pExtension, ".png" ) == 0 || _stricmp( pExtension, ".sdy" ) == 0 ) )	// *[6] Added .png and .sdy.												// *[1]
						DeleteFile( FullFileSpec );
					}
				}
			// Look for another file in the source directory.
			bFileFound = FindNextFile( hFindFile, &FindFileInfo );
			}
		if ( hFindFile != INVALID_HANDLE_VALUE )
			FindClose( hFindFile );
		}
}


void CCustomizePage::DeleteImageFolderContents()
{
	CMainFrame			*pMainFrame;
	char				SearchDirectory[ FULL_FILE_SPEC_STRING_LENGTH ];

	strncpy_s( SearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.ImageDirectory, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
	if ( SearchDirectory[ strlen( SearchDirectory ) - 1 ] != '\\' )
		strncat_s( SearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );										// *[4] Replaced strcat with strncat_s.
	DeleteFolderContents( SearchDirectory, 0 );

	strncpy_s( SearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.InboxDirectory, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
	if ( SearchDirectory[ strlen( SearchDirectory ) - 1 ] != '\\' )
		strncat_s( SearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );										// *[4] Replaced strcat with strncat_s.
	DeleteFolderContents( SearchDirectory, 0 );

	strncpy_s( SearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.WatchDirectory, _TRUNCATE );				// *[1] Replaced strcpy with strncpy_s.
	if ( SearchDirectory[ strlen( SearchDirectory ) - 1 ] != '\\' )
		strncat_s( SearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );										// *[4] Replaced strcat with strncat_s.
	DeleteFolderContents( SearchDirectory, 0 );

	strncpy_s( SearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.BRetrieverDataDirectory, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
	if ( SearchDirectory[ strlen( SearchDirectory ) - 1 ] != '\\' )
		strncat_s( SearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );										// *[4] Replaced strcat with strncat_s.
	strncat_s( SearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, "Queued Files\\", _TRUNCATE );								// *[4] Replaced strcat with strncat_s.
	DeleteFolderContents( SearchDirectory, 0 );

	strncpy_s( SearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.BRetrieverDataDirectory, _TRUNCATE );	// *[1] Replaced strcpy with strncpy_s.
	if ( SearchDirectory[ strlen( SearchDirectory ) - 1 ] != '\\' )
		strncat_s( SearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );										// *[4] Replaced strcat with strncat_s.
	strncat_s( SearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, "Errored Files\\", _TRUNCATE );								// *[4] Replaced strcat with strncat_s.
	DeleteFolderContents( SearchDirectory, 0 );

	strncpy_s( SearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, BViewerConfiguration.DataDirectory, _TRUNCATE );				// *[6] Delete .sdy files, too.
	if ( SearchDirectory[ strlen( SearchDirectory ) - 1 ] != '\\' )
		strncat_s( SearchDirectory, FULL_FILE_SPEC_STRING_LENGTH, "\\", _TRUNCATE );
	DeleteFolderContents( SearchDirectory, 0 );

	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( pMainFrame != 0 )
		pMainFrame -> m_pControlPanel -> m_SelectStudyPage.DeleteStudyList();
}


void CCustomizePage::OnBnClickedClearImageFolders( NMHDR *pNMHDR, LRESULT *pResult )
{
	BOOL			bProceedWithReset;

	if ( !bMakeDumbButtons )
		{
		bProceedWithReset = ThisBViewerApp.WarnUserOfDataResetConsequences();
		if ( bProceedWithReset )
			{
			DeleteImageFolderContents();
			}
		}

	*pResult = 0;
}


void CCustomizePage::UpdateDisplaySettings()
{
	CMainFrame			*pMainFrame;

	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( pMainFrame != 0 )
		pMainFrame -> UpdateDisplayCustomization();
}


void CCustomizePage::OnEditPrimaryMonitorWidthKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	long				IntegerValue;
	char				NumberConvertedToText[ _CVTBUFSIZE ];

	if ( !bMakeDumbButtons )
		{
		if ( m_EditPrimaryMonitorWidth.m_IdleBkgColor != m_EditPrimaryMonitorWidth.m_SpecialBkgColor )
			{
			m_EditPrimaryMonitorWidth.GetWindowText( NumberConvertedToText, _CVTBUFSIZE );
			IntegerValue = atol( NumberConvertedToText );
			if ( IntegerValue > 0 && IntegerValue < 100000 )											// *[1]
				BViewerCustomization.m_PrimaryMonitorWidthInMM = (unsigned long)IntegerValue;			// *[1] Forced integer conversion.
			else																						// *[1]
				BViewerCustomization.m_PrimaryMonitorWidthInMM = 350;									// *[1]
			m_EditPrimaryMonitorWidth.Invalidate( TRUE );
			UpdateDisplaySettings();
			}
		}

	*pResult = 0;
}


void CCustomizePage::OnEditMonitor2WidthKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	long				IntegerValue;
	char				NumberConvertedToText[ _CVTBUFSIZE ];

	if ( !bMakeDumbButtons )
		{
		if ( m_EditMonitor2Width.m_IdleBkgColor != m_EditMonitor2Width.m_SpecialBkgColor )
			{
			m_EditMonitor2Width.GetWindowText( NumberConvertedToText, _CVTBUFSIZE );
			IntegerValue = atol( NumberConvertedToText );
			if ( IntegerValue > 0 && IntegerValue < 100000 )									// *[1]
				BViewerCustomization.m_Monitor2WidthInMM = (unsigned long)IntegerValue;			// *[1] Forced integer conversion.
			else																				// *[1]
				BViewerCustomization.m_Monitor2WidthInMM = 350;									// *[1]
			m_EditMonitor2Width.Invalidate( TRUE );
			UpdateDisplaySettings();
			}
		}

	*pResult = 0;
}


void CCustomizePage::OnEditMonitor3WidthKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	long				IntegerValue;
	char				NumberConvertedToText[ _CVTBUFSIZE ];

	if ( !bMakeDumbButtons )
		{
		if ( m_EditMonitor3Width.m_IdleBkgColor != m_EditMonitor3Width.m_SpecialBkgColor )
			{
			m_EditMonitor3Width.GetWindowText( NumberConvertedToText, _CVTBUFSIZE );
			IntegerValue = atol( NumberConvertedToText );
			if ( IntegerValue > 0 && IntegerValue < 100000 )									// *[1]
				BViewerCustomization.m_Monitor3WidthInMM = (unsigned long)IntegerValue;			// *[1] Forced integer conversion.
			else																				// *[1]
				BViewerCustomization.m_Monitor3WidthInMM = 350;									// *[1]
			m_EditMonitor3Width.Invalidate( TRUE );
			UpdateDisplaySettings();
			}
		}

	*pResult = 0;
}


void CCustomizePage::OnEditPrimaryMonitorHeightKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	long				IntegerValue;
	char				NumberConvertedToText[ _CVTBUFSIZE ];

	if ( !bMakeDumbButtons )
		{
		if ( m_EditPrimaryMonitorHeight.m_IdleBkgColor != m_EditPrimaryMonitorHeight.m_SpecialBkgColor )
			{
			m_EditPrimaryMonitorHeight.GetWindowText( NumberConvertedToText, _CVTBUFSIZE );
			IntegerValue = atol( NumberConvertedToText );
			if ( IntegerValue > 0 && IntegerValue < 100000 )									// *[1]
				BViewerCustomization.m_PrimaryMonitorHeightInMM = (unsigned long)IntegerValue;	// *[1] Forced integer conversion.
			else																				// *[1]
				BViewerCustomization.m_PrimaryMonitorHeightInMM = 450;							// *[1]
			m_EditPrimaryMonitorHeight.Invalidate( TRUE );
			UpdateDisplaySettings();
			}
		}

	*pResult = 0;
}


void CCustomizePage::OnEditMonitor2HeightKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	long				IntegerValue;
	char				NumberConvertedToText[ _CVTBUFSIZE ];

	if ( !bMakeDumbButtons )
		{
		if ( m_EditMonitor2Height.m_IdleBkgColor != m_EditMonitor2Height.m_SpecialBkgColor )
			{
			m_EditMonitor2Height.GetWindowText( NumberConvertedToText, _CVTBUFSIZE );
			IntegerValue = atol( NumberConvertedToText );
			if ( IntegerValue > 0 && IntegerValue < 100000 )									// *[1]
				BViewerCustomization.m_Monitor2HeightInMM = (unsigned long)IntegerValue;		// *[1] Forced integer conversion.
			else																				// *[1]
				BViewerCustomization.m_Monitor2HeightInMM = 450;								// *[1]
			m_EditMonitor2Height.Invalidate( TRUE );
			UpdateDisplaySettings();
			}
		}

	*pResult = 0;
}


void CCustomizePage::OnEditMonitor3HeightKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	long				IntegerValue;
	char				NumberConvertedToText[ _CVTBUFSIZE ];

	if ( !bMakeDumbButtons )
		{
		if ( m_EditMonitor3Height.m_IdleBkgColor != m_EditMonitor3Height.m_SpecialBkgColor )
			{
			m_EditMonitor3Height.GetWindowText( NumberConvertedToText, _CVTBUFSIZE );
			IntegerValue = atol( NumberConvertedToText );
			if ( IntegerValue > 0 && IntegerValue < 100000 )									// *[1]
				BViewerCustomization.m_Monitor3HeightInMM = (unsigned long)IntegerValue;		// *[1] Forced integer conversion.
			else																				// *[1]
				BViewerCustomization.m_Monitor3HeightInMM = 450;								// *[1]
			m_EditMonitor3Height.Invalidate( TRUE );
			UpdateDisplaySettings();
			}
		}

	*pResult = 0;
}


static DISPLAY_RENDERING_METHOD_ITEM		DisplayRenderingMethodArray[] =
{
	{ "Conventional 8-bit Color", RENDER_METHOD_8BIT_COLOR },
	{ "Packed 10-bit Grayscale", RENDER_METHOD_16BIT_PACKED_GRAYSCALE },
	{ "30-bit Color (Deep Color)", RENDER_METHOD_30BIT_COLOR },
	{ "", 0 }
};


// *[3] Added the following function to look up the rendering method text for a specified method.
char* GetRenderingMethodText( unsigned long RenderingMethod )
{
	BOOL								bEndOfList;
	DISPLAY_RENDERING_METHOD_ITEM		*pCurrentRenderingMethodItem;
	int									nRenderingMethod;
	char								*pRenderingMethodText = 0;

	nRenderingMethod = 0;
	do
		{
		pCurrentRenderingMethodItem = &DisplayRenderingMethodArray[ nRenderingMethod ];
		bEndOfList = ( strlen( pCurrentRenderingMethodItem -> RenderingMethodName ) == 0 );
		if ( !bEndOfList )
			{
			if ( RenderingMethod == pCurrentRenderingMethodItem -> RenderingMethodValue )
				{
				pRenderingMethodText = pCurrentRenderingMethodItem -> RenderingMethodName;
				bEndOfList = TRUE;		// Indicate a match was found.
				}
			}
		nRenderingMethod++;
		}
	while ( !bEndOfList );

	return pRenderingMethodText;
}



BOOL CCustomizePage::LoadRenderingMethodSelectionLists()
{
	BOOL								bNoError = TRUE;
	BOOL								bEndOfList;
	DISPLAY_RENDERING_METHOD_ITEM		*pCurrentRenderingMethodItem;
	int									nRenderingMethod;

	m_ComboBoxSelectPrimaryMonitorRenderingMethod.ResetContent();
	m_ComboBoxSelectPrimaryMonitorRenderingMethod.SetWindowTextA( "Display Panel Rendering Method" );
	m_ComboBoxSelectMonitor2RenderingMethod.ResetContent();
	m_ComboBoxSelectMonitor2RenderingMethod.SetWindowTextA( "Display Panel Rendering Method" );
	m_ComboBoxSelectMonitor3RenderingMethod.ResetContent();
	m_ComboBoxSelectMonitor3RenderingMethod.SetWindowTextA( "Display Panel Rendering Method" );

	nRenderingMethod = 0;

	// Insert the possible image rendering methods into the list for each combo box.
	do
		{
		pCurrentRenderingMethodItem = &DisplayRenderingMethodArray[ nRenderingMethod ];
		bEndOfList = ( strlen( pCurrentRenderingMethodItem -> RenderingMethodName ) == 0 );
		if ( !bEndOfList )
			{
			m_ComboBoxSelectPrimaryMonitorRenderingMethod.AddString( pCurrentRenderingMethodItem -> RenderingMethodName );
			m_ComboBoxSelectMonitor2RenderingMethod.AddString( pCurrentRenderingMethodItem -> RenderingMethodName );
			m_ComboBoxSelectMonitor3RenderingMethod.AddString( pCurrentRenderingMethodItem -> RenderingMethodName );
			}
		nRenderingMethod++;
		}
	while ( !bEndOfList );

	if ( BViewerCustomization.m_PrimaryMonitorRenderingMethod > 0 && BViewerCustomization.m_PrimaryMonitorRenderingMethod <= MAX_RENDER_METHOD )
		m_ComboBoxSelectPrimaryMonitorRenderingMethod.SetCurSel( BViewerCustomization.m_PrimaryMonitorRenderingMethod - 1 );
	else
		m_ComboBoxSelectPrimaryMonitorRenderingMethod.SetCurSel( RENDER_METHOD_8BIT_COLOR - 1 );
	m_ComboBoxSelectPrimaryMonitorRenderingMethod.UpdateWindow();
		
	if ( BViewerCustomization.m_Monitor2RenderingMethod > 0 && BViewerCustomization.m_Monitor2RenderingMethod <= MAX_RENDER_METHOD )
		m_ComboBoxSelectMonitor2RenderingMethod.SetCurSel( BViewerCustomization.m_Monitor2RenderingMethod - 1 );
	else
		m_ComboBoxSelectMonitor2RenderingMethod.SetCurSel( RENDER_METHOD_8BIT_COLOR - 1 );
	m_ComboBoxSelectMonitor2RenderingMethod.Invalidate( TRUE );

	if ( BViewerCustomization.m_Monitor3RenderingMethod > 0 && BViewerCustomization.m_Monitor3RenderingMethod <= MAX_RENDER_METHOD )
		m_ComboBoxSelectMonitor3RenderingMethod.SetCurSel( BViewerCustomization.m_Monitor3RenderingMethod - 1 );
	else
		m_ComboBoxSelectMonitor3RenderingMethod.SetCurSel( RENDER_METHOD_8BIT_COLOR - 1 );
	m_ComboBoxSelectMonitor3RenderingMethod.Invalidate( TRUE );

	return bNoError;
}


void CCustomizePage::OnPrimaryMonitorRenderingMethodSelected()
{
	int									nRenderingMethod;
	int									nItemIndex;

	if ( !bMakeDumbButtons )
		{
		nItemIndex = m_ComboBoxSelectPrimaryMonitorRenderingMethod.GetCurSel();
		nRenderingMethod = nItemIndex + 1;
		BViewerCustomization.m_PrimaryMonitorRenderingMethod = (unsigned short)nRenderingMethod;	// *[1] Forced data type conversion.
		}
}


void CCustomizePage::OnMonitor2RenderingMethodSelected()
{
	int									nRenderingMethod;
	int						nItemIndex;

	if ( !bMakeDumbButtons )
		{
		nItemIndex = m_ComboBoxSelectMonitor2RenderingMethod.GetCurSel();
		nRenderingMethod = nItemIndex + 1;
		BViewerCustomization.m_Monitor2RenderingMethod = (unsigned short)nRenderingMethod;			// *[1] Forced data type conversion.
		}
}


void CCustomizePage::OnMonitor3RenderingMethodSelected()
{
	int						nRenderingMethod;
	int						nItemIndex;

	if ( !bMakeDumbButtons )
		{
		nItemIndex = m_ComboBoxSelectMonitor3RenderingMethod.GetCurSel();
		nRenderingMethod = nItemIndex + 1;
		BViewerCustomization.m_Monitor3RenderingMethod = (unsigned short)nRenderingMethod;			// *[1] Forced data type conversion.
		}
}


BOOL CCustomizePage::WriteBViewerConfiguration()
{
	BOOL					bNoError;
	char					FileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					ConfigurationDirectory[ FILE_PATH_STRING_LENGTH ];
	FILE					*pBViewerCfgFile;
	size_t					FileSizeInBytes;
	size_t					nBytesToWrite;
	size_t					nBytesWritten;
	BOOL					bFileWrittenSuccessfully = TRUE;

	if ( bOKToSaveReaderInfo )
		{
		bFileWrittenSuccessfully = FALSE;
		FileSizeInBytes = sizeof( CCustomization );
		strncpy_s( ConfigurationDirectory, MAX_CFG_STRING_LENGTH, BViewerConfiguration.ConfigDirectory, _TRUNCATE );	// *[3] Replaced strncat with strncpy_s.
		if ( ConfigurationDirectory[ strlen( ConfigurationDirectory ) - 1 ] != '\\' )
			strncat_s( ConfigurationDirectory, MAX_CFG_STRING_LENGTH, "\\", _TRUNCATE );								// *[3] Replaced strcat with strncat_s.
		// Check existence of path to configuration directory.
		bNoError = SetCurrentDirectory( ConfigurationDirectory );
		if ( bNoError )
			{
			strncpy_s( FileSpec, FULL_FILE_SPEC_STRING_LENGTH, ConfigurationDirectory, _TRUNCATE );						// *[1] Replaced strcpy with strncpy_s.
			strncat_s( FileSpec, FULL_FILE_SPEC_STRING_LENGTH, "CriticalData2.sav", _TRUNCATE );						// *[3] Replaced strcat with strncat_s.
			pBViewerCfgFile = fopen( FileSpec, "wb" );
			if ( pBViewerCfgFile != 0 )
				{
				nBytesToWrite = sizeof( unsigned long);
				nBytesWritten = fwrite( &FileSizeInBytes, 1, nBytesToWrite, pBViewerCfgFile );
				if ( nBytesWritten == nBytesToWrite )		// If the table size was written correctly...
					{
					nBytesToWrite = FileSizeInBytes;
					nBytesWritten = fwrite( &BViewerCustomization, 1, nBytesToWrite, pBViewerCfgFile );
					if ( nBytesWritten == nBytesToWrite )
						bFileWrittenSuccessfully = TRUE;
					}
				fclose( pBViewerCfgFile );
				}
			}
		}

	return bFileWrittenSuccessfully;
}


BOOL CCustomizePage::OnNotify( WPARAM wParam, LPARAM lParam, LRESULT *pResult )
{
	return CPropertyPage::OnNotify( wParam, lParam, pResult );
}


typedef struct
	{
	unsigned long		EditControlID;
	unsigned long		BufferSize;
	long				DataStructureOffset;
	} EDIT_TEXT_DESTINATION;


static EDIT_TEXT_DESTINATION	EditIDArray[] =
		{
			{ IDC_EDIT_READER_LAST_NAME,		MAX_USER_INFO_LENGTH,		offsetof( READER_PERSONAL_INFO, LastName ) },
			{ IDC_EDIT_LOGIN_NAME,				MAX_USER_INFO_LENGTH,		offsetof( READER_PERSONAL_INFO, LoginName ) },
			{ IDC_EDIT_READER_ID,				12,							offsetof( READER_PERSONAL_INFO, ID ) },
			{ IDC_EDIT_READER_INITIALS,			4,							offsetof( READER_PERSONAL_INFO, Initials ) },
			{ IDC_EDIT_AE_TITLE,				20,							offsetof( READER_PERSONAL_INFO, AE_TITLE ) },
			{ IDC_EDIT_READER_STREET_ADDRESS,	64,							offsetof( READER_PERSONAL_INFO, StreetAddress ) },
			{ IDC_EDIT_READER_CITY,				MAX_USER_INFO_LENGTH,		offsetof( READER_PERSONAL_INFO, City ) },
			{ IDC_EDIT_READER_STATE,			4,							offsetof( READER_PERSONAL_INFO, State ) },
			{ IDC_EDIT_READER_ZIPCODE,			12,							offsetof( READER_PERSONAL_INFO, ZipCode ) },
			{ 0,								0,							0 }
		};


BOOL CCustomizePage::OnKillActive()
{
	EDIT_TEXT_DESTINATION	*pEditTextDestination;
	unsigned long			EditID;
	int						nItem;
	TomEdit					*pEditControl;
	char					TextString[ 64 ];
	char					*pTextBuffer;
	unsigned long			BufferSize;				// *[3] Added variable.

	// Loop through all the data entry fields to be sure that their current
	// data values have been captured.
	nItem = 0;
	do
		{
		pEditTextDestination = &EditIDArray[ nItem ];
		EditID = pEditTextDestination -> EditControlID;
		BufferSize = pEditTextDestination -> BufferSize;		// *[3] Added initialization.
		if ( EditID != 0 )
			{
			pEditControl = (TomEdit*)GetDlgItem( EditID );
			if ( pEditControl != 0 )
				{
				pEditControl -> GetWindowText( TextString, 64 );
				pTextBuffer = (char*)&BViewerCustomization.m_ReaderInfo + pEditTextDestination -> DataStructureOffset;
				strncpy_s( pTextBuffer, BufferSize, TextString, _TRUNCATE );										// *[3] Replaced strncat with strncpy_s.
				}
			}
		nItem++;
		}
	while ( EditID != 0 );

	memcpy( &LoggedInReaderInfo, &BViewerCustomization.m_ReaderInfo, sizeof( READER_PERSONAL_INFO ) );
	WriteBViewerConfiguration();
	WriteUserList();					// *[5] Changed function location.
	if ( m_pControlTip != 0 )
		{
		delete m_pControlTip;
		m_pControlTip = 0;
		}

	return CPropertyPage::OnKillActive();
}


void CCustomizePage::OnAppAbout( NMHDR *pNMHDR, LRESULT *pResult )
{
	CTextWindow			*pAboutBox;
	RECT				ClientRect;
	int					ClientWidth;
	int					ClientHeight;
	
	GetClientRect( &ClientRect );
	ClientWidth = ClientRect.right - ClientRect.left;
	ClientHeight = ClientRect.bottom - ClientRect.top;

	pAboutBox = new CTextWindow;
	if ( pAboutBox != 0 )
		{
		pAboutBox -> SetPosition( ( ClientWidth - 620 ) / 2, ( ClientHeight - 550 ) / 2, this, PopupWindowClass );
		pAboutBox -> ReadTextFileForDisplay( BViewerConfiguration.BViewerAboutFile );
		pAboutBox -> BringWindowToTop();
		pAboutBox -> SetFocus();
		}

	*pResult = 0;
}


void CCustomizePage::OnBnClickedTechnicalRequirements( NMHDR *pNMHDR, LRESULT *pResult )
{
	CTextWindow			*pTechnicalRequirementsBox;
	RECT				ClientRect;
	int					ClientWidth;
	int					ClientHeight;
	
	GetClientRect( &ClientRect );
	ClientWidth = ClientRect.right - ClientRect.left;
	ClientHeight = ClientRect.bottom - ClientRect.top;

	pTechnicalRequirementsBox = new CTextWindow;
	if ( pTechnicalRequirementsBox != 0 )
		{
		pTechnicalRequirementsBox -> SetPosition( ( ClientWidth - 620 ) / 2, ( ClientHeight - 550 ) / 2, this, PopupWindowClass );
		pTechnicalRequirementsBox -> BringWindowToTop();
		pTechnicalRequirementsBox -> SetFocus();
		}
	pTechnicalRequirementsBox -> ReadTextFileForDisplay( BViewerConfiguration.BViewerTechnicalRequirementsFile );


	*pResult = 0;
}


static void ProcessEditUserResponse( void *pResponseDialog )
{
	CPopupDialog			*pPopupDialog;
	
	pPopupDialog = (CPopupDialog*)pResponseDialog;
	if ( pPopupDialog != 0 )			// *[1] Added safety check.
		delete pPopupDialog;
}


void CCustomizePage::EditUserInfo( BOOL bSetInitialReader )			// *[5] Created this as a separate function.
{
	CSelectUser						*pReaderSelectionScreen;
	CMainFrame						*pMainFrame;
	USER_NOTIFICATION				UserNoticeOfTermination;
	BOOL							bReaderHasChanged;
	BOOL							bSuccessfulLogin;
	char							Msg[ MAX_EXTRA_LONG_STRING_LENGTH ];

	pReaderSelectionScreen = new CSelectUser( this, NULL, bSetInitialReader );
	if ( pReaderSelectionScreen != 0 )
		{
		pReaderSelectionScreen -> DoModal();
		bReaderHasChanged = pReaderSelectionScreen -> m_bChangingCurrentReader;
		delete pReaderSelectionScreen;
		pReaderSelectionScreen = 0;
		ResetReaderInfo();
		if ( RegisteredUserList == 0 )													// *[5] If there is no legitimate reader logged in
			{
			// Notify user of shutdown.
			pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
			if ( pMainFrame != 0 )
				{
				strncpy_s( UserNoticeOfTermination.Source, 16, BViewerConfiguration.ProgramName, _TRUNCATE );
				UserNoticeOfTermination.ModuleCode = 0;
				UserNoticeOfTermination.ErrorCode = 0;
				strncpy_s( UserNoticeOfTermination.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH,
													"Shutting down.\nBViewer requires a registered reader.\n", _TRUNCATE );
				UserNoticeOfTermination.TypeOfUserResponseSupported = USER_RESPONSE_TYPE_CONTINUE;
				UserNoticeOfTermination.UserNotificationCause = USER_NOTIFICATION_CAUSE_NEEDS_ACKNOWLEDGMENT;
				strncpy_s( UserNoticeOfTermination.SuggestedActionText, MAX_CFG_STRING_LENGTH, "Restart BViewer for a new prompt.\n", _TRUNCATE );
				UserNoticeOfTermination.UserResponseCode = 0L;
				UserNoticeOfTermination.TextLinesRequired = 10;
				pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
				if ( pMainFrame != 0 )
					pMainFrame -> ProcessUserNotificationAndWaitForResponse( &UserNoticeOfTermination );
				}
			ThisBViewerApp.TerminateTimers();											// *[5]  exit the application.
			AfxGetMainWnd() -> SendMessage( WM_CLOSE );
			}
		else if ( bReaderHasChanged )													// *[5] Force a new login if the current reader changed.
			{
			bSuccessfulLogin = FALSE;
			if ( strlen( BViewerCustomization.m_ReaderInfo.LoginName ) == 0 || strlen( BViewerCustomization.m_ReaderInfo.EncodedPassword ) == 0 )
				bSuccessfulLogin = TRUE;												// If no login name has been set or this is a new installation, proceed.
			else
				bSuccessfulLogin = SuccessfulLogin();
			if ( !bSuccessfulLogin )
				{
				// Notify user of shutdown.
				pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
				if ( pMainFrame != 0 )
					{
					strncpy_s( UserNoticeOfTermination.Source, 16, BViewerConfiguration.ProgramName, _TRUNCATE );
					UserNoticeOfTermination.ModuleCode = 0;
					UserNoticeOfTermination.ErrorCode = 0;
					strncpy_s( UserNoticeOfTermination.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH,
														"Shutting down.\nBViewer requires a registered reader.\n", _TRUNCATE );
					UserNoticeOfTermination.TypeOfUserResponseSupported = USER_RESPONSE_TYPE_CONTINUE;
					UserNoticeOfTermination.UserNotificationCause = USER_NOTIFICATION_CAUSE_NEEDS_ACKNOWLEDGMENT;
					strncpy_s( UserNoticeOfTermination.SuggestedActionText, MAX_CFG_STRING_LENGTH, "Restart BViewer for a new prompt.\n", _TRUNCATE );
					UserNoticeOfTermination.UserResponseCode = 0L;
					UserNoticeOfTermination.TextLinesRequired = 10;
					pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
					if ( pMainFrame != 0 )
						pMainFrame -> ProcessUserNotificationAndWaitForResponse( &UserNoticeOfTermination );
					}
				ThisBViewerApp.TerminateTimers();										// *[5]  exit the application.
				AfxGetMainWnd() -> SendMessage( WM_CLOSE );
				}
			else
				{
				sprintf_s( Msg, MAX_EXTRA_LONG_STRING_LENGTH, "Current reader logged in: %s", BViewerCustomization.m_ReaderInfo.ReportSignatureName );	// *[5] Log the current reader.
				LogMessage( Msg, MESSAGE_TYPE_NORMAL_LOG );							// *[5]	
				}
			}
		}
}


void CCustomizePage::OnBnClickedEditUser( NMHDR *pNMHDR, LRESULT *pResult )			// *[5] Added user selection functionality.
{
	if ( !bMakeDumbButtons )
		EditUserInfo( FALSE );														// *[5] Created this as a separate function.

	*pResult = 0;
}


void CCustomizePage::OnBnClickedBeginNewTestSession( NMHDR *pNMHDR, LRESULT *pResult)
{
	CMainFrame						*pMainFrame;
	USER_NOTIFICATION				UserNoticeOfTermination;

	RemoveCurrentReader();															// *[5] Added this function call.
	ClearReaderInfoDisplay();
	memset( &BViewerCustomization.m_ReaderInfo, '\0', sizeof( READER_PERSONAL_INFO ) );
	WriteBViewerConfiguration();
	WriteUserList();																// *[5] Change this function's location.
	if ( RegisteredUserList == 0 )													// *[3] If there is no legitimate reader logged in
		{
		// Notify user of shutdown.
		strncpy_s( UserNoticeOfTermination.Source, 16, BViewerConfiguration.ProgramName, _TRUNCATE );
		UserNoticeOfTermination.ModuleCode = 0;
		UserNoticeOfTermination.ErrorCode = 0;
		strncpy_s( UserNoticeOfTermination.NoticeText, MAX_EXTRA_LONG_STRING_LENGTH,
											"Shutting down.\nBViewer requires a\nregistered reader.\n", _TRUNCATE );
		UserNoticeOfTermination.TypeOfUserResponseSupported = USER_RESPONSE_TYPE_CONTINUE;
		UserNoticeOfTermination.UserNotificationCause = USER_NOTIFICATION_CAUSE_NEEDS_ACKNOWLEDGMENT;
		strncpy_s( UserNoticeOfTermination.SuggestedActionText, MAX_CFG_STRING_LENGTH, "Restart BViewer for a new prompt.\n", _TRUNCATE );
		UserNoticeOfTermination.UserResponseCode = 0L;
		UserNoticeOfTermination.TextLinesRequired = 10;
		pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
		if ( pMainFrame != 0 )
			pMainFrame -> ProcessUserNotificationAndWaitForResponse( &UserNoticeOfTermination );
		ThisBViewerApp.TerminateTimers();											// *[3]  exit the application.
		AfxGetMainWnd() -> SendMessage( WM_CLOSE );
		}
}


void CCustomizePage::OnMouseMove( UINT nFlags, CPoint MouseCursorLocation )
{
	// If the window owning the controls (this window) receives a WM_MOUSEMOVE message, this
	// indicates that the mouse is not over any of the controls.  Therefore, disable the
	// control tip.
	if ( m_pControlTip -> m_pTipText != 0 && strlen( m_pControlTip -> m_pTipText ) > 0 )
		{
		m_pControlTip -> m_pTipText = "";
		m_pControlTip -> ShowTipText( MouseCursorLocation, this );
		}
	
	CDialog::OnMouseMove( nFlags, MouseCursorLocation );
}
