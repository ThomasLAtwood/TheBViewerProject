// ImportDicomdir.h : Header file defining the structure of the CImportDicomdir
//  class, which implements the image import selection process for DICOMDIR
//  file sets.
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


#define IMPORT_DICOMDIR_ERROR_INSUFFICIENT_MEMORY		1
#define IMPORT_DICOMDIR_ERROR_FILE_MOVE					2
#define IMPORT_DICOMDIR_ERROR_DICOMDIR_READ				3
#define IMPORT_DICOMDIR_ERROR_NO_DICOMDIR_FILES			4

#define IMPORT_DICOMDIR_ERROR_DICT_LENGTH				4


typedef 	void (*IMPORT_CALLBACK_FUNCTION)( void *pParentWindow);

// CImportDicomdir
class CImportDicomdir : public CWnd
{
public:
	CImportDicomdir( BOOL bSelectionIsAFolder, BOOL bSelectionIsADICOMDIR, char *pSelectedFileSpec, IMPORT_CALLBACK_FUNCTION CallbackFunction,
						int DialogWidth, int DialogHeight, COLORREF BackgroundColor, DWORD WindowStyle );
	virtual ~CImportDicomdir();

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

	char							m_SelectedFileSpec[ FULL_FILE_SPEC_STRING_LENGTH ];
	BOOL							m_bSelectionIsAFolder;
	BOOL							m_bSelectionIsADICOMDIR;
	LIST_HEAD						m_ListOfProcessedItemFileNames;
	unsigned long					m_nDuplicateFileNamesDetected;
	IMAGE_FILE_SET_SPECIFICATION	*m_pListOfFileSetItems;
	HTREEITEM						m_SelectedItem;
	unsigned long					m_TotalImageFilesImported;
	IMPORT_CALLBACK_FUNCTION		m_CallbackFunction;

// Method prototypes:
//
public:
	BOOL				SetPosition( int x, int y, CWnd *pParentWnd, CString WindowClass );
	BOOL				CopyDesignatedFile( char *pSourceImageFileSpec );
	void				EraseFileSpecList( IMAGE_FILE_SET_SPECIFICATION **ppFileSpecList );
	BOOL				ReadDicomDirectoryFile( char *pDicomdirFileSpec );
	BOOL				SearchForDICOMDIRFiles( char *pSourceDirectorySpec );
	void				DisplayDicomdirFileTree();
	void				OnExitImportDicomdirSelector();

// Overrides
protected:

	DECLARE_MESSAGE_MAP()
	//{{AFX_MSG(CImportDicomdir)
	afx_msg int			OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg void		OnBnClickedImportCheckedItems( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedImportCancel( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg BOOL		OnEraseBkgnd( CDC *pDC );
	//}}AFX_MSG

};


// Function prototypes:
//
	void				InitImportDicomdirModule();
	void				CloseImportDicomdirModule();



