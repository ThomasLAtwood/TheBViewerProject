// CustomizePage.cpp : Implementation file for the CCustomizePage class of
//	CPropertyPage, which implements the "Set Up BViewer" tab of the main Control Panel.
//
//	Written by Thomas L. Atwood
//	P.O. Box 1089
//	West Fork, Arkansas 72774
//	(479)445-4690
//	TomAtwood@Earthlink.net
//
//	Copyright � 2010 CDC
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


extern CBViewerApp					ThisBViewerApp;
extern CONFIGURATION				BViewerConfiguration;
extern CCustomization				BViewerCustomization;
extern LIST_HEAD					RegisteredUserList;
extern READER_PERSONAL_INFO			*pCurrentReaderInfo;		// Points at item in user list that matches login.
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

				m_StaticGrayscaleResolution( "Display\nGrayscale Resolution", 250, 40, 18, 9, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE | CONTROL_VISIBLE,
									IDC_STATIC_GRAYSCALE_RESOLUTION,
										"Enter the number of grayscale bits for each pixel.\n"
										"For the primary (\"desktop\") color monitor this is usually 8.\n"
										"For the better medical image monitors it could be 10\n"
										"or 12 bits of greyscale resolution." ),
				m_StaticGrayscaleBitDepth( "Display Monitor\nGrayscale Bit Depth Capability\n(8, 10 or 12 bits per pixel)", 250, 50, 14, 7, 6,
									COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_GRAYSCALE_BIT_DEPTH,
										"Enter the number of grayscale bits for each pixel.\n"
										"For the primary (\"desktop\") color monitor this is usually 8.\n"
										"For the better medical image monitors it could be 10\n"
										"or 12 bits of greyscale resolution." ),
				m_EditPrimaryMonitorGrayscaleBitDepth( "", 60, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_NUMERIC, IDC_EDIT_PRIMARY_MONITOR_BIT_DEPTH ),
				m_EditMonitor2GrayscaleBitDepth( "", 60, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_NUMERIC, IDC_EDIT_MONITOR2_BIT_DEPTH ),
				m_EditMonitor3GrayscaleBitDepth( "", 60, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_NUMERIC, IDC_EDIT_MONITOR3_BIT_DEPTH ),

				m_StaticReaderIdentification( "Reader Identification", 230, 20, 18, 9, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_TOP_JUSTIFIED | CONTROL_VISIBLE,
									IDC_STATIC_READER_IDENTIFICATION,
										"Enter the information describing the reader.\n"
										"Some of this information is included on each report." ),
				m_StaticReaderLastName( "Last Name (Family Name)", 200, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_READER_LAST_NAME,
										"Enter this reader's last name." ),
				m_EditReaderLastName( "", 160, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_NONE, IDC_EDIT_READER_LAST_NAME ),

				m_StaticLoginName( "Login Name", 100, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_LOGIN_NAME,
										"This is the user name that you will\n"
										"provide when you log into BViewer." ),
				m_EditLoginName( "", 120, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_NONE, IDC_EDIT_LOGIN_NAME ),

				m_StaticReaderID( "ID", 200, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_READER_SSN,
										"Enter numerical digits only.\n"
										"No spaces, dashes, etc." ),
				m_EditReaderID( "", 120, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
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
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_NONE, IDC_EDIT_LOGIN_PASSWORD ),

				m_StaticReaderInitials( "Initials", 60, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_READER_INITIALS,
										"Enter your initials for the report." ),
				m_EditReaderInitials( "", 70, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
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
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_NONE, IDC_EDIT_AE_TITLE ),

				m_StaticReaderReportSignatureName( "Signature Name for Report ", 200, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_READER_SIGNATURE_NAME,
										"This will appear on the report." ),
				m_EditReaderReportSignatureName( "", 460, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_NONE, IDC_EDIT_READER_SIGNATURE_NAME ),

				m_StaticReaderStreetAddress( "Street Address", 120, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_READER_STREET_ADDRESS,
										"This will appear on the report." ),
				m_EditReaderStreetAddress( "", 300, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_NONE, IDC_EDIT_READER_STREET_ADDRESS ),

				m_StaticReaderCity( "City", 40, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_READER_CITY,
										"This will appear on the report." ),
				m_EditReaderCity( "", 200, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_NONE, IDC_EDIT_READER_CITY ),

				m_StaticReaderState( "State", 50, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_READER_STATE,
										"This will appear on the report." ),
				m_EditReaderState( "", 50, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_NONE, IDC_EDIT_READER_STATE ),

				m_StaticReaderZipCode( "Zip Code", 70, 20, 14, 7, 6, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_VISIBLE,
									IDC_STATIC_READER_ZIPCODE,
										"This will appear on the report." ),
				m_EditReaderZipCode( "123", 120, 20, 16, 8, 6, VARIABLE_PITCH_FONT, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
									CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
									EDIT_VALIDATION_NONE, IDC_EDIT_READER_ZIPCODE ),

				m_GroupEditSequencing( GROUP_EDIT, GROUP_SEQUENCING, 20,
									&m_EditPrimaryMonitorWidth, &m_EditMonitor2Width, &m_EditMonitor3Width,
									&m_EditPrimaryMonitorHeight, &m_EditMonitor2Height, &m_EditMonitor3Height,
									&m_EditPrimaryMonitorGrayscaleBitDepth, &m_EditMonitor2GrayscaleBitDepth, &m_EditMonitor3GrayscaleBitDepth,
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
				m_ButtonAddUser( "Add New User", 150, 30, 14, 7, 6,
									COLOR_BLACK, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_VISIBLE |
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
									IDC_BUTTON_ADD_USER,
										"A single BViewer workstation can support multiple\n"
										"readers.  You can add another authorized user here." ),
				m_ButtonEditUser( "Edit Existing User", 150, 30, 14, 7, 6,
									COLOR_BLACK, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_VISIBLE |
									CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED,
									IDC_BUTTON_EDIT_USER,
										"This shows how to change user information." ),
				m_StaticSelectCountry( "Select Country", 200, 30, 14, 7, 6,
										COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | CONTROL_MULTILINE | CONTROL_VISIBLE,
										IDC_STATIC_SELECT_COUNTRY ),
				m_ComboBoxSelectCountry( "", 280, 300, 18, 9, 5, VARIABLE_PITCH_FONT,
										COLOR_BLACK, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR, COLOR_CONFIG_SELECTOR,
										CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_VSCROLL | EDIT_BORDER | CONTROL_VISIBLE,
										EDIT_VALIDATION_NONE, IDC_COMBO_SELECT_COUNTRY ),
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
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_ADD_USER, OnBnClickedAddUser )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_EDIT_USER, OnBnClickedEditUser )
	ON_NOTIFY( WM_LBUTTONUP,  IDC_BUTTON_BEGIN_NEW_TEST_SESSION, OnBnClickedBeginNewTestSession )

	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_PRIMARY_MONITOR_WIDTH, OnEditPrimaryMonitorWidthKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_MONITOR2_WIDTH, OnEditMonitor2WidthKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_MONITOR3_WIDTH, OnEditMonitor3WidthKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_PRIMARY_MONITOR_HEIGHT, OnEditPrimaryMonitorHeightKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_MONITOR2_HEIGHT, OnEditMonitor2HeightKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_MONITOR3_HEIGHT, OnEditMonitor3HeightKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_PRIMARY_MONITOR_BIT_DEPTH, OnEditPrimaryMonitorBitDepthKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_MONITOR2_BIT_DEPTH, OnEditMonitor2BitDepthKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_MONITOR3_BIT_DEPTH, OnEditMonitor3BitDepthKillFocus )

	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_READER_LAST_NAME, OnEditReaderLastNameKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_LOGIN_NAME, OnEditLoginNameKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_READER_ID, OnEditReaderIDKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_LOGIN_PASSWORD, OnEditLoginPasswordKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_READER_INITIALS, OnEditReaderInitialsKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_AE_TITLE, OnEditAE_TitleKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_READER_SIGNATURE_NAME, OnEditReaderReportSignatureNameKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_READER_STREET_ADDRESS, OnEditReaderStreetAddressKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_READER_CITY, OnEditReaderCityKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_READER_STATE, OnEditReaderStateKillFocus )
	ON_NOTIFY( WM_KILLFOCUS, IDC_EDIT_READER_ZIPCODE, OnEditReaderZipCodeKillFocus )
	ON_CBN_SELENDOK( IDC_COMBO_SELECT_COUNTRY, OnCountrySelected )

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
	m_EditPrimaryMonitorGrayscaleBitDepth.SetPosition( 950, 135, this );
	m_EditMonitor2GrayscaleBitDepth.SetPosition( 950, 165, this );
	m_EditMonitor3GrayscaleBitDepth.SetPosition( 950, 195, this );
	
	m_StaticReaderIdentification.SetPosition( 490, 260, this );
	m_StaticReaderLastName.SetPosition( 440, 300, this );
	m_EditReaderLastName.SetPosition( 640, 300, this );

	m_StaticLoginName.SetPosition( 820, 300, this );
	m_EditLoginName.SetPosition( 980, 300, this );
	
	m_StaticLoginPassword.SetPosition( 820, 330, this );
	m_EditLoginPassword.SetPosition( 980, 330, this );
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
		m_StaticReaderReportSignatureName.m_ControlText = "Name (Last, First, Middle)";
		m_StaticReaderReportSignatureName.Invalidate();
		m_EditReaderReportSignatureName.SetPosition( 640, 410, this );
		}
	else if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_TEST )
		{
		m_StaticReaderInitials.SetPosition( 440, 360, this );
		m_EditReaderInitials.SetPosition( 640, 360, this );
		m_ButtonBeginNewTestSession.SetPosition( 950, 610, this );

		m_StaticReaderReportSignatureName.SetPosition( 440, 410, this );
		m_StaticReaderReportSignatureName.m_ControlText = "Name (Last, First, Middle)";
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

	m_ButtonInstallStandards.SetPosition( 20, 260, this );
	m_ButtonAboutBViewer.SetPosition( 20, 390, this );
	m_ButtonTechnicalRequirements.SetPosition( 20, 430, this );
	m_ButtonControlBRetriever.SetPosition( 20, 550, this );
	m_ButtonSetNetworkAddress.SetPosition( 20, 600, this );
	m_ButtonClearImageFolders.SetPosition( 230, 600, this );

	m_StaticSelectCountry.SetPosition( 440, 540, this );
	m_ComboBoxSelectCountry.SetPosition( 440, 565, this );

	m_StaticHelpfulTips.SetPosition( 440, 600, this );

	// Only enable the following buttons after the first user has entered
	// his reader information.
	if ( strlen( BViewerCustomization.m_ReaderInfo.LastName ) != 0 )
		{
		if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL ||
					BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_NIOSH )
			{
			m_ButtonAddUser.SetPosition( 770, 560, this );
			m_ButtonEditUser.SetPosition( 950, 560, this );
			}
		}

	m_bPageIsInitialized = TRUE;
	ResetPage();
	m_bImageDisplaysAreConfigured = TRUE;
	LoadCountrySelectionList();

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
		// Set a default AE_TITLE.
		if ( strlen( BViewerCustomization.m_ReaderInfo.AE_TITLE ) == 0 )
			{
			strcpy( BViewerCustomization.m_ReaderInfo.AE_TITLE, "BViewer" );
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
	m_EditAE_Title.SetWindowText( "" );
	m_EditReaderStreetAddress.SetWindowText( "" );
	m_EditReaderCity.SetWindowText( "" );
	m_EditReaderState.SetWindowText( "" );
	m_EditReaderZipCode.SetWindowText( "" );
}


