// Mainfrm.cpp : Implementation file for the CMainFrame class, which
//  serves as the frame window for the tabbed control panel.
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
#include <process.h>
#include "BViewer.h"
#include "Module.h"
#include "ReportStatus.h"
#include "DiagnosticImage.h"
#include "Mouse.h"
#include "ImageView.h"
#include "MainFrm.h"
#include "ImageFrame.h"
#include "PopupDialog.h"


CCustomizePage				*pCustomizePage;
BOOL						bTheLastKeyPressedWasESC = FALSE;


extern CBViewerApp			ThisBViewerApp;
extern CCustomization		*pBViewerCustomization;
extern CONFIGURATION		BViewerConfiguration;
extern BOOL					bABatchStudyIsBeingProcessed;



extern CString			ChildFrameWindowClass;
extern CString			PopupWindowClass;
extern CString			ExplorerWindowClass;

// CMainFrame
BEGIN_MESSAGE_MAP( CMainFrame, CFrameWnd )
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_DELETE_IMAGES, OnDeleteCheckedImages )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_IMPORT_IMAGES, OnImportLocalImages )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_SHOW_NEW_IMAGES, OnUpdateImageList )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_ENTER_MANUAL_STUDY, OnCreateAManualStudy )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_SHOW_LOG_DETAILS, OnShowLogDetail )
	ON_NOTIFY( WM_LBUTTONUP, ID_APP_EXIT, OnAppExit )
	ON_WM_CLOSE()
	ON_WM_CTLCOLOR()
	ON_WM_CHAR()
	ON_MESSAGE( WM_AUTOLOAD, OnAutoload )
	ON_MESSAGE( WM_AUTOPROCESS, OnAutoProcess )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CMainFrame::CMainFrame()
{
	m_DisplayMonitorCount = 1;
	m_pGraphicsAdapterList = 0;
	m_pDisplayMonitorInfoList = 0;
	m_pPrimaryDisplayMonitorInfo = 0;
	m_bProcessingNewImages = FALSE;
	m_bOKToRenderImage = TRUE;
	m_pControlPanel = new CControlPanel( "BViewer", this, 0 );
	if ( m_pControlPanel != 0 )
		pCustomizePage = &m_pControlPanel -> m_CustomizePage;
	else
		pCustomizePage = 0;
	m_BkgdBrush.CreateSolidBrush( COLOR_REPORT_HEADER );
	m_bSplashScreenIsUp = FALSE;
	m_pSplashWnd = 0;
	m_pImageFrame[ 0 ] = 0;
	m_pImageFrame[ 1 ] = 0;
	m_pImageFrame[ 2 ] = 0;
}


CMainFrame::~CMainFrame()
{
}


// An MFC application is terminated by sending a WM_CLOSE message to the
// main window of the application.
void CMainFrame::OnClose()
{
	int						nImageFrame;
	MONITOR_INFO			*pDisplayMonitorInfo;
	MONITOR_INFO			*pPrevDisplayMonitorInfo;
	CGraphicsAdapter		*pGraphicsAdapter;
	CGraphicsAdapter		*pPrevGraphicsAdapter;

	if ( m_pSplashWnd != 0 )
		{
		delete m_pSplashWnd;
		m_pSplashWnd = 0;
		}

	if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
		{
		for ( nImageFrame = 0; nImageFrame < MAX_VIEW_COUNT; nImageFrame++ )
			{
			if ( m_pImageFrame[ nImageFrame ] != 0 )
				{
				if ( m_pImageFrame[ nImageFrame ] -> m_pAssignedDiagnosticImage != 0 )
					{
					delete m_pImageFrame[ nImageFrame ] -> m_pAssignedDiagnosticImage;
					m_pImageFrame[ nImageFrame ] -> m_pAssignedDiagnosticImage = 0;
					}
//				m_pImageFrame[ nImageFrame ] -> OnClose();
				delete m_pImageFrame[ nImageFrame ];
				m_pImageFrame[ nImageFrame ] = 0;
				}
			}
		}
	else
		{
		if ( m_pImageFrame[ IMAGE_FRAME_STANDARD ] -> m_pAssignedDiagnosticImage != 0 )
			{
			delete m_pImageFrame[ IMAGE_FRAME_STANDARD ] -> m_pAssignedDiagnosticImage;
			m_pImageFrame[ IMAGE_FRAME_STANDARD ] -> m_pAssignedDiagnosticImage = 0;
			}
		m_pImageFrame[ IMAGE_FRAME_STANDARD ] -> OnClose();
		delete m_pImageFrame[ IMAGE_FRAME_STANDARD ];
		}

	pDisplayMonitorInfo = m_pDisplayMonitorInfoList;
	while ( pDisplayMonitorInfo != 0 )
		{
		pPrevDisplayMonitorInfo = pDisplayMonitorInfo;
		pDisplayMonitorInfo = pDisplayMonitorInfo -> pNextMonitor;
		free( pPrevDisplayMonitorInfo );
		}
	m_pDisplayMonitorInfoList = 0;

	if ( m_pControlPanel != 0 )
		{
		switch ( m_pControlPanel -> m_CurrentlyActivePage )
			{
			case STUDY_SELECTION_PAGE:
				if ( m_pControlPanel -> m_SelectStudyPage.m_hWnd != 0 )
					m_pControlPanel -> m_SelectStudyPage.OnKillActive();
				break;
			case INTERPRETATION_PAGE:
				if ( m_pControlPanel -> m_PerformAnalysisPage.m_hWnd != 0 )
					m_pControlPanel -> m_PerformAnalysisPage.OnKillActive();
				break;
			case REPORT_PAGE:
				if ( m_pControlPanel -> m_ComposeReportPage.m_hWnd != 0 )
					m_pControlPanel -> m_ComposeReportPage.OnKillActive();
				break;
			case LOG_PAGE:
				break;
			case SETUP_PAGE:
				if ( m_pControlPanel -> m_CustomizePage.m_hWnd != 0 )
					m_pControlPanel -> m_CustomizePage.OnKillActive();
				break;
			case USER_MANUAL_PAGE:
				break;
			}
		delete m_pControlPanel;
		}
	m_pControlPanel = 0;
	
	if ( m_pSelectStandardDlg != 0 && m_pSelectStandardDlg -> m_GroupSelectStdButtons.m_pMemberPointerArray != 0 )
		{
		free( m_pSelectStandardDlg -> m_GroupSelectStdButtons.m_pMemberPointerArray );
		m_pSelectStandardDlg -> m_GroupSelectStdButtons.m_pMemberPointerArray = 0;
		}
	if ( m_pSelectStandardDlg != 0 )
		delete m_pSelectStandardDlg;
	m_pSelectStandardDlg = 0;

	pGraphicsAdapter = m_pGraphicsAdapterList;
	while ( pGraphicsAdapter != 0 )
		{
		pPrevGraphicsAdapter = pGraphicsAdapter;
		pGraphicsAdapter = pGraphicsAdapter -> m_pNextGraphicsAdapter;
		free( pPrevGraphicsAdapter );
		}
	m_pGraphicsAdapterList = 0;
	
	ThisBViewerApp.EraseUserList();

	LogMessage( "BViewer is closing.", MESSAGE_TYPE_NORMAL_LOG );

	CFrameWnd::OnClose();
}


BOOL CMainFrame::PreTranslateMessage( MSG *pMsg )
{
	BOOL			bMsgFound = FALSE;
	

	if ( pMsg->message == WM_KEYDOWN  )
		if ( pMsg->wParam == VK_ESCAPE )
			bMsgFound = TRUE;

	return CFrameWnd::PreTranslateMessage( pMsg );
}


