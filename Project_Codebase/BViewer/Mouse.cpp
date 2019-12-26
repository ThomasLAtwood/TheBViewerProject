// Mouse.cpp : Implementation file for the CMouse class, which
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
#include "StdAfx.h"
#include "BViewer.h"
#include "DiagnosticImage.h"
#include "Mouse.h"
#include "ImageView.h"


CMouse::CMouse( void )
{
	m_pTargetImage = 0;
	m_pImageView = 0;
	m_MouseLocation.x = 0;
	m_MouseLocation.y = 0;
	m_MouseStartingLocation.x = 0;
	m_MouseStartingLocation.y = 0;
	m_bEnableMeasure = FALSE;
}


CMouse::~CMouse( void )
{
}


void CMouse::OnLButtonDown( UINT nFlags, CPoint point )
{
	m_MouseLocation.x = point.x;
	m_MouseLocation.y = point.y;
	m_MouseStartingLocation = m_MouseLocation;
	m_ButtonDown = LEFT_BUTTON;
	if ( m_pTargetImage != 0 )
		{
		m_StartingFocalPoint.x = m_pTargetImage -> m_FocalPoint.x;
		m_StartingFocalPoint.y = m_pTargetImage -> m_FocalPoint.y;
		}
}


void CMouse::OnLButtonUp( UINT nFlags, CPoint point )
{
	m_MouseLocation.x = point.x;
	m_MouseLocation.y = point.y;
	m_ButtonDown = NO_BUTTON;
}


void CMouse::OnRButtonDown( UINT nFlags, CPoint point )
{
	double			MouseHorizontalDisplacement;
	double			MouseVerticalDisplacement;
	RECT			ClientRect;
	int				ClientWidth;
	int				ClientHeight;

	m_MouseLocation.x = point.x;
	m_MouseLocation.y = point.y;
	m_ButtonDown = RIGHT_BUTTON;
	if ( m_pImageView != 0 )
		{
		if ( !m_bEnableMeasure && m_pTargetImage != 0 )
			{
			this -> m_pImageView -> GetClientRect( &ClientRect );
			ClientWidth = ClientRect.right - ClientRect.left;
			ClientHeight = ClientRect.bottom - ClientRect.top;
			MouseHorizontalDisplacement = FULL_GRAYSCALE_PREFERENCE_RANGE * (double)( m_MouseLocation.x - ClientWidth / 2 ) / (double)ClientWidth;
			MouseVerticalDisplacement = -FULL_GRAYSCALE_PREFERENCE_RANGE * (double)( m_MouseLocation.y - ClientHeight / 2 ) / (double)ClientHeight;
			( (CImageView*)m_pImageView ) -> SetImageGrayscalePreference( MouseHorizontalDisplacement, MouseVerticalDisplacement );
			}
		}
}


void CMouse::OnRButtonUp( UINT nFlags, CPoint point )
{
	m_MouseLocation.x = point.x;
	m_MouseLocation.y = point.y;
	m_ButtonDown = NO_BUTTON;
}


void CMouse::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
{
	double			ScaleChange;
	RECT			ClientRect;
	INT				ClientWidth;
	INT				ClientHeight;
	
	this -> m_pImageView -> GetClientRect( &ClientRect );
	ClientWidth = ClientRect.right - ClientRect.left;
	ClientHeight = ClientRect.bottom - ClientRect.top;
	if ( m_pTargetImage != 0 )
		{
		if ( zDelta > 0 )
			ScaleChange = 1.05;
		else
			ScaleChange = 0.95;
		m_pTargetImage -> AdjustScale( ScaleChange );
		}
}


void CMouse::OnMouseMove( UINT nFlags, CPoint point )
{
	POINT			MovementVector;
	double			MouseHorizontalDisplacement;
	double			MouseVerticalDisplacement;
	RECT			ClientRect;
	int				ClientWidth;
	int				ClientHeight;
	
	m_MouseLocation.x = point.x;
	m_MouseLocation.y = point.y;
	switch( m_ButtonDown )
		{
		case NO_BUTTON:
			break;
		case LEFT_BUTTON:
			if ( m_pTargetImage != 0 )
				{
				MovementVector.x = m_MouseLocation.x - m_MouseStartingLocation.x;
				MovementVector.y = m_MouseLocation.y - m_MouseStartingLocation.y;
				m_pTargetImage -> m_FocalPoint.x = m_StartingFocalPoint.x -
									(INT)( (double)MovementVector.x / m_pTargetImage -> m_ScaleFactor );
				m_pTargetImage -> m_FocalPoint.y = m_StartingFocalPoint.y -
									(INT)( (double)MovementVector.y / m_pTargetImage -> m_ScaleFactor );
				if ( m_pImageView != 0 )
					{
					if ( ( (CImageView*)m_pImageView ) -> LoadImageAsTexture() )
						{
						( (CImageView*)m_pImageView ) -> PrepareImage();
						( (CImageView*)m_pImageView ) -> RepaintFast();
						}
					}
				}
			break;
		case RIGHT_BUTTON:
			if ( m_pImageView != 0 )
				{
				if ( !m_bEnableMeasure && m_pTargetImage != 0 )
					{
					m_pImageView -> GetClientRect( &ClientRect );
					ClientWidth = ClientRect.right - ClientRect.left;
					ClientHeight = ClientRect.bottom - ClientRect.top;
					MouseHorizontalDisplacement = FULL_GRAYSCALE_PREFERENCE_RANGE * (double)( m_MouseLocation.x - ClientWidth / 2 ) / (double)ClientWidth;
					MouseVerticalDisplacement = -FULL_GRAYSCALE_PREFERENCE_RANGE * (double)( m_MouseLocation.y - ClientHeight / 2 ) / (double)ClientHeight;
					( (CImageView*)m_pImageView ) -> SetImageGrayscalePreference( MouseHorizontalDisplacement, MouseVerticalDisplacement );
					if ( m_pImageView != 0 )
						((CImageView*)m_pImageView) -> RepaintFast();
					}
				}
			break;
		}
}