static COUNTRY_INFO		CountryInfoArray[] =
{
	{ "Afghanistan", DATE_FORMAT_DMY },
	{ "�land Islands", DATE_FORMAT_YMD },
	{ "Albania", DATE_FORMAT_DMY },
	{ "Algeria", DATE_FORMAT_DMY },
	{ "American Samoa", DATE_FORMAT_MDY },
	{ "Andorra", DATE_FORMAT_DMY },
	{ "Angola", DATE_FORMAT_DMY },
	{ "Anguilla", DATE_FORMAT_DMY },
	{ "Antigua and Barbuda", DATE_FORMAT_DMY },
	{ "Argentina", DATE_FORMAT_DMY },
	{ "Armenia", DATE_FORMAT_DMY },
	{ "Aruba", DATE_FORMAT_DMY },
	{ "Ascension", DATE_FORMAT_DMY },
	{ "Australia", DATE_FORMAT_DMY },
	{ "Austria", DATE_FORMAT_DMY },
	{ "Azerbaijan", DATE_FORMAT_DMY },
	{ "Bahamas", DATE_FORMAT_DMY },
	{ "Bahrain", DATE_FORMAT_DMY },
	{ "Bangladesh", DATE_FORMAT_DMY },
	{ "Barbados", DATE_FORMAT_DMY },
	{ "Belarus", DATE_FORMAT_DMY },
	{ "Belgium", DATE_FORMAT_DMY },
	{ "Belize", DATE_FORMAT_DMY },
	{ "Benin", DATE_FORMAT_DMY },
	{ "Bermuda", DATE_FORMAT_DMY },
	{ "Bhutan", DATE_FORMAT_YMD },
	{ "Bolivia", DATE_FORMAT_DMY },
	{ "Bonaire", DATE_FORMAT_DMY },
	{ "Bosnia and Herzegovina", DATE_FORMAT_DMY },
	{ "Botswana", DATE_FORMAT_DMY },
	{ "Brazil", DATE_FORMAT_DMY },
	{ "British Indian Ocean Territory", DATE_FORMAT_DMY },
	{ "British Virgin Islands", DATE_FORMAT_DMY },
	{ "Brunei", DATE_FORMAT_DMY },
	{ "Bulgaria", DATE_FORMAT_DMY },
	{ "Burkina Faso", DATE_FORMAT_DMY },
	{ "Burundi", DATE_FORMAT_DMY },
	{ "Cambodia", DATE_FORMAT_DMY },
	{ "Cameroon", DATE_FORMAT_DMY },
	{ "Canada", DATE_FORMAT_YMD },
	{ "Cape Verde", DATE_FORMAT_DMY },
	{ "Cayman Islands", DATE_FORMAT_DMY },
	{ "Central African Republic", DATE_FORMAT_DMY },
	{ "Chad", DATE_FORMAT_DMY },
	{ "Chile", DATE_FORMAT_DMY },
	{ "China", DATE_FORMAT_DMY },
	{ "Cocos (Keeling) Islands", DATE_FORMAT_DMY },
	{ "Colombia", DATE_FORMAT_DMY },
	{ "Comoros", DATE_FORMAT_DMY },
	{ "Congo", DATE_FORMAT_DMY },
	{ "Cook Islands", DATE_FORMAT_DMY },
	{ "Costa Rica", DATE_FORMAT_DMY },
	{ "Croatia", DATE_FORMAT_DMY },
	{ "Cuba", DATE_FORMAT_DMY },
	{ "Cura�ao", DATE_FORMAT_DMY },
	{ "Cyprus", DATE_FORMAT_DMY },
	{ "Czech Republic", DATE_FORMAT_DMY },
	{ "Denmark", DATE_FORMAT_DMY },
	{ "Djibouti", DATE_FORMAT_DMY },
	{ "Dominica", DATE_FORMAT_DMY },
	{ "Dominican Republic", DATE_FORMAT_DMY },
	{ "East Timor", DATE_FORMAT_DMY },
	{ "Ecuador", DATE_FORMAT_DMY },
	{ "Egypt", DATE_FORMAT_DMY },
	{ "El Salvador", DATE_FORMAT_DMY },
	{ "Equatorial Guinea", DATE_FORMAT_DMY },
	{ "Eritrea", DATE_FORMAT_DMY },
	{ "Estonia", DATE_FORMAT_DMY },
	{ "Ethiopia", DATE_FORMAT_DMY },
	{ "Falkland Islands", DATE_FORMAT_DMY },
	{ "Faroe Islands", DATE_FORMAT_DMY },
	{ "Federated States of Micronesia", DATE_FORMAT_MDY },
	{ "Finland", DATE_FORMAT_DMY },
	{ "Fiji", DATE_FORMAT_DMY },
	{ "France", DATE_FORMAT_DMY },
	{ "French Guiana", DATE_FORMAT_DMY },
	{ "French Polynesia", DATE_FORMAT_DMY },
	{ "Gabon", DATE_FORMAT_DMY },
	{ "Gambia", DATE_FORMAT_DMY },
	{ "Georgia", DATE_FORMAT_DMY },
	{ "Germany", DATE_FORMAT_YMD },
	{ "Ghana", DATE_FORMAT_DMY },
	{ "Gibraltar", DATE_FORMAT_DMY },
	{ "Greece", DATE_FORMAT_DMY },
	{ "Greenland", DATE_FORMAT_MDY },
	{ "Grenada", DATE_FORMAT_DMY },
	{ "Guadeloupe", DATE_FORMAT_DMY },
	{ "Guam", DATE_FORMAT_MDY },
	{ "Guatemala", DATE_FORMAT_DMY },
	{ "Guernsey", DATE_FORMAT_DMY },
	{ "Guinea", DATE_FORMAT_DMY },
	{ "Guinea-Bissau", DATE_FORMAT_DMY },
	{ "Guyana", DATE_FORMAT_DMY },
	{ "Haiti", DATE_FORMAT_DMY },
	{ "Hong Kong", DATE_FORMAT_DMY },
	{ "Honduras", DATE_FORMAT_DMY },
	{ "Hungary", DATE_FORMAT_YMD },
	{ "Iceland", DATE_FORMAT_DMY },
	{ "India", DATE_FORMAT_DMY },
	{ "Indonesia", DATE_FORMAT_DMY },
	{ "Iran, Islamic Republic of", DATE_FORMAT_YMD },
	{ "Iraq", DATE_FORMAT_DMY },
	{ "Ireland", DATE_FORMAT_DMY },
	{ "Isle of Man", DATE_FORMAT_DMY },
	{ "Israel", DATE_FORMAT_DMY },
	{ "Italy", DATE_FORMAT_DMY },
	{ "Ivory Coast", DATE_FORMAT_DMY },
	{ "Jamaica", DATE_FORMAT_DMY },
	{ "Jan Mayen", DATE_FORMAT_DMY },
	{ "Japan", DATE_FORMAT_YMD },
	{ "Jersey", DATE_FORMAT_DMY },
	{ "Jordan", DATE_FORMAT_DMY },
	{ "Kazakhstan", DATE_FORMAT_DMY },
	{ "Kenya", DATE_FORMAT_DMY },
	{ "Kiribati", DATE_FORMAT_DMY },
	{ "North Korea", DATE_FORMAT_YMD },
	{ "South Korea", DATE_FORMAT_YMD },
	{ "Kosovo", DATE_FORMAT_DMY },
	{ "Kuwait", DATE_FORMAT_DMY },
	{ "Kyrgyz Republic", DATE_FORMAT_DMY },
	{ "Lao People's Democratic Republic", DATE_FORMAT_DMY },
	{ "Latvia", DATE_FORMAT_DMY },
	{ "Lebanon", DATE_FORMAT_DMY },
	{ "Lesotho", DATE_FORMAT_DMY },
	{ "Liberia", DATE_FORMAT_DMY },
	{ "Libya", DATE_FORMAT_DMY },
	{ "Liechtenstein", DATE_FORMAT_DMY },
	{ "Lithuania", DATE_FORMAT_YMD },
	{ "Luxembourg", DATE_FORMAT_DMY },
	{ "Macau", DATE_FORMAT_DMY },
	{ "Macedonia", DATE_FORMAT_DMY },
	{ "Madagascar", DATE_FORMAT_DMY },
	{ "Malawi", DATE_FORMAT_DMY },
	{ "Malaysia", DATE_FORMAT_DMY },
	{ "Maldives", DATE_FORMAT_YMD },
	{ "Mali", DATE_FORMAT_DMY },
	{ "Malta", DATE_FORMAT_DMY },
	{ "Marshall Islands", DATE_FORMAT_MDY },
	{ "Martinique", DATE_FORMAT_DMY },
	{ "Mauritania", DATE_FORMAT_DMY },
	{ "Mauritius", DATE_FORMAT_DMY },
	{ "Mayotte", DATE_FORMAT_DMY },
	{ "Mexico", DATE_FORMAT_DMY },
	{ "Moldova", DATE_FORMAT_DMY },
	{ "Monaco", DATE_FORMAT_DMY },
	{ "Mongolia", DATE_FORMAT_YMD },
	{ "Montenegro", DATE_FORMAT_DMY },
	{ "Montserrat", DATE_FORMAT_DMY },
	{ "Morocco", DATE_FORMAT_DMY },
	{ "Mozambique", DATE_FORMAT_DMY },
	{ "Myanmar", DATE_FORMAT_DMY },
	{ "Nagorno-Karabakh Republic", DATE_FORMAT_DMY },
	{ "Namibia", DATE_FORMAT_DMY },
	{ "Nauru", DATE_FORMAT_DMY },
	{ "Nepal", DATE_FORMAT_YMD },
	{ "Netherlands", DATE_FORMAT_DMY },
	{ "New Caledonia", DATE_FORMAT_DMY },
	{ "New Zealand", DATE_FORMAT_DMY },
	{ "Nicaragua", DATE_FORMAT_DMY },
	{ "Niger", DATE_FORMAT_DMY },
	{ "Nigeria", DATE_FORMAT_DMY },
	{ "Niue", DATE_FORMAT_DMY },
	{ "Norfolk Island", DATE_FORMAT_DMY },
	{ "Northern Mariana Islands", DATE_FORMAT_MDY },
	{ "Norway", DATE_FORMAT_DMY },
	{ "Oman", DATE_FORMAT_DMY },
	{ "Pakistan", DATE_FORMAT_DMY },
	{ "Palestine", DATE_FORMAT_DMY },
	{ "Palau", DATE_FORMAT_DMY },
	{ "Panama", DATE_FORMAT_MDY },
	{ "Papua New Guinea", DATE_FORMAT_DMY },
	{ "Paraguay", DATE_FORMAT_DMY },
	{ "Peru", DATE_FORMAT_DMY },
	{ "Philippines", DATE_FORMAT_MDY },
	{ "Pitcairn Islands", DATE_FORMAT_DMY },
	{ "Poland", DATE_FORMAT_DMY },
	{ "Portugal", DATE_FORMAT_DMY },
	{ "Puerto Rico", DATE_FORMAT_MDY },
	{ "Qatar", DATE_FORMAT_DMY },
	{ "R�union", DATE_FORMAT_DMY },
	{ "Romania", DATE_FORMAT_DMY },
	{ "Russian Federation", DATE_FORMAT_DMY },
	{ "Rwanda", DATE_FORMAT_DMY },
	{ "Saba", DATE_FORMAT_DMY },
	{ "Saint Barth�lemy", DATE_FORMAT_DMY },
	{ "Saint Helena", DATE_FORMAT_DMY },
	{ "Saint Kitts and Nevis", DATE_FORMAT_DMY },
	{ "Saint Lucia", DATE_FORMAT_DMY },
	{ "Saint Martin", DATE_FORMAT_DMY },
	{ "Saint Pierre and Miquelon", DATE_FORMAT_DMY },
	{ "Saint Vincent and the Grenadines", DATE_FORMAT_DMY },
	{ "Samoa", DATE_FORMAT_DMY },
	{ "S�o Tom� and Pr�ncipe", DATE_FORMAT_DMY },
	{ "Saudi Arabia", DATE_FORMAT_DMY },
	{ "Senegal", DATE_FORMAT_DMY },
	{ "Serbia", DATE_FORMAT_DMY },
	{ "Seychelles", DATE_FORMAT_DMY },
	{ "Sierra Leone", DATE_FORMAT_DMY },
	{ "Singapore", DATE_FORMAT_DMY },
	{ "Sint Eustatius", DATE_FORMAT_DMY },
	{ "Sint Maarten", DATE_FORMAT_DMY },
	{ "Slovakia", DATE_FORMAT_DMY },
	{ "Slovenia", DATE_FORMAT_DMY },
	{ "Solomon Islands", DATE_FORMAT_DMY },
	{ "Somalia", DATE_FORMAT_DMY },
	{ "South Africa", DATE_FORMAT_YMD },
	{ "Spain", DATE_FORMAT_DMY },
	{ "Sri Lanka", DATE_FORMAT_YMD },
	{ "Sudan", DATE_FORMAT_DMY },
	{ "Suriname", DATE_FORMAT_DMY },
	{ "Svalbard", DATE_FORMAT_DMY },
	{ "Swaziland", DATE_FORMAT_DMY },
	{ "Sweden", DATE_FORMAT_YMD },
	{ "Switzerland", DATE_FORMAT_DMY },
	{ "Syrian Arab Republic", DATE_FORMAT_DMY },
	{ "Taiwan", DATE_FORMAT_YMD },
	{ "Tajikistan", DATE_FORMAT_DMY },
	{ "Tanzania", DATE_FORMAT_DMY },
	{ "Thailand", DATE_FORMAT_DMY },
	{ "Togo", DATE_FORMAT_DMY },
	{ "Tokelau", DATE_FORMAT_DMY },
	{ "Tonga", DATE_FORMAT_DMY },
	{ "Trinidad and Tobago", DATE_FORMAT_DMY },
	{ "Tristan da Cunha", DATE_FORMAT_DMY },
	{ "Tunisia", DATE_FORMAT_DMY },
	{ "Turkey", DATE_FORMAT_DMY },
	{ "Turkmenistan", DATE_FORMAT_DMY },
	{ "Turks and Caicos Islands", DATE_FORMAT_DMY },
	{ "Tuvalu", DATE_FORMAT_DMY },
	{ "Uganda", DATE_FORMAT_DMY },
	{ "Ukraine", DATE_FORMAT_DMY },
	{ "United Arab Emirates", DATE_FORMAT_DMY },
	{ "United Kingdom", DATE_FORMAT_DMY },
	{ "United States Minor Outlying Islands", DATE_FORMAT_MDY },
	{ "United States of America", DATE_FORMAT_MDY },
	{ "United States Virgin Islands", DATE_FORMAT_MDY },
	{ "Uruguay", DATE_FORMAT_DMY },
	{ "Uzbekistan", DATE_FORMAT_DMY },
	{ "Vanuatu", DATE_FORMAT_DMY },
	{ "Venezuela", DATE_FORMAT_DMY },
	{ "Vietnam", DATE_FORMAT_DMY },
	{ "Wallis and Futuna", DATE_FORMAT_DMY },
	{ "Yemen", DATE_FORMAT_DMY },
	{ "Zambia", DATE_FORMAT_DMY },
	{ "Zimbabwe", DATE_FORMAT_DMY },
	{ "", DATE_FORMAT_UNSPECIFIED }
};


