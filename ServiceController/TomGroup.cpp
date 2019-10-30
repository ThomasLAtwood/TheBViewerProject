// TomGroup.cpp : Implementation of the TomGroup class used
//	to group controls in a user interface window.
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
#include "TomGroup.h"
#include "TomButton.h"
#include "TomStatic.h"
#include "TomEdit.h"


TomGroup::TomGroup( unsigned long GroupType, unsigned long GroupBehavior, unsigned MemberCount, void *pFirstMember, ... )
{
	va_list				VA_Position;
	unsigned			nMember;
	void				*pMember;

	m_GroupType = GroupType;
	m_GroupBehavior = GroupBehavior;
	m_MemberCount = MemberCount;
	// Read in a variable-length list of member pointers.
	m_pMemberPointerArray = (void**)malloc( MemberCount * sizeof( void*) );
	if ( m_pMemberPointerArray != 0 )
		{
		va_start( VA_Position, pFirstMember );
		pMember = pFirstMember;

		for ( nMember = 0; nMember < MemberCount; nMember++ )
			{
			m_pMemberPointerArray[ nMember ] = pMember;
			if ( m_GroupType == BUTTON_PUSHBUTTON || m_GroupType == BUTTON_CHECKBOX )
				( (TomButton*)m_pMemberPointerArray[ nMember ] ) -> m_pGroup = this;
			else if ( m_GroupType == GROUP_EDIT )
				( (TomEdit*)m_pMemberPointerArray[ nMember ] ) -> m_pGroup = this;
			pMember = (void*)va_arg( VA_Position, char* );	// Point to the next function argument in the variable list.
			}
		va_end( VA_Position );
		}
}


TomGroup::~TomGroup(void)
{
	if ( m_pMemberPointerArray != 0 )
		free( m_pMemberPointerArray );
	m_pMemberPointerArray = 0;
}


void TomGroup::InitializeMembers()
{
	unsigned			nMember;

	for ( nMember = 0; nMember < m_MemberCount; nMember++ )
		if ( m_GroupType == BUTTON_CHECKBOX || m_GroupType == BUTTON_PUSHBUTTON )
			( (TomButton*)m_pMemberPointerArray[ nMember ] ) -> Reinitialize();
}


void TomGroup::RespondToSelection( void *pSelectedMember )
{
	unsigned			nMember;
	BOOL				bChangeOfFocus;

	if ( m_GroupBehavior & GROUP_SINGLE_SELECT )
		{
		if ( m_GroupType == BUTTON_CHECKBOX && ((TomButton*)pSelectedMember) -> m_ToggleState == BUTTON_ON )
			{
			for ( nMember = 0; nMember < m_MemberCount; nMember++ )
				if ( m_pMemberPointerArray[ nMember ] != pSelectedMember )
					( (TomButton*)m_pMemberPointerArray[ nMember ] ) -> m_ToggleState = BUTTON_OFF;
			}
		}
	if ( m_GroupBehavior & GROUP_SELECT_FIRST_OR_MULTIPLE )
		{
		if ( m_GroupType == BUTTON_CHECKBOX && ((TomButton*)pSelectedMember) -> m_ToggleState == BUTTON_ON )
			{
			if ( m_pMemberPointerArray[ 0 ] != pSelectedMember )
				( (TomButton*)m_pMemberPointerArray[ 0 ] ) -> m_ToggleState = BUTTON_OFF;
			for ( nMember = 1; nMember < m_MemberCount; nMember++ )
				{
				if ( m_pMemberPointerArray[ nMember ] == pSelectedMember )
					( (TomButton*)m_pMemberPointerArray[ 0 ] ) -> m_ToggleState = BUTTON_OFF;
				if ( m_pMemberPointerArray[ 0 ] == pSelectedMember )
					( (TomButton*)m_pMemberPointerArray[ nMember ] ) -> m_ToggleState = BUTTON_OFF;
				}
			}
		}
	if ( m_GroupBehavior & GROUP_ONE_TOUCHES_ALL )
		{
		if ( ( m_GroupType == BUTTON_CHECKBOX || m_GroupType == BUTTON_PUSHBUTTON ) &&
					( pSelectedMember == 0 || ((TomButton*)pSelectedMember) -> m_ToggleState == BUTTON_ON ) )
			{
			for ( nMember = 0; nMember < m_MemberCount; nMember++ )
				if ( m_pMemberPointerArray[ nMember ] != pSelectedMember &&
							( m_GroupType != BUTTON_CHECKBOX || ( (TomButton*)m_pMemberPointerArray[ nMember ] ) -> m_ToggleState == BUTTON_OFF) )
					( (TomButton*)m_pMemberPointerArray[ nMember ] ) -> m_IdleBkgColor =
								( (TomButton*)m_pMemberPointerArray[ nMember ] ) -> m_VisitedBkgdColor;
			}
		}
	if ( m_GroupBehavior & GROUP_SEQUENCING )
		{
		if ( m_GroupType == GROUP_EDIT && pSelectedMember != 0 )
			{
			bChangeOfFocus = FALSE;
			for ( nMember = 0; nMember < m_MemberCount && !bChangeOfFocus; nMember++ )
				if ( m_pMemberPointerArray[ nMember ] == pSelectedMember )
					{
					bChangeOfFocus = TRUE;
					if ( nMember == m_MemberCount - 1 )
						nMember = 0;
					else
						nMember++;
					( (TomEdit*)m_pMemberPointerArray[ nMember ] ) -> SetFocus();
					}
			}
		}
}


