// Mainfrm.h : Header file defining the structure of the CMainFrame class, which
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
// UPDATE HISTORY:
//
//	*[1] 03/14/2023 by Tom Atwood
//		Fixed code security issues.
//
#pragma once

#include "ControlPanel.h"
#include "ImageFrame.h"
#include "SelectStandard.h"
#include "GraphicsAdapter.h"
#include "SplashWnd.h"

#define		MAX_VIEW_COUNT		3


class CMainFrame : public CFrameWnd
{
friend CBViewerApp;
friend CControlPanel;
friend CCustomizePage;

public:
	CMainFrame();

// Attributes
public:
	int							m_DisplayMonitorCount;
	CGraphicsAdapter			*m_pGraphicsAdapterList;
	MONITOR_INFO				*m_pDisplayMonitorInfoList;
	MONITOR_INFO				*m_pPrimaryDisplayMonitorInfo;
	CFrameHeader				m_wndDlgBar;
									#define MAIN_DIALOG_BAR_HEIGHT		40
	CSelectStandard				*m_pSelectStandardDlg;
	CControlPanel				*m_pControlPanel;
	CImageFrame					*m_pImageFrame[ MAX_VIEW_COUNT ];
									#define IMAGE_FRAME_SUBJECT_STUDY		0
									#define IMAGE_FRAME_STANDARD			1
									#define IMAGE_FRAME_REPORT				2
	CBrush						m_BkgdBrush;
	HGLRC						m_hRC;				// OpenGL permanent rendering context.
	HDC							m_hDC;				// Private GDI device context.
	CSplashWnd					*m_pSplashWnd;
	BOOL						m_bSplashScreenIsUp;
	BOOL						m_bProcessingNewImages;
	BOOL						m_bOKToRenderImage;	// This flag is used to prevent a Windows paint message from
													//  one of the image windows from walking on the OpenGL
													//  rendering context of a different window.  When one window
													//  is busy preparing and rendering an image, another is not
													//  allowed to render.

public:
// Overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL			PreCreateWindow( CREATESTRUCT& cs );
	virtual BOOL			OnCmdMsg( UINT nID, int nCode, void *pExtra, AFX_CMDHANDLERINFO *pHandlerInfo );
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual					~CMainFrame();

	CGraphicsAdapter		*CatalogDisplayAdapter( char *pDisplayAdapterName );
	void					SurveyGraphicsAdapters();
	void					OrganizeMultipleDisplayMonitorLayout();
	void					UpdateDisplayCustomization();
	void					PerformUserInput( USER_NOTIFICATION_INFO *pUserQueryInfo );
	void					ProcessUserNotificationAndWaitForResponse( USER_NOTIFICATION *pUserQCNotice );
	void					ProcessUserNotificationWithoutWaiting( USER_NOTIFICATION *pUserQCNotice );			// *[1] Changed from BOOL return to void.
	void					MakeAnnouncement( char *pMsg );
	void					AddNewlyArrivedStudies();
	void					UpdateImageList();
	void					AutoImportNewImage();
	void					AutoProcessTheNextImage();

protected:
// Generated message map functions
	DECLARE_MESSAGE_MAP()
	//{{AFX_MSG(CMainFrame)
	afx_msg int				OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg void			OnSize( UINT nType, int cx, int cy );
	afx_msg void			OnDeleteCheckedImages( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnImportLocalImages( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnUpdateImageList( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnCreateAManualStudy( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnShowLogDetail( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnAppExit( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnClose();
	afx_msg HBRUSH			OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor );
	afx_msg void			OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg LRESULT			OnAutoload( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT			OnAutoProcess( WPARAM wParam, LPARAM lParam );
	//}}AFX_MSG


};

void			FinishReaderInfoResponse( void *pResponseDialog );