BOOL CCustomizePage::LoadCountrySelectionList()
{
	BOOL				bNoError = TRUE;
	BOOL				bEndOfList;
	BOOL				bCountryWasPreviouslySelected;
	COUNTRY_INFO		*pCountryInfo;
	int					nCountry;
	int					nItemIndex;
	int					nDefaultCountry;

	m_ComboBoxSelectCountry.ResetContent();
	m_ComboBoxSelectCountry.SetWindowTextA( "Country List" );

	nCountry = 0;
	nDefaultCountry = 0;
	bCountryWasPreviouslySelected = ( strlen( BViewerCustomization.m_CountryInfo.CountryName ) != 0 );
	bEndOfList = FALSE;
	do
		{
		pCountryInfo = &CountryInfoArray[ nCountry ];
		bEndOfList = ( strlen( pCountryInfo -> CountryName ) == 0 );
		if ( !bEndOfList )
			{
			nItemIndex = m_ComboBoxSelectCountry.AddString( pCountryInfo -> CountryName );
			if ( !bCountryWasPreviouslySelected )
				{
				if ( _stricmp( pCountryInfo -> CountryName, "United States of America" ) == 0 )
					nDefaultCountry = nItemIndex;
				}
			else
				{
				if ( _stricmp( pCountryInfo -> CountryName, BViewerCustomization.m_CountryInfo.CountryName ) == 0 )
					nDefaultCountry = nItemIndex;
				}
			m_ComboBoxSelectCountry.SetItemDataPtr( nItemIndex, (void*)pCountryInfo );
			}
		nCountry++;
		}
	while ( !bEndOfList );
	m_ComboBoxSelectCountry.SetCurSel( nDefaultCountry );

	return bNoError;
}


