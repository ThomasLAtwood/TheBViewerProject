// PopupDialog.cpp : Implementation file for the general-purpose popup dialog
//  box class.
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
//	*[1] 08/05/2025 by Tom Atwood
//		Added scaling of window width, height depending on display resolution.
//		Added semi-automatic resizing of the dialog box.
//
//
//
#include "stdafx.h"
#include "BViewer.h"
#include "PopupDialog.h"


// CPopupDialog
// *[1] Added ActiveDisplayScaleFactor distribution to all daughter windows to support display scaling.
CPopupDialog::CPopupDialog( int DialogWidth, int DialogHeight, COLORREF BackgroundColor, DWORD WindowStyle, UINT nID, double ActiveDisplayScaleFactor ):		// *[1]
				m_StaticUserMessage( "", 340, 120, 24, 12, 6, ActiveDisplayScaleFactor, 0x00000000, COLOR_CONFIG, COLOR_CONFIG,
										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_TOP_JUSTIFIED | CONTROL_MULTILINE | CONTROL_CLIP | CONTROL_VISIBLE,
//										CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_MULTILINE | CONTROL_CLIP | CONTROL_VISIBLE,
										IDC_STATIC_MESSAGE_FOR_USER ),
				m_EditUserTextInput( "", 300, 20, 16, 8, 6, VARIABLE_PITCH_FONT, ActiveDisplayScaleFactor, COLOR_BLACK, COLOR_CONFIG, COLOR_CONFIG, COLOR_CONFIG,
								CONTROL_TEXT_LEFT_JUSTIFIED | CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_CLIP | EDIT_BORDER | CONTROL_VISIBLE,
								EDIT_VALIDATION_NONE, IDC_EDIT_USER_TEXTUAL_INPUT ),
				m_ButtonPopupSave( "Update", 100, 30, 14, 7, 6, ActiveDisplayScaleFactor,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_POPUP_SAVE ),
				m_ButtonPopupOK( "OK", 100, 30, 14, 7, 6, ActiveDisplayScaleFactor,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_POPUP_OK ),
				m_ButtonPopupYes( "Yes", 100, 30, 14, 7, 6, ActiveDisplayScaleFactor,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_POPUP_YES ),
				m_ButtonPopupNo( "No", 100, 30, 14, 7, 6, ActiveDisplayScaleFactor,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_POPUP_NO ),
				m_ButtonSuspendUpdates( "Suspend Error\nReports", 150, 40, 14, 7, 6, ActiveDisplayScaleFactor,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED | CONTROL_MULTILINE |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_POPUP_SUSPEND ),
				m_ButtonPopupCancel( "Cancel", 100, 30, 14, 7, 6, ActiveDisplayScaleFactor,
									COLOR_WHITE, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR, COLOR_PATIENT_SELECTOR,
									BUTTON_PUSHBUTTON | CONTROL_TEXT_HORIZONTALLY_CENTERED |
									CONTROL_TEXT_VERTICALLY_CENTERED | CONTROL_VISIBLE, IDC_BUTTON_POPUP_CANCEL )
{
	m_DialogWidth = DialogWidth;
	m_DialogHeight = DialogHeight;
	m_BackgroundColor = BackgroundColor;
	m_WindowStyle = WindowStyle;
	m_nObjectID = nID;
	m_bExternalCancelActivated = FALSE;
	m_BkgdBrush.CreateSolidBrush( COLOR_CONFIG );
	m_ActiveDisplayScaleFactor = ActiveDisplayScaleFactor;			// *[1]
}


CPopupDialog::~CPopupDialog()
{
	DestroyWindow();
}


// *[1] Added this function for use in estimating the vertical layout of the PopupDialog window.
static int CountTextLines( char *pTextString )
{
	int				nChar;
	int				nChars;
	int				nTextLines;

	nTextLines = 1;
	nChars = strlen( pTextString );
	for ( nChar = 0; nChar < nChars; nChar++ )
		if ( pTextString[ nChar ] == '\n' )
			nTextLines++;

	return nTextLines;
}


