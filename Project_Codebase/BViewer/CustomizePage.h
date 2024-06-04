// CustomizePage.h : Header file defining the structure of the CCustomizePage class of
//	CPropertyPage, which implements the "Set Up BViewer" tab of the main Control Panel.
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
//	*[3] 05/14/2024 by Tom Atwood
//		Removed obsolete film standard reference images.
//	*[2] 08/29/2023 by Tom Atwood
//		Improved user editing.  Removed two buttons, modified edit user button.
//		Moved user field editing functions to the ReaderInfoScreen module.
//		Added EditUserInfo() declaration.  Removed LoadCountrySelectionList()
//		declaration.  Removed the OnEditFocus functions since the reader info
//		edit fields are being made read-only.
//	*[1] 04/11/2023 by Tom Atwood
//		Added the GetRenderingMethodText() function.
//
#pragma once

#include "TomGroup.h"
#include "TomStatic.h"
#include "TomButton.h"
#include "TomEdit.h"
#include "TomEditDate.h"
#include "TomComboBox.h"
#include "Customization.h"
#include "ControlTip.h"


// CCustomizePage dialog
class CCustomizePage : public CPropertyPage
{
public:
	CCustomizePage();
	virtual ~CCustomizePage();

// Dialog Data
	enum { IDD = IDD_PROP_PAGE_CFG };

	BOOL				m_bPageIsInitialized;
	BOOL				m_bImageDisplaysAreConfigured;
	CBrush				m_BkgdBrush;
	CControlTip			*m_pControlTip;

	TomStatic			m_StaticImagePlacement;
		TomStatic			m_StaticShowStdImageOn;
		TomButton			m_ButtonStdDisplayAuto;
		TomButton			m_ButtonStdDisplayPrimary;
		TomButton			m_ButtonStdDisplayMonitor2;
		TomButton			m_ButtonStdDisplayMonitor3;
		TomGroup			m_GroupStdDisplayButtons;
		
		TomStatic			m_StaticShowStudyImageOn;
		TomButton			m_ButtonStudyDisplayAuto;
		TomButton			m_ButtonStudyDisplayPrimary;
		TomButton			m_ButtonStudyDisplayMonitor2;
		TomButton			m_ButtonStudyDisplayMonitor3;
		TomGroup			m_GroupStudyDisplayButtons;

	TomStatic			m_StaticStudySelection;
		TomStatic			m_StaticStudySelectionDisplayEmphasis;
		TomButton			m_ButtonShowSummaryInfo;
		TomButton			m_ButtonShowStudyInfo;
		TomButton			m_ButtonShowSeriesInfo;
		TomButton			m_ButtonShowImageInfo;
		TomGroup			m_GroupShowInfoButtons;

	TomStatic			m_StaticImageFullSizeAdjust;
		TomStatic			m_StaticPrimaryMonitor;
		TomStatic			m_StaticMonitor2;
		TomStatic			m_StaticMonitor3;

		TomStatic			m_StaticScreenWidthInMillimeters;
		TomEdit				m_EditPrimaryMonitorWidth;
		TomEdit				m_EditMonitor2Width;
		TomEdit				m_EditMonitor3Width;

		TomStatic			m_StaticScreenHeightInMillimeters;
		TomEdit				m_EditPrimaryMonitorHeight;
		TomEdit				m_EditMonitor2Height;
		TomEdit				m_EditMonitor3Height;
		
		TomStatic			m_StaticGrayscaleResolution;
		TomStatic			m_StaticGrayscaleBitDepth;
		TomComboBox			m_ComboBoxSelectPrimaryMonitorRenderingMethod;
		TomComboBox			m_ComboBoxSelectMonitor2RenderingMethod;
		TomComboBox			m_ComboBoxSelectMonitor3RenderingMethod;

	TomStatic			m_StaticReaderIdentification;
		TomStatic			m_StaticReaderLastName;
		TomEdit				m_EditReaderLastName;