void CCustomizePage::OnCountrySelected()
{
	COUNTRY_INFO			*pCountryInfo;
	int						nItemIndex;

	nItemIndex = m_ComboBoxSelectCountry.GetCurSel();
	pCountryInfo = (COUNTRY_INFO*)m_ComboBoxSelectCountry.GetItemDataPtr( nItemIndex );
	memcpy( &BViewerCustomization.m_CountryInfo, pCountryInfo, sizeof(COUNTRY_INFO) );
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
			m_EditMonitor2GrayscaleBitDepth.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
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
			m_EditMonitor3GrayscaleBitDepth.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
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

		IntegerValue = (int)BViewerCustomization.m_PrimaryMonitorGrayScaleBitDepth;
		_ltoa( IntegerValue, NumberConvertedToText, 10 );
		m_EditPrimaryMonitorGrayscaleBitDepth.SetWindowText( NumberConvertedToText );

		IntegerValue = (int)BViewerCustomization.m_Monitor2GrayScaleBitDepth;
		_ltoa( IntegerValue, NumberConvertedToText, 10 );
		m_EditMonitor2GrayscaleBitDepth.SetWindowText( NumberConvertedToText );

		IntegerValue = (int)BViewerCustomization.m_Monitor3GrayScaleBitDepth;
		_ltoa( IntegerValue, NumberConvertedToText, 10 );
		m_EditMonitor3GrayscaleBitDepth.SetWindowText( NumberConvertedToText );

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

		if ( BViewerCustomization.m_ReaderInfo.bPasswordEntered )
			m_EditLoginPassword.SetWindowText( "************" );
		else
			m_EditLoginPassword.SetWindowText( "" );
		m_EditAE_Title.SetWindowText( BViewerCustomization.m_ReaderInfo.AE_TITLE );
		m_EditReaderStreetAddress.SetWindowText( BViewerCustomization.m_ReaderInfo.StreetAddress );
		m_EditReaderCity.SetWindowText( BViewerCustomization.m_ReaderInfo.City );
		m_EditReaderState.SetWindowText( BViewerCustomization.m_ReaderInfo.State );
		m_EditReaderZipCode.SetWindowText( BViewerCustomization.m_ReaderInfo.ZipCode );
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
	char					Msg[ 512 ];
	
	if ( !bMakeDumbButtons )
		{
		strcpy( Msg, "Note:  When you insert the I.L.O. reference standards \n" );
		strcat( Msg, "media, the I.L.O. viewer may try to start.\n" );
		strcat( Msg, "If Windows asks your permission to run it, answer \"No\".\n" );
		strcat( Msg, "If the viewer starts automatically, wait patiently for\n" );
		strcat( Msg, "it to come up, then exit out of it before continuing\n" );
		strcat( Msg, "with this reference image installation.\n" );
		strcat( Msg, "\n" );
		strcat( Msg, "If you like, you may insert the I.L.O. media now.\n" );
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
	
	if ( !bMakeDumbButtons )
		{
		strcpy( ProgramPath, BViewerConfiguration.ProgramPath );
		strcat( ProgramPath, "BRetriever\\" );
		strcpy( ServiceControllerExeFile, ProgramPath );
		strcat( ServiceControllerExeFile, "ServiceController.exe" );
		LogMessage( "Launching the service controller:", MESSAGE_TYPE_NORMAL_LOG );
		LogMessage( ServiceControllerExeFile, MESSAGE_TYPE_NORMAL_LOG );
		_spawnl( _P_NOWAIT, ServiceControllerExeFile, ServiceControllerExeFile, 0 );
		}

	*pResult = 0;
}


static void ProcessBRetrieverNetworkAddressResponse( void *pResponseDialog )
{
	CPopupDialog			*pPopupDialog;
	CString					UserResponseString;
	char					ProgramPath[ FULL_FILE_SPEC_STRING_LENGTH ];
	char					ServiceControllerExeFile[ FULL_FILE_SPEC_STRING_LENGTH ];
	
	if ( !bMakeDumbButtons )
		{
		pPopupDialog = (CPopupDialog*)pResponseDialog;
		if ( pPopupDialog -> m_pUserNotificationInfo -> UserResponse == POPUP_RESPONSE_SAVE )
			{
			pPopupDialog -> m_EditUserTextInput.GetWindowText( UserResponseString );
			strcpy( pPopupDialog -> m_pUserNotificationInfo -> UserTextResponse, (const char*)UserResponseString );
			strcpy( BViewerConfiguration.NetworkAddress, pPopupDialog -> m_pUserNotificationInfo -> UserTextResponse );
			// Write out updated configuration file.
			RewriteConfigurationFile( BViewerConfiguration.BRetrieverServiceDirectory, "Shared.cfg" );
			// Restart BRetriever.
			strcpy( ProgramPath, BViewerConfiguration.ProgramPath );
			strcat( ProgramPath, "BRetriever\\" );
			strcpy( ServiceControllerExeFile, ProgramPath );
			strcat( ServiceControllerExeFile, "ServiceController.exe" );
			LogMessage( "Launching the service controller for restart:", MESSAGE_TYPE_NORMAL_LOG );
			LogMessage( ServiceControllerExeFile, MESSAGE_TYPE_NORMAL_LOG );
			_spawnl( _P_NOWAIT, ServiceControllerExeFile, ServiceControllerExeFile, "/restart", 0 );
			}
		}

	delete pPopupDialog;
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
			strcpy( UserNotificationInfo.UserTextResponse, BViewerConfiguration.NetworkAddress );

			CWaitCursor			HourGlass;
		
			pMainFrame -> PerformUserInput( &UserNotificationInfo );
			}
		}

	*pResult = 0;
}


