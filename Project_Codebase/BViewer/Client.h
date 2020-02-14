// Client.h - Defines the data structures and functions for the client information
//  dialog box.  This provides the client name and address for the report.
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
#include "TomStatic.h"
#include "TomButton.h"
#include "TomEdit.h"

#define CLIENT_ERROR_INSUFFICIENT_MEMORY		1
#define CLIENT_ERROR_PATH_NOT_FOUND				2
#define CLIENT_ERROR_FILE_OPEN_FOR_READ			3
#define CLIENT_ERROR_FILE_READ					4
#define CLIENT_ERROR_FILE_OPEN_FOR_WRITE		5
#define CLIENT_ERROR_PARSE_UNKNOWN_ATTR			6

#define CLIENT_ERROR_DICT_LENGTH				6



// CClient dialog

class CClient : public CDialog
{
public:
	CClient( CWnd *pParent = NULL, CLIENT_INFO *pClientInfo = NULL );   // standard constructor
	virtual ~CClient();


// Dialog Data
	enum { IDD = IDD_DIALOG_CLIENT };
	CBrush				m_BkgdBrush;

	TomStatic			m_StaticClientIdentification;
	TomStatic			m_StaticClientHelpInfo;
	TomStatic			m_StaticClientName;
	TomEdit				m_EditClientName;

	TomStatic			m_StaticClientStreetAddress;
	TomEdit				m_EditClientStreetAddress;

	TomStatic			m_StaticClientCity;
	TomEdit				m_EditClientCity;

	TomStatic			m_StaticClientState;
	TomEdit				m_EditClientState;

	TomStatic			m_StaticClientZipCode;
	TomEdit				m_EditClientZipCode;

	TomStatic			m_StaticClientPhone;
	TomEdit				m_EditClientPhone;

	TomStatic			m_StaticClientOtherContactInfo;
	TomEdit				m_EditClientOtherContactInfo;

	TomButton			m_ButtonSave;
	TomButton			m_ButtonDelete;
	TomButton			m_ButtonCancel;

	TomGroup			m_GroupEditSequencing;

	BOOL				m_bAddingNewClient;
	CLIENT_INFO			m_ClientInfo;

protected:
// Overrides
	//{{AFX_VIRTUAL(CClient)
	//}}AFX_VIRTUAL


	DECLARE_MESSAGE_MAP()

protected:
	//{{AFX_MSG(CClient)
	afx_msg void		OnBnClickedSaveClientInfo( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedDeleteClientInfo( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedCancelClientInfo( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH		OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor );
	afx_msg BOOL		OnEraseBkgnd( CDC *pDC );
	virtual BOOL		OnInitDialog();
	afx_msg void		OnClose();
	//}}AFX_MSG

	void				SetClientFileSpecification( char *pClientInfoFileSpec );

public:
	BOOL				WriteClientFile();
};




// Function prototypes:
//
	void				InitClientModule();
	void				CloseClientModule();
	BOOL				ReadAllClientFiles();
	void				EraseClientList();

