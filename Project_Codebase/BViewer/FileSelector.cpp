// FileSelector.cpp : implementation file
//

#include "stdafx.h"
#include "BViewer.h"
#include "Module.h"
#include "ReportStatus.h"
#include "Configuration.h"
#include "Access.h"
#include "DiagnosticImage.h"
#include "Mouse.h"
#include "ImageView.h"
#include "MainFrm.h"
#include "ImageFrame.h"
#include "FileSelector.h"


extern CBViewerApp				ThisBViewerApp;
extern CONFIGURATION			BViewerConfiguration;
extern CCustomization			BViewerCustomization;


// NOTE:  No functionality has so far been added to CFileDialog.  The only purpose for this
// derived class is to provide a platform for future extensions, if and when they are needed.


// CFileSelector dialog

IMPLEMENT_DYNAMIC( CFileSelector, CFileDialog )

CFileSelector::CFileSelector( CWnd* pParent /*=NULL*/ )
			: CFileDialog( TRUE, NULL, NULL, OFN_NONETWORKBUTTON | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
							NULL, pParent, sizeof( OPENFILENAME ) )
{
}


CFileSelector::~CFileSelector()
{
	DestroyWindow();
}


void CFileSelector::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP( CFileSelector, CFileDialog )
	ON_WM_CLOSE()
END_MESSAGE_MAP()


BOOL CFileSelector::OnInitDialog()
{
	RECT			ClientRect;
	INT				ClientWidth;
	INT				ClientHeight;
	static char		TextString[ 64 ];

	CFileDialog::OnInitDialog();

	GetClientRect( &ClientRect );
	ClientWidth = ClientRect.right - ClientRect.left;
	ClientHeight = ClientRect.bottom - ClientRect.top;

	SetIcon( ThisBViewerApp.m_hApplicationIcon, FALSE );
	
	return TRUE; 
}


// CFileSelector message handlers


void CFileSelector::OnClose()
{
	CFileDialog::OnClose();
}


