// ControlPanel.h : Structure definition for the CControlPanel class.
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

#include "TomStatic.h"
#include "TomButton.h"
#include "TomEdit.h"

// CControlPanel window

class CControlPanel : public CWnd
{
// Construction
public:
	CControlPanel();

// Attributes
public:

	TomStatic			m_StaticLogo;
	TomStatic			m_StaticTitle;
	TomStatic			m_StaticServiceController;
	
	TomButton			m_ButtonInstall;
	TomButton			m_ButtonStart;
	TomButton			m_ButtonShowStatus;
	TomButton			m_ButtonStop;
	TomButton			m_ButtonRemove;
	TomButton			m_ButtonShowLogDetail;
	TomButton			m_ButtonAbout;
	TomButton			m_ButtonExit;

	BOOL				m_bLogDisplayInitialized;
	unsigned char		m_LogGranularity;
							#define		SUMMARY_LOG		0
							#define		DETAIL_LOG		1
	TomEdit				m_EditLog;
	char				*m_pLogText;

// Operations
public:
	BOOL				ReadLogFile();

// Overrides
	protected:
	virtual BOOL		PreCreateWindow( CREATESTRUCT &cs );

// Implementation
public:
	virtual ~CControlPanel();

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL		OnEraseBkgnd( CDC* pDC );
	afx_msg int			OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg void		OnShowLogDetail();
	afx_msg void		OnButtonInstall();
	afx_msg void		OnButtonStart();
	afx_msg void		OnButtonShowStatus();
	afx_msg void		OnButtonStop();
	afx_msg void		OnButtonRemove();
	afx_msg void		OnSize( UINT nType, int cx, int cy );
	afx_msg void		OnAppAbout();
	afx_msg void		OnAppExit();
};

