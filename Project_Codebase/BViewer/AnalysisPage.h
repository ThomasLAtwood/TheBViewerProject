// AnalysisPage.h : Header file defining the structure of the CAnalysisPage
//  class of CPropertyPage, which implements the "Enter Interpretation" tab
//  of the main Control Panel.
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
//	*[1] 02/27/2023 by Tom Atwood
//		Added OnBnClickedImageScapulaOverlay() function to correctly set the Image Quality button status.
//		Fixed a problem where a previously interpreted Unreadable Image study did not initialize the
//		interpretation screen correctly.
//
#pragma once

#include "TomGroup.h"
#include "TomStatic.h"
#include "TomButton.h"
#include "TomEdit.h"
#include "TomEditDate.h"

// CAnalysisPage dialog

class CAnalysisPage : public CPropertyPage
{
public:
	CAnalysisPage();
	virtual ~CAnalysisPage();
	
	CBrush				m_BkgdBrush;
	BOOL				m_bPageIsInitialized;
	TomStatic			m_StaticSubject;
	TomStatic			m_StaticPatientName;

	TomButton			m_ImageQualityButton;
	BOOL				m_bImageQualityPageIsCompleted;
	TomStatic			m_StaticImageQualityTitle;
		TomButton			m_ImageGrade1;
		TomButton			m_ImageGrade2;
		TomButton			m_ImageGrade3;
		TomButton			m_ImageGradeUR;
		TomGroup			m_ImageGradeButtonGroup;
		TomStatic			m_ImageGradeExplanation;
			
		TomButton			m_ImageOverexposed;
		TomButton			m_ImageUnderexposed;
		TomButton			m_ImageArtifacts;
		TomButton			m_ImageImproperPosition;
		TomButton			m_ImagePoorContrast;
		TomButton			m_ImagePoorProcessing;
		TomButton			m_ImageUnderinflation;
		TomButton			m_ImageMottle;
		TomButton			m_ImageExcessiveEdgeEnhancement;
		TomButton			m_ImageScapulaOverlay;
		TomButton			m_ImageOther;
		TomGroup			m_ImageQualificationButtonGroup;
		
		TomStatic			m_StaticPleaseSpecify;
		TomEdit				m_EditImageQualityOther;

	TomButton			m_ParenchymalAbnormalityButton;
	TomStatic			m_StaticParenchymalAbnormalitiesTitle;
	BOOL				m_bParenchymalAbnormalityPageIsCompleted;
		TomStatic			m_AnyParenchymalAbnormalities;
		TomButton			m_ButtonParenchymalYes;
		TomButton			m_ButtonParenchymalNo;
		TomGroup			m_ParenchymalYesNoButtonGroup;
		TomGroup			m_ParenchymalStaticTextGroup;
		
		TomStatic			m_SmallOpacitiesSubtitle;
			TomStatic			m_StaticShapeAndSize;
			TomStatic			m_StaticPrimary;
			TomButton			m_ButtonPrimary_P;
			TomButton			m_ButtonPrimary_S;
			TomButton			m_ButtonPrimary_Q;
			TomButton			m_ButtonPrimary_T;
			TomButton			m_ButtonPrimary_R;
			TomButton			m_ButtonPrimary_U;
			TomGroup			m_SmallOpacityPrimaryButtonGroup;

			TomStatic			m_StaticSecondary;
			TomButton			m_ButtonSecondary_P;
			TomButton			m_ButtonSecondary_S;
			TomButton			m_ButtonSecondary_Q;
			TomButton			m_ButtonSecondary_T;
			TomButton			m_ButtonSecondary_R;
			TomButton			m_ButtonSecondary_U;
			TomGroup			m_SmallOpacitySecondaryButtonGroup;

			TomStatic			m_StaticZones;
			TomButton			m_ButtonSmallOpacityZoneUpperRight;
			TomButton			m_ButtonSmallOpacityZoneUpperLeft;
			TomButton			m_ButtonSmallOpacityZoneMiddleRight;
			TomButton			m_ButtonSmallOpacityZoneMiddleLeft;
			TomButton			m_ButtonSmallOpacityZoneLowerRight;
			TomButton			m_ButtonSmallOpacityZoneLowerLeft;
			TomGroup			m_SmallOpacityZoneButtonGroup;
			TomButton			m_ButtonAllSmallOpacityZones;