CGraphicsAdapter *CMainFrame::CatalogDisplayAdapter( char *pDisplayAdapterName )
{
	CGraphicsAdapter	*pNewGraphicsAdapter;
	CGraphicsAdapter	*pAdapter;
	BOOL				bDisplayAdapterAlreadySeen;

	bDisplayAdapterAlreadySeen = FALSE;
	pAdapter = m_pGraphicsAdapterList;
	while ( pAdapter != 0 && !bDisplayAdapterAlreadySeen )
		{
		// Has this display adapter already been cataloged?
		if ( strcmp( pAdapter -> m_DisplayAdapterName, pDisplayAdapterName ) == 0 )
			{
			// If so, return a pointer to it.
			bDisplayAdapterAlreadySeen = TRUE;
			}
		else
			pAdapter = pAdapter -> m_pNextGraphicsAdapter;
		}
	if ( !bDisplayAdapterAlreadySeen )
		{
		// Allocate a structure for the newly found graphics adapter.
		pNewGraphicsAdapter = new CGraphicsAdapter();
		if ( pNewGraphicsAdapter != 0 )
			{
			pNewGraphicsAdapter -> m_DisplayMonitorCount = 0;
			pNewGraphicsAdapter -> m_pDisplayMonitorInfoList = 0;
			strcpy( pNewGraphicsAdapter -> m_DisplayAdapterName, pDisplayAdapterName );
			// Link it to the list.
			if ( m_pGraphicsAdapterList == 0 )
				m_pGraphicsAdapterList = pNewGraphicsAdapter;
			else
				{
				pAdapter = m_pGraphicsAdapterList;
				while ( pAdapter != 0 && pAdapter -> m_pNextGraphicsAdapter != 0 )
					pAdapter = pAdapter -> m_pNextGraphicsAdapter;
				pAdapter -> m_pNextGraphicsAdapter = pNewGraphicsAdapter;
				}
			pNewGraphicsAdapter -> m_pNextGraphicsAdapter = 0;
			pAdapter = pNewGraphicsAdapter;
			}
		}

	return pAdapter;
}


// Iterate through the one or more display adapters installed on this computer.  The
// graphics adapter device string is stored in a CGraphicsAdapter instance and linked
// to the m_pGraphicsAdapterList.
//
// Catalog each display device found on each adapter.
//
void CMainFrame::SurveyGraphicsAdapters()
{
	CGraphicsAdapter	*pAdapter;
	DEVMODE				DisplayDeviceMode;
	MONITOR_INFO		*pDisplayMonitorInfo;
	MONITOR_INFO		*pNewDisplayMonitorInfo;
	char				GraphicsAdapterDescription[ 128 ];
	BOOL				bDisplayAdapterFound;
	BOOL				bDisplayIsPartOfDesktop;
	BOOL				bDisplayIsPrimaryDevice;
	DWORD				nAdapterDevice;
	DISPLAY_DEVICE		DisplayAdapterInformation;
	MONITOR_INFO		*pLastLinkedGlobalMonitorInfo;
	MONITOR_INFO		*pNewGlobalMonitorInfo;
	char				Msg[ 256 ];

	bDisplayAdapterFound = TRUE;
	nAdapterDevice = 0;
	m_DisplayMonitorCount = 0;
	DisplayAdapterInformation.cb = (DWORD)sizeof( DISPLAY_DEVICE );
	while ( bDisplayAdapterFound )
		{
		bDisplayIsPartOfDesktop = FALSE;
		bDisplayIsPrimaryDevice = FALSE;

		// This function is called for successive devices by incrementing nAdapterDevice and querying for another device.
		// The first argument is set to null when getting information on graphics adapters.
		bDisplayAdapterFound = EnumDisplayDevices( NULL, nAdapterDevice, &DisplayAdapterInformation, 0 );
		if ( bDisplayAdapterFound && DisplayAdapterInformation.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP )
			{
			bDisplayIsPartOfDesktop = TRUE;
			sprintf( GraphicsAdapterDescription, "%s", DisplayAdapterInformation.DeviceString );
			sprintf( Msg, "Using display monitor %s on display adapter %s", DisplayAdapterInformation.DeviceName, GraphicsAdapterDescription );
			if ( DisplayAdapterInformation.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE )
				{
				bDisplayIsPrimaryDevice = TRUE;
				strcat( Msg, ":  Primary display device." );
				}
			LogMessage( Msg, MESSAGE_TYPE_SUPPLEMENTARY );

			pAdapter = CatalogDisplayAdapter( GraphicsAdapterDescription );
			if ( pAdapter != 0 )
				{
				// Whether this is a different adapter or not, the display is definitely different.
				// Catalog the new display.
				memset( (void*)&DisplayDeviceMode, 0, sizeof(DEVMODE) );
				DisplayDeviceMode.dmSize = sizeof(DEVMODE);
				EnumDisplaySettings( DisplayAdapterInformation.DeviceName, ENUM_CURRENT_SETTINGS, &DisplayDeviceMode );

				pNewDisplayMonitorInfo = (MONITOR_INFO*)malloc( sizeof(MONITOR_INFO) );
				if ( pNewDisplayMonitorInfo != 0 )
					{
					pAdapter -> m_DisplayMonitorCount++;
					// Go to the end of the list of display monitors for this graphics adapter and append the information for the new one.
					pDisplayMonitorInfo = pAdapter -> m_pDisplayMonitorInfoList;
					while ( pDisplayMonitorInfo != 0 && pDisplayMonitorInfo -> pNextMonitor != 0 )
						pDisplayMonitorInfo = pDisplayMonitorInfo -> pNextMonitor;
					if ( pDisplayMonitorInfo != 0 )
						pDisplayMonitorInfo -> pNextMonitor = pNewDisplayMonitorInfo;
					else
						pAdapter -> m_pDisplayMonitorInfoList = pNewDisplayMonitorInfo;
					pNewDisplayMonitorInfo -> pNextMonitor = 0;
					pNewDisplayMonitorInfo -> DisplayAssignment = DISPLAY_ASSIGNMENT_UNSPECIFIED;
					pNewDisplayMonitorInfo -> m_pGraphicsAdapter = (void*)pAdapter;
					pNewDisplayMonitorInfo -> DesktopCoverageRectangle.left = DisplayDeviceMode.dmPosition.x;
					pNewDisplayMonitorInfo -> DesktopCoverageRectangle.top = DisplayDeviceMode.dmPosition.y;
					pNewDisplayMonitorInfo -> DesktopCoverageRectangle.right =
								pNewDisplayMonitorInfo -> DesktopCoverageRectangle.left + DisplayDeviceMode.dmPelsWidth;
					pNewDisplayMonitorInfo -> DesktopCoverageRectangle.bottom =
								pNewDisplayMonitorInfo -> DesktopCoverageRectangle.top + DisplayDeviceMode.dmPelsHeight;
					// Copy the new monitor to the global display list.
					// Move to the end of the global monitor list.
					m_DisplayMonitorCount++;
					pLastLinkedGlobalMonitorInfo = m_pDisplayMonitorInfoList;
					while ( pLastLinkedGlobalMonitorInfo != 0 && pLastLinkedGlobalMonitorInfo -> pNextMonitor != 0 )
						pLastLinkedGlobalMonitorInfo = pLastLinkedGlobalMonitorInfo -> pNextMonitor;
					pNewGlobalMonitorInfo = (MONITOR_INFO*)malloc( sizeof(MONITOR_INFO) );
					if ( pNewGlobalMonitorInfo != 0 )
						{
						memcpy( (void*)pNewGlobalMonitorInfo, (void*)pNewDisplayMonitorInfo, sizeof(MONITOR_INFO) );
						pNewGlobalMonitorInfo -> pNextMonitor = 0;
						if ( pNewGlobalMonitorInfo != 0 )
							{
							if ( m_pDisplayMonitorInfoList == 0 )
								{
								pLastLinkedGlobalMonitorInfo = pNewGlobalMonitorInfo;
								m_pDisplayMonitorInfoList = pLastLinkedGlobalMonitorInfo;
								}
							else
								{
								pLastLinkedGlobalMonitorInfo -> pNextMonitor = pNewGlobalMonitorInfo;
								pLastLinkedGlobalMonitorInfo = pNewGlobalMonitorInfo;
								}
							}
						}
					}
				}
			}
		nAdapterDevice++;
		}
}



