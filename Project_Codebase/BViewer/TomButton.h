// TomButton.h : Header file defining the structure of the TomButton class of
//	enhanced button controls for use in a user interface window.
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
#include "ControlTip.h"
/////////////////////////////////////////////////////////////////////////////


class TomButton : public TomControl
{
public:
	TomButton( char *pButtonText, int ButtonWidth, int ButtonHeight, int FontHeight, int FontWidth, int FontWeight,
				COLORREF TextColor, COLORREF BackgroundColor, COLORREF ActivatedBkgdColor, COLORREF VisitedBkgdColor,
				DWORD ButtonStyle, UINT nID, char *pControlTipText = 0 );

// Attributes
	TomGroup							*m_pGroup;				// Point to the button group, if any, of which
																// this button is a member.
public:
											// Button style definitions, additional to those in TomControl.h.
											#define BUTTON_PUSHBUTTON		0x00010000
											#define BUTTON_CHECKBOX			0x00020000
											#define GROUP_STATIC			0x00040000		// Used to identify static groups.
											#define GROUP_EDIT				0x00080000		// Used to identify edit groups.
											#define BUTTON_FROZEN			0x00100000
											#define BUTTON_DEFAULT			0x00200000
											#define BUTTON_NO_TOGGLE_OFF	0x00400000
	COLORREF							m_VisitedBkgdColor;
	COLORREF							m_PressedBkgColor;
	COLORREF							m_SpecialBkgColor;
	COLORREF							m_DisabledBkgndColor;
	COLORREF							m_Light;
	COLORREF							m_Highlight;
	COLORREF							m_Shadow;
	COLORREF							m_DarkShadow;

public:
	unsigned int						m_ButtonState;
											#define BUTTON_IN				0x01
											#define BUTTON_OUT				0x02
											#define BUTTON_BLACK_BORDER		0x04
	unsigned							m_ToggleState;
											#define BUTTON_OFF				0
											#define BUTTON_ON				1
	unsigned							m_SemanticState;			// A button controls some activity with semantic implications for the software application.
											#define BUTTON_UNTOUCHED		0		// The button's activity has not been addressed, even by implication from other related buttons.
											#define BUTTON_TOUCHED			1		// The button's activity has been addressed, either directly or by implication
																					//  from other related buttons.
											#define BUTTON_COMPLETED		2		// The button's activity has been successfully completed.

	bool								m_EngageSpecialState;

// Operations
private:
	void			DrawFrame( CDC *pDC, CRect rc, int state );
	void			DrawFilledRect( CDC *pDC, CRect rc, COLORREF color );
	void			DrawLine( CDC *pDC, long sx, long sy, long ex, long ey, int nWidth, COLORREF color );
	void			DrawButtonText( CDC *pDC, CRect rc, char *pCaption, COLORREF textcolor );
	
public:
	void			HasBeenPressed( BOOL bHasBeenPressed );
	void			RecomputePressedColor();
	void			SetCheckBoxColor();
	void			TurnOnSpecialState();
	void			TurnOffSpecialState();
	void			Reinitialize();
	void			Draw( CDC *pDC );

// Implementation
public:
	virtual				~TomButton();

// Overrides
	//{{AFX_VIRTUAL( TomButton )
	//}}AFX_VIRTUAL

	// Generated message map functions
	DECLARE_MESSAGE_MAP()
public:
	//{{AFX_MSG( TomButton )
	afx_msg void		OnLButtonDown( UINT nFlags, CPoint Point );
	afx_msg void		OnLButtonUp( UINT nFlags, CPoint Point );
	afx_msg void		OnChar( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg void		OnPaint();
	afx_msg UINT		OnGetDlgCode();
	//}}AFX_MSG

};

/////////////////////////////////////////////////////////////////////////////
