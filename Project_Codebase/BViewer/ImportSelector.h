// ImportSelector.h : Header file defining the structure of the CImportSelector
//  class, which implements the image import selection process.
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

#include "ImportDicomdir.h"


#define IMPORT_ERROR_INSUFFICIENT_MEMORY		1
#define IMPORT_ERROR_FILE_COPY					2
#define IMPORT_ERROR_FILE_MOVE					3
#define IMPORT_ERROR_DICOMDIR_READ				4

#define IMPORT_ERROR_DICT_LENGTH				4


#define ITEM_IS_IMAGE							0
#define ITEM_IS_FOLDER							1

// CImportSelector
class CImportSelector : public CWnd
{
public:
	CImportSelector( int DialogWidth, int DialogHeight, COLORREF BackgroundColor, DWORD WindowStyle );
	virtual ~CImportSelector();

// Attributes
public:
	int								m_DialogWidth;
	int								m_DialogHeight;
	int								m_Column1XOffset;
	int								m_Row1YOffset;
	int								m_Row2YOffset;
	int								m_Column2XOffset;
	COLORREF						m_BackgroundColor;
	DWORD							m_WindowStyle;

	TomStatic						m_StaticUserMessage;
	TomStatic						m_StaticStep1;
	TomStatic						m_StaticStep1Text;
	TomButton						m_ButtonRefreshView;
	TomStatic						m_StaticStep2;
	TomStatic						m_StaticStep2Text;
	TomStatic						m_StaticStep3;
	TomStatic						m_StaticStep3Text;
	TomButton						m_ButtonAutoImport;
	TomStatic						m_StaticStep4;
	TomStatic						m_StaticStep4Text;
	TomButton						m_ButtonImport;
	TomButton						m_ButtonImportCancel;
	
	CTreeCtrl						*m_pExplorer;
	CImageList						m_FolderIcons;
	CBitmap							m_DriveBitmap;
	CBitmap							m_FolderBitmap;
	CBitmap							m_FolderOpenBitmap;
	CBitmap							m_DicomImageBitmap;
	CImportDicomdir					*m_pImportDicomdir;


	char							m_SelectedFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	BOOL							m_bSelectionIsAFolder;
	BOOL							m_bSelectionIsADICOMDIR;
	LIST_HEAD						m_ListOfProcessedItemFileNames;
	unsigned long					m_nDuplicateFileNamesDetected;
	IMAGE_FILE_SET_SPECIFICATION	*m_pListOfFileSetItems;
	HTREEITEM						m_SelectedItem;
	unsigned long					m_TotalImageFilesImported;

// Method prototypes:
//
public:
	BOOL				SetPosition( int x, int y, CWnd *pParentWnd, CString WindowClass );
	BOOL				CopyDesignatedFile( char *pSourceImageFileSpec );
	BOOL				CopyDirectoryContents( char *pSourceDirectorySpec );
	void				EraseFileSpecList( IMAGE_FILE_SET_SPECIFICATION **ppFileSpecList );
	void				DeleteChildItems( HTREEITEM hParentItem );
	void				ExpandSelection( HTREEITEM hNewlySelectedItem );
	void				OnExitImportSelector();
	void				CloseThisWindow();

// Overrides
protected:
	//{{AFX_VIRTUAL(CImportSelector)
	virtual BOOL		OnNotify( WPARAM wParam, LPARAM lParam, LRESULT *pResult );
	//}}AFX_VIRTUAL


	DECLARE_MESSAGE_MAP()
	//{{AFX_MSG(CImportSelector)
	afx_msg int			OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg void		OnBnClickedRefreshView( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedImportCheckedItems( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedAutoImport( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedImportCancel( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg BOOL		OnEraseBkgnd( CDC *pDC );
	//}}AFX_MSG

};


// Function prototypes:
//
	void				InitImportModule();
	void				CloseImportModule();
	void				CloseImportSelector( void *pParentWindow );