void CMainFrame::OrganizeMultipleDisplayMonitorLayout()
{
	MONITOR_INFO	*pDisplayMonitorInfo;
	MONITOR_INFO	*pPrimaryDisplayMonitorInfo;
	MONITOR_INFO	*pSelectedDisplayMonitorInfo;
	unsigned short	nImageMonitor;

	// Assign the primary display monitor.
	pPrimaryDisplayMonitorInfo = 0;
	nImageMonitor = 2;
	pDisplayMonitorInfo = m_pDisplayMonitorInfoList;
	while ( pDisplayMonitorInfo != 0 )
		{
		// By definition, the primary monitor contains the origin of the extended desktop
		// display coordinates, (0,0).
		if ( pDisplayMonitorInfo -> DesktopCoverageRectangle.top == 0 &&
						pDisplayMonitorInfo -> DesktopCoverageRectangle.left == 0 )
			{
			pDisplayMonitorInfo -> DisplayAssignment = DISPLAY_ASSIGNMENT_PRIMARY;
			pDisplayMonitorInfo -> DisplayIdentity = DISPLAY_IDENTITY_PRIMARY;
			pPrimaryDisplayMonitorInfo = pDisplayMonitorInfo;
			}
		else
			pDisplayMonitorInfo -> DisplayIdentity = nImageMonitor++;
		pDisplayMonitorInfo = pDisplayMonitorInfo -> pNextMonitor;
		}
	// We have to have a primary monitor, so if, for any reason, none qualifies, take the first one.
	// (This code should never execute.)
	if ( pPrimaryDisplayMonitorInfo == 0 )
		{
		pPrimaryDisplayMonitorInfo = m_pDisplayMonitorInfoList;
		pPrimaryDisplayMonitorInfo -> DisplayAssignment = DISPLAY_ASSIGNMENT_PRIMARY;
		}

	// Assign the standards display monitor.  Make it the one farthest to the right (or left)
	// since it doesn't require as much mouse usage as the study monitor.
	// Start with the first one, in case that is the only one.
	pSelectedDisplayMonitorInfo = m_pDisplayMonitorInfoList;

	pDisplayMonitorInfo = m_pDisplayMonitorInfoList;
	while ( pDisplayMonitorInfo != 0 )
		{
		if ( pDisplayMonitorInfo -> DesktopCoverageRectangle.left > 0 &&
							pDisplayMonitorInfo -> DesktopCoverageRectangle.left >
								pSelectedDisplayMonitorInfo -> DesktopCoverageRectangle.left )
			pSelectedDisplayMonitorInfo = pDisplayMonitorInfo;
		else if ( pDisplayMonitorInfo -> DesktopCoverageRectangle.left < 0 &&
							pDisplayMonitorInfo -> DesktopCoverageRectangle.left <
								pSelectedDisplayMonitorInfo -> DesktopCoverageRectangle.left )
			pSelectedDisplayMonitorInfo = pDisplayMonitorInfo;
		pDisplayMonitorInfo = pDisplayMonitorInfo -> pNextMonitor;
		}
	pSelectedDisplayMonitorInfo -> DisplayAssignment |= DISPLAY_ASSIGNMENT_STANDARDS;

	// Assign the subject study display monitor.  Make it the one closest to the right (or left).
	//
	// For now, start with the primary monitor, and only shift off of it if an unassigned monitor
	// is found.
	if ( m_DisplayMonitorCount == 1 )
		pSelectedDisplayMonitorInfo = pPrimaryDisplayMonitorInfo;
	else if ( m_DisplayMonitorCount == 2 )
		{
		pDisplayMonitorInfo = m_pDisplayMonitorInfoList;
		while ( pDisplayMonitorInfo != 0 )
			{
			if ( pDisplayMonitorInfo != pPrimaryDisplayMonitorInfo )
				pSelectedDisplayMonitorInfo = pDisplayMonitorInfo;
			pDisplayMonitorInfo = pDisplayMonitorInfo -> pNextMonitor;
			}
		}
	else
		{
		pDisplayMonitorInfo = m_pDisplayMonitorInfoList;
		while ( pDisplayMonitorInfo != 0 )
			{
			if ( pDisplayMonitorInfo -> DisplayAssignment == DISPLAY_ASSIGNMENT_UNSPECIFIED  )
				pSelectedDisplayMonitorInfo = pDisplayMonitorInfo;
			pDisplayMonitorInfo = pDisplayMonitorInfo -> pNextMonitor;
			}
		}
	pSelectedDisplayMonitorInfo -> DisplayAssignment |= DISPLAY_ASSIGNMENT_STUDIES;
}



void CMainFrame::UpdateDisplayCustomization()
{
	MONITOR_INFO		*pDisplayMonitorInfo;

	pDisplayMonitorInfo = m_pDisplayMonitorInfoList;
	while ( pDisplayMonitorInfo != 0 )
		{
		// Get a reference to the primary monitor.
		if ( pDisplayMonitorInfo -> DisplayAssignment & DISPLAY_ASSIGNMENT_PRIMARY )
			{
			if ( pBViewerCustomization -> m_PrimaryMonitorWidthInMM == 0 )
				pBViewerCustomization -> m_PrimaryMonitorWidthInMM = (unsigned long)( ( (double)pDisplayMonitorInfo -> DesktopCoverageRectangle.right - 
																		(double)pDisplayMonitorInfo -> DesktopCoverageRectangle.left ) / 3.937 );
			if ( pBViewerCustomization -> m_PrimaryMonitorHeightInMM == 0 )
				pBViewerCustomization -> m_PrimaryMonitorHeightInMM = (unsigned long)( ( (double)pDisplayMonitorInfo -> DesktopCoverageRectangle.bottom - 
																		(double)pDisplayMonitorInfo -> DesktopCoverageRectangle.top ) / 3.937 );
			pDisplayMonitorInfo -> m_MonitorWidthInMM = pBViewerCustomization -> m_PrimaryMonitorWidthInMM;
			pDisplayMonitorInfo -> m_MonitorHeightInMM = pBViewerCustomization -> m_PrimaryMonitorHeightInMM;
			pDisplayMonitorInfo -> m_AssignedRenderingMethod = pBViewerCustomization -> m_PrimaryMonitorRenderingMethod;
			}
		else if ( pDisplayMonitorInfo -> DisplayIdentity == DISPLAY_IDENTITY_IMAGE2 )
			{
			if ( pBViewerCustomization -> m_Monitor2WidthInMM == 0 )
				pBViewerCustomization -> m_Monitor2WidthInMM = (unsigned long)( ( (double)pDisplayMonitorInfo -> DesktopCoverageRectangle.right - 
																		(double)pDisplayMonitorInfo -> DesktopCoverageRectangle.left ) / 3.937 );
			if ( pBViewerCustomization -> m_Monitor2HeightInMM == 0 )
				pBViewerCustomization -> m_Monitor2HeightInMM = (unsigned long)( ( (double)pDisplayMonitorInfo -> DesktopCoverageRectangle.bottom - 
																		(double)pDisplayMonitorInfo -> DesktopCoverageRectangle.top ) / 3.937 );
			pDisplayMonitorInfo -> m_MonitorWidthInMM = pBViewerCustomization -> m_Monitor2WidthInMM;
			pDisplayMonitorInfo -> m_MonitorHeightInMM = pBViewerCustomization -> m_Monitor2HeightInMM;
			pDisplayMonitorInfo -> m_AssignedRenderingMethod = pBViewerCustomization -> m_Monitor2RenderingMethod;
			}
		else if ( pDisplayMonitorInfo -> DisplayIdentity == DISPLAY_IDENTITY_IMAGE3 )
			{
			if ( pBViewerCustomization -> m_Monitor3WidthInMM == 0 )
				pBViewerCustomization -> m_Monitor3WidthInMM = (unsigned long)( ( (double)pDisplayMonitorInfo -> DesktopCoverageRectangle.right - 
																		(double)pDisplayMonitorInfo -> DesktopCoverageRectangle.left ) / 3.937 );
			if ( pBViewerCustomization -> m_Monitor3HeightInMM == 0 )
				pBViewerCustomization -> m_Monitor3HeightInMM = (unsigned long)( ( (double)pDisplayMonitorInfo -> DesktopCoverageRectangle.bottom - 
																		(double)pDisplayMonitorInfo -> DesktopCoverageRectangle.top ) / 3.937 );
			pDisplayMonitorInfo -> m_MonitorWidthInMM = pBViewerCustomization -> m_Monitor3WidthInMM;
			pDisplayMonitorInfo -> m_MonitorHeightInMM = pBViewerCustomization -> m_Monitor3HeightInMM;
			pDisplayMonitorInfo -> m_AssignedRenderingMethod = pBViewerCustomization -> m_Monitor3RenderingMethod;
			}
		pDisplayMonitorInfo = pDisplayMonitorInfo -> pNextMonitor;
		}
}


