// LoginScreen.h - Defines the data structures and functions for the login
//  dialog box.
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
//	*[1] 10/09/2023 by Tom Atwood
//		Added the READER_PERSONAL_INFO specification to the class declaration.
//
//
#pragma once

#include "TomGroup.h"
#include "TomStatic.h"
#include "TomButton.h"
#include "ControlTip.h"


// CLoginScreen dialog
class CLoginScreen : public CDialog
{
public:
	CLoginScreen( CWnd *pParent = NULL, READER_PERSONAL_INFO *pCurrReaderInfo = NULL );		// *[1]
	virtual ~CLoginScreen();

	BOOL				m_bUserRecognized;
	BOOL				m_bAccessGranted;
	BOOL				m_bUserRecognizedOnLastPass;
	BOOL				m_bAccessGrantedOnLastPass;

	TomStatic			m_StaticLoginBanner;
	TomStatic			m_StaticLoginTitle;

	TomStatic			m_StaticLoginName;
	TomEdit				m_EditLoginName;

	TomStatic			m_StaticLoginPassword;
	TomEdit				m_EditLoginPassword;
		
	TomStatic			m_StaticErrorNotification;

	TomButton			m_ButtonLogin;
	TomButton			m_ButtonCancelLogin;

	unsigned long		m_NumberOfRegisteredUsers;
	CControlTip			*m_pControlTip;
	CBrush				m_BkgdBrush;

	READER_PERSONAL_INFO *m_pCurrReaderInfo;					// *[1] Added this pointer.

	enum { IDD = IDD_DIALOG_LOGIN_SCREEN };

protected:
// Overrides
	//{{AFX_VIRTUAL(CLoginScreen)
	//}}AFX_VIRTUAL

	void					InitializeControlTips();

	DECLARE_MESSAGE_MAP()
public:
	//{{AFX_MSG(CLoginScreen)
	afx_msg void			OnEditLoginNameKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnEditLoginPasswordKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnBnClickedLogin( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnBnClickedCancelLogin( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnMouseMove(UINT nFlags, CPoint point);
	afx_msg HBRUSH			OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor );
	afx_msg BOOL			OnEraseBkgnd( CDC *pDC );
	//}}AFX_MSG

	virtual BOOL			OnInitDialog();
	virtual BOOL			CertifyLogin();
};


