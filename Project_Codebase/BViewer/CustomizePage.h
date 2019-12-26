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
#pragma once

#include "TomGroup.h"
#include "TomStatic.h"
#include "TomButton.h"
#include "TomEdit.h"
#include "TomEditDate.h"
#include "TomComboBox.h"
#include "Customization.h"
#include "StandardSelector.h"
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
	CStandardSelector	*m_pStandardImageInstaller;
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
		TomEdit				m_EditPrimaryMonitorGrayscaleBitDepth;
		TomEdit				m_EditMonitor2GrayscaleBitDepth;
		TomEdit				m_EditMonitor3GrayscaleBitDepth;

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
		TomButton			m_ButtonInstallStandards;

		TomButton			m_ButtonControlBRetriever;
		TomButton			m_ButtonSetNetworkAddress;
		TomButton			m_ButtonClearImageFolders;
		TomButton			m_ButtonAddUser;
		TomButton			m_ButtonEditUser;

	TomStatic			m_StaticSelectCountry;
		TomComboBox			m_ComboBoxSelectCountry;

		TomButton			m_ButtonBeginNewTestSession;
		TomStatic			m_StaticHelpfulTips;

protected:
	void				ClearReaderInfoDisplay();
	BOOL				LoadCountrySelectionList();
	void				ResetPage();
	void				UpdateDisplaySettings();
	BOOL				DirectoryExists( char *pFullDirectorySpecification );
	void				DeleteImageFolderContents();

public:
	void				InitializeControlTips();
	void				DeleteFolderContents( char *SearchDirectory, int FolderIndent );
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

	afx_msg void		OnBnClickedInstallStandards( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedControlBRetriever( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedSetNetworkAddress( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedClearImageFolders( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedAddUser( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedEditUser( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedBeginNewTestSession( NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void		OnEditPrimaryMonitorWidthKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditMonitor2WidthKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditMonitor3WidthKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditPrimaryMonitorHeightKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditMonitor2HeightKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditMonitor3HeightKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditPrimaryMonitorBitDepthKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditMonitor2BitDepthKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditMonitor3BitDepthKillFocus( NMHDR *pNMHDR, LRESULT *pResult );

	afx_msg void		OnEditReaderLastNameKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditLoginNameKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditReaderIDKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditLoginPasswordKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditReaderInitialsKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditAE_TitleKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditReaderReportSignatureNameKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditReaderStreetAddressKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditReaderCityKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditReaderStateKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEditReaderZipCodeKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnCountrySelected();

	afx_msg void		OnAppAbout( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnBnClickedTechnicalRequirements( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG

};