			TomStatic			m_StaticProfusion;
			TomButton			m_ButtonSmallOpacityProfusion_0Minus;
			TomButton			m_ButtonSmallOpacityProfusion_00;
			TomButton			m_ButtonSmallOpacityProfusion_01;
			TomButton			m_ButtonSmallOpacityProfusion_10;
			TomButton			m_ButtonSmallOpacityProfusion_11;
			TomButton			m_ButtonSmallOpacityProfusion_12;
			TomButton			m_ButtonSmallOpacityProfusion_21;
			TomButton			m_ButtonSmallOpacityProfusion_22;
			TomButton			m_ButtonSmallOpacityProfusion_23;
			TomButton			m_ButtonSmallOpacityProfusion_32;
			TomButton			m_ButtonSmallOpacityProfusion_33;
			TomButton			m_ButtonSmallOpacityProfusion_3Plus;
			TomGroup			m_SmallOpacityProfusionButtonGroup;

		TomStatic			m_LargeOpacitiesSubtitle;
			TomStatic			m_StaticSize;
			TomButton			m_ButtonLargeOpacitySize_0;
			TomButton			m_ButtonLargeOpacitySize_A;
			TomButton			m_ButtonLargeOpacitySize_B;
			TomButton			m_ButtonLargeOpacitySize_C;
			TomGroup			m_LargeOpacitySizeButtonGroup;

	TomButton			m_PleuralAbnormalityButton;
	TomStatic			m_StaticPleuralAbnormalitiesTitle;
	BOOL				m_bPleuralAbnormalityPageIsCompleted;
		TomStatic			m_AnyPleuralAbnormalities;
		TomButton			m_ButtonPleuralYes;
		TomButton			m_ButtonPleuralNo;
		TomGroup			m_PleuralYesNoButtonGroup;
		TomGroup			m_PleuralStaticTextGroup;

		TomStatic			m_PleuralPlaquesSubtitle;
			TomStatic			m_StaticPleuralPlaquesSite;
			TomStatic			m_StaticPleuralPlaquesChestProfile;
			TomStatic			m_StaticPleuralPlaquesChestFaceOn;
			TomStatic			m_StaticPleuralPlaquesDiaphragm;
			TomStatic			m_StaticPleuralPlaquesOther;
			TomButton			m_ButtonPleuralSiteChestProfileNone;
			TomButton			m_ButtonPleuralSiteChestProfileRight;
			TomButton			m_ButtonPleuralSiteChestProfileLeft;
			TomGroup			m_GroupButtonsPleuralSiteChestProfile;

			TomButton			m_ButtonPleuralSiteChestFaceOnNone;
			TomButton			m_ButtonPleuralSiteChestFaceOnRight;
			TomButton			m_ButtonPleuralSiteChestFaceOnLeft;
			TomGroup			m_GroupButtonsPleuralSiteChestFaceOn;

			TomButton			m_ButtonPleuralSiteDiaphragmNone;
			TomButton			m_ButtonPleuralSiteDiaphragmRight;
			TomButton			m_ButtonPleuralSiteDiaphragmLeft;
			TomGroup			m_GroupButtonsPleuralSiteDiaphragm;

			TomButton			m_ButtonPleuralSiteOtherNone;
			TomButton			m_ButtonPleuralSiteOtherRight;
			TomButton			m_ButtonPleuralSiteOtherLeft;
			TomGroup			m_GroupButtonsPleuralSiteOther;

			TomStatic			m_StaticPleuralCalcification;
			TomButton			m_ButtonCalcificationChestProfileNone;
			TomButton			m_ButtonCalcificationChestProfileRight;
			TomButton			m_ButtonCalcificationChestProfileLeft;
			TomGroup			m_GroupButtonsCalcificationChestProfile;

