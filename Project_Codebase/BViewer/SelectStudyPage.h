// SelectStudyPage.h : Header file defining the structure of the CSelectStudyPage class of
//	CPropertyPage, which implements the "Select Subject Study" tab of the main Control Panel.
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

#include "StudySelector.h"
#include "ImportSelector.h"


// CSelectStudyPage dialog
class CSelectStudyPage : public CPropertyPage
{
public:
	CSelectStudyPage();
	virtual ~CSelectStudyPage();
	
	CBrush				m_BkgdBrush;

public:
	CStudySelector		*m_pPatientListCtrl;
	CImportSelector		*m_pImportSelector;
	char				m_AutoOpenFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];


// Dialog Data
	enum { IDD = IDD_PROP_PAGE_STUDY };

public:
	void					UpdateSelectionList();
	void					DeleteCheckedImages();
	void					DeleteStudyList();
	void					OnImportLocalImages();
	void					OnCreateAManualStudy();
	void					ResetCurrentSelection();

public:
// Overrides
	//{{AFX_VIRTUAL(CSelectStudyPage)
	virtual BOOL			OnInitDialog();
	virtual BOOL			OnSetActive();
	virtual BOOL			OnKillActive();
	virtual BOOL			OnNotify( WPARAM wParam, LPARAM lParam, LRESULT *pResult );
	//}}AFX_VIRTUAL

	DECLARE_MESSAGE_MAP()

	//{{AFX_VIRTUAL( CSelectStudyPage )
	afx_msg HBRUSH			OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor );
	afx_msg void			OnSize( UINT nType, int cx, int cy );
	//}}AFX_VIRTUAL

};
