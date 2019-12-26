// FrameHeader.h : Header file defining the structure of the CFrameHeader class
//  of CDialogBar, which implements the header at the top of each image window.
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
#include "ControlTip.h"


// CFrameHeader
class CFrameHeader : public CDialogBar
{
public:
	CFrameHeader();
	virtual ~CFrameHeader();

	unsigned long		m_FrameFunction;
							#define IMAGE_FRAME_FUNCTION_PATIENT	1
							#define IMAGE_FRAME_FUNCTION_STANDARD	2
							#define IMAGE_FRAME_FUNCTION_REPORT		3
							#define IMAGE_FRAME_FUNCTION_CONTROL	4
	CBrush				m_BkgdBrush;
	CControlTip			*m_pControlTip;

	TomButton			m_ButtonExitBViewer;
	TomButton			m_ButtonDeleteCheckedImages;
	TomButton			m_ButtonImportImages;
	TomButton			m_ButtonShowNewImages;
	TomButton			m_ButtonShowLogDetail;
	TomStatic			m_StaticBRetrieverStatus;
	TomButton			m_ButtonEnterManualStudy;

	TomEdit				m_EditImageName;

	TomButton			m_ButtonResetImage;
	TomButton			m_ButtonClearImage;
	TomButton			m_ButtonImageSize;

	TomStatic			m_StaticSelectWindowingBehavior;
	TomButton			m_ButtonNoWindowing;
	TomButton			m_ButtonLinearWindowing;
	TomButton			m_ButtonSigmoidWindowing;
	TomGroup			m_GroupWindowingBehaviorButtons;

	TomStatic			m_StaticGamma;
	TomStatic			m_StaticWindowCenter;
	TomStatic			m_StaticWindowWidth;
	TomEdit				m_EditGamma;
	TomEdit				m_EditWindowCenter;
	TomEdit				m_EditWindowWidth;

	TomButton			m_ButtonInvertColors;
	TomButton			m_ButtonRotateImage;
	TomButton			m_ButtonFlipVertically;
	TomButton			m_ButtonFlipHorizontally;

	TomButton			m_ButtonMeasureDistance;
	TomButton			m_ButtonEraseMeasurements;
	TomButton			m_ButtonCalibrateMeasurements;
	TomButton			m_ButtonEnableAnnotations;

	TomButton			m_ButtonSaveImageSettings;
	TomButton			m_ButtonApplySavedImagePreset;
	TomButton			m_ButtonViewAlternatePage;
	TomButton			m_ButtonPrintReport;
	TomStatic			m_StaticNoDataEntryHere;

	TomButton			m_ButtonShowHistogram;
	TomButton			m_ButtonFlattenHistogram;
	TomButton			m_ButtonCenterHistogram;
	TomStatic			m_StaticHistogram;

public:
// Overrides
	//{{AFX_VIRTUAL(CFrameHeader)
	//}}AFX_VIRTUAL

	void					InitializeControlTips();

	DECLARE_MESSAGE_MAP()

public:
	//{{AFX_MSG(CFrameHeader)
	afx_msg HBRUSH			OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor );
	afx_msg int				OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg void			OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
};


