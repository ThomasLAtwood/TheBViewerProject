// StudySelector.h : Header file defining the structure of the CStudySelector class of
//	CListCtrl, which implements the Subject Study selection list.
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
//	*[1] 11/24/2025 by Tom Atwood
//		Added support for display resolution scaling.
//
//
#pragma once

#include "SelectorHeading.h"

typedef struct
	{
	char				*pColumnTitle;
	unsigned long		DatabaseHierarchyLevel;
	int					ColumnWidth;
	long				DataStructureOffset;
	} LIST_COLUMN_FORMAT;

#define MAX_LIST_COLUMNS		30


typedef struct
	{
	unsigned long		nColumns;
	LIST_COLUMN_FORMAT	ColumnFormatArray[ MAX_LIST_COLUMNS ];
	} LIST_FORMAT;


// CStudySelector
class CStudySelector : public CListCtrl
{
public:
	CStudySelector( double ActiveDisplayScaleFactor = 1.0 );		// *[1]
	virtual ~CStudySelector();

	CSelectorHeading		*m_pSelectorHeading;					// *[1] Convert to pointer.
	CFont					m_SelectorHeadingFont;					// *[1] CFont object for the CStudySelector list control.
	int						m_nCurrentlySelectedItem;
	LIST_FORMAT				*m_pListFormat;
	int						m_nColumns;
	int						m_nColumnToSort;
	double					m_ActiveDisplayScaleFactor;				// *[1]


	void				ResetColumnWidth( int nItemAffected, int NewWidth );
	int					GetCurrentlySelectedItem();
	void				UpdatePatientList();
	void				AutoSelectPatientItem( char *pSelectedSOPInstanceUID );
	void				OnHeaderClick( NMLISTVIEW *pListViewNotification, LRESULT *pResult );

// Overrides
protected:
	//{{AFX_VIRTUAL(CStudySelector)
	//}}AFX_VIRTUAL

protected:
	//{{AFX_VIRTUAL( CStudySelector )
	DECLARE_MESSAGE_MAP();
	afx_msg int			OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg void		OnPatientItemSelected();
	afx_msg BOOL		OnEraseBkgnd( CDC *pDC );
	afx_msg void		OnNMClick( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnHdnEndtrack( NMHDR *pNMHDR, LRESULT *pResult );
	//}}AFX_VIRTUAL
};


