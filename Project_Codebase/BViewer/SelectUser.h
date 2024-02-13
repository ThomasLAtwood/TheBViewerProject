#pragma once
// SelectUser.h - Defines the data structures and functions for BViewer user
//	management.
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
//		Created this module.
//
//
#pragma once

#include "TomGroup.h"
#include "TomStatic.h"
#include "TomButton.h"
#include "TomEdit.h"
#include "TomComboBox.h"


// CSelectUser dialog

class CSelectUser : public CDialog
{
public:
	CSelectUser( CWnd *pParent = NULL, READER_PERSONAL_INFO *pReaderInfo = NULL, BOOL bSetInitialReader = FALSE );   // standard constructor
	virtual ~CSelectUser();


// Dialog Data
	enum { IDD = IDD_DIALOG_SELECT_READER };
	CBrush					m_BkgdBrush;

	TomStatic				m_StaticReaderSelection;
	TomStatic				m_StaticSelectReader;
	TomComboBox				m_ComboBoxSelectReader;
	TomButton				m_ButtonAddReader;
	TomButton				m_ButtonEditReader;
	TomButton				m_ButtonDeleteReader;
	TomButton				m_ButtonSetdDefaultReader;

	TomStatic				m_StaticReaderReportSignatureName;
	TomEdit					m_EditReaderReportSignatureName;

	TomButton				m_ButtonExit;

	int						m_nSelectedReaderItem;
	READER_PERSONAL_INFO	*m_pDefaultReaderInfo;
	READER_PERSONAL_INFO	*m_pInitialDefaultReaderInfo;

	BOOL					m_bChangingCurrentReader;
	READER_PERSONAL_INFO	m_ReaderInfo;
	CControlTip				*m_pControlTip;


protected:
// Overrides
	//{{AFX_VIRTUAL(CSelectUser)
	//}}AFX_VIRTUAL


	DECLARE_MESSAGE_MAP()

protected:
	void				InitializeControlTips();

	//{{AFX_MSG(CSelectUser)
	afx_msg void		OnReaderSelected();
	afx_msg void		OnBnClickedAddNewReader( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedEditReader( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedRemoveReader( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedSetDefaultReader( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedExitReaderSelection( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH		OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor );
	afx_msg BOOL		OnEraseBkgnd( CDC *pDC );
	virtual BOOL		OnInitDialog();
	afx_msg void		OnClose();
	//}}AFX_MSG

public:
	BOOL					LoadReaderSelectionList();
	void					ClearDefaultReaderFlag();
	void					SetAsDefaultReader( READER_PERSONAL_INFO *pReaderInfo );
};




// Function prototypes:
//
	void					AddNewReader();
	void					EditCurrentReader();
	void					RemoveCurrentReader();
	READER_PERSONAL_INFO	*GetDefaultReader();
	void					ReadUserList();
	void					WriteUserList();

