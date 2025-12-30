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
// UPDATE HISTORY:
//
//	*[1] 11/24/2025 by Tom Atwood
//		Added support for display resolution scaling.
//
//
#pragma once

#include "TomButton.h"
#include "TomEdit.h"


// TextWindow
class CTextWindow : public CDialog		// *[1] Switched to CDialog from CWnd.
{
public:
	CTextWindow(  CWnd *pParent /*=NULL*/, unsigned short TextWindowType, double ActiveDisplayScaleFactor = 1.0 );			// *[1]
	virtual ~CTextWindow();

// Dialog Data
	enum { IDD = IDD_DIALOG_TEXT_WINDOW };										// *[1]

	TomEdit					m_EditControl;
	TomButton				m_ButtonTextWindowOK;

	unsigned short			m_TextWindowType;									// *[1]
		#define					TEXT_WINDOW_ABOUT_BOX					1		// *[1]
		#define					TEXT_WINDOW_TECHNICAL_REQUIREMENTS		2		// *[1]
	char					*m_pTextForDisplay;
	CBrush					m_BkgdBrush;
	double					m_ActiveDisplayScaleFactor;							// *[1]

public:
//	BOOL				SetPosition( int x, int y, CWnd *pParentWnd, CString WindowClass );		// *[1]
	BOOL				ReadTextFileForDisplay( char *pFullTextFileSpecification );

protected:
// Overrides
	//{{AFX_VIRTUAL(CTextWindow)
	virtual BOOL		OnInitDialog();											// *[1]
	//}}AFX_VIRTUAL

	DECLARE_MESSAGE_MAP()

	//{{AFX_MSG( CTextWindow )
	afx_msg void		OnBnClickedTextWindowOK( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH		OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor );
	afx_msg BOOL		OnEraseBkgnd( CDC *pDC );
	//}}AFX_MSG
};


