// UserManualPage.h : Header file defining the structure of the CUserManualPage class
//  of CPropertyPage, which implements the "View User Manual" tab of the main Control Panel.
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

#define HELP_ERROR_INSUFFICIENT_MEMORY			1
#define HELP_ERROR_FILE_NOT_FOUND				2

#define HELP_ERROR_DICT_LENGTH					2

class CUserManualPage : public CPropertyPage
{
public:
	CUserManualPage();
	virtual ~CUserManualPage();

// Dialog Data
	enum { IDD = IDD_PROP_PAGE_HELP };

protected:
	BOOL			m_bHelpWindowIsOpen;
	HWND			m_hHelpWindow;
	HH_WINTYPE		m_HelpWindowType;
	BOOL			m_bHelpPageIsActivated;

			void		OpenHelpFile();
			void		CloseHelpFile();

// Overrides
	//{{AFX_VIRTUAL(CUserManualPage)
	virtual BOOL		PreTranslateMessage( MSG *pMsg );
	virtual BOOL		OnInitDialog();
	virtual BOOL		OnSetActive();
	virtual BOOL		OnNotify( WPARAM wParam, LPARAM lParam, LRESULT *pResult );
	//}}AFX_VIRTUAL

	DECLARE_MESSAGE_MAP()
public:
	//{{AFX_MSG(CUserManualPage)
	afx_msg void		OnSize( UINT nType, int cx, int cy );
	//}}AFX_MSG

};


// Function prototypes:
	void			InitHelpModule();
	void			CloseHelpModule();

