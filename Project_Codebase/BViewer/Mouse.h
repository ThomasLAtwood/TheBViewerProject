// Mouse.h : Header file defining the structure of the CMouse class, which
//  implements the functioning of the mouse buttons and positioning.
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


#define FULL_GRAYSCALE_PREFERENCE_RANGE		200.0
#define GRAYSCALE_PREFERENCE_HALF_RANGE		100.0


class CMouse
{
public:
	CMouse(void);
	virtual ~CMouse(void);

	POINT				m_MouseLocation;
	POINT				m_MouseStartingLocation;
	POINT				m_StartingFocalPoint;			// Image coordinates of the point at the center of the display window.
	int					m_ButtonDown;
							#define NO_BUTTON		0
							#define LEFT_BUTTON		1
							#define	RIGHT_BUTTON	2
	CDiagnosticImage	*m_pTargetImage;
	CWnd				*m_pImageView;
	BOOL				m_bEnableMeasure;


	// Method prototypes.
	//
	void			OnLButtonDown( UINT nFlags, CPoint point );
	void			OnLButtonUp( UINT nFlags, CPoint point );
	void			OnRButtonDown( UINT nFlags, CPoint point );
	void			OnRButtonUp( UINT nFlags, CPoint point );
	void			OnMouseWheel( UINT nFlags, short zDelta, CPoint pt );
	void			OnMouseMove( UINT nFlags, CPoint point );
};
