// ReportSelector.h : Header file defining the structure of the CReportSelector
//  class of CListCtrl, which implements the Report selection list.
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

#include "ReportSelectorHeading.h"


typedef struct _ReportInfo
	{
	char					SubjectName[ 96 ];
	char					SubjectLastName[ 64 ];
	char					SubjectFirstName[ 64 ];
	char					Date[ 12 ];
	char					Time[ 8 ];
	char					ReportFileName[ FULL_FILE_SPEC_STRING_LENGTH ];	// File name without extension or "__ReportPage<n>" suffix.
	struct _ReportInfo		*pNextReportInfo;
	} REPORT_INFO;


typedef struct
	{
	char				*pColumnTitle;
	int					ColumnWidth;
	long				DataStructureOffset;
	} REPORT_LIST_COLUMN_FORMAT;


#define MAX_REPORT_LIST_COLUMNS		5

typedef struct
	{
	unsigned long				nColumns;
	REPORT_LIST_COLUMN_FORMAT	ColumnFormatArray[ MAX_REPORT_LIST_COLUMNS ];
	} REPORT_LIST_FORMAT;


// CReportSelector
class CReportSelector : public CListCtrl
{
public:
	CReportSelector();
	virtual ~CReportSelector();

	REPORT_INFO					*m_pFirstReport;
	REPORT_LIST_FORMAT			*m_pReportListFormat;
	CReportSelectorHeading		m_ReportSelectorHeading;
	int							m_nCurrentlySelectedItem;

public:
	void				UpdateReportListDisplay();
	void				EraseReportList();
	void				CreateReportList();
	void				DisplaySelectedReportImage( REPORT_INFO *pSelectedReportInfo );
	void				UpdateSelectionList();

// Overrides
	//{{AFX_VIRTUAL(CReportSelector)
	//}}AFX_VIRTUAL


	DECLARE_MESSAGE_MAP()
public:
	//{{AFX_MSG(CReportSelector)
	afx_msg int			OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg BOOL		OnEraseBkgnd( CDC *pDC );
	afx_msg void		OnReportItemSelected();
	afx_msg void		OnNMClick( NMHDR *pNMHDR, LRESULT *pResult );
	//}}AFX_MSG
};


