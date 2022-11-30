// SelectStandard.h - Defines the data structures and functions for the standard
//  image selection dialog box.  Clicking on one of the buttons in this dialog box
//  results in the correspoinding standard image being displayed.
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


#define STANDARD_0_EXAMPLE1		0
#define STANDARD_0_EXAMPLE2		1
#define STANDARD_1P				2
#define STANDARD_2P				3
#define STANDARD_3P				4
#define STANDARD_1Q				5
#define STANDARD_2Q				6
#define STANDARD_3Q				7
#define STANDARD_1R				8
#define STANDARD_2R				9
#define STANDARD_3R				10
#define STANDARD_A				11
#define STANDARD_B				12
#define STANDARD_C				13
#define STANDARD_1S				14
#define STANDARD_2S				15
#define STANDARD_3S				16
#define STANDARD_1T				17
#define STANDARD_2T				18
#define STANDARD_3T				19
#define STANDARD_U				20
#define STANDARD_ABN			21
#define STANDARD_CPANGLE		22

#define STANDARD_NAME_SIZE		128

// CSelectStandard dialog class:
class CSelectStandard : public CDialog
{
public:
	CSelectStandard( CWnd *pParent = NULL );
	virtual ~CSelectStandard();

	TomStatic			m_StaticExample1;
	TomStatic			m_StaticExample2;
	TomStatic			m_StaticClickButtonToView;
	TomButton			m_ButtonSelectStdExample1;
	TomButton			m_ButtonSelectStdExample2;

	TomButton			m_ButtonSelectStd_1p;
	TomButton			m_ButtonSelectStd_2p;
	TomButton			m_ButtonSelectStd_3p;

	TomButton			m_ButtonSelectStd_1q;
	TomButton			m_ButtonSelectStd_2q;
	TomButton			m_ButtonSelectStd_3q;

	TomButton			m_ButtonSelectStd_1r;
	TomButton			m_ButtonSelectStd_2r;
	TomButton			m_ButtonSelectStd_3r;

	TomButton			m_ButtonSelectStd_A;
	TomButton			m_ButtonSelectStd_B;
	TomButton			m_ButtonSelectStd_C;

	TomButton			m_ButtonSelectStd_1s;
	TomButton			m_ButtonSelectStd_2s;
	TomButton			m_ButtonSelectStd_3s;

	TomButton			m_ButtonSelectStd_1t;
	TomButton			m_ButtonSelectStd_2t;
	TomButton			m_ButtonSelectStd_3t;

	TomButton			m_ButtonSelectStd_u;
	TomButton			m_ButtonSelectStd_abn;
	TomButton			m_ButtonSelectStd_CPAngle;

	TomGroup			m_GroupSelectStdButtons;

// Dialog Data
	enum { IDD = IDD_DIALOG_SELECT_STD };
	CBrush				m_BkgdBrush;

protected:
// Overrides
	//{{AFX_VIRTUAL(CSelectStandard)
	virtual BOOL	OnInitDialog();
	//}}AFX_VIRTUAL

	void			DisplaytSelectedStandardImage( unsigned long StandardIndex, char *pImageFileName );

	DECLARE_MESSAGE_MAP()
	//{{AFX_VIRTUAL( CSelectStandard )
	afx_msg void OnBnClickedButtonExample1( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonExample2( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStd11pp( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStd22pp( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStd33pp( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStd11qq( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStd22qq( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStd33qq( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStd11rr( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStd22rr( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStd33rr( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStdA( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStdB( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStdC( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStd11st( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStd22ss( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStd33ss( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStd11tt( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStd22tt( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStd33tt( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStduu( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStdabn( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStdCPangle( NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor );
	afx_msg void OnClose();
	afx_msg void OnAppExit( NMHDR *pNMHDR, LRESULT *pResult );
	//}}AFX_VIRTUAL
};


