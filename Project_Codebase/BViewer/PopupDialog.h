// PopupDialog.h - Defines the data structures and functions for the general-purpose
//  popup dialog box class.
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

typedef 	void (*POPUP_CALLBACK_FUNCTION)( void *pPopupDialog );

typedef struct
	{
	int							WindowWidth;
	int							WindowHeight;
	int							FontHeight;
	int							FontWidth;
	unsigned long				UserInputType;
									#define USER_INPUT_TYPE_UNSPECIFIED				0
									#define USER_INPUT_TYPE_BOOLEAN					1
									#define USER_INPUT_TYPE_BOOLEAN_NO_CANCEL		2
									#define USER_INPUT_TYPE_OK						4
									#define USER_INPUT_TYPE_EDIT					8
									#define USER_INPUT_TYPE_SUSPEND					16
	char						*pUserNotificationMessage;
	POPUP_CALLBACK_FUNCTION		CallbackFunction;
	void						*pUserData;
	unsigned long				UserResponse;
									#define POPUP_RESPONSE_OK			0x00000001
									#define POPUP_RESPONSE_SAVE			0x00000001
									#define POPUP_RESPONSE_YES			0x00000001
									#define POPUP_RESPONSE_NO			0x00000002
									#define POPUP_RESPONSE_SUSPEND		0x00000004
									#define POPUP_RESPONSE_CANCEL		0x00000008
	char						UserTextResponse[ MAX_CFG_STRING_LENGTH ];
	} USER_NOTIFICATION_INFO;



// CPopupDialog
class CPopupDialog : public CWnd
{
public:
	CPopupDialog( int DialogWidth, int DialogHeight, COLORREF BackgroundColor, DWORD WindowStyle, UINT nID );

	virtual ~CPopupDialog();

// Attributes
public:
	int						m_DialogWidth;
	int						m_DialogHeight;
	COLORREF				m_BackgroundColor;
	DWORD					m_WindowStyle;
	UINT					m_nObjectID;
	BOOL					m_bExternalCancelActivated;

	TomStatic				m_StaticUserMessage;
	TomButton				m_ButtonPopupSave;
	TomButton				m_ButtonPopupOK;
	TomButton				m_ButtonPopupYes;
	TomButton				m_ButtonPopupNo;
	TomButton				m_ButtonPopupCancel;
	TomButton				m_ButtonSuspendUpdates;
	TomEdit					m_EditUserTextInput;

	USER_NOTIFICATION_INFO	*m_pUserNotificationInfo;
	CBrush					m_BkgdBrush;


public:
	BOOL				SetPosition( int x, int y, CWnd *pParentWnd, CString WindowClass );
	void				CancelDialog();

// Overrides
protected:
	//{{AFX_VIRTUAL(CPopupDialog)
	virtual BOOL		OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult );
	//}}AFX_VIRTUAL


protected:
	DECLARE_MESSAGE_MAP()

	//{{AFX_MSG(CPopupDialog)
	afx_msg void		OnBnClickedPopupSave( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnBnClickedPopupOK( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnBnClickedPopupYes( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnBnClickedPopupNo( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnBnClickedPopupSuspend( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnBnClickedPopupCancel( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg BOOL		OnEraseBkgnd( CDC *pDC );
	afx_msg HBRUSH		OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
};