BOOL CCustomizePage::DirectoryExists( char *pFullDirectorySpecification )
{
	BOOL			bDirectoryExists;
	char			SavedCurrentDirectory[ FULL_FILE_SPEC_STRING_LENGTH ];

	GetCurrentDirectory( FULL_FILE_SPEC_STRING_LENGTH, SavedCurrentDirectory );
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
	char						FolderAnnotation[ 128 ];
	char						NewSearchDirectory[ FULL_FILE_SPEC_STRING_LENGTH ];
	char						FullFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];

	strcpy( FolderAnnotation, "" );
	strncat( FolderAnnotation, "                                                   ", FolderIndent );
	// Check existence of source path.
	bNoError = DirectoryExists( SearchDirectory );
	if ( bNoError )
		{
		sprintf( Msg, "    %sDeleting contents of %s:", FolderAnnotation, SearchDirectory );
		LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
		strcpy( SearchFileSpec, SearchDirectory );
		strcat( SearchFileSpec, "*.*" );
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
					strcpy( NewSearchDirectory, SearchDirectory );
					strcat( NewSearchDirectory, FindFileInfo.cFileName );
					strcat( NewSearchDirectory, "\\" );
					DeleteFolderContents( NewSearchDirectory, FolderIndent + 4 );
					RemoveDirectory( NewSearchDirectory );
					}
				else
					{
					sprintf( Msg, "        %sDeleting %s", FolderAnnotation, FindFileInfo.cFileName );
					LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );
					strcpy( FullFileSpec, SearchDirectory );
					strcat( FullFileSpec, FindFileInfo.cFileName );
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

	strcpy( SearchDirectory, BViewerConfiguration.ImageDirectory );
	if ( SearchDirectory[ strlen( SearchDirectory ) - 1 ] != '\\' )
		strcat( SearchDirectory, "\\" );
	DeleteFolderContents( SearchDirectory, 0 );

	strcpy( SearchDirectory, BViewerConfiguration.InboxDirectory );
	if ( SearchDirectory[ strlen( SearchDirectory ) - 1 ] != '\\' )
		strcat( SearchDirectory, "\\" );
	DeleteFolderContents( SearchDirectory, 0 );

	strcpy( SearchDirectory, BViewerConfiguration.WatchDirectory );
	if ( SearchDirectory[ strlen( SearchDirectory ) - 1 ] != '\\' )
		strcat( SearchDirectory, "\\" );
	DeleteFolderContents( SearchDirectory, 0 );

	strcpy( SearchDirectory, BViewerConfiguration.BRetrieverDataDirectory );
	if ( SearchDirectory[ strlen( SearchDirectory ) - 1 ] != '\\' )
		strcat( SearchDirectory, "\\" );
	strcat( SearchDirectory, "Queued Files\\" );
	DeleteFolderContents( SearchDirectory, 0 );

	strcpy( SearchDirectory, BViewerConfiguration.BRetrieverDataDirectory );
	if ( SearchDirectory[ strlen( SearchDirectory ) - 1 ] != '\\' )
		strcat( SearchDirectory, "\\" );
	strcat( SearchDirectory, "Errored Files\\" );
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
			BViewerCustomization.m_PrimaryMonitorWidthInMM = IntegerValue;
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
			BViewerCustomization.m_Monitor2WidthInMM = IntegerValue;
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
			BViewerCustomization.m_Monitor3WidthInMM = IntegerValue;
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
			BViewerCustomization.m_PrimaryMonitorHeightInMM = IntegerValue;
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
			BViewerCustomization.m_Monitor2HeightInMM = IntegerValue;
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
			BViewerCustomization.m_Monitor3HeightInMM = IntegerValue;
			m_EditMonitor3Height.Invalidate( TRUE );
			UpdateDisplaySettings();
			}
		}

	*pResult = 0;
}


