// ControlTip.h : Header file defining the structure of the CControlTip class, which
//  implements a popup window that displays helpful information when the mouse passes
//  over the lower right part of a button or a static control.
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


class CControlTip : public CWnd
{
//	DECLARE_DYNAMIC( CControlTip );

public:
	CControlTip();
	virtual ~CControlTip();

	CFont				m_TextFont;
	char				*m_pTipText;
	CPoint				m_LastKnownMouseLocation;

public:
	void			ActivateTips();
	BOOL			CreateSpecifiedFont();
	void			SetWindowDimensions( CPoint ControlTipPosition );
	void			ShowTipText( CPoint MouseCursorLocation, CWnd *pParentDialogWnd );

	DECLARE_MESSAGE_MAP()

protected:
	// Generated message map functions
	//{{AFX_MSG(CControlTip)
	afx_msg void		OnPaint();
	//}}AFX_MSG

};
