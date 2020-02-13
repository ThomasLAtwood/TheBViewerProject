// ServiceController.h : main header file for the ServiceController application.
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

#include "stdafx.h"

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include <WinSvc.h>


#define MAIN_ERROR_INSUFFICIENT_MEMORY			1
#define MAIN_ERROR_OPEN_CFG_FILE				2
#define MAIN_ERROR_READ_CFG_FILE				3

#define MAIN_ERROR_DICT_LENGTH					3


#define COLOR_MAIN_BKGD			0x00eebb88
#define COLOR_MAIN_FONT			0x00ffffff
#define COLOR_AUX_FONT			0x00ffff99

#define COLOR_BUTTON_FONT		0x00000000
#define COLOR_BUTTON_BKGD		0x00ffcc99

#define COLOR_LOG_FONT			0x00000000
#define COLOR_LOG_BKGD			0x00ffffff

#define COLOR_CANCEL			0x000000ff
#define COLOR_WHITE				0x00ffffff

// CServiceControllerApp:
// See ServiceController.cpp for the implementation of this class.
//
class CServiceControllerApp : public CWinApp
{
public:
	CServiceControllerApp();

	CString					m_MainWindowClassName;
	HICON					m_hApplicationIcon;
	unsigned long			m_CommandLineInstructions;
								#define CMD_LINE_INSTALL		0x01
								#define CMD_LINE_START			0x02
								#define CMD_LINE_RESTART		0x04
								#define CMD_LINE_STOP			0x08
								#define CMD_LINE_REMOVE			0x10
	char					m_ProgramPath[ FULL_FILE_SPEC_STRING_LENGTH ];

// Overrides
public:
	virtual BOOL		InitInstance();
	virtual int			ExitInstance();
	void				OnAppAbout();
	void				OnAppExit();

// Implementation

public:
	DECLARE_MESSAGE_MAP()
};




// Function prototypes.
//
void				InitMainModule();
void				CloseMainModule();

void				GetCmdLine();
void				Configure();

//____________________________________________________________________________________
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};