		TomStatic			m_StaticLoginName;
		TomEdit				m_EditLoginName;

		TomStatic			m_StaticReaderID;
		TomEdit				m_EditReaderID;

		TomStatic			m_StaticLoginPassword;
		TomEdit				m_EditLoginPassword;

		TomStatic			m_StaticAE_Title;
		TomEdit				m_EditAE_Title;

		TomStatic			m_StaticReaderInitials;
		TomEdit				m_EditReaderInitials;

		TomStatic			m_StaticReaderReportSignatureName;
		TomEdit				m_EditReaderReportSignatureName;

		TomStatic			m_StaticReaderStreetAddress;
		TomEdit				m_EditReaderStreetAddress;

		TomStatic			m_StaticReaderCity;
		TomEdit				m_EditReaderCity;

		TomStatic			m_StaticReaderState;
		TomEdit				m_EditReaderState;

		TomStatic			m_StaticReaderZipCode;
		TomEdit				m_EditReaderZipCode;

	TomGroup			m_GroupEditSequencing;
	
		TomButton			m_ButtonAboutBViewer;
		TomButton			m_ButtonTechnicalRequirements;

		TomButton			m_ButtonControlBRetriever;
		TomButton			m_ButtonSetNetworkAddress;
		TomButton			m_ButtonClearImageFolders;
		TomButton			m_ButtonEditUser;

		TomStatic			m_StaticReaderCountry;
		TomEdit				m_EditReaderCountry;

		TomButton			m_ButtonBeginNewTestSession;
		TomStatic			m_StaticHelpfulTips;

protected:
	void				ClearReaderInfoDisplay();
	BOOL				LoadRenderingMethodSelectionLists();
	void				EditUserInfo( BOOL bSetInitialReader );			// *[2] Added function declaration.
	void				ResetPage();
	void				UpdateDisplaySettings();
	BOOL				DirectoryExists( char *pFullDirectorySpecification );
	void				DeleteImageFolderContents();
	void				InitializeControlTips();						// *[2] Was Public, now Protected.

public:
	void				DeleteFolderContents( char *SearchDirectory, int FolderIndent );
	void				ResetReaderInfo();			// *[2] Created as a separate, unprotected function.
	BOOL				WriteBViewerConfiguration();

// Overrides
	//{{AFX_VIRTUAL(CCustomizePage)
	virtual BOOL		OnInitDialog();
	virtual BOOL		OnNotify( WPARAM wParam, LPARAM lParam, LRESULT *pResult );
	virtual BOOL		OnSetActive();
	virtual BOOL		OnKillActive();
	//}}AFX_VIRTUAL

public:

	DECLARE_MESSAGE_MAP()

	//{{AFX_MSG(CCustomizePage)
	afx_msg HBRUSH		OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor );

	afx_msg void		OnBnClickedStdDisplayAuto( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedStdDisplayPrimary( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedStdDisplayMonitor2( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedStdDisplayMonitor3( NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void		OnBnClickedStudyDisplayAuto( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedStudyDisplayPrimary( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedStudyDisplayMonitor2( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedStudyDisplayMonitor3( NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void		OnBnClickedShowSummaryInfo( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedShowStudyInfo( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedShowSeriesInfo( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedShowImageInfo( NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void		OnBnClickedControlBRetriever( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedSetNetworkAddress( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedClearImageFolders( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedEditUser( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedBeginNewTestSession( NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void		OnEditPrimaryMonitorWidthKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditMonitor2WidthKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditMonitor3WidthKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditPrimaryMonitorHeightKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditMonitor2HeightKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditMonitor3HeightKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnPrimaryMonitorRenderingMethodSelected();
	afx_msg void		OnMonitor2RenderingMethodSelected();
	afx_msg void		OnMonitor3RenderingMethodSelected();

	afx_msg void		OnAppAbout( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnBnClickedTechnicalRequirements( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG

};

	char			*GetRenderingMethodText( unsigned long RenderingMethod );		// [3] Added function.