void CCustomizePage::OnEditPrimaryMonitorBitDepthKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	long				IntegerValue;
	char				NumberConvertedToText[ _CVTBUFSIZE ];

	if ( !bMakeDumbButtons )
		{
		if ( m_EditPrimaryMonitorGrayscaleBitDepth.m_IdleBkgColor != m_EditPrimaryMonitorGrayscaleBitDepth.m_SpecialBkgColor )
			{
			m_EditPrimaryMonitorGrayscaleBitDepth.GetWindowText( NumberConvertedToText, _CVTBUFSIZE );
			IntegerValue = atol( NumberConvertedToText );
			BViewerCustomization.m_PrimaryMonitorGrayScaleBitDepth = (unsigned short)IntegerValue;
			m_EditPrimaryMonitorGrayscaleBitDepth.Invalidate( TRUE );
			UpdateDisplaySettings();
			}
		}

	*pResult = 0;
}


void CCustomizePage::OnEditMonitor2BitDepthKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	long				IntegerValue;
	char				NumberConvertedToText[ _CVTBUFSIZE ];

	if ( !bMakeDumbButtons )
		{
		if ( m_EditMonitor2GrayscaleBitDepth.m_IdleBkgColor != m_EditMonitor2GrayscaleBitDepth.m_SpecialBkgColor )
			{
			m_EditMonitor2GrayscaleBitDepth.GetWindowText( NumberConvertedToText, _CVTBUFSIZE );
			IntegerValue = atol( NumberConvertedToText );
			BViewerCustomization.m_Monitor2GrayScaleBitDepth = (unsigned short)IntegerValue;
			m_EditMonitor2GrayscaleBitDepth.Invalidate( TRUE );
			UpdateDisplaySettings();
			}
		}

	*pResult = 0;
}


