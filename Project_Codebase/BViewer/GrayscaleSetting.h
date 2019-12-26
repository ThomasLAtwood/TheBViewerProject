// GrayscaleSetting.h - Defines the data structures and functions for the grayscale
// settings associated with each image.
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
#include "TomComboBox.h"

#define PRESET_ERROR_INSUFFICIENT_MEMORY		1
#define PRESET_ERROR_FILE_OPEN_FOR_READ			2
#define PRESET_ERROR_FILE_OPEN_FOR_WRITE		3

#define PRESET_ERROR_DICT_LENGTH				3



typedef struct
	{
	char					m_PresetName[ MAX_CFG_STRING_LENGTH ];
	double					m_Gamma;
	double					m_WindowCenter;
	double					m_WindowWidth;
	BOOL					m_bColorsInverted;
	double					m_RelativeMouseHorizontalPosition;
	double					m_RelativeMouseVerticalPosition;
	double					m_WindowMinPixelAmplitude;
	double					m_WindowMaxPixelAmplitude;
	} IMAGE_GRAYSCALE_SETTING;



// CPreset dialog

class CPreset : public CDialog
{
//	DECLARE_DYNAMIC( CPreset )

public:
	CPreset( CWnd *pParent = NULL );   // standard constructor
	virtual ~CPreset();


// Dialog Data
	enum { IDD = IDD_DIALOG_PRESET };
	CBrush						m_BkgdBrush;

	TomStatic					m_StaticGrayscalePresets;
	TomStatic					m_StaticApplyPresetHelpInfo;
	TomStatic					m_StaticSavePresetHelpInfo;
	TomButton					m_ButtonSave;
	TomButton					m_ButtonApply;
	TomButton					m_ButtonDelete;
	TomButton					m_ButtonCancel;

	TomStatic					m_StaticEditPresetName;
	TomEdit						m_EditPresetName;

	TomStatic					m_StaticSelectPreset;
	TomStatic					m_StaticCurrentPresets;
	TomComboBox					m_ComboBoxSelectPreset;

	IMAGE_GRAYSCALE_SETTING		*m_pCurrentPreset;

	BOOL						m_bSaveImageSetting;


protected:
// Overrides
	//{{AFX_VIRTUAL(CPreset)
	//}}AFX_VIRTUAL


	DECLARE_MESSAGE_MAP()

protected:
	//{{AFX_MSG(CPreset)
	afx_msg void		OnBnClickedSaveGrayscalePreset( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedApplyGrayscalePreset( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedDeleteGrayscalePreset( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedCancelGrayscalePresets( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH		OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor );
	afx_msg BOOL		OnEraseBkgnd( CDC *pDC );
	virtual BOOL		OnInitDialog();
	afx_msg void		OnPresetSelected();
	afx_msg void		OnClose();
	//}}AFX_MSG

public:
	BOOL				LoadPresetSelectionList();
};




// Function prototypes:
//
	void				InitPresetModule();
	void				ClosePresetModule();
	void				ErasePresetList();
	void				SetPresetFileSpecification( char *pImagePresetFileSpec );
	BOOL				ReadPresetFile();
	BOOL				WritePresetFile();

