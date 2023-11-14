// BViewer.h : Main header file for the BViewer application.
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
//	*[1] 01/13/2023 by Tom Atwood
//		Fixed code security issues.
//
//
#pragma once

// Detect memory leaks while debugging.
#define _CRTDBG_MAP_ALLOC 

#define UNICODE

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // resource symbols
#include "Module.h"
#include "Study.h"
#include "PopupDialog.h"
#include "LoginScreen.h"

// Define colors used by the program.
//
#define COLOR_STANDARD				0x00885533
#define COLOR_STD_SELECTOR			0x00996644

#define COLOR_PATIENT				0x00120033
#define COLOR_PATIENT_SELECTOR		0x00140055
#define COLOR_PATIENT_OPTIONAL		0X00334070

#define COLOR_REPORT_FONT			0x00ffffff
#define COLOR_REPORT				0x00ffffff
#define COLOR_REPORT_BKGD			0x00333333
#define COLOR_REPORT_SELECTOR_BKGD	0x00444444
#define COLOR_REPORT_HEADER			0x00dddddd
#define COLOR_REPORT_SELECTOR		0x00cccccc

#define COLOR_BLACK					0x00000000
#define COLOR_WHITE					0x00ffffff
#define COLOR_GREEN					0x0000ff00
#define COLOR_RED					0x000000ff
#define COLOR_DARK_GREEN			0x00009900
#define COLOR_DAARK_GREEN			0x00005500

#define COLOR_ANALYSIS_FONT			0x00ffffff
#define COLOR_ANALYSIS_BKGD			0x00333333

#define COLOR_PANEL_FONT			0x00ffffff
#define COLOR_PANEL_BKGD			0x00333333

#define COLOR_UNTOUCHED				0x006666aa
#define COLOR_TOUCHED				0x00444444
#define COLOR_COMPLETED				0x0066aa66

#define COLOR_UNTOUCHED_LIGHT		0x006666aa
#define COLOR_COMPLETED_LIGHT		0x0066aa66

#define COLOR_CANCEL				0x000000ff
#define COLOR_PROCEED				0x0033ccff

#define COLOR_CONFIG				0x00ccffff
#define COLOR_CONFIG_SELECTOR		0x00bbeeee

#define COLOR_LOG_FONT				0x0099ffff
#define COLOR_LOG_BKGD				0x00102210

// Define a windows message for signaling the arrival of an auto-load image.
#define        WM_AUTOLOAD			(WM_USER + 0)
// Define a windows message for initiating the automatic processing of an image.
#define        WM_AUTOPROCESS		(WM_USER + 1)
// Define a windows message for initiating the unattended production of a report for the current study.
// #define        WM_AUTOREPORT		(WM_USER + 2)


// CBViewerApp:
//
class CBViewerApp : public CWinApp
{
public:
	CBViewerApp();

	CString					m_MainWindowClassName;
	LIST_HEAD				m_ActiveStudyList;
	LIST_HEAD				m_AvailableStudyList;
	LIST_HEAD				m_NewlyArrivedStudyList;
	CStudy					*m_pCurrentStudy;
	char					m_AutoLoadSOPInstanceUID[ DICOM_ATTRIBUTE_UI_STRING_LENGTH ];
	HICON					m_hApplicationIcon;
	BOOL					m_bNewAbstractsAreAvailable;
	BOOL					m_bAutoViewStudyReceived;
	BOOL					m_bAutoViewStudyInProgress;
	int						m_nNewStudiesImported;
	unsigned long			m_nSecondsSinceHearingFromBRetriever;
	unsigned long			m_BRetrieverStatus;
								#define BRETRIEVER_STATUS_STOPPED			1
								#define BRETRIEVER_STATUS_ACTIVE			2
								#define BRETRIEVER_STATUS_PROCESSING		4
	unsigned long			m_NumberOfSpawnedThreads;

// Overrides
public:
	virtual BOOL				InitInstance();
	virtual int					ExitInstance();

// Implementation
	BOOL						ReadBViewerConfiguration();
	BOOL						SetUpAvailableStudies();
	void						EnableNewStudyPosting();
	void						MakeAnnouncement( char *pMsg );
	void						NotifyUserToAcknowledgeContinuation( char *pNoticeText );
	void						NotifyUserOfImageFileError( unsigned int ErrorCode, char *pNoticeText, char *pSuggestionText );
	void						NotifyUserOfImportSearchStatus( unsigned int ErrorCode, char *pNoticeText, char *pSuggestionText );
	void						NotifyUserOfInstallSearchStatus( unsigned int ErrorCode, char *pNoticeText, char *pSuggestionText );
	void						ReadNewAbstractData();
	BOOL						WarnUserOfDataResetConsequences();
	void						LaunchStudyUpdateTimer();
	void						UpdateBRetrieverStatusDisplay();
	void						DeallocateListOfStudies();
	void						DeleteUserNoticeList();			// *[1]
	void						TerminateTimers();
	void						EraseUserList();

//	DECLARE_MESSAGE_MAP()
public:
	afx_msg void				OnAppExit();
};

	void						CheckWindowsMessages();
	BOOL						SuccessfulLogin();

extern CBViewerApp			ThisBViewerApp;