			TomButton			m_ButtonCalcificationChestFaceOnNone;
			TomButton			m_ButtonCalcificationChestFaceOnRight;
			TomButton			m_ButtonCalcificationChestFaceOnLeft;
			TomGroup			m_GroupButtonsCalcificationChestFaceOn;

			TomButton			m_ButtonCalcificationDiaphragmNone;
			TomButton			m_ButtonCalcificationDiaphragmRight;
			TomButton			m_ButtonCalcificationDiaphragmLeft;
			TomGroup			m_GroupButtonsCalcificationDiaphragm;

			TomButton			m_ButtonCalcificationOtherNone;
			TomButton			m_ButtonCalcificationOtherRight;
			TomButton			m_ButtonCalcificationOtherLeft;
			TomGroup			m_GroupButtonsCalcificationOther;

			TomStatic			m_StaticPleuralExtent;
			TomButton			m_ButtonPleuralExtentNoneOnRight;
			TomButton			m_ButtonPleuralExtentRight;
			TomGroup			m_GroupButtonsPleuralExtentRight;

			TomButton			m_ButtonPleuralExtentRightSize1;
			TomButton			m_ButtonPleuralExtentRightSize2;
			TomButton			m_ButtonPleuralExtentRightSize3;
			TomGroup			m_GroupButtonsPleuralExtentRightSize;

			TomButton			m_ButtonPleuralExtentNoneOnLeft;
			TomButton			m_ButtonPleuralExtentLeft;
			TomGroup			m_GroupButtonsPleuralExtentLeft;

			TomButton			m_ButtonPleuralExtentLeftSize1;
			TomButton			m_ButtonPleuralExtentLeftSize2;
			TomButton			m_ButtonPleuralExtentLeftSize3;
			TomGroup			m_GroupButtonsPleuralExtentLeftSize;

			TomStatic			m_StaticPleuralWidth;
			TomButton			m_ButtonPleuralWidthNoneOnRight;
			TomButton			m_ButtonPleuralWidthRight;
			TomGroup			m_GroupButtonsPleuralWidthRight;

			TomButton			m_ButtonPleuralWidthRightSize1;
			TomButton			m_ButtonPleuralWidthRightSize2;
			TomButton			m_ButtonPleuralWidthRightSize3;
			TomGroup			m_GroupButtonsPleuralWidthRightSize;

			TomButton			m_ButtonPleuralWidthNoneOnLeft;
			TomButton			m_ButtonPleuralWidthLeft;
			TomGroup			m_GroupButtonsPleuralWidthLeft;

			TomButton			m_ButtonPleuralWidthLeftSize1;
			TomButton			m_ButtonPleuralWidthLeftSize2;
			TomButton			m_ButtonPleuralWidthLeftSize3;
			TomGroup			m_GroupButtonsPleuralWidthLeftSize;

		TomStatic			m_AngleObliterationSubtitle;
			TomButton			m_ButtonAngleObliterationNone;
			TomButton			m_ButtonAngleObliterationRight;
			TomButton			m_ButtonAngleObliterationLeft;
			TomGroup			m_GroupButtonsAngleObliteration;

		TomStatic			m_PleuralThickeningSubtitle;
			TomGroup			m_PleuralThickeningStaticTextGroup;
			TomStatic			m_StaticPleuralThickeningSite;
			TomStatic			m_StaticPleuralThickeningChestProfile;
			TomStatic			m_StaticPleuralThickeningChestFaceOn;

			TomButton			m_ButtonThickeningSiteChestProfileNone;
			TomButton			m_ButtonThickeningSiteChestProfileRight;
			TomButton			m_ButtonThickeningSiteChestProfileLeft;
			TomGroup			m_GroupButtonsThickeningSiteChestProfile;

			TomButton			m_ButtonThickeningSiteChestFaceOnNone;
			TomButton			m_ButtonThickeningSiteChestFaceOnRight;
			TomButton			m_ButtonThickeningSiteChestFaceOnLeft;
			TomGroup			m_GroupButtonsThickeningSiteChestFaceOn;