BOOL TomGroup::IsAnyButtonChecked()
{
	unsigned		nMember;
	BOOL			bIsAnyButtonChecked = FALSE;

	if ( m_GroupType == BUTTON_CHECKBOX )
		{
		for ( nMember = 0; nMember < m_MemberCount; nMember++ )
			if ( ( (TomButton*)m_pMemberPointerArray[ nMember ] ) -> m_ToggleState == BUTTON_ON )
				bIsAnyButtonChecked = TRUE;
		}

	return bIsAnyButtonChecked;
}


void TomGroup::SetGroupVisibility( unsigned long Visibility )
{
	unsigned			nMember;

	if ( m_GroupType == BUTTON_CHECKBOX || m_GroupType == BUTTON_PUSHBUTTON )
		{
		for ( nMember = 0; nMember < m_MemberCount; nMember++ )
			if ( Visibility == BUTTON_VISIBLE )
				{
				( (TomButton*)m_pMemberPointerArray[ nMember ] ) -> m_ButtonStyle &= ~BUTTON_INVISIBLE;
				( (TomButton*)m_pMemberPointerArray[ nMember ] ) -> m_ButtonStyle |= BUTTON_VISIBLE;
				if ( ::IsWindow( ( (CWnd*)m_pMemberPointerArray[ nMember ] ) -> m_hWnd ) )
					((CWnd*) m_pMemberPointerArray[ nMember ] ) -> ShowWindow( SW_SHOW );
				}
			else if ( Visibility == BUTTON_INVISIBLE )
				{
				( (TomButton*)m_pMemberPointerArray[ nMember ] ) -> m_ButtonStyle |= BUTTON_INVISIBLE;
				( (TomButton*)m_pMemberPointerArray[ nMember ] ) -> m_ButtonStyle &= ~BUTTON_VISIBLE;
				if ( ::IsWindow( ( (CWnd*)m_pMemberPointerArray[ nMember ] ) -> m_hWnd ) )
					( (CWnd*)m_pMemberPointerArray[ nMember ] ) -> ShowWindow( SW_HIDE );
				}
		}
	else if ( m_GroupType == GROUP_STATIC )
		{
		for ( nMember = 0; nMember < m_MemberCount; nMember++ )
			if ( Visibility == STATIC_VISIBLE )
				{
				( (TomStatic*)m_pMemberPointerArray[ nMember ] ) -> m_StaticStyle &= ~STATIC_INVISIBLE;
				( (TomStatic*)m_pMemberPointerArray[ nMember ] ) -> m_StaticStyle |= STATIC_VISIBLE;
				}
			else if ( Visibility == STATIC_INVISIBLE )
				{
				( (TomStatic*)m_pMemberPointerArray[ nMember ] ) -> m_StaticStyle |= STATIC_INVISIBLE;
				( (TomStatic*)m_pMemberPointerArray[ nMember ] ) -> m_StaticStyle &= ~STATIC_VISIBLE;
				}
		}
}


void TomGroup::HasBeenCompleted( BOOL bHasBeenCompleted )
{
	unsigned			nMember;

	if ( m_GroupType == GROUP_STATIC )
		for ( nMember = 0; nMember < m_MemberCount; nMember++ )
			( (TomStatic*)m_pMemberPointerArray[ nMember ] ) -> HasBeenCompleted( bHasBeenCompleted );
}



