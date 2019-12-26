// TextWindow.h : Header file defining the structure of the CTextWindow class, which
//  implements a window that displays an arbitrary buffer of text.
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

#include "TomButton.h"
#include "TomEdit.h"


// TextWindow
class CTextWindow : public CWnd
{
public:
	CTextWindow();
	virtual ~CTextWindow();

	TomEdit					m_EditControl;
	TomButton				m_ButtonTextWindowOK;

	char					*m_pTextForDisplay;
	CBrush					m_BkgdBrush;

public:
	BOOL				SetPosition( int x, int y, CWnd *pParentWnd, CString WindowClass );
	BOOL				ReadTextFileForDisplay( char *pFullTextFileSpecification );

protected:
// Overrides
	//{{AFX_VIRTUAL(CTextWindow)
	//}}AFX_VIRTUAL

	DECLARE_MESSAGE_MAP()

	//{{AFX_VIRTUAL( CTextWindow )
	afx_msg void		OnBnClickedTextWindowOK( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH		OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor );
	afx_msg BOOL		OnEraseBkgnd( CDC *pDC );
	//}}AFX_VIRTUAL
};