// Caution:		Since this function creates the object, it must be called before any
//				functions such as Invalidate(), etc., that manipulate an active window.
//
// *[1]			Major revision to support window scaling and automatic dialog layout
//				and sizing.  Automatic layout eliminates the need to format each
//				notification separately, making the message formatting appear
//				uniform.
//
// NOTE:	It is important to keep track of which object members have already been
//			scaled before the call to this function occurs.
BOOL CPopupDialog::SetPosition( int x, int y, CWnd *pParentWnd, CString WindowClass )
{
	BOOL			bResult;
	int				PrimaryScreenWidth;			// *[1]
	int				PrimaryScreenHeight;		// *[1]

	int				nStaticTextLines;			// *[1]
	CRect			DialogRect;
	DWORD			WindowsStyle;
	int				ScaledDialogWidth;			// *[1]
	int				ScaledDialogHeight;			// *[1]
	int				UnScaledMessageWidth;		// *[1]
	int				UnScaledMessageHeight;		// *[1]
	int				BottomOfMessage;			// *[1]

	// Ignore the dialog box position specifications and position it at the center of the screen.
	PrimaryScreenWidth = ::GetSystemMetrics( SM_CXSCREEN );					// *[1]
	PrimaryScreenHeight = ::GetSystemMetrics( SM_CYSCREEN );				// *[1]
	// Examine the text of the static message control.
	m_StaticUserMessage.m_ControlText = m_pUserNotificationInfo -> pUserNotificationMessage;	// *[1]
	nStaticTextLines = CountTextLines( m_StaticUserMessage.m_ControlText );	// *[1]
	if ( m_pUserNotificationInfo -> FontHeight == 0 )						// *[1]
		m_pUserNotificationInfo -> FontHeight = 18;							// *[1]
	UnScaledMessageHeight = nStaticTextLines * (int)( 0.9 * (double)m_StaticUserMessage.m_FontHeight / m_ActiveDisplayScaleFactor );	// *[1]
	m_StaticUserMessage.m_FontHeight = (int)( (double)m_pUserNotificationInfo -> FontHeight * m_ActiveDisplayScaleFactor );				// *[1]
	if ( m_pUserNotificationInfo -> FontWidth == 0 )						// *[1]
		m_pUserNotificationInfo -> FontWidth = 9;							// *[1]
	m_StaticUserMessage.m_FontWidth = (int)( (double)m_pUserNotificationInfo -> FontWidth * m_ActiveDisplayScaleFactor );				// *[1]
	UnScaledMessageWidth = m_pUserNotificationInfo -> WindowWidth - 60;		// *[1]
	m_StaticUserMessage.m_ControlWidth = (int)( (double)UnScaledMessageWidth * m_ActiveDisplayScaleFactor );							// *[1]
	m_StaticUserMessage.m_ControlHeight = (int)( (double)UnScaledMessageHeight * m_ActiveDisplayScaleFactor );							// *[1]

	BottomOfMessage = 30 + UnScaledMessageHeight;							// *[1]
	if ( ( m_pUserNotificationInfo -> UserInputType & USER_INPUT_TYPE_EDIT ) != 0 )								// *[1]
		ScaledDialogHeight = (int)( (double)( BottomOfMessage + 130 ) * m_ActiveDisplayScaleFactor );			// *[1]
	else if ( ( m_pUserNotificationInfo -> UserInputType & USER_INPUT_TYPE_BOOLEAN ) != 0 )						// *[1]
		ScaledDialogHeight = (int)( (double)( BottomOfMessage + 90 ) * m_ActiveDisplayScaleFactor );			// *[1]
	else if ( ( m_pUserNotificationInfo -> UserInputType & USER_INPUT_TYPE_BOOLEAN_NO_CANCEL ) != 0 )			// *[1]
		ScaledDialogHeight = (int)( (double)( BottomOfMessage + 90 ) * m_ActiveDisplayScaleFactor );			// *[1]
	else if ( ( m_pUserNotificationInfo -> UserInputType & USER_INPUT_TYPE_OK ) != 0 )							// *[1]
		{
		if ( ( m_pUserNotificationInfo -> UserInputType & USER_INPUT_TYPE_SUSPEND ) != 0 )						// *[1]
			ScaledDialogHeight = (int)( (double)( BottomOfMessage + 90 ) * m_ActiveDisplayScaleFactor );		// *[1]
		else
			ScaledDialogHeight = (int)( (double)( 50 + BottomOfMessage + 40 ) * m_ActiveDisplayScaleFactor );	// *[1]
		}

	WindowsStyle = DS_MODALFRAME | WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_EX_TOPMOST;
	ScaledDialogWidth = (int)( (double)m_DialogWidth * m_ActiveDisplayScaleFactor );		// *[1]
	x = ( PrimaryScreenWidth - ScaledDialogWidth ) / 2;										// *[1]
	y = ( PrimaryScreenHeight - ScaledDialogHeight ) / 2;									// *[1]
	DialogRect.SetRect( x, y, x + ScaledDialogWidth, y + ScaledDialogHeight );				// *[1]
	bResult = CreateEx( WS_EX_DLGMODALFRAME, (const char*)WindowClass, "Tell Me", WindowsStyle, DialogRect, pParentWnd, 0, NULL );

	if ( ( m_pUserNotificationInfo -> UserInputType & USER_INPUT_TYPE_EDIT ) != 0 )
		{
		m_ButtonPopupSave.SetPosition( 50, BottomOfMessage + 50, this );					// *[1]
		m_ButtonPopupCancel.SetPosition( m_DialogWidth - 160, BottomOfMessage + 50, this );	// *[1]
		m_EditUserTextInput.m_EditWidth = (int)( (double)( m_DialogWidth - 80 )  * m_ActiveDisplayScaleFactor );	// *[1]
		m_EditUserTextInput.SetPosition( 30, BottomOfMessage + 10, this );					// *[1]
		m_EditUserTextInput.SetWindowText( m_pUserNotificationInfo -> UserTextResponse );
		m_EditUserTextInput.SetFocus();
		}
	else if ( ( m_pUserNotificationInfo -> UserInputType & USER_INPUT_TYPE_BOOLEAN ) != 0 )
		{
		m_ButtonPopupYes.SetPosition( ( m_DialogWidth - 340 ) / 2, BottomOfMessage + 10, this );			// *[1]
		m_ButtonPopupNo.SetPosition( 120 + ( m_DialogWidth - 340 ) / 2, BottomOfMessage + 10, this );		// *[1]
		m_ButtonPopupCancel.SetPosition( 240 + ( m_DialogWidth - 340 ) / 2, BottomOfMessage + 10, this );	// *[1]
		}
	else if ( ( m_pUserNotificationInfo -> UserInputType & USER_INPUT_TYPE_BOOLEAN_NO_CANCEL ) != 0 )
		{
		m_ButtonPopupYes.SetPosition( 50, BottomOfMessage, this );											// *[1]
		m_ButtonPopupNo.SetPosition( m_DialogWidth - 150, BottomOfMessage, this );							// *[1]
		}
	else if ( ( m_pUserNotificationInfo -> UserInputType & USER_INPUT_TYPE_OK ) != 0 )
		{
		if ( ( m_pUserNotificationInfo -> UserInputType & USER_INPUT_TYPE_SUSPEND ) != 0 )
			{
			m_ButtonSuspendUpdates.SetPosition(  m_DialogWidth / 2 - 200, BottomOfMessage, this );			// *[1]
			m_ButtonPopupOK.SetPosition( m_DialogWidth / 2 + 50, BottomOfMessage, this );					// *[1]
			}
		else
			m_ButtonPopupOK.SetPosition( ( m_DialogWidth - 100 ) / 2, BottomOfMessage + 10, this );			// *[1]
		}
	m_StaticUserMessage.SetPosition( ( m_DialogWidth - UnScaledMessageWidth ) / 2,  20, this );				// *[1]
	UpdateWindow();

	return bResult;
}