void FinishReaderInfoResponse( void *pResponseDialog )
{
	CPopupDialog			*pPopupDialog;
	
	pPopupDialog = (CPopupDialog*)pResponseDialog;
	delete pPopupDialog;
}


void CMainFrame::MakeAnnouncement( char *pMsg )
{
	static USER_NOTIFICATION_INFO	UserNotificationInfo;

	// Alert user that is program is not currently for diagnostic use.
	UserNotificationInfo.WindowWidth = 400;
	UserNotificationInfo.WindowHeight = 300;
	UserNotificationInfo.FontHeight = 0;	// Use default setting;
	UserNotificationInfo.FontWidth = 0;		// Use default setting;
	UserNotificationInfo.UserInputType = USER_INPUT_TYPE_OK;
	UserNotificationInfo.pUserNotificationMessage = pMsg;
	UserNotificationInfo.CallbackFunction = FinishReaderInfoResponse;
	PerformUserInput( &UserNotificationInfo );
}


int CMainFrame::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	int					PrimaryScreenWidth;
	int					PrimaryScreenHeight;
	RECT				ClientRect;
	int					ClientWidth;
	int					ClientHeight;
	RECT				DialogBarRect;
	int					DialogBarHeight;
	RECT				ImageWindowRect;
	MONITOR_INFO		*pDisplayMonitorInfo;
	MONITOR_INFO		*pPrimaryDisplayMonitorInfo;
	MONITOR_INFO		*pSubjectStudyDisplayMonitorInfo;
	MONITOR_INFO		*pStandardsDisplayMonitorInfo;
	
	PrimaryScreenWidth = ::GetSystemMetrics( SM_CXSCREEN );
	PrimaryScreenHeight = ::GetSystemMetrics( SM_CYSCREEN );

	pPrimaryDisplayMonitorInfo = 0;
	pStandardsDisplayMonitorInfo = 0;
	pSubjectStudyDisplayMonitorInfo = 0;
	UpdateDisplayCustomization();
	pDisplayMonitorInfo = m_pDisplayMonitorInfoList;
	// Set default assignments in case the configuration has been corrupted.
	pSubjectStudyDisplayMonitorInfo = pDisplayMonitorInfo;
	pStandardsDisplayMonitorInfo = pDisplayMonitorInfo;
	while ( pDisplayMonitorInfo != 0 )
		{
		// Get a reference to the primary monitor.
		if ( pDisplayMonitorInfo -> DisplayAssignment & DISPLAY_ASSIGNMENT_PRIMARY )
			{
			pPrimaryDisplayMonitorInfo = pDisplayMonitorInfo;
			if ( pBViewerCustomization -> m_DisplayAssignments & ASSIGN_STD_DISPLAY_PRIMARY )
				pStandardsDisplayMonitorInfo = pDisplayMonitorInfo;
			if ( pBViewerCustomization -> m_DisplayAssignments & ASSIGN_STUDY_DISPLAY_PRIMARY )
				pSubjectStudyDisplayMonitorInfo = pDisplayMonitorInfo;
			}

		if ( pBViewerCustomization -> m_DisplayAssignments & ASSIGN_STD_DISPLAY_AUTO )
			{
			if ( pDisplayMonitorInfo -> DisplayAssignment & DISPLAY_ASSIGNMENT_STANDARDS )
				pStandardsDisplayMonitorInfo = pDisplayMonitorInfo;
			}
		else if ( pBViewerCustomization -> m_DisplayAssignments & ASSIGN_STD_DISPLAY_MONITOR2 )
			{
			if ( pDisplayMonitorInfo -> DisplayIdentity == DISPLAY_IDENTITY_IMAGE2 )
				pStandardsDisplayMonitorInfo = pDisplayMonitorInfo;
			}
		else if ( pBViewerCustomization -> m_DisplayAssignments & ASSIGN_STD_DISPLAY_MONITOR3 )
			{
			if ( pDisplayMonitorInfo -> DisplayIdentity == DISPLAY_IDENTITY_IMAGE3 )
				pStandardsDisplayMonitorInfo = pDisplayMonitorInfo;
			}

		if ( pBViewerCustomization -> m_DisplayAssignments & ASSIGN_STUDY_DISPLAY_AUTO )
			{
			if ( pDisplayMonitorInfo -> DisplayAssignment & DISPLAY_ASSIGNMENT_STUDIES )
				pSubjectStudyDisplayMonitorInfo = pDisplayMonitorInfo;
			}
		else if ( pBViewerCustomization -> m_DisplayAssignments & ASSIGN_STUDY_DISPLAY_MONITOR2 )
			{
			if ( pDisplayMonitorInfo -> DisplayIdentity == DISPLAY_IDENTITY_IMAGE2 )
				pSubjectStudyDisplayMonitorInfo = pDisplayMonitorInfo;
			}
		else if ( pBViewerCustomization -> m_DisplayAssignments & ASSIGN_STUDY_DISPLAY_MONITOR3 )
			{
			if ( pDisplayMonitorInfo -> DisplayIdentity == DISPLAY_IDENTITY_IMAGE3 )
				pSubjectStudyDisplayMonitorInfo = pDisplayMonitorInfo;
			}

		pDisplayMonitorInfo = pDisplayMonitorInfo -> pNextMonitor;
		}
	m_pPrimaryDisplayMonitorInfo = pPrimaryDisplayMonitorInfo;
	if ( PopupWindowClass.GetLength() == 0 )
		PopupWindowClass = AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
			::LoadCursor(NULL, IDC_ARROW), (HBRUSH)::GetStockObject(WHITE_BRUSH), ThisBViewerApp.m_hApplicationIcon );

	if ( ExplorerWindowClass.GetLength() == 0 )
		ExplorerWindowClass = AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
			::LoadCursor(NULL, IDC_ARROW), (HBRUSH)::GetStockObject(WHITE_BRUSH), ThisBViewerApp.m_hApplicationIcon );

	if ( ChildFrameWindowClass.GetLength() == 0 )
		ChildFrameWindowClass = AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS, 
			::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), ThisBViewerApp.m_hApplicationIcon );

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	GetClientRect( &ClientRect );
	ClientWidth = ClientRect.right - ClientRect.left;
	ClientHeight = ClientRect.bottom - ClientRect.top;

	// Create the dialog bar across the top of the main window.
	m_wndDlgBar.m_FrameFunction = IMAGE_FRAME_FUNCTION_CONTROL;
	m_wndDlgBar.m_BkgdBrush.CreateSolidBrush( COLOR_PANEL_BKGD );
	if ( !m_wndDlgBar.Create( this, IDD_DIALOGBAR_MAIN, WS_CHILD, IDD_DIALOGBAR_MAIN ) )
		return -1;      // fail to create

	// Create views to occupy the remaining client area of the main frame.
	GetClientRect( &ClientRect );
	m_wndDlgBar.GetWindowRect( &DialogBarRect );
	DialogBarHeight = DialogBarRect.bottom - DialogBarRect.top;
	ClientRect.top += DialogBarHeight;
	ClientWidth = ClientRect.right - ClientRect.left;
	ClientHeight = ClientRect.bottom - ClientRect.top;

	// Create the control panel window on the primary display monitor.
	if ( m_pControlPanel != 0 )
		{
		if ( !m_pControlPanel -> Create( this, DS_CONTEXTHELP | WS_CHILD | WS_VISIBLE, 0 ) )
			return -1;
		}


	if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
		{
		if ( PrimaryScreenWidth > PrimaryScreenHeight )
			ImageWindowRect = CRect( PrimaryScreenWidth * 11 / 20,
									0,
									PrimaryScreenWidth,
									PrimaryScreenHeight - 25 );
		else
			ImageWindowRect = CRect( PrimaryScreenWidth * 5 / 20,
									PrimaryScreenWidth * 15 * 14 / ( 20 * 17),
									PrimaryScreenWidth,
									PrimaryScreenHeight - 25 );

		// Create the report window on the primary display monitor.
		m_pImageFrame[ IMAGE_FRAME_REPORT ] = (CImageFrame*)new CImageFrame();
		if ( m_pImageFrame[ IMAGE_FRAME_REPORT ] != 0 )
			{
			m_pImageFrame[ IMAGE_FRAME_REPORT ] -> m_FrameFunction = IMAGE_FRAME_FUNCTION_REPORT;
			m_pImageFrame[ IMAGE_FRAME_REPORT ] -> m_BkgdBrush.CreateSolidBrush( COLOR_REPORT_HEADER );
			m_pImageFrame[ IMAGE_FRAME_REPORT ] -> m_wndDlgBar.m_BkgdBrush.CreateSolidBrush( COLOR_REPORT_HEADER );
			m_pImageFrame[ IMAGE_FRAME_REPORT ] -> m_wndDlgBar.m_FrameFunction = IMAGE_FRAME_FUNCTION_REPORT;
			m_pImageFrame[ IMAGE_FRAME_REPORT ] -> m_pDisplayMonitor = pPrimaryDisplayMonitorInfo;
			if ( !m_pImageFrame[ IMAGE_FRAME_REPORT ] -> CreateEx( WS_EX_APPWINDOW | WS_EX_DLGMODALFRAME, (const char*)ChildFrameWindowClass,
							"Interpretation Report", WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
							ImageWindowRect,
							NULL, AFX_IDW_PANE_FIRST + 2, NULL ))
				return -1;
			else
				m_pImageFrame[ IMAGE_FRAME_REPORT ] -> UpdateWindow();
			}
		}


	if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
		{
		if ( PrimaryScreenWidth > PrimaryScreenHeight )
			ImageWindowRect = CRect( PrimaryScreenWidth * 8 / 20,
									0,
									PrimaryScreenWidth * 17 / 20,
									PrimaryScreenHeight - 25 );
		else
			ImageWindowRect = CRect( 0,
									PrimaryScreenWidth * 15 * 14 / ( 20 * 17),
									PrimaryScreenWidth * 15 / 20,
									PrimaryScreenHeight - 25 );

		// Create the subject study image window.
		if ( ( pSubjectStudyDisplayMonitorInfo -> DisplayAssignment & DISPLAY_ASSIGNMENT_PRIMARY ) == 0 )
			// If the subject study images are not being displayed on the primary monitor, reset the image rectangle.
			ImageWindowRect = pSubjectStudyDisplayMonitorInfo -> DesktopCoverageRectangle;

		m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] = (CImageFrame*)new CImageFrame();
		if ( m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] != 0 )
			{
			m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> m_FrameFunction = IMAGE_FRAME_FUNCTION_PATIENT;
			m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> m_wndDlgBar.m_FrameFunction = IMAGE_FRAME_FUNCTION_PATIENT;
			m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> m_BkgdBrush.CreateSolidBrush( COLOR_PATIENT );
			m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> m_wndDlgBar.m_BkgdBrush.CreateSolidBrush( COLOR_PATIENT );
			m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> m_pDisplayMonitor = pSubjectStudyDisplayMonitorInfo;
			if ( !m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> CreateEx( WS_EX_APPWINDOW | WS_EX_DLGMODALFRAME, (const char*)ChildFrameWindowClass,
							"Subject Study Image", WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
							ImageWindowRect,
							NULL, AFX_IDW_PANE_FIRST, NULL ))
				{
				return -1;
				}
			else
				{
				m_pImageFrame[ IMAGE_FRAME_SUBJECT_STUDY ] -> UpdateWindow();
				}
			}
		}

	if ( PrimaryScreenWidth > PrimaryScreenHeight )
		ImageWindowRect = CRect( PrimaryScreenWidth * 11 / 20,
								0,
								PrimaryScreenWidth,
								PrimaryScreenHeight - 25 );
	else
		ImageWindowRect = CRect( PrimaryScreenWidth * 5 / 20,
								PrimaryScreenWidth * 15 * 14 / ( 20 * 17),
								PrimaryScreenWidth,
								PrimaryScreenHeight - 25 );

	// Create the standard image window on the standards display monitor.
	if ( ( pStandardsDisplayMonitorInfo -> DisplayAssignment & DISPLAY_ASSIGNMENT_PRIMARY ) == 0 )
		// If the standards are not being displayed on the primary monitor, reset the image rectangle.
		ImageWindowRect = pStandardsDisplayMonitorInfo -> DesktopCoverageRectangle;

	m_pImageFrame[ IMAGE_FRAME_STANDARD ] = (CImageFrame*)new CImageFrame();
	if ( m_pImageFrame[ IMAGE_FRAME_STANDARD ] != 0 )
		{
		m_pImageFrame[ IMAGE_FRAME_STANDARD ] -> m_FrameFunction = IMAGE_FRAME_FUNCTION_STANDARD;
		m_pImageFrame[ IMAGE_FRAME_STANDARD ] -> m_wndDlgBar.m_FrameFunction = IMAGE_FRAME_FUNCTION_STANDARD;
		m_pImageFrame[ IMAGE_FRAME_STANDARD ] -> m_BkgdBrush.CreateSolidBrush( COLOR_STANDARD );
		m_pImageFrame[ IMAGE_FRAME_STANDARD ] -> m_wndDlgBar.m_BkgdBrush.CreateSolidBrush( COLOR_STANDARD );
		m_pImageFrame[ IMAGE_FRAME_STANDARD ] -> m_pDisplayMonitor = pStandardsDisplayMonitorInfo;
		if ( !m_pImageFrame[ IMAGE_FRAME_STANDARD ] -> CreateEx( WS_EX_APPWINDOW | WS_EX_DLGMODALFRAME, (const char*)ChildFrameWindowClass,
						"ILO Standard Image", WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
						ImageWindowRect,
						NULL, AFX_IDW_PANE_FIRST + 1, NULL ))
			return -1;
		else
			m_pImageFrame[ IMAGE_FRAME_STANDARD ] -> UpdateWindow();
		}

	// Set the displayed version number.
	SetWindowText( " BViewer 1.2p Control Panel" );

	CRect			StandardDlgRect;
	
	m_pSelectStandardDlg = new CSelectStandard();
	if ( m_pSelectStandardDlg != 0 )
		{
		m_pSelectStandardDlg -> GetWindowRect( &StandardDlgRect );
		m_pSelectStandardDlg -> SetWindowPos( 0, 10, ::GetSystemMetrics( SM_CYSCREEN ) - StandardDlgRect.Height() - 30,
											StandardDlgRect.Width(), StandardDlgRect.Height(), SWP_NOSIZE | SWP_NOZORDER );
		m_pSelectStandardDlg -> ShowWindow( SW_SHOW );
		}

	if ( BViewerConfiguration.InterpretationEnvironment != INTERP_ENVIRONMENT_STANDARDS )
		{
		m_pSplashWnd = new CSplashWnd();
		if ( m_pSplashWnd != 0 )
			{
			m_pSplashWnd -> SetPosition( ( ClientWidth - 620 ) / 2, ( ClientHeight - 550 ) / 2, this, PopupWindowClass );
			m_pSplashWnd -> BringWindowToTop();
			m_pSplashWnd -> SetFocus();
			m_bSplashScreenIsUp = TRUE;
			}
		}
	
	return 0;
}


