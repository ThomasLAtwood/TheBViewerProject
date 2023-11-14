// ReaderInfoScreen.h - Defines the data structures and functions for the reader
//  information dialog box.  This is where the user enters his or her identifying
//  information.
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
//	*[1] 08/02/2023 by Tom Atwood
//		Fixed the multiuser login logic.
//
//
#pragma once

#include "TomGroup.h"
#include "TomStatic.h"
#include "TomButton.h"
#include "TomComboBox.h"			// *[1] Added include file.


#define READER_INFO_CONTEXT_INSERT			0			// *[1] Added definitions.
#define READER_INFO_CONTEXT_CONFIRM			1			// *[1]


// CReaderInfoScreen dialog
class CReaderInfoScreen : public CDialog
{
public:
	// *[1] Added two constructor parameters: pReaderInfo and Context.
	CReaderInfoScreen( CWnd *pParent = NULL, READER_PERSONAL_INFO *pReaderInfo = NULL, int Context = READER_INFO_CONTEXT_INSERT );
	virtual ~CReaderInfoScreen();

// Dialog Data
	enum { IDD = IDD_DIALOG_READER_INFO };
	CBrush					m_BkgdBrush;

	TomStatic				m_StaticReaderIdentification;
	TomStatic				m_StaticReaderLastName;
	TomEdit					m_EditReaderLastName;

	TomStatic				m_StaticLoginName;
	TomEdit					m_EditLoginName;

	TomStatic				m_StaticReaderID;
	TomEdit					m_EditReaderID;

	TomStatic				m_StaticLoginPassword;
	TomEdit					m_EditLoginPassword;

	TomStatic				m_StaticAE_Title;
	TomEdit					m_EditAE_Title;

	TomStatic				m_StaticReaderInitials;
	TomEdit					m_EditReaderInitials;

	TomStatic				m_StaticReaderReportSignatureName;
	TomEdit					m_EditReaderReportSignatureName;

	TomStatic				m_StaticReaderStreetAddress;
	TomEdit					m_EditReaderStreetAddress;

	TomStatic				m_StaticReaderCity;
	TomEdit					m_EditReaderCity;

	TomStatic				m_StaticReaderState;
	TomEdit					m_EditReaderState;

	TomStatic				m_StaticReaderZipCode;
	TomEdit					m_EditReaderZipCode;

	TomStatic				m_StaticSelectCountry;		// *[1] Added
	TomComboBox				m_ComboBoxSelectCountry;	// *[1] Added

	TomButton				m_ButtonSave;
	TomButton				m_ButtonCancel;

	TomGroup				m_GroupEditSequencing;

	READER_PERSONAL_INFO	m_ReaderInfo;				// *[1] Added this structure to better isolate this module.
	READER_PERSONAL_INFO	*m_pReaderInfo;				// *[1] Added this structure to better isolate this module.
	BOOL					m_bReaderInfoLoaded;		// *[1] Added
	int						m_ReaderInputContext;		// *[1] Added
	CControlTip				*m_pControlTip;				// *[1] Added


protected:
// Overrides
	//{{AFX_VIRTUAL(CReaderInfoScreen)
	//}}AFX_VIRTUAL

	BOOL				LoadCountrySelectionList();		// *[1] Moved here from CustomizePage.h.
	void				LoadCurrentReaderInfo();		// *[1] Moved here from CustomizePage.h.
	void				InitializeControlTips();		// *[1] Added.
	BOOL				ValidateReaderInfo();					// *[1] Added this function.

	DECLARE_MESSAGE_MAP()

protected:
	//{{AFX_MSG(CReaderInfoScreen)
	afx_msg void		OnBnClickedSaveReaderInfo( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedCancelReaderInfo( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH		OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor );
	afx_msg BOOL		OnEraseBkgnd( CDC *pDC );
	virtual BOOL		OnInitDialog();
	//}}AFX_MSG
};


