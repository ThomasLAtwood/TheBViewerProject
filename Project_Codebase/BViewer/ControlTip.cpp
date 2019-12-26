// ControlTip.cpp : Implementation file for the CControlTip class, which
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

#include "StdAfx.h"
#include "BViewer.h"
#include "ControlTip.h"


extern CString			ControlTipWindowClass;


// IMPLEMENT_DYNAMIC( CControlTip, CWnd )


CControlTip::CControlTip()
{
	m_pTipText = 0;
	m_LastKnownMouseLocation.x = 0;
	m_LastKnownMouseLocation.y = 0;
}


CControlTip::~CControlTip()
{
	ShowWindow( SW_HIDE );
	m_TextFont.DeleteObject();
	DestroyWindow();
}


BEGIN_MESSAGE_MAP( CControlTip, CWnd )
	//{{AFX_MSG_MAP( CControlTip )
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CControlTip::CreateSpecifiedFont() 
{
	BOOL			Ok;
	
	Ok = ( m_TextFont.CreateFont(
			-14,						// nHeight in device units.
			0,							// nWidth - use available aspect ratio
			0,							// nEscapement - make character lines horizontal
			0,							// nOrientation - individual chars are horizontal
			FW_NORMAL,					// nWeight - character stroke thickness
			FALSE,						// bItalic - not italic
			FALSE,						// bUnderline - not underlined
			0,							// cStrikeOut - not a strikeout font
			DEFAULT_CHARSET,			// nCharSet - normal ansi characters
			0,							// nOutPrecision - choose font type using default search
			0,							// nClipPrecision - use default clipping
			0,							// nQuality - best possible appearance
			0,							// nPitchAndFamily - fixed or variable pitch
			"Tahoma"					// lpszFacename
			) != 0 );

	return Ok;
}


void CControlTip::ActivateTips()
{
	CreateEx( WS_EX_TOOLWINDOW, (const char*)ControlTipWindowClass, 0,
				WS_POPUP | WS_BORDER | WS_VISIBLE, CRect( 0, 0, 0, 0 ), 0, 0, 0 );

	CreateSpecifiedFont();
	SetFont( &m_TextFont, FALSE );
}


void CControlTip::SetWindowDimensions( CPoint ControlTipPosition )
{
	CClientDC			DeviceContext( this );
	CFont*				pOldFont;
	CRect				WindowRectangle;

	if ( m_pTipText != 0 && strlen( m_pTipText ) > 0 )
		{
		WindowRectangle.left = ControlTipPosition.x;
		WindowRectangle.top = ControlTipPosition.y;

		pOldFont = DeviceContext.SelectObject( &m_TextFont );

		DeviceContext.DrawText( m_pTipText, (int)strlen( m_pTipText ), &WindowRectangle, DT_CALCRECT );
		WindowRectangle.InflateRect( CSize( 5, 3 ) );
		SetWindowPos( 0, WindowRectangle.left, WindowRectangle.top,
						WindowRectangle.Width(), WindowRectangle.Height(), SWP_NOACTIVATE | SWP_NOZORDER );

		DeviceContext.SelectObject( pOldFont );
		}
}


void CControlTip::OnPaint()
{
	CPaintDC			DeviceContext( this );
	CRect				WindowRectangle;
	CFont				*pOldFont;

	if ( m_pTipText != 0 && strlen( m_pTipText ) > 0 )
		{
		GetClientRect( &WindowRectangle );
		DeviceContext.FillSolidRect( WindowRectangle, COLOR_CONFIG );
		DeviceContext.SetBkMode( TRANSPARENT );
		DeviceContext.SetTextColor( COLOR_BLACK );

		pOldFont = DeviceContext.SelectObject( &m_TextFont );
		
		if ( strchr( m_pTipText, '\n' ) == NULL )
			DeviceContext.DrawText( m_pTipText, (int)strlen( m_pTipText ), &WindowRectangle, DT_CENTER | DT_SINGLELINE | DT_VCENTER );
		else
			DeviceContext.DrawText( m_pTipText, (int)strlen( m_pTipText ), &WindowRectangle, DT_CENTER );

		DeviceContext.SelectObject( pOldFont );
		}
}


void CControlTip::ShowTipText( CPoint MouseCursorLocation, CWnd *pParentDialogWnd )
{
	if ( m_pTipText != 0 && strlen( m_pTipText ) > 0 )
		{
		// Update the tip window only if it has moved significantly.
		if ( abs( MouseCursorLocation.x - m_LastKnownMouseLocation.x ) >= 2 ||
					abs( MouseCursorLocation.y - m_LastKnownMouseLocation.y ) >= 2 )
			{
			CPoint			ControlTipPosition( MouseCursorLocation.x, MouseCursorLocation.y + 20 );

			m_LastKnownMouseLocation = MouseCursorLocation;
			ShowWindow( SW_HIDE );
			pParentDialogWnd -> ClientToScreen( &ControlTipPosition );
     		SetWindowDimensions( ControlTipPosition );
     		SetWindowText( m_pTipText );
			ShowWindow( SW_SHOWNA );
			}
		}
	else
		ShowWindow( SW_HIDE );
}