BOOL CMainFrame::PreCreateWindow( CREATESTRUCT &cs )
{
	if( !CFrameWnd::PreCreateWindow( cs ) )
		return FALSE;

	// Size and position the control window.
	cs.cy = 860;
	cs.cx = 1280;
	cs.x = 0;
	cs.y = 0;

	return TRUE;
}


BOOL CMainFrame::OnCmdMsg( UINT nID, int nCode, void *pExtra, AFX_CMDHANDLERINFO *pHandlerInfo )
{
	return CFrameWnd::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
}


void CMainFrame::OnSize( UINT nType, int cx, int cy )
{
	CFrameWnd::OnSize( nType, cx, cy );

	RECT			ClientRect;
	INT				ClientWidth;
	INT				ClientHeight;
	RECT			DialogBarRect;
	INT				DialogBarHeight;

	GetClientRect( &ClientRect );
	ClientWidth = ClientRect.right - ClientRect.left;
	ClientHeight = ClientRect.bottom - ClientRect.top;
	m_wndDlgBar.SetWindowPos( 0, ClientRect.left, ClientRect.top, ClientWidth, MAIN_DIALOG_BAR_HEIGHT, 0 );

	GetClientRect( &ClientRect );
	m_wndDlgBar.GetWindowRect( &DialogBarRect );
	DialogBarHeight = DialogBarRect.bottom - DialogBarRect.top;
	ClientRect.top += DialogBarHeight;

	ClientWidth = ClientRect.right - ClientRect.left;
	ClientHeight = ClientRect.bottom - ClientRect.top;
	if ( m_pControlPanel != 0 )
		m_pControlPanel -> SetWindowPos( 0, ClientRect.left, ClientRect.top, ClientWidth, ClientHeight, 0 );
}


