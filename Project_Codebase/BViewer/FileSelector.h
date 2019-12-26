// CFileSelector.h
//

#pragma once

#include <afxdlgs.h>


// CFileSelector dialog

class CFileSelector : public CFileDialog
{
	DECLARE_DYNAMIC( CFileSelector )

public:
	CFileSelector( CWnd* pParent = NULL );   // standard constructor
	virtual ~CFileSelector();


// Dialog Data
	enum { IDD = IDD_DIALOG_FILE_SELECTOR };

protected:
	virtual void		DoDataExchange( CDataExchange* pDX );    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL		OnInitDialog();
	afx_msg void		OnClose();
};