BEGIN_MESSAGE_MAP( CPopupDialog, CWnd )
	//{{AFX_MSG_MAP(CPopupDialog)
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_POPUP_SAVE, OnBnClickedPopupSave )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_POPUP_OK, OnBnClickedPopupOK )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_POPUP_YES, OnBnClickedPopupYes )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_POPUP_NO, OnBnClickedPopupNo )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_POPUP_SUSPEND, OnBnClickedPopupSuspend )
	ON_NOTIFY( WM_LBUTTONUP, IDC_BUTTON_POPUP_CANCEL, OnBnClickedPopupCancel )
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CPopupDialog::OnBnClickedPopupSave( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( m_pUserNotificationInfo != 0 )
		{
		m_pUserNotificationInfo -> UserResponse = POPUP_RESPONSE_SAVE;
		if ( m_pUserNotificationInfo -> CallbackFunction != 0 )
			( *m_pUserNotificationInfo -> CallbackFunction )( (void*)this );
		}

	*pResult = 0;
}


void CPopupDialog::OnBnClickedPopupOK( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( m_pUserNotificationInfo != 0 )
		{
		m_pUserNotificationInfo -> UserResponse = POPUP_RESPONSE_OK;
		if ( m_pUserNotificationInfo -> CallbackFunction != 0 )
			( *m_pUserNotificationInfo -> CallbackFunction )( (void*)this );
		}

	*pResult = 0;
}


