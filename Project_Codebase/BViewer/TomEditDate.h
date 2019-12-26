// TomEditDate.h : Header file defining the structure of the TomEditDate class of
//	enhanced date display and editing controls for use in a user interface window.
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

#include "TomControl.h"
#include "TomGroup.h"


// TomEditDate control
class TomEditDate : public CDateTimeCtrl
{
public:
	TomEditDate( char *pEditText, int EditWidth, int EditHeight, int FontHeight, int FontWidth, int FontWeight,
				COLORREF TextColor, COLORREF BackgroundColor, COLORREF ActivatedBkgdColor, COLORREF VisitedBkgdColor,
				DWORD EditStyle, UINT nID );

// Attributes
public:
	TomGroup		*m_pGroup;				// Point to the edit group, if any, of which
											// this button is a member.
	CString			m_EditText;
	int				m_EditWidth;
	int				m_EditHeight;
	int				m_FontHeight;
	int				m_FontWidth;
	int				m_FontWeight;
	COLORREF		m_TextColor;
	COLORREF		m_OriginalIdleBkgColor;
	COLORREF		m_IdleBkgColor;
	COLORREF		m_ActivatedBkgdColor;
	COLORREF		m_VisitedBkgdColor;
	DWORD			m_EditStyle;
						#define EDIT_VSCROLL							0x00010000
						#define EDIT_BORDER								0x00020000
	UINT			m_nObjectID;
	CFont			m_TextFont;
	BOOL			m_bHasReceivedInput;

// Operations
public:
	BOOL			CreateSpecifiedFont();
	BOOL			SetPosition( int x, int y, CWnd *pParentWnd );
	void			ChangeStatus( DWORD ClearStatus, DWORD SetStatus );
	void			HasBeenCompleted( BOOL bHasBeenCompleted );

// Overrides
	//{{AFX_VIRTUAL(TomEditDate)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual			~TomEditDate();

// Generated message map functions
protected:

	DECLARE_MESSAGE_MAP()

	//{{AFX_MSG(TomEditDate)
	afx_msg BOOL OnEraseBkgnd( CDC *pDC );
	afx_msg void OnSetFocus( CWnd *pOldWnd );
	//}}AFX_MSG
};

