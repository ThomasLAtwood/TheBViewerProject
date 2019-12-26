// ImageFrame.h : Header file defining the structure of the CImageFrame class, which
//  implements the image viewing windows within which the subject study, standard
//  and report images are displayed.
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

#include "FrameHeader.h"


// CImageFrame window
class CImageFrame : public CFrameWnd
{
public:
	CImageFrame();
	virtual ~CImageFrame();

// Attributes
public:
	CFrameHeader		m_wndDlgBar;
							#define IMAGE_DIALOG_BAR_HEIGHT		80
	CDiagnosticImage	*m_pAssignedDiagnosticImage;
	CImageView			m_ImageView;
	MONITOR_INFO		*m_pDisplayMonitor;
	char				m_FrameName[ 32 ];
	unsigned long		m_FrameFunction;
						//	#define IMAGE_FRAME_FUNCTION_PATIENT	1
						//	#define IMAGE_FRAME_FUNCTION_STANDARD	2
						//	#define IMAGE_FRAME_FUNCTION_REPORT		3
	CBrush				m_BkgdBrush;
	char				m_CurrentReportFileName[ FULL_FILE_SPEC_STRING_LENGTH ];
	BOOL				m_bReportSavedSuccessfully;
	BOOL				m_bALuminosityTransformationHasBeenApplied;

public:
	void					RebuildHistogram();
	BOOL					LoadReportPage( int nPageNumber, BOOL *pbUseCurrentStudy );
	void					ClearImageDisplay();
	void					PerformUserInput( USER_NOTIFICATION_INFO *pUserNotificationInfo );
	BOOL					GetEditWindowValue( int EditWindowResourceID, double *pNumericalValue );
	void					UpdateEffectiveWindowCenterValue( double WindowCenterValueEntered );
	void					UpdateEffectiveWindowWidthValue( double WindowWidthValueEntered );
	void					ApplyCurrentWindowingSettings();


// Overrides
protected:
	//{{AFX_VIRTUAL(CImageFrame)
	virtual BOOL			PreCreateWindow( CREATESTRUCT &cs);
	virtual BOOL			OnNotify( WPARAM wParam, LPARAM lParam, LRESULT *pResult );
	//}}AFX_VIRTUAL


	// Generated message map functions

	DECLARE_MESSAGE_MAP()

public:
	//{{AFX_MSG(CImageFrame)
	afx_msg int				OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg void			OnShowWindow( BOOL bShow, UINT nStatus );
	afx_msg void			OnSize( UINT nType, int cx, int cy );
	afx_msg BOOL			OnSelectImage( void *pStudy, char *pImagePath, char *pImageFileName, char *pImageFileExtension );
	afx_msg void			OnResetImage( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnClearImage( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnSetImageSize( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnBnClickedNoWindowing( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void			OnBnClickedLinearWindowing( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void			OnBnClickedSigmoidWindowing( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void			OnInvertImageColors( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnRotateImage( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnFlipImageVertically( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnFlipImageHorizontally( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnSaveImageSettings( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnApplyImagePreset( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnButtonMeasureDistance( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnButtonEraseMeasurements( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnButtonCalibrateMeasurements( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnButtonShowHistogram( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnButtonFlattenHistogram( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnButtonCenterHistogram( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnButtonEnableAnnotations( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnSetReportPage( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnPrintReport( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnSaveReport( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg BOOL			OnMouseWheel( UINT nFlags, short zDelta, CPoint pt );
	afx_msg HBRUSH			OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor );
	afx_msg void			OnClose();
	afx_msg void			OnEditGammaKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnEditWindowCenterKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnEditWindowWidthKillFocus( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void			OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	afx_msg void OnPaint();
};


