// ControlPanel.h : Header file defining the structure of the CControlPanel class of
//	CPropertySheet, which implements the main set of tabbed windows for controlling the
//  BViewer application.
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
#pragma once

#include "PanelTabCtrl.h"
#include "AnalysisPage.h"
#include "SelectStudyPage.h"
#include "ComposeReportPage.h"
#include "ViewLogPage.h"
#include "CustomizePage.h"
#include "UserManualPage.h"


// CControlPanel

// Define flags for indicating the currently active property page.
typedef enum
	{
	STUDY_SELECTION_PAGE,
	INTERPRETATION_PAGE,
	REPORT_PAGE,
	LOG_PAGE,
	SETUP_PAGE,
	USER_MANUAL_PAGE
	} CURRENTLY_ACTIVE_PAGE;


class CControlPanel : public CPropertySheet
{
//	DECLARE_DYNAMIC( CControlPanel )

public:
	CControlPanel( UINT nIDCaption, CWnd *pParentWnd = NULL, UINT iSelectPage = 0 );
	CControlPanel( LPCTSTR pszCaption, CWnd *pParentWnd = NULL, UINT iSelectPage = 0 );
	virtual ~CControlPanel();

	CPanelTabCtrl			m_PanelTabControl;
	CSelectStudyPage		m_SelectStudyPage;
	CAnalysisPage			m_PerformAnalysisPage;
	CComposeReportPage		m_ComposeReportPage;
	CViewLogPage			m_ViewLogPage;
	CCustomizePage			m_CustomizePage;
	CUserManualPage			m_UserManualPage;
	BOOL					m_bPropertyPagesCreated;
	BOOL					m_bControlPanelInitialized;
	CURRENTLY_ACTIVE_PAGE	m_CurrentlyActivePage;
	CBrush					m_BkgdBrush;
	
protected:
// Overrides
	//{{AFX_VIRTUAL(CControlPanel)
	virtual BOOL				OnInitDialog();
	virtual BOOL				PreTranslateMessage( MSG *pMsg );
	//}}AFX_VIRTUAL

	DECLARE_MESSAGE_MAP()

public:
	//{{AFX_MSG(CControlPanel)
	afx_msg void				OnSize( UINT nType, int cx, int cy );
	afx_msg HBRUSH				OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor );
	afx_msg BOOL				OnEraseBkgnd( CDC *pDC );
	//}}AFX_MSG

};


