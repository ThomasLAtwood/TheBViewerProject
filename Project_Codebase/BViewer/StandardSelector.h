// StandardSelector.h : Header file defining the structure of the CStandardSelector
//  class, which implements the reference image selection for the standards
//  installation process.
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

#include "Dicom.h"
#include "TomStatic.h"


#define INSTALL_ERROR_INSUFFICIENT_MEMORY		1
#define INSTALL_ERROR_FILE_COPY					2
#define INSTALL_ERROR_FILE_MOVE					3
#define INSTALL_ERROR_DICOMDIR_READ				4
#define INSTALL_ERROR_WAITING_FOR_MEDIA			5

#define INSTALL_ERROR_DICT_LENGTH				5



// CStandardSelector
class CStandardSelector : public CWnd
{
public:
	CStandardSelector( int DialogWidth, int DialogHeight, COLORREF BackgroundColor, DWORD WindowStyle );
	virtual ~CStandardSelector();

// Attributes
public:
	int								m_DialogWidth;
	int								m_DialogHeight;
	int								m_Column1XOffset;
	int								m_Row1YOffset;
	int								m_Column2XOffset;
	COLORREF						m_BackgroundColor;
	DWORD							m_WindowStyle;
	BOOL							m_bStandardsVolumeFound;

	TomStatic						m_StaticUserMessage;
	TomButton						m_ButtonInstallFromSelectedDrive;

	TomStatic						m_StaticStep1;
	TomStatic						m_StaticStep1Text;
	TomStatic						m_StaticStep1Status;
	TomStatic						m_StaticStep2;
	TomStatic						m_StaticStep2Text;
	TomStatic						m_StaticStep2Status;
	TomStatic						m_StaticStep3;
	TomStatic						m_StaticStep3Text;
	TomStatic						m_StaticStep3Status;
	TomStatic						m_StaticStep4;
	TomStatic						m_StaticStep4Text;
	TomStatic						m_StaticStep4Status;

	TomButton						m_ButtonInstallSelectedStandard;
	TomButton						m_ButtonAutoInstall;
	TomButton						m_ButtonInstall;
	TomButton						m_ButtonInstallCancel;
	
	CTreeCtrl						*m_pExplorer;
	CImageList						m_FolderIcons;
	CBitmap							m_DriveBitmap;
	CBitmap							m_FolderBitmap;
	CBitmap							m_FolderOpenBitmap;
	CBitmap							m_DicomImageBitmap;

	char							m_SelectedDriveSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	char							m_SelectedFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	BOOL							m_bSelectionIsAFolder;
	BOOL							m_bSelectionIsADICOMDIR;
	IMAGE_FILE_SET_SPECIFICATION	*m_pListOfFileSetItems;
	HTREEITEM						m_SelectedItem;
	unsigned long					m_nFilesCopiedFromMedia;
	unsigned long					m_nFilesEncodedAsPNG;
	unsigned long					m_nFilesReadyForUse;
	unsigned long					m_TotalImageFilesInstalled;

// Method prototypes:
//
public:
	void				CheckWindowsMessages();
	void				ListStorageDevices();
	void				InstallFromDICOMDIRFile();
	BOOL				SetPosition( int x, int y, CWnd *pParentWnd, CString WindowClass );
	BOOL				CopyDesignatedFile( char *pSourceImageFileSpec );
	BOOL				CopyDirectoryContents( char *pSourceDirectorySpec );
	void				EraseFileSpecList( IMAGE_FILE_SET_SPECIFICATION **ppFileSpecList );
	BOOL				ReadDicomDirectoryFile( char *pDicomdirFileSpec );
	BOOL				SearchForDICOMDIRFiles( char *pSourceDirectorySpec );
	void				DeleteChildItems( HTREEITEM hParentItem );
	void				ExpandSelection( HTREEITEM hNewlySelectedItem );
	BOOL				LoadReferenceStandards( BOOL bCountOnly );
	void				OnExitStandardSelector();

protected:
// Overrides
	//{{AFX_VIRTUAL(CStandardSelector)
	virtual BOOL		OnNotify( WPARAM wParam, LPARAM lParam, LRESULT *pResult );
	//}}AFX_VIRTUAL

	DECLARE_MESSAGE_MAP()
	//{{AFX_VIRTUAL( CStandardSelector )
	afx_msg int			OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg void		OnBnClickedInstallFromSelectedDrive( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedInstallCancel( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg BOOL		OnEraseBkgnd( CDC *pDC );
	//}}AFX_VIRTUAL

};


// Function prototypes:
//
	void				InitInstallModule();
	void				CloseInstallModule();

