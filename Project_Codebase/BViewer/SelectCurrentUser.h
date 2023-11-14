#pragma once
// SelectCurrentUser.h - Defines the data structures and functions for BViewer user
//	selection.
//
//	Written by Thomas L. Atwood
//	P.O. Box 1089
//	West Fork, Arkansas 72774
//	(479)445-4690
//	TomAtwood@Earthlink.net
//
//	Copyright © 2023 CDC
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
//	*[1] 07/31/2023 by Tom Atwood
//		Created this file.
//
//
#pragma once

#include "TomStatic.h"
#include "TomButton.h"
#include "TomComboBox.h"
#include "ControlTip.h"


// CSelectCurrentUser dialog

class CSelectCurrentUser : public CDialog
{
public:
	CSelectCurrentUser( CWnd *pParent = NULL );   // standard constructor
	virtual ~CSelectCurrentUser();


// Dialog Data
	enum { IDD = IDD_DIALOG_SELECT_CURRENT_READER };
	CBrush					m_BkgdBrush;

	TomStatic				m_StaticReaderReportSignatureName;
	TomEdit					m_EditReaderReportSignatureName;

	TomStatic				m_StaticSelectReader;
	TomComboBox				m_ComboBoxSelectReader;

	int						m_nSelectedReaderItem;
	READER_PERSONAL_INFO	m_DefaultReaderInfo;

	TomButton				m_ButtonExit;


protected:
// Overrides
	//{{AFX_VIRTUAL(CSelectCurrentUser)
	//}}AFX_VIRTUAL


	DECLARE_MESSAGE_MAP()

protected:
	//{{AFX_MSG(CSelectCurrentUser)
	afx_msg void		OnReaderSelected();
	afx_msg void		OnBnClickedExitReaderSelection( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH		OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor );
	afx_msg BOOL		OnEraseBkgnd( CDC *pDC );
	afx_msg void		OnClose();
	//}}AFX_MSG

public:
	virtual BOOL		OnInitDialog();
	BOOL				LoadReaderSelectionList();
	void				ClearDefaultReaderFlag();

};




// Function prototypes:
//