			TomStatic			m_StaticThickeningCalcification;
			TomButton			m_ButtonThickeningCalcificationChestProfileNone;
			TomButton			m_ButtonThickeningCalcificationChestProfileRight;
			TomButton			m_ButtonThickeningCalcificationChestProfileLeft;
			TomGroup			m_GroupButtonsThickeningCalcificationChestProfile;

			TomButton			m_ButtonThickeningCalcificationChestFaceOnNone;
			TomButton			m_ButtonThickeningCalcificationChestFaceOnRight;
			TomButton			m_ButtonThickeningCalcificationChestFaceOnLeft;
			TomGroup			m_GroupButtonsThickeningCalcificationChestFaceOn;

			TomStatic			m_StaticPleuralThickeningExtent;
			TomButton			m_ButtonPleuralThickeningExtentNoneOnRight;
			TomButton			m_ButtonPleuralThickeningExtentRight;
			TomGroup			m_GroupButtonsPleuralThickeningExtentRight;

			TomButton			m_ButtonPleuralThickeningExtentRightSize1;
			TomButton			m_ButtonPleuralThickeningExtentRightSize2;
			TomButton			m_ButtonPleuralThickeningExtentRightSize3;
			TomGroup			m_GroupButtonsPleuralThickeningExtentRightSize;

			TomButton			m_ButtonPleuralThickeningExtentNoneOnLeft;
			TomButton			m_ButtonPleuralThickeningExtentLeft;
			TomGroup			m_GroupButtonsPleuralThickeningExtentLeft;

			TomButton			m_ButtonPleuralThickeningExtentLeftSize1;
			TomButton			m_ButtonPleuralThickeningExtentLeftSize2;
			TomButton			m_ButtonPleuralThickeningExtentLeftSize3;
			TomGroup			m_GroupButtonsPleuralThickeningExtentLeftSize;

			TomStatic			m_StaticPleuralThickeningWidth;
			TomButton			m_ButtonPleuralThickeningWidthNoneOnRight;
			TomButton			m_ButtonPleuralThickeningWidthRight;
			TomGroup			m_GroupButtonsPleuralThickeningWidthRight;

			TomButton			m_ButtonPleuralThickeningWidthRightSize1;
			TomButton			m_ButtonPleuralThickeningWidthRightSize2;
			TomButton			m_ButtonPleuralThickeningWidthRightSize3;
			TomGroup			m_GroupButtonsPleuralThickeningWidthRightSize;

			TomButton			m_ButtonPleuralThickeningWidthNoneOnLeft;
			TomButton			m_ButtonPleuralThickeningWidthLeft;
			TomGroup			m_GroupButtonsPleuralThickeningWidthLeft;

			TomButton			m_ButtonPleuralThickeningWidthLeftSize1;
			TomButton			m_ButtonPleuralThickeningWidthLeftSize2;
			TomButton			m_ButtonPleuralThickeningWidthLeftSize3;
			TomGroup			m_GroupButtonsPleuralThickeningWidthLeftSize;

	TomButton			m_OtherAbnormalityButton;
	TomStatic			m_StaticOtherAbnormalitiesTitle;
	BOOL				m_bOtherAbnormalityPageIsCompleted;
		TomButton			m_ButtonOtherYes;
		TomButton			m_ButtonOtherNo;
		TomGroup			m_OtherYesNoButtonGroup;
		TomGroup			m_OtherStaticTextGroup;

		TomStatic			m_OtherSymbolsSubtitle;
			TomButton			m_ButtonSymbol_aa;
			TomButton			m_ButtonSymbol_at;
			TomButton			m_ButtonSymbol_ax;
			TomButton			m_ButtonSymbol_bu;
			TomButton			m_ButtonSymbol_ca;
			TomButton			m_ButtonSymbol_cg;
			TomButton			m_ButtonSymbol_cn;
			TomButton			m_ButtonSymbol_co;
			TomButton			m_ButtonSymbol_cp;
			TomButton			m_ButtonSymbol_cv;
			TomButton			m_ButtonSymbol_di;
			TomButton			m_ButtonSymbol_ef;
			TomButton			m_ButtonSymbol_em;
			TomButton			m_ButtonSymbol_es;
			TomButton			m_ButtonSymbol_fr;
			TomButton			m_ButtonSymbol_hi;
			TomButton			m_ButtonSymbol_ho;
			TomButton			m_ButtonSymbol_id;
			TomButton			m_ButtonSymbol_ih;
			TomButton			m_ButtonSymbol_kl;
			TomButton			m_ButtonSymbol_me;
			TomButton			m_ButtonSymbol_pa;
			TomButton			m_ButtonSymbol_pb;
			TomButton			m_ButtonSymbol_pi;
			TomButton			m_ButtonSymbol_px;
			TomButton			m_ButtonSymbol_ra;
			TomButton			m_ButtonSymbol_rp;
			TomButton			m_ButtonSymbol_tb;
			TomButton			m_ButtonSymbol_od;
			TomGroup			m_GroupButtonSymbols;