void CMainFrame::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
			BOOL					bRequestChangeWindow;
		 	CMainFrame				*pMainFrame;
			CImageFrame				*pSubjectImageFrame;
			CImageFrame				*pReferenceImageFrame;
			CImageFrame				*pInterpretationReportImageFrame;
			WINDOWPLACEMENT			WindowPlacement;
			RECT					ImageWindowRect;

	if ( m_bSplashScreenIsUp && m_pSplashWnd != 0 )
		m_pSplashWnd -> OnChar( nChar, nRepCnt, nFlags );
	else
		{
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
					SetFocus();
					bRequestChangeWindow = TRUE;
					}
				}
			}
		if ( nChar == 0x1B )		// If this is the ESC (escape) key...
			bTheLastKeyPressedWasESC = TRUE;
		CFrameWnd::OnChar( nChar, nRepCnt, nFlags );
		}
}


void CMainFrame::OnDeleteCheckedImages( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( m_pControlPanel != 0 )
		m_pControlPanel -> m_SelectStudyPage.DeleteCheckedImages();
	*pResult = 0;
}


void CMainFrame::OnImportLocalImages( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( m_pControlPanel != 0 )
		m_pControlPanel -> m_SelectStudyPage.OnImportLocalImages();
	*pResult = 0;
}


void CMainFrame::OnShowLogDetail( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( m_pControlPanel != 0 )
		m_pControlPanel -> m_ViewLogPage.OnShowLogDetail();
	*pResult = 0;
}


void CMainFrame::UpdateImageList()
{
	if ( m_pControlPanel != 0 && !m_bProcessingNewImages )
		{
		m_bProcessingNewImages = TRUE;
		LogMessage( "Processing new abstract data.", MESSAGE_TYPE_SUPPLEMENTARY );
		ThisBViewerApp.ReadNewAbstractData( );
		m_pControlPanel -> m_SelectStudyPage.UpdateSelectionList();
		LogMessage( "Disabling new images button.", MESSAGE_TYPE_SUPPLEMENTARY );
		m_wndDlgBar.m_ButtonShowNewImages.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
		m_wndDlgBar.Invalidate();
		m_wndDlgBar.UpdateWindow();
		ThisBViewerApp.m_nNewStudiesImported = 0;
		m_bProcessingNewImages = FALSE;
		}
}


void CMainFrame::OnCreateAManualStudy( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( m_pControlPanel != 0 )
		m_pControlPanel -> m_SelectStudyPage.OnCreateAManualStudy();
	*pResult = 0;
}


void CMainFrame::AddNewlyArrivedStudies()
{
	LIST_ELEMENT			*pListElement;
	void					*pNewStudy;

	pListElement = ThisBViewerApp.m_NewlyArrivedStudyList;
	while ( pListElement != 0 )
		{
		pNewStudy = pListElement -> pItem;
		if ( pNewStudy != 0 )
			AppendToList( &ThisBViewerApp.m_AvailableStudyList, pNewStudy );
		pListElement = pListElement -> pNextListElement;
		}
	DissolveList( &ThisBViewerApp.m_NewlyArrivedStudyList );
}


// Respond to "new images" button press.
void CMainFrame::OnUpdateImageList( NMHDR *pNMHDR, LRESULT *pResult )
{
	LogMessage( "New images button pressed.", MESSAGE_TYPE_SUPPLEMENTARY );
	if ( m_pControlPanel != 0 && !m_bProcessingNewImages )
		{
		m_bProcessingNewImages = TRUE;
		AddNewlyArrivedStudies();
		m_pControlPanel -> m_SelectStudyPage.UpdateSelectionList();
		LogMessage( "Disabling new images button.", MESSAGE_TYPE_SUPPLEMENTARY );
		m_wndDlgBar.m_ButtonShowNewImages.ChangeStatus( CONTROL_VISIBLE, CONTROL_INVISIBLE );
		m_wndDlgBar.Invalidate();
		m_wndDlgBar.UpdateWindow();
		ThisBViewerApp.m_nNewStudiesImported = 0;
		m_bProcessingNewImages = FALSE;
		if ( ThisBViewerApp.m_bAutoViewStudyReceived )
			{
			if ( m_pControlPanel -> m_SelectStudyPage.m_pPatientListCtrl != 0 )
				m_pControlPanel -> m_SelectStudyPage.m_pPatientListCtrl -> AutoSelectPatientItem( ThisBViewerApp.m_AutoLoadSOPInstanceUID );
			strcpy( ThisBViewerApp.m_AutoLoadSOPInstanceUID, "" );
			ThisBViewerApp.m_bAutoViewStudyReceived = FALSE;
			}
		}
	*pResult = 0;
}