void CCustomizePage::OnEditMonitor3BitDepthKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	long				IntegerValue;
	char				NumberConvertedToText[ _CVTBUFSIZE ];

	if ( !bMakeDumbButtons )
		{
		if ( m_EditMonitor3GrayscaleBitDepth.m_IdleBkgColor != m_EditMonitor3GrayscaleBitDepth.m_SpecialBkgColor )
			{
			m_EditMonitor3GrayscaleBitDepth.GetWindowText( NumberConvertedToText, _CVTBUFSIZE );
			IntegerValue = atol( NumberConvertedToText );
			BViewerCustomization.m_Monitor3GrayScaleBitDepth = (unsigned short)IntegerValue;
			m_EditMonitor3GrayscaleBitDepth.Invalidate( TRUE );
			UpdateDisplaySettings();
			}
		}

	*pResult = 0;
}


void CCustomizePage::OnEditReaderLastNameKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	char				TextString[ 64 ];

	if ( m_EditReaderLastName.m_IdleBkgColor != m_EditReaderLastName.m_SpecialBkgColor )
		{
		m_EditReaderLastName.GetWindowText( TextString, 64 );
		strcpy( BViewerCustomization.m_ReaderInfo.LastName, TextString );
		m_EditReaderLastName.Invalidate( TRUE );
		}

	*pResult = 0;
}


void CCustomizePage::OnEditLoginNameKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	char				TextString[ 64 ];

	if ( !bMakeDumbButtons )
		{
		if ( m_EditLoginName.m_IdleBkgColor != m_EditLoginName.m_SpecialBkgColor )
			{
			m_EditLoginName.GetWindowText( TextString, 64 );
			strcpy( BViewerCustomization.m_ReaderInfo.LoginName, TextString );
			BViewerCustomization.m_ReaderInfo.bLoginNameEntered = ( strlen( TextString ) > 0 );
			m_EditLoginName.Invalidate( TRUE );
			}
		}

	*pResult = 0;
}


void CCustomizePage::OnEditReaderIDKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	char				TextString[ 12 ];

	if ( !bMakeDumbButtons && BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_NIOSH )
		{
		if ( m_EditReaderID.m_IdleBkgColor != m_EditReaderID.m_SpecialBkgColor )
			{
			m_EditReaderID.GetWindowText( TextString, 12 );
			strcpy( BViewerCustomization.m_ReaderInfo.ID, TextString );
			m_EditReaderID.Invalidate( TRUE );
			}
		}

	*pResult = 0;
}


void CCustomizePage::OnEditLoginPasswordKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	char							TextString[ 64 ];
 	CMainFrame						*pMainFrame;
	static USER_NOTIFICATION_INFO	UserNotificationInfo;
	static BOOL						bNotificationDisplayed = FALSE;

	if ( !bMakeDumbButtons )
		{
		if ( m_EditLoginPassword.m_IdleBkgColor != m_EditLoginPassword.m_SpecialBkgColor )
			{
			m_EditLoginPassword.GetWindowText( TextString, MAX_USER_INFO_LENGTH );
			// Don't allow the password to contain the * character, since it is used as the
			// mask and may be entered by mistake if not intentionally deleted.
			if ( strchr( TextString, '*' ) == NULL )
				{
				SaveAccessCode( &BViewerCustomization.m_ReaderInfo, TextString );
				BViewerCustomization.m_ReaderInfo.bPasswordEntered = ( strlen( TextString ) > 0 );
				}
			else if ( !bNotificationDisplayed )
				{
				pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
				if ( pMainFrame != 0 )
					{
					UserNotificationInfo.WindowWidth = 500;
					UserNotificationInfo.WindowHeight = 400;
					UserNotificationInfo.FontHeight = 0;	// Use default setting;
					UserNotificationInfo.FontWidth = 0;		// Use default setting;
					UserNotificationInfo.UserInputType = USER_INPUT_TYPE_OK;
					UserNotificationInfo.pUserNotificationMessage = "The password must not\ncontain the * character";
					UserNotificationInfo.CallbackFunction = FinishReaderInfoResponse;
					pMainFrame -> PerformUserInput( &UserNotificationInfo );
					}
				}
			m_EditLoginPassword.Invalidate( TRUE );
			bNotificationDisplayed = !bNotificationDisplayed;
			}
		}

	*pResult = 0;
}


void CCustomizePage::OnEditReaderInitialsKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	char				TextString[ 4 ];

	if ( BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_GENERAL ||
				BViewerConfiguration.InterpretationEnvironment == INTERP_ENVIRONMENT_NIOSH )
	if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_GENERAL )
		{
		if ( m_EditReaderInitials.m_IdleBkgColor != m_EditReaderInitials.m_SpecialBkgColor )
			{
			m_EditReaderInitials.GetWindowText( TextString, 4 );
			strcpy( BViewerCustomization.m_ReaderInfo.Initials, TextString );
			m_EditReaderInitials.Invalidate( TRUE );
			}
		}

	*pResult = 0;
}


void CCustomizePage::OnEditAE_TitleKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	char				TextString[ 20 ];

	if ( !bMakeDumbButtons )
		{
		if ( m_EditAE_Title.m_IdleBkgColor != m_EditAE_Title.m_SpecialBkgColor )
			{
			m_EditAE_Title.GetWindowText( TextString, 16 );
			strcpy( BViewerCustomization.m_ReaderInfo.AE_TITLE, TextString );
			m_EditAE_Title.Invalidate( TRUE );
			}
		}

	*pResult = 0;
}


void CCustomizePage::OnEditReaderReportSignatureNameKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	char				TextString[ 64 ];

	if ( m_EditReaderReportSignatureName.m_IdleBkgColor != m_EditReaderReportSignatureName.m_SpecialBkgColor )
		{
		m_EditReaderReportSignatureName.GetWindowText( TextString, 64 );
		strcpy( BViewerCustomization.m_ReaderInfo.ReportSignatureName, TextString );
		m_EditReaderReportSignatureName.Invalidate( TRUE );
		}

	*pResult = 0;
}


void CCustomizePage::OnEditReaderStreetAddressKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	char				TextString[ 64 ];

	if ( !bMakeDumbButtons )
		{
		if ( m_EditReaderStreetAddress.m_IdleBkgColor != m_EditReaderStreetAddress.m_SpecialBkgColor )
			{
			m_EditReaderStreetAddress.GetWindowText( TextString, 64 );
			strcpy( BViewerCustomization.m_ReaderInfo.StreetAddress, TextString );
			m_EditReaderStreetAddress.Invalidate( TRUE );
			}
		}

	*pResult = 0;
}