			TomStatic			m_StaticSymbol_aa;
			TomStatic			m_StaticSymbol_at;
			TomStatic			m_StaticSymbol_ax;
			TomStatic			m_StaticSymbol_bu;
			TomStatic			m_StaticSymbol_ca;
			TomStatic			m_StaticSymbol_cg;
			TomStatic			m_StaticSymbol_cn;
			TomStatic			m_StaticSymbol_co;
			TomStatic			m_StaticSymbol_cp;
			TomStatic			m_StaticSymbol_cv;
			TomStatic			m_StaticSymbol_di;
			TomStatic			m_StaticSymbol_ef;
			TomStatic			m_StaticSymbol_em;
			TomStatic			m_StaticSymbol_es;
			TomStatic			m_StaticSymbol_fr;
			TomStatic			m_StaticSymbol_hi;
			TomStatic			m_StaticSymbol_ho;
			TomStatic			m_StaticSymbol_id;
			TomStatic			m_StaticSymbol_ih;
			TomStatic			m_StaticSymbol_kl;
			TomStatic			m_StaticSymbol_me;
			TomStatic			m_StaticSymbol_pa;
			TomStatic			m_StaticSymbol_pb;
			TomStatic			m_StaticSymbol_pi;
			TomStatic			m_StaticSymbol_px;
			TomStatic			m_StaticSymbol_ra;
			TomStatic			m_StaticSymbol_rp;
			TomStatic			m_StaticSymbol_tb;
			TomStatic			m_StaticSymbol_od;
			TomGroup			m_GroupStaticSymbols;
			
			TomStatic			m_StaticMarkAllThatApply;
			TomStatic			m_StaticUseIsOptional;

			TomStatic			m_StaticDiaphragmAbnormalities;
			TomButton			m_ButtonEventration;
			TomButton			m_ButtonHiatalHernia;

			TomStatic			m_StaticAirwayDisorders;
			TomButton			m_ButtonBronchovascularMarkings;
			TomButton			m_ButtonHyperinflation;

			TomStatic			m_StaticBonyAbnormalities;
			TomButton			m_ButtonBonyChestCage;
			TomButton			m_ButtonFractureHealed;
			TomButton			m_ButtonFractureNonHealed;
			TomButton			m_ButtonScoliosis;
			TomButton			m_ButtonVertebralColumn;

			TomStatic			m_StaticLungParenchymalAbn;
			TomButton			m_ButtonAzygosLobe;
			TomButton			m_ButtonLungDensity;
			TomButton			m_ButtonInfiltrate;
			TomButton			m_ButtonNodularLesion;

			TomStatic			m_StaticMiscAbnormalities;
			TomButton			m_ButtonForeignBody;
			TomButton			m_ButtonPostSurgical;
			TomButton			m_ButtonCyst;

			TomStatic			m_StaticVascularDisorders;
			TomButton			m_ButtonAortaAnomaly;
			TomButton			m_ButtonVascularAbnormality;

			TomStatic			m_StaticOtherComments;
			TomEdit				m_EditOtherComments;

			TomGroup			m_GroupStaticAbnormalities;
			TomGroup			m_GroupButtonDisorders;