// Auto-process a new abstract.
void CMainFrame::AutoImportNewImage()
{
	LogMessage( "Automatically processing and viewing a new abstract.", MESSAGE_TYPE_SUPPLEMENTARY );
	if ( m_pControlPanel != 0 && !m_bProcessingNewImages )
		{
		m_bProcessingNewImages = TRUE;
		LogMessage( "Processing new abstract data.", MESSAGE_TYPE_SUPPLEMENTARY );

		m_pControlPanel -> SetActivePage( STUDY_SELECTION_PAGE );
		m_pControlPanel -> m_SelectStudyPage.UpdateSelectionList();
		ThisBViewerApp.m_nNewStudiesImported = 0;
		if ( m_pControlPanel -> m_SelectStudyPage.m_pPatientListCtrl != 0 )
			m_pControlPanel -> m_SelectStudyPage.m_pPatientListCtrl -> AutoSelectPatientItem( ThisBViewerApp.m_AutoLoadSOPInstanceUID );
		strcpy( ThisBViewerApp.m_AutoLoadSOPInstanceUID, "" );
		ThisBViewerApp.m_bAutoViewStudyReceived = FALSE;
		ThisBViewerApp.m_bAutoViewStudyInProgress = TRUE;
		m_bProcessingNewImages = FALSE;
		}
}


// Auto-process a new abstract.
void CMainFrame::AutoProcessTheNextImage()
{
	LIST_ELEMENT			*pAvailableStudyListElement;
	CStudy					*pStudy;
	BOOL					bEligibleStudyFound;
	BOOL					bMatchingImageFound;
	DIAGNOSTIC_STUDY		*pStudyDataRow;
	DIAGNOSTIC_SERIES		*pSeriesDataRow;
	DIAGNOSTIC_IMAGE		*pImageDataRow;

	LogMessage( "Automatically processing and generating a report from an abstract export file.", MESSAGE_TYPE_SUPPLEMENTARY );
	if ( m_pControlPanel != 0 )
		m_pControlPanel -> SetActivePage( REPORT_PAGE );
	pAvailableStudyListElement = ThisBViewerApp.m_AvailableStudyList;
	bEligibleStudyFound = FALSE;
	bMatchingImageFound = FALSE;
	while ( pAvailableStudyListElement != 0 && !bEligibleStudyFound )
		{
		pStudy = (CStudy*)pAvailableStudyListElement -> pItem;
		if ( pStudy != 0 )
			{
			// Preset the pointer to the following element in the list.
			pAvailableStudyListElement = pAvailableStudyListElement -> pNextListElement;
			if ( pStudy -> m_bStudyWasPreviouslyInterpreted )
				{
				bEligibleStudyFound = TRUE;
				ThisBViewerApp.m_pCurrentStudy = pStudy;
				pStudyDataRow = pStudy -> m_pDiagnosticStudyList;
				while ( pStudyDataRow != 0 && !bMatchingImageFound )
					{
					pSeriesDataRow = pStudyDataRow -> pDiagnosticSeriesList;
					while ( pSeriesDataRow != 0 && !bMatchingImageFound )
						{
						pImageDataRow = pSeriesDataRow -> pDiagnosticImageList;
						while ( pImageDataRow != 0 && !bMatchingImageFound )
							{
							// The selected study has been identified.
							bMatchingImageFound = TRUE;
							pStudy -> m_pCurrentStudyInfo = pStudyDataRow;
							pStudy -> m_pCurrentSeriesInfo = pSeriesDataRow;
							pStudy -> m_pCurrentImageInfo = pImageDataRow;
							pImageDataRow = pImageDataRow -> pNextDiagnosticImage;
							}
						pSeriesDataRow = pSeriesDataRow -> pNextDiagnosticSeries;
						}
					pStudyDataRow = pStudyDataRow -> pNextDiagnosticStudy;
					}	
				if ( m_pControlPanel != 0 )
					m_pControlPanel -> m_ComposeReportPage.ProduceAutomaticReport();
				}
			}
		}
	// NOTE: The report approval process will have concluded by changing the current
	// control panel tab to the study selection screen.
	m_pControlPanel -> m_SelectStudyPage.UpdateSelectionList();
	bABatchStudyIsBeingProcessed = FALSE;
}


void CMainFrame::OnAppExit( NMHDR *pNMHDR, LRESULT *pResult )
{
	ThisBViewerApp.TerminateTimers();
	AfxGetMainWnd() -> SendMessage( WM_CLOSE );

	*pResult = 0;
}


HBRUSH CMainFrame::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
{
    if ( nCtlColor == CTLCOLOR_EDIT )
		{
		pDC -> SetBkColor( ( (TomEdit*)pWnd ) -> m_IdleBkgColor );
		pDC -> SetTextColor( ( (TomEdit*)pWnd ) -> m_TextColor );
		}

	return HBRUSH( m_BkgdBrush );
}


// This function will not wait for a user response before it returns to the calling function.
void CMainFrame::PerformUserInput( USER_NOTIFICATION_INFO *pUserNotificationInfo )
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
		pPopupDialog -> SetPosition( ( ClientWidth - DialogWidth ) / 2, ( ClientHeight - DialogHeight ) / 2, this, PopupWindowClass );
		pPopupDialog -> BringWindowToTop();
		pPopupDialog -> SetFocus();
		}
}


static unsigned __stdcall PerformThreadedUserInput( void *pUserData )
{
	RECT					ClientRect;
	INT						ClientWidth;
	INT						ClientHeight;
	int						DialogWidth;
	int						DialogHeight;
	USER_NOTIFICATION_INFO	*pUserNotificationInfo;
	USER_NOTIFICATION		*pUserQCNotice;
 	CMainFrame				*pMainFrame;
	MSG						WindowsMessage;
	BOOL					bExitWindowsMessageLoop;
	CPopupDialog			*pThreadedPopupDialog = 0;

	ThisBViewerApp.m_NumberOfSpawnedThreads++;
	// Request the user to enter the BRetriever's AE_Title.
	pMainFrame = (CMainFrame*)ThisBViewerApp.m_pMainWnd;
	if ( pMainFrame != 0 )
		{
		pUserNotificationInfo = (USER_NOTIFICATION_INFO*)pUserData;
		pMainFrame -> GetClientRect( &ClientRect );
		ClientWidth = ClientRect.right - ClientRect.left;
		ClientHeight = ClientRect.bottom - ClientRect.top;
		DialogWidth = pUserNotificationInfo -> WindowWidth;
		DialogHeight = pUserNotificationInfo -> WindowHeight;
		pUserQCNotice = (USER_NOTIFICATION*)pUserNotificationInfo -> pUserData;

		pThreadedPopupDialog = new CPopupDialog( DialogWidth, DialogHeight, COLOR_CONFIG, 0, IDD_DIALOG_POPUP );
		if ( pThreadedPopupDialog != 0 )
			{
			pThreadedPopupDialog -> m_pUserNotificationInfo = pUserNotificationInfo;
			pThreadedPopupDialog -> SetPosition( ( ClientWidth - DialogWidth ) / 2, ( ClientHeight - DialogHeight ) / 2, pMainFrame, PopupWindowClass );
			pThreadedPopupDialog -> SetFocus();

			bExitWindowsMessageLoop = FALSE;
			// Hold the thread active until the user supplies a response.
			while ( pThreadedPopupDialog != 0 && pUserQCNotice -> UserResponseCode == 0 && !bExitWindowsMessageLoop )
				{
				if ( PeekMessage( &WindowsMessage, NULL, 0, 0, PM_REMOVE ) )	// Is There A Message Waiting?
					{
					if ( WindowsMessage.message == WM_QUIT )					// Have We Received A Quit Message?
						{
						bExitWindowsMessageLoop = TRUE;							// If So done=TRUE
						}
					TranslateMessage( &WindowsMessage );					// Translate The Message
					DispatchMessage( &WindowsMessage );						// Dispatch The Message
					}
				}
			}
		}
	ThisBViewerApp.m_NumberOfSpawnedThreads--;
	
	return 0;
}


