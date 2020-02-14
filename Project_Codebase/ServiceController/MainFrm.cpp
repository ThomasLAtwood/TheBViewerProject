// MainFrm.cpp : Implementation file for the CMainFrame class.
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
#include "Module.h"
#include "ServiceController.h"
#include "MainFrm.h"


extern CServiceControllerApp		ServiceControllerApp;

// CMainFrame

IMPLEMENT_DYNAMIC( CMainFrame, CFrameWnd )

BEGIN_MESSAGE_MAP( CMainFrame, CFrameWnd )
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
}


CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if ( CFrameWnd::OnCreate( lpCreateStruct ) == -1 )
		return -1;

	// Create a view to occupy the client area of the frame.
	if ( !m_ControlPanel.Create( NULL, NULL, AFX_WS_DEFAULT_VIEW,
									CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL ) )
		return -1;
	else
		SetIcon( ServiceControllerApp.m_hApplicationIcon, FALSE );

	return 0;
}


BOOL CMainFrame::PreCreateWindow( CREATESTRUCT& cs )
{
	if( !CFrameWnd::PreCreateWindow( cs ) )
		return FALSE;

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass( 0 );

	return TRUE;
}


// CMainFrame message handlers

void CMainFrame::OnSetFocus( CWnd* /*pOldWnd*/ )
{
	m_ControlPanel.SetFocus();
}


BOOL CMainFrame::OnCmdMsg( UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo )
{
	if ( m_ControlPanel.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) )
		return TRUE;

	return CFrameWnd::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
}