	TomStatic			m_Static_OR_;
	TomButton			m_ApproveStudyButton;
	TomButton			m_ProduceReportButton;
	TomButton			m_CancelAndResetButton;

protected:
	int					m_nCurHeight;
	int					m_nScrollPos;
	CRect				m_rect;

// Dialog Data
	enum { IDD = IDD_PROP_PAGE_ANALYSIS };

public:

// Overrides
	//{{AFX_VIRTUAL(CAnalysisPage)
	virtual BOOL		OnInitDialog();
	virtual BOOL		OnSetActive();
	virtual BOOL		OnKillActive();
	//}}AFX_VIRTUAL

	DECLARE_MESSAGE_MAP()
public:
	void			RedisplayPage();
	void			TurnToggleButtonOn( TomButton *pButton );
	void			LoadAnalysisFromCurrentStudy();
	BOOL			IsInterpretationComplete();
	void			CheckForIncompleteInterpretation( BOOL *pbOKToProceed );

protected:
	void			SetImageQualityPageVisibility( unsigned long Visibility );
	void			UpdateImageQualityPageStatus();
	void			SetParenchymalPageVisibility( unsigned long Visibility );
	void			SetSmallOpacityCompletion();
	void			UpdateParenchymalAbnormalityPageStatus();
	void			SetPleuralPageVisibility( unsigned long Visibility );
	void			UpdatePleuralAbnormalityPageStatus();
	void			FreezeButton( TomButton *pButton );
	void			AdjustChestProfileButtonStates();
	void			AdjustChestFaceOnButtonStates();
	void			SetPleuralExtentButtonStates();
	void			SetPleuralWidthButtonStates();
	void			AdjustChestDiaphragmButtonStates();
	void			AdjustPleuralSiteOtherButtonStates();
	void			SetStatusOfStaticPleuralPlaquesChestProfile();
	void			SetStatusOfStaticPleuralPlaquesChestFaceOn();
	void			SetStatusOfStaticPleuralPlaquesDiaphragm();
	void			SetStatusOfStaticPleuralPlaquesOther();
	void			SetStatusOfStaticPleuralPlaquesSite();
	void			SetStatusOfStaticPleuralCalcification();
	void			SetStatusOfStaticPleuralExtent();
	void			SetStatusOfStaticPleuralWidth();
	void			SetStatusOfStaticPleuralThickeningChestProfile();
	void			SetStatusOfStaticPleuralThickeningChestFaceOn();
	void			SetStatusOfStaticPleuralThickeningSite();
	void			SetStatusOfStaticPleuralThickeningCalcification();
	void			SetStatusOfStaticPleuralThickeningExtent();
	void			SetStatusOfStaticPleuralThickeningWidth();
	void			SetStatusOfPleuralPlaquesSubtitle();
	void			SetStatusOfPleuralThickeningSubtitle();
	void			AdjustChestThickeningProfileButtonStates();
	void			AdjustChestThickeningFaceOnButtonStates();
	void			SetPleuralThickeningExtentButtonStates();
	void			SetPleuralThickeningWidthButtonStates();
	void			SetStatusOfStaticSymbols();
	void			SetStatusOfStaticDisorders();
	void			SetOtherPageVisibility( unsigned long Visibility );
	void			UpdateOtherAbnormalityPageStatus();
	void			UpdateApproveStudyButtonStatus();
	void			ResetPage();
	void			DrawLine( CDC *pDC, long xStart, long yStart, long xEnd, long yEnd, int nWidth, COLORREF Color );
	void			LoadStudyDataFromScreens( CStudy *pCurrentStudy );



