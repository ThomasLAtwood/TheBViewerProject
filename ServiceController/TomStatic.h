// TomStatic.h : Header file defining the structure of the TomStatic class of static
//	text object controls for use in populating a user interface window.
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

/////////////////////////////////////////////////////////////////////////////
// TomStatic window

class TomStatic : public CWnd
{
// Construction
public:
	TomStatic( char *pStaticText, int StaticWidth, int StaticHeight, int FontHeight, int FontWidth, int FontWeight,
				COLORREF TextColor, COLORREF BackgroundColor, COLORREF ActivatedBkgdColor, DWORD StaticStyle, UINT nID );

	TomStatic( UINT StaticBitmapID, int StaticBitmapWidth, int StaticBitmapHeight, DWORD StaticStyle, UINT nID );


// Attributes
public:
	char			*m_StaticText;
	int				m_StaticWidth;
	int				m_StaticHeight;
	int				m_FontHeight;
	int				m_FontWidth;
	int				m_FontWeight;
	COLORREF		m_TextColor;
	COLORREF		m_OriginalIdleBkgColor;
	COLORREF		m_IdleBkgColor;
	COLORREF		m_ActivatedBkgdColor;
	DWORD			m_StaticStyle;
						#define STATIC_TEXT_LEFT_JUSTIFIED				0x00000001
						#define STATIC_TEXT_RIGHT_JUSTIFIED				0x00000002
						#define STATIC_TEXT_HORIZONTALLY_CENTERED		0x00000004
						#define STATIC_TEXT_TOP_JUSTIFIED				0x00000008
						#define STATIC_TEXT_BOTTOM_JUSTIFIED			0x00000010
						#define STATIC_TEXT_VERTICALLY_CENTERED			0x00000020
						#define STATIC_VISIBLE							0x00000040
						#define STATIC_INVISIBLE						0x00000080
						#define STATIC_MULTILINE						0x00000100
						#define STATIC_CLIP								0x00000200
						#define STATIC_BACKGROUND_TRANSPARENT			0x00002000
						#define STATIC_BITMAP							0x00004000
	UINT			m_nObjectID;
	UINT			m_BitmapID;
	CFont			m_TextFont;

// Operations
public:
	bool			CreateSpecifiedFont();
	BOOL			SetPosition( int x, int y, CWnd* pParentWnd );
	void			ChangeStatus( DWORD ClearStatus, DWORD SetStatus );
	void			HasBeenCompleted( BOOL bHasBeenCompleted );
	BOOL			IsCompleted();

// Implementation
public:
	virtual			~TomStatic();

	// Generated message map functions
protected:
	//{{AFX_MSG(TomStatic)
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
public:
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