void CPopupDialog::OnBnClickedPopupYes( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( m_pUserNotificationInfo != 0 )
		{
		m_pUserNotificationInfo -> UserResponse = POPUP_RESPONSE_YES;
		if ( m_pUserNotificationInfo -> CallbackFunction != 0 )
			( *m_pUserNotificationInfo -> CallbackFunction )( (void*)this );
		}

	*pResult = 0;
}


void CPopupDialog::OnBnClickedPopupNo( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( m_pUserNotificationInfo != 0 )
		{
		m_pUserNotificationInfo -> UserResponse = POPUP_RESPONSE_NO;
		if ( m_pUserNotificationInfo -> CallbackFunction != 0 )
			( *(m_pUserNotificationInfo -> CallbackFunction) )( (void*)this );
		}

	*pResult = 0;
}


void CPopupDialog::OnBnClickedPopupSuspend( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( m_pUserNotificationInfo != 0 )
		{
		m_pUserNotificationInfo -> UserResponse = POPUP_RESPONSE_SUSPEND;
		if ( m_pUserNotificationInfo -> CallbackFunction != 0 )
			( *(m_pUserNotificationInfo -> CallbackFunction) )( (void*)this );
		}

	*pResult = 0;
}


void CPopupDialog::OnBnClickedPopupCancel( NMHDR *pNMHDR, LRESULT *pResult )
{
	if ( m_pUserNotificationInfo != 0 )
		{
		m_pUserNotificationInfo -> UserResponse = POPUP_RESPONSE_CANCEL;
		if ( m_pUserNotificationInfo -> CallbackFunction != 0 )
			( *m_pUserNotificationInfo -> CallbackFunction )( (void*)this );
		}

	*pResult = 0;
}


BOOL CPopupDialog::OnEraseBkgnd( CDC *pDC )
{
	CBrush		BackgroundBrush( m_BackgroundColor );
	CRect		BackgroundRectangle;
	CBrush		*pOldBrush = pDC -> SelectObject( &BackgroundBrush );

	GetClientRect( BackgroundRectangle );
	pDC -> FillRect( BackgroundRectangle, &BackgroundBrush );
	pDC -> SelectObject( pOldBrush );

	return TRUE;
}


void CPopupDialog::CancelDialog()
{
	m_bExternalCancelActivated = TRUE;
	Sleep( 1000 );
}


BOOL CPopupDialog::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	LRESULT				Result;
	
	if ( m_bExternalCancelActivated )
		{
		m_bExternalCancelActivated = FALSE;
		OnBnClickedPopupNo( 0, &Result );
		}

	return CWnd::OnWndMsg( message, wParam, lParam, pResult );
}


HBRUSH CPopupDialog::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
{
	HBRUSH			hBrush;

	if ( nCtlColor == CTLCOLOR_EDIT )
		{
		pDC -> SetBkColor( ( (TomEdit*)pWnd ) -> m_IdleBkgColor );
		pDC -> SetTextColor( ( (TomEdit*)pWnd ) -> m_TextColor );
		pDC -> SetBkMode( OPAQUE );
		hBrush = HBRUSH( *( (TomEdit*)pWnd ) -> m_pCurrentBkgdBrush );
		}
	else
		hBrush = HBRUSH( m_BkgdBrush );

	return hBrush;
}