	//{{AFX_MSG(CAnalysisPage)
	afx_msg void	OnBnClickedImageQualityButton( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedImageGrade1Button( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedImageGrade2Button( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedImageGrade3Button( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedImageGradeURButton( NMHDR *pNMHDR, LRESULT *pResult );

		afx_msg void	OnBnClickedImageOverexposed( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedImageUnderexposed( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedImageArtifacts( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedImageImproperPosition( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedImagePoorContrast( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedImagePoorProcessing( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedImageUnderinflation( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedImageMottle( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedImageExcessiveEdgeEnhancement( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedImageScapulaOverlay( NMHDR *pNMHDR, LRESULT *pResult );			// *[1] Added this function.
		afx_msg void	OnBnClickedImageOther( NMHDR *pNMHDR, LRESULT *pResult );

	afx_msg void	OnBnClickedParenchymalAbnormalityButton( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedParenchymalYes( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedParenchymalNo( NMHDR *pNMHDR, LRESULT *pResult );

		afx_msg void	OnBnClickedButtonPrimary_P( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonPrimary_S( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonPrimary_Q( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonPrimary_T( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonPrimary_R( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonPrimary_U( NMHDR *pNMHDR, LRESULT *pResult );

		afx_msg void	OnBnClickedButtonSecondary_P( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonSecondary_S( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonSecondary_Q( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonSecondary_T( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonSecondary_R( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonSecondary_U( NMHDR *pNMHDR, LRESULT *pResult );

		afx_msg void	OnBnClickedButtonSmallOpacityZoneUpperRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonSmallOpacityZoneUpperLeft( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonSmallOpacityZoneMiddleRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonSmallOpacityZoneMiddleLeft( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonSmallOpacityZoneLowerRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonSmallOpacityZoneLowerLeft( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonSmallOpacityClickAllZones( NMHDR *pNMHDR, LRESULT *pResult );

		afx_msg void	OnBnClickedButtonSmallOpacityProfusion_0Minus( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonSmallOpacityProfusion_00( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonSmallOpacityProfusion_01( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonSmallOpacityProfusion_10( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonSmallOpacityProfusion_11( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonSmallOpacityProfusion_12( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonSmallOpacityProfusion_21( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonSmallOpacityProfusion_22( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonSmallOpacityProfusion_23( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonSmallOpacityProfusion_32( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonSmallOpacityProfusion_33( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonSmallOpacityProfusion_3Plus( NMHDR *pNMHDR, LRESULT *pResult );

		afx_msg void	OnBnClickedButtonLargeOpacitySize_0( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonLargeOpacitySize_A( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonLargeOpacitySize_B( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedButtonLargeOpacitySize_C( NMHDR *pNMHDR, LRESULT *pResult );

	afx_msg void	OnBnClickedPleuralAbnormalityButton( NMHDR *pNMHDR, LRESULT *pResult );

		afx_msg void	OnBnClickedPleuralYes( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralNo( NMHDR *pNMHDR, LRESULT *pResult );
		
		afx_msg void	OnBnClickedPleuralSiteChestProfileNone( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralSiteChestProfileRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralSiteChestProfileLeft( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralSiteChestFaceOnNone( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralSiteChestFaceOnRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralSiteChestFaceOnLeft( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralSiteDiaphragmNone( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralSiteDiaphragmRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralSiteDiaphragmLeft( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralSiteOtherNone( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralSiteOtherRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralSiteOtherLeft( NMHDR *pNMHDR, LRESULT *pResult );

		afx_msg void	OnBnClickedCalcificationChestProfileNone( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedCalcificationChestProfileRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedCalcificationChestProfileLeft( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedCalcificationChestFaceOnNone( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedCalcificationChestFaceOnRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedCalcificationChestFaceOnLeft( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedCalcificationDiaphragmNone( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedCalcificationDiaphragmRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedCalcificationDiaphragmLeft( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedCalcificationOtherNone( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedCalcificationOtherRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedCalcificationOtherLeft( NMHDR *pNMHDR, LRESULT *pResult );
		
		afx_msg void	OnBnClickedPleuralExtentNoneOnRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralExtentRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralExtentRightSize1( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralExtentRightSize2( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralExtentRightSize3( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralExtentNoneOnLeft( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralExtentLeft( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralExtentLeftSize1( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralExtentLeftSize2( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralExtentLeftSize3( NMHDR *pNMHDR, LRESULT *pResult );

		afx_msg void	OnBnClickedPleuralWidthNoneOnRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralWidthRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralWidthRightSize1( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralWidthRightSize2( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralWidthRightSize3( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralWidthNoneOnLeft( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralWidthLeft( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralWidthLeftSize1( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralWidthLeftSize2( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralWidthLeftSize3( NMHDR *pNMHDR, LRESULT *pResult );

		afx_msg void	OnBnClickedAngleObliterationNone( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedAngleObliterationRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedAngleObliterationLeft( NMHDR *pNMHDR, LRESULT *pResult );


		afx_msg void	OnBnClickedThickeningSiteChestProfileNone( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedThickeningSiteChestProfileRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedThickeningSiteChestProfileLeft( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedThickeningSiteChestFaceOnNone( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedThickeningSiteChestFaceOnRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedThickeningSiteChestFaceOnLeft( NMHDR *pNMHDR, LRESULT *pResult );

		afx_msg void	OnBnClickedThickeningCalcificationChestProfileNone( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedThickeningCalcificationChestProfileRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedThickeningCalcificationChestProfileLeft( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedThickeningCalcificationChestFaceOnNone( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedThickeningCalcificationChestFaceOnRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedThickeningCalcificationChestFaceOnLeft( NMHDR *pNMHDR, LRESULT *pResult );

		afx_msg void	OnBnClickedPleuralThickeningExtentNoneOnRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralThickeningExtentRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralThickeningExtentRightSize1( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralThickeningExtentRightSize2( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralThickeningExtentRightSize3( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralThickeningExtentNoneOnLeft( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralThickeningExtentLeft( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralThickeningExtentLeftSize1( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralThickeningExtentLeftSize2( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralThickeningExtentLeftSize3( NMHDR *pNMHDR, LRESULT *pResult );

		afx_msg void	OnBnClickedPleuralThickeningWidthNoneOnRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralThickeningWidthRight( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralThickeningWidthRightSize1( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralThickeningWidthRightSize2( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralThickeningWidthRightSize3( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralThickeningWidthNoneOnLeft( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralThickeningWidthLeft( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralThickeningWidthLeftSize1( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralThickeningWidthLeftSize2( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPleuralThickeningWidthLeftSize3( NMHDR *pNMHDR, LRESULT *pResult );

	afx_msg void	OnBnClickedOtherAbnormalityButton( NMHDR *pNMHDR, LRESULT *pResult );

		afx_msg void	OnBnClickedOtherYes( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedOtherNo( NMHDR *pNMHDR, LRESULT *pResult );

		afx_msg void	OnBnClickedSymbol_aa( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_at( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_ax( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_bu( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_ca( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_cg( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_cn( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_co( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_cp( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_cv( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_di( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_ef( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_em( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_es( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_fr( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_hi( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_ho( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_id( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_ih( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_kl( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_me( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_pa( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_pb( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_pi( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_px( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_ra( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_rp( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_tb( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedSymbol_od( NMHDR *pNMHDR, LRESULT *pResult );

		afx_msg void	OnBnClickedEventration( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedHiatalHernia( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedBronchovascularMarkings( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedHyperinflation( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedBonyChestCage( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedFractureHealed( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedFractureNonHealed( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedScoliosis( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedVertebralColumn( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedAzygosLobe( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedLungDensity( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedInfiltrate( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedNodularLesion( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedForeignBody( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedPostSurgical( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedCyst( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedAortaAnomaly( NMHDR *pNMHDR, LRESULT *pResult );
		afx_msg void	OnBnClickedVascularAbnormality( NMHDR *pNMHDR, LRESULT *pResult );

		afx_msg void	OnEditOtherCommentsFocus( NMHDR *pNotifyStruct, LRESULT *pResult );
		
	afx_msg void	OnBnClickedApproveStudyButton( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void	OnBnClickedCancelAndResetButton( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void	OnBnClickedProduceReportButton( NMHDR *pNMHDR, LRESULT *pResult );

	afx_msg void		OnPaint();
	afx_msg HBRUSH		OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor );
	afx_msg void		OnSize( UINT nType, int cx, int cy );
	afx_msg void		OnVScroll( UINT nSBCode, UINT nPos, CScrollBar *pScrollBar );
	//}}AFX_MSG
};

void CheckForIncompleteInterpretation( BOOL *pbOKToProceed );



