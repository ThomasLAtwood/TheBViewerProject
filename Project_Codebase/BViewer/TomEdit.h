// TomEdit.h : Header file defining the structure of the TomEdit class of
//	enhanced text display and editing controls for use in a user interface window.
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


// TomEdit window
class TomEdit : public CEdit
{
public:
	TomEdit( char *pEditText, int EditWidth, int EditHeight, int FontHeight, int FontWidth, int FontWeight, int FontType,
				COLORREF TextColor, COLORREF BackgroundColor, COLORREF ActivatedBkgdColor, COLORREF VisitedBkgdColor,
				DWORD EditStyle, unsigned long ValidationType, UINT nID );

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
	COLORREF		m_SpecialBkgColor;
	DWORD			m_EditStyle;
						#define EDIT_VSCROLL							0x00010000
						#define EDIT_BORDER								0x00020000
						#define EDIT_READONLY							0x00040000
	UINT			m_nObjectID;
	CFont			m_TextFont;
	int				m_FontType;
						#define VARIABLE_PITCH_FONT		0
						#define FIXED_PITCH_FONT		1
	CBrush			m_BkgdBrush;
	CBrush			m_ActivatedBkgdBrush;
	CBrush			m_VisitedBkgdBrush;
	CBrush			*m_pCurrentBkgdBrush;
	BOOL			m_bHasReceivedInput;
	unsigned long	m_ValidationType;
						#define EDIT_VALIDATION_NONE					0x00000001
						#define EDIT_VALIDATION_NUMERIC					0x00000002
						#define EDIT_VALIDATION_NUMERIC_RANGE			0x00000004
						#define EDIT_VALIDATION_DECIMAL					0x00000008
						#define EDIT_VALIDATION_DECIMAL_RANGE			0x00000010
						#define EDIT_VALIDATION_SSN						0x00000020
	double			m_MinimumDecimalValue;
	double			m_MaximumDecimalValue;
	int				m_DecimalDigitsDisplayed;

// Operations
public:
	BOOL			CreateSpecifiedFont();
	BOOL			SetPosition( int x, int y, CWnd *pParentWnd );
	void			ChangeStatus( DWORD ClearStatus, DWORD SetStatus );
	void			HasBeenCompleted( BOOL bHasBeenCompleted );

// Overrides
	//{{AFX_VIRTUAL(TomEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual			~TomEdit();
	void			Reinitialize();
	void			SetDecimalRange( double MinValue, double MaxValue, int DecimalDigitsDisplayed );

	DECLARE_MESSAGE_MAP()

protected:
	// Generated message map functions
	//{{AFX_MSG(TomEdit)
	afx_msg void OnSetFocus( CWnd *pOldWnd );
	afx_msg void OnChar( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg void OnKillFocus( CWnd *pNewWnd );
	afx_msg UINT OnGetDlgCode();
	//}}AFX_MSG
};