void CCustomizePage::OnEditReaderCityKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	char				TextString[ 32 ];

	if ( !bMakeDumbButtons )
		{
		if ( m_EditReaderCity.m_IdleBkgColor != m_EditReaderCity.m_SpecialBkgColor )
			{
			m_EditReaderCity.GetWindowText( TextString, 32 );
			strcpy( BViewerCustomization.m_ReaderInfo.City, TextString );
			m_EditReaderCity.Invalidate( TRUE );
			}
		}

	*pResult = 0;
}


void CCustomizePage::OnEditReaderStateKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	char				TextString[ 4 ];

	if ( !bMakeDumbButtons )
		{
		if ( m_EditReaderState.m_IdleBkgColor != m_EditReaderState.m_SpecialBkgColor )
			{
			m_EditReaderState.GetWindowText( TextString, 4 );
			strcpy( BViewerCustomization.m_ReaderInfo.State, TextString );
			m_EditReaderState.Invalidate( TRUE );
			}
		}

	*pResult = 0;
}


void CCustomizePage::OnEditReaderZipCodeKillFocus( NMHDR *pNMHDR, LRESULT *pResult )
{
	char				TextString[ 12 ];

	if ( !bMakeDumbButtons )
		{
		if ( m_EditReaderZipCode.m_IdleBkgColor != m_EditReaderZipCode.m_SpecialBkgColor )
			{
			m_EditReaderZipCode.GetWindowText( TextString, 12 );
			strcpy( BViewerCustomization.m_ReaderInfo.ZipCode, TextString );
			m_EditReaderZipCode.Invalidate( TRUE );
			}
		}

	*pResult = 0;
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
		// Copy and updates to the user list for the current user.
		if ( pCurrentReaderInfo != 0 )
			memcpy( (void*)pCurrentReaderInfo, (void*)&BViewerCustomization.m_ReaderInfo, sizeof(READER_PERSONAL_INFO) );

		bFileWrittenSuccessfully = FALSE;
		FileSizeInBytes = sizeof( CCustomization );
		strcpy( ConfigurationDirectory, "" );
		strncat( ConfigurationDirectory, BViewerConfiguration.ConfigDirectory, FILE_PATH_STRING_LENGTH );
		if ( ConfigurationDirectory[ strlen( ConfigurationDirectory ) - 1 ] != '\\' )
			strcat( ConfigurationDirectory, "\\" );
		// Check existence of path to configuration directory.
		bNoError = SetCurrentDirectory( ConfigurationDirectory );
		if ( bNoError )
			{
			strcpy( FileSpec, ConfigurationDirectory );
			strcat( FileSpec, "CriticalData2.sav" );
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

	// Loop through all the data entry fields to be sure that their current
	// data values have been captured.
	nItem = 0;
	do
		{
		pEditTextDestination = &EditIDArray[ nItem ];
		EditID = pEditTextDestination -> EditControlID;
		if ( EditID != 0 )
			{
			pEditControl = (TomEdit*)GetDlgItem( EditID );
			if ( pEditControl != 0 )
				{
				pEditControl -> GetWindowText( TextString, 64 );
				pTextBuffer = (char*)&BViewerCustomization.m_ReaderInfo + pEditTextDestination -> DataStructureOffset;
				strcpy( pTextBuffer, "" );
				strncat( pTextBuffer, TextString, pEditTextDestination -> BufferSize - 1 );
				}
			}
		nItem++;
		}
	while ( EditID != 0 );

	memcpy( &LoggedInReaderInfo, &BViewerCustomization.m_ReaderInfo, sizeof( READER_PERSONAL_INFO ) );
	WriteBViewerConfiguration();
	BViewerCustomization.WriteUserList();
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
		pTechnicalRequirementsBox -> ReadTextFileForDisplay( BViewerConfiguration.BViewerTechnicalRequirementsFile );
		pTechnicalRequirementsBox -> BringWindowToTop();
		pTechnicalRequirementsBox -> SetFocus();
		}

	*pResult = 0;
}


void CCustomizePage::OnBnClickedAddUser( NMHDR *pNMHDR, LRESULT *pResult )
{
	CReaderInfoScreen		*pReaderInfoScreen;
	READER_PERSONAL_INFO	*pNewReaderInfo;
	BOOL					bCancel;

	if ( !bMakeDumbButtons )
		{
		pReaderInfoScreen = new( CReaderInfoScreen );
		if ( pReaderInfoScreen != 0 )
			{
			bCancel = !( pReaderInfoScreen -> DoModal() == IDOK );
			if ( !bCancel )
				{
				pNewReaderInfo = (READER_PERSONAL_INFO*)malloc( sizeof(READER_PERSONAL_INFO) );
				if ( pNewReaderInfo != 0 )
					{
					memcpy( pNewReaderInfo, &BViewerCustomization.m_ReaderInfo, sizeof(READER_PERSONAL_INFO) );
					AppendToList( &RegisteredUserList, (void*)pNewReaderInfo );
					}
				BViewerCustomization.WriteUserList();
				}
			delete pReaderInfoScreen;
			}
		}

	Invalidate( TRUE );

	*pResult = 0;
}


static void ProcessEditUserResponse( void *pResponseDialog )
{
	CPopupDialog			*pPopupDialog;
	
	pPopupDialog = (CPopupDialog*)pResponseDialog;
	delete pPopupDialog;
}


void CCustomizePage::OnBnClickedEditUser( NMHDR *pNMHDR, LRESULT *pResult )
{
 	CMainFrame						*pMainFrame;
	static USER_NOTIFICATION_INFO	UserNotificationInfo;

	if ( !bMakeDumbButtons )
		{
		// Request user to enter reader information.
		pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
		if ( pMainFrame != 0 )
			{
			UserNotificationInfo.WindowWidth = 400;
			UserNotificationInfo.WindowHeight = 300;
			UserNotificationInfo.FontHeight = 18;	// Use default setting;
			UserNotificationInfo.FontWidth = 9;		// Use default setting;
			UserNotificationInfo.UserInputType = USER_INPUT_TYPE_OK;
			UserNotificationInfo.pUserNotificationMessage = "To edit the data for an existing\nuser, log in as that user and\nedit your changes on the\n\"Set Up BViewer\" tab.";
			UserNotificationInfo.CallbackFunction = ProcessEditUserResponse;
			pMainFrame -> PerformUserInput( &UserNotificationInfo );
			}
		}

	*pResult = 0;
}


void CCustomizePage::OnBnClickedBeginNewTestSession( NMHDR *pNMHDR, LRESULT *pResult)
{
	EraseList( &RegisteredUserList );
	memset( &BViewerCustomization.m_ReaderInfo, '\0', sizeof( READER_PERSONAL_INFO ) );
	ClearReaderInfoDisplay();
	WriteBViewerConfiguration();
	BViewerCustomization.WriteUserList();
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