static void ProcessUserNotificationResponse( void *pResponseDialog )
{
	CPopupDialog			*pUserPopupDialog;
	USER_NOTIFICATION		*pUserQCNotice;
	
	pUserPopupDialog = (CPopupDialog*)pResponseDialog;
	pUserQCNotice = (USER_NOTIFICATION*)pUserPopupDialog -> m_pUserNotificationInfo -> pUserData;
	pUserQCNotice -> UserResponseCode = 0;
	if ( ( pUserQCNotice -> TypeOfUserResponseSupported & USER_RESPONSE_TYPE_SUSPEND ) != 0 )
		{
		if ( ( pUserPopupDialog -> m_pUserNotificationInfo -> UserResponse & POPUP_RESPONSE_SUSPEND ) != 0 )
			pUserQCNotice -> UserResponseCode |= USER_RESPONSE_CODE_SUSPEND;
		}
	if ( ( pUserQCNotice -> TypeOfUserResponseSupported & ( USER_RESPONSE_TYPE_YESNO | USER_RESPONSE_TYPE_YESNO_NO_CANCEL ) ) != 0 )
		{
		if ( ( pUserPopupDialog -> m_pUserNotificationInfo -> UserResponse & POPUP_RESPONSE_YES ) != 0 )
			pUserQCNotice -> UserResponseCode |= USER_RESPONSE_CODE_YES;
		else
			pUserQCNotice -> UserResponseCode |= USER_RESPONSE_CODE_NO;
		}
	if ( ( pUserQCNotice -> TypeOfUserResponseSupported & USER_RESPONSE_TYPE_CONTINUE ) != 0 )
		pUserQCNotice -> UserResponseCode |= USER_RESPONSE_CODE_CONTINUE;

	delete pUserPopupDialog;
}


// This function will wait for a user response before it returns to the calling function.
void CMainFrame::ProcessUserNotificationAndWaitForResponse( USER_NOTIFICATION *pUserQCNotice )
{
	static USER_NOTIFICATION_INFO	UserNotificationInfo;
	static char						TextString[ 1024 ];
	HANDLE							hTimerThreadHandle;
    unsigned						TimerThreadID;
    BOOL							bResponseHasBeenSolicited;
	MSG								WindowsMessage;
	BOOL							bExitWindowsMessageLoop;

	if ( ( pUserQCNotice -> TypeOfUserResponseSupported & USER_RESPONSE_TYPE_YESNO ) != 0 )
		UserNotificationInfo.UserInputType = USER_INPUT_TYPE_BOOLEAN;
	else if ( ( pUserQCNotice -> TypeOfUserResponseSupported & USER_RESPONSE_TYPE_YESNO_NO_CANCEL ) != 0 )
		UserNotificationInfo.UserInputType = USER_INPUT_TYPE_BOOLEAN_NO_CANCEL;
	else
		UserNotificationInfo.UserInputType = USER_INPUT_TYPE_OK;
	UserNotificationInfo.WindowWidth = 550;
	UserNotificationInfo.WindowHeight = pUserQCNotice -> TextLinesRequired * 30;
	UserNotificationInfo.FontHeight = 16;
	UserNotificationInfo.FontWidth = 8;
	strcpy( TextString, pUserQCNotice -> Source );
	strcat( TextString, " Quality Control:\n\n" );
	strcat( TextString, pUserQCNotice -> NoticeText );
	strcat( TextString, "\n\n" );
	strcat( TextString, pUserQCNotice -> SuggestedActionText );
	UserNotificationInfo.pUserNotificationMessage = TextString;
	UserNotificationInfo.CallbackFunction = ProcessUserNotificationResponse;
	UserNotificationInfo.pUserData = (void*)pUserQCNotice;
	UserNotificationInfo.UserResponse = 0;
	strcpy( UserNotificationInfo.UserTextResponse, "" );
	pUserQCNotice -> UserResponseCode = 0;

	hTimerThreadHandle = (HANDLE)_beginthreadex(	NULL,						// No security issues for child processes.
													0,							// Use same stack size as parent process.
													PerformThreadedUserInput,	// Thread function.
													&UserNotificationInfo,		// Argument for thread function.
													0,							// Initialize thread state as running.
													&TimerThreadID );
	bResponseHasBeenSolicited = ( hTimerThreadHandle != 0 );

	// Wait for an answer.
	bExitWindowsMessageLoop = FALSE;
	while ( pUserQCNotice -> UserResponseCode == 0 && !bExitWindowsMessageLoop )
		{
		if ( PeekMessage( &WindowsMessage, NULL, 0, 0, PM_REMOVE ) )	// Is There A Message Waiting?
			{
			if ( WindowsMessage.message == WM_QUIT )					// Have We Received A Quit Message?
				bExitWindowsMessageLoop = TRUE;							// If So done=TRUE
				TranslateMessage( &WindowsMessage );					// Translate The Message
				DispatchMessage( &WindowsMessage );						// Dispatch The Message
			}
		}
}


// This function returns to the calling function immediately.
// This function runs on the timer thread.
BOOL CMainFrame::ProcessUserNotificationWithoutWaiting( USER_NOTIFICATION *pUserQCNotice )
{
	static USER_NOTIFICATION_INFO	UserNotificationInfo;
	static char						TextString[ 1024 ];
	static USER_NOTIFICATION		UserQCNotice;
	BOOL							bUserNotificationWasProcessed;

	bUserNotificationWasProcessed = FALSE;
	memcpy( &UserQCNotice, pUserQCNotice, sizeof(USER_NOTIFICATION) );
	if ( ( pUserQCNotice -> TypeOfUserResponseSupported & USER_RESPONSE_TYPE_YESNO ) != 0 )
		UserNotificationInfo.UserInputType = USER_INPUT_TYPE_BOOLEAN;
	else if ( ( pUserQCNotice -> TypeOfUserResponseSupported & USER_RESPONSE_TYPE_YESNO_NO_CANCEL ) != 0 )
		UserNotificationInfo.UserInputType = USER_INPUT_TYPE_BOOLEAN_NO_CANCEL;
	else
		UserNotificationInfo.UserInputType = USER_INPUT_TYPE_OK;
	if ( ( pUserQCNotice -> TypeOfUserResponseSupported & USER_RESPONSE_TYPE_SUSPEND ) != 0 )
		UserNotificationInfo.UserInputType |= USER_INPUT_TYPE_SUSPEND;
	UserNotificationInfo.WindowWidth = 550;
	UserNotificationInfo.WindowHeight = ( 2 + pUserQCNotice -> TextLinesRequired ) * 30;
	UserNotificationInfo.FontHeight = 16;
	UserNotificationInfo.FontWidth = 8;
	strcpy( TextString, pUserQCNotice -> Source );
	strcat( TextString, " Quality Control:\n\n" );
	strcat( TextString, pUserQCNotice -> NoticeText );
	strcat( TextString, "\n\n" );
	strcat( TextString, pUserQCNotice -> SuggestedActionText );
	UserNotificationInfo.pUserNotificationMessage = TextString;
	UserNotificationInfo.CallbackFunction = ProcessUserNotificationResponse;
	UserNotificationInfo.pUserData = (void*)pUserQCNotice;
	UserNotificationInfo.UserResponse = 0;
	strcpy( UserNotificationInfo.UserTextResponse, "" );
	pUserQCNotice -> UserResponseCode = 0;
	PerformThreadedUserInput( (void*)&UserNotificationInfo );
	bUserNotificationWasProcessed = TRUE;

	return bUserNotificationWasProcessed;
}


LRESULT CMainFrame::OnAutoload( WPARAM wParam, LPARAM lParam )
{
	AutoImportNewImage();

	return 1;
}

LRESULT CMainFrame::OnAutoProcess( WPARAM wParam, LPARAM lParam )
{
	AutoProcessTheNextImage();

	return 1;
}

