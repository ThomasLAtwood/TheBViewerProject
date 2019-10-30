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

#include "TomGroup.h"
/////////////////////////////////////////////////////////////////////////////

class TomButton : public CButton
{
// Construction
public:
	TomButton( char *pButtonText, int ButtonWidth, int ButtonHeight, int FontHeight, int FontWidth, int FontWeight,
				COLORREF TextColor, COLORREF BackgroundColor, COLORREF ActivatedBkgdColor, COLORREF VisitedBkgdColor,
				DWORD ButtonStyle, UINT nID );

// Attributes
	TomGroup		*m_pGroup;				// Point to the button group, if any, of which
											// this button is a member.
public:
	char			*m_ButtonText;
	int				m_ButtonWidth;
	int				m_ButtonHeight;
	int				m_FontHeight;
	int				m_FontWidth;
	int				m_FontWeight;
	COLORREF		m_TextColor;
	COLORREF		m_OriginalIdleBkgColor;
	COLORREF		m_ActivatedBkgdColor;
	COLORREF		m_VisitedBkgdColor;
	DWORD			m_ButtonStyle;
						#define BUTTON_TEXT_LEFT_JUSTIFIED				0x00000001
						#define BUTTON_TEXT_RIGHT_JUSTIFIED				0x00000002
						#define BUTTON_TEXT_HORIZONTALLY_CENTERED		0x00000004
						#define BUTTON_TEXT_TOP_JUSTIFIED				0x00000008
						#define BUTTON_TEXT_BOTTOM_JUSTIFIED			0x00000010
						#define BUTTON_TEXT_VERTICALLY_CENTERED			0x00000020
						#define BUTTON_VISIBLE							0x00000040
						#define BUTTON_INVISIBLE						0x00000080
						#define BUTTON_MULTILINE						0x00000100
						#define BUTTON_CLIP								0x00000200
						#define BUTTON_PUSHBUTTON						0x00010000
						#define BUTTON_CHECKBOX							0x00020000
						#define GROUP_STATIC							0x00040000		// Used to identify static groups.
						#define GROUP_EDIT								0x00080000		// Used to identify edit groups.
						#define BUTTON_FROZEN							0x00100000

	UINT			m_nObjectID;

	COLORREF		m_IdleBkgColor;
	COLORREF		m_PressedBkgColor;
	COLORREF		m_SpecialBkgColor;
	COLORREF		m_DisabledBkgndColor;
	COLORREF		m_Light;
	COLORREF		m_Highlight;
	COLORREF		m_Shadow;
	COLORREF		m_DarkShadow;

public:
	unsigned int	m_ButtonState;
						#define BUTTON_IN				0x01
						#define BUTTON_OUT				0x02
						#define BUTTON_BLACK_BORDER		0x04
	unsigned		m_ToggleState;
						#define BUTTON_OFF				0
						#define BUTTON_ON				1
	unsigned		m_SemanticState;							// A button controls some activity with semantic implications for the software application.
						#define BUTTON_UNTOUCHED		0		// The button's activity has not been addressed, even by implication from other related buttons.
						#define BUTTON_TOUCHED			1		// The button's activity has been addressed, either directly or by implication
																//  from other related buttons.
						#define BUTTON_COMPLETED		2		// The button's activity has been successfully completed.

	bool			m_EngageSpecialState;
	CFont			m_TextFont;

// Operations
private:
	bool			CreateSpecifiedFont();
	void			DrawFrame( CDC *pDC, CRect rc, int state );
	void			DrawFilledRect( CDC *pDC, CRect rc, COLORREF color );
	void			DrawLine( CDC *pDC, long sx, long sy, long ex, long ey, int nWidth, COLORREF color );
	void			DrawButtonText( CDC *pDC, CRect rc, char *pCaption, COLORREF textcolor );
	
public:
	void			ChangeStatus( DWORD ClearStatus, DWORD SetStatus );
	void			HasBeenPressed( BOOL bHasBeenPressed );
	void			RecomputePressedColor();
	void			SetCheckBoxColor();
	void			TurnOnSpecialState();
	void			TurnOffSpecialState();
	BOOL			SetPosition( int x, int y, CWnd* pParentWnd );
	void			Reinitialize();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TomButton)
	public:
		virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~TomButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(TomButton)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
