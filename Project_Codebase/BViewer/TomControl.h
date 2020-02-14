// TomControl.h : Header file defining the structure of the TomControl base class of
//	all user interface control classes.
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

#include "Module.h"

typedef 	void (*CONTROL_TIP_ACTIVATION_FUNCTION)( CWnd *pParentWindow, char *pTipText, CPoint MouseCursorLocation );


#define COLOR_UNTOUCHED				0x006666aa
#define COLOR_TOUCHED				0x00444444


// TomControl window
class TomControl : public CWnd
{
DECLARE_DYNAMIC( TomControl )

// Construction
public:
	TomControl( char *pControlText, int ControlWidth, int ControlHeight, int FontHeight, int FontWidth, int FontWeight,
				COLORREF TextColor, COLORREF BackgroundColor, COLORREF ActivatedBkgdColor, DWORD ControlStyle, UINT nID, char *pControlTipText = 0 );

// Attributes
public:
	char								*m_ControlText;
	int									m_ControlWidth;
	int									m_ControlHeight;
	int									m_FontHeight;
	int									m_FontWidth;
	int									m_FontWeight;
	COLORREF							m_TextColor;
	COLORREF							m_OriginalIdleBkgColor;
	COLORREF							m_IdleBkgColor;
	COLORREF							m_ActivatedBkgdColor;
	DWORD								m_ControlStyle;
											#define CONTROL_TEXT_LEFT_JUSTIFIED				0x00000001
											#define CONTROL_TEXT_RIGHT_JUSTIFIED			0x00000002
											#define CONTROL_TEXT_HORIZONTALLY_CENTERED		0x00000004
											#define CONTROL_TEXT_TOP_JUSTIFIED				0x00000008
											#define CONTROL_TEXT_BOTTOM_JUSTIFIED			0x00000010
											#define CONTROL_TEXT_VERTICALLY_CENTERED		0x00000020
											#define CONTROL_VISIBLE							0x00000040
											#define CONTROL_INVISIBLE						0x00000080
											#define CONTROL_MULTILINE						0x00000100
											#define CONTROL_CLIP							0x00000200
											#define CONTROL_BACKGROUND_TRANSPARENT			0x00000400
											#define CONTROL_BITMAP							0x00000800
											#define CONTROL_HISTOGRAM						0x00001000
	UINT								m_nObjectID;
	UINT								m_BitmapID;
	HISTOGRAM_DATA						*m_pHistogramData;
	CFont								m_TextFont;
	char								*m_pControlTipText;
	CONTROL_TIP_ACTIVATION_FUNCTION		m_ControlTipActivator;
	BOOL								m_bMouseIsOverMe;
	BOOL								m_bHasBeenCompleted;

// Operations
public:
	virtual BOOL			CreateSpecifiedFont();
	virtual BOOL			SetPosition( int x, int y, CWnd *pParentWnd );
	virtual void			ChangeStatus( DWORD ClearStatus, DWORD SetStatus );
	virtual void			HasBeenCompleted( BOOL bHasBeenCompleted );
	virtual BOOL			IsCompleted();
	virtual BOOL			IsVisible();
			BOOL			IsStatic();

// Overrides
	//{{AFX_VIRTUAL(TomControl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual			~TomControl();

	DECLARE_MESSAGE_MAP()

	// Generated message map functions
protected:
	//{{AFX_MSG(TomControl)
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
 	//}}AFX_MSG

};

